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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/cached-dict.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/rodict.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/align.h>
#include <hybrid/int128.h>
#include <hybrid/typecore.h>

#include <stdarg.h>

#include "../objects/int_logic.h"
#include "../objects/seq/default-compare.h"
#include "../objects/seq/default-iterators.h"
#include "../objects/seq/default-sequences.h"
#include "../objects/seq/each.h"
#include "../objects/seq/svec.h"
#include "runtime_error.h"
#include "strings.h"

#ifndef INT32_MIN
#include <hybrid/limitcore.h>
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */

#ifndef INT32_MAX
#include <hybrid/limitcore.h>
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */

#ifndef UINT32_MAX
#include <hybrid/limitcore.h>
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */

#ifndef INT64_MIN
#include <hybrid/limitcore.h>
#define INT64_MIN __INT64_MIN__
#endif /* !INT64_MIN */

#ifndef INT64_MAX
#include <hybrid/limitcore.h>
#define INT64_MAX __INT64_MAX__
#endif /* !INT64_MAX */

#ifndef UINT64_MAX
#include <hybrid/limitcore.h>
#define UINT64_MAX __UINT64_MAX__
#endif /* !UINT64_MAX */

#ifndef CHAR_BIT
#include <hybrid/typecore.h>
#define CHAR_BIT __CHAR_BIT__
#endif /* !CHAR_BIT */

#undef byte_t
#define byte_t __BYTE_TYPE__

/************************************************************************/
/* Operator invocation.                                                 */
/************************************************************************/


/* Trace self-optimizing operator inheritance. */
#if 1
#define LOG_INHERIT(base, self, what)                        \
	Dee_DPRINTF("[RT] Inherit `" what "' from %q into %q\n", \
	            (base)->tp_name, (self)->tp_name)
#else
#define LOG_INHERIT(base, self, what) (void)0
#endif

DECL_BEGIN

#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

#define do_fix_negative_range_index(index, size) \
	((size) - ((size_t)(-(index)) % (size)))

#ifndef DEFINE_OPERATOR
#define DEFINE_OPERATOR(return, name, args) \
	PUBLIC return (DCALL DeeObject_##name)args
#endif /* !DEFINE_OPERATOR */
#ifndef DEFINE_INTERNAL_OPERATOR
#define DEFINE_INTERNAL_OPERATOR(return, name, args) \
	INTERN return (DCALL DeeObject_##name)args
#endif /* !DEFINE_INTERNAL_OPERATOR */
#ifndef DEFINE_INTERNAL_SEQ_OPERATOR
#define DEFINE_INTERNAL_SEQ_OPERATOR(return, name, args) \
	INTERN return (DCALL DeeSeq_##name)args
#endif /* !DEFINE_INTERNAL_SEQ_OPERATOR */
#ifndef DEFINE_INTERNAL_SET_OPERATOR
#define DEFINE_INTERNAL_SET_OPERATOR(return, name, args) \
	INTERN return (DCALL DeeSet_##name)args
#endif /* !DEFINE_INTERNAL_SET_OPERATOR */
#ifndef DEFINE_INTERNAL_MAP_OPERATOR
#define DEFINE_INTERNAL_MAP_OPERATOR(return, name, args) \
	INTERN return (DCALL DeeMap_##name)args
#endif /* !DEFINE_INTERNAL_MAP_OPERATOR */

#ifdef DEFINE_TYPED_OPERATORS
#define RESTRICT_IF_NOTYPE /* nothing */
#else /* DEFINE_TYPED_OPERATORS */
#define RESTRICT_IF_NOTYPE __restrict
#endif /* !DEFINE_TYPED_OPERATORS */



/* Setup how operator callbacks are invoked.
 * NOTE: When executing operators in a super-context, we must be
 *       careful when invoking class operators, because otherwise
 *       we'd accidentally invoke the overwritten operator:
 * DEEMON:
 * >> class MyClass {
 * >>     operator + (other) {
 * >>         print "MyClass() +", repr other;
 * >>         return this;
 * >>     }
 * >> }
 * >> class MySubClass: MyClass {
 * >>     operator + (other) {
 * >>         print "MySubClass() +", repr other;
 * >>         return this;
 * >>     }
 * >> }
 * >> class MySubSubClass: MySubClass {
 * >> }
 * >> local inst = MySubSubClass();
 * >> (inst as MyClass) + 7;
 * C:
 * >> DeeObject_TAdd(MyClass, inst, 7);
 * >> func = MyClass->tp_math->tp_add; // func == &instance_add
 * >> // Invoking `func' directly at this point would result in
 * >> // all information about `MyClass' being referred to, to
 * >> // be lost, resulting in `MySubClass.operator + ' to be
 * >> // invoked instead.
 * >> if (func == &instance_add) {
 * >>     instance_tadd(MyClass, inst, 7);
 * >> } else {
 * >>     (*func)(inst, 7);
 * >> }
 * >> // The solution is to manually check for this case, and
 * >> // invoke the typed class function when that happens.
 * NOTE: This problem only arises in a super-context:
 * >> DeeObject_Add(inst, 7); // Same as DeeObject_TAdd(MySubSubClass, inst, 7)
 * >> FIND_FRIST_MATCH(tp_math->tp_add); // Found in `MySubClass'
 * >> INHERIT_MATCH();                   // Inherit `operator +' from `MySubClass' into `MySubSubClass'
 * >>                                    // NOTE: This will only inherit the C-wrapper for the operator,
 * >>                                    //       essentially meaning:
 * >>                                    //       >> MySubSubClass->tp_math = MySubClass->tp_math;
 * >>                                    //       Where `tp_math->tp_add == &instance_add'
 * >> // With the tp_math set of operators now inherited from `MySubClass',
 * >> // we can directly invoke the operator from `MySubSubClass', without
 * >> // the need to check for class operators, because we know that no
 * >> // base type of `inst' before `MySubClass' defined an `operator add'
 * >> (*MySubSubClass->tp_math->tp_add)(inst, 7);  // Invokes `instance_add()'
 * >> // `instance_add()' then invokes the following:
 * >> instance_tadd(MySubSubClass, inst, 7);
 * >> // This will then invoke:
 * >> DeeClass_GetOperator(MySubSubClass, OPERATOR_ADD);
 * >> // After failing to find `OPERATOR_ADD' as part of `MySubSubClass', this
 * >> // function will then continue to search base-classes for the operator,
 * >> // until it finds it in `MySubClass', following which the associated
 * >> // callback will become cached as part of `MySubSubClass'
 * >> INVOKE_OPERATOR(inst, 7);
 * So because of this, we only need to check for class operator callbacks in
 * a super-context, or in other words: when `DEFINE_TYPED_OPERATORS' is defined! */
#ifdef DEFINE_TYPED_OPERATORS
#define DeeType_INVOKE_ASSIGN                            DeeType_InvokeInitAssign
#define DeeType_INVOKE_ASSIGN_NODEFAULT                  DeeType_InvokeInitAssign_NODEFAULT
#define DeeType_INVOKE_MOVEASSIGN                        DeeType_InvokeInitMoveAssign
#define DeeType_INVOKE_MOVEASSIGN_NODEFAULT              DeeType_InvokeInitMoveAssign_NODEFAULT
#define DeeType_INVOKE_STR                               DeeType_InvokeCastStr
#define DeeType_INVOKE_STR_NODEFAULT                     DeeType_InvokeCastStr_NODEFAULT
#define DeeType_INVOKE_PRINT                             DeeType_InvokeCastPrint
#define DeeType_INVOKE_PRINT_NODEFAULT                   DeeType_InvokeCastPrint_NODEFAULT
#define DeeType_INVOKE_REPR                              DeeType_InvokeCastRepr
#define DeeType_INVOKE_REPR_NODEFAULT                    DeeType_InvokeCastRepr_NODEFAULT
#define DeeType_INVOKE_PRINTREPR                         DeeType_InvokeCastPrintRepr
#define DeeType_INVOKE_PRINTREPR_NODEFAULT               DeeType_InvokeCastPrintRepr_NODEFAULT
#define DeeType_INVOKE_BOOL                              DeeType_InvokeCastBool
#define DeeType_INVOKE_BOOL_NODEFAULT                    DeeType_InvokeCastBool_NODEFAULT
#define DeeType_INVOKE_NEXT                              DeeType_InvokeIterNext
#define DeeType_INVOKE_NEXT_NODEFAULT                    DeeType_InvokeIterNext_NODEFAULT
#define DeeType_INVOKE_CALL                              DeeType_InvokeCall
#define DeeType_INVOKE_CALL_NODEFAULT                    DeeType_InvokeCall_NODEFAULT
#define DeeType_INVOKE_CALLKW                            DeeType_InvokeCallKw
#define DeeType_INVOKE_CALLKW_NODEFAULT                  DeeType_InvokeCallKw_NODEFAULT
#define DeeType_INVOKE_INT32                             DeeType_InvokeMathInt32
#define DeeType_INVOKE_INT32_NODEFAULT                   DeeType_InvokeMathInt32_NODEFAULT
#define DeeType_INVOKE_INT64                             DeeType_InvokeMathInt64
#define DeeType_INVOKE_INT64_NODEFAULT                   DeeType_InvokeMathInt64_NODEFAULT
#define DeeType_INVOKE_DOUBLE                            DeeType_InvokeMathDouble
#define DeeType_INVOKE_DOUBLE_NODEFAULT                  DeeType_InvokeMathDouble_NODEFAULT
#define DeeType_INVOKE_INT                               DeeType_InvokeMathInt
#define DeeType_INVOKE_INT_NODEFAULT                     DeeType_InvokeMathInt_NODEFAULT
#define DeeType_INVOKE_INV                               DeeType_InvokeMathInv
#define DeeType_INVOKE_INV_NODEFAULT                     DeeType_InvokeMathInv_NODEFAULT
#define DeeType_INVOKE_POS                               DeeType_InvokeMathPos
#define DeeType_INVOKE_POS_NODEFAULT                     DeeType_InvokeMathPos_NODEFAULT
#define DeeType_INVOKE_NEG                               DeeType_InvokeMathNeg
#define DeeType_INVOKE_NEG_NODEFAULT                     DeeType_InvokeMathNeg_NODEFAULT
#define DeeType_INVOKE_ADD                               DeeType_InvokeMathAdd
#define DeeType_INVOKE_ADD_NODEFAULT                     DeeType_InvokeMathAdd_NODEFAULT
#define DeeType_INVOKE_SUB                               DeeType_InvokeMathSub
#define DeeType_INVOKE_SUB_NODEFAULT                     DeeType_InvokeMathSub_NODEFAULT
#define DeeType_INVOKE_MUL                               DeeType_InvokeMathMul
#define DeeType_INVOKE_MUL_NODEFAULT                     DeeType_InvokeMathMul_NODEFAULT
#define DeeType_INVOKE_DIV                               DeeType_InvokeMathDiv
#define DeeType_INVOKE_DIV_NODEFAULT                     DeeType_InvokeMathDiv_NODEFAULT
#define DeeType_INVOKE_MOD                               DeeType_InvokeMathMod
#define DeeType_INVOKE_MOD_NODEFAULT                     DeeType_InvokeMathMod_NODEFAULT
#define DeeType_INVOKE_SHL                               DeeType_InvokeMathShl
#define DeeType_INVOKE_SHL_NODEFAULT                     DeeType_InvokeMathShl_NODEFAULT
#define DeeType_INVOKE_SHR                               DeeType_InvokeMathShr
#define DeeType_INVOKE_SHR_NODEFAULT                     DeeType_InvokeMathShr_NODEFAULT
#define DeeType_INVOKE_AND                               DeeType_InvokeMathAnd
#define DeeType_INVOKE_AND_NODEFAULT                     DeeType_InvokeMathAnd_NODEFAULT
#define DeeType_INVOKE_OR                                DeeType_InvokeMathOr
#define DeeType_INVOKE_OR_NODEFAULT                      DeeType_InvokeMathOr_NODEFAULT
#define DeeType_INVOKE_XOR                               DeeType_InvokeMathXor
#define DeeType_INVOKE_XOR_NODEFAULT                     DeeType_InvokeMathXor_NODEFAULT
#define DeeType_INVOKE_POW                               DeeType_InvokeMathPow
#define DeeType_INVOKE_POW_NODEFAULT                     DeeType_InvokeMathPow_NODEFAULT
#define DeeType_INVOKE_INC                               DeeType_InvokeMathInc
#define DeeType_INVOKE_INC_NODEFAULT                     DeeType_InvokeMathInc_NODEFAULT
#define DeeType_INVOKE_DEC                               DeeType_InvokeMathDec
#define DeeType_INVOKE_DEC_NODEFAULT                     DeeType_InvokeMathDec_NODEFAULT
#define DeeType_INVOKE_IADD                              DeeType_InvokeMathInplaceAdd
#define DeeType_INVOKE_IADD_NODEFAULT                    DeeType_InvokeMathInplaceAdd_NODEFAULT
#define DeeType_INVOKE_ISUB                              DeeType_InvokeMathInplaceSub
#define DeeType_INVOKE_ISUB_NODEFAULT                    DeeType_InvokeMathInplaceSub_NODEFAULT
#define DeeType_INVOKE_IMUL                              DeeType_InvokeMathInplaceMul
#define DeeType_INVOKE_IMUL_NODEFAULT                    DeeType_InvokeMathInplaceMul_NODEFAULT
#define DeeType_INVOKE_IDIV                              DeeType_InvokeMathInplaceDiv
#define DeeType_INVOKE_IDIV_NODEFAULT                    DeeType_InvokeMathInplaceDiv_NODEFAULT
#define DeeType_INVOKE_IMOD                              DeeType_InvokeMathInplaceMod
#define DeeType_INVOKE_IMOD_NODEFAULT                    DeeType_InvokeMathInplaceMod_NODEFAULT
#define DeeType_INVOKE_ISHL                              DeeType_InvokeMathInplaceShl
#define DeeType_INVOKE_ISHL_NODEFAULT                    DeeType_InvokeMathInplaceShl_NODEFAULT
#define DeeType_INVOKE_ISHR                              DeeType_InvokeMathInplaceShr
#define DeeType_INVOKE_ISHR_NODEFAULT                    DeeType_InvokeMathInplaceShr_NODEFAULT
#define DeeType_INVOKE_IAND                              DeeType_InvokeMathInplaceAnd
#define DeeType_INVOKE_IAND_NODEFAULT                    DeeType_InvokeMathInplaceAnd_NODEFAULT
#define DeeType_INVOKE_IOR                               DeeType_InvokeMathInplaceOr
#define DeeType_INVOKE_IOR_NODEFAULT                     DeeType_InvokeMathInplaceOr_NODEFAULT
#define DeeType_INVOKE_IXOR                              DeeType_InvokeMathInplaceXor
#define DeeType_INVOKE_IXOR_NODEFAULT                    DeeType_InvokeMathInplaceXor_NODEFAULT
#define DeeType_INVOKE_IPOW                              DeeType_InvokeMathInplacePow
#define DeeType_INVOKE_IPOW_NODEFAULT                    DeeType_InvokeMathInplacePow_NODEFAULT
#define DeeType_INVOKE_HASH                              DeeType_InvokeCmpHash
#define DeeType_INVOKE_HASH_NODEFAULT                    DeeType_InvokeCmpHash_NODEFAULT
#define DeeType_INVOKE_EQ                                DeeType_InvokeCmpEq
#define DeeType_INVOKE_EQ_NODEFAULT                      DeeType_InvokeCmpEq_NODEFAULT
#define DeeType_INVOKE_NE                                DeeType_InvokeCmpNe
#define DeeType_INVOKE_NE_NODEFAULT                      DeeType_InvokeCmpNe_NODEFAULT
#define DeeType_INVOKE_LO                                DeeType_InvokeCmpLo
#define DeeType_INVOKE_LO_NODEFAULT                      DeeType_InvokeCmpLo_NODEFAULT
#define DeeType_INVOKE_LE                                DeeType_InvokeCmpLe
#define DeeType_INVOKE_LE_NODEFAULT                      DeeType_InvokeCmpLe_NODEFAULT
#define DeeType_INVOKE_GR                                DeeType_InvokeCmpGr
#define DeeType_INVOKE_GR_NODEFAULT                      DeeType_InvokeCmpGr_NODEFAULT
#define DeeType_INVOKE_GE                                DeeType_InvokeCmpGe
#define DeeType_INVOKE_GE_NODEFAULT                      DeeType_InvokeCmpGe_NODEFAULT
#define DeeType_INVOKE_COMPAREEQ                         DeeType_InvokeCmpCompareEq
#define DeeType_INVOKE_COMPAREEQ_NODEFAULT               DeeType_InvokeCmpCompareEq_NODEFAULT
#define DeeType_INVOKE_COMPARE                           DeeType_InvokeCmpCompare
#define DeeType_INVOKE_COMPARE_NODEFAULT                 DeeType_InvokeCmpCompare_NODEFAULT
#define DeeType_INVOKE_TRYCOMPAREEQ                      DeeType_InvokeCmpTryCompareEq
#define DeeType_INVOKE_TRYCOMPAREEQ_NODEFAULT            DeeType_InvokeCmpTryCompareEq_NODEFAULT
#define DeeType_INVOKE_ITER                              DeeType_InvokeSeqIter
#define DeeType_INVOKE_ITER_NODEFAULT                    DeeType_InvokeSeqIter_NODEFAULT
#define DeeType_INVOKE_SIZEOB                            DeeType_InvokeSeqSizeOb
#define DeeType_INVOKE_SIZEOB_NODEFAULT                  DeeType_InvokeSeqSizeOb_NODEFAULT
#define DeeType_INVOKE_CONTAINS                          DeeType_InvokeSeqContains
#define DeeType_INVOKE_CONTAINS_NODEFAULT                DeeType_InvokeSeqContains_NODEFAULT
#define DeeType_INVOKE_GETITEM                           DeeType_InvokeSeqGetItem
#define DeeType_INVOKE_GETITEM_NODEFAULT                 DeeType_InvokeSeqGetItem_NODEFAULT
#define DeeType_INVOKE_DELITEM                           DeeType_InvokeSeqDelItem
#define DeeType_INVOKE_DELITEM_NODEFAULT                 DeeType_InvokeSeqDelItem_NODEFAULT
#define DeeType_INVOKE_SETITEM                           DeeType_InvokeSeqSetItem
#define DeeType_INVOKE_SETITEM_NODEFAULT                 DeeType_InvokeSeqSetItem_NODEFAULT
#define DeeType_INVOKE_GETRANGE                          DeeType_InvokeSeqGetRange
#define DeeType_INVOKE_GETRANGE_NODEFAULT                DeeType_InvokeSeqGetRange_NODEFAULT
#define DeeType_INVOKE_DELRANGE                          DeeType_InvokeSeqDelRange
#define DeeType_INVOKE_DELRANGE_NODEFAULT                DeeType_InvokeSeqDelRange_NODEFAULT
#define DeeType_INVOKE_SETRANGE                          DeeType_InvokeSeqSetRange
#define DeeType_INVOKE_SETRANGE_NODEFAULT                DeeType_InvokeSeqSetRange_NODEFAULT
#define DeeType_INVOKE_FOREACH                           DeeType_InvokeSeqForeach
#define DeeType_INVOKE_FOREACH_NODEFAULT                 DeeType_InvokeSeqForeach_NODEFAULT
#define DeeType_INVOKE_FOREACH_PAIR                      DeeType_InvokeSeqForeachPair
#define DeeType_INVOKE_FOREACH_PAIR_NODEFAULT            DeeType_InvokeSeqForeachPair_NODEFAULT
#define DeeType_INVOKE_ENUMERATE                         DeeType_InvokeSeqEnumerate
#define DeeType_INVOKE_ENUMERATE_NODEFAULT               DeeType_InvokeSeqEnumerate_NODEFAULT
#define DeeType_INVOKE_ENUMERATE_INDEX                   DeeType_InvokeSeqEnumerateIndex
#define DeeType_INVOKE_ENUMERATE_INDEX_NODEFAULT         DeeType_InvokeSeqEnumerateIndex_NODEFAULT
#define DeeType_INVOKE_BOUNDITEM                         DeeType_InvokeSeqBoundItem
#define DeeType_INVOKE_BOUNDITEM_NODEFAULT               DeeType_InvokeSeqBoundItem_NODEFAULT
#define DeeType_INVOKE_HASITEM                           DeeType_InvokeSeqHasItem
#define DeeType_INVOKE_HASITEM_NODEFAULT                 DeeType_InvokeSeqHasItem_NODEFAULT
#define DeeType_INVOKE_SIZE                              DeeType_InvokeSeqSize
#define DeeType_INVOKE_SIZE_NODEFAULT                    DeeType_InvokeSeqSize_NODEFAULT
#define DeeType_INVOKE_SIZEFAST                          DeeType_InvokeSeqSizeFast
#define DeeType_INVOKE_SIZEFAST_NODEFAULT                DeeType_InvokeSeqSizeFast_NODEFAULT
#define DeeType_INVOKE_GETITEMINDEX                      DeeType_InvokeSeqGetItemIndex
#define DeeType_INVOKE_GETITEMINDEX_NODEFAULT            DeeType_InvokeSeqGetItemIndex_NODEFAULT
#define DeeType_INVOKE_DELITEMINDEX                      DeeType_InvokeSeqDelItemIndex
#define DeeType_INVOKE_DELITEMINDEX_NODEFAULT            DeeType_InvokeSeqDelItemIndex_NODEFAULT
#define DeeType_INVOKE_SETITEMINDEX                      DeeType_InvokeSeqSetItemIndex
#define DeeType_INVOKE_SETITEMINDEX_NODEFAULT            DeeType_InvokeSeqSetItemIndex_NODEFAULT
#define DeeType_INVOKE_BOUNDITEMINDEX                    DeeType_InvokeSeqBoundItemIndex
#define DeeType_INVOKE_BOUNDITEMINDEX_NODEFAULT          DeeType_InvokeSeqBoundItemIndex_NODEFAULT
#define DeeType_INVOKE_HASITEMINDEX                      DeeType_InvokeSeqHasItemIndex
#define DeeType_INVOKE_HASITEMINDEX_NODEFAULT            DeeType_InvokeSeqHasItemIndex_NODEFAULT
#define DeeType_INVOKE_GETRANGEINDEX                     DeeType_InvokeSeqGetRangeIndex
#define DeeType_INVOKE_GETRANGEINDEX_NODEFAULT           DeeType_InvokeSeqGetRangeIndex_NODEFAULT
#define DeeType_INVOKE_DELRANGEINDEX                     DeeType_InvokeSeqDelRangeIndex
#define DeeType_INVOKE_DELRANGEINDEX_NODEFAULT           DeeType_InvokeSeqDelRangeIndex_NODEFAULT
#define DeeType_INVOKE_SETRANGEINDEX                     DeeType_InvokeSeqSetRangeIndex
#define DeeType_INVOKE_SETRANGEINDEX_NODEFAULT           DeeType_InvokeSeqSetRangeIndex_NODEFAULT
#define DeeType_INVOKE_GETRANGEINDEXN                    DeeType_InvokeSeqGetRangeIndexN
#define DeeType_INVOKE_GETRANGEINDEXN_NODEFAULT          DeeType_InvokeSeqGetRangeIndexN_NODEFAULT
#define DeeType_INVOKE_DELRANGEINDEXN                    DeeType_InvokeSeqDelRangeIndexN
#define DeeType_INVOKE_DELRANGEINDEXN_NODEFAULT          DeeType_InvokeSeqDelRangeIndexN_NODEFAULT
#define DeeType_INVOKE_SETRANGEINDEXN                    DeeType_InvokeSeqSetRangeIndexN
#define DeeType_INVOKE_SETRANGEINDEXN_NODEFAULT          DeeType_InvokeSeqSetRangeIndexN_NODEFAULT
#define DeeType_INVOKE_TRYGETITEM                        DeeType_InvokeSeqTryGetItem
#define DeeType_INVOKE_TRYGETITEM_NODEFAULT              DeeType_InvokeSeqTryGetItem_NODEFAULT
#define DeeType_INVOKE_TRYGETITEMINDEX                   DeeType_InvokeSeqTryGetItemIndex
#define DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT         DeeType_InvokeSeqTryGetItemIndex_NODEFAULT
#define DeeType_INVOKE_TRYGETITEMSTRINGHASH              DeeType_InvokeSeqTryGetItemStringHash
#define DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT    DeeType_InvokeSeqTryGetItemStringHash_NODEFAULT
#define DeeType_INVOKE_GETITEMSTRINGHASH                 DeeType_InvokeSeqGetItemStringHash
#define DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT       DeeType_InvokeSeqGetItemStringHash_NODEFAULT
#define DeeType_INVOKE_DELITEMSTRINGHASH                 DeeType_InvokeSeqDelItemStringHash
#define DeeType_INVOKE_DELITEMSTRINGHASH_NODEFAULT       DeeType_InvokeSeqDelItemStringHash_NODEFAULT
#define DeeType_INVOKE_SETITEMSTRINGHASH                 DeeType_InvokeSeqSetItemStringHash
#define DeeType_INVOKE_SETITEMSTRINGHASH_NODEFAULT       DeeType_InvokeSeqSetItemStringHash_NODEFAULT
#define DeeType_INVOKE_BOUNDITEMSTRINGHASH               DeeType_InvokeSeqBoundItemStringHash
#define DeeType_INVOKE_BOUNDITEMSTRINGHASH_NODEFAULT     DeeType_InvokeSeqBoundItemStringHash_NODEFAULT
#define DeeType_INVOKE_HASITEMSTRINGHASH                 DeeType_InvokeSeqHasItemStringHash
#define DeeType_INVOKE_HASITEMSTRINGHASH_NODEFAULT       DeeType_InvokeSeqHasItemStringHash_NODEFAULT
#define DeeType_INVOKE_TRYGETITEMSTRINGLENHASH           DeeType_InvokeSeqTryGetItemStringLenHash
#define DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT DeeType_InvokeSeqTryGetItemStringLenHash_NODEFAULT
#define DeeType_INVOKE_GETITEMSTRINGLENHASH              DeeType_InvokeSeqGetItemStringLenHash
#define DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT    DeeType_InvokeSeqGetItemStringLenHash_NODEFAULT
#define DeeType_INVOKE_DELITEMSTRINGLENHASH              DeeType_InvokeSeqDelItemStringLenHash
#define DeeType_INVOKE_DELITEMSTRINGLENHASH_NODEFAULT    DeeType_InvokeSeqDelItemStringLenHash_NODEFAULT
#define DeeType_INVOKE_SETITEMSTRINGLENHASH              DeeType_InvokeSeqSetItemStringLenHash
#define DeeType_INVOKE_SETITEMSTRINGLENHASH_NODEFAULT    DeeType_InvokeSeqSetItemStringLenHash_NODEFAULT
#define DeeType_INVOKE_BOUNDITEMSTRINGLENHASH            DeeType_InvokeSeqBoundItemStringLenHash
#define DeeType_INVOKE_BOUNDITEMSTRINGLENHASH_NODEFAULT  DeeType_InvokeSeqBoundItemStringLenHash_NODEFAULT
#define DeeType_INVOKE_HASITEMSTRINGLENHASH              DeeType_InvokeSeqHasItemStringLenHash
#define DeeType_INVOKE_HASITEMSTRINGLENHASH_NODEFAULT    DeeType_InvokeSeqHasItemStringLenHash_NODEFAULT
#define DeeType_INVOKE_GETATTR                           DeeType_InvokeAttrGetAttr
#define DeeType_INVOKE_GETATTR_NODEFAULT                 DeeType_InvokeAttrGetAttr_NODEFAULT
#define DeeType_INVOKE_DELATTR                           DeeType_InvokeAttrDelAttr
#define DeeType_INVOKE_DELATTR_NODEFAULT                 DeeType_InvokeAttrDelAttr_NODEFAULT
#define DeeType_INVOKE_SETATTR                           DeeType_InvokeAttrSetAttr
#define DeeType_INVOKE_SETATTR_NODEFAULT                 DeeType_InvokeAttrSetAttr_NODEFAULT
#define DeeType_INVOKE_ENTER                             DeeType_InvokeWithEnter
#define DeeType_INVOKE_ENTER_NODEFAULT                   DeeType_InvokeWithEnter_NODEFAULT
#define DeeType_INVOKE_LEAVE                             DeeType_InvokeWithLeave
#define DeeType_INVOKE_LEAVE_NODEFAULT                   DeeType_InvokeWithLeave_NODEFAULT
#else /* DEFINE_TYPED_OPERATORS */
#define DeeType_INVOKE_ASSIGN(tp_self, self, other)                                  (*(tp_self)->tp_init.tp_assign)(self, other)
#define DeeType_INVOKE_MOVEASSIGN(tp_self, self, other)                              (*(tp_self)->tp_init.tp_move_assign)(self, other)
#define DeeType_INVOKE_STR(tp_self, self)                                            (*(tp_self)->tp_cast.tp_str)(self)
#define DeeType_INVOKE_REPR(tp_self, self)                                           (*(tp_self)->tp_cast.tp_repr)(self)
#define DeeType_INVOKE_PRINT(tp_self, self, printer, arg)                            (*(tp_self)->tp_cast.tp_print)(self, printer, arg)
#define DeeType_INVOKE_PRINTREPR(tp_self, self, printer, arg)                        (*(tp_self)->tp_cast.tp_printrepr)(self, printer, arg)
#define DeeType_INVOKE_BOOL(tp_self, self)                                           (*(tp_self)->tp_cast.tp_bool)(self)
#define DeeType_INVOKE_NEXT(tp_self, self)                                           (*(tp_self)->tp_iter_next)(self)
#define DeeType_INVOKE_CALL(tp_self, self, argc, argv)                               (*(tp_self)->tp_call)(self, argc, argv)
#define DeeType_INVOKE_CALLKW(tp_self, self, argc, argv, kw)                         (*(tp_self)->tp_call_kw)(self, argc, argv, kw)
#define DeeType_INVOKE_INT32(tp_self, self, result)                                  (*(tp_self)->tp_math->tp_int32)(self, result)
#define DeeType_INVOKE_INT64(tp_self, self, result)                                  (*(tp_self)->tp_math->tp_int64)(self, result)
#define DeeType_INVOKE_DOUBLE(tp_self, self, result)                                 (*(tp_self)->tp_math->tp_double)(self, result)
#define DeeType_INVOKE_INT(tp_self, self)                                            (*(tp_self)->tp_math->tp_int)(self)
#define DeeType_INVOKE_INV(tp_self, self)                                            (*(tp_self)->tp_math->tp_inv)(self)
#define DeeType_INVOKE_POS(tp_self, self)                                            (*(tp_self)->tp_math->tp_pos)(self)
#define DeeType_INVOKE_NEG(tp_self, self)                                            (*(tp_self)->tp_math->tp_neg)(self)
#define DeeType_INVOKE_ADD(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_add)(self, other)
#define DeeType_INVOKE_SUB(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_sub)(self, other)
#define DeeType_INVOKE_MUL(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_mul)(self, other)
#define DeeType_INVOKE_DIV(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_div)(self, other)
#define DeeType_INVOKE_MOD(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_mod)(self, other)
#define DeeType_INVOKE_SHL(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_shl)(self, other)
#define DeeType_INVOKE_SHR(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_shr)(self, other)
#define DeeType_INVOKE_AND(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_and)(self, other)
#define DeeType_INVOKE_OR(tp_self, self, other)                                      (*(tp_self)->tp_math->tp_or)(self, other)
#define DeeType_INVOKE_XOR(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_xor)(self, other)
#define DeeType_INVOKE_POW(tp_self, self, other)                                     (*(tp_self)->tp_math->tp_pow)(self, other)
#define DeeType_INVOKE_INC(tp_self, p_self)                                          (*(tp_self)->tp_math->tp_inc)(p_self)
#define DeeType_INVOKE_DEC(tp_self, p_self)                                          (*(tp_self)->tp_math->tp_dec)(p_self)
#define DeeType_INVOKE_IADD(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_add)(p_self, other)
#define DeeType_INVOKE_ISUB(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_sub)(p_self, other)
#define DeeType_INVOKE_IMUL(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_mul)(p_self, other)
#define DeeType_INVOKE_IDIV(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_div)(p_self, other)
#define DeeType_INVOKE_IMOD(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_mod)(p_self, other)
#define DeeType_INVOKE_ISHL(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_shl)(p_self, other)
#define DeeType_INVOKE_ISHR(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_shr)(p_self, other)
#define DeeType_INVOKE_IAND(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_and)(p_self, other)
#define DeeType_INVOKE_IOR(tp_self, p_self, other)                                   (*(tp_self)->tp_math->tp_inplace_or)(p_self, other)
#define DeeType_INVOKE_IXOR(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_xor)(p_self, other)
#define DeeType_INVOKE_IPOW(tp_self, p_self, other)                                  (*(tp_self)->tp_math->tp_inplace_pow)(p_self, other)
#define DeeType_INVOKE_HASH(tp_self, self)                                           (*(tp_self)->tp_cmp->tp_hash)(self)
#define DeeType_INVOKE_EQ(tp_self, self, other)                                      (*(tp_self)->tp_cmp->tp_eq)(self, other)
#define DeeType_INVOKE_NE(tp_self, self, other)                                      (*(tp_self)->tp_cmp->tp_ne)(self, other)
#define DeeType_INVOKE_LO(tp_self, self, other)                                      (*(tp_self)->tp_cmp->tp_lo)(self, other)
#define DeeType_INVOKE_LE(tp_self, self, other)                                      (*(tp_self)->tp_cmp->tp_le)(self, other)
#define DeeType_INVOKE_GR(tp_self, self, other)                                      (*(tp_self)->tp_cmp->tp_gr)(self, other)
#define DeeType_INVOKE_GE(tp_self, self, other)                                      (*(tp_self)->tp_cmp->tp_ge)(self, other)
#define DeeType_INVOKE_COMPAREEQ(tp_self, self, other)                               (*(tp_self)->tp_cmp->tp_compare_eq)(self, other)
#define DeeType_INVOKE_COMPARE(tp_self, self, other)                                 (*(tp_self)->tp_cmp->tp_compare)(self, other)
#define DeeType_INVOKE_TRYCOMPAREEQ(tp_self, self, other)                            (*(tp_self)->tp_cmp->tp_trycompare_eq)(self, other)
#define DeeType_INVOKE_ITER(tp_self, self)                                           (*(tp_self)->tp_seq->tp_iter)(self)
#define DeeType_INVOKE_SIZEOB(tp_self, self)                                         (*(tp_self)->tp_seq->tp_sizeob)(self)
#define DeeType_INVOKE_CONTAINS(tp_self, self, other)                                (*(tp_self)->tp_seq->tp_contains)(self, other)
#define DeeType_INVOKE_GETITEM(tp_self, self, index)                                 (*(tp_self)->tp_seq->tp_getitem)(self, index)
#define DeeType_INVOKE_DELITEM(tp_self, self, index)                                 (*(tp_self)->tp_seq->tp_delitem)(self, index)
#define DeeType_INVOKE_SETITEM(tp_self, self, index, value)                          (*(tp_self)->tp_seq->tp_setitem)(self, index, value)
#define DeeType_INVOKE_GETRANGE(tp_self, self, start, end)                           (*(tp_self)->tp_seq->tp_getrange)(self, start, end)
#define DeeType_INVOKE_DELRANGE(tp_self, self, start, end)                           (*(tp_self)->tp_seq->tp_delrange)(self, start, end)
#define DeeType_INVOKE_SETRANGE(tp_self, self, start, end, values)                   (*(tp_self)->tp_seq->tp_setrange)(self, start, end, values)
#define DeeType_INVOKE_FOREACH(tp_self, self, proc, arg)                             (*(tp_self)->tp_seq->tp_foreach)(self, proc, arg)
#define DeeType_INVOKE_FOREACH_PAIR(tp_self, self, proc, arg)                        (*(tp_self)->tp_seq->tp_foreach_pair)(self, proc, arg)
#define DeeType_INVOKE_ENUMERATE(tp_self, self, proc, arg)                           (*(tp_self)->tp_seq->tp_enumerate)(self, proc, arg)
#define DeeType_INVOKE_ENUMERATE_INDEX(tp_self, self, proc, arg, start, end) (*(tp_self)->tp_seq->tp_enumerate_index)(self, proc, arg, start, end)
#define DeeType_INVOKE_BOUNDITEM(tp_self, self, index)                               (*(tp_self)->tp_seq->tp_bounditem)(self, index)
#define DeeType_INVOKE_HASITEM(tp_self, self, index)                                 (*(tp_self)->tp_seq->tp_hasitem)(self, index)
#define DeeType_INVOKE_SIZE(tp_self, self)                                           (*(tp_self)->tp_seq->tp_size)(self)
#define DeeType_INVOKE_SIZEFAST(tp_self, self)                                       (*(tp_self)->tp_seq->tp_size_fast)(self)
#define DeeType_INVOKE_GETITEMINDEX(tp_self, self, index)                            (*(tp_self)->tp_seq->tp_getitem_index)(self, index)
#define DeeType_INVOKE_DELITEMINDEX(tp_self, self, index)                            (*(tp_self)->tp_seq->tp_delitem_index)(self, index)
#define DeeType_INVOKE_SETITEMINDEX(tp_self, self, index, value)                     (*(tp_self)->tp_seq->tp_setitem_index)(self, index, value)
#define DeeType_INVOKE_BOUNDITEMINDEX(tp_self, self, index)                          (*(tp_self)->tp_seq->tp_bounditem_index)(self, index)
#define DeeType_INVOKE_HASITEMINDEX(tp_self, self, index)                            (*(tp_self)->tp_seq->tp_hasitem_index)(self, index)
#define DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start, end)                      (*(tp_self)->tp_seq->tp_getrange_index)(self, start, end)
#define DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start, end)                      (*(tp_self)->tp_seq->tp_delrange_index)(self, start, end)
#define DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start, end, value)               (*(tp_self)->tp_seq->tp_setrange_index)(self, start, end, value)
#define DeeType_INVOKE_GETRANGEINDEXN(tp_self, self, start)                          (*(tp_self)->tp_seq->tp_getrange_index_n)(self, start)
#define DeeType_INVOKE_DELRANGEINDEXN(tp_self, self, start)                          (*(tp_self)->tp_seq->tp_delrange_index_n)(self, start)
#define DeeType_INVOKE_SETRANGEINDEXN(tp_self, self, start, value)                   (*(tp_self)->tp_seq->tp_setrange_index_n)(self, start, value)
#define DeeType_INVOKE_TRYGETITEM(tp_self, self, index)                              (*(tp_self)->tp_seq->tp_trygetitem)(self, index)
#define DeeType_INVOKE_TRYGETITEMINDEX(tp_self, self, index)                         (*(tp_self)->tp_seq->tp_trygetitem_index)(self, index)
#define DeeType_INVOKE_TRYGETITEMSTRINGHASH(tp_self, self, key, hash)                (*(tp_self)->tp_seq->tp_trygetitem_string_hash)(self, key, hash)
#define DeeType_INVOKE_GETITEMSTRINGHASH(tp_self, self, key, hash)                   (*(tp_self)->tp_seq->tp_getitem_string_hash)(self, key, hash)
#define DeeType_INVOKE_DELITEMSTRINGHASH(tp_self, self, key, hash)                   (*(tp_self)->tp_seq->tp_delitem_string_hash)(self, key, hash)
#define DeeType_INVOKE_SETITEMSTRINGHASH(tp_self, self, key, hash, value)            (*(tp_self)->tp_seq->tp_setitem_string_hash)(self, key, hash, value)
#define DeeType_INVOKE_BOUNDITEMSTRINGHASH(tp_self, self, key, hash)                 (*(tp_self)->tp_seq->tp_bounditem_string_hash)(self, key, hash)
#define DeeType_INVOKE_HASITEMSTRINGHASH(tp_self, self, key, hash)                   (*(tp_self)->tp_seq->tp_hasitem_string_hash)(self, key, hash)
#define DeeType_INVOKE_TRYGETITEMSTRINGLENHASH(tp_self, self, key, keylen, hash)     (*(tp_self)->tp_seq->tp_trygetitem_string_len_hash)(self, key, keylen, hash)
#define DeeType_INVOKE_GETITEMSTRINGLENHASH(tp_self, self, key, keylen, hash)        (*(tp_self)->tp_seq->tp_getitem_string_len_hash)(self, key, keylen, hash)
#define DeeType_INVOKE_DELITEMSTRINGLENHASH(tp_self, self, key, keylen, hash)        (*(tp_self)->tp_seq->tp_delitem_string_len_hash)(self, key, keylen, hash)
#define DeeType_INVOKE_SETITEMSTRINGLENHASH(tp_self, self, key, keylen, hash, value) (*(tp_self)->tp_seq->tp_setitem_string_len_hash)(self, key, keylen, hash, value)
#define DeeType_INVOKE_BOUNDITEMSTRINGLENHASH(tp_self, self, key, keylen, hash)      (*(tp_self)->tp_seq->tp_bounditem_string_len_hash)(self, key, keylen, hash)
#define DeeType_INVOKE_HASITEMSTRINGLENHASH(tp_self, self, key, keylen, hash)        (*(tp_self)->tp_seq->tp_hasitem_string_len_hash)(self, key, keylen, hash)
#define DeeType_INVOKE_GETATTR(tp_self, self, name)                                  (*(tp_self)->tp_attr->tp_getattr)(self, name)
#define DeeType_INVOKE_DELATTR(tp_self, self, name)                                  (*(tp_self)->tp_attr->tp_delattr)(self, name)
#define DeeType_INVOKE_SETATTR(tp_self, self, name, value)                           (*(tp_self)->tp_attr->tp_setattr)(self, name, value)
#define DeeType_INVOKE_ENTER(tp_self, self)                                          (*(tp_self)->tp_with->tp_enter)(self)
#define DeeType_INVOKE_LEAVE(tp_self, self)                                          (*(tp_self)->tp_with->tp_leave)(self)

#define DeeType_INVOKE_ASSIGN_NODEFAULT                  DeeType_INVOKE_ASSIGN
#define DeeType_INVOKE_MOVEASSIGN_NODEFAULT              DeeType_INVOKE_MOVEASSIGN
#define DeeType_INVOKE_STR_NODEFAULT                     DeeType_INVOKE_STR
#define DeeType_INVOKE_REPR_NODEFAULT                    DeeType_INVOKE_REPR
#define DeeType_INVOKE_PRINT_NODEFAULT                   DeeType_INVOKE_PRINT
#define DeeType_INVOKE_PRINTREPR_NODEFAULT               DeeType_INVOKE_PRINTREPR
#define DeeType_INVOKE_BOOL_NODEFAULT                    DeeType_INVOKE_BOOL
#define DeeType_INVOKE_NEXT_NODEFAULT                    DeeType_INVOKE_NEXT
#define DeeType_INVOKE_CALL_NODEFAULT                    DeeType_INVOKE_CALL
#define DeeType_INVOKE_CALLKW_NODEFAULT                  DeeType_INVOKE_CALLKW
#define DeeType_INVOKE_INT32_NODEFAULT                   DeeType_INVOKE_INT32
#define DeeType_INVOKE_INT64_NODEFAULT                   DeeType_INVOKE_INT64
#define DeeType_INVOKE_DOUBLE_NODEFAULT                  DeeType_INVOKE_DOUBLE
#define DeeType_INVOKE_INT_NODEFAULT                     DeeType_INVOKE_INT
#define DeeType_INVOKE_INV_NODEFAULT                     DeeType_INVOKE_INV
#define DeeType_INVOKE_POS_NODEFAULT                     DeeType_INVOKE_POS
#define DeeType_INVOKE_NEG_NODEFAULT                     DeeType_INVOKE_NEG
#define DeeType_INVOKE_ADD_NODEFAULT                     DeeType_INVOKE_ADD
#define DeeType_INVOKE_SUB_NODEFAULT                     DeeType_INVOKE_SUB
#define DeeType_INVOKE_MUL_NODEFAULT                     DeeType_INVOKE_MUL
#define DeeType_INVOKE_DIV_NODEFAULT                     DeeType_INVOKE_DIV
#define DeeType_INVOKE_MOD_NODEFAULT                     DeeType_INVOKE_MOD
#define DeeType_INVOKE_SHL_NODEFAULT                     DeeType_INVOKE_SHL
#define DeeType_INVOKE_SHR_NODEFAULT                     DeeType_INVOKE_SHR
#define DeeType_INVOKE_AND_NODEFAULT                     DeeType_INVOKE_AND
#define DeeType_INVOKE_OR_NODEFAULT                      DeeType_INVOKE_OR
#define DeeType_INVOKE_XOR_NODEFAULT                     DeeType_INVOKE_XOR
#define DeeType_INVOKE_POW_NODEFAULT                     DeeType_INVOKE_POW
#define DeeType_INVOKE_INC_NODEFAULT                     DeeType_INVOKE_INC
#define DeeType_INVOKE_DEC_NODEFAULT                     DeeType_INVOKE_DEC
#define DeeType_INVOKE_IADD_NODEFAULT                    DeeType_INVOKE_IADD
#define DeeType_INVOKE_ISUB_NODEFAULT                    DeeType_INVOKE_ISUB
#define DeeType_INVOKE_IMUL_NODEFAULT                    DeeType_INVOKE_IMUL
#define DeeType_INVOKE_IDIV_NODEFAULT                    DeeType_INVOKE_IDIV
#define DeeType_INVOKE_IMOD_NODEFAULT                    DeeType_INVOKE_IMOD
#define DeeType_INVOKE_ISHL_NODEFAULT                    DeeType_INVOKE_ISHL
#define DeeType_INVOKE_ISHR_NODEFAULT                    DeeType_INVOKE_ISHR
#define DeeType_INVOKE_IAND_NODEFAULT                    DeeType_INVOKE_IAND
#define DeeType_INVOKE_IOR_NODEFAULT                     DeeType_INVOKE_IOR
#define DeeType_INVOKE_IXOR_NODEFAULT                    DeeType_INVOKE_IXOR
#define DeeType_INVOKE_IPOW_NODEFAULT                    DeeType_INVOKE_IPOW
#define DeeType_INVOKE_HASH_NODEFAULT                    DeeType_INVOKE_HASH
#define DeeType_INVOKE_EQ_NODEFAULT                      DeeType_INVOKE_EQ
#define DeeType_INVOKE_NE_NODEFAULT                      DeeType_INVOKE_NE
#define DeeType_INVOKE_LO_NODEFAULT                      DeeType_INVOKE_LO
#define DeeType_INVOKE_LE_NODEFAULT                      DeeType_INVOKE_LE
#define DeeType_INVOKE_GR_NODEFAULT                      DeeType_INVOKE_GR
#define DeeType_INVOKE_GE_NODEFAULT                      DeeType_INVOKE_GE
#define DeeType_INVOKE_COMPAREEQ_NODEFAULT               DeeType_INVOKE_COMPAREEQ
#define DeeType_INVOKE_COMPARE_NODEFAULT                 DeeType_INVOKE_COMPARE
#define DeeType_INVOKE_TRYCOMPAREEQ_NODEFAULT            DeeType_INVOKE_TRYCOMPAREEQ
#define DeeType_INVOKE_ITER_NODEFAULT                    DeeType_INVOKE_ITER
#define DeeType_INVOKE_SIZEOB_NODEFAULT                  DeeType_INVOKE_SIZEOB
#define DeeType_INVOKE_CONTAINS_NODEFAULT                DeeType_INVOKE_CONTAINS
#define DeeType_INVOKE_GETITEM_NODEFAULT                 DeeType_INVOKE_GETITEM
#define DeeType_INVOKE_DELITEM_NODEFAULT                 DeeType_INVOKE_DELITEM
#define DeeType_INVOKE_SETITEM_NODEFAULT                 DeeType_INVOKE_SETITEM
#define DeeType_INVOKE_GETRANGE_NODEFAULT                DeeType_INVOKE_GETRANGE
#define DeeType_INVOKE_DELRANGE_NODEFAULT                DeeType_INVOKE_DELRANGE
#define DeeType_INVOKE_SETRANGE_NODEFAULT                DeeType_INVOKE_SETRANGE
#define DeeType_INVOKE_FOREACH_NODEFAULT                 DeeType_INVOKE_FOREACH
#define DeeType_INVOKE_FOREACH_PAIR_NODEFAULT            DeeType_INVOKE_FOREACH_PAIR
#define DeeType_INVOKE_ENUMERATE_NODEFAULT               DeeType_INVOKE_ENUMERATE
#define DeeType_INVOKE_ENUMERATE_INDEX_NODEFAULT         DeeType_INVOKE_ENUMERATE_INDEX
#define DeeType_INVOKE_BOUNDITEM_NODEFAULT               DeeType_INVOKE_BOUNDITEM
#define DeeType_INVOKE_HASITEM_NODEFAULT                 DeeType_INVOKE_HASITEM
#define DeeType_INVOKE_SIZE_NODEFAULT                    DeeType_INVOKE_SIZE
#define DeeType_INVOKE_SIZEFAST_NODEFAULT                DeeType_INVOKE_SIZEFAST
#define DeeType_INVOKE_GETITEMINDEX_NODEFAULT            DeeType_INVOKE_GETITEMINDEX
#define DeeType_INVOKE_DELITEMINDEX_NODEFAULT            DeeType_INVOKE_DELITEMINDEX
#define DeeType_INVOKE_SETITEMINDEX_NODEFAULT            DeeType_INVOKE_SETITEMINDEX
#define DeeType_INVOKE_BOUNDITEMINDEX_NODEFAULT          DeeType_INVOKE_BOUNDITEMINDEX
#define DeeType_INVOKE_HASITEMINDEX_NODEFAULT            DeeType_INVOKE_HASITEMINDEX
#define DeeType_INVOKE_GETRANGEINDEX_NODEFAULT           DeeType_INVOKE_GETRANGEINDEX
#define DeeType_INVOKE_DELRANGEINDEX_NODEFAULT           DeeType_INVOKE_DELRANGEINDEX
#define DeeType_INVOKE_SETRANGEINDEX_NODEFAULT           DeeType_INVOKE_SETRANGEINDEX
#define DeeType_INVOKE_GETRANGEINDEXN_NODEFAULT          DeeType_INVOKE_GETRANGEINDEXN
#define DeeType_INVOKE_DELRANGEINDEXN_NODEFAULT          DeeType_INVOKE_DELRANGEINDEXN
#define DeeType_INVOKE_SETRANGEINDEXN_NODEFAULT          DeeType_INVOKE_SETRANGEINDEXN
#define DeeType_INVOKE_TRYGETITEM_NODEFAULT              DeeType_INVOKE_TRYGETITEM
#define DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT         DeeType_INVOKE_TRYGETITEMINDEX
#define DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT    DeeType_INVOKE_TRYGETITEMSTRINGHASH
#define DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT       DeeType_INVOKE_GETITEMSTRINGHASH
#define DeeType_INVOKE_DELITEMSTRINGHASH_NODEFAULT       DeeType_INVOKE_DELITEMSTRINGHASH
#define DeeType_INVOKE_SETITEMSTRINGHASH_NODEFAULT       DeeType_INVOKE_SETITEMSTRINGHASH
#define DeeType_INVOKE_BOUNDITEMSTRINGHASH_NODEFAULT     DeeType_INVOKE_BOUNDITEMSTRINGHASH
#define DeeType_INVOKE_HASITEMSTRINGHASH_NODEFAULT       DeeType_INVOKE_HASITEMSTRINGHASH
#define DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT DeeType_INVOKE_TRYGETITEMSTRINGLENHASH
#define DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT    DeeType_INVOKE_GETITEMSTRINGLENHASH
#define DeeType_INVOKE_DELITEMSTRINGLENHASH_NODEFAULT    DeeType_INVOKE_DELITEMSTRINGLENHASH
#define DeeType_INVOKE_SETITEMSTRINGLENHASH_NODEFAULT    DeeType_INVOKE_SETITEMSTRINGLENHASH
#define DeeType_INVOKE_BOUNDITEMSTRINGLENHASH_NODEFAULT  DeeType_INVOKE_BOUNDITEMSTRINGLENHASH
#define DeeType_INVOKE_HASITEMSTRINGLENHASH_NODEFAULT    DeeType_INVOKE_HASITEMSTRINGLENHASH
#define DeeType_INVOKE_GETATTR_NODEFAULT                 DeeType_INVOKE_GETATTR
#define DeeType_INVOKE_DELATTR_NODEFAULT                 DeeType_INVOKE_DELATTR
#define DeeType_INVOKE_SETATTR_NODEFAULT                 DeeType_INVOKE_SETATTR
#define DeeType_INVOKE_ENTER_NODEFAULT                   DeeType_INVOKE_ENTER
#define DeeType_INVOKE_LEAVE_NODEFAULT                   DeeType_INVOKE_LEAVE
#endif /* !DEFINE_TYPED_OPERATORS */

/* CONFIG: Allow types that are inheriting their constructors to
 *         become GC objects when the object that they're inheriting
 *         from wasn't one.
 *         This is probably not something that should ever happen
 *         and if this were ever to occur, it would probably be
 *         a mistake.
 *         But still: It is something that ?~could~? make sense to allow
 *         In any case: A GC-enabled object providing inheritable constructors
 *                      to non-GC objects is something that's definitely illegal!
 * Anyways: Since the specs only state that an active VAR-flag must be inherited
 *          by all sub-classes, yet remains silent on how the GC type-flag must
 *          behave when it comes to inherited constructors, enabling this option
 *          is the best course of action, considering it opens up the possibility
 *          of further, quite well-defined behavioral options or GC-objects
 *          inheriting their constructor from non-GC sub-classes.
 */
#undef CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS
#define CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS

#ifndef DEFINE_TYPED_OPERATORS
/* Assuming that the operator-class at offset `oi_class' is non-NULL,
 * check if that class has been inherited from a direct base of `self'.
 *
 * If so, return that base-type. If not, return `NULL'. */
INTERN WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeType_GetOpClassOrigin(DeeTypeObject *__restrict self, uint16_t oi_class) {
	DeeTypeMRO mro;
	void *cls = *(void **)((byte_t *)self + oi_class);
	DeeTypeObject *base;
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		void *base_cls = *(void **)((byte_t *)base + oi_class);
		if (cls == base_cls)
			return base;
	}
	return NULL;
}

#define DeeType_GetMathOrigin(self)   DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_math))
#define DeeType_GetCmpOrigin(self)    DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_cmp))
#define DeeType_GetSeqOrigin(self)    DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_seq))
#define DeeType_GetWithOrigin(self)   DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_with))
#define DeeType_GetBufferOrigin(self) DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_buffer))


INTERN NONNULL((1)) bool DCALL
DeeType_InheritConstructors(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	if (!(self->tp_flags & TP_FINHERITCTOR))
		return false;
	base = self->tp_base;
	if (base == NULL)
		return false;
	DeeType_InheritConstructors(base);
	ASSERT((base->tp_flags & TP_FVARIABLE) ==
	       (self->tp_flags & TP_FVARIABLE));
	LOG_INHERIT(base, self, "operator constructor");
	if (self->tp_flags & TP_FVARIABLE) {
		self->tp_init.tp_var.tp_ctor        = base->tp_init.tp_var.tp_ctor;
		self->tp_init.tp_var.tp_copy_ctor   = base->tp_init.tp_var.tp_copy_ctor;
		self->tp_init.tp_var.tp_deep_ctor   = base->tp_init.tp_var.tp_deep_ctor;
		self->tp_init.tp_var.tp_any_ctor    = base->tp_init.tp_var.tp_any_ctor;
		self->tp_init.tp_var.tp_free        = base->tp_init.tp_var.tp_free;
		self->tp_init.tp_var.tp_any_ctor_kw = base->tp_init.tp_var.tp_any_ctor_kw;
	} else {
#if 0 /* Allocators should not be inheritable! */
		if (base->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS
			ASSERT((base->tp_flags & TP_FGC) == (self->tp_flags & TP_FGC));
#else /* !CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
			ASSERTF(!(base->tp_flags & TP_FGC) || (self->tp_flags & TP_FGC),
			        "Non-GC object is inheriting its constructors for a GC-enabled object");
#endif /* CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
			self->tp_init.tp_alloc.tp_alloc = base->tp_init.tp_alloc.tp_alloc;
			self->tp_init.tp_alloc.tp_free  = base->tp_init.tp_alloc.tp_free;
		}
#endif
		self->tp_init.tp_alloc.tp_ctor        = base->tp_init.tp_alloc.tp_ctor;
		self->tp_init.tp_alloc.tp_copy_ctor   = base->tp_init.tp_alloc.tp_copy_ctor;
		self->tp_init.tp_alloc.tp_deep_ctor   = base->tp_init.tp_alloc.tp_deep_ctor;
		self->tp_init.tp_alloc.tp_any_ctor    = base->tp_init.tp_alloc.tp_any_ctor;
		self->tp_init.tp_alloc.tp_any_ctor_kw = base->tp_init.tp_alloc.tp_any_ctor_kw;
	}
	self->tp_init.tp_deepload = base->tp_init.tp_deepload;

	/* Only inherit assign operators if the class itself doesn't define any already. */
	if (self->tp_init.tp_assign == NULL)
		self->tp_init.tp_assign = base->tp_init.tp_assign;
	if (self->tp_init.tp_move_assign == NULL)
		self->tp_init.tp_move_assign = base->tp_init.tp_move_assign;
	return true;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_NewDefault(DeeTypeObject *__restrict object_type) {
	DREF DeeObject *result;
	ASSERT_OBJECT(object_type);
	ASSERT(DeeType_Check(object_type));
	if (object_type->tp_flags & TP_FVARIABLE) {
		if (object_type->tp_init.tp_var.tp_ctor) {
do_invoke_var_ctor:
			return (*object_type->tp_init.tp_var.tp_ctor)();
		}
		if (object_type->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
			return (*object_type->tp_init.tp_var.tp_any_ctor)(0, NULL);
		}
		if (object_type->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
			return (*object_type->tp_init.tp_var.tp_any_ctor_kw)(0, NULL, NULL);
		}
		if (DeeType_InheritConstructors(object_type)) {
			if (object_type->tp_init.tp_var.tp_ctor)
				goto do_invoke_var_ctor;
			if (object_type->tp_init.tp_var.tp_any_ctor)
				goto do_invoke_var_any_ctor;
			if (object_type->tp_init.tp_var.tp_any_ctor_kw)
				goto do_invoke_var_any_ctor_kw;
		}
	} else {
		int error;
		ASSERT(!(object_type->tp_flags & TP_FVARIABLE));
		result = DeeType_AllocInstance(object_type);
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, object_type);
		if (object_type->tp_init.tp_alloc.tp_ctor) {
do_invoke_alloc_ctor:
			error = (*object_type->tp_init.tp_alloc.tp_ctor)(result);
		} else if (object_type->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
			error = (*object_type->tp_init.tp_alloc.tp_any_ctor)(result, 0, NULL);
		} else if (object_type->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
			error = (*object_type->tp_init.tp_alloc.tp_any_ctor_kw)(result, 0, NULL, NULL);
		} else {
			DeeObject_FreeTracker(result);
			DeeType_FreeInstance(object_type, result);
			if (!DeeType_InheritConstructors(object_type))
				goto err_not_implemented;
			result = DeeType_AllocInstance(object_type);
			if unlikely(!result)
				goto err_object_type;
			DeeObject_InitNoref(result, object_type);
			if (object_type->tp_init.tp_alloc.tp_ctor)
				goto do_invoke_alloc_ctor;
			if (object_type->tp_init.tp_alloc.tp_any_ctor)
				goto do_invoke_alloc_any_ctor;
			if (object_type->tp_init.tp_alloc.tp_any_ctor_kw)
				goto do_invoke_alloc_any_ctor_kw;
			goto err_not_implemented_r;
		}
		if unlikely(error)
			goto err_r;
		/* Begin tracking the returned object. */
		if (object_type->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
		return result;
	}
err_not_implemented:
	err_unimplemented_constructor(object_type, 0, NULL);
err:
	return NULL;
err_not_implemented_r:
	err_unimplemented_constructor(object_type, 0, NULL);
err_r:
	DeeObject_FreeTracker(result);
	DeeType_FreeInstance(object_type, result);
err_object_type:
	Dee_DecrefNokill(object_type);
	goto err;
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_New(DeeTypeObject *object_type, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	ASSERT_OBJECT(object_type);
	ASSERT(DeeType_Check(object_type));
	if (object_type->tp_flags & TP_FVARIABLE) {
		if (object_type->tp_init.tp_var.tp_ctor && !argc) {
do_invoke_var_ctor:
			return (*object_type->tp_init.tp_var.tp_ctor)();
		}
		if (object_type->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
			return (*object_type->tp_init.tp_var.tp_any_ctor)(argc, argv);
		}
		if (object_type->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
			return (*object_type->tp_init.tp_var.tp_any_ctor_kw)(argc, argv, NULL);
		}
		if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
		    DeeObject_InstanceOf(argv[0], object_type)) {
do_invoke_var_copy:
			return (*object_type->tp_init.tp_var.tp_copy_ctor)(argv[0]);
		}
		if (DeeType_InheritConstructors(object_type)) {
			if (object_type->tp_init.tp_var.tp_ctor && !argc)
				goto do_invoke_var_ctor;
			if (object_type->tp_init.tp_var.tp_any_ctor)
				goto do_invoke_var_any_ctor;
			if (object_type->tp_init.tp_var.tp_any_ctor_kw)
				goto do_invoke_var_any_ctor_kw;
			if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
			    DeeObject_InstanceOf(argv[0], object_type))
				goto do_invoke_var_copy;
		}
	} else {
		int error;
		ASSERT(!(object_type->tp_flags & TP_FVARIABLE));
		result = DeeType_AllocInstance(object_type);
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, object_type);
		if (object_type->tp_init.tp_alloc.tp_ctor && !argc) {
do_invoke_alloc_ctor:
			error = (*object_type->tp_init.tp_alloc.tp_ctor)(result);
		} else if (object_type->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
			error = (*object_type->tp_init.tp_alloc.tp_any_ctor)(result, argc, argv);
		} else if (object_type->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
			error = (*object_type->tp_init.tp_alloc.tp_any_ctor_kw)(result, argc, argv, NULL);
		} else if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
		           DeeObject_InstanceOf(argv[0], object_type)) {
do_invoke_alloc_copy:
			error = (*object_type->tp_init.tp_alloc.tp_copy_ctor)(result, argv[0]);
		} else {
			DeeObject_FreeTracker(result);
			DeeType_FreeInstance(object_type, result);
			if (!DeeType_InheritConstructors(object_type))
				goto err_not_implemented;
			result = DeeType_AllocInstance(object_type);
			if unlikely(!result)
				goto err_object_type;
			DeeObject_InitNoref(result, object_type);
			if (object_type->tp_init.tp_alloc.tp_ctor && argc == 0)
				goto do_invoke_alloc_ctor;
			if (object_type->tp_init.tp_alloc.tp_any_ctor)
				goto do_invoke_alloc_any_ctor;
			if (object_type->tp_init.tp_alloc.tp_any_ctor_kw)
				goto do_invoke_alloc_any_ctor_kw;
			if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
			    DeeObject_InstanceOf(argv[0], object_type))
				goto do_invoke_alloc_copy;
			goto err_not_implemented_r;
		}
		if unlikely(error)
			goto err_r;
		/* Begin tracking the returned object. */
		if (object_type->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
		return result;
	}
err_not_implemented:
	err_unimplemented_constructor(object_type, argc, argv);
err:
	return NULL;
err_not_implemented_r:
	err_unimplemented_constructor(object_type, argc, argv);
err_r:
	DeeObject_FreeTracker(result);
	DeeType_FreeInstance(object_type, result);
err_object_type:
	Dee_DecrefNokill(object_type);
	goto err;
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_NewKw(DeeTypeObject *object_type, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	ASSERT_OBJECT(object_type);
	ASSERT(DeeType_Check(object_type));
	ASSERT(!kw || DeeObject_IsKw(kw));
	if (object_type->tp_flags & TP_FVARIABLE) {
		if (object_type->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
			return (*object_type->tp_init.tp_var.tp_any_ctor_kw)(argc, argv, kw);
		}
		if (object_type->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
			if (kw) {
				if (DeeKwds_Check(kw)) {
					if (DeeKwds_SIZE(kw) != 0)
						goto err_no_keywords;
				} else {
					size_t kw_size = DeeObject_Size(kw);
					if unlikely(kw_size == (size_t)-1)
						goto err;
					if (kw_size != 0)
						goto err_no_keywords;
				}
			}
			if (object_type->tp_init.tp_var.tp_ctor && !argc)
				goto do_invoke_var_ctor_nokw;
			return (*object_type->tp_init.tp_var.tp_any_ctor)(argc, argv);
		}
		if (object_type->tp_init.tp_var.tp_ctor && !argc) {
do_invoke_var_ctor:
			if (kw) {
				if (DeeKwds_Check(kw)) {
					if (DeeKwds_SIZE(kw) != 0)
						goto err_no_keywords;
				} else {
					size_t kw_size = DeeObject_Size(kw);
					if unlikely(kw_size == (size_t)-1)
						goto err;
					if (kw_size != 0)
						goto err_no_keywords;
				}
			}
do_invoke_var_ctor_nokw:
			return (*object_type->tp_init.tp_var.tp_ctor)();
		}
		if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
		    DeeObject_InstanceOf(argv[0], object_type)) {
do_invoke_var_copy:
			if (kw) {
				if (DeeKwds_Check(kw)) {
					if (DeeKwds_SIZE(kw) != 0)
						goto err_no_keywords;
				} else {
					size_t kw_size = DeeObject_Size(kw);
					if unlikely(kw_size == (size_t)-1)
						goto err;
					if (kw_size != 0)
						goto err_no_keywords;
				}
			}
			return (*object_type->tp_init.tp_var.tp_copy_ctor)(argv[0]);
		}
		if (DeeType_InheritConstructors(object_type)) {
			if (object_type->tp_init.tp_var.tp_any_ctor_kw)
				goto do_invoke_var_any_ctor_kw;
			if (object_type->tp_init.tp_var.tp_any_ctor)
				goto do_invoke_var_any_ctor;
			if (object_type->tp_init.tp_var.tp_ctor && !argc)
				goto do_invoke_var_ctor;
			if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
			    DeeObject_InstanceOf(argv[0], object_type))
				goto do_invoke_var_copy;
		}
	} else {
		int error;
		ASSERT(!(object_type->tp_flags & TP_FVARIABLE));
		result = DeeType_AllocInstance(object_type);
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, object_type);
		if (object_type->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
			error = (*object_type->tp_init.tp_alloc.tp_any_ctor_kw)(result, argc, argv, kw);
		} else if (object_type->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
			if (kw) {
				if (DeeKwds_Check(kw)) {
					if (DeeKwds_SIZE(kw) != 0)
						goto err_no_keywords_r;
				} else {
					size_t kw_size = DeeObject_Size(kw);
					if unlikely(kw_size == (size_t)-1)
						goto err_r;
					if (kw_size != 0)
						goto err_no_keywords_r;
				}
			}
			if (object_type->tp_init.tp_alloc.tp_ctor && !argc)
				goto do_invoke_alloc_ctor_nokw;
			error = (*object_type->tp_init.tp_alloc.tp_any_ctor)(result, argc, argv);
		} else if (object_type->tp_init.tp_alloc.tp_ctor && !argc) {
do_invoke_alloc_ctor:
			if (kw) {
				if (DeeKwds_Check(kw)) {
					if (DeeKwds_SIZE(kw) != 0)
						goto err_no_keywords_r;
				} else {
					size_t kw_size = DeeObject_Size(kw);
					if unlikely(kw_size == (size_t)-1)
						goto err_r;
					if (kw_size != 0)
						goto err_no_keywords_r;
				}
			}
do_invoke_alloc_ctor_nokw:
			error = (*object_type->tp_init.tp_alloc.tp_ctor)(result);
		} else if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
		           DeeObject_InstanceOf(argv[0], object_type)) {
do_invoke_alloc_copy:
			if (kw) {
				if (DeeKwds_Check(kw)) {
					if (DeeKwds_SIZE(kw) != 0)
						goto err_no_keywords_r;
				} else {
					size_t kw_size = DeeObject_Size(kw);
					if unlikely(kw_size == (size_t)-1)
						goto err_r;
					if (kw_size != 0)
						goto err_no_keywords_r;
				}
			}
			error = (*object_type->tp_init.tp_alloc.tp_copy_ctor)(result, argv[0]);
		} else {
			DeeObject_FreeTracker(result);
			DeeType_FreeInstance(object_type, result);
			if (!DeeType_InheritConstructors(object_type))
				goto err_not_implemented;
			result = DeeType_AllocInstance(object_type);
			if unlikely(!result)
				goto err_object_type;
			DeeObject_InitNoref(result, object_type);
			if (object_type->tp_init.tp_alloc.tp_any_ctor_kw)
				goto do_invoke_alloc_any_ctor_kw;
			if (object_type->tp_init.tp_alloc.tp_any_ctor)
				goto do_invoke_alloc_any_ctor;
			if (object_type->tp_init.tp_alloc.tp_ctor && argc == 0)
				goto do_invoke_alloc_ctor;
			if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
			    DeeObject_InstanceOf(argv[0], object_type))
				goto do_invoke_alloc_copy;
			goto err_not_implemented_r;
		}
		if unlikely(error)
			goto err_r;
		/* Begin tracking the returned object. */
		if (object_type->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
		return result;
	}
err_not_implemented:
	err_unimplemented_constructor(object_type, argc, argv);
err:
	return NULL;
err_no_keywords_r:
	err_keywords_ctor_not_accepted(object_type, kw);
	goto err_r;
err_not_implemented_r:
	err_unimplemented_constructor(object_type, argc, argv);
err_r:
	DeeObject_FreeTracker(result);
	DeeType_FreeInstance(object_type, result);
err_object_type:
	Dee_DecrefNokill(object_type);
	goto err;
err_no_keywords:
	err_keywords_ctor_not_accepted(object_type, kw);
	goto err;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_NewTuple)(DeeTypeObject *object_type, DeeObject *args) {
	return DeeObject_New(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_NewTupleKw)(DeeTypeObject *object_type, DeeObject *args, DeeObject *kw) {
	return DeeObject_NewKw(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}


#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VNewPack, 12),
                    ASSEMBLY_NAME(DeeObject_New, 12));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *object_type,
                   size_t argc, va_list args) {
	return DeeObject_New(object_type, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *object_type,
                   size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_New(object_type, argc, DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !CONFIG_VA_LIST_IS_STACK_POINTER */

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VNewf(DeeTypeObject *object_type,
                char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_New(object_type,
	                       DeeTuple_SIZE(args_tuple),
	                       DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1)) DREF DeeObject *
DeeObject_NewPack(DeeTypeObject *object_type, size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VNewPack(object_type, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_Newf(DeeTypeObject *object_type,
               char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VNewf(object_type, format, args);
	va_end(args);
	return result;
}
#endif /* DEFINE_TYPED_OPERATORS */

#ifdef DEFINE_TYPED_OPERATORS
#define LOAD_TP_SELF   ASSERT_OBJECT_TYPE_A(self, tp_self)
#define LOAD_TP_SELFP  ASSERT_OBJECT_TYPE_A(*p_self, tp_self)
#define LOAD_ITER      DeeTypeObject *iter = tp_self
#define LOAD_ITERP     DeeTypeObject *iter = tp_self
#define GET_TP_SELF()  tp_self
#define GET_TP_PSELF() tp_self
#else /* DEFINE_TYPED_OPERATORS */
#define LOAD_TP_SELF   DeeTypeObject *tp_self; \
                       ASSERT_OBJECT(self);    \
                       tp_self = Dee_TYPE(self)
#define LOAD_TP_SELFP  DeeTypeObject *tp_self; \
                       ASSERT_OBJECT(*p_self); \
                       tp_self = Dee_TYPE(*p_self)
#define LOAD_ITER      DeeTypeObject *iter; \
                       ASSERT_OBJECT(self); \
                       iter = Dee_TYPE(self)
#define LOAD_ITERP     DeeTypeObject *iter;    \
                       ASSERT_OBJECT(*p_self); \
                       iter = Dee_TYPE(*p_self)
#define GET_TP_SELF()  Dee_TYPE(self)
#define GET_TP_PSELF() Dee_TYPE(*p_self)
#endif /* !DEFINE_TYPED_OPERATORS */



STATIC_ASSERT(offsetof(DeeTypeObject, tp_init.tp_alloc.tp_deep_ctor) ==
              offsetof(DeeTypeObject, tp_init.tp_var.tp_deep_ctor));

DEFINE_OPERATOR(DREF DeeObject *, DeepCopy, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *result;
	DeeThreadObject *thread_self = DeeThread_Self();
	LOAD_TP_SELF;

	/* Check to make sure that a deepcopy construction is implemented by this type.
	 * Note that the variable-and fixed-length constructors are located at the same
	 * offset in the type structure, meaning that we only need to check one address. */
	if unlikely(!tp_self->tp_init.tp_alloc.tp_deep_ctor) {
		if (!tp_self->tp_init.tp_alloc.tp_copy_ctor) {
			if (DeeType_InheritConstructors(tp_self)) {
				if (tp_self->tp_init.tp_alloc.tp_deep_ctor)
					goto got_deep_copy;
				if (tp_self->tp_init.tp_alloc.tp_copy_ctor)
					goto got_normal_copy;
			}

			/* when neither a deepcopy, nor a regular copy operator are present,
			 * assume that the object is immutable and re-return the object itself. */
			return_reference_(self);
		}
got_normal_copy:
		/* There isn't a deepcopy operator, but there is a copy operator.
		 * Now, if there also is a deepload operator, then we can invoke that one! */
		if unlikely(!tp_self->tp_init.tp_deepload) {
			err_unimplemented_operator(tp_self, OPERATOR_DEEPCOPY);
			return NULL;
		}
	}
got_deep_copy:

	/* Check if this object is already being constructed. */
	result = deepcopy_lookup(thread_self, self, tp_self);
	if (result)
		return_reference_(result);
	deepcopy_begin(thread_self);

	/* Allocate an to basic construction of the deepcopy object. */
	if (tp_self->tp_flags & TP_FVARIABLE) {
		/* Variable-length object. */
		result = tp_self->tp_init.tp_var.tp_deep_ctor
		         ? (*tp_self->tp_init.tp_var.tp_deep_ctor)(self)
		         : (*tp_self->tp_init.tp_var.tp_copy_ctor)(self);
		if unlikely(!result)
			goto done_endcopy;
	} else {
		ASSERT(!(tp_self->tp_flags & TP_FVARIABLE));

		/* Static-length object. */
		result = DeeType_AllocInstance(tp_self);
		if unlikely(!result)
			goto done_endcopy;

		/* Perform basic object initialization. */
		DeeObject_Init(result, tp_self);

		/* Invoke the deepcopy constructor first. */
		if unlikely(tp_self->tp_init.tp_alloc.tp_deep_ctor
		            ? (*tp_self->tp_init.tp_alloc.tp_deep_ctor)(result, self)
		            : (*tp_self->tp_init.tp_alloc.tp_copy_ctor)(result, self)) {
			/* Undo allocating and base-initializing the new object. */
			DeeObject_FreeTracker(result);
			DeeType_FreeInstance(tp_self, result);
			Dee_DecrefNokill(tp_self);
			result = NULL;
			goto done_endcopy;
		}

		/* Begin tracking the returned object if this is a GC type. */
		if (tp_self->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
	}

	/* Now comes the interesting part concerning
	 * recursion possible with deepcopy. */
	if (tp_self->tp_init.tp_deepload) {
		/* The type implements the deepload callback, meaning that we must
		 * always track the copies association to allow for recursion before
		 * attempting to invoke the object. */

		/* Must always track this association in case the object attempts
		 * to reference itself (which should only happen by GC objects, but
		 * then again: it is likely that only GC objects would ever implement
		 * tp_deepload to begin with, as non-GC objects could just create all
		 * the necessary deep copies straight from the regular tp_deep_ctor
		 * callback) */
		if (Dee_DeepCopyAddAssoc(result, self))
			goto err_result_endcopy;
		if unlikely((*tp_self->tp_init.tp_deepload)(result))
			goto err_result_endcopy;
done_endcopy:
		deepcopy_end(thread_self);
	} else {
		/* Optimization: In the event that this is the first-level deepcopy call,
		 *               yet the type does not implement the deepload protocol
		 *               (as could be the case for immutable sequence types like
		 *               tuple, which could still contain the same object twice),
		 *               then we don't need to track the association of this
		 *               specific deepcopy, as it would have just become undone
		 *               as soon as `deepcopy_end()' cleared the association map.
		 *               However if this deepcopy is part of a larger hierarchy of
		 *               recursive deepcopy operations, then we must still trace
		 *               the association of this new entry in case it also appears
		 *               in some different branch of the tree of remaining objects
		 *               still to-be copied. */
		if (deepcopy_end(thread_self)) {
			if (Dee_DeepCopyAddAssoc(result, self))
				goto err_result;
		}
	}
done:
	return result;
err_result_endcopy:
	deepcopy_end(thread_self);
err_result:
	Dee_Clear(result);
	goto done;
}

DEFINE_OPERATOR(DREF DeeObject *, Copy, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if (tp_self->tp_flags & TP_FVARIABLE) {
		if (tp_self->tp_init.tp_var.tp_copy_ctor) {
do_invoke_var_copy:
			return (*tp_self->tp_init.tp_var.tp_copy_ctor)(self);
		}
		if (tp_self->tp_init.tp_var.tp_deep_ctor) {
do_invoke_var_deep:
#ifdef DEFINE_TYPED_OPERATORS
			return DeeObject_TDeepCopy(tp_self, self);
#else /* DEFINE_TYPED_OPERATORS */
			return DeeObject_DeepCopy(self);
#endif /* !DEFINE_TYPED_OPERATORS */
		}
		if (tp_self->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
			return (*tp_self->tp_init.tp_var.tp_any_ctor)(1, (DeeObject **)&self);
		}
		if (tp_self->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
			return (*tp_self->tp_init.tp_var.tp_any_ctor_kw)(1, (DeeObject **)&self, NULL);
		}
		if (DeeType_InheritConstructors(tp_self)) {
			if (tp_self->tp_init.tp_var.tp_copy_ctor)
				goto do_invoke_var_copy;
			if (tp_self->tp_init.tp_var.tp_deep_ctor)
				goto do_invoke_var_deep;
			if (tp_self->tp_init.tp_var.tp_any_ctor)
				goto do_invoke_var_any_ctor;
			if (tp_self->tp_init.tp_var.tp_any_ctor_kw)
				goto do_invoke_var_any_ctor_kw;
		}
	} else if (tp_self->tp_init.tp_alloc.tp_copy_ctor) {
		int error;
do_invoke_alloc_copy:
		result = DeeType_AllocInstance(tp_self);
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, tp_self);
		error = (*tp_self->tp_init.tp_alloc.tp_copy_ctor)(result, self);
		if unlikely(error)
			goto err_r;

		/* Begin tracking the returned object. */
		if (tp_self->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
		return result;
	} else if (tp_self->tp_init.tp_alloc.tp_deep_ctor) {
		goto do_invoke_var_deep;
	} else {
		int error;
		ASSERT(!(tp_self->tp_flags & TP_FVARIABLE));
		result = DeeType_AllocInstance(tp_self);
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, tp_self);
		if (tp_self->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
			error = (*tp_self->tp_init.tp_alloc.tp_any_ctor)(result, 1, (DeeObject **)&self);
		} else if (tp_self->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
			error = (*tp_self->tp_init.tp_alloc.tp_any_ctor_kw)(result, 1, (DeeObject **)&self, NULL);
		} else {
			DeeObject_FreeTracker(result);
			DeeType_FreeInstance(tp_self, result);
			if (!DeeType_InheritConstructors(tp_self))
				goto err_not_implemented;
			if (tp_self->tp_init.tp_alloc.tp_copy_ctor) {
				Dee_DecrefNokill(tp_self);
				goto do_invoke_alloc_copy;
			}
			if (tp_self->tp_init.tp_alloc.tp_deep_ctor) {
				Dee_DecrefNokill(tp_self);
				goto do_invoke_var_deep;
			}
			result = DeeType_AllocInstance(tp_self);
			if unlikely(!result)
				goto err_object_type;
			DeeObject_InitNoref(result, tp_self);
			if (tp_self->tp_init.tp_alloc.tp_any_ctor)
				goto do_invoke_alloc_any_ctor;
			if (tp_self->tp_init.tp_alloc.tp_any_ctor_kw)
				goto do_invoke_alloc_any_ctor_kw;
			goto err_not_implemented_r;
		}
		if unlikely(error)
			goto err_r;
		/* Begin tracking the returned object. */
		if (tp_self->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
		return result;
	}
err_not_implemented:
	err_unimplemented_constructor(tp_self, 0, NULL);
err:
	return NULL;
err_not_implemented_r:
	err_unimplemented_constructor(tp_self, 0, NULL);
err_r:
	DeeObject_FreeTracker(result);
	DeeType_FreeInstance(tp_self, result);
err_object_type:
	Dee_DecrefNokill(tp_self);
	goto err;
}


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_InplaceDeepCopy)(DREF DeeObject **__restrict p_self) {
	DeeObject *objcopy, *old_object;
	old_object = *p_self;
	ASSERT_OBJECT(old_object);
	objcopy = DeeObject_DeepCopy(old_object);
	if unlikely(!objcopy)
		goto err;
	Dee_Decref(old_object);
	*p_self = objcopy;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_InplaceDeepCopyv)(/*in|out*/ DREF DeeObject **__restrict object_vector,
                                   size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		if (DeeObject_InplaceDeepCopy(&object_vector[i]))
			goto err;
	}
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceDeepCopyWithLock)(DREF DeeObject **__restrict p_self,
                                          Dee_atomic_rwlock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_rwlock_read(p_lock);
	temp = *p_self;
	Dee_Incref(temp);
	Dee_atomic_rwlock_endread(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_rwlock_write(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_rwlock_endwrite(p_lock);
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_XInplaceDeepCopyWithLock)(DREF DeeObject **__restrict p_self,
                                           Dee_atomic_rwlock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_rwlock_read(p_lock);
	temp = *p_self;
	if (!temp) {
		Dee_atomic_rwlock_endread(p_lock);
		goto done;
	}
	Dee_Incref(temp);
	Dee_atomic_rwlock_endread(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_rwlock_write(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_rwlock_endwrite(p_lock);
	Dee_XDecref(temp);
done:
	return 0;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(int, Assign, (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(some_object);
	if (tp_self->tp_init.tp_assign) {
do_assign:
		return DeeType_INVOKE_ASSIGN(tp_self, self, some_object);
	}
	if (DeeType_InheritConstructors(tp_self)) {
		if (tp_self->tp_init.tp_assign)
			goto do_assign;
	}
	return err_unimplemented_operator(GET_TP_SELF(), OPERATOR_ASSIGN);
}

DEFINE_OPERATOR(int, MoveAssign, (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(other);
	if (!(tp_self->tp_flags & TP_FMOVEANY)) {
		if (DeeObject_AssertImplements(other, tp_self))
			goto err;
	}
	if (tp_self->tp_init.tp_move_assign) {
do_move_assign:
		return DeeType_INVOKE_MOVEASSIGN(tp_self, self, other);
	}
	if (tp_self->tp_init.tp_assign) {
do_assign:
		return DeeType_INVOKE_ASSIGN(tp_self, self, other);
	}
	if (DeeType_InheritConstructors(tp_self)) {
		if (tp_self->tp_init.tp_move_assign)
			goto do_move_assign;
		if (tp_self->tp_init.tp_assign)
			goto do_assign;
	}
	return err_unimplemented_operator(GET_TP_SELF(), OPERATOR_MOVEASSIGN);
err:
	return -1;
}


#ifdef DEFINE_TYPED_OPERATORS
LOCAL WUNUSED bool DCALL
repr_contains(struct trepr_frame *chain, DeeTypeObject *tp, DeeObject *ob)
#else /* DEFINE_TYPED_OPERATORS */
LOCAL WUNUSED bool DCALL
repr_contains(struct repr_frame *chain, DeeObject *__restrict ob)
#endif /* !DEFINE_TYPED_OPERATORS */
{
	for (; chain; chain = chain->rf_prev) {
#ifdef DEFINE_TYPED_OPERATORS
		if (chain->rf_obj == ob &&
		    chain->rf_type == tp)
			return true;
#else /* DEFINE_TYPED_OPERATORS */
		if (chain->rf_obj == ob)
			return true;
#endif /* !DEFINE_TYPED_OPERATORS */
	}
	return false;
}

/* Make sure the repr-frame offsets match. */
STATIC_ASSERT(offsetof(struct trepr_frame, rf_prev) ==
              offsetof(struct repr_frame, rf_prev));
STATIC_ASSERT(offsetof(struct trepr_frame, rf_obj) ==
              offsetof(struct repr_frame, rf_obj));

#ifdef DEFINE_TYPED_OPERATORS
#define Xrepr_frame trepr_frame
#else /* DEFINE_TYPED_OPERATORS */
#define Xrepr_frame repr_frame
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultStrWithPrint, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	Dee_ssize_t print_error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	LOAD_TP_SELF;
	ASSERT(tp_self->tp_cast.tp_print &&
	       tp_self->tp_cast.tp_print != &DeeObject_DefaultPrintWithStr);
	print_error = DeeType_INVOKE_PRINT_NODEFAULT(tp_self, self, &unicode_printer_print, &printer);
	if unlikely(print_error < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultReprWithPrintRepr, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	Dee_ssize_t print_error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	LOAD_TP_SELF;
	ASSERT(tp_self->tp_cast.tp_printrepr);
	ASSERT(tp_self->tp_cast.tp_printrepr != &DeeObject_DefaultPrintReprWithRepr);
	print_error = DeeType_INVOKE_PRINTREPR_NODEFAULT(tp_self, self, &unicode_printer_print, &printer);
	if unlikely(print_error < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultPrintWithStr, (DeeObject *RESTRICT_IF_NOTYPE self,
                                                            dformatprinter printer, void *arg)) {
	Dee_ssize_t result;
	DREF DeeObject *str;
	LOAD_TP_SELF;
	ASSERT(tp_self->tp_cast.tp_str);
	ASSERT(tp_self->tp_cast.tp_str != &DeeObject_DefaultStrWithPrint);
	str = DeeType_INVOKE_STR_NODEFAULT(tp_self, self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultPrintReprWithRepr, (DeeObject *RESTRICT_IF_NOTYPE self,
                                                                 dformatprinter printer, void *arg)) {
	Dee_ssize_t result;
	DREF DeeObject *str;
	LOAD_TP_SELF;
	ASSERT(tp_self->tp_cast.tp_repr);
	ASSERT(tp_self->tp_cast.tp_repr != &DeeObject_DefaultReprWithPrintRepr);
	str = DeeType_INVOKE_REPR_NODEFAULT(tp_self, self);
	if unlikely(!str)
		goto err;
	result = DeeString_PrintUtf8(str, printer, arg);
	Dee_Decref_likely(str);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
DeeType_InheritStr(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	DeeTypeMRO mro;
	if (self->tp_cast.tp_print) {
		/* Substitute str with print */
		self->tp_cast.tp_str = &DeeObject_DefaultStrWithPrint;
		return true;
	} else if (self->tp_cast.tp_str) {
		/* Substitute print with str */
		self->tp_cast.tp_print = &DeeObject_DefaultPrintWithStr;
		return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_cast.tp_str || !base->tp_cast.tp_print) {
			if (!DeeType_InheritStr(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator str");
		self->tp_cast.tp_str   = base->tp_cast.tp_str;
		self->tp_cast.tp_print = base->tp_cast.tp_print;
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritRepr(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	DeeTypeMRO mro;
	if (self->tp_cast.tp_printrepr) {
		/* Substitute repr with printrepr */
		self->tp_cast.tp_repr = &DeeObject_DefaultReprWithPrintRepr;
		return true;
	} else if (self->tp_cast.tp_repr) {
		/* Substitute printrepr with repr */
		self->tp_cast.tp_printrepr = &DeeObject_DefaultPrintReprWithRepr;
		return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_cast.tp_repr || !base->tp_cast.tp_printrepr) {
			if (!DeeType_InheritRepr(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator repr");
		self->tp_cast.tp_repr      = base->tp_cast.tp_repr;
		self->tp_cast.tp_printrepr = base->tp_cast.tp_printrepr;
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritCall(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	/* Substitute tp_call <===> tp_call_kw */
	if (self->tp_call) {
		if (self->tp_call_kw == NULL)
			self->tp_call_kw = &DeeObject_DefaultCallKwWithCall;
		return true;
	} else if (self->tp_call_kw) {
		if (self->tp_call == NULL)
			self->tp_call = &DeeObject_DefaultCallWithCallKw;
		return true;
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_call && !base->tp_call_kw) {
			if (!DeeType_InheritCall(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator call");
		self->tp_call    = base->tp_call;
		self->tp_call_kw = base->tp_call_kw;
		return true;
	}
	return false;
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(DREF DeeObject *, Str, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_str && !DeeType_InheritStr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if unlikely(tp_self->tp_flags & TP_FGC) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __str__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_str_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj          = self;
		opframe.rf_type         = tp_self;
		this_thread->t_str_curr = (struct repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_str_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_STR(tp_self, self);
		this_thread->t_str_curr = (struct repr_frame *)opframe.rf_prev;
		ASSERT_OBJECT_TYPE_EXACT_OPT(result, &DeeString_Type);
		return result;
	}

	/* Non-gc object (much simpler) */
	return DeeType_INVOKE_STR(tp_self, self);
missing:
	err_unimplemented_operator(tp_self, OPERATOR_STR);
	return NULL;
recursion:
	return_reference_((DeeObject *)&str_dots);
}

DEFINE_OPERATOR(DREF DeeObject *, Repr, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_repr && !DeeType_InheritRepr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if (tp_self->tp_flags & TP_FGC) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __repr__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_repr_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj           = self;
		opframe.rf_type          = tp_self;
		this_thread->t_repr_curr = (struct repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_repr_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_REPR(tp_self, self);
		this_thread->t_repr_curr = (struct repr_frame *)opframe.rf_prev;
		ASSERT_OBJECT_TYPE_EXACT_OPT(result, &DeeString_Type);
		return result;
	}

	/* Non-gc object (much simpler) */
	return DeeType_INVOKE_REPR(tp_self, self);
missing:
	err_unimplemented_operator(tp_self, OPERATOR_REPR);
	return NULL;
recursion:
	return_reference_((DeeObject *)&str_dots);
}

DEFINE_OPERATOR(Dee_ssize_t, Print, (DeeObject *RESTRICT_IF_NOTYPE self,
                                  dformatprinter printer, void *arg)) {
	Dee_ssize_t result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_print && !DeeType_InheritStr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if unlikely(tp_self->tp_flags & TP_FGC) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __str__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_str_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj          = self;
		opframe.rf_type         = tp_self;
		this_thread->t_str_curr = (struct repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_str_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_PRINT(tp_self, self, printer, arg);
		this_thread->t_str_curr = (struct repr_frame *)opframe.rf_prev;
		return result;
	}

	/* Non-gc object (much simpler) */
	return DeeType_INVOKE_PRINT(tp_self, self, printer, arg);
missing:
	return err_unimplemented_operator(tp_self, OPERATOR_STR);
recursion:
	return DeeString_PrintAscii(&str_dots, printer, arg);
}

DEFINE_OPERATOR(Dee_ssize_t, PrintRepr, (DeeObject *RESTRICT_IF_NOTYPE self,
                                      dformatprinter printer, void *arg)) {
	Dee_ssize_t result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_printrepr && !DeeType_InheritRepr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if (tp_self->tp_flags & TP_FGC) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __repr__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_repr_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj           = self;
		opframe.rf_type          = tp_self;
		this_thread->t_repr_curr = (struct repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_repr_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_PRINTREPR(tp_self, self, printer, arg);
		this_thread->t_repr_curr = (struct repr_frame *)opframe.rf_prev;
		return result;
	}

	/* Non-gc object (much simpler) */
	return DeeType_INVOKE_PRINTREPR(tp_self, self, printer, arg);
missing:
	return err_unimplemented_operator(tp_self, OPERATOR_REPR);
recursion:
	return DeeString_PrintAscii(&str_dots, printer, arg);
}

#undef Xrepr_frame

DEFINE_OPERATOR(int, Bool, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	/* _very_ likely case: `self' is one of the boolean constants
	 *  -> In this case, we return the result immediately! */
#ifndef __OPTIMIZE_SIZE__
	if (self == Dee_True)
		return 1;
	if (self == Dee_False)
		return 0;
#endif /* !__OPTIMIZE_SIZE__ */

	/* General case: invoke the "bool" operator. */
	{
		LOAD_TP_SELF;
		if likely(tp_self->tp_cast.tp_bool || DeeType_InheritBool(tp_self))
			return DeeType_INVOKE_BOOL(tp_self, self);
		return err_unimplemented_operator(tp_self, OPERATOR_BOOL);
	}
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(int, BoolInherited, (/*inherit(always)*/ DREF DeeObject *RESTRICT_IF_NOTYPE self)) {
	/* _very_ likely case: `self' is one of the boolean constants
	 *  -> In this case, we return the result immediately! */
#ifndef __OPTIMIZE_SIZE__
	if (self == Dee_True) {
		Dee_DecrefNokill(Dee_True);
		return 1;
	}
	if (self == Dee_False) {
		Dee_DecrefNokill(Dee_False);
		return 0;
	}
#endif /* !__OPTIMIZE_SIZE__ */

	/* General case: invoke the "bool" operator. */
	{
		int result;
		LOAD_TP_SELF;
		if likely(tp_self->tp_cast.tp_bool || DeeType_InheritBool(tp_self)) {
			result = DeeType_INVOKE_BOOL(tp_self, self);
		} else {
			result = err_unimplemented_operator(tp_self, OPERATOR_BOOL);
		}
		Dee_Decref(self);
		return result;
	}
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, Call,
                (DeeObject *self, size_t argc, DeeObject *const *argv)) {
	LOAD_TP_SELF;
	if (tp_self->tp_call || DeeType_InheritCall(tp_self))
		return DeeType_INVOKE_CALL(tp_self, self, argc, argv);
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, CallKw,
                (DeeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw)) {
	LOAD_TP_SELF;
	ASSERT(!kw || DeeObject_IsKw(kw));
	if likely(tp_self->tp_call_kw || DeeType_InheritCall(tp_self))
		return DeeType_INVOKE_CALLKW(tp_self, self, argc, argv, kw);
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(DREF DeeObject *, CallTuple,
               (DeeObject *self, DeeObject *args)) {
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	LOAD_TP_SELF;
	ASSERT_OBJECT_TYPE_EXACT(args, &DeeTuple_Type);
	if (tp_self == &DeeFunction_Type)
		return DeeFunction_CallTuple((DeeFunctionObject *)self, args);
	if likely(tp_self->tp_call || DeeType_InheritCall(tp_self)) {
		return DeeType_INVOKE_CALL(tp_self, self,
		                           DeeTuple_SIZE(args),
		                           DeeTuple_ELEM(args));
	}
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
	return NULL;
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	return DeeObject_Call(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
}

DEFINE_OPERATOR(DREF DeeObject *, CallTupleKw,
               (DeeObject *self, DeeObject *args, DeeObject *kw)) {
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	LOAD_TP_SELF;
	ASSERT_OBJECT_TYPE_EXACT(args, &DeeTuple_Type);
	ASSERT(!kw || DeeObject_IsKw(kw));
	if (tp_self == &DeeFunction_Type)
		return DeeFunction_CallTupleKw((DeeFunctionObject *)self, args, kw);
	if likely(tp_self->tp_call_kw || DeeType_InheritCall(tp_self)) {
		return DeeType_INVOKE_CALLKW(tp_self, self,
		                             DeeTuple_SIZE(args),
		                             DeeTuple_ELEM(args),
		                             kw);
	}
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
	return NULL;
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	return DeeObject_CallKw(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, ThisCall,
                (DeeObject *self, DeeObject *this_arg,
                 size_t argc, DeeObject *const *argv)) {
	DREF DeeTupleObject *full_args;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
#ifndef DEFINE_TYPED_OPERATORS
	if (DeeSuper_Check(self)) {
		return DeeObject_TThisCall(DeeSuper_TYPE(self),
		                           DeeSuper_SELF(self),
		                           this_arg, argc, argv);
	}
#endif /* !DEFINE_TYPED_OPERATORS */

	/* Check for special callback optimizations. */
	if (GET_TP_SELF() == &DeeFunction_Type)
		return DeeFunction_ThisCall((DeeFunctionObject *)self, this_arg, argc, argv);
	if (GET_TP_SELF() == &DeeClsMethod_Type) {
		DeeClsMethodObject *me = (DeeClsMethodObject *)self;
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertTypeOrAbstract(this_arg, me->cm_type))
			goto err;
		return (*me->cm_func)(this_arg, argc, argv);
	}
	if (GET_TP_SELF() == &DeeKwClsMethod_Type) {
		DeeKwClsMethodObject *me = (DeeKwClsMethodObject *)self;
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertTypeOrAbstract(this_arg, me->cm_type))
			goto err;
		return (*me->cm_func)(this_arg, argc, argv, NULL);
	}

	/* sigh... Looks like we need to create a temporary argument tuple... */
	full_args = DeeTuple_NewUninitialized(1 + argc);
	if unlikely(!full_args)
		goto err;

	/* Lazily alias arguments in the `full_args' tuple. */
	DeeTuple_SET(full_args, 0, this_arg);
	memcpyc(&DeeTuple_ELEM(full_args)[1],
	        argv, argc, sizeof(DeeObject *));
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TCall(tp_self, self,
	                         DeeTuple_SIZE(full_args),
	                         DeeTuple_ELEM(full_args));
#else /* DEFINE_TYPED_OPERATORS */
	/* Using `DeeObject_CallTuple()' here would be counterproductive:
	 * - Said function only has optimization for types which we've
	 *   already checked for, so there's no point in trying to check
	 *   for them a second time! */
	result = DeeObject_Call(self,
	                        DeeTuple_SIZE(full_args),
	                        DeeTuple_ELEM(full_args));
#endif /* !DEFINE_TYPED_OPERATORS */
	DeeTuple_DecrefSymbolic((DREF DeeObject *)full_args);
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, ThisCallKw,
                (DeeObject *self, DeeObject *this_arg,
                 size_t argc, DeeObject *const *argv,
                 DeeObject *kw)) {
	DREF DeeTupleObject *full_args;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
	ASSERT(!kw || DeeObject_IsKw(kw));
#ifndef DEFINE_TYPED_OPERATORS
	if (DeeSuper_Check(self)) {
		return DeeObject_TThisCallKw(DeeSuper_TYPE(self),
		                             DeeSuper_SELF(self),
		                             this_arg, argc, argv, kw);
	}
#endif /* !DEFINE_TYPED_OPERATORS */

	/* Check for special callback optimizations. */
	if (GET_TP_SELF() == &DeeFunction_Type)
		return DeeFunction_ThisCallKw((DeeFunctionObject *)self, this_arg, argc, argv, kw);
	if (GET_TP_SELF() == &DeeKwClsMethod_Type) {
		DeeKwClsMethodObject *me = (DeeKwClsMethodObject *)self;
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertTypeOrAbstract(this_arg, me->cm_type))
			goto err;
		return (*me->cm_func)(this_arg, argc, argv, kw);
	}
	if (GET_TP_SELF() == &DeeClsMethod_Type) {
		DeeClsMethodObject *me = (DeeClsMethodObject *)self;
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertTypeOrAbstract(this_arg, me->cm_type))
			goto err;
		if (kw) {
			if (DeeKwds_Check(kw)) {
				if (DeeKwds_SIZE(kw) != 0)
					goto err_no_keywords;
			} else {
				size_t kw_length;
				kw_length = DeeObject_Size(kw);
				if unlikely(kw_length == (size_t)-1)
					goto err;
				if (kw_length != 0)
					goto err_no_keywords;
			}
		}
		return (*me->cm_func)(this_arg, argc, argv);
	}

	/* sigh... Looks like we need to create a temporary argument tuple... */
	full_args = DeeTuple_NewUninitialized(1 + argc);
	if unlikely(!full_args)
		goto err;

	/* Lazily alias arguments in the `full_args' tuple. */
	DeeTuple_SET(full_args, 0, this_arg);
	memcpyc(&DeeTuple_ELEM(full_args)[1],
	        argv, argc, sizeof(DeeObject *));
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TCallKw(tp_self, self,
	                           DeeTuple_SIZE(full_args),
	                           DeeTuple_ELEM(full_args),
	                           kw);
#else /* DEFINE_TYPED_OPERATORS */
	/* Using `DeeObject_CallTupleKw()' here would be counterproductive:
	 * - Said function only has optimization for types which we've
	 *   already checked for, so there's no point in trying to check
	 *   for them a second time! */
	result = DeeObject_CallKw(self,
	                          DeeTuple_SIZE(full_args),
	                          DeeTuple_ELEM(full_args),
	                          kw);
#endif /* !DEFINE_TYPED_OPERATORS */
	DeeTuple_DecrefSymbolic((DeeObject *)full_args);
	return result;
err_no_keywords:
	err_keywords_not_accepted(GET_TP_SELF(), kw);
err:
	return NULL;
}


#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(DREF DeeObject *, ThisCallTuple,
                (DeeObject *self, DeeObject *this_arg, DeeObject *args)) {
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* Check for special callback optimizations. */
	if (GET_TP_SELF() == &DeeFunction_Type)
		return DeeFunction_ThisCallTuple((DeeFunctionObject *)self, this_arg, args);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	return DeeObject_ThisCall(self,
	                          this_arg,
	                          DeeTuple_SIZE(args),
	                          DeeTuple_ELEM(args));
}

DEFINE_OPERATOR(DREF DeeObject *, ThisCallTupleKw,
                (DeeObject *self, DeeObject *this_arg, DeeObject *args, DeeObject *kw)) {
	ASSERT(!kw || DeeObject_IsKw(kw));
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* Check for special callback optimizations. */
	if (GET_TP_SELF() == &DeeFunction_Type)
		return DeeFunction_ThisCallTupleKw((DeeFunctionObject *)self, this_arg, args, kw);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	return DeeObject_ThisCallKw(self,
	                            this_arg,
	                            DeeTuple_SIZE(args),
	                            DeeTuple_ELEM(args),
	                            kw);
}
#endif /* !DEFINE_TYPED_OPERATORS */


#ifndef DEFINE_TYPED_OPERATORS
#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallPack, 12),
                    ASSEMBLY_NAME(DeeObject_Call, 12));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VCallPack(DeeObject *self, size_t argc, va_list args) {
	return DeeObject_Call(self, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VCallPack(DeeObject *self, size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_Call(self, argc, DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !CONFIG_VA_LIST_IS_STACK_POINTER */

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VCallf(DeeObject *self, char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_Call(self,
	                        DeeTuple_SIZE(args_tuple),
	                        DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObject_VThisCallf(DeeObject *self, DeeObject *this_arg,
                     char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_ThisCall(self, this_arg,
	                            DeeTuple_SIZE(args_tuple),
	                            DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1)) DREF DeeObject *
DeeObject_CallPack(DeeObject *self, size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VCallPack(self, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_Callf(DeeObject *self,
                char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VCallf(self, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
DeeObject_ThisCallf(DeeObject *self, DeeObject *this_arg,
                    char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VThisCallf(self, this_arg, format, args);
	va_end(args);
	return result;
}
#endif /* !DEFINE_TYPED_OPERATORS */

#ifdef DEFINE_TYPED_OPERATORS
#define Xrepr_frame trepr_frame
#else /* DEFINE_TYPED_OPERATORS */
#define Xrepr_frame repr_frame
#endif /* !DEFINE_TYPED_OPERATORS */

WUNUSED /*ATTR_PURE*/
DEFINE_OPERATOR(dhash_t, Hash, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_cmp && tp_self->tp_cmp->tp_hash) ||
	          (DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_hash)) {
		if likely(!(tp_self->tp_flags & TP_FGC)) {
			return DeeType_INVOKE_HASH(tp_self, self);
		} else {
			/* Handle hash recursion for GC objects. */
			dhash_t result;
			struct Xrepr_frame opframe;
			DeeThreadObject *this_thread = DeeThread_Self();

			/* Trace objects for which __hash__ is being invoked. */
			opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_hash_curr;
#ifdef DEFINE_TYPED_OPERATORS
			if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
				goto recursion;
			opframe.rf_obj           = self;
			opframe.rf_type          = tp_self;
			this_thread->t_hash_curr = (struct repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
			if unlikely(repr_contains(opframe.rf_prev, self))
				goto recursion;
			opframe.rf_obj = self;
			this_thread->t_hash_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
			result = DeeType_INVOKE_HASH(tp_self, self);
			this_thread->t_hash_curr = (struct repr_frame *)opframe.rf_prev;
			return result;
		}
	}
	return DeeObject_HashGeneric(self);
recursion:
	return DEE_HASHOF_RECURSIVE_ITEM;
}

#undef Xrepr_frame

#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED /*ATTR_PURE*/ ATTR_INS(1, 2) dhash_t
(DCALL DeeObject_Hashv)(DeeObject *const *__restrict object_vector,
                        size_t object_count) {
	size_t i;
	dhash_t result;
	/* Check for special case: no objects, i.e.: an empty sequence */
	if unlikely(!object_count)
		return DEE_HASHOF_EMPTY_SEQUENCE;

	/* Important: when only a single object is given, our
	 * return value must be equal to `DeeObject_Hash()'.
	 *
	 * This is required so that:
	 * >> import hash from deemon;
	 * >> assert hash(42) == (42).operator hash();
	 * >> assert hash(42) == (42); */
	result = DeeObject_Hash(object_vector[0]);
	for (i = 1; i < object_count; ++i) {
		dhash_t item;
		item   = DeeObject_Hash(object_vector[i]);
		result = Dee_HashCombine(result, item);
	}
	return result;
}
#endif /* DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(void, Visit, (DeeObject *__restrict self, dvisit_t proc, void *arg)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_visit) {
			if (tp_self->tp_visit == &instance_visit) {
				/* Required to prevent redundancy in class instances.
				 * Without this, all instance levels would be visited more
				 * than once by the number of recursive user-types, when
				 * one visit (as implemented here) is already enough. */
				instance_tvisit(tp_self, self, proc, arg);
			} else {
				(*tp_self->tp_visit)(self, proc, arg);
			}
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);

	/* Only visit heap-allocated types. */
	if (Dee_TYPE(self)->tp_flags & TP_FHEAP)
		(*proc)((DeeObject *)Dee_TYPE(self), arg);
}

DEFINE_OPERATOR(void, Clear, (DeeObject *__restrict self)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_gc && tp_self->tp_gc->tp_clear) {
			if (tp_self->tp_gc->tp_clear == &instance_clear) {
				/* Same deal as with visit above: Reduce
				 * overhead from recursive redundancies. */
				instance_tclear(tp_self, self);
			} else {
				(*tp_self->tp_gc->tp_clear)(self);
			}
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
}

DEFINE_OPERATOR(void, PClear, (DeeObject *__restrict self, unsigned int gc_priority)) {
	LOAD_TP_SELF;
	if unlikely(gc_priority == Dee_GC_PRIORITY_LATE) {
#ifdef DEFINE_TYPED_OPERATORS
		DeeObject_TClear(tp_self, self);
#else /* DEFINE_TYPED_OPERATORS */
		DeeObject_Clear(self);
#endif /* !DEFINE_TYPED_OPERATORS */
		return;
	}
	do {
		if (tp_self->tp_gc &&
		    tp_self->tp_gc->tp_pclear) {
			if (tp_self->tp_gc->tp_pclear == &instance_pclear) {
				/* Same deal as with visit above: Reduce
				 * overhead from recursive redundancies. */
				instance_tpclear(tp_self, self, gc_priority);
			} else {
				(*tp_self->tp_gc->tp_pclear)(self, gc_priority);
			}
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
}


DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultCallWithCallKw,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t argc, DeeObject *const *argv)) {
	LOAD_TP_SELF;
	ASSERT(tp_self->tp_call_kw);
	ASSERT(tp_self->tp_call_kw != &DeeObject_DefaultCallKwWithCall);
	return DeeType_INVOKE_CALLKW_NODEFAULT(tp_self, self, argc, argv, NULL);
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultCallKwWithCall,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t argc, DeeObject *const *argv, DeeObject *kw)) {
	LOAD_TP_SELF;
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_length;
			kw_length = DeeObject_Size(kw);
			if unlikely(kw_length == (size_t)-1)
				goto err;
			if (kw_length != 0)
				goto err_no_keywords;
		}
	}
	ASSERT(tp_self->tp_call);
	ASSERT(tp_self->tp_call != &DeeObject_DefaultCallWithCallKw);
	return DeeType_INVOKE_CALL_NODEFAULT(tp_self, self, argc, argv);
err_no_keywords:
	err_keywords_not_accepted(tp_self, kw);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInt32WithInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, int32_t *__restrict result)) {
	int status;
	DREF DeeObject *temp;
	LOAD_TP_SELF;
	temp = DeeType_INVOKE_INT_NODEFAULT(tp_self, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get32Bit(temp, result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInt64WithInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, int64_t *__restrict result)) {
	int status;
	DREF DeeObject *temp;
	LOAD_TP_SELF;
	temp = DeeType_INVOKE_INT_NODEFAULT(tp_self, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get64Bit(temp, result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDoubleWithInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, double *__restrict result)) {
	int status;
	DREF DeeObject *temp;
	LOAD_TP_SELF;
	temp = DeeType_INVOKE_INT_NODEFAULT(tp_self, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_AsDouble(temp, result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIntWithInt32, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	int32_t value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_INT32_NODEFAULT(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt32((uint32_t)value);
	return DeeInt_NewInt32(value);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInt64WithInt32,
                         (DeeObject *RESTRICT_IF_NOTYPE self, int64_t *__restrict result)) {
	int32_t value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_INT32_NODEFAULT(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*result = (int64_t)(uint64_t)(uint32_t)value;
	} else {
		*result = (int64_t)value;
	}
	return status;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDoubleWithInt32,
                         (DeeObject *RESTRICT_IF_NOTYPE self, double *__restrict result)) {
	int32_t value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_INT32_NODEFAULT(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*result = (double)(uint32_t)value;
	} else {
		*result = (double)value;
	}
	return status;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIntWithInt64, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	int64_t value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_INT64_NODEFAULT(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt64((uint64_t)value);
	return DeeInt_NewInt64(value);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInt32WithInt64,
                         (DeeObject *RESTRICT_IF_NOTYPE self, int32_t *__restrict result)) {
	int64_t value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_INT64_NODEFAULT(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		if ((uint64_t)value > UINT32_MAX && !(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*result = (int32_t)(uint32_t)(uint64_t)value;
	} else {
		if ((value < INT32_MIN || value > INT32_MAX) && status == Dee_INT_SIGNED &&
		    !(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*result = (int32_t)value;
	}
	return status;
err_overflow:
	return err_integer_overflow(self, 32, status == Dee_INT_SIGNED);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDoubleWithInt64,
                         (DeeObject *RESTRICT_IF_NOTYPE self, double *__restrict result)) {
	int64_t value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_INT64_NODEFAULT(tp_self, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*result = (double)(uint64_t)value;
	} else {
		*result = (double)value;
	}
	return status;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIntWithDouble, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	double value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_DOUBLE_NODEFAULT(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	return DeeInt_NewDouble(value);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInt32WithDouble,
                         (DeeObject *RESTRICT_IF_NOTYPE self, int32_t *__restrict result)) {
	double value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_DOUBLE_NODEFAULT(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT32_MIN && !(tp_self->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT32_MAX) {
		if (value <= UINT32_MAX) {
			*result = (int32_t)(uint32_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*result = (int32_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 32, value >= 0);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInt64WithDouble,
                         (DeeObject *RESTRICT_IF_NOTYPE self, int64_t *__restrict result)) {
	double value;
	int status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_DOUBLE_NODEFAULT(tp_self, self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT64_MIN && !(tp_self->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT64_MAX) {
		if (value <= UINT64_MAX) {
			*result = (int64_t)(uint64_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(tp_self->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*result = (int64_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 64, value >= 0);
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
DeeType_InheritInt(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	struct type_math *math = self->tp_math;
	/* Assign proxy operators instead of letting the caller do the select. */
	if (math) {
		if (math->tp_int) {
			if (math->tp_int32 == NULL)
				math->tp_int32 = &DeeObject_DefaultInt32WithInt;
			if (math->tp_int64 == NULL)
				math->tp_int64 = &DeeObject_DefaultInt64WithInt;
			if (math->tp_double == NULL)
				math->tp_double = &DeeObject_DefaultDoubleWithInt;
			return true;
		} else if (math->tp_double) {
			if (math->tp_int32 == NULL)
				math->tp_int32 = &DeeObject_DefaultInt32WithDouble;
			if (math->tp_int64 == NULL)
				math->tp_int64 = &DeeObject_DefaultInt64WithDouble;
			if (math->tp_int == NULL)
				math->tp_int = &DeeObject_DefaultIntWithDouble;
			return true;
		} else if (math->tp_int64) {
			if (math->tp_int32 == NULL)
				math->tp_int32 = &DeeObject_DefaultInt32WithInt64;
			if (math->tp_double == NULL)
				math->tp_double = &DeeObject_DefaultDoubleWithInt64;
			if (math->tp_int == NULL)
				math->tp_int = &DeeObject_DefaultIntWithInt64;
			return true;
		} else if (math->tp_int32) {
			if (math->tp_int64 == NULL)
				math->tp_int64 = &DeeObject_DefaultInt64WithInt32;
			if (math->tp_double == NULL)
				math->tp_double = &DeeObject_DefaultDoubleWithInt32;
			if (math->tp_int == NULL)
				math->tp_int = &DeeObject_DefaultIntWithInt32;
			return true;
		}
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		struct type_math *base_math = base->tp_math;
		if (base_math == NULL ||
		    (!base_math->tp_int && !base_math->tp_int32 &&
		     !base_math->tp_int64 && !base_math->tp_double)) {
			if (!DeeType_InheritInt(base))
				continue;
			base_math = base->tp_math;
		}
		if (self->tp_math != NULL) {
			DeeTypeObject *origin = DeeType_GetMathOrigin(self);
			if unlikely(origin)
				return DeeType_InheritInt(origin);
			self->tp_math->tp_int32  = base_math->tp_int32;
			self->tp_math->tp_int64  = base_math->tp_int64;
			self->tp_math->tp_int    = base_math->tp_int;
			self->tp_math->tp_double = base_math->tp_double;
		} else {
			self->tp_math = base_math;
		}
		LOG_INHERIT(base, self, "operator int");
		return true;
	}
	return false;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(int, Get32Bit,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 int32_t *__restrict result)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int32) ||
	          DeeType_InheritInt(tp_self))
		return DeeType_INVOKE_INT32(tp_self, self, result);
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
}

DEFINE_OPERATOR(int, Get64Bit,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 int64_t *__restrict result)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int64) ||
	          DeeType_InheritInt(tp_self))
		return DeeType_INVOKE_INT64(tp_self, self, result);
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
}

DEFINE_OPERATOR(int, AsDouble,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 double *__restrict result)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_double) ||
	          DeeType_InheritInt(tp_self))
		return DeeType_INVOKE_DOUBLE(tp_self, self, result);
	return err_unimplemented_operator(tp_self, OPERATOR_FLOAT);
}

DEFINE_OPERATOR(DREF DeeObject *, Int, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int) ||
	          DeeType_InheritInt(tp_self))
		return DeeType_INVOKE_INT(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_INT);
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(int, Get128Bit,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 Dee_int128_t *__restrict result)) {
	LOAD_TP_SELF;
	if (tp_self == &DeeInt_Type)
		return DeeInt_Get128Bit(self, result);
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int) ||
	          DeeType_InheritInt(tp_self)) {
		int error;
		/* Cast to integer, then read its value. */
		DREF DeeObject *intob;
		intob = DeeType_INVOKE_INT(tp_self, self);
		if unlikely(!intob)
			goto err;
		error = DeeInt_Get128Bit(intob, result);
		Dee_Decref(intob);
		return error;
	}
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}
#endif


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt32)(DeeObject *__restrict self,
                           uint32_t *__restrict result) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(self);
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int32) ||
	          DeeType_InheritInt(tp_self)) {
		int error = DeeType_INVOKE_INT32(tp_self, self, (int32_t *)result);
		if unlikely(error < 0)
			goto err;
		if unlikely(error == INT_SIGNED && *(int32_t *)result < 0) {
			if (tp_self->tp_flags & TP_FTRUNCATE)
				return 0;
			return err_integer_overflow(self, 32, 0);
		}
		return 0;
	}
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt32)(DeeObject *__restrict self,
                          int32_t *__restrict result) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(self);
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int32) ||
	          DeeType_InheritInt(tp_self)) {
		int error = DeeType_INVOKE_INT32(tp_self, self, result);
		if unlikely(error < 0)
			goto err;
		if unlikely(error == INT_UNSIGNED && (uint32_t)*result > INT32_MAX) {
			if (tp_self->tp_flags & TP_FTRUNCATE)
				return 0;
			return err_integer_overflow(self, 32, 1);
		}
		return 0;
	}
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt64)(DeeObject *__restrict self,
                           uint64_t *__restrict result) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(self);
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int64) ||
	          DeeType_InheritInt(tp_self)) {
		int error = DeeType_INVOKE_INT64(tp_self, self, (int64_t *)result);
		if unlikely(error < 0)
			goto err;
		if unlikely(error == INT_SIGNED && *(int64_t *)result < 0) {
			if (tp_self->tp_flags & TP_FTRUNCATE)
				return 0;
			return err_integer_overflow(self, 64, false);
		}
		return 0;
	}
	err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt64)(DeeObject *__restrict self,
                          int64_t *__restrict result) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(self);
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int64) ||
	          DeeType_InheritInt(tp_self)) {
		int error = DeeType_INVOKE_INT64(tp_self, self, result);
		if unlikely(error < 0)
			goto err;
		if unlikely(error == INT_UNSIGNED && (uint64_t)*result > INT64_MAX) {
			if (tp_self->tp_flags & TP_FTRUNCATE)
				return 0;
			return err_integer_overflow(self, 64, true);
		}
		return 0;
	}
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt128)(DeeObject *__restrict self,
                           Dee_int128_t *__restrict result) {
	int error = DeeObject_Get128Bit(self, result);
	if (error == INT_UNSIGNED) {
		if (__hybrid_int128_isneg(*result))
			return err_integer_overflow(self, 128, true);
#if INT_UNSIGNED != 0
		return 0;
#endif /* INT_UNSIGNED != 0 */
	} else {
#if INT_UNSIGNED != 0
		ASSERT(error <= 0);
#else /* INT_UNSIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_UNSIGNED == 0 */
	}
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt128)(DeeObject *__restrict self,
                            Dee_uint128_t *__restrict result) {
	int error = DeeObject_Get128Bit(self, (Dee_int128_t *)result);
	if (error == INT_SIGNED) {
		if (__hybrid_int128_isneg(*(Dee_int128_t const *)result))
			return err_integer_overflow(self, 128, false);
#if INT_SIGNED != 0
		return 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	return error;
}
#endif /* !DEFINE_TYPED_OPERATORS */

#ifndef DEFINE_TYPED_OPERATORS
INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
type_get_int_caster(DeeTypeObject *__restrict start) {
	DeeTypeMRO mro;
	start = DeeTypeMRO_Init(&mro, start);
	for (;;) {
		if (DeeType_HasPrivateOperator(start, OPERATOR_INT))
			break;
		start = DeeTypeMRO_NextDirectBase(&mro, start);
		ASSERT(start);
	}
	return start;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeObject_Get8Bit(DeeObject *__restrict self,
                  int8_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (error == INT_SIGNED) {
		if (val32 < INT8_MIN || val32 > INT8_MAX) {
			if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE) {
				*result = (int8_t)val32;
				return *result < 0 ? INT_SIGNED : INT_UNSIGNED;
			}
			return err_integer_overflow(self, 8, val32 > 0);
		}
		*result = (int8_t)val32;
	} else {
		if ((uint32_t)val32 > UINT8_MAX) {
			if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE) {
				*result = (uint8_t)val32;
				return INT_UNSIGNED;
			}
			return err_integer_overflow(self, 8, true);
		}
		*result = (int8_t)(uint8_t)val32;
	}
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeObject_Get16Bit(DeeObject *__restrict self,
                   int16_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (error == INT_SIGNED) {
		if (val32 < INT16_MIN || val32 > INT16_MAX) {
			if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE) {
				*result = (int16_t)val32;
				return *result < 0 ? INT_SIGNED : INT_UNSIGNED;
			}
			return err_integer_overflow(self, 16, val32 > 0);
		}
		*result = (int16_t)val32;
	} else {
		if ((uint32_t)val32 > UINT16_MAX) {
			if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE) {
				*result = (uint16_t)val32;
				return INT_UNSIGNED;
			}
			return err_integer_overflow(self, 16, true);
		}
		*result = (int16_t)(uint16_t)val32;
	}
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt8)(DeeObject *__restrict self,
                         int8_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_AsInt32(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (val32 < INT8_MIN || val32 > INT8_MAX) {
		if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 8, val32 > 0);
	}
return_value:
	*result = (int8_t)val32;
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt16)(DeeObject *__restrict self,
                          int16_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_AsInt32(self, &val32);
	if unlikely(error < 0)
		goto err;
	if (val32 < INT16_MIN || val32 > INT16_MAX) {
		if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 16, val32 > 0);
	}
return_value:
	*result = (int16_t)val32;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt8)(DeeObject *__restrict self,
                          uint8_t *__restrict result) {
	uint32_t val32;
	int error = DeeObject_AsUInt32(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (val32 > UINT8_MAX) {
		if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 8, true);
	}
return_value:
	*result = (uint8_t)val32;
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt16)(DeeObject *__restrict self,
                           uint16_t *__restrict result) {
	uint32_t val32;
	int error = DeeObject_AsUInt32(self, &val32);
	if unlikely(error < 0)
		goto err;
	if (val32 > UINT16_MAX) {
		if (type_get_int_caster(Dee_TYPE(self))->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 16, true);
	}
return_value:
	*result = (uint16_t)val32;
	return 0;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */



#ifdef DEFINE_TYPED_OPERATORS
#define COPY_SELF() DeeObject_TCopy(tp_self, self)
#else /* DEFINE_TYPED_OPERATORS */
#define COPY_SELF() DeeObject_Copy(self)
#endif /* !DEFINE_TYPED_OPERATORS */

/* Default wrappers for implementing math operators using copy + their inplace variants. */
#define DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Name, NAME)                 \
	DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, Default##Name##WithInplace##Name, \
	                         (DeeObject *self, DeeObject *other)) {              \
		DREF DeeObject *copy;                                                    \
		LOAD_TP_SELF;                                                            \
		copy = COPY_SELF();                                                      \
		if unlikely(!copy)                                                       \
			goto err;                                                            \
		ASSERT_OBJECT_TYPE(copy, tp_self);                                       \
		if unlikely(DeeType_INVOKE_I##NAME(tp_self, &copy, other))               \
			goto err_copy;                                                       \
		return copy;                                                             \
	err_copy:                                                                    \
		Dee_Decref(copy);                                                        \
	err:                                                                         \
		return NULL;                                                             \
	}
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Add, ADD)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Sub, SUB)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Mul, MUL)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Div, DIV)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Mod, MOD)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Shl, SHL)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Shr, SHR)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(And, AND)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Or, OR)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Xor, XOR)
DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR(Pow, POW)
#undef DEFINE_DEFAULT_FOO_WITH_INPLACE_FOO_OPERATOR

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultAddWithSub,
                         (DeeObject *self, DeeObject *other)) {
	DREF DeeObject *result, *neg_other;
	LOAD_TP_SELF;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = DeeType_INVOKE_SUB_NODEFAULT(tp_self, self, neg_other);
	Dee_Decref(neg_other);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultSubWithAdd,
                         (DeeObject *self, DeeObject *other)) {
	DREF DeeObject *result, *neg_other;
	LOAD_TP_SELF;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = DeeType_INVOKE_ADD_NODEFAULT(tp_self, self, neg_other);
	Dee_Decref(neg_other);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultAddWithInplaceSub,
                         (DeeObject *self, DeeObject *other)) {
	DREF DeeObject *result, *neg_other;
	LOAD_TP_SELF;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = COPY_SELF();
	if unlikely(!result)
		goto err_neg_other;
	if unlikely(DeeType_INVOKE_ISUB_NODEFAULT(tp_self, &result, neg_other))
		goto err_neg_other_result;
	Dee_Decref(neg_other);
	return result;
err_neg_other_result:
	Dee_Decref(result);
err_neg_other:
	Dee_Decref(neg_other);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultSubWithInplaceAdd,
                         (DeeObject *self, DeeObject *other)) {
	DREF DeeObject *result, *neg_other;
	LOAD_TP_SELF;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = COPY_SELF();
	if unlikely(!result)
		goto err_neg_other;
	if unlikely(DeeType_INVOKE_IADD_NODEFAULT(tp_self, &result, neg_other))
		goto err_neg_other_result;
	Dee_Decref(neg_other);
	return result;
err_neg_other_result:
	Dee_Decref(result);
err_neg_other:
	Dee_Decref(neg_other);
err:
	return NULL;
}

#undef COPY_SELF

/* Default wrappers for implementing inplace math operators using their non-inplace variants. */
#define DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Name, NAME)       \
	DEFINE_INTERNAL_OPERATOR(int, DefaultInplace##Name##With##Name,    \
	                         (DeeObject **p_self, DeeObject *other)) { \
		DREF DeeObject *result;                                        \
		LOAD_TP_SELFP;                                                 \
		result = DeeType_INVOKE_##NAME(tp_self, *p_self, other);       \
		if unlikely(!result)                                           \
			goto err;                                                  \
		Dee_Decref(*p_self);                                           \
		*p_self = result;                                              \
		return 0;                                                      \
	err:                                                               \
		return -1;                                                     \
	}
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Add, ADD)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Sub, SUB)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Mul, MUL)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Div, DIV)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Mod, MOD)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Shl, SHL)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Shr, SHR)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(And, AND)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Or, OR)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Xor, XOR)
DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR(Pow, POW)
#undef DEFINE_DEFAULT_INPLACE_FOO_WITH_FOO_OPERATOR

DEFINE_INTERNAL_OPERATOR(int, DefaultInplaceAddWithInplaceSub,
                         (DREF DeeObject **p_self, DeeObject *other)) {
	int result;
	DREF DeeObject *neg_other;
	LOAD_TP_SELFP;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = DeeType_INVOKE_ISUB_NODEFAULT(tp_self, p_self, neg_other);
	Dee_Decref(neg_other);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInplaceSubWithInplaceAdd,
                         (DREF DeeObject **p_self, DeeObject *other)) {
	int result;
	DREF DeeObject *neg_other;
	LOAD_TP_SELFP;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = DeeType_INVOKE_IADD_NODEFAULT(tp_self, p_self, neg_other);
	Dee_Decref(neg_other);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInplaceAddWithSub,
                         (DREF DeeObject **p_self, DeeObject *other)) {
	DREF DeeObject *result, *neg_other;
	LOAD_TP_SELFP;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = DeeType_INVOKE_SUB_NODEFAULT(tp_self, *p_self, neg_other);
	Dee_Decref(neg_other);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result;
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultInplaceSubWithAdd,
                         (DREF DeeObject **p_self, DeeObject *other)) {
	DREF DeeObject *result, *neg_other;
	LOAD_TP_SELFP;
	neg_other = DeeObject_Neg(other);
	if unlikely(!neg_other)
		goto err;
	result = DeeType_INVOKE_ADD_NODEFAULT(tp_self, *p_self, neg_other);
	Dee_Decref(neg_other);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result;
	return 0;
err:
	return -1;
}


/* Default wrappers for implementing inc/dec. */
DEFINE_INTERNAL_OPERATOR(int, DefaultIncWithInplaceAdd, (DREF DeeObject **p_self)) {
	LOAD_TP_SELFP;
	return DeeType_INVOKE_IADD_NODEFAULT(tp_self, p_self, DeeInt_One);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultIncWithInplaceSub, (DREF DeeObject **p_self)) {
	LOAD_TP_SELFP;
	return DeeType_INVOKE_IADD_NODEFAULT(tp_self, p_self, DeeInt_MinusOne);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDecWithInplaceAdd, (DREF DeeObject **p_self)) {
	LOAD_TP_SELFP;
	return DeeType_INVOKE_IADD_NODEFAULT(tp_self, p_self, DeeInt_MinusOne);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDecWithInplaceSub, (DREF DeeObject **p_self)) {
	LOAD_TP_SELFP;
	return DeeType_INVOKE_IADD_NODEFAULT(tp_self, p_self, DeeInt_One);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultIncWithAdd, (DREF DeeObject **p_self)) {
	DREF DeeObject *result;
	LOAD_TP_SELFP;
	result = DeeType_INVOKE_ADD_NODEFAULT(tp_self, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result;
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultIncWithSub, (DREF DeeObject **p_self)) {
	DREF DeeObject *result;
	LOAD_TP_SELFP;
	result = DeeType_INVOKE_SUB_NODEFAULT(tp_self, *p_self, DeeInt_MinusOne);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result;
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDecWithAdd, (DREF DeeObject **p_self)) {
	DREF DeeObject *result;
	LOAD_TP_SELFP;
	result = DeeType_INVOKE_ADD_NODEFAULT(tp_self, *p_self, DeeInt_MinusOne);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result;
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDecWithSub, (DREF DeeObject **p_self)) {
	DREF DeeObject *result;
	LOAD_TP_SELFP;
	result = DeeType_INVOKE_SUB_NODEFAULT(tp_self, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result;
	return 0;
err:
	return -1;
}


/* Default wrappers for implementing ==/!=/</<=/>/>= using their logical inverse. */
PRIVATE WUNUSED DREF DeeObject *DCALL
xinvoke_not(/*[0..1],inherit(always)*/ DREF DeeObject *ob) {
	if (ob) {
		int temp = DeeObject_BoolInherited(ob);
		if likely(temp >= 0) {
			ob = DeeBool_For(!temp);
			Dee_Incref(ob);
		} else {
			ob = NULL;
		}
	}
	return ob;
}


INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self);
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self);
INTDEF NONNULL((1)) Dee_hash_t DCALL DeeMap_HandleHashError(DeeObject *self);

#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) Dee_hash_t DCALL DeeSeq_HandleHashError(DeeObject *self) {
	DeeError_Print("Unhandled error in `Sequence.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}

INTERN NONNULL((1)) Dee_hash_t DCALL DeeSet_HandleHashError(DeeObject *self) {
	DeeError_Print("Unhandled error in `Set.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}

INTERN NONNULL((1)) Dee_hash_t DCALL DeeMap_HandleHashError(DeeObject *self) {
	DeeError_Print("Unhandled error in `Mapping.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}
#endif /* !DEFINE_TYPED_OPERATORS */

/* tp_bool */
DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoolWithSize, (DeeObject *self)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return size ? 1 : 0;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoolWithSizeOb, (DeeObject *self)) {
	DREF DeeObject *sizeob;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB(tp_self, self);
	if unlikely(!sizeob)
		goto err;
	/* XXX: This causes an unhandled stack-overflow in:
	 * >> class MySeq: Sequence from deemon {
	 * >>     operator # () -> this;
	 * >> }
	 * >> if (MySeq()) {
	 * >>     print "Won't ever get here...";
	 * >> } */
	return DeeObject_BoolInherited(sizeob);
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return -2;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoolWithForeach, (DeeObject *self)) {
	Dee_ssize_t foreach_status;
	LOAD_TP_SELF;
	foreach_status = DeeType_INVOKE_FOREACH_NODEFAULT(tp_self, self, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoolWithCompareEq, (DeeObject *self)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPAREEQ_NODEFAULT(tp_self, self, Dee_EmptySeq);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	if (result == -1)
		result = 1;
	ASSERT(result == 0 || result == 1);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoolWithEq, (DeeObject *self)) {
	int result;
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
	result_ob = DeeType_INVOKE_EQ_NODEFAULT(tp_self, self, Dee_EmptySeq);
	if unlikely(!result_ob)
		goto err;
	/* XXX: This causes an unhandled stack-overflow in:
	 * >> class MySeq: Sequence from deemon {
	 * >>     operator == (other) -> this;
	 * >> }
	 * >> if (MySeq()) {
	 * >>     print "Won't ever get here...";
	 * >> } */
	result = DeeObject_BoolInherited(result_ob);
	if (result > 0)
		result = !result;
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoolWithNe, (DeeObject *self)) {
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
	result_ob = DeeType_INVOKE_NE_NODEFAULT(tp_self, self, Dee_EmptySeq);
	if unlikely(!result_ob)
		goto err;
	/* XXX: This causes an unhandled stack-overflow in:
	 * >> class MySeq: Sequence from deemon {
	 * >>     operator != (other) -> this;
	 * >> }
	 * >> if (MySeq()) {
	 * >>     print "Won't ever get here...";
	 * >> } */
	return DeeObject_BoolInherited(result_ob);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoolWithForeachDefault, (DeeObject *self)) {
	Dee_ssize_t foreach_status;
	LOAD_TP_SELF;
	foreach_status = DeeType_INVOKE_FOREACH(tp_self, self, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}


/* tp_hash */
DEFINE_INTERNAL_SEQ_OPERATOR(Dee_hash_t, DefaultHashWithSizeAndGetItemIndexFast, (DeeObject *self)) {
	Dee_hash_t result;
	size_t i, size;
	DREF DeeObject *elem;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	elem = (*tp_self->tp_seq->tp_getitem_index_fast)(self, 0);
	if unlikely(!elem) {
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = (*tp_self->tp_seq->tp_getitem_index_fast)(self, i);
		if unlikely(!elem) {
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
	}
	return result;
err:
	return DeeSeq_HandleHashError(self);
}


struct default_seq_hash_with_foreach_data {
	Dee_hash_t sqhwf_result;   /* Hash result (or DEE_HASHOF_EMPTY_SEQUENCE when sqhwf_nonempty=false) */
	bool       sqhwf_nonempty; /* True after the first element */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_hash_with_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_hash_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_seq_hash_with_foreach_data *data;
	Dee_hash_t elem_hash;
	data = (struct default_seq_hash_with_foreach_data *)arg;
	elem_hash = DeeObject_Hash(elem);
	if (data->sqhwf_nonempty) {
		data->sqhwf_result = Dee_HashCombine(data->sqhwf_result, elem_hash);
	} else {
		data->sqhwf_result = elem_hash;
		data->sqhwf_nonempty = true;
	}
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_INTERNAL_SEQ_OPERATOR(Dee_hash_t, DefaultHashWithForeach, (DeeObject *self)) {
	struct default_seq_hash_with_foreach_data data;
	LOAD_TP_SELF;
	data.sqhwf_result   = DEE_HASHOF_EMPTY_SEQUENCE;
	data.sqhwf_nonempty = false;
	if unlikely(DeeType_INVOKE_FOREACH_NODEFAULT(tp_self, self, &default_seq_hash_with_foreach_cb, &data))
		goto err;
	return data.sqhwf_result;
err:
	return DeeSeq_HandleHashError(self);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_hash_t, DefaultHashWithSizeAndTryGetItemIndex, (DeeObject *self)) {
	Dee_hash_t result;
	size_t i, size;
	DREF DeeObject *elem;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	elem = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, 0);
	if unlikely(!elem)
		goto err;
	if unlikely(elem == ITER_DONE) {
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(elem)
			goto err;
		if unlikely(elem == ITER_DONE) {
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_hash_t, DefaultHashWithSizeAndGetItemIndex, (DeeObject *self)) {
	Dee_hash_t result;
	size_t i, size;
	DREF DeeObject *elem;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	elem = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, 0);
	if unlikely(!elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err;
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return DeeSeq_HandleHashError(self);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_hash_t, DefaultHashWithSizeObAndGetItem, (DeeObject *self)) {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *indexob, *sizeob;
	DREF DeeObject *elem;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	temp = DeeObject_CmpLoAsBool(indexob, sizeob);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err_sizeob_indexob;
		Dee_Decref(indexob);
		Dee_Decref(sizeob);
		return DEE_HASHOF_EMPTY_SEQUENCE;
	}
	elem = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
	if unlikely(!elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_sizeob_indexob;
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (;;) {
		Dee_hash_t elem_hash;
		if (DeeObject_Inc(&indexob))
			goto err_sizeob_indexob;
		temp = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (temp <= 0) {
			if unlikely(temp < 0)
				goto err_sizeob_indexob;
			break;
		}
		elem = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_indexob;
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return DeeSeq_HandleHashError(self);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_hash_t, DefaultHashWithForeachDefault, (DeeObject *self)) {
	struct default_seq_hash_with_foreach_data data;
	LOAD_TP_SELF;
	data.sqhwf_result   = DEE_HASHOF_EMPTY_SEQUENCE;
	data.sqhwf_nonempty = false;
	if unlikely(DeeType_INVOKE_FOREACH(tp_self, self, &default_seq_hash_with_foreach_cb, &data))
		goto err;
	return data.sqhwf_result;
err:
	return DeeSeq_HandleHashError(self);
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_hash_with_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_hash_with_foreach_cb(void *arg, DeeObject *elem) {
	*(Dee_hash_t *)arg ^= DeeObject_Hash(elem);
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SET_OPERATOR(Dee_hash_t, DefaultHashWithForeachDefault, (DeeObject *self)) {
	Dee_hash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	LOAD_TP_SELF;
	if unlikely(DeeType_INVOKE_FOREACH(tp_self, self, &default_set_hash_with_foreach_cb, &result))
		goto err;
	return result;
err:
	return DeeSet_HandleHashError(self);
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_hash_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_hash_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	/* Note that we still combine the hashes for the key and value,
	 * thus not only mirroring the behavior of hash of the item (that
	 * is the tuple `(key, value)', including the order between the
	 * key and value within the hash, so that swapping the key and
	 * value would produce a different hash) */
	*(Dee_hash_t *)arg ^= Dee_HashCombine(DeeObject_Hash(key),
	                                      DeeObject_Hash(value));
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(Dee_hash_t, DefaultHashWithForeachPairDefault, (DeeObject *self)) {
	Dee_hash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	LOAD_TP_SELF;
	if unlikely(DeeType_INVOKE_FOREACH_PAIR(tp_self, self, &default_map_hash_with_foreach_pair_cb, &result))
		goto err;
	return result;
err:
	return DeeMap_HandleHashError(self);
}


/* tp_eq */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultEqWithCompareEq,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPAREEQ_NODEFAULT(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result == 0);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultEqWithNe,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_NE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultEqWithLoAndGr,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LO_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		goto not_equal;
	cmp_ob = DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		goto not_equal;
	return_true;
not_equal:
	return_false;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultEqWithLeAndGe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LE_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		goto not_equal;
	cmp_ob = DeeType_INVOKE_GE_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		goto not_equal;
	return_true;
not_equal:
	return_false;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultEqWithCompareEqDefault,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPAREEQ(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result == 0);
err:
	return NULL;
}


/* tp_ne */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultNeWithCompareEq,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPAREEQ_NODEFAULT(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result != 0);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultNeWithEq,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_NE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultNeWithLoAndGr,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LO_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		goto not_equal;
	cmp_ob = DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		goto not_equal;
	return_false;
not_equal:
	return_true;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultNeWithLeAndGe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LE_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		goto not_equal;
	cmp_ob = DeeType_INVOKE_GE_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		goto not_equal;
	return_false;
not_equal:
	return_true;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultNeWithCompareEqDefault,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPAREEQ(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result != 0);
err:
	return NULL;
}


/* tp_lo */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLoWithCompare,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE_NODEFAULT(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result < 0);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLoWithGe,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_GE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLoWithCompareDefault,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result < 0);
err:
	return NULL;
}

DEFINE_INTERNAL_SET_OPERATOR(DREF DeeObject *, DefaultLoWithForeachDefault,
                             (DeeObject *self, DeeObject *other)) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct set_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_contains) && !DeeType_InheritContains(tp_other))
		goto err_other_no_contains;
	data.sc_lfr_rhs       = other;
	data.sc_lfr_rcontains = tp_other->tp_seq->tp_contains;
	contains_status = DeeType_INVOKE_FOREACH(tp_self, self, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self" */
	rhs_size = DeeObject_Size(other);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "other" contains element not found in "self" */
	return_true;
missing_item:
	return_false;
err_other_no_contains:
	err_unimplemented_operator(tp_other, OPERATOR_CONTAINS);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultLoWithForeachPairDefault,
                             (DeeObject *self, DeeObject *other)) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct map_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_trygetitem) && !DeeType_InheritGetItem(tp_other))
		goto err_other_no_getitem;
	data.mc_lfr_rhs         = other;
	data.mc_lfr_rtrygetitem = tp_other->tp_seq->tp_trygetitem;
	contains_status = DeeType_INVOKE_FOREACH_PAIR(tp_self, self, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self", or has a different value for it */
	rhs_size = DeeObject_Size(other);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "other" contains element not found in "self" */
	return_true;
missing_item:
	return_false;
err_other_no_getitem:
	err_unimplemented_operator(tp_other, OPERATOR_GETITEM);
err:
	return NULL;
}




/* tp_le */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLeWithCompare,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE_NODEFAULT(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result <= 0);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLeWithGr,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLeWithCompareDefault,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result <= 0);
err:
	return NULL;
}

DEFINE_INTERNAL_SET_OPERATOR(DREF DeeObject *, DefaultLeWithForeachDefault,
                             (DeeObject *self, DeeObject *other)) {
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct set_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_contains) && !DeeType_InheritContains(tp_other))
		goto err_other_no_contains;
	data.sc_lfr_rhs       = other;
	data.sc_lfr_rcontains = tp_other->tp_seq->tp_contains;
	contains_status = DeeType_INVOKE_FOREACH(tp_self, self, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self" */
	return_true;
missing_item:
	return_false;
err_other_no_contains:
	err_unimplemented_operator(tp_other, OPERATOR_CONTAINS);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultLeWithForeachPairDefault,
                             (DeeObject *self, DeeObject *other)) {
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct map_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_trygetitem) && !DeeType_InheritGetItem(tp_other))
		goto err_other_no_getitem;
	data.mc_lfr_rhs         = other;
	data.mc_lfr_rtrygetitem = tp_other->tp_seq->tp_trygetitem;
	contains_status = DeeType_INVOKE_FOREACH_PAIR(tp_self, self, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self", or has a different value for it */
	return_true;
missing_item:
	return_false;
err_other_no_getitem:
	err_unimplemented_operator(tp_other, OPERATOR_GETITEM);
err:
	return NULL;
}



/* tp_gr */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGrWithCompare,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE_NODEFAULT(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result > 0);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGrWithLe,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_LE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGrWithCompareDefault,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result > 0);
err:
	return NULL;
}

DEFINE_INTERNAL_SET_OPERATOR(DREF DeeObject *, DefaultGrWithForeachDefault,
                             (DeeObject *self, DeeObject *other)) {
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct set_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_contains) && !DeeType_InheritContains(tp_other))
		goto err_other_no_contains;
	data.sc_lfr_rhs       = other;
	data.sc_lfr_rcontains = tp_other->tp_seq->tp_contains;
	contains_status = DeeType_INVOKE_FOREACH(tp_self, self, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self" */
	return_false;
missing_item:
	return_true;
err_other_no_contains:
	err_unimplemented_operator(tp_other, OPERATOR_CONTAINS);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGrWithForeachPairDefault,
                             (DeeObject *self, DeeObject *other)) {
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct map_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_trygetitem) && !DeeType_InheritGetItem(tp_other))
		goto err_other_no_getitem;
	data.mc_lfr_rhs         = other;
	data.mc_lfr_rtrygetitem = tp_other->tp_seq->tp_trygetitem;
	contains_status = DeeType_INVOKE_FOREACH_PAIR(tp_self, self, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self", or has a different value for it */
	return_false;
missing_item:
	return_true;
err_other_no_getitem:
	err_unimplemented_operator(tp_other, OPERATOR_GETITEM);
err:
	return NULL;
}



/* tp_ge */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGeWithCompare,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE_NODEFAULT(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result >= 0);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGeWithLo,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_LO_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGeWithCompareDefault,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE(tp_self, self, other);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool_(result >= 0);
err:
	return NULL;
}

DEFINE_INTERNAL_SET_OPERATOR(DREF DeeObject *, DefaultGeWithForeachDefault,
                             (DeeObject *self, DeeObject *other)) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct set_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_contains) && !DeeType_InheritContains(tp_other))
		goto err_other_no_contains;
	data.sc_lfr_rhs       = other;
	data.sc_lfr_rcontains = tp_other->tp_seq->tp_contains;
	contains_status = DeeType_INVOKE_FOREACH(tp_self, self, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self" */
	rhs_size = DeeObject_Size(other);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "other" contains element not found in "self" */
	return_false;
missing_item:
	return_true;
err_other_no_contains:
	err_unimplemented_operator(tp_other, OPERATOR_CONTAINS);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGeWithForeachPairDefault,
                             (DeeObject *self, DeeObject *other)) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct map_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_trygetitem) && !DeeType_InheritGetItem(tp_other))
		goto err_other_no_getitem;
	data.mc_lfr_rhs         = other;
	data.mc_lfr_rtrygetitem = tp_other->tp_seq->tp_trygetitem;
	contains_status = DeeType_INVOKE_FOREACH_PAIR(tp_self, self, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "other" is missing some element of "self", or has a different value for it */
	rhs_size = DeeObject_Size(other);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "other" contains element not found in "self" */
	return_false;
missing_item:
	return_true;
err_other_no_getitem:
	err_unimplemented_operator(tp_other, OPERATOR_GETITEM);
err:
	return NULL;
}



/* tp_trycompare_eq */
DEFINE_INTERNAL_OPERATOR(int, DefaultTryCompareEqWithCompareEq,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPAREEQ_NODEFAULT(tp_self, self, other);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultTryCompareEqWithEq,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	DREF DeeObject *resultob;
	LOAD_TP_SELF;
	resultob = DeeType_INVOKE_EQ_NODEFAULT(tp_self, self, other);
	if unlikely(!resultob) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			return -1;
		goto err;
	}
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultTryCompareEqWithNe,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	DREF DeeObject *resultob;
	LOAD_TP_SELF;
	resultob = DeeType_INVOKE_NE_NODEFAULT(tp_self, self, other);
	if unlikely(!resultob) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			return -1;
		goto err;
	}
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultTryCompareEqWithCompare,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_COMPARE_NODEFAULT(tp_self, self, other);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultTryCompareEqWithLoAndGr,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LO_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err_tryhandle;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Different */
	cmp_ob = DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err_tryhandle;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Different */
	return 0;
err_tryhandle:
	if (DeeError_Catch(&DeeError_NotImplemented) ||
	    DeeError_Catch(&DeeError_TypeError) ||
	    DeeError_Catch(&DeeError_ValueError))
		return -1;
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultTryCompareEqWithLeAndGe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LE_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err_tryhandle;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 1; /* Different */
	cmp_ob = DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err_tryhandle;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return -1; /* Different */
	return 0;
err_tryhandle:
	if (DeeError_Catch(&DeeError_NotImplemented) ||
	    DeeError_Catch(&DeeError_TypeError) ||
	    DeeError_Catch(&DeeError_ValueError))
		return -1;
err:
	return Dee_COMPARE_ERR;
}




/* tp_compare_eq */
DEFINE_INTERNAL_OPERATOR(int, DefaultCompareEqWithEq,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_EQ_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareEqWithNe,
                         (DeeObject *self, DeeObject *other)) {
	int result;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_NE_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareEqWithLoAndGr,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LO_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Different */
	cmp_ob = DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareEqWithLeAndGe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_LE_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 1; /* Different */
	cmp_ob = DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return -1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareEqWithForeachDefault,
                             (DeeObject *self, DeeObject *other)) {
	DeeTypeObject *tp_other = Dee_TYPE(other);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	Dee_ssize_t result;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = other;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_other->tp_seq->tp_getitem_index_fast;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = other;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_other->tp_seq->tp_trygetitem_index;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compareeq__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = other;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_other->tp_seq->tp_getitem_index;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = other;
			data.scf_sg_ogetitem = tp_other->tp_seq->tp_getitem;
			result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compareeq__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				Dee_Decref(data.scf_sg_oindex);
				if unlikely(temp < 0) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
				}
			} else {
				Dee_Decref(data.scf_sg_oindex);
			}
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = (*tp_other->tp_seq->tp_iter)(other);
		if unlikely(!rhs_iter)
			goto err;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compareeq__lhs_foreach__rhs_iter__cb, rhs_iter);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			DREF DeeObject *next = DeeObject_IterNext(rhs_iter);
			Dee_Decref(rhs_iter);
			if unlikely(!next)
				goto err;
			if (next != ITER_DONE) {
				Dee_Decref(next);
				result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
			}
		} else {
			Dee_Decref(rhs_iter);
		}
	}
	ASSERT(result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
	       result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
	       result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
	if unlikely(result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
		goto err;
	if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL)
		return 0;
	return 1;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareEqWithSizeAndGetItemIndexFast,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem_index_fast = tp_self->tp_seq->tp_getitem_index_fast;
	lhs_size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(self, lhs_size, lhs_getitem_index_fast,
		                                                                                           other, rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(self, lhs_size, lhs_getitem_index_fast,
		                                                                                         other, rhs_size, tp_other->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(self, lhs_size, lhs_getitem_index_fast,
		                                                                                      other, rhs_size, tp_other->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(self, lhs_size, lhs_getitem_index_fast,
		                                                                                  other, rhs_sizeob, tp_other->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = self;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*other_tp_foreach)(other, &seq_compareeq__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&
		    data.scf_sgi_oindex >= data.scf_sgi_osize) {
			result = 0;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareEqWithSizeAndTryGetItemIndex,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_trygetitem_index = tp_self->tp_seq->tp_trygetitem_index;
	lhs_size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(self, lhs_size, lhs_trygetitem_index,
		                                                                                         other, rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(self, lhs_size, lhs_trygetitem_index,
		                                                                                       other, rhs_size, tp_other->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(self, lhs_size, lhs_trygetitem_index,
		                                                                                    other, rhs_size, tp_other->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(self, lhs_size, lhs_trygetitem_index,
		                                                                                other, rhs_sizeob, tp_other->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = self;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_trygetitem_index;
		foreach_result = (*other_tp_foreach)(other, &seq_compareeq__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&
		    data.scf_sgi_oindex >= data.scf_sgi_osize) {
			result = 0;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareEqWithSizeAndGetItemIndex,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem_index = tp_self->tp_seq->tp_getitem_index;
	lhs_size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(self, lhs_size, lhs_getitem_index,
		                                                                                      other, rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(self, lhs_size, lhs_getitem_index,
		                                                                                    other, rhs_size, tp_other->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index(self, lhs_size, lhs_getitem_index,
		                                                                                 other, rhs_size, tp_other->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(self, lhs_size, lhs_getitem_index,
		                                                                             other, rhs_sizeob, tp_other->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = self;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index;
		foreach_result = (*other_tp_foreach)(other, &seq_compareeq__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&
		    data.scf_sgi_oindex >= data.scf_sgi_osize) {
			result = 0;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareEqWithSizeObAndGetItem,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	DREF DeeObject *lhs_sizeob;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index);
#ifdef DEFINE_TYPED_OPERATORS
	DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
#endif /* DEFINE_TYPED_OPERATORS */
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem = tp_self->tp_seq->tp_getitem;
#ifdef DEFINE_TYPED_OPERATORS
	lhs_tgetitem = lhs_getitem == &instance_getitem ? &instance_tgetitem : NULL;
#endif /* DEFINE_TYPED_OPERATORS */
	lhs_sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
	if unlikely(!lhs_sizeob)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompareeq__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index_fast(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                                   rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(self, lhs_sizeob, lhs_getitem, other,
			                                                                                  rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
		}
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompareeq__lhs_tsizeob_and_getitem__rhs_size_and_trygetitem_index(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                                 rhs_size, tp_other->tp_seq->tp_trygetitem_index);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(self, lhs_sizeob, lhs_getitem, other,
			                                                                                rhs_size, tp_other->tp_seq->tp_trygetitem_index);
		}
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompareeq__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                              rhs_size, tp_other->tp_seq->tp_getitem_index);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(self, lhs_sizeob, lhs_getitem, other,
			                                                                             rhs_size, tp_other->tp_seq->tp_getitem_index);
		}
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompareeq__lhs_tsizeob_and_getitem__rhs_sizeob_and_getitem(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                          rhs_sizeob, tp_other->tp_seq->tp_getitem);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(self, lhs_sizeob, lhs_getitem, other,
			                                                                         rhs_sizeob, tp_other->tp_seq->tp_getitem);
		}
		Dee_Decref(rhs_sizeob);
	} else {
		Dee_ssize_t foreach_result;
		DREF DeeObject *lhs_indexob;
		lhs_indexob = DeeObject_NewDefault(Dee_TYPE(lhs_sizeob));
		if unlikely(!lhs_indexob)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			struct seq_compare_foreach__tsizeob_and_getitem__data data;
			data.scf_tsg_tpother   = tp_self;
			data.scf_tsg_other     = self;
			data.scf_tsg_osize     = lhs_sizeob;
			data.scf_tsg_oindex    = lhs_indexob;
			data.scf_tsg_otgetitem = lhs_tgetitem;
			foreach_result = (*other_tp_foreach)(other, &seq_compareeq__tlhs_sizeob_and_getitem__rhs_foreach__cb, &data);
			lhs_indexob = data.scf_tsg_oindex;
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			struct seq_compare_foreach__sizeob_and_getitem__data data;
			data.scf_sg_other    = self;
			data.scf_sg_osize    = lhs_sizeob;
			data.scf_sg_oindex   = lhs_indexob;
			data.scf_sg_ogetitem = lhs_getitem;
			foreach_result = (*other_tp_foreach)(other, &seq_compareeq__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
			lhs_indexob = data.scf_sg_oindex;
		}
		Dee_Decref(lhs_indexob);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err_lhs_sizeob;
		result = foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ? 0 : 1;
	}
	Dee_Decref(lhs_sizeob);
	return result;
err_lhs_sizeob:
	Dee_Decref(lhs_sizeob);
	return Dee_COMPARE_ERR;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SET_OPERATOR(int, DefaultCompareEqWithForeachDefault,
                             (DeeObject *self, DeeObject *other)) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct set_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_contains) && !DeeType_InheritContains(tp_other))
		goto err_other_no_contains;
	data.sc_lfr_rhs       = other;
	data.sc_lfr_rcontains = tp_other->tp_seq->tp_contains;
	contains_status = DeeType_INVOKE_FOREACH(tp_self, self, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		return 1; /* "other" is missing some element of "self" */
	rhs_size = DeeObject_Size(other);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status != rhs_size)
		return 1; /* Sets have different sizes */
	return 0;
err_other_no_contains:
	err_unimplemented_operator(tp_other, OPERATOR_CONTAINS);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultCompareEqWithForeachPairDefault,
                             (DeeObject *self, DeeObject *other)) {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	struct map_compare__lhs_foreach__rhs__data data;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_trygetitem) && !DeeType_InheritGetItem(tp_other))
		goto err_other_no_getitem;
	data.mc_lfr_rhs         = other;
	data.mc_lfr_rtrygetitem = tp_other->tp_seq->tp_trygetitem;
	contains_status = DeeType_INVOKE_FOREACH_PAIR(tp_self, self, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		return 1; /* "other" is missing some element of "self", or has a different value for it */
	rhs_size = DeeObject_Size(other);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status != rhs_size)
		return 1; /* Maps have different sizes */
	return 0;
err_other_no_getitem:
	err_unimplemented_operator(tp_other, OPERATOR_GETITEM);
err:
	return Dee_COMPARE_ERR;
}


/* tp_compare */
DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithEqAndLo,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_EQ(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_LO(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithEqAndLe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_EQ(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_LE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithEqAndGr,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_EQ(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_GR(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithEqAndGe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_EQ(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_GE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithNeAndLo,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_NE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_LO(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithNeAndLe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_NE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_LE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithNeAndGr,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_NE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_GR(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultCompareWithNeAndGe,
                         (DeeObject *self, DeeObject *other)) {
	int temp;
	DREF DeeObject *cmp_ob;
	LOAD_TP_SELF;
	cmp_ob = DeeType_INVOKE_NE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = DeeType_INVOKE_GE(tp_self, self, other);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareWithForeachDefault,
                             (DeeObject *self, DeeObject *other)) {
	DeeTypeObject *tp_other = Dee_TYPE(other);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	Dee_ssize_t result;
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = other;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_other->tp_seq->tp_getitem_index_fast;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compare__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = other;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_other->tp_seq->tp_trygetitem_index;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compare__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = other;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_other->tp_seq->tp_getitem_index;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compare__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = other;
			data.scf_sg_ogetitem = tp_other->tp_seq->tp_getitem;
			result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compare__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				Dee_Decref(data.scf_sg_oindex);
				if unlikely(temp < 0) {
					result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPARE_FOREACH_RESULT_LESS;
				}
			} else {
				Dee_Decref(data.scf_sg_oindex);
			}
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = (*tp_other->tp_seq->tp_iter)(other);
		if unlikely(!rhs_iter)
			goto err;
		result = DeeType_INVOKE_FOREACH(tp_self, self, &seq_compare__lhs_foreach__rhs_iter__cb, rhs_iter);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			DREF DeeObject *next = DeeObject_IterNext(rhs_iter);
			Dee_Decref(rhs_iter);
			if unlikely(!next)
				goto err;
			if (next != ITER_DONE) {
				Dee_Decref(next);
				result = SEQ_COMPARE_FOREACH_RESULT_LESS;
			}
		} else {
			Dee_Decref(rhs_iter);
		}
	}
	ASSERT(result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
	       result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
	       result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
	       result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
	if unlikely(result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
		goto err;
	if (result == SEQ_COMPARE_FOREACH_RESULT_LESS)
		return -1;
	if (result == SEQ_COMPARE_FOREACH_RESULT_GREATER)
		return 1;
	return 0; /* Equal */
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareWithSizeAndGetItemIndexFast,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem_index_fast = tp_self->tp_seq->tp_getitem_index_fast;
	lhs_size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(self, lhs_size, lhs_getitem_index_fast,
		                                                                                         other, rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(self, lhs_size, lhs_getitem_index_fast,
		                                                                                       other, rhs_size, tp_other->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(self, lhs_size, lhs_getitem_index_fast,
		                                                                                    other, rhs_size, tp_other->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(self, lhs_size, lhs_getitem_index_fast,
		                                                                                other, rhs_sizeob, tp_other->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = self;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*other_tp_foreach)(other, &seq_compare__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			result = 0;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareWithSizeAndTryGetItemIndex,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_trygetitem_index = tp_self->tp_seq->tp_getitem_index;
	lhs_size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(self, lhs_size, lhs_trygetitem_index,
		                                                                                       other, rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(self, lhs_size, lhs_trygetitem_index,
		                                                                                     other, rhs_size, tp_other->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(self, lhs_size, lhs_trygetitem_index,
		                                                                                  other, rhs_size, tp_other->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(self, lhs_size, lhs_trygetitem_index,
		                                                                              other, rhs_sizeob, tp_other->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = self;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_trygetitem_index;
		foreach_result = (*other_tp_foreach)(other, &seq_compare__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			result = 0;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareWithSizeAndGetItemIndex,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem_index = tp_self->tp_seq->tp_getitem_index;
	lhs_size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(self, lhs_size, lhs_getitem_index,
		                                                                                    other, rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(self, lhs_size, lhs_getitem_index,
		                                                                                  other, rhs_size, tp_other->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index(self, lhs_size, lhs_getitem_index,
		                                                                               other, rhs_size, tp_other->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(self, lhs_size, lhs_getitem_index,
		                                                                           other, rhs_sizeob, tp_other->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = self;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index;
		foreach_result = (*other_tp_foreach)(other, &seq_compare__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			result = 0;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultCompareWithSizeObAndGetItem,
                             (DeeObject *self, DeeObject *other)) {
	int result;
	DREF DeeObject *lhs_sizeob;
	DeeTypeObject *tp_other = Dee_TYPE(other);
	DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index);
#ifdef DEFINE_TYPED_OPERATORS
	DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
#endif /* DEFINE_TYPED_OPERATORS */
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	LOAD_TP_SELF;
	if ((!tp_other->tp_seq || !tp_other->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_other))
		goto err_other_no_iter;
	other_tp_foreach = tp_other->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem = tp_self->tp_seq->tp_getitem;
#ifdef DEFINE_TYPED_OPERATORS
	lhs_tgetitem = lhs_getitem == &instance_getitem ? &instance_tgetitem : NULL;
#endif /* DEFINE_TYPED_OPERATORS */
	lhs_sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
	if unlikely(!lhs_sizeob)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompare__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index_fast(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                                 rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(self, lhs_sizeob, lhs_getitem, other,
			                                                                                rhs_size, tp_other->tp_seq->tp_getitem_index_fast);
		}
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompare__lhs_tsizeob_and_getitem__rhs_size_and_trygetitem_index(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                               rhs_size, tp_other->tp_seq->tp_trygetitem_index);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(self, lhs_sizeob, lhs_getitem, other,
			                                                                              rhs_size, tp_other->tp_seq->tp_trygetitem_index);
		}
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_other->tp_seq->tp_size)(other);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompare__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                            rhs_size, tp_other->tp_seq->tp_getitem_index);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(self, lhs_sizeob, lhs_getitem, other,
			                                                                           rhs_size, tp_other->tp_seq->tp_getitem_index);
		}
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_other->tp_seq->tp_sizeob)(other);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			result = seq_docompare__lhs_tsizeob_and_getitem__rhs_sizeob_and_getitem(tp_self, self, lhs_sizeob, lhs_tgetitem, other,
			                                                                        rhs_sizeob, tp_other->tp_seq->tp_getitem);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			result = seq_docompare__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(self, lhs_sizeob, lhs_getitem, other,
			                                                                       rhs_sizeob, tp_other->tp_seq->tp_getitem);
		}
		Dee_Decref(rhs_sizeob);
	} else {
		Dee_ssize_t foreach_result;
		DREF DeeObject *lhs_indexob;
		lhs_indexob = DeeObject_NewDefault(Dee_TYPE(lhs_sizeob));
		if unlikely(!lhs_indexob)
			goto err_lhs_sizeob;
#ifdef DEFINE_TYPED_OPERATORS
		if (lhs_tgetitem) {
			struct seq_compare_foreach__tsizeob_and_getitem__data data;
			data.scf_tsg_tpother   = tp_self;
			data.scf_tsg_other     = self;
			data.scf_tsg_osize     = lhs_sizeob;
			data.scf_tsg_oindex    = lhs_indexob;
			data.scf_tsg_otgetitem = lhs_tgetitem;
			foreach_result = (*other_tp_foreach)(other, &seq_compare__tlhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		} else
#endif /* DEFINE_TYPED_OPERATORS */
		{
			struct seq_compare_foreach__sizeob_and_getitem__data data;
			data.scf_sg_other    = self;
			data.scf_sg_osize    = lhs_sizeob;
			data.scf_sg_oindex   = lhs_indexob;
			data.scf_sg_ogetitem = lhs_getitem;
			foreach_result = (*other_tp_foreach)(other, &seq_compare__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		}
		Dee_Decref(lhs_indexob);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err_lhs_sizeob;
		result = foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ? 0 : 1;
	}
	Dee_Decref(lhs_sizeob);
	return result;
err_lhs_sizeob:
	Dee_Decref(lhs_sizeob);
	return Dee_COMPARE_ERR;
err_other_no_iter:
	err_unimplemented_operator(tp_other, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}





/* Default wrappers for implementing OPERATOR_ITER via `tp_iter <===> tp_foreach <===> tp_foreach_pair' */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIterWithForeach, (DeeObject *__restrict self)) {
	LOAD_TP_SELF;
	/* TODO: Custom iterator type that uses "tp_foreach" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIterWithForeachPair,
                         (DeeObject *__restrict self)) {
	LOAD_TP_SELF;
	/* TODO: Custom iterator type that uses "tp_foreach_pair" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIterWithEnumerate,
                         (DeeObject *__restrict self)) {
	LOAD_TP_SELF;
	/* TODO: Custom iterator type that uses "tp_enumerate" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIterWithEnumerateIndex,
                         (DeeObject *__restrict self)) {
	LOAD_TP_SELF;
	/* TODO: Custom iterator type that uses "tp_enumerate_index" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}


/* tp_foreach */
DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachWithIter,
                         (DeeObject *__restrict self, Dee_foreach_t proc, void *arg)) {
	Dee_ssize_t temp, result;
	DREF DeeObject *iter, *elem;
	LOAD_TP_SELF;
	iter = DeeType_INVOKE_ITER(tp_self, self);
	if unlikely(!iter)
		goto err;
	result = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err_temp_iter;
		result += temp;
		/* Must check for interrupts because iterator may produce infinite results. */
		if (DeeThread_CheckInterrupt())
			goto err_temp_iter_m1;
	}
	Dee_Decref_likely(iter);
	if unlikely(!elem)
		goto err;
	return result;
err_temp_iter_m1:
	temp = -1;
err_temp_iter:
	Dee_Decref_likely(iter);
	return temp;
err:
	return -1;
}

struct default_foreach_with_enumerate_data {
	Dee_foreach_t dfwe_proc; /* [1..1] Underlying callback */
	void         *dfwe_arg;  /* [?..?] Cookie for `dfwe_proc' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_with_enumerate_data *data;
	data = (struct default_foreach_with_enumerate_data *)arg;
	(void)key;
	if likely(value)
		return (*data->dfwe_proc)(data->dfwe_arg, value);
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachWithEnumerate,
                         (DeeObject *__restrict self, Dee_foreach_t proc, void *arg)) {
	struct default_foreach_with_enumerate_data data;
	LOAD_TP_SELF;
	data.dfwe_proc = proc;
	data.dfwe_arg  = arg;
	return DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_foreach_with_enumerate_cb, &data);
}

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachWithEnumerateIndex,
                         (DeeObject *__restrict self, Dee_foreach_t proc, void *arg)) {
	struct default_foreach_with_enumerate_data data;
	LOAD_TP_SELF;
	data.dfwe_proc = proc;
	data.dfwe_arg  = arg;
	return DeeType_INVOKE_ENUMERATE_INDEX_NODEFAULT(tp_self, self, (Dee_enumerate_index_t)&default_foreach_with_enumerate_cb, &data, 0, (size_t)-1);
}


DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachPairWithIter,
                         (DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg)) {
	Dee_ssize_t temp, result;
	DREF DeeObject *iter, *elem;
	LOAD_TP_SELF;
	iter = DeeType_INVOKE_ITER(tp_self, self);
	if unlikely(!iter)
		goto err;
	result = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		DREF DeeObject *pair[2];
		if unlikely(DeeObject_Unpack(elem, 2, pair))
			goto err_temp_iter_elem_m1;
		Dee_Decref(elem);
		temp = (*proc)(arg, pair[0], pair[1]);
		Dee_Decref(pair[1]);
		Dee_Decref(pair[0]);
		if unlikely(temp < 0)
			goto err_temp_iter;
		result += temp;
		/* Must check for interrupts because iterator may produce infinite results. */
		if (DeeThread_CheckInterrupt())
			goto err_temp_iter_m1;
	}
	Dee_Decref_likely(iter);
	if unlikely(!elem)
		goto err;
	return result;
err_temp_iter_m1:
	temp = -1;
	goto err_temp_iter;
err_temp_iter_elem_m1:
	temp = -1;
	Dee_Decref(elem);
err_temp_iter:
	Dee_Decref_likely(iter);
	return temp;
err:
	return -1;
}

struct default_map_foreach_pair_with_enumerate_data {
	Dee_foreach_pair_t dmfpwe_proc; /* [1..1] Underlying callback */
	void              *dmfpwe_arg;  /* [?..?] Cookie for `dmfpwe_proc' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_foreach_pair_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_foreach_pair_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_foreach_pair_with_enumerate_data *data;
	data = (struct default_map_foreach_pair_with_enumerate_data *)arg;
	if likely(value)
		return (*data->dmfpwe_proc)(data->dmfpwe_arg, key, value);
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(Dee_ssize_t, DefaultForeachPairWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_pair_t proc, void *arg)) {
	struct default_map_foreach_pair_with_enumerate_data data;
	LOAD_TP_SELF;
	data.dmfpwe_proc = proc;
	data.dmfpwe_arg  = arg;
	return DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_foreach_pair_with_enumerate_cb, &data);
}

DEFINE_INTERNAL_MAP_OPERATOR(Dee_ssize_t, DefaultForeachPairWithEnumerateIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_pair_t proc, void *arg)) {
	struct default_map_foreach_pair_with_enumerate_data data;
	LOAD_TP_SELF;
	data.dmfpwe_proc = proc;
	data.dmfpwe_arg  = arg;
	return DeeType_INVOKE_ENUMERATE_INDEX_NODEFAULT(tp_self, self, (Dee_enumerate_index_t)&default_map_foreach_pair_with_enumerate_cb, &data, 0, (size_t)-1);
}


struct default_foreach_with_foreach_pair_data {
	Dee_foreach_t dfwfp_proc; /* [1..1] Underlying callback. */
	void         *dfwfp_arg;  /* Cookie for `dfwfp_proc' */
};

INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_foreach_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_with_foreach_pair_data *data;
	Dee_ssize_t result;
	DREF DeeObject *pair;
	data = (struct default_foreach_with_foreach_pair_data *)arg;
	pair = DeeTuple_PackSymbolic(2, key, value);
	if unlikely(!pair)
		goto err;
	result = (*data->dfwfp_proc)(data->dfwfp_arg, pair);
	DeeTuple_DecrefSymbolic(pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachWithForeachPair,
                         (DeeObject *__restrict self, Dee_foreach_t proc, void *arg)) {
	struct default_foreach_with_foreach_pair_data data;
	LOAD_TP_SELF;
	data.dfwfp_proc = proc;
	data.dfwfp_arg  = arg;
	return DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_foreach_with_foreach_pair_cb, &data);
}

struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_proc; /* [1..1] Underlying callback. */
	void              *dfpwf_arg;  /* Cookie for `dfpwf_proc' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_foreach_pair_with_foreach_data *data;
	Dee_ssize_t result;
	data = (struct default_foreach_pair_with_foreach_data *)arg;
	if likely(DeeTuple_Check(elem) && DeeTuple_SIZE(elem) == 2) {
		result = (*data->dfpwf_proc)(data->dfpwf_arg,
		                             DeeTuple_GET(elem, 0),
		                             DeeTuple_GET(elem, 1));
	} else {
		DREF DeeObject *pair[2];
		if unlikely(DeeObject_Unpack(elem, 2, pair))
			goto err;
		result = (*data->dfpwf_proc)(data->dfpwf_arg, pair[0], pair[1]);
		Dee_Decref_unlikely(pair[1]);
		Dee_Decref_unlikely(pair[0]);
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	(void)key;
	if likely(value)
		return default_foreach_pair_with_foreach_cb(arg, value);
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachPairWithForeach,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_pair_t proc, void *arg)) {
	struct default_foreach_pair_with_foreach_data data;
	LOAD_TP_SELF;
	data.dfpwf_proc = proc;
	data.dfpwf_arg  = arg;
	return DeeType_INVOKE_FOREACH_NODEFAULT(tp_self, self, &default_foreach_pair_with_foreach_cb, &data);
}

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachPairWithEnumerate,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_pair_t proc, void *arg)) {
	struct default_foreach_pair_with_foreach_data data;
	LOAD_TP_SELF;
	data.dfpwf_proc = proc;
	data.dfpwf_arg  = arg;
	return DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_foreach_pair_with_enumerate_cb, &data);
}


/* tp_enumerate */
struct default_foreach_pair_with_enumerate_index_data {
	Dee_foreach_pair_t dfpwei_proc; /* [1..1] Wrapped callback. */
	void              *dfpwei_arg;  /* [?..?] Cookie for `dfpwei_proc' */
};

INTDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_foreach_pair_with_enumerate_index_cb(void *arg, size_t index, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_foreach_pair_with_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	Dee_ssize_t result;
	DREF DeeObject *indexob;
	struct default_foreach_pair_with_enumerate_index_data *data;
	if unlikely(!value)
		return 0;
	data = (struct default_foreach_pair_with_enumerate_index_data *)arg;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*data->dfpwei_proc)(data->dfpwei_arg, indexob, value);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachPairWithEnumerateIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_pair_t proc, void *arg)) {
	struct default_foreach_pair_with_enumerate_index_data data;
	LOAD_TP_SELF;
	data.dfpwei_proc = proc;
	data.dfpwei_arg  = arg;
	return DeeType_INVOKE_ENUMERATE_INDEX_NODEFAULT(tp_self, self, &default_foreach_pair_with_enumerate_index_cb, &data, 0, (size_t)-1);
}

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachPairWithForeachDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_pair_t proc, void *arg)) {
	struct default_foreach_pair_with_foreach_data data;
	LOAD_TP_SELF;
	data.dfpwf_proc = proc;
	data.dfpwf_arg  = arg;
	return DeeType_INVOKE_FOREACH(tp_self, self, &default_foreach_pair_with_foreach_cb, &data);
}


DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultIterWithSizeAndGetItemIndexFast,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	size_t size;
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index_fast;
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultIterWithSizeAndTryGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	size_t size;
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	ASSERT(!DeeType_IsDefaultGetItemIndex(tp_self->tp_seq->tp_getitem_index));
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = tp_self->tp_seq->tp_trygetitem_index;
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultIterWithSizeAndGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	size_t size;
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	ASSERT(!DeeType_IsDefaultGetItemIndex(tp_self->tp_seq->tp_getitem_index));
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index;
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultIterWithGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DefaultIterator_WithGetItemIndex *result;
	LOAD_TP_SELF;
	ASSERT(!DeeType_IsDefaultGetItemIndex(tp_self->tp_seq->tp_getitem_index));
	result = DeeGCObject_MALLOC(DefaultIterator_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->digi_seq              = self;
	result->digi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index;
	result->digi_index            = 0;
	DeeObject_Init(result, &DefaultIterator_WithGetItemIndex_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultIterWithSizeObAndGetItem,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *sizeob;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB(tp_self, self);
	if unlikely(!sizeob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	if (!Dee_TYPE(self)->tp_seq ||
	    (Dee_TYPE(self)->tp_seq->tp_getitem != tp_self->tp_seq->tp_getitem)) {
		DREF DeeObject *(DCALL *tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
		tp_tgetitem = tp_self->tp_seq->tp_getitem == &instance_getitem ? &instance_tgetitem : NULL;
		if (tp_tgetitem) {
			DREF DefaultIterator_WithTSizeAndGetItem *result;
			result = DeeGCObject_MALLOC(DefaultIterator_WithTSizeAndGetItem);
			if unlikely(!result)
				goto err_size_ob;
			result->ditsg_index = DeeObject_NewDefault(Dee_TYPE(sizeob));
			if unlikely(!result->ditsg_index) {
				DeeGCObject_FREE(result);
				goto err_size_ob;
			}
			Dee_Incref(self);
			result->ditsg_seq         = self; /* Inherit reference */
			result->ditsg_tp_tgetitem = tp_tgetitem;
			result->ditsg_end         = sizeob; /* Inherit reference */
			Dee_atomic_lock_init(&result->ditsg_lock);
			DeeObject_Init(result, &DefaultIterator_WithTSizeAndGetItem_Type);
			return DeeGC_Track((DREF DeeObject *)result);
		}
	}
#endif /* DEFINE_TYPED_OPERATORS */
	{
		DREF DefaultIterator_WithSizeAndGetItem *result;
		result = DeeGCObject_MALLOC(DefaultIterator_WithSizeAndGetItem);
		if unlikely(!result)
			goto err_size_ob;
		result->disg_index = DeeObject_NewDefault(Dee_TYPE(sizeob));
		if unlikely(!result->disg_index) {
			DeeGCObject_FREE(result);
			goto err_size_ob;
		}
		Dee_Incref(self);
		result->disg_seq        = self; /* Inherit reference */
		result->disg_tp_getitem = tp_self->tp_seq->tp_getitem;
		result->disg_end        = sizeob; /* Inherit reference */
		Dee_atomic_lock_init(&result->disg_lock);
		DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItem_Type);
		return DeeGC_Track((DREF DeeObject *)result);
	}
	__builtin_unreachable();
err_size_ob:
	Dee_Decref(sizeob);
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultIterWithGetItem,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
#ifdef DEFINE_TYPED_OPERATORS
	if (!Dee_TYPE(self)->tp_seq ||
	    (Dee_TYPE(self)->tp_seq->tp_getitem != tp_self->tp_seq->tp_getitem)) {
		DREF DeeObject *(DCALL *tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
		tp_tgetitem = tp_self->tp_seq->tp_getitem == &instance_getitem ? &instance_tgetitem : NULL;
		if (tp_tgetitem) {
			DREF DefaultIterator_WithTGetItem *result;
			result = DeeGCObject_MALLOC(DefaultIterator_WithTGetItem);
			if unlikely(!result)
				goto err;
			result->ditg_tp_seq = tp_self;
			Dee_Incref(DeeInt_Zero);
			result->ditg_index = DeeInt_Zero;
			Dee_Incref(self);
			result->ditg_seq         = self; /* Inherit reference */
			result->ditg_tp_tgetitem = tp_tgetitem;
			Dee_atomic_lock_init(&result->ditg_lock);
			DeeObject_Init(result, &DefaultIterator_WithTSizeAndGetItem_Type);
			return DeeGC_Track((DREF DeeObject *)result);
		}
	}
#endif /* DEFINE_TYPED_OPERATORS */
	{
		DREF DefaultIterator_WithGetItem *result;
		result = DeeGCObject_MALLOC(DefaultIterator_WithGetItem);
		if unlikely(!result)
			goto err;
		Dee_Incref(DeeInt_Zero);
		result->dig_index = DeeInt_Zero;
		Dee_Incref(self);
		result->dig_seq        = self; /* Inherit reference */
		result->dig_tp_getitem = tp_self->tp_seq->tp_getitem;
		Dee_atomic_lock_init(&result->dig_lock);
		DeeObject_Init(result, &DefaultIterator_WithGetItem_Type);
		return DeeGC_Track((DREF DeeObject *)result);
	}
	__builtin_unreachable();
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultForeachWithSizeAndGetItemIndexFast,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = (*tp_self->tp_seq->tp_getitem_index_fast)(self, i);
		if unlikely(!elem)
			continue;
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultForeachWithSizeAndTryGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem == ITER_DONE)
				continue; /* Unbound item */
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err;
		}
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultForeachWithSizeAndGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err;
		}
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultForeachWithSizeObAndGetItem,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *i, *size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
	if unlikely(!size)
		goto err;
	i = DeeObject_NewDefault(Dee_TYPE(size));
	if unlikely(!i)
		goto err_size;
	for (;;) {
		DREF DeeObject *elem;
		int cmp_status;
		cmp_status = DeeObject_CmpLoAsBool(i, size);
		if unlikely(cmp_status < 0)
			goto err_size_i;
		if (!cmp_status)
			break;
		elem = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err_size_i;
		}
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0) {
			result = temp;
			break;
		}
		result += temp;
		if (DeeObject_Inc(&i))
			goto err_size_i;
	}
	Dee_Decref(i);
	Dee_Decref(size);
	return result;
err_size_i:
	Dee_Decref(i);
err_size:
	Dee_Decref(size);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultForeachWithSizeDefaultAndGetItemIndexDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = DeeType_INVOKE_GETITEMINDEX(tp_self, self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err;
		}
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultForeachWithGetItemIndexDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	LOAD_TP_SELF;
	for (i = 0;; ++i) {
		DREF DeeObject *elem;
		elem = DeeType_INVOKE_GETITEMINDEX(tp_self, self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err;
		}
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}






/* tp_enumerate */
struct default_enumerate_with_enumerate_index_data {
	Dee_enumerate_t dewei_proc; /* [1..1] Wrapped callback. */
	void           *dewei_arg;  /* [?..?] Cookie for `dewei_proc' */
};

INTDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_enumerate_with_enumerate_index_cb(void *arg, size_t index, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_enumerate_with_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	Dee_ssize_t result;
	DREF DeeObject *indexob;
	struct default_enumerate_with_enumerate_index_data *data;
	data = (struct default_enumerate_with_enumerate_index_data *)arg;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*data->dewei_proc)(data->dewei_arg, indexob, value);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultEnumerateWithEnumerateIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	struct default_enumerate_with_enumerate_index_data data;
	LOAD_TP_SELF;
	data.dewei_proc = proc;
	data.dewei_arg  = arg;
	return DeeType_INVOKE_ENUMERATE_INDEX_NODEFAULT(tp_self, self, &default_enumerate_with_enumerate_index_cb, &data, 0, (size_t)-1);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithSizeAndGetItemIndexFast,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *indexob, *index_value;
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = (*tp_self->tp_seq->tp_getitem_index_fast)(self, i);
		temp = (*proc)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithSizeAndTryGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	DREF DeeObject *indexob, *index_value;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(!index_value)
			goto err_indexob;
		if (index_value == ITER_DONE) {
			temp = (*proc)(arg, indexob, NULL);
		} else {
			temp = (*proc)(arg, indexob, index_value);
			Dee_Decref(index_value);
		}
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithSizeAndGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	DREF DeeObject *indexob, *index_value;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_indexob;
		}
		temp = (*proc)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithSizeObAndGetItem,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value, *sizeob;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	for (;;) {
		int index_is_less_than_size = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (index_is_less_than_size <= 0) {
			if unlikely(index_is_less_than_size < 0)
				goto err_sizeob_indexob;
			break;
		}
		index_value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_indexob;
		}
		temp = (*proc)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_sizeob_indexob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_indexob:
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return temp;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

struct default_enumerate_with_counter_and_foreach_data {
	Dee_enumerate_t dewcaf_proc;    /* [1..1] Wrapped callback */
	void           *dewcaf_arg;     /* [?..?] Cookie for `dewcaf_proc' */
	size_t          dewcaf_counter; /* Index of the next element that will be enumerated */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_with_counter_and_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_with_counter_and_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	DREF DeeObject *indexob;
	struct default_enumerate_with_counter_and_foreach_data *data;
	data = (struct default_enumerate_with_counter_and_foreach_data *)arg;
	indexob = DeeInt_NewSize(data->dewcaf_counter);
	if unlikely(!indexob)
		goto err;
	++data->dewcaf_counter;
	result = (*data->dewcaf_proc)(data->dewcaf_arg, indexob, elem);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithCounterAndForeach,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	struct default_enumerate_with_counter_and_foreach_data data;
	LOAD_TP_SELF;
	data.dewcaf_proc    = proc;
	data.dewcaf_arg     = arg;
	data.dewcaf_counter = 0;
	return DeeType_INVOKE_FOREACH_NODEFAULT(tp_self, self, &default_enumerate_with_counter_and_foreach_cb, &data);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithCounterAndIter,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	size_t counter = 0;
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *iter, *elem;
	LOAD_TP_SELF;
	iter = DeeType_INVOKE_ITER_NODEFAULT(tp_self, self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		DREF DeeObject *indexob;
		indexob = DeeInt_NewSize(counter);
		if unlikely(!indexob)
			goto err_iter_elem;
		temp = (*proc)(arg, indexob, elem);
		Dee_Decref(elem);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp_iter;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_iter;
		++counter;
	}
	if unlikely(!elem)
		goto err_iter;
	Dee_Decref(iter);
	return result;
err_temp_iter:
	Dee_Decref(iter);
/*err_temp:*/
	return temp;
err_iter_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(Dee_ssize_t, DefaultEnumerateWithForeachPairDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_FOREACH_PAIR(tp_self, self, proc, arg);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	DREF DeeObject *indexob, *index_value;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = DeeType_INVOKE_GETITEMINDEX(tp_self, self, i);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_indexob;
		}
		temp = (*proc)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateWithCounterAndForeachDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	struct default_enumerate_with_counter_and_foreach_data data;
	LOAD_TP_SELF;
	data.dewcaf_proc    = proc;
	data.dewcaf_arg     = arg;
	data.dewcaf_counter = 0;
	return DeeType_INVOKE_FOREACH(tp_self, self, &default_enumerate_with_counter_and_foreach_cb, &data);
}






/* tp_enumerate_index */
struct default_enumerate_index_with_enumerate_data {
	Dee_enumerate_index_t deiwe_proc;  /* [1..1] Underlying callback. */
	void                 *deiwe_arg;   /* [?..?] Cookie for `deiwe_proc' */
	size_t                deiwe_start; /* Enumeration start index */
	size_t                deiwe_end;   /* Enumeration end index */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_enumerate_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	size_t index;
	struct default_enumerate_index_with_enumerate_data *data;
	data = (struct default_enumerate_index_with_enumerate_data *)arg;
	if (DeeObject_AsSize(key, &index)) /* TODO: Handle overflow */
		goto err;
	if (index >= data->deiwe_start && index < data->deiwe_end)
		return (*data->deiwe_proc)(data->deiwe_arg, index, value);
	return 0;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithEnumerate,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	struct default_enumerate_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.deiwe_proc  = proc;
	data.deiwe_arg   = arg;
	data.deiwe_start = start;
	data.deiwe_end   = end;
	return DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_enumerate_index_with_enumerate_cb, &data);
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithSizeAndGetItemIndexFast,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = (*tp_self->tp_seq->tp_getitem_index_fast)(self, i);
		temp = (*proc)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithSizeAndTryGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(!index_value)
			goto err;
		if (index_value == ITER_DONE) {
			temp = (*proc)(arg, i, NULL);
		} else {
			temp = (*proc)(arg, i, index_value);
			Dee_Decref(index_value);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithSizeAndGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, i);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*proc)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithSizeObAndGetItem,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value, *sizeob;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	if (start != 0) {
		DREF DeeObject *new_indexob;
		new_indexob = DeeObject_AddSize(indexob, start);
		Dee_Decref(indexob);
		if unlikely(!new_indexob)
			goto err_sizeob;
		indexob = new_indexob;
	}
	if (end != (size_t)-1) {
		int size_is_greater_than_endhint;
		DREF DeeObject *endhintob;
		endhintob = DeeInt_NewSize(end);
		if unlikely(!endhintob)
			goto err_sizeob_indexob;
		size_is_greater_than_endhint = DeeObject_CmpGrAsBool(sizeob, endhintob);
		if (size_is_greater_than_endhint != 0) {
			Dee_Decref(sizeob);
			sizeob = endhintob;
			if unlikely(size_is_greater_than_endhint < 0)
				goto err_sizeob_indexob;
		}
	}
	for (;;) {
		size_t index;
		int index_is_less_than_size = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (index_is_less_than_size <= 0) {
			if unlikely(index_is_less_than_size < 0)
				goto err_sizeob_indexob;
			break;
		}
		index_value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_indexob;
		}
		if unlikely(DeeObject_AsSize(indexob, &index))
			goto err_sizeob_indexob;
		temp = (*proc)(arg, index, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_sizeob_indexob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_indexob:
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return temp;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}


#define default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP \
	(__SSIZE_MIN__ + 99) /* Shhht. We don't talk about this one... */

struct default_enumerate_index_with_counter_and_foreach_data {
	Dee_enumerate_index_t deiwcaf_proc;  /* [1..1] Wrapped callback */
	void                 *deiwcaf_arg;   /* [?..?] Cookie for `deiwcaf_proc' */
	size_t                deiwcaf_index; /* Index of the next element that will be enumerate_indexd */
	size_t                deiwcaf_start; /* Enumeration start index */
	size_t                deiwcaf_end;   /* Enumeration end index */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_index_with_counter_and_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_index_with_counter_and_foreach_cb(void *arg, DeeObject *elem) {
	size_t index;
	struct default_enumerate_index_with_counter_and_foreach_data *data;
	data = (struct default_enumerate_index_with_counter_and_foreach_data *)arg;
	if (data->deiwcaf_index >= data->deiwcaf_end)
		return default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP;
	index = data->deiwcaf_index++;
	if (index < data->deiwcaf_start)
		return 0; /* Skipped... */
	return (*data->deiwcaf_proc)(data->deiwcaf_arg, index, elem);
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithCounterAndForeach,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	struct default_enumerate_index_with_counter_and_foreach_data data;
	Dee_ssize_t result;
	LOAD_TP_SELF;
	data.deiwcaf_proc  = proc;
	data.deiwcaf_arg   = arg;
	data.deiwcaf_index = 0;
	data.deiwcaf_start = start;
	data.deiwcaf_end   = end;
	result = DeeType_INVOKE_FOREACH_NODEFAULT(tp_self, self, &default_enumerate_index_with_counter_and_foreach_cb, &data);
	if unlikely(result == default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP)
		result = 0;
	return result;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithCounterAndIter,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	size_t counter = 0;
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *iter, *elem;
	LOAD_TP_SELF;
	iter = DeeType_INVOKE_ITER_NODEFAULT(tp_self, self);
	if unlikely(!iter)
		goto err;
	if (start != 0) {
		counter = DeeObject_IterAdvance(iter, start);
		if unlikely(counter == (size_t)-1)
			goto err_iter;
		if unlikely(counter < start)
			goto done;
	}
	for (;; ++counter) {
		if (counter >= end)
			goto done;
		elem = DeeObject_IterNext(iter);
		if (!ITER_ISOK(elem))
			break;
		temp = (*proc)(arg, counter, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err_temp_iter;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	if unlikely(!elem)
		goto err_iter;
done:
	Dee_Decref(iter);
	return result;
err_temp_iter:
	Dee_Decref(iter);
/*err_temp:*/
	return temp;
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = DeeType_INVOKE_GETITEMINDEX(tp_self, self, i);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*proc)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithCounterAndForeachDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	struct default_enumerate_index_with_counter_and_foreach_data data;
	Dee_ssize_t result;
	LOAD_TP_SELF;
	data.deiwcaf_proc  = proc;
	data.deiwcaf_arg   = arg;
	data.deiwcaf_index = 0;
	data.deiwcaf_start = start;
	data.deiwcaf_end   = end;
	result = DeeType_INVOKE_FOREACH(tp_self, self, &default_enumerate_index_with_counter_and_foreach_cb, &data);
	if unlikely(result == default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP)
		result = 0;
	return result;
}

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithEnumerateDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end)) {
	struct default_enumerate_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.deiwe_proc  = proc;
	data.deiwe_arg   = arg;
	data.deiwe_start = start;
	data.deiwe_end   = end;
	return DeeType_INVOKE_ENUMERATE(tp_self, self, &default_enumerate_index_with_enumerate_cb, &data);
}







/* tp_sizeob */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultSizeObWithSize,
                         (DeeObject *RESTRICT_IF_NOTYPE self)) {
	size_t result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultSizeObWithSizeDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self)) {
	size_t result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


DEFINE_INTERNAL_OPERATOR(size_t, DefaultSizeWithSizeOb,
                         (DeeObject *RESTRICT_IF_NOTYPE self)) {
	int temp;
	size_t result;
	DREF DeeObject *sizeob;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_AsSize(sizeob, &result);
	Dee_Decref(sizeob);
	if unlikely(temp)
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)-1;
}

INTDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_size_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_size_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	(void)arg;
	(void)key;
	(void)value;
	return 1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(size_t, DefaultSizeWithEnumerateIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	return (size_t)DeeType_INVOKE_ENUMERATE_INDEX_NODEFAULT(tp_self, self, (Dee_enumerate_index_t)&default_size_with_foreach_pair_cb, NULL, 0, (size_t)-1);
}

DEFINE_INTERNAL_SEQ_OPERATOR(size_t, DefaultSizeWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	return (size_t)DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_size_with_foreach_pair_cb, NULL);
}

DEFINE_INTERNAL_SEQ_OPERATOR(size_t, DefaultSizeWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	return (size_t)DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_size_with_foreach_pair_cb, NULL);
}

INTDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_size_with_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_size_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return 1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(size_t, DefaultSizeWithForeach,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	return (size_t)DeeType_INVOKE_FOREACH_NODEFAULT(tp_self, self, &default_size_with_foreach_cb, NULL);
}

DEFINE_INTERNAL_SEQ_OPERATOR(size_t, DefaultSizeWithIter,
                             (DeeObject *RESTRICT_IF_NOTYPE self)) {
	size_t result = 0;
	DREF DeeObject *iter, *elem;
	LOAD_TP_SELF;
	iter = DeeType_INVOKE_ITER_NODEFAULT(tp_self, self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
		++result;
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return (size_t)-1;
}

/* tp_size_fast */
#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(size_t, DefaultSizeFastWithErrorNotFast,
                         (DeeObject *RESTRICT_IF_NOTYPE self)) {
	(void)self;
	return (size_t)-1;
}
#endif /* !DEFINE_TYPED_OPERATORS */


INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_contains_with_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_contains_with_foreach_cb(void *arg, DeeObject *elem) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	if (temp == 0)
		return -2;
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultContainsWithForeachDefault,
                             (DeeObject *self, DeeObject *elem)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_FOREACH(tp_self, self, &default_contains_with_foreach_cb, elem);
	if unlikely(status == -1)
		goto err;
	return_bool_(status != 0);
err:
	return NULL;
}


DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithHasItem,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithBoundItem,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithTryGetItem,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithGetItem,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithHasItemStringHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithHasItemStringLenHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithHasItemIndex,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithBoundItemStringHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithBoundItemStringLenHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithBoundItemIndex,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithTryGetItemStringHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithTryGetItemIndex,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithTryGetItemStringLenHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithGetItemStringHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithGetItemStringLenHash,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithGetItemIndex,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithEnumerate,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithEnumerateDefault,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}



DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithSizeAndGetItemIndexFast,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	size_t index_value, size;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index_value >= size)
		goto err_oob;
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index_value);
	if unlikely(!result)
		err_unbound_index(self, index_value);
	return result;
err_oob:
	err_index_out_of_bounds(self, index_value, size);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, index_value);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                  DeeString_STR(index),
	                                                  DeeString_Hash(index));
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                     DeeString_STR(index),
	                                                     DeeString_SIZE(index),
	                                                     DeeString_Hash(index));
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithTryGetItem,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, index);
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unknown_key(self, index);
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithTryGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	result = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unknown_key(self, index);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithTryGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	result = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                       DeeString_STR(index),
	                                                       DeeString_Hash(index));
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unknown_key(self, index);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithTryGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	result = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                          DeeString_STR(index),
	                                                          DeeString_SIZE(index),
	                                                          DeeString_Hash(index));
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unknown_key(self, index);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemWithGetItemIndexDefault,
                         (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_GETITEMINDEX(tp_self, self, index_value);
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemWithTryGetItemAndSizeOb,
                             (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, index);
	if (result == ITER_DONE) {
		int temp;
		DREF DeeObject *sizeob;
		sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_CmpGeAsBool(index, sizeob);
		if likely(temp >= 0) {
			if (temp) {
				err_index_out_of_bounds_ob_x(self, index, sizeob);
			} else {
				err_unbound_index_ob(self, index);
			}
		}
		Dee_Decref(sizeob);
		goto err;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemWithTryGetItemAndSize,
                             (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, index);
	if (result == ITER_DONE) {
		size_t size, index_value;
		size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		if unlikely(DeeObject_AsSize(index, &index_value))
			goto err;
		if (index_value >= size) {
			err_index_out_of_bounds(self, index_value, size);
		} else {
			err_unbound_index_ob(self, index);
		}
		goto err;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemWithSizeAndTryGetItemIndexOb,
                             (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if unlikely(DeeObject_AsSize(index, &index_value))
		goto err;
	result = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if (result == ITER_DONE) {
		int temp;
		size_t size;
		DREF DeeObject *sizeob;
		sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_AsSize(sizeob, &size);
		Dee_Decref(sizeob);
		if unlikely(temp)
			goto err;
		if (index_value >= size) {
			err_index_out_of_bounds(self, index_value, size);
		} else {
			err_unbound_index_ob(self, index);
		}
		goto err;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemWithSizeAndTryGetItemIndex,
                             (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if unlikely(DeeObject_AsSize(index, &index_value))
		goto err;
	result = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if (result == ITER_DONE) {
		size_t size;
		size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (index_value >= size) {
			err_index_out_of_bounds(self, index_value, size);
		} else {
			err_unbound_index_ob(self, index);
		}
		goto err;
	}
	return result;
err:
	return NULL;
}



struct default_map_getitem_with_enumerate_data {
	DeeObject      *mgied_key;    /* [1..1] The key we're looking for. */
	DREF DeeObject *mgied_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	struct default_map_getitem_with_enumerate_data *data;
	data = (struct default_map_getitem_with_enumerate_data *)arg;
	temp = DeeObject_TryCompareEq(data->mgied_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgied_result = value;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemWithEnumerate,
                             (DeeObject *self, DeeObject *key)) {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgied_key = key;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if unlikely(status == -3) {
		err_unbound_key(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key(self, key);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemWithEnumerateDefault,
                             (DeeObject *self, DeeObject *key)) {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgied_key = key;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if unlikely(status == -3) {
		err_unbound_key(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key(self, key);
err:
	return NULL;
}


DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithSizeAndGetItemIndexFast,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index);
	if unlikely(!result)
		err_unbound_index(self, index);
	return result;
err_oob:
	err_index_out_of_bounds(self, index, size);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithTryGetItemIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEMINDEX(tp_self, self, index);
	if unlikely(result == ITER_DONE) {
		err_unbound_index(self, index);
		result = NULL;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, indexob);
	if unlikely(result == ITER_DONE) {
		err_unknown_key(self, indexob);
		result = NULL;
	}
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithErrorRequiresString,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *indexob;
	(void)self;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	DeeObject_TypeAssertFailed(indexob, &DeeString_Type);
	Dee_Decref(indexob);
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithGetItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_GETITEM(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}

struct default_getitem_index_with_foreach_data {
	DREF DeeObject *dgiiwfd_result; /* [?..1][out] Item lookup result */
	size_t          dgiiwfd_nskip;  /* Number of indices left to skip. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_getitem_index_with_foreach_data *data;
	data = (struct default_getitem_index_with_foreach_data *)arg;
	if (data->dgiiwfd_nskip == 0) {
		data->dgiiwfd_result = elem;
		Dee_Incref(elem);
		return -2; /* Stop enumeration */
	}
	--data->dgiiwfd_nskip;
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithSizeAndTryGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index);
	if unlikely(result == ITER_DONE) {
		size_t size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (index >= size) {
			err_index_out_of_bounds(self, index, size);
		} else {
			err_unbound_index(self, index);
		}
		goto err;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithSizeAndTryGetItemIndexOb,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index);
	if unlikely(result == ITER_DONE) {
		int temp;
		size_t size;
		DREF DeeObject *sizeob;
		sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_AsSize(sizeob, &size);
		Dee_Decref(sizeob);
		if unlikely(temp)
			goto err;
		if (index >= size) {
			err_index_out_of_bounds(self, index, size);
		} else {
			err_unbound_index(self, index);
		}
		goto err;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithTryGetItemAndSize,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result, *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	if unlikely(result == ITER_DONE) {
		size_t size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (index >= size) {
			err_index_out_of_bounds(self, index, size);
		} else {
			err_unbound_index(self, index);
		}
		goto err;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithTryGetItemAndSizeOb,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result, *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	if unlikely(result == ITER_DONE) {
		int temp;
		size_t size;
		DREF DeeObject *sizeob;
		sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_AsSize(sizeob, &size);
		Dee_Decref(sizeob);
		if unlikely(temp)
			goto err;
		if (index >= size) {
			err_index_out_of_bounds(self, index, size);
		} else {
			err_unbound_index(self, index);
		}
		goto err;
	}
	return result;
err:
	return NULL;
}


DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithForeachDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	struct default_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	LOAD_TP_SELF;
	data.dgiiwfd_nskip = index;
	status = DeeType_INVOKE_FOREACH(tp_self, self, &default_getitem_index_with_foreach_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	return data.dgiiwfd_result;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, index - data.dgiiwfd_nskip);
err:
	return NULL;
}

struct default_map_getitem_index_with_enumerate_data {
	size_t          mgiied_key;    /* The index we're looking for. */
	DREF DeeObject *mgiied_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
/* @return: 1 : Not-equal
 * @return: 0 : Equal
 * @return: -1: Not-equal
 * @return: Dee_COMPARE_ERR: Error */
PRIVATE WUNUSED NONNULL((2)) int DCALL
size_t_equals_object(size_t lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *lhs_value;
	if (DeeInt_Check(rhs)) {
		size_t rhs_value;
		return (DeeInt_TryAsSize(rhs, &rhs_value) && lhs == rhs_value) ? 0 : 1;
	}
	lhs_value = DeeInt_NewSize(lhs);
	if unlikely(!lhs_value)
		goto err;
	result = DeeObject_TryCompareEq(lhs_value, rhs);
	Dee_Decref(lhs_value);
	return result;
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	struct default_map_getitem_index_with_enumerate_data *data;
	data = (struct default_map_getitem_index_with_enumerate_data *)arg;
	temp = size_t_equals_object(data->mgiied_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgiied_result = value;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	struct default_map_getitem_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgiied_key = index;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_index_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgiied_result;
	if unlikely(status == -3) {
		err_unbound_index(self, index);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_int(self, index);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	struct default_map_getitem_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgiied_key = index;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_index_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgiied_result;
	if unlikely(status == -3) {
		err_unbound_index(self, index);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_int(self, index);
err:
	return NULL;
}




/* tp_getitem_string_hash */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithTryGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str(self, key);
		result = NULL;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithTryGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str(self, key);
		result = NULL;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str(self, key);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithGetItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_GETITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

struct default_map_getitem_string_hash_with_enumerate_data {
	char const     *mgished_key;    /* [1..1] The key we're looking for. */
	Dee_hash_t      mgished_hash;   /* Hash for `mgished_key'. */
	DREF DeeObject *mgished_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
PRIVATE WUNUSED NONNULL((1, 3)) bool DCALL
string_hash_equals_object(char const *lhs, Dee_hash_t lhs_hash, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return (DeeString_Hash(rhs) == lhs_hash && strcmp(lhs, DeeString_STR(rhs)) == 0);
	if (DeeBytes_Check(rhs))
		return (strlen(lhs) == DeeBytes_SIZE(rhs) && bcmp(lhs, DeeBytes_DATA(rhs), DeeBytes_SIZE(rhs)) == 0);
	/* `string.operator ==' isn't implemented for any other types. */
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_getitem_string_hash_with_enumerate_data *data;
	data = (struct default_map_getitem_string_hash_with_enumerate_data *)arg;
	if (string_hash_equals_object(data->mgished_key, data->mgished_hash, key)) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgished_result = value;
		return -2;
	}
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	if unlikely(status == -3) {
		err_unbound_key_str(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str(self, key);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	if unlikely(status == -3) {
		err_unbound_key_str(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str(self, key);
err:
	return NULL;
}

#if defined(__ARCH_PAGESIZE) && !defined(__OPTIMIZE_SIZE__)
#define is_nulterm_string(str, len)                              \
	((len) > 0 &&                                                \
	 (((uintptr_t)((str) + (len)-1) & ~(__ARCH_PAGESIZE - 1)) == \
	  ((uintptr_t)((str) + (len)) & ~(__ARCH_PAGESIZE - 1))) &&  \
	 (str)[len] == '\0')
#define LOCAL_WITH_ZSTRING_COPY(err_label, copy_varname, str, len, ...)     \
	do {                                                                    \
		char *copy_varname;                                                 \
		if (is_nulterm_string(str, len)) {                                  \
			copy_varname = (char *)(str);                                   \
			__VA_ARGS__;                                                    \
		} else {                                                            \
			copy_varname = (char *)Dee_Mallocac((len + 1), sizeof(char));   \
			if unlikely(!copy_varname)                                      \
				goto err_label;                                             \
			*(char *)mempcpyc(copy_varname, str, len, sizeof(char)) = '\0'; \
			__VA_ARGS__;                                                    \
			Dee_Freea(copy_varname);                                        \
		}                                                                   \
	}	__WHILE0
#else /* __ARCH_PAGESIZE && !__OPTIMIZE_SIZE__ */
#define LOCAL_WITH_ZSTRING_COPY(err_label, copy_varname, str, len, ...) \
	do {                                                                \
		char *copy_varname;                                             \
		copy_varname = (char *)Dee_Mallocac((len + 1), sizeof(char));   \
		if unlikely(!copy_varname)                                      \
			goto err_label;                                             \
		*(char *)mempcpyc(copy_varname, str, len, sizeof(char)) = '\0'; \
		__VA_ARGS__;                                                    \
		Dee_Freea(copy_varname);                                        \
	}	__WHILE0
#endif /* !__ARCH_PAGESIZE || __OPTIMIZE_SIZE__ */

/* tp_getitem_string_len_hash */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithTryGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str(self, key);
		result = NULL;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithTryGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str(self, key);
err:
		result = NULL;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str(self, key);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithGetItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_GETITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

struct default_map_getitem_string_len_hash_with_enumerate_data {
	char const     *mgislhed_key;    /* [1..1] The key we're looking for. */
	size_t          mgislhed_keylen; /* Length of `mgislhed_key'. */
	Dee_hash_t      mgislhed_hash;   /* Hash for `mgislhed_key'. */
	DREF DeeObject *mgislhed_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
PRIVATE WUNUSED NONNULL((1, 4)) bool DCALL
string_len_hash_equals_object(char const *lhs, size_t lhs_len, Dee_hash_t lhs_hash, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return (DeeString_Hash(rhs) == lhs_hash && DeeString_EqualsBuf(rhs, lhs, lhs_len));
	if (DeeBytes_Check(rhs))
		return (lhs_len == DeeBytes_SIZE(rhs) && bcmp(lhs, DeeBytes_DATA(rhs), lhs_len) == 0);
	/* `string.operator ==' isn't implemented for any other types. */
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_getitem_string_len_hash_with_enumerate_data *data;
	data = (struct default_map_getitem_string_len_hash_with_enumerate_data *)arg;
	if (string_len_hash_equals_object(data->mgislhed_key, data->mgislhed_keylen, data->mgislhed_hash, key)) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgislhed_result = value;
		return -2;
	}
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if unlikely(status == -3) {
		err_unbound_key_str_len(self, key, keylen);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str_len(self, key, keylen);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if unlikely(status == -3) {
		err_unbound_key_str_len(self, key, keylen);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str_len(self, key, keylen);
err:
	return NULL;
}



/* tp_trygetitem */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithTryGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithTryGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                     DeeString_STR(index),
	                                                     DeeString_Hash(index));
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithTryGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                        DeeString_STR(index),
	                                                        DeeString_SIZE(index),
	                                                        DeeString_Hash(index));
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithSizeAndGetItemIndexFast,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	size_t index_value, size;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (index_value >= size)
		return ITER_DONE;
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index_value);
	if (result == NULL)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	result = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithGetItem,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	result = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                       DeeString_STR(index),
	                                                       DeeString_Hash(index));
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	result = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                          DeeString_STR(index),
	                                                          DeeString_SIZE(index),
	                                                          DeeString_Hash(index));
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithGetItemDefault,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_GETITEM(tp_self, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, DeeObject *index)) {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgied_key = index;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	ASSERT(status == -1);
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, DeeObject *index)) {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgied_key = index;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	ASSERT(status == -1);
	return NULL;
}


/* tp_trygetitem_index */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithSizeAndGetItemIndexFast,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return ITER_DONE;
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index);
	if (result == NULL)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result, *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithGetItemIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result, *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithErrorRequiresString,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *indexob;
	(void)self;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	DeeObject_TypeAssertFailed(indexob, &DeeString_Type);
	Dee_Decref(indexob);
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithGetItemIndexDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_GETITEMINDEX(tp_self, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	struct default_map_getitem_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgiied_key = index;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_index_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgiied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	struct default_map_getitem_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgiied_key = index;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_index_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgiied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	return NULL;
}


/* tp_trygetitem_string_hash */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithTryGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	if (!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
	if (!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithTryGetItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}


/* tp_trygetitem_string_len_hash */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithTryGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	if (!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if (!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (!result) {
		if (DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithTryGetItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_TRYGETITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	return NULL;
}



DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemWithDelItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_DELITEMINDEX_NODEFAULT(tp_self, self, index_value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemWithDelItemIndexDefault,
                         (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_DELITEMINDEX(tp_self, self, index_value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemWithDelItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_DELITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                  DeeString_STR(index),
	                                                  DeeString_Hash(index));
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemWithDelItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_DELITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                     DeeString_STR(index),
	                                                     DeeString_SIZE(index),
	                                                     DeeString_Hash(index));
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemIndexWithDelItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_DELITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemIndexWithErrorRequiresString,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *indexob;
	(void)self;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	DeeObject_TypeAssertFailed(indexob, &DeeString_Type);
	Dee_Decref(indexob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelItemIndexWithDelRangeIndexDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_DELRANGEINDEX(tp_self, self, index, index + 1);
}


/* tp_delitem_string_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemStringHashWithDelItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_DELITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemStringHashWithDelItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_DELITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemStringHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

/* tp_delitem_string_len_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemStringLenHashWithDelItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_DELITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemStringLenHashWithDelItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_DELITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultDelItemStringLenHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemWithSetItemIndex,
                         (DeeObject *self, DeeObject *index, DeeObject *value)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_SETITEMINDEX_NODEFAULT(tp_self, self, index_value, value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemWithSetItemIndexDefault,
                         (DeeObject *self, DeeObject *index, DeeObject *value)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_SETITEMINDEX(tp_self, self, index_value, value);
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemWithSetItemStringHash,
                         (DeeObject *self, DeeObject *index, DeeObject *value)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_SETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                  DeeString_STR(index),
	                                                  DeeString_Hash(index),
	                                                  value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemWithSetItemStringLenHash,
                         (DeeObject *self, DeeObject *index, DeeObject *value)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_SETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                     DeeString_STR(index),
	                                                     DeeString_SIZE(index),
	                                                     DeeString_Hash(index),
	                                                     value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemIndexWithSetItem,
                         (DeeObject *self, size_t index, DeeObject *value)) {
	int result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_SETITEM_NODEFAULT(tp_self, self, indexob, value);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemIndexWithErrorRequiresString,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index, DeeObject *value)) {
	DREF DeeObject *indexob;
	(void)self;
	(void)value;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	DeeObject_TypeAssertFailed(indexob, &DeeString_Type);
	Dee_Decref(indexob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetItemIndexWithSetRangeIndexDefault,
                             (DeeObject *self, size_t index, DeeObject *value)) {
	int result;
	DREF DeeObject *seq1;
	LOAD_TP_SELF;
	seq1 = DeeTuple_NewVectorSymbolic(1, &value);
	if unlikely(!seq1)
		goto err;
	result = DeeType_INVOKE_SETRANGEINDEX(tp_self, self, index, index + 1, seq1);
	DeeTuple_DecrefSymbolic(seq1);
	return result;
err:
	return -1;
}

/* tp_setitem_string_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemStringHashWithSetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash, DeeObject *value)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_SETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash, value);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemStringHashWithSetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash, DeeObject *value)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_SETITEM_NODEFAULT(tp_self, self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemStringHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash, DeeObject *value)) {
	DREF DeeObject *keyob;
	(void)self;
	(void)value;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

/* tp_setitem_string_len_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemStringLenHashWithSetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value)) {
	int result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_SETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash, value)
	));
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemStringLenHashWithSetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_SETITEM_NODEFAULT(tp_self, self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultSetItemStringLenHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value)) {
	DREF DeeObject *keyob;
	(void)self;
	(void)value;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithBoundItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_BOUNDITEMINDEX_NODEFAULT(tp_self, self, index_value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithBoundItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_BOUNDITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                    DeeString_STR(index),
	                                                    DeeString_Hash(index));
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithBoundItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_BOUNDITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                       DeeString_STR(index),
	                                                       DeeString_SIZE(index),
	                                                       DeeString_Hash(index));
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                   DeeString_STR(index),
	                                                   DeeString_Hash(index));
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                      DeeString_STR(index),
	                                                      DeeString_SIZE(index),
	                                                      DeeString_Hash(index));
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithGetItem,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return -2;
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *item_value;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	item_value = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItemAndHasItem,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEM_NODEFAULT(tp_self, self, index);
	return result ? result : -2;
err:
	return -1;
}



DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItemIndexAndHasItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	DREF DeeObject *value;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEMINDEX_NODEFAULT(tp_self, self, index_value);
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                         DeeString_STR(index),
	                                                         DeeString_SIZE(index),
	                                                         DeeString_Hash(index));
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                       DeeString_STR(index),
	                                                       DeeString_SIZE(index),
	                                                       DeeString_HASH(index));
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                      DeeString_STR(index),
	                                                      DeeString_Hash(index));
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                    DeeString_STR(index),
	                                                    DeeString_HASH(index));
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoundItemWithTryGetItemAndSizeOb,
                             (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE) {
		int temp;
		DREF DeeObject *sizeob;
		sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_CmpLoAsBool(index, sizeob);
		Dee_Decref(sizeob);
		if unlikely(temp < 0)
			goto err;
		return temp ? 0 : -2;
	}
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoundItemWithSizeAndTryGetItemIndex,
                             (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE) {
		size_t size;
		size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		return index_value < size ? 0 : -2;
	}
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItem,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return -2;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return -2;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                         DeeString_STR(index),
	                                                         DeeString_SIZE(index),
	                                                         DeeString_Hash(index));
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return -2;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithTryGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                      DeeString_STR(index),
	                                                      DeeString_Hash(index));
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return -2;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemWithGetItemDefault,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEM(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return -2;
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemWithContains,
                             (DeeObject *self, DeeObject *index)) {
	int result_status;
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_CONTAINS_NODEFAULT(tp_self, self, index);
	if unlikely(!result)
		goto err;
	result_status = DeeObject_BoolInherited(result);
	if (result_status == 0)
		result_status = -2;
	return result_status;
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	(void)value;
	temp = DeeObject_TryCompareEq((DeeObject *)arg, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0)
		return value ? -2 : -3; /* Stop iteration */
	return 0;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemWithEnumerate,
                             (DeeObject *self, DeeObject *index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_with_enumerate_cb, index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemWithEnumerateDefault,
                             (DeeObject *self, DeeObject *index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_with_enumerate_cb, index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}




DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithBoundItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithSizeAndGetItemIndexFast,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return -2; /* Index does not exist. */
	result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, index);
	if (!result)
		return 0; /* Index isn't bound */
	Dee_Decref(result);
	return 1;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithGetItemIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return -2;
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	item_value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithGetItemIndexDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEMINDEX(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return -2;
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result;
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEMINDEX_NODEFAULT(tp_self, self, index);
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithTryGetItemAndHasItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result;
	DREF DeeObject *value;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, indexob);
	if unlikely(!value)
		goto err_indexob;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		Dee_Decref(indexob);
		return 1;
	}
	result = DeeType_INVOKE_HASITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result ? result : -2;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoundItemIndexWithSizeAndTryGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE) {
		size_t size;
		size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		return index < size ? 0 : -2;
	}
	Dee_Decref(value);
	return 1;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultBoundItemIndexWithTryGetItemAndSizeOb,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *value;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE) {
		int temp;
		size_t size;
		DREF DeeObject *sizeob;
		sizeob = DeeType_INVOKE_SIZEOB_NODEFAULT(tp_self, self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_AsSize(sizeob, &size);
		Dee_Decref(sizeob);
		if unlikely(temp < 0)
			goto err;
		return index < size ? 0 : -2;
	}
	Dee_Decref(value);
	return 1;
err:
	return -1;
}


#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemIndexWithErrorRequiresString,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *indexob;
	(void)self;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	DeeObject_TypeAssertFailed(indexob, &DeeString_Type);
	Dee_Decref(indexob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemIndexWithContains,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result_status;
	DREF DeeObject *result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_CONTAINS_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	result_status = DeeObject_BoolInherited(result);
	if (result_status == 0)
		result_status = -2;
	return result_status;
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	(void)value;
	temp = size_t_equals_object((size_t)(uintptr_t)arg, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0)
		return value ? -2 : -3;
	return 0;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemIndexWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_index_with_enumerate_cb, (void *)(uintptr_t)index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemIndexWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_index_with_enumerate_cb, (void *)(uintptr_t)index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}


/* tp_bounditem_string_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithBoundItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_BOUNDITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithBoundItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *value;
	size_t keylen = strlen(key);
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithTryGetItemAndHasItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *value, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	if unlikely(!value)
		goto err_keyob;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		Dee_Decref(keyob);
		return 1;
	}
	result = DeeType_INVOKE_HASITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result ? result : -2;
err_keyob:
	Dee_Decref(keyob);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithTryGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	if (value) {
		if (value == ITER_DONE)
			return -2;
		Dee_Decref(value);
		return 1;
	}
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithTryGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
	if (value) {
		if (value == ITER_DONE)
			return -2;
		Dee_Decref(value);
		return 1;
	}
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		if (value == ITER_DONE)
			return -2;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithBoundItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringHashWithContains,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result_status;
	DREF DeeObject *result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_CONTAINS_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	result_status = DeeObject_BoolInherited(result);
	if (result_status == 0)
		result_status = -2;
	return result_status;
err:
	return -1;
}

struct default_map_bounditem_string_hash_with_enumerate_data {
	char const *mbished_key;    /* [1..1] The key we're looking for. */
	Dee_hash_t  mbished_hash;   /* Hash for `mbished_key'. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_bounditem_string_hash_with_enumerate_data *data;
	(void)value;
	data = (struct default_map_bounditem_string_hash_with_enumerate_data *)arg;
	if (string_hash_equals_object(data->mbished_key, data->mbished_hash, key))
		return value ? -2 : -3;
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbished_key  = key;
	data.mbished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_string_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbished_key  = key;
	data.mbished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_string_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}



/* tp_bounditem_string_len_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithBoundItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_BOUNDITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithBoundItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		value = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_KeyError))
		return -2;
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	result = DeeType_INVOKE_HASITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *value;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return 1;
	}
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_HASITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	return result ? result : -2;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithTryGetItemAndHasItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *value, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	if unlikely(!value)
		goto err_keyob;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		Dee_Decref(keyob);
		return 1;
	}
	result = DeeType_INVOKE_HASITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result ? result : -2;
err_keyob:
	Dee_Decref(keyob);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithTryGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if (value) {
		if (value == ITER_DONE)
			return -2;
		Dee_Decref(value);
		return 1;
	}
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithTryGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash)
	));
	if (value) {
		if (value == ITER_DONE)
			return -2;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value, *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		if (value == ITER_DONE)
			return -2;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithBoundItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultBoundItemStringLenHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringLenHashWithContains,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result_status;
	DREF DeeObject *result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_CONTAINS_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	result_status = DeeObject_BoolInherited(result);
	if (result_status == 0)
		result_status = -2;
	return result_status;
err:
	return -1;
}

struct default_map_bounditem_string_len_hash_with_enumerate_data {
	char const *mbislhed_key;    /* [1..1] The key we're looking for. */
	size_t      mbislhed_keylen; /* Length of `mbislhed_key'. */
	Dee_hash_t  mbislhed_hash;   /* Hash for `mbislhed_key'. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_bounditem_string_len_hash_with_enumerate_data *data;
	(void)value;
	data = (struct default_map_bounditem_string_len_hash_with_enumerate_data *)arg;
	if (string_len_hash_equals_object(data->mbislhed_key, data->mbislhed_keylen, data->mbislhed_hash, key))
		return value ? -2 : -3;
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringLenHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbislhed_key    = key;
	data.mbislhed_keylen = keylen;
	data.mbislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_string_len_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringLenHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbislhed_key    = key;
	data.mbislhed_keylen = keylen;
	data.mbislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_string_len_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}




DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithHasItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeType_INVOKE_HASITEMINDEX_NODEFAULT(tp_self, self, index_value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithHasItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertType(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_HASITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                  DeeString_STR(index),
	                                                  DeeString_Hash(index));
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithHasItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if (DeeObject_AssertType(index, &DeeString_Type))
		goto err;
	return DeeType_INVOKE_HASITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                     DeeString_STR(index),
	                                                     DeeString_SIZE(index),
	                                                     DeeString_Hash(index));
err:
	return -1;
}

LOCAL ATTR_CONST int DCALL bound2has(int return_status) {
	if (return_status == 0)
		return 1; /* Unbound, but exists */
	if (return_status == -2)
		return 0; /* Not bound */
	ASSERT(return_status == 1 || return_status == -1);
	return return_status; /* Bound or error */
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithBoundItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_BOUNDITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                      DeeString_STR(index),
	                                                      DeeString_Hash(index));
	return bound2has(result);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithBoundItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_BOUNDITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                         DeeString_STR(index),
	                                                         DeeString_SIZE(index),
	                                                         DeeString_Hash(index));
	return bound2has(result);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithTryGetItem,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, index);
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithTryGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertType(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                      DeeString_STR(index),
	                                                      DeeString_Hash(index));
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithTryGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertType(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                         DeeString_STR(index),
	                                                         DeeString_SIZE(index),
	                                                         DeeString_Hash(index));
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithTryGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithGetItemStringHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertType(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self,
	                                                   DeeString_STR(index),
	                                                   DeeString_Hash(index));
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError) ||
	    DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_UnboundItem))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithGetItemStringLenHash,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	if (DeeObject_AssertType(index, &DeeString_Type))
		goto err;
	value = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self,
	                                                      DeeString_STR(index),
	                                                      DeeString_SIZE(index),
	                                                      DeeString_Hash(index));
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError) ||
	    DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_UnboundItem))
		return 0;
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithBoundItem,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_BOUNDITEM_NODEFAULT(tp_self, self, index);
	return bound2has(result);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithBoundItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	int result;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	result = DeeType_INVOKE_BOUNDITEMINDEX_NODEFAULT(tp_self, self, index_value);
	return bound2has(result);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithGetItem,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1; /* Unbound, but present */
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0; /* Item does not exist */
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithGetItemIndex,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *item_value;
	size_t index_value;
	LOAD_TP_SELF;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	item_value = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, index_value);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1; /* Unbound, but present */
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0; /* Item does not exist */
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemWithGetItemDefault,
                         (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEM(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1; /* Unbound, but present */
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0; /* Item does not exist */
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultHasItemWithSize,
                             (DeeObject *self, DeeObject *index)) {
	size_t index_value, size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(DeeObject_AsSize(index, &index_value))
		goto err;
	return index_value < size ? 1 : 0;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultHasItemWithSizeOb,
                             (DeeObject *self, DeeObject *index)) {
	int result;
	DREF DeeObject *sizeob;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB(tp_self, self);
	if unlikely(!sizeob)
		goto err;
	result = DeeObject_CmpLoAsBool(index, sizeob);
	Dee_Decref(sizeob);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemWithContains,
                             (DeeObject *self, DeeObject *index)) {
	int result_status;
	DREF DeeObject *result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_CONTAINS(tp_self, self, index);
	if unlikely(!result)
		goto err;
	result_status = DeeObject_BoolInherited(result);
	if (result_status == 0)
		result_status = -2;
	return result_status;
err:
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemWithEnumerate,
                             (DeeObject *self, DeeObject *index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_with_enumerate_cb, index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemWithEnumerateDefault,
                             (DeeObject *self, DeeObject *index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_with_enumerate_cb, index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithHasItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_HASITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithBoundItemIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_BOUNDITEMINDEX_NODEFAULT(tp_self, self, index);
	return bound2has(result);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithBoundItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	int result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return bound2has(result);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithTryGetItemIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_TRYGETITEMINDEX_NODEFAULT(tp_self, self, index);
	if (item_value) {
		if (item_value == ITER_DONE)
			return 0;
		Dee_Decref(item_value);
		return 1;
	}
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	item_value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	if (item_value) {
		if (item_value == ITER_DONE)
			return 0;
		Dee_Decref(item_value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithGetItemIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEMINDEX_NODEFAULT(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1; /* Unbound, but present */
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0; /* Item does not exist */
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	item_value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1; /* Unbound, but present */
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0; /* Item does not exist */
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithGetItemIndexDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *item_value;
	LOAD_TP_SELF;
	item_value = DeeType_INVOKE_GETITEMINDEX(tp_self, self, index);
	if (item_value) {
		Dee_Decref(item_value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1; /* Unbound, but present */
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0; /* Item does not exist */
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemIndexWithErrorRequiresString,
                         (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *indexob;
	(void)self;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	DeeObject_TypeAssertFailed(indexob, &DeeString_Type);
	Dee_Decref(indexob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultHasItemIndexWithSize,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return index < size ? 1 : 0;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultHasItemIndexWithSizeDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return index < size ? 1 : 0;
err:
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemIndexWithContains,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	DREF DeeObject *indexob;
	LOAD_TP_SELF;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = DeeType_INVOKE_CONTAINS_NODEFAULT(tp_self, self, indexob);
	Dee_Decref(indexob);
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemIndexWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_index_with_enumerate_cb, (void *)(uintptr_t)index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemIndexWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_index_with_enumerate_cb, (void *)(uintptr_t)index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}


/* tp_hasitem_string_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithHasItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_HASITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithBoundItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_BOUNDITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	return bound2has(result);
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithBoundItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_BOUNDITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
	return bound2has(result);
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithTryGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithTryGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self, key, hash);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, strlen(key), hash);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithHasItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_HASITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithBoundItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return bound2has(result);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *value;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithHasItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_HASITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringHashWithContains,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewAutoWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_CONTAINS_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbished_key  = key;
	data.mbished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_string_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbished_key  = key;
	data.mbished_hash = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_string_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}


/* tp_hasitem_string_len_hash */
DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithHasItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_HASITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithBoundItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		result = DeeType_INVOKE_BOUNDITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	return bound2has(result);
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithBoundItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	LOAD_TP_SELF;
	result = DeeType_INVOKE_BOUNDITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	return bound2has(result);
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithTryGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		value = DeeType_INVOKE_TRYGETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithTryGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_TRYGETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithGetItemStringHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	LOCAL_WITH_ZSTRING_COPY(err, keyz, key, keylen, (
		value = DeeType_INVOKE_GETITEMSTRINGHASH_NODEFAULT(tp_self, self, keyz, hash)
	));
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0;
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithGetItemStringLenHash,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	LOAD_TP_SELF;
	value = DeeType_INVOKE_GETITEMSTRINGLENHASH_NODEFAULT(tp_self, self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithHasItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_HASITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithBoundItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_BOUNDITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return bound2has(result);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithTryGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_TRYGETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		if (value == ITER_DONE)
			return 0;
		Dee_Decref(value);
		return 1;
	}
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithGetItem,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *value;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	value = DeeType_INVOKE_GETITEM_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return 1;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return 0;
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithHasItemDefault,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	int result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_HASITEM(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_INTERNAL_OPERATOR(int, DefaultHasItemStringLenHashWithErrorRequiresInt,
                         (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *keyob;
	(void)self;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	DeeObject_TypeAssertFailed(keyob, &DeeInt_Type);
	Dee_Decref(keyob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringLenHashWithContains,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeType_INVOKE_CONTAINS_NODEFAULT(tp_self, self, keyob);
	Dee_Decref(keyob);
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringLenHashWithEnumerate,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbislhed_key    = key;
	data.mbislhed_keylen = keylen;
	data.mbislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE_NODEFAULT(tp_self, self, &default_map_bounditem_string_len_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringLenHashWithEnumerateDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbislhed_key    = key;
	data.mbislhed_keylen = keylen;
	data.mbislhed_hash   = hash;
	status = DeeType_INVOKE_ENUMERATE(tp_self, self, &default_map_bounditem_string_len_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}


DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN,
                         (DeeObject *self, DeeObject *start, DeeObject *end)) {
	Dee_ssize_t start_index, end_index;
	LOAD_TP_SELF;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeType_INVOKE_GETRANGEINDEXN_NODEFAULT(tp_self, self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeType_INVOKE_GETRANGEINDEX_NODEFAULT(tp_self, self, start_index, end_index);
err:
	return NULL;
}


DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault,
                         (DeeObject *self, DeeObject *start, DeeObject *end)) {
	Dee_ssize_t start_index, end_index;
	LOAD_TP_SELF;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeType_INVOKE_GETRANGEINDEXN(tp_self, self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start_index, end_index);
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeWithSizeDefaultAndGetItemIndex,
                             (DeeObject *self, DeeObject *start, DeeObject *end)) {
	size_t size;
	Dee_ssize_t start_index;
	struct Dee_seq_range range;
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end)) {
		range.sr_start = DeeSeqRange_Clamp_n(start_index, size);
		range.sr_end   = size;
	} else {
		Dee_ssize_t end_index;
		if (DeeObject_AsSSize(end, &end_index))
			goto err;
		DeeSeqRange_Clamp(&range, start_index, end_index, size);
	}
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index;
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
	__builtin_unreachable();
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeWithSizeDefaultAndTryGetItemIndex,
                             (DeeObject *self, DeeObject *start, DeeObject *end)) {
	size_t size;
	Dee_ssize_t start_index;
	struct Dee_seq_range range;
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end)) {
		range.sr_start = DeeSeqRange_Clamp_n(start_index, size);
		range.sr_end   = size;
	} else {
		Dee_ssize_t end_index;
		if (DeeObject_AsSSize(end, &end_index))
			goto err;
		DeeSeqRange_Clamp(&range, start_index, end_index, size);
	}
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = tp_self->tp_seq->tp_trygetitem_index;
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
	__builtin_unreachable();
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeWithSizeObAndGetItem,
                             (DeeObject *self, DeeObject *start, DeeObject *end)) {
	int temp;
	DREF DeeObject *startob_and_endob[2];
	DREF DeeObject *startob_and_endob_tuple;
	DREF DeeObject *sizeob;
	LOAD_TP_SELF;
	sizeob = DeeType_INVOKE_SIZEOB(tp_self, self);
	if unlikely(!sizeob)
		goto err;
	/* Make a call to "util.clamprange()" to do the range-fixup. */
	startob_and_endob_tuple = DeeModule_CallExternStringf("util", "clamprange", "ooo", start, end, sizeob);
	Dee_Decref(sizeob);
	if unlikely(!startob_and_endob_tuple)
		goto err;
	temp = DeeObject_Unpack(startob_and_endob_tuple, 2, startob_and_endob);
	Dee_Decref(startob_and_endob_tuple);
	if unlikely(temp)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	if (!Dee_TYPE(self)->tp_seq ||
	    (Dee_TYPE(self)->tp_seq->tp_getitem != tp_self->tp_seq->tp_getitem)) {
		DREF DeeObject *(DCALL *tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
		tp_tgetitem = tp_self->tp_seq->tp_getitem == &instance_getitem
		              ? &instance_tgetitem
		              : NULL/*DeeType_MapDefaultGetItem(tp_self->tp_seq->tp_getitem, &, NULL)*/;
		if (tp_tgetitem) {
			DREF DefaultSequence_WithTSizeAndGetItem *result;
			result = DeeObject_MALLOC(DefaultSequence_WithTSizeAndGetItem);
			if unlikely(!result)
				goto err;
			result->dstsg_start = startob_and_endob[0]; /* Inherit reference */
			result->dstsg_end   = startob_and_endob[1]; /* Inherit reference */
			result->dstsg_tp_seq = tp_self;
			Dee_Incref(self);
			result->dstsg_seq         = self;
			result->dstsg_tp_tgetitem = tp_tgetitem;
			DeeObject_Init(result, &DefaultSequence_WithTSizeAndGetItem_Type);
			return (DREF DeeObject *)result;
		}
	}
#endif /* DEFINE_TYPED_OPERATORS */
	{
		DREF DefaultSequence_WithSizeAndGetItem *result;
		result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
		if unlikely(!result)
			goto err;
		result->dssg_start = startob_and_endob[0]; /* Inherit reference */
		result->dssg_end   = startob_and_endob[1]; /* Inherit reference */
		Dee_Incref(self);
		result->dssg_seq        = self;
		result->dssg_tp_getitem = tp_self->tp_seq->tp_getitem;
		DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
		return (DREF DeeObject *)result;
	}
	__builtin_unreachable();
err:
	return NULL;
}


DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexWithGetRange,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	DREF DeeObject *result;
	DREF DeeObject *startob, *endob;
	LOAD_TP_SELF;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeType_INVOKE_GETRANGE_NODEFAULT(tp_self, self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}


DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexWithSizeAndGetItemIndexFast,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	size_t size;
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	struct Dee_seq_range range;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index_fast;
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	size_t size;
	struct Dee_seq_range range;
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index;
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
	__builtin_unreachable();
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexWithSizeDefaultAndGetItem,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	size_t size;
	struct Dee_seq_range range;
	DREF DefaultSequence_WithSizeAndGetItem *result;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
#ifdef DEFINE_TYPED_OPERATORS
	if (!Dee_TYPE(self)->tp_seq ||
	    (Dee_TYPE(self)->tp_seq->tp_getitem != tp_self->tp_seq->tp_getitem)) {
		DREF DeeObject *(DCALL *tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
		tp_tgetitem = tp_self->tp_seq->tp_getitem == &instance_getitem
		              ? &instance_tgetitem
		              : NULL/*DeeType_MapDefaultGetItem(tp_self->tp_seq->tp_getitem, &, NULL)*/;
		if (tp_tgetitem) {
			DREF DefaultSequence_WithTSizeAndGetItem *tresult;
			tresult = DeeObject_MALLOC(DefaultSequence_WithTSizeAndGetItem);
			if unlikely(!tresult)
				goto err;
			tresult->dstsg_start = DeeInt_NewSize(range.sr_start);
			if unlikely(!tresult->dstsg_start) {
err_tr:
				DeeObject_FREE(tresult);
				goto err;
			}
			tresult->dstsg_end = DeeInt_NewSize(range.sr_end);
			if unlikely(!tresult->dstsg_end) {
				Dee_Decref(tresult->dstsg_start);
				goto err_tr;
			}
			tresult->dstsg_tp_seq = tp_self;
			Dee_Incref(self);
			tresult->dstsg_seq         = self;
			tresult->dstsg_tp_tgetitem = tp_tgetitem;
			DeeObject_Init(tresult, &DefaultSequence_WithTSizeAndGetItem_Type);
			return (DREF DeeObject *)tresult;
		}
	}
#endif /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = DeeInt_NewSize(range.sr_start);
	if unlikely(!result->dssg_start)
		goto err_r;
	result->dssg_end = DeeInt_NewSize(range.sr_end);
	if unlikely(!result->dssg_end)
		goto err_r_start;
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = tp_self->tp_seq->tp_getitem;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return (DREF DeeObject *)result;
err_r_start:
	Dee_Decref(result->dssg_start);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexWithSizeDefaultAndIter,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	DREF DefaultSequence_WithIter *result;
	struct Dee_seq_range range;
	LOAD_TP_SELF;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size;
		size = DeeType_INVOKE_SIZE(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		goto empty_seq;
	result = DeeObject_MALLOC(DefaultSequence_WithIter);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsi_seq     = self;
	result->dsi_start   = range.sr_start;
	result->dsi_limit   = range.sr_end - range.sr_start;
	result->dsi_tp_iter = tp_self->tp_seq->tp_iter;
	DeeObject_Init(result, &DefaultSequence_WithIter_Type);
	return (DREF DeeObject *)result;
empty_seq:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexWithSizeDefaultAndIterDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	struct Dee_seq_range range;
	LOAD_TP_SELF;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size;
		size = DeeType_INVOKE_SIZE(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		goto empty_seq;
#ifdef DEFINE_TYPED_OPERATORS
	if (!Dee_TYPE(self)->tp_seq ||
	    (Dee_TYPE(self)->tp_seq->tp_iter != tp_self->tp_seq->tp_iter)) {
		DREF DeeObject *(DCALL *tp_titer)(DeeTypeObject *tp_self, DeeObject *self);
		tp_titer = tp_self->tp_seq->tp_iter == &instance_iter
		           ? &instance_titer
		           : DeeType_MapDefaultIter(tp_self->tp_seq->tp_iter, &, NULL);
		if (tp_titer) {
			DREF DefaultSequence_WithTIter *tresult;
			tresult = DeeObject_MALLOC(DefaultSequence_WithTIter);
			if unlikely(!tresult)
				goto err;
			Dee_Incref(self);
			tresult->dsti_seq      = self;
			tresult->dsti_tp_titer = tp_titer;
			tresult->dsti_start    = range.sr_start;
			tresult->dsti_limit     = range.sr_end - range.sr_start;
			tresult->dsti_tp_seq   = tp_self;
			DeeObject_Init(tresult, &DefaultSequence_WithTIter_Type);
			return (DREF DeeObject *)tresult;
		}
	}
#endif /* DEFINE_TYPED_OPERATORS */
	{
		DREF DefaultSequence_WithIter *result;
		result = DeeObject_MALLOC(DefaultSequence_WithIter);
		if unlikely(!result)
			goto err;
		Dee_Incref(self);
		result->dsi_seq     = self;
		result->dsi_start   = range.sr_start;
		result->dsi_limit   = range.sr_end - range.sr_start;
		result->dsi_tp_iter = tp_self->tp_seq->tp_iter;
		DeeObject_Init(result, &DefaultSequence_WithIter_Type);
		return (DREF DeeObject *)result;
	}
	__builtin_unreachable();
empty_seq:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithGetRange,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	DREF DeeObject *result;
	DREF DeeObject *startob;
	LOAD_TP_SELF;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeType_INVOKE_GETRANGE_NODEFAULT(tp_self, self, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeAndGetRangeIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_GETRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size);
empty_range:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_GETRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size);
empty_range:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeAndGetRangeIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_GETRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size);
empty_range:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_GETRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size);
empty_range:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeAndGetItemIndexFast,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index_fast;
	result->dssgi_start            = (size_t)start;
	result->dssgi_end              = size;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = tp_self->tp_seq->tp_getitem_index;
	result->dssgi_start            = (size_t)start;
	result->dssgi_end              = size;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeDefaultAndGetItem,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	DREF DefaultSequence_WithSizeAndGetItem *result;
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
#ifdef DEFINE_TYPED_OPERATORS
	if (!Dee_TYPE(self)->tp_seq ||
	    (Dee_TYPE(self)->tp_seq->tp_getitem != tp_self->tp_seq->tp_getitem)) {
		DREF DeeObject *(DCALL *tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
		tp_tgetitem = tp_self->tp_seq->tp_getitem == &instance_getitem
		              ? &instance_tgetitem
		              : NULL/*DeeType_MapDefaultGetItem(tp_self->tp_seq->tp_getitem, &, NULL)*/;
		if (tp_tgetitem) {
			DREF DefaultSequence_WithTSizeAndGetItem *tresult;
			tresult = DeeObject_MALLOC(DefaultSequence_WithTSizeAndGetItem);
			if unlikely(!tresult)
				goto err;
			tresult->dstsg_start = DeeInt_NewSize((size_t)start);
			if unlikely(!tresult->dstsg_start) {
err_tr:
				DeeObject_FREE(tresult);
				goto err;
			}
			tresult->dstsg_end = DeeInt_NewSize(size);
			if unlikely(!tresult->dstsg_end) {
				Dee_Decref(tresult->dstsg_start);
				goto err_tr;
			}
			tresult->dstsg_tp_seq = tp_self;
			Dee_Incref(self);
			tresult->dstsg_seq         = self;
			tresult->dstsg_tp_tgetitem = tp_tgetitem;
			DeeObject_Init(tresult, &DefaultSequence_WithTSizeAndGetItem_Type);
			return (DREF DeeObject *)tresult;
		}
	}
#endif /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = DeeInt_NewSize((size_t)start);
	if unlikely(!result->dssg_start)
		goto err_r;
	result->dssg_end = DeeInt_NewSize(size);
	if unlikely(!result->dssg_end)
		goto err_r_start;
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = tp_self->tp_seq->tp_getitem;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err_r_start:
	Dee_Decref(result->dssg_start);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeDefaultAndIter,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	DREF DefaultSequence_WithIter *result;
	size_t used_start;
	LOAD_TP_SELF;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size;
		size = DeeType_INVOKE_SIZE(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
	result = DeeObject_MALLOC(DefaultSequence_WithIter);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsi_seq     = self;
	result->dsi_start   = used_start;
	result->dsi_limit   = (size_t)-1;
	result->dsi_tp_iter = tp_self->tp_seq->tp_iter;
	DeeObject_Init(result, &DefaultSequence_WithIter_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

DEFINE_INTERNAL_SEQ_OPERATOR(DREF DeeObject *, DefaultGetRangeIndexNWithSizeDefaultAndIterDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t used_start;
	LOAD_TP_SELF;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size;
		size = DeeType_INVOKE_SIZE(tp_self, self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
#ifdef DEFINE_TYPED_OPERATORS
	if (!Dee_TYPE(self)->tp_seq ||
	    (Dee_TYPE(self)->tp_seq->tp_iter != tp_self->tp_seq->tp_iter)) {
		DREF DeeObject *(DCALL *tp_titer)(DeeTypeObject *tp_self, DeeObject *self);
		tp_titer = tp_self->tp_seq->tp_iter == &instance_iter
		           ? &instance_titer
		           : DeeType_MapDefaultIter(tp_self->tp_seq->tp_iter, &, NULL);
		if (tp_titer) {
			DREF DefaultSequence_WithTIter *tresult;
			tresult = DeeObject_MALLOC(DefaultSequence_WithTIter);
			if unlikely(!tresult)
				goto err;
			Dee_Incref(self);
			tresult->dsti_seq      = self;
			tresult->dsti_tp_titer = tp_titer;
			tresult->dsti_start    = used_start;
			tresult->dsti_limit     = (size_t)-1;
			tresult->dsti_tp_seq   = tp_self;
			DeeObject_Init(tresult, &DefaultSequence_WithTIter_Type);
			return (DREF DeeObject *)tresult;
		}
	}
#endif /* DEFINE_TYPED_OPERATORS */
	{
		DREF DefaultSequence_WithIter *result;
		result = DeeObject_MALLOC(DefaultSequence_WithIter);
		if unlikely(!result)
			goto err;
		Dee_Incref(self);
		result->dsi_seq     = self;
		result->dsi_start   = used_start;
		result->dsi_limit   = (size_t)-1;
		result->dsi_tp_iter = tp_self->tp_seq->tp_iter;
		DeeObject_Init(result, &DefaultSequence_WithIter_Type);
		return (DREF DeeObject *)result;
	}
	__builtin_unreachable();
err:
	return NULL;
}


DEFINE_INTERNAL_OPERATOR(int, DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN,
                         (DeeObject *self, DeeObject *start, DeeObject *end)) {
	Dee_ssize_t start_index, end_index;
	LOAD_TP_SELF;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeType_INVOKE_DELRANGEINDEXN_NODEFAULT(tp_self, self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeType_INVOKE_DELRANGEINDEX_NODEFAULT(tp_self, self, start_index, end_index);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault,
                         (DeeObject *self, DeeObject *start, DeeObject *end)) {
	Dee_ssize_t start_index, end_index;
	LOAD_TP_SELF;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeType_INVOKE_DELRANGEINDEXN(tp_self, self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start_index, end_index);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeWithSetRangeNone,
                             (DeeObject *self, DeeObject *start, DeeObject *end)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_SETRANGE_NODEFAULT(tp_self, self, start, end, Dee_None);
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeWithSetRangeNoneDefault,
                             (DeeObject *self, DeeObject *start, DeeObject *end)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_SETRANGE(tp_self, self, start, end, Dee_None);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelRangeIndexWithDelRange,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	int result;
	DREF DeeObject *startob, *endob;
	LOAD_TP_SELF;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeType_INVOKE_DELRANGE_NODEFAULT(tp_self, self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeIndexWithSetRangeIndexNone,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_SETRANGEINDEX_NODEFAULT(tp_self, self, start, end, Dee_None);
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeIndexWithSetRangeIndexNoneDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start, end, Dee_None);
}

DEFINE_INTERNAL_OPERATOR(int, DefaultDelRangeIndexNWithDelRange,
                         (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	int result;
	DREF DeeObject *startob;
	LOAD_TP_SELF;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeType_INVOKE_DELRANGE_NODEFAULT(tp_self, self, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeIndexNWithSizeAndDelRangeIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_DELRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size);
empty_range:
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_DELRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size);
empty_range:
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeIndexNWithSetRangeIndexNNone,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_SETRANGEINDEXN_NODEFAULT(tp_self, self, start, Dee_None);
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	LOAD_TP_SELF;
	return DeeType_INVOKE_SETRANGEINDEXN(tp_self, self, start, Dee_None);
}


DEFINE_INTERNAL_OPERATOR(int, DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN,
                         (DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value)) {
	Dee_ssize_t start_index, end_index;
	LOAD_TP_SELF;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeType_INVOKE_SETRANGEINDEXN_NODEFAULT(tp_self, self, start_index, value);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeType_INVOKE_SETRANGEINDEX_NODEFAULT(tp_self, self, start_index, end_index, value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault,
                         (DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *value)) {
	Dee_ssize_t start_index, end_index;
	LOAD_TP_SELF;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeType_INVOKE_SETRANGEINDEXN(tp_self, self, start_index, value);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start_index, end_index, value);
err:
	return -1;
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetRangeIndexWithSetRange,
                         (DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value)) {
	int result;
	DREF DeeObject *startob, *endob;
	LOAD_TP_SELF;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeType_INVOKE_SETRANGE_NODEFAULT(tp_self, self, startob, endob, value);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex,
                             (DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)end;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault,
                             (DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)end;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexWithSizeAndSetItemIndex,
                             (DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)end;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault,
                             (DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)end;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

DEFINE_INTERNAL_OPERATOR(int, DefaultSetRangeIndexNWithSetRange,
                         (DeeObject *self, Dee_ssize_t start, DeeObject *value)) {
	int result;
	DREF DeeObject *startob;
	LOAD_TP_SELF;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeType_INVOKE_SETRANGE_NODEFAULT(tp_self, self, startob, Dee_None, value);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}


DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexNWithSizeAndSetRangeIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, DeeObject *value)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE_NODEFAULT(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_SETRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size, value);
empty_range:
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, DeeObject *value)) {
	size_t size;
	LOAD_TP_SELF;
	size = DeeType_INVOKE_SIZE(tp_self, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return DeeType_INVOKE_SETRANGEINDEX_NODEFAULT(tp_self, self, start, (Dee_ssize_t)size, value);
empty_range:
	return 0;
err:
	return -1;
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex,
                             (DeeObject *self, Dee_ssize_t start, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault,
                             (DeeObject *self, Dee_ssize_t start, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexNWithSizeAndSetItemIndex,
                             (DeeObject *self, Dee_ssize_t start, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

DEFINE_INTERNAL_SEQ_OPERATOR(int, DefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault,
                             (DeeObject *self, Dee_ssize_t start, DeeObject *value)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)start;
	(void)value;
	return DeeError_NOTIMPLEMENTED();
}

#ifndef DEFINE_TYPED_OPERATORS
/* Extra map functions that are needed for implementing generic map operator. */
DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultContainsWithForeachPair,
                             (DeeObject *self, DeeObject *elem)) {
	LOAD_TP_SELF;
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)elem;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(Dee_ssize_t, DefaultEnumerateIndexWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc,
                              void *arg, size_t start, size_t end)) {
	struct default_enumerate_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.deiwe_proc  = proc;
	data.deiwe_arg   = arg;
	data.deiwe_start = start;
	data.deiwe_end   = end;
	return DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_enumerate_index_with_enumerate_cb, &data);
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, DeeObject *key)) {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgied_key = key;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if unlikely(status == -3) {
		err_unbound_key(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key(self, key);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemIndexWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t key)) {
	Dee_ssize_t status;
	struct default_map_getitem_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgiied_key = key;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_index_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgiied_result;
	if unlikely(status == -3) {
		err_unbound_index(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_int(self, key);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemStringHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	if unlikely(status == -3) {
		err_unbound_key_str(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str(self, key);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultGetItemStringLenHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if unlikely(status == -3) {
		err_unbound_key_str_len(self, key, keylen);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str_len(self, key, keylen);
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, DeeObject *key)) {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgied_key = key;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	ASSERT(status == -1);
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemIndexWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t key)) {
	Dee_ssize_t status;
	struct default_map_getitem_index_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgiied_key = key;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_index_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgiied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(DREF DeeObject *, DefaultTryGetItemStringLenHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	return NULL;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemWithForeachPair,
                             (DeeObject *self, DeeObject *key)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_with_enumerate_cb, key);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemIndexWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t key)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_index_with_enumerate_cb, (void *)(uintptr_t)key);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbished_key  = key;
	data.mbished_hash = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_string_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultBoundItemStringLenHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbislhed_key    = key;
	data.mbislhed_keylen = keylen;
	data.mbislhed_hash   = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_string_len_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = 1;
	} else if (status == -3) {
		status = 0;
	} else if (status == 0) {
		status = -2;
	}
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemWithForeachPair,
                             (DeeObject *self, DeeObject *key)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_with_enumerate_cb, key);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemIndexWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, size_t key)) {
	Dee_ssize_t status;
	LOAD_TP_SELF;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_index_with_enumerate_cb, (void *)(uintptr_t)key);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbished_key  = key;
	data.mbished_hash = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_string_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}

DEFINE_INTERNAL_MAP_OPERATOR(int, DefaultHasItemStringLenHashWithForeachPair,
                             (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	Dee_ssize_t status;
	struct default_map_bounditem_string_len_hash_with_enumerate_data data;
	LOAD_TP_SELF;
	data.mbislhed_key    = key;
	data.mbislhed_keylen = keylen;
	data.mbislhed_hash   = hash;
	status = DeeType_INVOKE_FOREACH_PAIR_NODEFAULT(tp_self, self, &default_map_bounditem_string_len_hash_with_enumerate_cb, &data);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		status = 1;
	return (int)status;
}
#endif /* !DEFINE_TYPED_OPERATORS */






#ifndef DEFINE_TYPED_OPERATORS
/* inc, dec, add, sub, iadd & isub are all apart of the same operator group. */
INTERN NONNULL((1)) bool DCALL DeeType_InheritAdd(DeeTypeObject *__restrict self) {
	struct type_math *base_math;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_math = self->tp_math;
	if (base_math) {
		bool ok = false;
		if (base_math->tp_add) {
			if (base_math->tp_sub == NULL)
				base_math->tp_sub = &DeeObject_DefaultSubWithAdd;
			if (base_math->tp_inplace_add == NULL)
				base_math->tp_inplace_add = &DeeObject_DefaultInplaceAddWithAdd;
			if (base_math->tp_inplace_sub == NULL)
				base_math->tp_inplace_sub = &DeeObject_DefaultInplaceSubWithAdd;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithAdd;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithAdd;
			ok = true;
		} else if (base_math->tp_inplace_add) {
			if (base_math->tp_add == NULL)
				base_math->tp_add = &DeeObject_DefaultAddWithInplaceAdd;
			if (base_math->tp_sub == NULL)
				base_math->tp_sub = &DeeObject_DefaultSubWithInplaceAdd;
			if (base_math->tp_inplace_sub == NULL)
				base_math->tp_inplace_sub = &DeeObject_DefaultInplaceSubWithInplaceAdd;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithInplaceAdd;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithInplaceAdd;
			ok = true;
		}
		if (base_math->tp_sub) {
			if (base_math->tp_add == NULL)
				base_math->tp_add = &DeeObject_DefaultAddWithSub;
			if (base_math->tp_inplace_add == NULL)
				base_math->tp_inplace_add = &DeeObject_DefaultInplaceAddWithSub;
			if (base_math->tp_inplace_sub == NULL)
				base_math->tp_inplace_sub = &DeeObject_DefaultInplaceSubWithSub;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithSub;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithSub;
			ok = true;
		} else if (base_math->tp_inplace_sub) {
			if (base_math->tp_add == NULL)
				base_math->tp_add = &DeeObject_DefaultAddWithInplaceSub;
			if (base_math->tp_inplace_add == NULL)
				base_math->tp_inplace_add = &DeeObject_DefaultInplaceAddWithInplaceSub;
			if (base_math->tp_sub == NULL)
				base_math->tp_sub = &DeeObject_DefaultSubWithInplaceSub;
			if (base_math->tp_inc == NULL)
				base_math->tp_inc = &DeeObject_DefaultIncWithInplaceSub;
			if (base_math->tp_dec == NULL)
				base_math->tp_dec = &DeeObject_DefaultDecWithInplaceSub;
			ok = true;
		}
		if (ok)
			return true;
		if (base_math->tp_inc || base_math->tp_dec)
			return true;
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_math = base->tp_math;
		if (base_math == NULL ||
		    (!base_math->tp_add && !base_math->tp_inplace_add &&
		     !base_math->tp_sub && !base_math->tp_inplace_sub &&
		     !base_math->tp_inc && !base_math->tp_dec)) {
			if (!DeeType_InheritAdd(base))
				continue;
			base_math = base->tp_math;
		}
		if (self->tp_math) {
			DeeTypeObject *origin = DeeType_GetMathOrigin(self);
			if unlikely(origin)
				return DeeType_InheritAdd(origin);
			self->tp_math->tp_inc = base_math->tp_inc;
			self->tp_math->tp_dec = base_math->tp_dec;
			self->tp_math->tp_add = base_math->tp_add;
			self->tp_math->tp_sub = base_math->tp_sub;
			self->tp_math->tp_inplace_add = base_math->tp_inplace_add;
			self->tp_math->tp_inplace_sub = base_math->tp_inplace_sub;
		} else {
			self->tp_math = base_math;
		}
		LOG_INHERIT(base, self, "operator add");
		return true;
	}
	return false;
}

#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field, Field)                                   \
	INTERN NONNULL((1)) bool DCALL                                                                 \
	name(DeeTypeObject *__restrict self) {                                                         \
		struct type_math *base_math;                                                               \
		DeeTypeMRO mro;                                                                            \
		DeeTypeObject *base;                                                                       \
		base_math = self->tp_math;                                                                 \
		if (base_math) {                                                                           \
			if (base_math->tp_##field) {                                                           \
				if (base_math->tp_inplace_##field == NULL)                                         \
					base_math->tp_inplace_##field = &DeeObject_DefaultInplace##Field##With##Field; \
				return true;                                                                       \
			} else if (base_math->tp_inplace_##field) {                                            \
				if (base_math->tp_##field == NULL)                                                 \
					base_math->tp_##field = &DeeObject_Default##Field##WithInplace##Field;         \
				return true;                                                                       \
			}                                                                                      \
		}                                                                                          \
		base = DeeTypeMRO_Init(&mro, self);                                                        \
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {                           \
			base_math = base->tp_math;                                                             \
			if (base_math == NULL ||                                                               \
			    (!base_math->tp_##field &&                                                         \
			     !base_math->tp_inplace_##field)) {                                                \
				if (!name(base))                                                                   \
					continue;                                                                      \
				base_math = base->tp_math;                                                         \
			}                                                                                      \
			if (self->tp_math) {                                                                   \
				DeeTypeObject *origin = DeeType_GetMathOrigin(self);                               \
				if unlikely(origin)                                                                \
					return name(origin);                                                           \
				self->tp_math->tp_##field         = base_math->tp_##field;                         \
				self->tp_math->tp_inplace_##field = base_math->tp_inplace_##field;                 \
			} else {                                                                               \
				self->tp_math = base_math;                                                         \
			}                                                                                      \
			LOG_INHERIT(base, self, opname);                                                       \
			return true;                                                                           \
		}                                                                                          \
		return false;                                                                              \
	}
#define DEFINE_TYPE_INHERIT_FUNCTION1(name, opname, field)               \
	INTERN NONNULL((1)) bool DCALL                                       \
	name(DeeTypeObject *__restrict self) {                               \
		struct type_math *base_math;                                     \
		DeeTypeMRO mro;                                                  \
		DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);               \
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) { \
			base_math = base->tp_math;                                   \
			if (base_math == NULL || !base_math->tp_##field) {           \
				if (!name(base))                                         \
					continue;                                            \
				base_math = base->tp_math;                               \
			}                                                            \
			if (self->tp_math) {                                         \
				DeeTypeObject *origin = DeeType_GetMathOrigin(self);     \
				if unlikely(origin)                                      \
					return name(origin);                                 \
				self->tp_math->tp_##field = base_math->tp_##field;       \
			} else {                                                     \
				self->tp_math = base_math;                               \
			}                                                            \
			LOG_INHERIT(base, self, opname);                             \
			return true;                                                 \
		}                                                                \
		return false;                                                    \
	}
DEFINE_TYPE_INHERIT_FUNCTION1(DeeType_InheritInv, "operator inv", inv)
DEFINE_TYPE_INHERIT_FUNCTION1(DeeType_InheritPos, "operator pos", pos)
DEFINE_TYPE_INHERIT_FUNCTION1(DeeType_InheritNeg, "operator neg", neg)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritMul, "operator mul", mul, Mul)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritDiv, "operator div", div, Div)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritMod, "operator mod", mod, Mod)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritShl, "operator shl", shl, Shl)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritShr, "operator shr", shr, Shr)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritAnd, "operator and", and, And)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritOr, "operator or", or, Or)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritXor, "operator xor", xor, Xor)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritPow, "operator pow", pow, Pow)
#undef DEFINE_TYPE_INHERIT_FUNCTION1
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(DREF DeeObject *, Inv, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_inv) ||
	          unlikely(DeeType_InheritInv(tp_self)))
		return DeeType_INVOKE_INV(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_INV);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Pos, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_pos) ||
	          unlikely(DeeType_InheritPos(tp_self)))
		return DeeType_INVOKE_POS(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_POS);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Neg, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_neg) ||
	          unlikely(DeeType_InheritNeg(tp_self)))
		return DeeType_INVOKE_NEG(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_NEG);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Add, (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_add) ||
	          unlikely(DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_add))
		return DeeType_INVOKE_ADD(tp_self, self, some_object);
	err_unimplemented_operator(tp_self, OPERATOR_ADD);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Sub, (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_sub) ||
	          unlikely(DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_sub))
		return DeeType_INVOKE_SUB(tp_self, self, some_object);
	err_unimplemented_operator(tp_self, OPERATOR_SUB);
	return NULL;
}

#define DEFINE_MATH_OPERATOR2(name, xxx, operator_name, invoke, inherit)    \
	DEFINE_OPERATOR(DREF DeeObject *, name,                                 \
	                (DeeObject *self, DeeObject *some_object)) {            \
		LOAD_TP_SELF;                                                       \
		ASSERT_OBJECT(some_object);                                         \
		if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_##xxx) || \
		          unlikely(inherit(tp_self)))                               \
			return invoke(tp_self, self, some_object);                      \
		err_unimplemented_operator(tp_self, operator_name);                 \
		return NULL;                                                        \
	}
DEFINE_MATH_OPERATOR2(Mul, mul, OPERATOR_MUL, DeeType_INVOKE_MUL, DeeType_InheritMul)
DEFINE_MATH_OPERATOR2(Div, div, OPERATOR_DIV, DeeType_INVOKE_DIV, DeeType_InheritDiv)
DEFINE_MATH_OPERATOR2(Mod, mod, OPERATOR_MOD, DeeType_INVOKE_MOD, DeeType_InheritMod)
DEFINE_MATH_OPERATOR2(Shl, shl, OPERATOR_SHL, DeeType_INVOKE_SHL, DeeType_InheritShl)
DEFINE_MATH_OPERATOR2(Shr, shr, OPERATOR_SHR, DeeType_INVOKE_SHR, DeeType_InheritShr)
DEFINE_MATH_OPERATOR2(And, and, OPERATOR_AND, DeeType_INVOKE_AND, DeeType_InheritAnd)
DEFINE_MATH_OPERATOR2(Or, or, OPERATOR_OR, DeeType_INVOKE_OR, DeeType_InheritOr)
DEFINE_MATH_OPERATOR2(Xor, xor, OPERATOR_XOR, DeeType_INVOKE_XOR, DeeType_InheritXor)
DEFINE_MATH_OPERATOR2(Pow, pow, OPERATOR_POW, DeeType_INVOKE_POW, DeeType_InheritPow)
#undef DEFINE_MATH_OPERATOR2

#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AddInt8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return (DREF DeeObject *)DeeInt_AddSDigit((DeeIntObject *)self, val);
	val_ob = DeeInt_NewInt8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Add(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_SubInt8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return (DREF DeeObject *)DeeInt_SubSDigit((DeeIntObject *)self, val);
	val_ob = DeeInt_NewInt8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Sub(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AddUInt32(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return (DREF DeeObject *)DeeInt_AddUInt32((DeeIntObject *)self, val);
	val_ob = DeeInt_NewUInt32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Add(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AddUInt64(DeeObject *__restrict self, uint64_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewUInt64(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Add(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_SubUInt32(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return (DREF DeeObject *)DeeInt_SubUInt32((DeeIntObject *)self, val);
	val_ob = DeeInt_NewUInt32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Sub(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_SubUInt64(DeeObject *__restrict self, uint64_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewUInt64(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Sub(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_MulInt8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewInt8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Mul(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_DivInt8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewInt8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Div(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_ModInt8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewInt8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Mod(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_ShlUInt8(DeeObject *__restrict self, uint8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewUInt8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Shl(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_ShrUInt8(DeeObject *__restrict self, uint8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewUInt8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Shr(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AndUInt32(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewUInt32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_And(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_OrUInt32(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewUInt32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Or(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_XorUInt32(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewUInt32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Xor(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(int, Inc, (DREF DeeObject **__restrict p_self)) {
	LOAD_TP_SELFP;
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_inc) ||
	          unlikely(DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_inc))
		return DeeType_INVOKE_INC(tp_self, p_self);
	return err_unimplemented_operator(tp_self, OPERATOR_INC);
}

DEFINE_OPERATOR(int, Dec, (DREF DeeObject **__restrict p_self)) {
	LOAD_TP_SELFP;
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_dec) ||
	          unlikely(DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_dec))
		return DeeType_INVOKE_DEC(tp_self, p_self);
	return err_unimplemented_operator(tp_self, OPERATOR_DEC);
}

DEFINE_OPERATOR(int, InplaceAdd, (DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object)) {
	LOAD_TP_SELFP;
	ASSERT_OBJECT(some_object);
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_inplace_add) ||
	          unlikely(DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_inplace_add))
		return DeeType_INVOKE_IADD(tp_self, p_self, some_object);
	return err_unimplemented_operator(tp_self, OPERATOR_INPLACE_ADD);
}

DEFINE_OPERATOR(int, InplaceSub, (DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object)) {
	LOAD_TP_SELFP;
	ASSERT_OBJECT(some_object);
	if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_inplace_sub) ||
	          unlikely(DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_inplace_sub))
		return DeeType_INVOKE_ISUB(tp_self, p_self, some_object);
	return err_unimplemented_operator(tp_self, OPERATOR_INPLACE_SUB);
}

#define DEFINE_MATH_INPLACE_OPERATOR2(name, xxx, operator_name, invoke_inplace, inherit) \
	DEFINE_OPERATOR(int, name, (DREF DeeObject **__restrict p_self,                      \
	                            DeeObject *some_object)) {                               \
		LOAD_TP_SELFP;                                                                   \
		ASSERT_OBJECT(some_object);                                                      \
		if likely(likely(tp_self->tp_math && tp_self->tp_math->tp_inplace_##xxx) ||      \
		          unlikely(inherit(tp_self)))                                            \
			return invoke_inplace(tp_self, p_self, some_object);                         \
		return err_unimplemented_operator(tp_self, operator_name);                       \
	}
DEFINE_MATH_INPLACE_OPERATOR2(InplaceMul, mul, OPERATOR_INPLACE_MUL, DeeType_INVOKE_IMUL, DeeType_InheritMul)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceDiv, div, OPERATOR_INPLACE_DIV, DeeType_INVOKE_IDIV, DeeType_InheritDiv)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceMod, mod, OPERATOR_INPLACE_MOD, DeeType_INVOKE_IMOD, DeeType_InheritMod)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceShl, shl, OPERATOR_INPLACE_SHL, DeeType_INVOKE_ISHL, DeeType_InheritShl)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceShr, shr, OPERATOR_INPLACE_SHR, DeeType_INVOKE_ISHR, DeeType_InheritShr)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceAnd, and, OPERATOR_INPLACE_AND, DeeType_INVOKE_IAND, DeeType_InheritAnd)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceOr, or, OPERATOR_INPLACE_OR, DeeType_INVOKE_IOR, DeeType_InheritOr)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceXor, xor, OPERATOR_INPLACE_XOR, DeeType_INVOKE_IXOR, DeeType_InheritXor)
DEFINE_MATH_INPLACE_OPERATOR2(InplacePow, pow, OPERATOR_INPLACE_POW, DeeType_INVOKE_IPOW, DeeType_InheritPow)
#undef DEFINE_MATH_INPLACE_OPERATOR2

#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceXXX, reg, DeeInt_NewXXX, intX_t, operator_name) \
	PUBLIC int DCALL                                                                                      \
	DeeObject_InplaceXXX(DREF DeeObject **__restrict p_self, intX_t val) {                                \
		DREF DeeObject *temp;                                                                             \
		int result;                                                                                       \
		/* TODO: Optimization for `int' */                                                                \
		temp = DeeInt_NewXXX(val);                                                                        \
		if unlikely(!temp)                                                                                \
			goto err;                                                                                     \
		result = reg(p_self, temp);                                                                       \
		Dee_Decref(temp);                                                                                 \
		return result;                                                                                    \
	err:                                                                                                  \
		return -1;                                                                                        \
	}
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAddInt8, DeeObject_InplaceAdd, DeeInt_NewInt8, int8_t, OPERATOR_INPLACE_ADD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceSubInt8, DeeObject_InplaceSub, DeeInt_NewInt8, int8_t, OPERATOR_INPLACE_SUB)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAddUInt32, DeeObject_InplaceAdd, DeeInt_NewUInt32, uint32_t, OPERATOR_INPLACE_ADD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceSubUInt32, DeeObject_InplaceSub, DeeInt_NewUInt32, uint32_t, OPERATOR_INPLACE_SUB)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceMulInt8, DeeObject_InplaceMul, DeeInt_NewInt8, int8_t, OPERATOR_INPLACE_MUL)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceDivInt8, DeeObject_InplaceDiv, DeeInt_NewInt8, int8_t, OPERATOR_INPLACE_DIV)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceModInt8, DeeObject_InplaceMod, DeeInt_NewInt8, int8_t, OPERATOR_INPLACE_MOD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceShlUInt8, DeeObject_InplaceShl, DeeInt_NewUInt8, uint8_t, OPERATOR_INPLACE_SHL)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceShrUInt8, DeeObject_InplaceShr, DeeInt_NewUInt8, uint8_t, OPERATOR_INPLACE_SHR)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAndUInt32, DeeObject_InplaceAnd, DeeInt_NewUInt32, uint32_t, OPERATOR_INPLACE_AND)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceOrUInt32, DeeObject_InplaceOr, DeeInt_NewUInt32, uint32_t, OPERATOR_INPLACE_OR)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceXorUInt32, DeeObject_InplaceXor, DeeInt_NewUInt32, uint32_t, OPERATOR_INPLACE_XOR)
#undef DEFINE_MATH_INPLACE_INT_OPERATOR
#endif /* !DEFINE_TYPED_OPERATORS */


#define DEFINE_OBJECT_COMPARE_OPERATOR(name, tp_xx, operator_name, invoke)             \
	DEFINE_OPERATOR(DREF DeeObject *, name,                                            \
	                (DeeObject *self, DeeObject *some_object)) {                       \
		LOAD_TP_SELF;                                                                  \
		ASSERT_OBJECT(some_object);                                                    \
		if likely(likely(tp_self->tp_cmp && tp_self->tp_cmp->tp_xx) ||                 \
		          unlikely(DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_xx)) \
			return invoke(tp_self, self, some_object);                                 \
		err_unimplemented_operator(tp_self, operator_name);                            \
		return NULL;                                                                   \
	}
DEFINE_OBJECT_COMPARE_OPERATOR(CmpEq, tp_eq, OPERATOR_EQ, DeeType_INVOKE_EQ)
DEFINE_OBJECT_COMPARE_OPERATOR(CmpNe, tp_ne, OPERATOR_NE, DeeType_INVOKE_NE)
DEFINE_OBJECT_COMPARE_OPERATOR(CmpLo, tp_lo, OPERATOR_LO, DeeType_INVOKE_LO)
DEFINE_OBJECT_COMPARE_OPERATOR(CmpLe, tp_le, OPERATOR_LE, DeeType_INVOKE_LE)
DEFINE_OBJECT_COMPARE_OPERATOR(CmpGr, tp_gr, OPERATOR_GR, DeeType_INVOKE_GR)
DEFINE_OBJECT_COMPARE_OPERATOR(CmpGe, tp_ge, OPERATOR_GE, DeeType_INVOKE_GE)
#undef DEFINE_OBJECT_COMPARE_OPERATOR


#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_COMPARE_ASBOOL_OPERATOR(name_asbool, name)  \
	PUBLIC WUNUSED NONNULL((1, 2)) int DCALL               \
	name_asbool(DeeObject *self, DeeObject *some_object) { \
		DREF DeeObject *result;                            \
		result = name(self, some_object);                  \
		if unlikely(!result)                               \
			goto err;                                      \
		return DeeObject_BoolInherited(result);            \
	err:                                                   \
		return -1;                                         \
	}
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpEqAsBool, DeeObject_CmpEq)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpNeAsBool, DeeObject_CmpNe)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpLoAsBool, DeeObject_CmpLo)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpLeAsBool, DeeObject_CmpLe)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpGrAsBool, DeeObject_CmpGr)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpGeAsBool, DeeObject_CmpGe)
#undef DEFINE_COMPARE_ASBOOL_OPERATOR

/* Deprecated wrapper around "DeeObject_TryCompareEq()"
 * @return: 1 : Compare returns "true"
 * @return: 0 : Compare returns "false"
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TryCmpEqAsBool)(DeeObject *self, DeeObject *some_object) {
	int result = DeeObject_TryCompareEq(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		return -1;
	return result == 0 ? 1 : 0;
}

/* Compare a pre-keyed `keyed_search_item' with `elem' using the given (optional) `key' function
 * @return:  > 0: `keyed_search_item == key(elem)'
 * @return: == 0: `keyed_search_item != key(elem)'
 * @return:  < 0: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TryCmpKeyEqAsBool)(DeeObject *keyed_search_item,
                                    DeeObject *elem, /*nullable*/ DeeObject *key) {
	int result = key ? DeeObject_TryCompareKeyEq(keyed_search_item, elem, key)
	                 : DeeObject_TryCompareEq(keyed_search_item, elem);
	if unlikely(result == Dee_COMPARE_ERR)
		return -1;
	return result == 0 ? 1 : 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */


/* @return: == -1: `lhs < rhs'
 * @return: == 0:  `lhs == rhs'
 * @return: == 1:  `lhs > rhs'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DEFINE_OPERATOR(int, Compare, (DeeObject *self, DeeObject *rhs)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_cmp && tp_self->tp_cmp->tp_compare) ||
	          unlikely(DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_compare))
		return DeeType_INVOKE_COMPARE(tp_self, self, rhs);
	err_unimplemented_operator(tp_self, OPERATOR_LO);
	return Dee_COMPARE_ERR;
}

/* @return: == -1: `lhs != rhs'
 * @return: == 0:  `lhs == rhs'
 * @return: == 1:  `lhs != rhs'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DEFINE_OPERATOR(int, CompareEq, (DeeObject *self, DeeObject *rhs)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_cmp && tp_self->tp_cmp->tp_compare_eq) ||
	          unlikely(DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_compare_eq))
		return DeeType_INVOKE_COMPAREEQ(tp_self, self, rhs);
	err_unimplemented_operator(tp_self, OPERATOR_EQ);
	return Dee_COMPARE_ERR;
}

/* Same as `DeeObject_CompareEq()', but automatically handles errors
 * that usually indicate that "lhs" and "rhs" cannot be compared by returning
 * either `-1' or `1' instead. The following errors get handled (so-long as
 * the effective `tp_trycompare_eq' callback doesn't end up throwing these):
 * - `Error.RuntimeError.NotImplemented' (`DeeError_NotImplemented'; Should indicate compare-not-implemented)
 * - `Error.TypeError'                   (`DeeError_TypeError';      Should indicate unsupported type combination)
 * - `Error.ValueError'                  (`DeeError_ValueError';     Should indicate unsupported instance combination)
 * @return: == -1: `lhs != rhs'
 * @return: == 0:  `lhs == rhs'
 * @return: == 1:  `lhs != rhs'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
DEFINE_OPERATOR(int, TryCompareEq, (DeeObject *self, DeeObject *rhs)) {
	LOAD_TP_SELF;
	if ((tp_self->tp_cmp && tp_self->tp_cmp->tp_trycompare_eq) ||
	    (DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_trycompare_eq))
		return DeeType_INVOKE_TRYCOMPAREEQ(tp_self, self, rhs);
	return -1; /* Implicit "NotImplemented" caught (would also be allowed to return "1" instead) */
}



#ifndef DEFINE_TYPED_OPERATORS

INTERN NONNULL((1)) bool DCALL
DeeType_InheritIterNext(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_iter_next) {
			if (!DeeType_InheritIterNext(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator iternext");
		self->tp_iter_next = base->tp_iter_next;
		return true;
	}
	return false;
}

/* Try to substitute default compare operators. */
PRIVATE NONNULL((1)) void DCALL
DeeType_SubstituteDefaultCompareOperators(DeeTypeObject *__restrict self) {
	struct type_cmp *cmp = (ASSERT(self->tp_cmp), self->tp_cmp);
	bool has_eq = cmp->tp_eq && !DeeType_IsDefaultEq(cmp->tp_eq);
	bool has_ne = cmp->tp_ne && !DeeType_IsDefaultNe(cmp->tp_ne);
	bool has_lo = cmp->tp_lo && !DeeType_IsDefaultLo(cmp->tp_lo);
	bool has_le = cmp->tp_le && !DeeType_IsDefaultLe(cmp->tp_le);
	bool has_gr = cmp->tp_gr && !DeeType_IsDefaultGr(cmp->tp_gr);
	bool has_ge = cmp->tp_ge && !DeeType_IsDefaultGe(cmp->tp_ge);
	bool has_compare_eq = cmp->tp_compare_eq && !DeeType_IsDefaultCompareEq(cmp->tp_compare_eq);
	bool has_compare = cmp->tp_compare && !DeeType_IsDefaultCompare(cmp->tp_compare);

	if (!cmp->tp_compare) {
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
			/* Sets and maps cannot be <=> compared:
			 *
			 *           +-----------+
			 *           |           |
			 *     +-----|-----+     |
			 *     |  A  |  B  |  C  |
			 *     +-----|-----+     |
			 *       ^   |           |
			 *       |   +-----------+
			 *       |       ^
			 *       |       |
			 *     SET_1   SET_2
			 *
			 * Here, SET_1 and SET_2 are neither sub-sets, nor super-set
			 * of each other, nor equal to each other:
			 * >> assert SET_1 != SET_2;
			 * >> assert !(SET_1 < SET_2);   // not subset
			 * >> assert !(SET_1 > SET_2);   // not superset
			 *
			 * As such, there is no valid value that could be returned
			 * by the rocketship operator (<=>), meaning that such a
			 * compare operation must not be substituted. */
			if (has_eq && has_lo) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndLo;
			} else if (has_eq && has_le) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndLe;
			} else if (has_eq && has_gr) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndGr;
			} else if (has_eq && has_ge) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithEqAndGe;
			} else if (has_ne && has_lo) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndLo;
			} else if (has_ne && has_le) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndLe;
			} else if (has_ne && has_gr) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndGr;
			} else if (has_ne && has_ge) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithNeAndGe;
			} else if (has_lo && has_gr) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithLoAndGr;
			} else if (has_le && has_ge) {
				cmp->tp_compare = &DeeObject_DefaultCompareWithLeAndGe;
			}
		}
	}

	if (!cmp->tp_compare_eq) {
		if (has_compare) {
			cmp->tp_compare_eq = cmp->tp_compare;
		} else if (has_eq) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithEq;
		} else if (has_ne) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithNe;
		} else if (has_lo && has_gr) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_compare_eq = &DeeObject_DefaultCompareEqWithLeAndGe;
		}
	}

	if (!cmp->tp_trycompare_eq) {
		if (has_compare_eq) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithCompareEq;
		} else if (has_eq) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithEq;
		} else if (has_ne) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithNe;
		} else if (has_compare) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithCompare;
		} else if (has_lo && has_gr) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_trycompare_eq = &DeeObject_DefaultTryCompareEqWithLeAndGe;
		}
	}

	if (!cmp->tp_eq) {
		if (has_compare_eq) {
			cmp->tp_eq = &DeeObject_DefaultEqWithCompareEq;
		} else if (has_ne) {
			cmp->tp_eq = &DeeObject_DefaultEqWithNe;
		} else if (has_lo && has_gr) {
			cmp->tp_eq = &DeeObject_DefaultEqWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_eq = &DeeObject_DefaultEqWithLeAndGe;
		} else if (cmp->tp_compare_eq) {
			cmp->tp_eq = &DeeObject_DefaultEqWithCompareEqDefault;
		}
	}

	if (!cmp->tp_ne) {
		if (has_compare_eq) {
			cmp->tp_ne = &DeeObject_DefaultNeWithCompareEq;
		} else if (has_eq) {
			cmp->tp_ne = &DeeObject_DefaultNeWithEq;
		} else if (has_lo && has_gr) {
			cmp->tp_ne = &DeeObject_DefaultNeWithLoAndGr;
		} else if (has_le && has_ge) {
			cmp->tp_ne = &DeeObject_DefaultNeWithLeAndGe;
		} else if (cmp->tp_compare_eq) {
			cmp->tp_ne = &DeeObject_DefaultNeWithCompareEqDefault;
		}
	}

	if (!cmp->tp_lo) {
		if (has_compare) {
			cmp->tp_lo = &DeeObject_DefaultLoWithCompare;
		} else if (has_ge) {
			cmp->tp_lo = &DeeObject_DefaultLoWithGe;
		} else if (cmp->tp_compare) {
			cmp->tp_lo = &DeeObject_DefaultLoWithCompareDefault;
		}
	}

	if (!cmp->tp_le) {
		if (has_compare) {
			cmp->tp_le = &DeeObject_DefaultLeWithCompare;
		} else if (has_gr) {
			cmp->tp_le = &DeeObject_DefaultLeWithGr;
		} else if (cmp->tp_compare) {
			cmp->tp_le = &DeeObject_DefaultLeWithCompareDefault;
		}
	}

	if (!cmp->tp_gr) {
		if (has_compare) {
			cmp->tp_gr = &DeeObject_DefaultGrWithCompare;
		} else if (has_le) {
			cmp->tp_gr = &DeeObject_DefaultGrWithLe;
		} else if (cmp->tp_compare) {
			cmp->tp_gr = &DeeObject_DefaultGrWithCompareDefault;
		}
	}

	if (!cmp->tp_ge) {
		if (has_compare) {
			cmp->tp_ge = &DeeObject_DefaultGeWithCompare;
		} else if (has_lo) {
			cmp->tp_ge = &DeeObject_DefaultGeWithLo;
		} else if (cmp->tp_compare) {
			cmp->tp_ge = &DeeObject_DefaultGeWithCompareDefault;
		}
	}
}

/* Sequence feature flags. */
enum seq_feature {
	FEAT_tp_iter,
	FEAT_tp_sizeob,
	FEAT_tp_contains,
	FEAT_tp_getitem,
	FEAT_tp_delitem,
	FEAT_tp_setitem,
	FEAT_tp_getrange,
	FEAT_tp_delrange,
	FEAT_tp_setrange,
	FEAT_tp_foreach,
	FEAT_tp_foreach_pair,
	FEAT_tp_enumerate,
	FEAT_tp_enumerate_index,
	FEAT_tp_bounditem,
	FEAT_tp_hasitem,
	FEAT_tp_size,
	FEAT_tp_getitem_index,
	FEAT_tp_delitem_index,
	FEAT_tp_setitem_index,
	FEAT_tp_bounditem_index,
	FEAT_tp_hasitem_index,
	FEAT_tp_getrange_index,
	FEAT_tp_delrange_index,
	FEAT_tp_setrange_index,
	FEAT_tp_getrange_index_n,
	FEAT_tp_delrange_index_n,
	FEAT_tp_setrange_index_n,
	/* FEAT_tp_size_fast, */ /* Default this one can't be substituted, it doesn't need a feature flag. */
	/* FEAT_tp_getitem_index_fast, */ /* Default this one can't be substituted, it doesn't need a feature flag. */
	FEAT_tp_trygetitem,
	FEAT_tp_trygetitem_index,
	FEAT_tp_trygetitem_string_hash,
	FEAT_tp_getitem_string_hash,
	FEAT_tp_delitem_string_hash,
	FEAT_tp_setitem_string_hash,
	FEAT_tp_bounditem_string_hash,
	FEAT_tp_hasitem_string_hash,
	FEAT_tp_trygetitem_string_len_hash,
	FEAT_tp_getitem_string_len_hash,
	FEAT_tp_delitem_string_len_hash,
	FEAT_tp_setitem_string_len_hash,
	FEAT_tp_bounditem_string_len_hash,
	FEAT_tp_hasitem_string_len_hash,
	FEAT_TP_COUNT
};

#define _SEQ_FEATURESET_NWORDS (CEILDIV(FEAT_TP_COUNT, sizeof(uintptr_t) * CHAR_BIT))
typedef uintptr_t seq_featureset_t[_SEQ_FEATURESET_NWORDS];
#define _seq_featureset_test_slot(i) ((i) / (sizeof(uintptr_t) * CHAR_BIT))
#define _seq_featureset_test_mask(i) ((uintptr_t)1 << ((i) % (sizeof(uintptr_t) * CHAR_BIT)))
#define seq_featureset_set(self, feat)  (void)((self)[_seq_featureset_test_slot(feat)] |= _seq_featureset_test_mask(feat))
#define seq_featureset_test(self, feat) ((self)[_seq_featureset_test_slot(feat)] & _seq_featureset_test_mask(feat))

LOCAL NONNULL((1)) void DCALL
seq_featureset_clear(seq_featureset_t self) {
	size_t i;
	for (i = 0; i < _SEQ_FEATURESET_NWORDS; ++i)
		self[i] = 0;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
seq_featureset_any(seq_featureset_t self) {
	size_t i;
	for (i = 0; i < _SEQ_FEATURESET_NWORDS; ++i) {
		if (self[i])
			return true;
	}
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_size(struct type_seq const *__restrict self) {
	return (self->tp_size != NULL) &&
	       !DeeType_IsDefaultSize(self->tp_size);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_sizeob(struct type_seq const *__restrict self) {
	return (self->tp_sizeob != NULL) &&
	       !DeeType_IsDefaultSizeOb(self->tp_sizeob);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_foreach(struct type_seq const *__restrict self) {
	return (self->tp_foreach != NULL) &&
	       !DeeType_IsDefaultForeach(self->tp_foreach);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem(struct type_seq const *__restrict self) {
	return (self->tp_getitem != NULL) &&
	       !DeeType_IsDefaultGetItem(self->tp_getitem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_index(struct type_seq const *__restrict self) {
	return (self->tp_getitem_index != NULL) &&
	       !DeeType_IsDefaultGetItemIndex(self->tp_getitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_getitem_string_hash != NULL) &&
	       !DeeType_IsDefaultGetItemStringHash(self->tp_getitem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_getitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultGetItemStringLenHash(self->tp_getitem_string_len_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem != NULL) &&
	       !DeeType_IsDefaultTryGetItem(self->tp_trygetitem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem_index(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem_index != NULL) &&
	       !DeeType_IsDefaultTryGetItemIndex(self->tp_trygetitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem_string_hash != NULL) &&
	       !DeeType_IsDefaultTryGetItemStringHash(self->tp_trygetitem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_trygetitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultTryGetItemStringLenHash(self->tp_trygetitem_string_len_hash);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem(struct type_seq const *__restrict self) {
	return (self->tp_bounditem != NULL) &&
	       !DeeType_IsDefaultBoundItem(self->tp_bounditem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_index(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_index != NULL) &&
	       !DeeType_IsDefaultBoundItemIndex(self->tp_bounditem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_string_hash != NULL) &&
	       !DeeType_IsDefaultBoundItemStringHash(self->tp_bounditem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultBoundItemStringLenHash(self->tp_bounditem_string_len_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem(struct type_seq const *__restrict self) {
	return (self->tp_hasitem != NULL) &&
	       !DeeType_IsDefaultHasItem(self->tp_hasitem);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem_index(struct type_seq const *__restrict self) {
	return (self->tp_hasitem_index != NULL) &&
	       !DeeType_IsDefaultHasItemIndex(self->tp_hasitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem_string_hash(struct type_seq const *__restrict self) {
	return (self->tp_hasitem_string_hash != NULL) &&
	       !DeeType_IsDefaultHasItemStringHash(self->tp_hasitem_string_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_hasitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_hasitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultHasItemStringLenHash(self->tp_hasitem_string_len_hash);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_delitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_delitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultDelItemStringLenHash(self->tp_delitem_string_len_hash);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_setitem_string_len_hash(struct type_seq const *__restrict self) {
	return (self->tp_setitem_string_len_hash != NULL) &&
	       !DeeType_IsDefaultSetItemStringLenHash(self->tp_setitem_string_len_hash);
}


/* Initialize "self" from features of "seq" */
PRIVATE NONNULL((1, 2)) void DCALL
seq_featureset_init(seq_featureset_t self, struct type_seq *__restrict seq, unsigned int seqclass) {
	seq_featureset_clear(self);
	/* Figure out what the type can do natively. */
	if (seq->tp_iter && !DeeType_IsDefaultIter(seq->tp_iter))
		seq_featureset_set(self, FEAT_tp_iter);
	if (Dee_type_seq_has_custom_tp_sizeob(seq))
		seq_featureset_set(self, FEAT_tp_sizeob);
	if (seq->tp_contains && !DeeType_IsDefaultContains(seq->tp_contains))
		seq_featureset_set(self, FEAT_tp_contains);
	if (seq->tp_getitem && !DeeType_IsDefaultGetItem(seq->tp_getitem))
		seq_featureset_set(self, FEAT_tp_getitem);
	if (seq->tp_delitem && !DeeType_IsDefaultDelItem(seq->tp_delitem))
		seq_featureset_set(self, FEAT_tp_delitem);
	if (seq->tp_setitem && !DeeType_IsDefaultSetItem(seq->tp_setitem))
		seq_featureset_set(self, FEAT_tp_setitem);
	if (seq->tp_getrange && !DeeType_IsDefaultGetRange(seq->tp_getrange))
		seq_featureset_set(self, FEAT_tp_getrange);
	if (seq->tp_delrange && !DeeType_IsDefaultDelRange(seq->tp_delrange))
		seq_featureset_set(self, FEAT_tp_delrange);
	if (seq->tp_setrange && !DeeType_IsDefaultSetRange(seq->tp_setrange))
		seq_featureset_set(self, FEAT_tp_setrange);
	if (Dee_type_seq_has_custom_tp_foreach(seq))
		seq_featureset_set(self, FEAT_tp_foreach);
	if (seq->tp_foreach_pair && !DeeType_IsDefaultForeachPair(seq->tp_foreach_pair)) {
		seq_featureset_set(self, FEAT_tp_foreach_pair);
		seq_featureset_set(self, FEAT_tp_enumerate);
		if (seq->tp_enumerate == NULL && seqclass == Dee_SEQCLASS_MAP) /* Binary compatible! (so cheat a little) */
			seq->tp_enumerate = seq->tp_foreach_pair;
	} else if (seq->tp_enumerate && !DeeType_IsDefaultEnumerate(seq->tp_enumerate)) {
		seq_featureset_set(self, FEAT_tp_enumerate);
	}
	if (seq->tp_enumerate_index && !DeeType_IsDefaultEnumerateIndex(seq->tp_enumerate_index))
		seq_featureset_set(self, FEAT_tp_enumerate_index);
	if (seq->tp_bounditem && !DeeType_IsDefaultBoundItem(seq->tp_bounditem))
		seq_featureset_set(self, FEAT_tp_bounditem);
	if (seq->tp_hasitem && !DeeType_IsDefaultHasItem(seq->tp_hasitem))
		seq_featureset_set(self, FEAT_tp_hasitem);
	if (Dee_type_seq_has_custom_tp_size(seq))
		seq_featureset_set(self, FEAT_tp_size);
	if (Dee_type_seq_has_custom_tp_getitem_index(seq))
		seq_featureset_set(self, FEAT_tp_getitem_index);
	if (seq->tp_delitem_index && !DeeType_IsDefaultDelItemIndex(seq->tp_delitem_index))
		seq_featureset_set(self, FEAT_tp_delitem_index);
	if (seq->tp_setitem_index && !DeeType_IsDefaultSetItemIndex(seq->tp_setitem_index))
		seq_featureset_set(self, FEAT_tp_setitem_index);
	if (Dee_type_seq_has_custom_tp_bounditem_index(seq))
		seq_featureset_set(self, FEAT_tp_bounditem_index);
	if (Dee_type_seq_has_custom_tp_hasitem_index(seq))
		seq_featureset_set(self, FEAT_tp_hasitem_index);
	if (seq->tp_getrange_index && !DeeType_IsDefaultGetRangeIndex(seq->tp_getrange_index))
		seq_featureset_set(self, FEAT_tp_getrange_index);
	if (seq->tp_delrange_index && !DeeType_IsDefaultDelRangeIndex(seq->tp_delrange_index))
		seq_featureset_set(self, FEAT_tp_delrange_index);
	if (seq->tp_setrange_index && !DeeType_IsDefaultSetRangeIndex(seq->tp_setrange_index))
		seq_featureset_set(self, FEAT_tp_setrange_index);
	if (seq->tp_getrange_index_n && !DeeType_IsDefaultGetRangeIndexN(seq->tp_getrange_index_n))
		seq_featureset_set(self, FEAT_tp_getrange_index_n);
	if (seq->tp_delrange_index_n && !DeeType_IsDefaultDelRangeIndexN(seq->tp_delrange_index_n))
		seq_featureset_set(self, FEAT_tp_delrange_index_n);
	if (seq->tp_setrange_index_n && !DeeType_IsDefaultSetRangeIndexN(seq->tp_setrange_index_n))
		seq_featureset_set(self, FEAT_tp_setrange_index_n);
	if (Dee_type_seq_has_custom_tp_trygetitem(seq))
		seq_featureset_set(self, FEAT_tp_trygetitem);
	if (Dee_type_seq_has_custom_tp_trygetitem_index(seq))
		seq_featureset_set(self, FEAT_tp_trygetitem_index);
	if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq))
		seq_featureset_set(self, FEAT_tp_trygetitem_string_hash);
	if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq))
		seq_featureset_set(self, FEAT_tp_getitem_string_hash);
	if (seq->tp_delitem_string_hash && !DeeType_IsDefaultDelItemStringHash(seq->tp_delitem_string_hash))
		seq_featureset_set(self, FEAT_tp_delitem_string_hash);
	if (seq->tp_setitem_string_hash && !DeeType_IsDefaultSetItemStringHash(seq->tp_setitem_string_hash))
		seq_featureset_set(self, FEAT_tp_setitem_string_hash);
	if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq) && seq->tp_bounditem_string_hash && !DeeType_IsDefaultBoundItemStringHash(seq->tp_bounditem_string_hash))
		seq_featureset_set(self, FEAT_tp_bounditem_string_hash);
	if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq) && seq->tp_hasitem_string_hash && !DeeType_IsDefaultHasItemStringHash(seq->tp_hasitem_string_hash))
		seq_featureset_set(self, FEAT_tp_hasitem_string_hash);
	if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq) && seq->tp_trygetitem_string_len_hash && !DeeType_IsDefaultTryGetItemStringLenHash(seq->tp_trygetitem_string_len_hash))
		seq_featureset_set(self, FEAT_tp_trygetitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq) && seq->tp_getitem_string_len_hash && !DeeType_IsDefaultGetItemStringLenHash(seq->tp_getitem_string_len_hash))
		seq_featureset_set(self, FEAT_tp_getitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_delitem_string_len_hash(seq) && seq->tp_delitem_string_len_hash && !DeeType_IsDefaultDelItemStringLenHash(seq->tp_delitem_string_len_hash))
		seq_featureset_set(self, FEAT_tp_delitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_setitem_string_len_hash(seq) && seq->tp_setitem_string_len_hash && !DeeType_IsDefaultSetItemStringLenHash(seq->tp_setitem_string_len_hash))
		seq_featureset_set(self, FEAT_tp_setitem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq) && seq->tp_bounditem_string_len_hash && !DeeType_IsDefaultBoundItemStringLenHash(seq->tp_bounditem_string_len_hash))
		seq_featureset_set(self, FEAT_tp_bounditem_string_len_hash);
	if (Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq) && seq->tp_hasitem_string_len_hash && !DeeType_IsDefaultHasItemStringLenHash(seq->tp_hasitem_string_len_hash))
		seq_featureset_set(self, FEAT_tp_hasitem_string_len_hash);
}

PRIVATE NONNULL((1, 2)) void DCALL
DeeSeqType_SubstituteDefaultOperators(DeeTypeObject *self, seq_featureset_t features, int seqclass) {
	struct type_seq *seq = self->tp_seq;
	ASSERT(seq);

	/* tp_iter */
	if (!seq->tp_iter) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_getitem) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_iter = &DeeSeq_DefaultIterWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_iter = &DeeObject_DefaultIterWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_foreach_pair)) {
			seq->tp_iter = &DeeObject_DefaultIterWithForeachPair;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_iter = &DeeObject_DefaultIterWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_iter = &DeeObject_DefaultIterWithEnumerateIndex;
		}
	}

	/* tp_size */
	if (!seq->tp_size) {
		if (seq_featureset_test(features, FEAT_tp_sizeob)) {
			seq->tp_size = &DeeObject_DefaultSizeWithSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_foreach_pair)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithForeachPair;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_size = &DeeSeq_DefaultSizeWithIter;
		}
	}

	/* tp_sizeob */
	if (!seq->tp_sizeob) {
		if (seq_featureset_test(features, FEAT_tp_size)) {
			seq->tp_sizeob = &DeeObject_DefaultSizeObWithSize;
		} else if (seq->tp_size) {
			seq->tp_sizeob = &DeeObject_DefaultSizeObWithSizeDefault;
		}
	}

	/* tp_foreach */
	if (!seq->tp_foreach) {
		if (seq_featureset_test(features, FEAT_tp_foreach_pair)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithForeachPair;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_getitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_foreach = &DeeObject_DefaultForeachWithIter;
		} else if (seq->tp_size &&
		           (seq_featureset_test(features, FEAT_tp_getitem) ||
		            seq_featureset_test(features, FEAT_tp_getitem_index) ||
		            seq_featureset_test(features, FEAT_tp_trygetitem)) &&
		           (seqclass == Dee_SEQCLASS_SEQ)) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault;
		} else if ((seq_featureset_test(features, FEAT_tp_getitem) ||
		            seq_featureset_test(features, FEAT_tp_getitem_index)) &&
		           (seqclass == Dee_SEQCLASS_SEQ)) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithGetItemIndexDefault;
		}
	}

	/* tp_contains (for maps) */
	if (!seq->tp_contains && seqclass == Dee_SEQCLASS_MAP) {
		if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_contains = &DeeMap_DefaultContainsWithEnumerate;
		}
	}

	/* tp_getitem */
	if (!seq->tp_getitem) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeSeq_DefaultGetItemWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeSeq_DefaultGetItemWithTryGetItemAndSize;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			       seq->tp_getitem = &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndexOb;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeSeq_DefaultGetItemWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seqclass != Dee_SEQCLASS_SEQ) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem = &DeeMap_DefaultGetItemWithEnumerate;
		}
	}

	/* tp_getitem_index */
	if (!seq->tp_getitem_index) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) &&
		           seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) &&
		           seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithSizeAndTryGetItemIndexOb;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) &&
		           seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSize;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) &&
		           seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItem;
		} else if (seq->tp_foreach && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getitem_index = &DeeSeq_DefaultGetItemIndexWithForeachDefault;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_index = &DeeMap_DefaultGetItemIndexWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
		} else if (seq->tp_getitem) {
			seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithGetItemDefault;
		}
	}

	/* tp_foreach */
	if (!seq->tp_foreach) {
		if (seq->tp_size && seq->tp_getitem_index) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault;
		} else if (seq->tp_getitem) {
			seq->tp_foreach = &DeeSeq_DefaultForeachWithGetItemIndexDefault;
		}
	}

	/* tp_contains */
	if (!seq->tp_contains) {
		if (seqclass == Dee_SEQCLASS_MAP && seq->tp_enumerate) {
			seq->tp_contains = &DeeMap_DefaultContainsWithEnumerateDefault;
		} else if (seqclass == Dee_SEQCLASS_SEQ && seq->tp_foreach) {
			seq->tp_contains = &DeeSeq_DefaultContainsWithForeachDefault;
		}
	}

	/* tp_foreach_pair */
	if (!seq->tp_foreach_pair) {
		if (seq_featureset_test(features, FEAT_tp_foreach)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithIter;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_foreach_pair = &DeeMap_DefaultForeachPairWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_foreach_pair = &DeeMap_DefaultForeachPairWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerateIndex;
		} else if (seq->tp_foreach) {
			seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithForeachDefault;
		}
	}

	/* tp_enumerate */
	if (!seq->tp_enumerate) {
		if (seq_featureset_test(features, FEAT_tp_enumerate_index)) {
			seq->tp_enumerate = &DeeObject_DefaultEnumerateWithEnumerateIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) && seq_featureset_test(features, FEAT_tp_getitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_foreach) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithCounterAndForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithCounterAndIter;
		} else if (seq_featureset_test(features, FEAT_tp_iter) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_enumerate = &DeeMap_DefaultEnumerateWithIter;
		} else if (seq->tp_size && seq->tp_getitem_index && seqclass == Dee_SEQCLASS_SEQ) {
			if (seq->tp_size == &DeeSeq_DefaultSizeWithForeachPair ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithForeach ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithIter ||
			    seq->tp_getitem_index == &DeeSeq_DefaultGetItemIndexWithForeachDefault) {
				seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithCounterAndForeachDefault;
			} else {
				seq->tp_enumerate = &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault;
			}
		} else if (seq->tp_foreach_pair && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_enumerate = &DeeMap_DefaultEnumerateWithForeachPairDefault;
		}
	}

	/* tp_getitem */
	if (!seq->tp_getitem) {
		if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem = &DeeMap_DefaultGetItemWithEnumerateDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemIndexDefault;
		}
	}

	/* tp_getitem_index */
	if (!seq->tp_getitem_index && seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP)
		seq->tp_getitem_index = &DeeMap_DefaultGetItemIndexWithEnumerateDefault;

	/* tp_delitem_index */
	if (!seq->tp_delitem_index && seq_featureset_test(features, FEAT_tp_delitem))
		seq->tp_delitem_index = &DeeObject_DefaultDelItemIndexWithDelItem;

	/* tp_setitem_index */
	if (!seq->tp_setitem_index && seq_featureset_test(features, FEAT_tp_setitem))
		seq->tp_setitem_index = &DeeObject_DefaultSetItemIndexWithSetItem;

	/* tp_enumerate_index */
	if (!seq->tp_enumerate_index) {
		if (seq_featureset_test(features, FEAT_tp_enumerate)) {
			seq->tp_enumerate_index = &DeeObject_DefaultEnumerateIndexWithEnumerate;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) && seq_featureset_test(features, FEAT_tp_getitem) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeObAndGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_foreach) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach;
		} else if (seq_featureset_test(features, FEAT_tp_iter) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithCounterAndIter;
		} else if (seq->tp_size && seq->tp_getitem_index && seqclass == Dee_SEQCLASS_SEQ) {
			if (seq->tp_size == &DeeSeq_DefaultSizeWithForeachPair ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithForeach ||
			    seq->tp_size == &DeeSeq_DefaultSizeWithIter ||
			    seq->tp_getitem_index == &DeeSeq_DefaultGetItemIndexWithForeachDefault) {
				seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault;
			} else {
				seq->tp_enumerate_index = &DeeSeq_DefaultEnumerateIndexWithSizeDefaultAndGetItemIndexDefault;
			}
		} else if (seq->tp_enumerate) {
			seq->tp_enumerate_index = &DeeObject_DefaultEnumerateIndexWithEnumerateDefault;
		}
	}

	/* tp_setrange_index */
	if (!seq->tp_setrange_index) {
		if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_setrange_index = &DeeObject_DefaultSetRangeIndexWithSetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			if (seq_featureset_test(features, FEAT_tp_size) &&
			    seq_featureset_test(features, FEAT_tp_delitem_index) &&
			    seq_featureset_test(features, FEAT_tp_setitem_index)) {
				seq->tp_setrange_index = &DeeSeq_DefaultSetRangeIndexWithSizeAndDelItemIndexAndSetItemIndex;
			} else if (seq->tp_size && seq->tp_setitem_index && seq->tp_delitem_index) {
				seq->tp_setrange_index = &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault;
			} else if (seq_featureset_test(features, FEAT_tp_size) &&
			           seq_featureset_test(features, FEAT_tp_setitem_index)) {
				seq->tp_setrange_index = &DeeSeq_DefaultSetRangeIndexWithSizeAndSetItemIndex;
			} else if (seq->tp_size && seq->tp_setitem_index) {
				seq->tp_setrange_index = &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndSetItemIndexDefault;
			}
		}
	}

	/* tp_setrange_index_n */
	if (!seq->tp_setrange_index_n) {
		if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_setrange_index_n = &DeeObject_DefaultSetRangeIndexNWithSetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			if (seq_featureset_test(features, FEAT_tp_size) &&
			    seq_featureset_test(features, FEAT_tp_setrange_index)) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetRangeIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_setrange_index)) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetRangeIndex;
			} else if (seq_featureset_test(features, FEAT_tp_size) &&
			           seq_featureset_test(features, FEAT_tp_delitem_index) &&
			           seq_featureset_test(features, FEAT_tp_setitem_index)) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeAndDelItemIndexAndSetItemIndex;
			} else if (seq->tp_size && seq->tp_setitem_index && seq->tp_delitem_index) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndDelItemIndexDefaultAndSetItemIndexDefault;
			} else if (seq_featureset_test(features, FEAT_tp_size) &&
			           seq_featureset_test(features, FEAT_tp_setitem_index)) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeAndSetItemIndex;
			} else if (seq->tp_size && seq->tp_setitem_index) {
				seq->tp_setrange_index_n = &DeeSeq_DefaultSetRangeIndexNWithSizeDefaultAndSetItemIndexDefault;
			}
		}
	}

	/* tp_setrange */
	if (!seq->tp_setrange) {
		if (seq_featureset_test(features, FEAT_tp_setrange_index) &&
		    seq_featureset_test(features, FEAT_tp_setrange_index_n)) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN;
		} else if (seq->tp_setrange_index || seq->tp_setrange_index_n) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault;
		}
	}

	/* tp_delrange_index */
	if (!seq->tp_delrange_index) {
		if (seq_featureset_test(features, FEAT_tp_delrange)) {
			seq->tp_delrange_index = &DeeObject_DefaultDelRangeIndexWithDelRange;
		} else if (seq_featureset_test(features, FEAT_tp_setrange_index)) {
			seq->tp_delrange_index = &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone;
		} else if (seq->tp_setrange_index) {
			seq->tp_delrange_index = &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault;
		}
	}

	/* tp_delrange_index_n */
	if (!seq->tp_delrange_index_n) {
		if (seq_featureset_test(features, FEAT_tp_delrange)) {
			seq->tp_delrange_index_n = &DeeObject_DefaultDelRangeIndexNWithDelRange;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq_featureset_test(features, FEAT_tp_delrange_index) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSizeAndDelRangeIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_delrange_index) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSizeDefaultAndDelRangeIndex;
		} else if (seq_featureset_test(features, FEAT_tp_setrange_index_n)) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone;
		} else if (seq->tp_setrange_index_n) {
			seq->tp_delrange_index_n = &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault;
		}
	}

	/* tp_delrange */
	if (!seq->tp_delrange) {
		if (seq_featureset_test(features, FEAT_tp_delrange_index) &&
		    seq_featureset_test(features, FEAT_tp_delrange_index_n)) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN;
		} else if (seq->tp_delrange_index || seq->tp_delrange_index_n) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault;
		} else if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNone;
		} else if (seq->tp_setrange) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNoneDefault;
		}
	}

	/* tp_delitem_index */
	if (!seq->tp_delitem_index) {
		if (seq->tp_delrange_index && seqclass == Dee_SEQCLASS_SEQ)
			seq->tp_delitem_index = &DeeSeq_DefaultDelItemIndexWithDelRangeIndexDefault;
	}

	/* tp_setitem_index */
	if (!seq->tp_setitem_index) {
		if (seq->tp_setrange_index && seqclass == Dee_SEQCLASS_SEQ)
			seq->tp_setitem_index = &DeeSeq_DefaultSetItemIndexWithSetRangeIndexDefault;
	}

	/* tp_delitem */
	if (!seq->tp_delitem) {
		if (seq_featureset_test(features, FEAT_tp_delitem_index)) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemIndex;
		} else if (seq->tp_delitem_index) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemIndexDefault;
		} else if (seq_featureset_test(features, FEAT_tp_delitem_string_hash)) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_delitem_string_len_hash)) {
			seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringLenHash;
		}
	}

	/* tp_setitem */
	if (!seq->tp_setitem) {
		if (seq_featureset_test(features, FEAT_tp_setitem_index)) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemIndex;
		} else if (seq->tp_setitem_index) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemIndexDefault;
		} else if (seq_featureset_test(features, FEAT_tp_setitem_string_hash)) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_setitem_string_len_hash)) {
			seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringLenHash;
		}
	}

	/* tp_getrange_index */
	if (!seq->tp_getrange_index) {
		if (seq_featureset_test(features, FEAT_tp_getrange)) {
			seq->tp_getrange_index = &DeeObject_DefaultGetRangeIndexWithGetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItem;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndGetItemIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_iter)) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIter;
			} else if (seq->tp_size && seq->tp_iter) {
				seq->tp_getrange_index = &DeeSeq_DefaultGetRangeIndexWithSizeDefaultAndIterDefault;
			}
		}
	}

	/* tp_getrange_index_n */
	if (!seq->tp_getrange_index_n) {
		if (seq_featureset_test(features, FEAT_tp_getrange)) {
			seq->tp_getrange_index_n = &DeeObject_DefaultGetRangeIndexNWithGetRange;
		} else if (seqclass == Dee_SEQCLASS_SEQ) {
			if (seq_featureset_test(features, FEAT_tp_size) &&
			    seq_featureset_test(features, FEAT_tp_getrange_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetRangeIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getrange_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetRangeIndex;
			} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex;
			} else if ((seq_featureset_test(features, FEAT_tp_size) ||
			            seq_featureset_test(features, FEAT_tp_sizeob)) &&
			           seq_featureset_test(features, FEAT_tp_getitem)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItem;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem_index)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndGetItemIndex;
			} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_iter)) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIter;
			} else if (seq->tp_size && seq->tp_iter) {
				seq->tp_getrange_index_n = &DeeSeq_DefaultGetRangeIndexNWithSizeDefaultAndIterDefault;
			}
		}
	}

	/* tp_getrange */
	if (!seq->tp_getrange) {
		if (seq_featureset_test(features, FEAT_tp_getrange_index) &&
		    seq_featureset_test(features, FEAT_tp_getrange_index_n)) {
			seq->tp_getrange = &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_getitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getrange = &DeeSeq_DefaultGetRangeWithSizeDefaultAndGetItemIndex;
		} else if (seq->tp_size && seq_featureset_test(features, FEAT_tp_trygetitem_index) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getrange = &DeeSeq_DefaultGetRangeWithSizeDefaultAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) &&
		           seq_featureset_test(features, FEAT_tp_getitem) &&
		           seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_getrange = &DeeSeq_DefaultGetRangeWithSizeObAndGetItem;
		} else if (seq->tp_getrange_index && seq->tp_getrange_index_n) {
			seq->tp_getrange = &DeeObject_DefaultGetRangeWithGetRangeIndexDefaultAndGetRangeIndexNDefault;
		}
	}

	/* tp_delrange */
	if (!seq->tp_delrange) {
		if (seq_featureset_test(features, FEAT_tp_delrange_index) &&
		    seq_featureset_test(features, FEAT_tp_delrange_index_n)) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN;
		} else if (seq_featureset_test(features, FEAT_tp_setrange)) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNone;
		} else if (seq->tp_setrange) {
			seq->tp_delrange = &DeeSeq_DefaultDelRangeWithSetRangeNoneDefault;
		} else if (seq->tp_delrange_index && seq->tp_delrange_index_n) {
			seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexDefaultAndDelRangeIndexNDefault;
		}
	}

	/* tp_setrange */
	if (!seq->tp_setrange) {
		if (seq_featureset_test(features, FEAT_tp_setrange_index) &&
		    seq_featureset_test(features, FEAT_tp_setrange_index_n)) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN;
		} else if (seq->tp_setrange_index && seq->tp_setrange_index_n) {
			seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexDefaultAndSetRangeIndexNDefault;
		}
	}

	/* tp_bounditem_index */
	if (!seq->tp_bounditem_index) {
		if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_size) &&
		           seq->tp_getitem_index_fast && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_index = &DeeMap_DefaultBoundItemIndexWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem_index = &DeeSeq_DefaultBoundItemIndexWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem_index = &DeeSeq_DefaultBoundItemIndexWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_index = &DeeMap_DefaultBoundItemIndexWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_index = &DeeMap_DefaultBoundItemIndexWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithErrorRequiresString;
		} else if (seq->tp_getitem_index) {
			seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault;
		}
	}

	/* tp_bounditem */
	if (!seq->tp_bounditem) {
		if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			/* TODO: If it's a map and implements one of the "hasitem" operators, assume that
			 *       items cannot be unbound and emulate "bounditem" using hasitem only! */
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem = &DeeMap_DefaultBoundItemWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem = &DeeMap_DefaultBoundItemWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem = &DeeMap_DefaultBoundItemWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem = &DeeSeq_DefaultBoundItemWithTryGetItemAndSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index) && seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_bounditem = &DeeSeq_DefaultBoundItemWithSizeAndTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHash;
		} else if (seq->tp_getitem) {
			seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemDefault;
		}
	}

	/* tp_hasitem_index */
	if (!seq->tp_hasitem_index) {
		if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_index = &DeeMap_DefaultHasItemIndexWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSize;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithErrorRequiresString;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_index = &DeeMap_DefaultHasItemIndexWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_index = &DeeMap_DefaultHasItemIndexWithEnumerateDefault;
		} else if (seq->tp_size && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSizeDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItemIndexDefault;
		}
	}

	/* tp_hasitem */
	if (!seq->tp_hasitem) {
		if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem = &DeeMap_DefaultHasItemWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_sizeob) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem = &DeeSeq_DefaultHasItemWithSizeOb;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seqclass == Dee_SEQCLASS_SEQ) {
			seq->tp_hasitem = &DeeSeq_DefaultHasItemWithSize;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem = &DeeMap_DefaultHasItemWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem = &DeeMap_DefaultHasItemWithEnumerateDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemDefault;
		}
	}

	/* tp_trygetitem */
	if (!seq->tp_trygetitem) {
		if (seq_featureset_test(features, FEAT_tp_trygetitem_index)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem = &DeeMap_DefaultTryGetItemWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem = &DeeMap_DefaultTryGetItemWithEnumerateDefault;
		} else if (seq->tp_getitem) {
			seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemDefault;
		}
	}

	/* tp_trygetitem_index */
	if (!seq->tp_trygetitem_index) {
		if (seq_featureset_test(features, FEAT_tp_size) && seq->tp_getitem_index_fast) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItemIndex;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_hash) ||
		           seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_index = &DeeMap_DefaultTryGetItemIndexWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_index = &DeeMap_DefaultTryGetItemIndexWithEnumerateDefault;
		} else if (seq->tp_getitem_index) {
			seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItemIndexDefault;
		}
	}

	/* tp_trygetitem_string_hash */
	if (!seq->tp_trygetitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_hash = &DeeMap_DefaultTryGetItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_hash = &DeeMap_DefaultTryGetItemStringHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithErrorRequiresInt;
		} else if (seq->tp_trygetitem) {
			seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemDefault;
		}
	}

	/* tp_trygetitem_string_len_hash */
	if (!seq->tp_trygetitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_len_hash = &DeeMap_DefaultTryGetItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_trygetitem_string_len_hash = &DeeMap_DefaultTryGetItemStringLenHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithErrorRequiresInt;
		} else if (seq->tp_trygetitem) {
			seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemDefault;
		}
	}

	/* tp_getitem_string_hash */
	if (!seq->tp_getitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithErrorRequiresInt;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_hash = &DeeMap_DefaultGetItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_hash = &DeeMap_DefaultGetItemStringHashWithEnumerateDefault;
		} else if (seq->tp_getitem) {
			seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemDefault;
		}
	}

	/* tp_getitem_string_len_hash */
	if (!seq->tp_getitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithErrorRequiresInt;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_len_hash = &DeeMap_DefaultGetItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_getitem_string_len_hash = &DeeMap_DefaultGetItemStringLenHashWithEnumerateDefault;
		} else if (seq->tp_getitem) {
			seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemDefault;
		}
	}

	/* tp_bounditem_string_hash */
	if (!seq->tp_bounditem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_hash = &DeeMap_DefaultBoundItemStringHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_hash = &DeeMap_DefaultBoundItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_hash = &DeeMap_DefaultBoundItemStringHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt;
		} else if (seq->tp_bounditem) {
			seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemDefault;
		}
	}

	/* tp_bounditem_string_len_hash */
	if (!seq->tp_bounditem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash) && seq_featureset_test(features, FEAT_tp_hasitem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem) && seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_len_hash = &DeeMap_DefaultBoundItemStringLenHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) || seq->tp_getitem_index_fast) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_len_hash = &DeeMap_DefaultBoundItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_bounditem_string_len_hash = &DeeMap_DefaultBoundItemStringLenHashWithEnumerateDefault;
		} else if (seq->tp_bounditem) {
			seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault;
		}
	}

	/* tp_hasitem_string_hash */
	if (!seq->tp_hasitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_hash = &DeeMap_DefaultHasItemStringHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_hash = &DeeMap_DefaultHasItemStringHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_hash = &DeeMap_DefaultHasItemStringHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) ||
		           seq->tp_getitem_index_fast) {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithErrorRequiresInt;
		} else {
			seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemDefault;
		}
	}

	/* tp_hasitem_string_len_hash */
	if (!seq->tp_hasitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_hasitem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem_string_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem_string_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_contains) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_len_hash = &DeeMap_DefaultHasItemStringLenHashWithContains;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_len_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_getitem_string_hash)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItem;
		} else if (seq_featureset_test(features, FEAT_tp_bounditem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItem;
		} else if (seq_featureset_test(features, FEAT_tp_trygetitem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_getitem)) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItem;
		} else if (seq_featureset_test(features, FEAT_tp_enumerate) && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_len_hash = &DeeMap_DefaultHasItemStringLenHashWithEnumerate;
		} else if (seq->tp_enumerate && seqclass == Dee_SEQCLASS_MAP) {
			seq->tp_hasitem_string_len_hash = &DeeMap_DefaultHasItemStringLenHashWithEnumerateDefault;
		} else if (seq_featureset_test(features, FEAT_tp_hasitem_index) ||
		           seq_featureset_test(features, FEAT_tp_bounditem_index) ||
		           seq_featureset_test(features, FEAT_tp_getitem_index) ||
		           seq_featureset_test(features, FEAT_tp_trygetitem_index) ||
		           seq->tp_getitem_index_fast) {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt;
		} else {
			seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemDefault;
		}
	}

	/* tp_delitem_string_hash */
	if (!seq->tp_delitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_delitem_string_len_hash)) {
			seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_delitem)) {
			seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItem;
		} else if (seq->tp_delitem) {
			seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithErrorRequiresInt;
		}
	}

	/* tp_delitem_string_len_hash */
	if (!seq->tp_delitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_delitem_string_hash)) {
			seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_delitem)) {
			seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItem;
		} else if (seq->tp_delitem) {
			seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithErrorRequiresInt;
		}
	}

	/* tp_setitem_string_hash */
	if (!seq->tp_setitem_string_hash) {
		if (seq_featureset_test(features, FEAT_tp_setitem_string_len_hash)) {
			seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash;
		} else if (seq_featureset_test(features, FEAT_tp_setitem)) {
			seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItem;
		} else if (seq->tp_setitem) {
			seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithErrorRequiresInt;
		}
	}

	/* tp_setitem_string_len_hash */
	if (!seq->tp_setitem_string_len_hash) {
		if (seq_featureset_test(features, FEAT_tp_setitem_string_hash)) {
			seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash;
		} else if (seq_featureset_test(features, FEAT_tp_setitem)) {
			seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItem;
		} else if (seq->tp_setitem) {
			seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithErrorRequiresInt;
		}
	}

	/* tp_size_fast (simply set to return `(size_t)-1') */
	if (!seq->tp_size_fast)
		seq->tp_size_fast = &DeeObject_DefaultSizeFastWithErrorNotFast;
}

INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeAndGetItemIndexFast = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeAndGetItemIndexFast,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeAndTryGetItemIndex = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeAndTryGetItemIndex,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeAndGetItemIndex = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeAndGetItemIndex,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeAndGetItemIndex,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithSizeObAndGetItem = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithSizeObAndGetItem,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithSizeObAndGetItem,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithSizeObAndGetItem,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSeq_DefaultCmpWithForeachDefault = {
	/* .tp_hash          = */ &DeeSeq_DefaultHashWithForeachDefault,
	/* .tp_compare_eq    = */ &DeeSeq_DefaultCompareEqWithForeachDefault,
	/* .tp_compare       = */ &DeeSeq_DefaultCompareWithForeachDefault,
	/* .tp_trycompare_eq = */ &DeeSeq_DefaultTryCompareEqWithForeachDefault,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeObject_DefaultLoWithCompareDefault,
	/* .tp_le            = */ &DeeObject_DefaultLeWithCompareDefault,
	/* .tp_gr            = */ &DeeObject_DefaultGrWithCompareDefault,
	/* .tp_ge            = */ &DeeObject_DefaultGeWithCompareDefault,
};
INTERN struct type_cmp DeeSet_DefaultCmpWithForeachDefault = {
	/* .tp_hash          = */ &DeeSet_DefaultHashWithForeachDefault,
	/* .tp_compare_eq    = */ &DeeSet_DefaultCompareEqWithForeachDefault,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &DeeSet_DefaultTryCompareEqWithForeachDefault,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeSet_DefaultLoWithForeachDefault,
	/* .tp_le            = */ &DeeSet_DefaultLeWithForeachDefault,
	/* .tp_gr            = */ &DeeSet_DefaultGrWithForeachDefault,
	/* .tp_ge            = */ &DeeSet_DefaultGeWithForeachDefault,
};
INTERN struct type_cmp DeeMap_DefaultCmpWithForeachPairDefault = {
	/* .tp_hash          = */ &DeeMap_DefaultHashWithForeachPairDefault,
	/* .tp_compare_eq    = */ &DeeMap_DefaultCompareEqWithForeachPairDefault,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &DeeMap_DefaultTryCompareEqWithForeachPairDefault,
	/* .tp_eq            = */ &DeeObject_DefaultEqWithCompareEqDefault,
	/* .tp_ne            = */ &DeeObject_DefaultNeWithCompareEqDefault,
	/* .tp_lo            = */ &DeeMap_DefaultLoWithForeachPairDefault,
	/* .tp_le            = */ &DeeMap_DefaultLeWithForeachPairDefault,
	/* .tp_gr            = */ &DeeMap_DefaultGrWithForeachPairDefault,
	/* .tp_ge            = */ &DeeMap_DefaultGeWithForeachPairDefault,
};


/* Inherit OPERATOR_ITER, OPERATOR_SIZE and OPERTOR_GETITEM for
 * a type with `DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ' */
PRIVATE NONNULL((1)) bool DCALL
DeeType_InheritSeqOperators(DeeTypeObject *__restrict self, unsigned int seqclass) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_seq = self->tp_seq;
	if (base_seq) {
		seq_featureset_t features;
		seq_featureset_init(features, base_seq, seqclass);

		/* If the type is implementing sequence features, auto-complete those
		 * features and don't try to import operators from base classes (when
		 * extending sequence types, sequence operators must be inherited all-
		 * at-once) */
		if (seq_featureset_any(features) || base_seq->tp_getitem_index_fast) {
			DeeSeqType_SubstituteDefaultOperators(self, features, seqclass);
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (DeeType_GetSeqClass(base) != seqclass)
			break; /* Stop when base of a different sequence class is reached. */
		if (!DeeType_InheritSeqOperators(base, seqclass))
			continue;
		base_seq = base->tp_seq;
		ASSERT(base_seq);
		LOG_INHERIT(base, self, "operator <sequence>");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSeqOperators(origin, seqclass);
			self->tp_seq->tp_iter                       = base_seq->tp_iter;
			self->tp_seq->tp_sizeob                     = base_seq->tp_sizeob;
			self->tp_seq->tp_contains                   = base_seq->tp_contains;
			self->tp_seq->tp_getitem                    = base_seq->tp_getitem;
			self->tp_seq->tp_delitem                    = base_seq->tp_delitem;
			self->tp_seq->tp_setitem                    = base_seq->tp_setitem;
			self->tp_seq->tp_getrange                   = base_seq->tp_getrange;
			self->tp_seq->tp_delrange                   = base_seq->tp_delrange;
			self->tp_seq->tp_setrange                   = base_seq->tp_setrange;
			self->tp_seq->tp_nsi                        = base_seq->tp_nsi;
			self->tp_seq->tp_foreach                    = base_seq->tp_foreach;
			self->tp_seq->tp_foreach_pair               = base_seq->tp_foreach_pair;
			self->tp_seq->tp_enumerate                  = base_seq->tp_enumerate;
			self->tp_seq->tp_enumerate_index            = base_seq->tp_enumerate_index;
			self->tp_seq->tp_bounditem                  = base_seq->tp_bounditem;
			self->tp_seq->tp_hasitem                    = base_seq->tp_hasitem;
			self->tp_seq->tp_size                       = base_seq->tp_size;
			self->tp_seq->tp_getitem_index              = base_seq->tp_getitem_index;
			self->tp_seq->tp_delitem_index              = base_seq->tp_delitem_index;
			self->tp_seq->tp_setitem_index              = base_seq->tp_setitem_index;
			self->tp_seq->tp_bounditem_index            = base_seq->tp_bounditem_index;
			self->tp_seq->tp_hasitem_index              = base_seq->tp_hasitem_index;
			self->tp_seq->tp_getrange_index             = base_seq->tp_getrange_index;
			self->tp_seq->tp_delrange_index             = base_seq->tp_delrange_index;
			self->tp_seq->tp_setrange_index             = base_seq->tp_setrange_index;
			self->tp_seq->tp_getrange_index_n           = base_seq->tp_getrange_index_n;
			self->tp_seq->tp_delrange_index_n           = base_seq->tp_delrange_index_n;
			self->tp_seq->tp_setrange_index_n           = base_seq->tp_setrange_index_n;
			self->tp_seq->tp_size_fast                  = base_seq->tp_size_fast;
			self->tp_seq->tp_getitem_index_fast         = base_seq->tp_getitem_index_fast;
			self->tp_seq->tp_trygetitem                 = base_seq->tp_trygetitem;
			self->tp_seq->tp_trygetitem_string_hash     = base_seq->tp_trygetitem_string_hash;
			self->tp_seq->tp_getitem_string_hash        = base_seq->tp_getitem_string_hash;
			self->tp_seq->tp_delitem_string_hash        = base_seq->tp_delitem_string_hash;
			self->tp_seq->tp_setitem_string_hash        = base_seq->tp_setitem_string_hash;
			self->tp_seq->tp_bounditem_string_hash      = base_seq->tp_bounditem_string_hash;
			self->tp_seq->tp_hasitem_string_hash        = base_seq->tp_hasitem_string_hash;
			self->tp_seq->tp_trygetitem_string_len_hash = base_seq->tp_trygetitem_string_len_hash;
			self->tp_seq->tp_getitem_string_len_hash    = base_seq->tp_getitem_string_len_hash;
			self->tp_seq->tp_delitem_string_len_hash    = base_seq->tp_delitem_string_len_hash;
			self->tp_seq->tp_setitem_string_len_hash    = base_seq->tp_setitem_string_len_hash;
			self->tp_seq->tp_bounditem_string_len_hash  = base_seq->tp_bounditem_string_len_hash;
			self->tp_seq->tp_hasitem_string_len_hash    = base_seq->tp_hasitem_string_len_hash;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}


INTERN NONNULL((1)) bool DCALL
DeeType_InheritIter(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_iter != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_iter) {
			if (base_seq->tp_foreach == NULL)
				base_seq->tp_foreach = &DeeObject_DefaultForeachWithIter;
			if (base_seq->tp_foreach_pair == NULL)
				base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithIter;
			return true;
		} else if (base_seq->tp_foreach) {
			base_seq->tp_iter = &DeeObject_DefaultIterWithForeach;
			if (base_seq->tp_foreach_pair == NULL)
				base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithForeach;
			return true;
		} else if (base_seq->tp_foreach_pair) {
			base_seq->tp_iter    = &DeeObject_DefaultIterWithForeachPair;
			base_seq->tp_foreach = &DeeObject_DefaultForeachWithForeachPair;
			return true;
		} else if (base_seq->tp_enumerate) {
			base_seq->tp_iter         = &DeeObject_DefaultIterWithEnumerate;
			base_seq->tp_foreach      = &DeeObject_DefaultForeachWithEnumerate;
			base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerate;
			if (base_seq->tp_enumerate_index == NULL)
				base_seq->tp_enumerate_index = &DeeObject_DefaultEnumerateIndexWithEnumerate;
		} else if (base_seq->tp_enumerate_index) {
			base_seq->tp_iter         = &DeeObject_DefaultIterWithEnumerateIndex;
			base_seq->tp_foreach      = &DeeObject_DefaultForeachWithEnumerateIndex;
			base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithEnumerateIndex;
			base_seq->tp_enumerate    = &DeeObject_DefaultEnumerateWithEnumerateIndex;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_iter ||
		                         !base_seq->tp_foreach ||
		                         !base_seq->tp_foreach_pair)) {
			if (!DeeType_InheritIter(base))
				continue;
		}
		base_seq = base->tp_seq;
		LOG_INHERIT(base, self, "operator iter");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritIter(origin);
			self->tp_seq->tp_iter         = base_seq->tp_iter;
			self->tp_seq->tp_foreach      = base_seq->tp_foreach;
			self->tp_seq->tp_foreach_pair = base_seq->tp_foreach_pair;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritSize(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_sizeob != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_size) {
			if (base_seq->tp_sizeob == NULL)
				base_seq->tp_sizeob = &DeeObject_DefaultSizeObWithSize;
			if (base_seq->tp_size_fast == NULL)
				base_seq->tp_size_fast = &DeeObject_DefaultSizeFastWithErrorNotFast;
			return true;
		} else if (base_seq->tp_sizeob) {
			if (base_seq->tp_size == NULL)
				base_seq->tp_size = &DeeObject_DefaultSizeWithSizeOb;
			if (base_seq->tp_size_fast == NULL)
				base_seq->tp_size_fast = &DeeObject_DefaultSizeFastWithErrorNotFast;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_size ||
		                         !base_seq->tp_sizeob ||
		                         !base_seq->tp_size_fast)) {
			if (!DeeType_InheritSize(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator size");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSize(origin);
			self->tp_seq->tp_size      = base_seq->tp_size;
			self->tp_seq->tp_sizeob    = base_seq->tp_sizeob;
			self->tp_seq->tp_size_fast = base_seq->tp_size_fast;
			if (!self->tp_seq->tp_getitem_index_fast) {
				if (self->tp_seq->tp_getitem ||
				    self->tp_seq->tp_getitem_index ||
				    self->tp_seq->tp_getitem_string_hash ||
				    self->tp_seq->tp_getitem_string_len_hash ||
				    self->tp_seq->tp_trygetitem ||
				    self->tp_seq->tp_trygetitem_string_hash ||
				    self->tp_seq->tp_trygetitem_string_len_hash) {
					/* Can't inherit "tp_getitem_index_fast" */
				} else {
					self->tp_seq->tp_getitem_index_fast = base_seq->tp_getitem_index_fast;
				}
			} else if (self->tp_seq->tp_getitem_index_fast != base_seq->tp_getitem_index_fast) {
				self->tp_seq->tp_getitem_index_fast = NULL;
			}
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritContains(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_contains != NULL;
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!DeeType_InheritContains(base))
			continue;
		base_seq = base->tp_seq;
		LOG_INHERIT(base, self, "operator contains");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritContains(origin);
			self->tp_seq->tp_contains = base_seq->tp_contains;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritGetItem(DeeTypeObject *__restrict self) {
	struct type_seq *seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_getitem != NULL;
	}

	seq = self->tp_seq;
	if (seq && ((seq->tp_getitem_index_fast && seq->tp_size) ||
	            seq->tp_getitem ||
	            seq->tp_getitem_index ||
	            seq->tp_getitem_string_hash ||
	            seq->tp_getitem_string_len_hash ||
	            seq->tp_trygetitem ||
	            seq->tp_trygetitem_index ||
	            seq->tp_trygetitem_string_hash ||
	            seq->tp_trygetitem_string_len_hash)) {

		/* Substitute related core operators. */
		if (!seq->tp_getitem_string_hash) {
			if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash;
			}
		}
		if (!seq->tp_getitem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash;
			}
		}
		if (!seq->tp_trygetitem_string_hash) {
			if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash;
			}
		}
		if (!seq->tp_trygetitem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash;
			}
		}

		/* Figure out what's the "main" way that getitem should be implemented. */
		if (seq->tp_getitem_index_fast && seq->tp_size)  {
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithSizeAndGetItemIndexFast;
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithSizeAndGetItemIndexFast;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithSizeAndGetItemIndexFast;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithSizeAndGetItemIndexFast;
			if (!seq->tp_bounditem_index)
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithSizeAndGetItemIndexFast;
			if (!seq->tp_hasitem)
				seq->tp_hasitem = &DeeSeq_DefaultHasItemWithSize; /* Special case: seq operator here is correct! */
			if (!seq->tp_hasitem_index)
				seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSize; /* Special case: seq operator here is correct! */

			/* No string-based getitem allowed -> set getitem operators. */
set_string_operators_as_error:
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithErrorRequiresInt;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithErrorRequiresInt;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithErrorRequiresInt;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithErrorRequiresInt;

			/* No string-based getitem allowed -> set related operators. */
			if (!seq->tp_bounditem_string_hash)
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt;
			if (!seq->tp_bounditem_string_len_hash)
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt;
			if (!seq->tp_hasitem_string_hash)
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithErrorRequiresInt;
			if (!seq->tp_hasitem_string_len_hash)
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt;
		} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemIndex;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItemIndex;
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemIndex;
			goto set_string_operators_as_error;
		} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemDefault;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItemIndex;
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemIndex;
			goto set_string_operators_as_error;
		} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithGetItem;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItem;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItem;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItem;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithGetItem;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItem;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItem;
		} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItem;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithTryGetItem;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItem;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItem;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithTryGetItem;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItem;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItem;
		} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithGetItemStringHash;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringHash;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringHash;
			if (!seq->tp_trygetitem_string_len_hash) {
				if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
					seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
				} else {
					seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringHash;
				}
			}
		} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithGetItemStringLenHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithGetItemStringLenHash;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithGetItemStringLenHash;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithGetItemStringLenHash;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithGetItemStringLenHash;
		} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringHash;
			if (!seq->tp_getitem_string_len_hash) {
				if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
					seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
				} else {
					seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringHash;
				}
			}
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringHash;
			if (!seq->tp_trygetitem_string_len_hash)
				seq->tp_trygetitem_string_len_hash = &DeeObject_DefaultTryGetItemStringLenHashWithTryGetItemStringHash;
		} else {
			ASSERT(Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq));
			if (!seq->tp_getitem)
				seq->tp_getitem = &DeeObject_DefaultGetItemWithTryGetItemStringLenHash;
			if (!seq->tp_getitem_index)
				seq->tp_getitem_index = &DeeObject_DefaultGetItemIndexWithErrorRequiresString;
			if (!seq->tp_trygetitem_index)
				seq->tp_trygetitem_index = &DeeObject_DefaultTryGetItemIndexWithErrorRequiresString;
			if (!seq->tp_getitem_string_hash)
				seq->tp_getitem_string_hash = &DeeObject_DefaultGetItemStringHashWithTryGetItemStringLenHash;
			if (!seq->tp_getitem_string_len_hash)
				seq->tp_getitem_string_len_hash = &DeeObject_DefaultGetItemStringLenHashWithTryGetItemStringLenHash;
			if (!seq->tp_trygetitem)
				seq->tp_trygetitem = &DeeObject_DefaultTryGetItemWithTryGetItemStringLenHash;
			if (!seq->tp_trygetitem_string_hash)
				seq->tp_trygetitem_string_hash = &DeeObject_DefaultTryGetItemStringHashWithTryGetItemStringLenHash;
		}

		/* Substitute sub-operators via equivalents within their group. */
		if (!seq->tp_bounditem) {
			if (Dee_type_seq_has_custom_tp_bounditem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemIndex;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq) && Dee_type_seq_has_custom_tp_hasitem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndexAndHasItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHashAndHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHashAndHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithTryGetItemStringHash;
			} else {
				seq->tp_bounditem = &DeeObject_DefaultBoundItemWithGetItemDefault;
			}
		}
		if (!seq->tp_bounditem_index) {
			if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq) && Dee_type_seq_has_custom_tp_hasitem_index(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemIndexAndHasItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithErrorRequiresString;
			} else {
				seq->tp_bounditem_index = &DeeObject_DefaultBoundItemIndexWithGetItemIndexDefault;
			}
		}
		if (!seq->tp_bounditem_string_hash) {
			if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHashAndHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithErrorRequiresInt;
			} else {
				seq->tp_bounditem_string_hash = &DeeObject_DefaultBoundItemStringHashWithBoundItemDefault;
			}
		}
		if (!seq->tp_bounditem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHashAndHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq) && Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHashAndHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq) && Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemAndHasItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithErrorRequiresInt;
			} else {
				seq->tp_bounditem_string_len_hash = &DeeObject_DefaultBoundItemStringLenHashWithBoundItemDefault;
			}
		}

		if (!seq->tp_hasitem) {
			if (Dee_type_seq_has_custom_tp_hasitem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemIndex;
			} else if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemIndex;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemStringLenHash;
			} else {
				seq->tp_hasitem = &DeeObject_DefaultHasItemWithGetItemDefault;
			}
		}
		if (!seq->tp_hasitem_index) {
			if (Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem_index(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItemIndex;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem_index(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_string_hash(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem_index = &DeeObject_DefaultHasItemIndexWithErrorRequiresString;
			} else {
				seq->tp_hasitem_index = &DeeSeq_DefaultHasItemIndexWithSizeDefault; /* Shouldn't get here... */
			}
		}

		if (!seq->tp_hasitem_string_hash) {
			if (Dee_type_seq_has_custom_tp_hasitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_hasitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithErrorRequiresInt;
			} else {
				seq->tp_hasitem_string_hash = &DeeObject_DefaultHasItemStringHashWithHasItemDefault;
			}
		}

		if (!seq->tp_hasitem_string_len_hash) {
			if (Dee_type_seq_has_custom_tp_hasitem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_len_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_bounditem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_trygetitem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_len_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringLenHash;
			} else if (Dee_type_seq_has_custom_tp_getitem_string_hash(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItemStringHash;
			} else if (Dee_type_seq_has_custom_tp_hasitem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItem;
			} else if (Dee_type_seq_has_custom_tp_bounditem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithBoundItem;
			} else if (Dee_type_seq_has_custom_tp_trygetitem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithTryGetItem;
			} else if (Dee_type_seq_has_custom_tp_getitem(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithGetItem;
			} else if (Dee_type_seq_has_custom_tp_hasitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_bounditem_index(seq) ||
			           Dee_type_seq_has_custom_tp_getitem_index(seq) ||
			           Dee_type_seq_has_custom_tp_trygetitem_index(seq)) {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithErrorRequiresInt;
			} else {
				seq->tp_hasitem_string_len_hash = &DeeObject_DefaultHasItemStringLenHashWithHasItemDefault;
			}
		}

		return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!DeeType_InheritGetItem(base))
			continue;
		seq = base->tp_seq;
		LOG_INHERIT(base, self, "operator getitem");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritGetItem(origin);
			self->tp_seq->tp_getitem                    = seq->tp_getitem;
			self->tp_seq->tp_getitem_index              = seq->tp_getitem_index;
			self->tp_seq->tp_getitem_string_hash        = seq->tp_getitem_string_hash;
			self->tp_seq->tp_getitem_string_len_hash    = seq->tp_getitem_string_len_hash;
			self->tp_seq->tp_trygetitem                 = seq->tp_trygetitem;
			self->tp_seq->tp_trygetitem_string_hash     = seq->tp_trygetitem_string_hash;
			self->tp_seq->tp_trygetitem_string_len_hash = seq->tp_trygetitem_string_len_hash;
			self->tp_seq->tp_bounditem                  = seq->tp_bounditem;
			self->tp_seq->tp_bounditem_index            = seq->tp_bounditem_index;
			self->tp_seq->tp_bounditem_string_hash      = seq->tp_bounditem_string_hash;
			self->tp_seq->tp_bounditem_string_len_hash  = seq->tp_bounditem_string_len_hash;
			self->tp_seq->tp_hasitem                    = seq->tp_hasitem;
			self->tp_seq->tp_hasitem_index              = seq->tp_hasitem_index;
			self->tp_seq->tp_hasitem_string_hash        = seq->tp_hasitem_string_hash;
			self->tp_seq->tp_hasitem_string_len_hash    = seq->tp_hasitem_string_len_hash;

			/* Special handling for "tp_getitem_index_fast", so it's `index < size'
			 * invariant doesn't break when someone overwrites "operator size" */
			if ((seq->tp_getitem_index_fast != NULL) &&
			    (self->tp_seq->tp_size || DeeType_InheritSize(self)) &&
			    (self->tp_seq->tp_size == seq->tp_size)) {
				/* Can only inherit "tp_getitem_index_fast" if "tp_size" is also being inherited. */
				self->tp_seq->tp_getitem_index_fast = seq->tp_getitem_index_fast;
			} else {
				/* Else: use of "tp_getitem_index_fast" would be illegal. */
				self->tp_seq->tp_getitem_index_fast = NULL;
			}
		} else {
			self->tp_seq = seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritDelItem(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_delitem != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_delitem_string_hash) {
			if (!base_seq->tp_delitem_string_len_hash)
				base_seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItemStringHash;
		} else {
			if (base_seq->tp_delitem_string_len_hash)
				base_seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItemStringLenHash;
		}
		if (base_seq->tp_delitem_index) {
			if (base_seq->tp_delitem == NULL)
				base_seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemIndex;
			if (base_seq->tp_delitem_string_hash == NULL)
				base_seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithErrorRequiresInt;
			if (base_seq->tp_delitem_string_len_hash == NULL)
				base_seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithErrorRequiresInt;
			return true;
		} else if (base_seq->tp_delitem) {
			if (base_seq->tp_delitem_index == NULL)
				base_seq->tp_delitem_index = &DeeObject_DefaultDelItemIndexWithDelItem;
			if (base_seq->tp_delitem_string_hash == NULL)
				base_seq->tp_delitem_string_hash = &DeeObject_DefaultDelItemStringHashWithDelItem;
			if (base_seq->tp_delitem_string_len_hash == NULL)
				base_seq->tp_delitem_string_len_hash = &DeeObject_DefaultDelItemStringLenHashWithDelItem;
			return true;
		} else if (base_seq->tp_delitem_string_hash) {
			ASSERT(base_seq->tp_delitem_string_len_hash);
			if (base_seq->tp_delitem == NULL) {
				if (Dee_type_seq_has_custom_tp_delitem_string_len_hash(base_seq)) {
					base_seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringLenHash;
				} else {
					base_seq->tp_delitem = &DeeObject_DefaultDelItemWithDelItemStringHash;
				}
			}
			if (base_seq->tp_delitem_index == NULL)
				base_seq->tp_delitem_index = &DeeObject_DefaultDelItemIndexWithErrorRequiresString;
			return true;
		} else {
			ASSERT(!base_seq->tp_delitem_string_len_hash);
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_delitem ||
		                         !base_seq->tp_delitem_index ||
		                         !base_seq->tp_delitem_string_hash ||
		                         !base_seq->tp_delitem_string_len_hash)) {
			if (!DeeType_InheritDelItem(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator delitem");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritDelItem(origin);
			self->tp_seq->tp_delitem                 = base_seq->tp_delitem;
			self->tp_seq->tp_delitem_index           = base_seq->tp_delitem_index;
			self->tp_seq->tp_delitem_string_hash     = base_seq->tp_delitem_string_hash;
			self->tp_seq->tp_delitem_string_len_hash = base_seq->tp_delitem_string_len_hash;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritSetItem(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_setitem != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_setitem_string_hash) {
			if (!base_seq->tp_setitem_string_len_hash)
				base_seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItemStringHash;
		} else {
			if (base_seq->tp_setitem_string_len_hash)
				base_seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItemStringLenHash;
		}
		if (base_seq->tp_setitem_index) {
			if (base_seq->tp_setitem == NULL)
				base_seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemIndex;
			if (base_seq->tp_setitem_string_hash == NULL)
				base_seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithErrorRequiresInt;
			if (base_seq->tp_setitem_string_len_hash == NULL)
				base_seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithErrorRequiresInt;
			return true;
		} else if (base_seq->tp_setitem) {
			if (base_seq->tp_setitem_index == NULL)
				base_seq->tp_setitem_index = &DeeObject_DefaultSetItemIndexWithSetItem;
			if (base_seq->tp_setitem_string_hash == NULL)
				base_seq->tp_setitem_string_hash = &DeeObject_DefaultSetItemStringHashWithSetItem;
			if (base_seq->tp_setitem_string_len_hash == NULL)
				base_seq->tp_setitem_string_len_hash = &DeeObject_DefaultSetItemStringLenHashWithSetItem;
			return true;
		} else if (base_seq->tp_setitem_string_hash) {
			ASSERT(base_seq->tp_setitem_string_len_hash);
			if (base_seq->tp_setitem == NULL) {
				if (Dee_type_seq_has_custom_tp_setitem_string_len_hash(base_seq)) {
					base_seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringLenHash;
				} else {
					base_seq->tp_setitem = &DeeObject_DefaultSetItemWithSetItemStringHash;
				}
			}
			if (base_seq->tp_setitem_index == NULL)
				base_seq->tp_setitem_index = &DeeObject_DefaultSetItemIndexWithErrorRequiresString;
			return true;
		} else {
			ASSERT(!base_seq->tp_setitem_string_len_hash);
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_setitem ||
		                         !base_seq->tp_setitem_index ||
		                         !base_seq->tp_setitem_string_hash ||
		                         !base_seq->tp_setitem_string_len_hash)) {
			if (!DeeType_InheritSetItem(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator setitem");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSetItem(origin);
			self->tp_seq->tp_setitem                 = base_seq->tp_setitem;
			self->tp_seq->tp_setitem_index           = base_seq->tp_setitem_index;
			self->tp_seq->tp_setitem_string_hash     = base_seq->tp_setitem_string_hash;
			self->tp_seq->tp_setitem_string_len_hash = base_seq->tp_setitem_string_len_hash;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritGetRange(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_getrange != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_getrange_index && base_seq->tp_getrange_index_n) {
			if (base_seq->tp_getrange == NULL)
				base_seq->tp_getrange = &DeeObject_DefaultGetRangeWithGetRangeIndexAndGetRangeIndexN;
			return true;
		} else if (base_seq->tp_getrange) {
			if (base_seq->tp_getrange_index == NULL)
				base_seq->tp_getrange_index = &DeeObject_DefaultGetRangeIndexWithGetRange;
			if (base_seq->tp_getrange_index_n == NULL)
				base_seq->tp_getrange_index_n = &DeeObject_DefaultGetRangeIndexNWithGetRange;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_getrange ||
		                         !base_seq->tp_getrange_index ||
		                         !base_seq->tp_getrange_index_n)) {
			if (!DeeType_InheritGetRange(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator getrange");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritGetRange(origin);
			self->tp_seq->tp_getrange         = base_seq->tp_getrange;
			self->tp_seq->tp_getrange_index   = base_seq->tp_getrange_index;
			self->tp_seq->tp_getrange_index_n = base_seq->tp_getrange_index_n;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritDelRange(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_delrange != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_delrange_index && base_seq->tp_delrange_index_n) {
			if (base_seq->tp_delrange == NULL)
				base_seq->tp_delrange = &DeeObject_DefaultDelRangeWithDelRangeIndexAndDelRangeIndexN;
			return true;
		} else if (base_seq->tp_delrange) {
			if (base_seq->tp_delrange_index == NULL)
				base_seq->tp_delrange_index = &DeeObject_DefaultDelRangeIndexWithDelRange;
			if (base_seq->tp_delrange_index_n == NULL)
				base_seq->tp_delrange_index_n = &DeeObject_DefaultDelRangeIndexNWithDelRange;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_delrange ||
		                         !base_seq->tp_delrange_index ||
		                         !base_seq->tp_delrange_index_n)) {
			if (!DeeType_InheritDelRange(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator delrange");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritDelRange(origin);
			self->tp_seq->tp_delrange         = base_seq->tp_delrange;
			self->tp_seq->tp_delrange_index   = base_seq->tp_delrange_index;
			self->tp_seq->tp_delrange_index_n = base_seq->tp_delrange_index_n;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritSetRange(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;

	/* Special case when it's a sequence type. */
	{
		unsigned int seqclass = DeeType_GetSeqClass(self);
		if (seqclass != Dee_SEQCLASS_NONE)
			return DeeType_InheritSeqOperators(self, seqclass) && self->tp_seq->tp_setrange != NULL;
	}

	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_setrange_index && base_seq->tp_setrange_index_n) {
			if (base_seq->tp_setrange == NULL)
				base_seq->tp_setrange = &DeeObject_DefaultSetRangeWithSetRangeIndexAndSetRangeIndexN;
			return true;
		} else if (base_seq->tp_setrange) {
			if (base_seq->tp_setrange_index == NULL)
				base_seq->tp_setrange_index = &DeeObject_DefaultSetRangeIndexWithSetRange;
			if (base_seq->tp_setrange_index_n == NULL)
				base_seq->tp_setrange_index_n = &DeeObject_DefaultSetRangeIndexNWithSetRange;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_setrange ||
		                         !base_seq->tp_setrange_index ||
		                         !base_seq->tp_setrange_index_n)) {
			if (!DeeType_InheritSetRange(base))
				continue;
			base_seq = base->tp_seq;
		}
		LOG_INHERIT(base, self, "operator setrange");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritSetRange(origin);
			self->tp_seq->tp_setrange         = base_seq->tp_setrange;
			self->tp_seq->tp_setrange_index   = base_seq->tp_setrange_index;
			self->tp_seq->tp_setrange_index_n = base_seq->tp_setrange_index_n;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

/* Optimizations when inheriting certain operators. */
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS

typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeType_tp_bool_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) Dee_hash_t (DCALL *DeeType_tp_hash_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_ne_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_lo_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_le_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_gr_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeType_tp_ge_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_compare_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_compare_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeType_tp_trycompare_eq_t)(DeeObject *self, DeeObject *some_object);

/* Special sequence operators which (when inherited) get replaced with
 * optimized `Dee(Seq|Set|Map)_Default*' functions based on sequence
 * features supported by the target type. */
INTDEF struct Dee_type_cmp generic_seq_cmp;
INTDEF WUNUSED NONNULL((1)) int DCALL generic_seq_bool(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_seq_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_seq_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_trycompare_eq(DeeObject *lhs, DeeObject *rhs);

INTDEF struct Dee_type_cmp generic_set_cmp;
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_set_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_set_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_set_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_set_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_set_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_set_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_set_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_set_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_set_trycompare_eq(DeeObject *lhs, DeeObject *rhs);

INTDEF struct Dee_type_cmp generic_map_cmp;
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_map_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_map_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_map_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_map_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_map_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_map_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_map_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_map_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_map_trycompare_eq(DeeObject *lhs, DeeObject *rhs);


PRIVATE NONNULL((1)) DeeType_tp_bool_t DCALL
DeeType_Optimize_tp_bool(DeeTypeObject *__restrict dst,
                         DeeType_tp_bool_t tp_bool) {
	if (tp_bool == &generic_seq_bool ||
	    tp_bool == &DeeSeq_DefaultBoolWithSize ||
	    tp_bool == &DeeSeq_DefaultBoolWithSizeOb ||
	    tp_bool == &DeeSeq_DefaultBoolWithForeach ||
	    tp_bool == &DeeSeq_DefaultBoolWithCompareEq ||
	    tp_bool == &DeeSeq_DefaultBoolWithEq ||
	    tp_bool == &DeeSeq_DefaultBoolWithNe ||
	    tp_bool == &DeeSeq_DefaultBoolWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (Dee_type_seq_has_custom_tp_size(dst_seq)) {
				return &DeeSeq_DefaultBoolWithSize;
			} else if (Dee_type_seq_has_custom_tp_sizeob(dst_seq)) {
				return &DeeSeq_DefaultBoolWithSizeOb;
			} else if (Dee_type_seq_has_custom_tp_foreach(dst_seq)) {
				return &DeeSeq_DefaultBoolWithForeach;
			} else if (dst->tp_cmp && dst->tp_cmp->tp_compare_eq &&
			           !DeeType_IsDefaultCompareEq(dst->tp_cmp->tp_compare_eq) &&
			           !DeeType_IsDefaultCompare(dst->tp_cmp->tp_compare_eq)) {
				return &DeeSeq_DefaultBoolWithCompareEq;
			} else if (dst->tp_cmp && dst->tp_cmp->tp_eq && !DeeType_IsDefaultEq(dst->tp_cmp->tp_eq)) {
				return &DeeSeq_DefaultBoolWithEq;
			} else if (dst->tp_cmp && dst->tp_cmp->tp_ne && !DeeType_IsDefaultNe(dst->tp_cmp->tp_ne)) {
				return &DeeSeq_DefaultBoolWithNe;
			} else if (dst_seq->tp_foreach || (DeeType_InheritIter(dst) && dst_seq->tp_foreach)) {
				return &DeeSeq_DefaultBoolWithForeachDefault;
			}
		}
		return &generic_seq_bool;
	}
	return tp_bool;
}

PRIVATE NONNULL((1)) struct type_cmp *DCALL
DeeType_Optimize_tp_cmp(DeeTypeObject *__restrict dst,
                        struct type_cmp *tp_cmp) {
	if (tp_cmp == &generic_seq_cmp ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeAndGetItemIndexFast ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeAndTryGetItemIndex ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeAndGetItemIndex ||
	    tp_cmp == &DeeSeq_DefaultCmpWithSizeObAndGetItem ||
	    tp_cmp == &DeeSeq_DefaultCmpWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			bool has_tp_size = Dee_type_seq_has_custom_tp_size(dst_seq);
			if (has_tp_size && dst_seq->tp_getitem_index_fast) {
				return &DeeSeq_DefaultCmpWithSizeAndGetItemIndexFast;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_trygetitem_index(dst_seq)) {
				return &DeeSeq_DefaultCmpWithSizeAndTryGetItemIndex;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_getitem_index(dst_seq)) {
				return &DeeSeq_DefaultCmpWithSizeAndGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_sizeob(dst_seq) &&
			           Dee_type_seq_has_custom_tp_getitem(dst_seq)) {
				return &DeeSeq_DefaultCmpWithSizeObAndGetItem;
			} else if (dst_seq->tp_foreach || (DeeType_InheritIter(dst) && dst_seq->tp_foreach)) {
				return &DeeSeq_DefaultCmpWithForeachDefault;
			}
		}
		return &generic_seq_cmp;
	} else if (tp_cmp == &generic_set_cmp ||
	           tp_cmp == &DeeSet_DefaultCmpWithForeachDefault) {
		if (DeeType_InheritIter(dst)) {
			struct type_seq *dst_seq = dst->tp_seq;
			if (dst_seq && dst_seq->tp_foreach)
				return &DeeSet_DefaultCmpWithForeachDefault;
		}
		return &generic_set_cmp;
	} else if (tp_cmp == &generic_map_cmp ||
	           tp_cmp == &DeeMap_DefaultCmpWithForeachPairDefault) {
		if (DeeType_InheritIter(dst)) {
			struct type_seq *dst_seq = dst->tp_seq;
			if (dst_seq && dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultCmpWithForeachPairDefault;
		}
		return &generic_map_cmp;
	}
	return tp_cmp;
}

PRIVATE NONNULL((1)) DeeType_tp_hash_t DCALL
DeeType_Optimize_tp_hash(DeeTypeObject *__restrict dst,
                         DeeType_tp_hash_t tp_hash) {
	if (tp_hash == &generic_seq_hash ||
	    tp_hash == &DeeSeq_DefaultHashWithSizeAndGetItemIndexFast ||
	    tp_hash == &DeeSeq_DefaultHashWithForeach ||
	    tp_hash == &DeeSeq_DefaultHashWithSizeAndTryGetItemIndex ||
	    tp_hash == &DeeSeq_DefaultHashWithSizeAndGetItemIndex ||
	    tp_hash == &DeeSeq_DefaultHashWithSizeObAndGetItem ||
	    tp_hash == &DeeSeq_DefaultHashWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			bool has_tp_size = Dee_type_seq_has_custom_tp_size(dst_seq);
			if (has_tp_size && dst_seq->tp_getitem_index_fast) {
				return &DeeSeq_DefaultHashWithSizeAndGetItemIndexFast;
			} else if (Dee_type_seq_has_custom_tp_foreach(dst_seq)) {
				return &DeeSeq_DefaultHashWithForeach;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_trygetitem_index(dst_seq)) {
				return &DeeSeq_DefaultHashWithSizeAndTryGetItemIndex;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_getitem_index(dst_seq)) {
				return &DeeSeq_DefaultHashWithSizeAndGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_sizeob(dst_seq) &&
			           Dee_type_seq_has_custom_tp_getitem(dst_seq)) {
				return &DeeSeq_DefaultHashWithSizeObAndGetItem;
			} else if (dst_seq->tp_foreach) {
				return &DeeSeq_DefaultHashWithForeachDefault;
			}
		}
		return &generic_seq_hash;
	} else if (tp_hash == &generic_set_hash ||
	           tp_hash == &DeeSet_DefaultHashWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach)
				return &DeeSet_DefaultHashWithForeachDefault;
		}
		return &generic_set_hash;
	} else if (tp_hash == &generic_map_hash ||
	           tp_hash == &DeeMap_DefaultHashWithForeachPairDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultHashWithForeachPairDefault;
		}
		return &generic_map_hash;
	}
	return tp_hash;
}

PRIVATE NONNULL((1)) DeeType_tp_eq_t DCALL
DeeType_Optimize_tp_eq(DeeTypeObject *__restrict dst,
                       DeeType_tp_eq_t tp_eq) {
	if (tp_eq == &generic_seq_eq || tp_eq == &generic_set_eq || tp_eq == &generic_map_eq) {
		if (dst->tp_cmp->tp_compare_eq)
			return &DeeObject_DefaultEqWithCompareEqDefault;
	}
	return tp_eq;
}

PRIVATE NONNULL((1)) DeeType_tp_ne_t DCALL
DeeType_Optimize_tp_ne(DeeTypeObject *__restrict dst,
                       DeeType_tp_ne_t tp_ne) {
	if (tp_ne == &generic_seq_ne || tp_ne == &generic_set_ne || tp_ne == &generic_map_ne) {
		if (dst->tp_cmp->tp_compare_eq)
			return &DeeObject_DefaultNeWithCompareEqDefault;
	}
	return tp_ne;
}

PRIVATE NONNULL((1)) DeeType_tp_lo_t DCALL
DeeType_Optimize_tp_lo(DeeTypeObject *__restrict dst,
                       DeeType_tp_lo_t tp_lo) {
	if (tp_lo == &generic_seq_lo) {
		if (dst->tp_cmp->tp_compare)
			return &DeeObject_DefaultLoWithCompareDefault;
	} else if (tp_lo == &generic_set_lo ||
	           tp_lo == &DeeSet_DefaultLoWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach)
				return &DeeSet_DefaultLoWithForeachDefault;
		}
		return &generic_set_lo;
	} else if (tp_lo == &generic_map_lo ||
	           tp_lo == &DeeMap_DefaultLoWithForeachPairDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultLoWithForeachPairDefault;
		}
		return &generic_map_lo;
	}
	return tp_lo;
}

PRIVATE NONNULL((1)) DeeType_tp_le_t DCALL
DeeType_Optimize_tp_le(DeeTypeObject *__restrict dst,
                       DeeType_tp_le_t tp_le) {
	if (tp_le == &generic_seq_le) {
		if (dst->tp_cmp->tp_compare)
			return &DeeObject_DefaultLeWithCompareDefault;
	} else if (tp_le == &generic_set_le ||
	           tp_le == &DeeSet_DefaultLeWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach)
				return &DeeSet_DefaultLeWithForeachDefault;
		}
		return &generic_set_le;
	} else if (tp_le == &generic_map_le ||
	           tp_le == &DeeMap_DefaultLeWithForeachPairDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultLeWithForeachPairDefault;
		}
		return &generic_map_le;
	}
	return tp_le;
}

PRIVATE NONNULL((1)) DeeType_tp_gr_t DCALL
DeeType_Optimize_tp_gr(DeeTypeObject *__restrict dst,
                       DeeType_tp_gr_t tp_gr) {
	if (tp_gr == &generic_seq_gr) {
		if (dst->tp_cmp->tp_compare)
			return &DeeObject_DefaultGrWithCompareDefault;
	} else if (tp_gr == &generic_set_gr ||
	           tp_gr == &DeeSet_DefaultGrWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach)
				return &DeeSet_DefaultGrWithForeachDefault;
		}
		return &generic_set_gr;
	} else if (tp_gr == &generic_map_gr ||
	           tp_gr == &DeeMap_DefaultGrWithForeachPairDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultGrWithForeachPairDefault;
		}
		return &generic_map_gr;
	}
	return tp_gr;
}

PRIVATE NONNULL((1)) DeeType_tp_ge_t DCALL
DeeType_Optimize_tp_ge(DeeTypeObject *__restrict dst,
                       DeeType_tp_ge_t tp_ge) {
	if (tp_ge == &generic_seq_ge) {
		if (dst->tp_cmp->tp_compare)
			return &DeeObject_DefaultGeWithCompareDefault;
	} else if (tp_ge == &generic_set_ge ||
	           tp_ge == &DeeSet_DefaultGeWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach)
				return &DeeSet_DefaultGeWithForeachDefault;
		}
		return &generic_set_ge;
	} else if (tp_ge == &generic_map_ge ||
	           tp_ge == &DeeMap_DefaultGeWithForeachPairDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultGeWithForeachPairDefault;
		}
		return &generic_map_ge;
	}
	return tp_ge;
}

PRIVATE NONNULL((1)) DeeType_tp_compare_t DCALL
DeeType_Optimize_tp_compare(DeeTypeObject *__restrict dst,
                            DeeType_tp_compare_t tp_compare) {
	if (tp_compare == &generic_seq_compare ||
	    tp_compare == &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast ||
	    tp_compare == &DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex ||
	    tp_compare == &DeeSeq_DefaultCompareWithSizeAndGetItemIndex ||
	    tp_compare == &DeeSeq_DefaultCompareWithSizeObAndGetItem ||
	    tp_compare == &DeeSeq_DefaultCompareWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			bool has_tp_size = Dee_type_seq_has_custom_tp_size(dst_seq);
			if (has_tp_size && dst_seq->tp_getitem_index_fast) {
				return &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_trygetitem_index(dst_seq)) {
				return &DeeSeq_DefaultCompareWithSizeAndTryGetItemIndex;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_getitem_index(dst_seq)) {
				return &DeeSeq_DefaultCompareWithSizeAndGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_sizeob(dst_seq) &&
			           Dee_type_seq_has_custom_tp_getitem(dst_seq)) {
				return &DeeSeq_DefaultCompareWithSizeObAndGetItem;
			} else if (dst_seq->tp_foreach) {
				return &DeeSeq_DefaultCompareWithForeachDefault;
			}
		}
		return &generic_seq_compare;
	}
	return tp_compare;
}

PRIVATE NONNULL((1)) DeeType_tp_compare_eq_t DCALL
DeeType_Optimize_tp_compare_eq(DeeTypeObject *__restrict dst,
                               DeeType_tp_compare_eq_t tp_compare_eq) {
	if (tp_compare_eq == &generic_seq_compare_eq ||
	    tp_compare_eq == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast ||
	    tp_compare_eq == &DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex ||
	    tp_compare_eq == &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex ||
	    tp_compare_eq == &DeeSeq_DefaultCompareEqWithSizeObAndGetItem ||
	    tp_compare_eq == &DeeSeq_DefaultCompareEqWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			bool has_tp_size = Dee_type_seq_has_custom_tp_size(dst_seq);
			if (has_tp_size && dst_seq->tp_getitem_index_fast) {
				return &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndexFast;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_trygetitem_index(dst_seq)) {
				return &DeeSeq_DefaultCompareEqWithSizeAndTryGetItemIndex;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_getitem_index(dst_seq)) {
				return &DeeSeq_DefaultCompareEqWithSizeAndGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_sizeob(dst_seq) &&
			           Dee_type_seq_has_custom_tp_getitem(dst_seq)) {
				return &DeeSeq_DefaultCompareEqWithSizeObAndGetItem;
			} else if (dst_seq->tp_foreach) {
				return &DeeSeq_DefaultCompareEqWithForeachDefault;
			}
		}
		return &generic_seq_compare_eq;
	} else if (tp_compare_eq == &generic_set_compare_eq ||
	           tp_compare_eq == &DeeSet_DefaultCompareEqWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach)
				return &DeeSet_DefaultCompareEqWithForeachDefault;
		}
		return &generic_set_compare_eq;
	} else if (tp_compare_eq == &generic_map_compare_eq ||
	           tp_compare_eq == &DeeMap_DefaultCompareEqWithForeachPairDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultCompareEqWithForeachPairDefault;
		}
		return &generic_map_compare_eq;
	}
	return DeeType_Optimize_tp_compare(dst, tp_compare_eq);
}

PRIVATE NONNULL((1)) DeeType_tp_trycompare_eq_t DCALL
DeeType_Optimize_tp_trycompare_eq(DeeTypeObject *__restrict dst,
                                  DeeType_tp_trycompare_eq_t tp_trycompare_eq) {
	if (tp_trycompare_eq == &generic_seq_trycompare_eq ||
	    tp_trycompare_eq == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast ||
	    tp_trycompare_eq == &DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex ||
	    tp_trycompare_eq == &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex ||
	    tp_trycompare_eq == &DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem ||
	    tp_trycompare_eq == &DeeSeq_DefaultTryCompareEqWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			bool has_tp_size = Dee_type_seq_has_custom_tp_size(dst_seq);
			if (has_tp_size && dst_seq->tp_getitem_index_fast) {
				return &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_trygetitem_index(dst_seq)) {
				return &DeeSeq_DefaultTryCompareEqWithSizeAndTryGetItemIndex;
			} else if (has_tp_size && Dee_type_seq_has_custom_tp_getitem_index(dst_seq)) {
				return &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndex;
			} else if (Dee_type_seq_has_custom_tp_sizeob(dst_seq) &&
			           Dee_type_seq_has_custom_tp_getitem(dst_seq)) {
				return &DeeSeq_DefaultTryCompareEqWithSizeObAndGetItem;
			} else if (dst_seq->tp_foreach) {
				return &DeeSeq_DefaultTryCompareEqWithForeachDefault;
			}
		}
		return &generic_seq_trycompare_eq;
	} else if (tp_trycompare_eq == &generic_set_trycompare_eq ||
	           tp_trycompare_eq == &DeeSet_DefaultTryCompareEqWithForeachDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach)
				return &DeeSet_DefaultTryCompareEqWithForeachDefault;
		}
		return &generic_set_trycompare_eq;
	} else if (tp_trycompare_eq == &generic_map_trycompare_eq ||
	           tp_trycompare_eq == &DeeMap_DefaultTryCompareEqWithForeachPairDefault) {
		struct type_seq *dst_seq = dst->tp_seq;
		if (dst_seq || (DeeType_InheritIter(dst) && (dst_seq = dst->tp_seq) != NULL)) {
			if (dst_seq->tp_foreach_pair)
				return &DeeMap_DefaultTryCompareEqWithForeachPairDefault;
		}
		return &generic_map_trycompare_eq;
	}
	return tp_trycompare_eq;
}
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
#define DeeType_Optimize_tp_bool(dst, tp_bool)                   tp_bool
#define DeeType_Optimize_tp_hash(dst, tp_hash)                   tp_hash
#define DeeType_Optimize_tp_cmp(dst, tp_cmp)                     tp_cmp
#define DeeType_Optimize_tp_eq(dst, tp_eq)                       tp_eq
#define DeeType_Optimize_tp_ne(dst, tp_ne)                       tp_ne
#define DeeType_Optimize_tp_lo(dst, tp_lo)                       tp_lo
#define DeeType_Optimize_tp_le(dst, tp_le)                       tp_le
#define DeeType_Optimize_tp_gr(dst, tp_gr)                       tp_gr
#define DeeType_Optimize_tp_ge(dst, tp_ge)                       tp_ge
#define DeeType_Optimize_tp_compare_eq(dst, tp_compare_eq)       tp_compare_eq
#define DeeType_Optimize_tp_compare(dst, tp_compare)             tp_compare
#define DeeType_Optimize_tp_trycompare_eq(dst, tp_trycompare_eq) tp_trycompare_eq
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

INTERN NONNULL((1)) bool DCALL
DeeType_InheritCompare(DeeTypeObject *__restrict self) {
	struct type_cmp *base_cmp;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_cmp = self->tp_cmp;
	if (base_cmp) {
		DeeType_SubstituteDefaultCompareOperators(self);
		if (base_cmp->tp_hash || base_cmp->tp_eq || base_cmp->tp_lo || base_cmp->tp_le)
			return true;
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_cmp = base->tp_cmp;
		if (base_cmp == NULL ||
		    (!base_cmp->tp_hash ||
		     !base_cmp->tp_compare_eq || !base_cmp->tp_compare ||
		     !base_cmp->tp_trycompare_eq ||
		     !base_cmp->tp_eq || !base_cmp->tp_ne ||
		     !base_cmp->tp_lo || !base_cmp->tp_le ||
		     !base_cmp->tp_gr || !base_cmp->tp_ge)) {
			if (!DeeType_InheritCompare(base))
				continue;
			base_cmp = base->tp_cmp;
		}
		LOG_INHERIT(base, self, "operator <compare>");
		if (self->tp_cmp) {
			DeeTypeObject *origin = DeeType_GetCmpOrigin(self);
			if unlikely(origin)
				return DeeType_InheritCompare(origin);
			self->tp_cmp->tp_hash          = DeeType_Optimize_tp_hash(self, base_cmp->tp_hash);
			self->tp_cmp->tp_compare       = DeeType_Optimize_tp_compare(self, base_cmp->tp_compare);
			self->tp_cmp->tp_compare_eq    = DeeType_Optimize_tp_compare_eq(self, base_cmp->tp_compare_eq);
			self->tp_cmp->tp_trycompare_eq = DeeType_Optimize_tp_trycompare_eq(self, base_cmp->tp_trycompare_eq);
			/* Important: must optimize these *after* the stuff above! */
			self->tp_cmp->tp_eq = DeeType_Optimize_tp_eq(self, base_cmp->tp_eq);
			self->tp_cmp->tp_ne = DeeType_Optimize_tp_ne(self, base_cmp->tp_ne);
			self->tp_cmp->tp_lo = DeeType_Optimize_tp_lo(self, base_cmp->tp_lo);
			self->tp_cmp->tp_le = DeeType_Optimize_tp_le(self, base_cmp->tp_le);
			self->tp_cmp->tp_gr = DeeType_Optimize_tp_gr(self, base_cmp->tp_gr);
			self->tp_cmp->tp_ge = DeeType_Optimize_tp_ge(self, base_cmp->tp_ge);
		} else {
			self->tp_cmp = DeeType_Optimize_tp_cmp(self, base_cmp);
		}
		return true;
	}
	return false;
}


INTERN NONNULL((1)) bool DCALL
DeeType_InheritBool(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_cast.tp_bool) {
			if (!DeeType_InheritBool(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator bool");
		self->tp_cast.tp_bool = DeeType_Optimize_tp_bool(self, base->tp_cast.tp_bool);
		return true;
	}
	return false;
}
#endif /* !DEFINE_TYPED_OPERATORS */




DEFINE_OPERATOR(DREF DeeObject *, Iter, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_iter) ||
	          unlikely(DeeType_InheritIter(tp_self)))
		return DeeType_INVOKE_ITER(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_ITER);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, IterNext, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_iter_next) ||
	          unlikely(DeeType_InheritIterNext(tp_self)))
		return DeeType_INVOKE_NEXT(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_ITERNEXT);
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
/* Advance an iterator by "step" items.
 * @return: step:       Success.
 * @return: < step:     Success, but advancing was stopped prematurely because ITER_DONE
 *                      was encountered. Return value is the # of successfully skipped
 *                      entries before "ITER_DONE" was encountered.
 * @return: (size_t)-1: Error. */
DEFINE_OPERATOR(size_t, IterAdvance, (DeeObject *__restrict self, size_t step)) {
	size_t result = 0;
	DREF DeeObject *(DCALL *tp_iter_next)(DeeObject *__restrict self);
	LOAD_TP_SELF;
	if unlikely(!step)
		goto done;
	if unlikely(!tp_self->tp_iter_next &&
	            !DeeType_InheritIterNext(tp_self))
		goto err_no_iter_next;
	/* TODO: Type-specific optimizations */
	tp_iter_next = tp_self->tp_iter_next;
	do {
		DREF DeeObject *elem = (*tp_iter_next)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		++result;
	} while (result < step);
done:
	return result;
err_no_iter_next:
	err_unimplemented_operator(tp_self, OPERATOR_ITERNEXT);
err:
	return (size_t)-1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

/* @return: (size_t)-1: Fast size cannot be determined */
DEFINE_OPERATOR(size_t, SizeFast, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_size_fast) ||
	          unlikely(DeeType_InheritSize(tp_self)))
		return DeeType_INVOKE_SIZEFAST(tp_self, self);
	return (size_t)-1;
}

DEFINE_OPERATOR(size_t, Size, (DeeObject *RESTRICT_IF_NOTYPE self)) {
#ifdef CONFIG_NO_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_size) ||
	          unlikely(DeeType_InheritSize(tp_self) && tp_self->tp_seq->tp_size))
		return DeeType_INVOKE_SIZE(tp_self, self);
	return (size_t)err_unimplemented_operator(tp_self, OPERATOR_SIZE);
#else /* CONFIG_NO_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	DREF DeeObject *sizeob;
	size_t result;
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_sizeob) ||
	          unlikely(DeeType_InheritSize(tp_self))) {
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi) {
			ASSERT(nsi->nsi_common.nsi_getsize);
			return (*nsi->nsi_common.nsi_getsize)(self);
		}
		sizeob = DeeType_INVOKE_SIZEOB(tp_self, self);
		if unlikely(!sizeob)
			goto err;
		if (DeeObject_AsSize(sizeob, &result))
			goto err_ob;
		Dee_Decref(sizeob);
		/* Deal with negative-one */
		if unlikely(result == (size_t)-1)
			return err_integer_overflow_i(sizeof(size_t) * 8, true);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_SIZE);
	return (size_t)-1;
err_ob:
	Dee_Decref(sizeob);
err:
	return (size_t)-1;
#endif /* !CONFIG_NO_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
}


#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(int, ContainsAsBool,
                (DeeObject *self, DeeObject *some_object)) {
	DREF DeeObject *resultob;
	resultob = DeeObject_Contains(self, some_object);
	if unlikely(!resultob)
		goto err;
	return DeeObject_BoolInherited(resultob);
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, SizeOb,
                (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_sizeob) ||
	          unlikely(DeeType_InheritSize(tp_self)))
		return DeeType_INVOKE_SIZEOB(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_SIZE);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Contains,
                (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_contains) ||
	          unlikely(DeeType_InheritContains(tp_self)))
		return DeeType_INVOKE_CONTAINS(tp_self, self, some_object);
	err_unimplemented_operator(tp_self, OPERATOR_CONTAINS);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItem,
                (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_GETITEM(tp_self, self, index);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(int, DelItem,
                (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delitem) ||
	          unlikely(DeeType_InheritDelItem(tp_self)))
		return DeeType_INVOKE_DELITEM(tp_self, self, index);
	return err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
}

DEFINE_OPERATOR(int, SetItem,
                (DeeObject *self, DeeObject *index, DeeObject *value)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setitem) ||
	          unlikely(DeeType_InheritSetItem(tp_self)))
		return DeeType_INVOKE_SETITEM(tp_self, self, index, value);
	return err_unimplemented_operator(tp_self, OPERATOR_SETITEM);
}

DEFINE_OPERATOR(DREF DeeObject *, GetRange,
                (DeeObject *self, DeeObject *start, DeeObject *end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange) ||
	          unlikely(DeeType_InheritGetRange(tp_self)))
		return DeeType_INVOKE_GETRANGE(tp_self, self, start, end);
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_ConcatInherited(DREF DeeObject *self, DeeObject *other) {
	DREF DeeObject *result;
	if (DeeTuple_CheckExact(other)) {
		size_t count = DeeTuple_SIZE(other);
		Dee_Increfv(DeeTuple_ELEM(other), count);
		result = DeeObject_ExtendInherited(self, count, DeeTuple_ELEM(other));
		if unlikely(!result)
			Dee_Decrefv(DeeTuple_ELEM(other), count);
		return result;
	}
	if (DeeTuple_CheckExact(self))
		return DeeTuple_ConcatInherited(self, other);
	if (DeeList_CheckExact(self))
		return DeeList_ConcatInherited(self, other);
	/* Fallback: perform an arithmetic add operation. */
	result = DeeObject_Add(self, other);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeObject_ExtendInherited(/*inherit(on_success)*/ DREF DeeObject *self, size_t argc,
                          /*inherit(on_success)*/ DREF DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeObject *other;
	if (DeeTuple_CheckExact(self))
		return DeeTuple_ExtendInherited(self, argc, argv);
	if (DeeList_CheckExact(self))
		return DeeList_ExtendInherited(self, argc, argv);
	/* Fallback: perform an arithmetic add operation. */
	other = DeeSharedVector_NewShared(argc, argv);
	if unlikely(!other)
		goto err;
	result = DeeObject_Add(self, (DeeObject *)other);
	DeeSharedVector_Decref(other);
	Dee_Decref(self);
	return result;
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */


#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
DEFINE_OPERATOR(DREF DeeObject *, GetRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t start, DeeObject *end)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	if (DeeNone_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index_n) ||
		          unlikely(DeeType_InheritGetRange(tp_self)))
			return DeeType_INVOKE_GETRANGEINDEXN(tp_self, self, start);
	} else if (DeeInt_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index) ||
		          unlikely(DeeType_InheritGetRange(tp_self))) {
			Dee_ssize_t end_index;
			if (DeeInt_AsSSize(end, &end_index))
				goto err;
			return DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start, end_index);
		}
	} else {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange) ||
		          unlikely(DeeType_InheritGetRange(tp_self))) {
			DREF DeeObject *result;
			DREF DeeObject *start_ob = DeeInt_NewSSize(start);
			if unlikely(!start_ob)
				goto err;
			result = DeeType_INVOKE_GETRANGE(tp_self, self, start_ob, end);
			Dee_Decref(start_ob);
			return result;
		}
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetRangeEndIndex,
                (DeeObject *self, DeeObject *start, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(start);
	if (DeeInt_Check(start)) {
		if likely((tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index) || DeeType_InheritGetRange(tp_self)) {
			Dee_ssize_t start_index;
			if (DeeInt_AsSSize(start, &start_index))
				goto err;
			return DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start_index, end);
		}
	} else {
		if likely((tp_self->tp_seq && tp_self->tp_seq->tp_getrange) || DeeType_InheritGetRange(tp_self)) {
			DREF DeeObject *result;
			DREF DeeObject *end_ob = DeeInt_NewSSize(end);
			if unlikely(!end_ob)
				goto err;
			result = DeeType_INVOKE_GETRANGE(tp_self, self, start, end_ob);
			Dee_Decref(end_ob);
			return result;
		}
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetRangeIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index) ||
	          unlikely(DeeType_InheritGetRange(tp_self)))
		return DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start, end);
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetRangeIndexN,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index_n) ||
	          unlikely(DeeType_InheritGetRange(tp_self)))
		return DeeType_INVOKE_GETRANGEINDEXN(tp_self, self, start);
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
	return NULL;
}

DEFINE_OPERATOR(int, DelRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t start, DeeObject *end)) {
	LOAD_TP_SELF;
	if (DeeNone_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index_n) ||
		          unlikely(DeeType_InheritDelRange(tp_self)))
			return DeeType_INVOKE_DELRANGEINDEXN(tp_self, self, start);
	} else if (DeeInt_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index) ||
		          unlikely(DeeType_InheritDelRange(tp_self))) {
			Dee_ssize_t end_index;
			if (DeeInt_AsSSize(end, &end_index))
				goto err;
			return DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start, end_index);
		}
	} else {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange) ||
		          unlikely(DeeType_InheritDelRange(tp_self))) {
			int result;
			DREF DeeObject *start_ob = DeeInt_NewSSize(start);
			if unlikely(!start_ob)
				goto err;
			result = DeeType_INVOKE_DELRANGE(tp_self, self, start_ob, end);
			Dee_Decref(start_ob);
			return result;
		}
	}
	err_unimplemented_operator(tp_self, OPERATOR_DELRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, DelRangeEndIndex,
                (DeeObject *self, DeeObject *start, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	if (DeeInt_Check(start)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index) ||
		          unlikely(DeeType_InheritDelRange(tp_self))) {
			Dee_ssize_t start_index;
			if (DeeInt_AsSSize(start, &start_index))
				goto err;
			return DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start_index, end);
		}
	} else {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange) ||
		          unlikely(DeeType_InheritDelRange(tp_self))) {
			int result;
			DREF DeeObject *end_ob = DeeInt_NewSSize(end);
			if unlikely(!end_ob)
				goto err;
			result = DeeType_INVOKE_DELRANGE(tp_self, self, start, end_ob);
			Dee_Decref(end_ob);
			return result;
		}
	}
	err_unimplemented_operator(tp_self, OPERATOR_DELRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, DelRangeIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index) ||
	          unlikely(DeeType_InheritDelRange(tp_self)))
		return DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start, end);
	return err_unimplemented_operator(tp_self, OPERATOR_DELRANGE);
}

DEFINE_OPERATOR(int, DelRangeIndexN,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t start)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index_n) ||
	          unlikely(DeeType_InheritDelRange(tp_self)))
		return DeeType_INVOKE_DELRANGEINDEXN(tp_self, self, start);
	return err_unimplemented_operator(tp_self, OPERATOR_DELRANGE);
}

DEFINE_OPERATOR(int, SetRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t start, DeeObject *end, DeeObject *values)) {
	LOAD_TP_SELF;
	if (DeeNone_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index_n) ||
		          unlikely(DeeType_InheritSetRange(tp_self)))
			return DeeType_INVOKE_SETRANGEINDEXN(tp_self, self, start, values);
	} else if (DeeInt_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index) ||
		          unlikely(DeeType_InheritSetRange(tp_self))) {
			Dee_ssize_t end_index;
			if (DeeInt_AsSSize(end, &end_index))
				goto err;
			return DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start, end_index, values);
		}
	} else {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange) ||
		          unlikely(DeeType_InheritSetRange(tp_self))) {
			int result;
			DREF DeeObject *start_ob = DeeInt_NewSSize(start);
			if unlikely(!start_ob)
				goto err;
			result = DeeType_INVOKE_SETRANGE(tp_self, self, start_ob, end, values);
			Dee_Decref(start_ob);
			return result;
		}
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, SetRangeEndIndex,
                (DeeObject *self, DeeObject *start, Dee_ssize_t end, DeeObject *values)) {
	LOAD_TP_SELF;
	if (DeeInt_Check(start)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index) ||
		          unlikely(DeeType_InheritSetRange(tp_self))) {
			Dee_ssize_t start_index;
			if (DeeInt_AsSSize(start, &start_index))
				goto err;
			return DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start_index, end, values);
		}
	} else {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange) ||
		          unlikely(DeeType_InheritSetRange(tp_self))) {
			int result;
			DREF DeeObject *end_ob = DeeInt_NewSSize(end);
			if unlikely(!end_ob)
				goto err;
			result = DeeType_INVOKE_SETRANGE(tp_self, self, start, end_ob, values);
			Dee_Decref(end_ob);
			return result;
		}
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, SetRangeIndex,
                (DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index) ||
	          unlikely(DeeType_InheritSetRange(tp_self)))
		return DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start, end, values);
	return err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
}

DEFINE_OPERATOR(int, SetRangeIndexN,
                (DeeObject *self, Dee_ssize_t start, DeeObject *values)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index_n) ||
	          unlikely(DeeType_InheritSetRange(tp_self)))
		return DeeType_INVOKE_SETRANGEINDEXN(tp_self, self, start, values);
	return err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
}

#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
DEFINE_OPERATOR(DREF DeeObject *, GetRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t begin, DeeObject *end)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange) ||
	          unlikely(DeeType_InheritGetRange(tp_self))) {
		Dee_ssize_t end_index;
		struct type_nsi const *nsi;
		DREF DeeObject *begin_ob, *result;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (DeeNone_Check(end)) {
				if (nsi->nsi_seqlike.nsi_getrange_n)
					return (*nsi->nsi_seqlike.nsi_getrange_n)(self, begin);
			} else if (nsi->nsi_seqlike.nsi_getrange) {
				if (DeeObject_AsSSize(end, &end_index))
					goto err;
				return (*nsi->nsi_seqlike.nsi_getrange)(self, begin, end_index);
			}
		}
		begin_ob = DeeInt_NewSSize(begin);
		if unlikely(!begin_ob)
			goto err;
		result = DeeType_INVOKE_GETRANGE(tp_self, self, begin_ob, end);
		Dee_Decref(begin_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetRangeEndIndex,
                (DeeObject *self, DeeObject *begin, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange) ||
	          unlikely(DeeType_InheritGetRange(tp_self))) {
		Dee_ssize_t begin_index;
		DREF DeeObject *end_ob, *result;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_getrange) {
				if (DeeObject_AsSSize(begin, &begin_index))
					goto err;
				return (*nsi->nsi_seqlike.nsi_getrange)(self, begin_index, end);
			}
		}
		end_ob = DeeInt_NewSSize(end);
		if unlikely(!end_ob)
			goto err;
		result = DeeType_INVOKE_GETRANGE(tp_self, self, begin, end_ob);
		Dee_Decref(end_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetRangeIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t begin, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange) ||
	          unlikely(DeeType_InheritGetRange(tp_self))) {
		DREF DeeObject *begin_ob, *end_ob, *result;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_getrange)
				return (*nsi->nsi_seqlike.nsi_getrange)(self, begin, end);
		}
		begin_ob = DeeInt_NewSSize(begin);
		if unlikely(!begin_ob)
			goto err;
		end_ob = DeeInt_NewSSize(end);
		if unlikely(!end_ob) {
			Dee_Decref(begin_ob);
			goto err;
		}
		result = DeeType_INVOKE_GETRANGE(tp_self, self, begin_ob, end_ob);
		Dee_Decref(end_ob);
		Dee_Decref(begin_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
err:
	return NULL;
}

DEFINE_OPERATOR(int, DelRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t begin, DeeObject *end)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange) ||
	          unlikely(DeeType_InheritSetRange(tp_self))) {
		int result;
		DREF DeeObject *begin_ob;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (DeeNone_Check(end)) {
				if (nsi->nsi_seqlike.nsi_delrange_n)
					return (*nsi->nsi_seqlike.nsi_delrange_n)(self, begin);
			} else if (nsi->nsi_seqlike.nsi_delrange) {
				Dee_ssize_t end_index;
				if (DeeObject_AsSSize(end, &end_index))
					goto err;
				return (*nsi->nsi_seqlike.nsi_delrange)(self, begin, end_index);
			}
		}
		begin_ob = DeeInt_NewSSize(begin);
		if unlikely(!begin_ob)
			goto err;
		result = DeeType_INVOKE_DELRANGE(tp_self, self, begin_ob, end);
		Dee_Decref(begin_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, DelRangeEndIndex,
                (DeeObject *self, DeeObject *begin, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange) ||
	          unlikely(DeeType_InheritSetRange(tp_self))) {
		int result;
		DREF DeeObject *end_ob;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_delrange) {
				Dee_ssize_t start_index;
				if (DeeObject_AsSSize(begin, &start_index))
					goto err;
				return (*nsi->nsi_seqlike.nsi_delrange)(self, start_index, end);
			}
		}
		end_ob = DeeInt_NewSSize(end);
		if unlikely(!end_ob)
			goto err;
		result = DeeType_INVOKE_DELRANGE(tp_self, self, begin, end_ob);
		Dee_Decref(end_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, DelRangeIndex,
                (DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange) ||
	          unlikely(DeeType_InheritSetRange(tp_self))) {
		DREF DeeObject *begin_ob, *end_ob;
		int result;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_delrange)
				return (*nsi->nsi_seqlike.nsi_delrange)(self, begin, end);
		}
		begin_ob = DeeInt_NewSSize(begin);
		if unlikely(!begin_ob)
			goto err;
		end_ob = DeeInt_NewSSize(end);
		if unlikely(!end_ob) {
			Dee_Decref(begin_ob);
			goto err;
		}
		result = DeeType_INVOKE_DELRANGE(tp_self, self, begin_ob, end_ob);
		Dee_Decref(end_ob);
		Dee_Decref(begin_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, SetRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t begin, DeeObject *end, DeeObject *values)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	ASSERT_OBJECT(values);
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange) ||
	          unlikely(DeeType_InheritSetRange(tp_self))) {
		int result;
		DREF DeeObject *begin_ob;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (DeeNone_Check(end)) {
				if (nsi->nsi_seqlike.nsi_setrange_n)
					return (*nsi->nsi_seqlike.nsi_setrange_n)(self, begin, values);
			} else if (nsi->nsi_seqlike.nsi_setrange) {
				Dee_ssize_t end_index;
				if (DeeObject_AsSSize(end, &end_index))
					goto err;
				return (*nsi->nsi_seqlike.nsi_setrange)(self, begin, end_index, values);
			}
		}
		begin_ob = DeeInt_NewSSize(begin);
		if unlikely(!begin_ob)
			goto err;
		result = DeeType_INVOKE_SETRANGE(tp_self, self, begin_ob, end, values);
		Dee_Decref(begin_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, SetRangeEndIndex,
                (DeeObject *self, DeeObject *begin, Dee_ssize_t end, DeeObject *values)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(values);
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange) ||
	          unlikely(DeeType_InheritSetRange(tp_self))) {
		int result;
		DREF DeeObject *end_ob;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_setrange) {
				Dee_ssize_t start_index;
				if (DeeObject_AsSSize(begin, &start_index))
					goto err;
				return (*nsi->nsi_seqlike.nsi_setrange)(self, start_index, end, values);
			}
		}
		end_ob = DeeInt_NewSSize(end);
		if unlikely(!end_ob)
			goto err;
		result = DeeType_INVOKE_SETRANGE(tp_self, self, begin, end_ob, values);
		Dee_Decref(end_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

DEFINE_OPERATOR(int, SetRangeIndex,
                (DeeObject *self, Dee_ssize_t begin, Dee_ssize_t end, DeeObject *values)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(values);
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange) ||
	          unlikely(DeeType_InheritSetRange(tp_self))) {
		DREF DeeObject *begin_ob, *end_ob;
		int result;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_setrange)
				return (*nsi->nsi_seqlike.nsi_setrange)(self, begin, end, values);
		}
		begin_ob = DeeInt_NewSSize(begin);
		if unlikely(!begin_ob)
			goto err;
		end_ob = DeeInt_NewSSize(end);
		if unlikely(!end_ob) {
			Dee_Decref(begin_ob);
			goto err;
		}
		result = DeeType_INVOKE_SETRANGE(tp_self, self, begin_ob, end_ob, values);
		Dee_Decref(end_ob);
		Dee_Decref(begin_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}


DEFINE_OPERATOR(DREF DeeObject *, GetRangeIndexN,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t begin)) {
#ifdef DEFINE_TYPED_OPERATORS
	return DeeObject_TGetRangeBeginIndex(tp_self, self, begin, Dee_None);
#else /* DEFINE_TYPED_OPERATORS */
	return DeeObject_GetRangeBeginIndex(self, begin, Dee_None);
#endif /* !DEFINE_TYPED_OPERATORS */
}

DEFINE_OPERATOR(int, DelRangeIndexN, (DeeObject *RESTRICT_IF_NOTYPE self, Dee_ssize_t begin)) {
#ifdef DEFINE_TYPED_OPERATORS
	return DeeObject_TDelRangeBeginIndex(tp_self, self, begin, Dee_None);
#else /* DEFINE_TYPED_OPERATORS */
	return DeeObject_DelRangeBeginIndex(self, begin, Dee_None);
#endif /* !DEFINE_TYPED_OPERATORS */
}

DEFINE_OPERATOR(int, SetRangeIndexN, (DeeObject *self, Dee_ssize_t begin, DeeObject *values)) {
#ifdef DEFINE_TYPED_OPERATORS
	return DeeObject_TSetRangeBeginIndex(tp_self, self, begin, Dee_None, values);
#else /* DEFINE_TYPED_OPERATORS */
	return DeeObject_SetRangeBeginIndex(self, begin, Dee_None, values);
#endif /* !DEFINE_TYPED_OPERATORS */
}
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */


DEFINE_OPERATOR(int, DelRange,
                (DeeObject *self, DeeObject *start, DeeObject *end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange) ||
	          unlikely(DeeType_InheritDelRange(tp_self)))
		return DeeType_INVOKE_DELRANGE(tp_self, self, start, end);
	return err_unimplemented_operator(tp_self, OPERATOR_DELRANGE);
}

DEFINE_OPERATOR(int, SetRange,
                (DeeObject *self, DeeObject *start,
                 DeeObject *end, DeeObject *value)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange) ||
	          unlikely(DeeType_InheritSetRange(tp_self)))
		return DeeType_INVOKE_SETRANGE(tp_self, self, start, end, value);
	return err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
}



#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
DEFINE_OPERATOR(int, BoundItem, (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_bounditem) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_BOUNDITEM(tp_self, self, index);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

DEFINE_OPERATOR(int, BoundItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_bounditem_index) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_BOUNDITEMINDEX(tp_self, self, index);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

DEFINE_OPERATOR(int, BoundItemStringHash, (DeeObject *self, char const *__restrict key, dhash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_bounditem_string_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_BOUNDITEMSTRINGHASH(tp_self, self, key, hash);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

DEFINE_OPERATOR(int, BoundItemStringLenHash, (DeeObject *self, char const *__restrict key, size_t keylen, dhash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_bounditem_string_len_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_BOUNDITEMSTRINGLENHASH(tp_self, self, key, keylen, hash);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

/* Check if a given item exists (`deemon.hasitem(self, index)')
 * @return: 1: Does exists
 * @return: 0:  Doesn't exist
 * @return: -1:     Error. */
DEFINE_OPERATOR(int, HasItem, (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_hasitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_HASITEM(tp_self, self, index);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

DEFINE_OPERATOR(int, HasItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_hasitem_index) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_HASITEMINDEX(tp_self, self, index);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

DEFINE_OPERATOR(int, HasItemStringHash, (DeeObject *self, char const *__restrict key, dhash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_hasitem_string_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_HASITEMSTRINGHASH(tp_self, self, key, hash);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

DEFINE_OPERATOR(int, HasItemStringLenHash, (DeeObject *self, char const *__restrict key, size_t keylen, dhash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_hasitem_string_len_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_HASITEMSTRINGLENHASH(tp_self, self, key, keylen, hash);
	return err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItem, (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_trygetitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_TRYGETITEM(tp_self, self, index);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_trygetitem_index) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_TRYGETITEMINDEX(tp_self, self, index);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItemStringHash, (DeeObject *self, char const *__restrict key, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_trygetitem_string_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_TRYGETITEMSTRINGHASH(tp_self, self, key, hash);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItemStringLenHash, (DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_trygetitem_string_len_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_TRYGETITEMSTRINGLENHASH(tp_self, self, key, keylen, hash);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem_index) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_GETITEMINDEX(tp_self, self, index);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(int, DelItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delitem_index) ||
	          unlikely(DeeType_InheritDelItem(tp_self)))
		return DeeType_INVOKE_DELITEMINDEX(tp_self, self, index);
	return err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
}

DEFINE_OPERATOR(int, SetItemIndex, (DeeObject *self, size_t index, DeeObject *value)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setitem_index) ||
	          unlikely(DeeType_InheritSetItem(tp_self)))
		return DeeType_INVOKE_SETITEMINDEX(tp_self, self, index, value);
	return err_unimplemented_operator(tp_self, OPERATOR_SETITEM);
}

DEFINE_OPERATOR(DREF DeeObject *, GetItemStringHash, (DeeObject *self, char const *__restrict key, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem_string_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_GETITEMSTRINGHASH(tp_self, self, key, hash);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItemStringLenHash, (DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem_string_len_hash) ||
	          unlikely(DeeType_InheritGetItem(tp_self)))
		return DeeType_INVOKE_GETITEMSTRINGLENHASH(tp_self, self, key, keylen, hash);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(int, DelItemStringHash, (DeeObject *self, char const *__restrict key, dhash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delitem_string_hash) ||
	          unlikely(DeeType_InheritDelItem(tp_self)))
		return DeeType_INVOKE_DELITEMSTRINGHASH(tp_self, self, key, hash);
	return err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
}

DEFINE_OPERATOR(int, DelItemStringLenHash, (DeeObject *self, char const *__restrict key, size_t keylen, dhash_t hash)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delitem_string_len_hash) ||
	          unlikely(DeeType_InheritDelItem(tp_self)))
		return DeeType_INVOKE_DELITEMSTRINGLENHASH(tp_self, self, key, keylen, hash);
	return err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
}

DEFINE_OPERATOR(int, SetItemStringHash, (DeeObject *self, char const *__restrict key, dhash_t hash, DeeObject *value)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setitem_string_hash) ||
	          unlikely(DeeType_InheritSetItem(tp_self)))
		return DeeType_INVOKE_SETITEMSTRINGHASH(tp_self, self, key, hash, value);
	return err_unimplemented_operator(tp_self, OPERATOR_SETITEM);
}

DEFINE_OPERATOR(int, SetItemStringLenHash, (DeeObject *self, char const *__restrict key, size_t keylen, dhash_t hash, DeeObject *value)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setitem_string_len_hash) ||
	          unlikely(DeeType_InheritSetItem(tp_self)))
		return DeeType_INVOKE_SETITEMSTRINGLENHASH(tp_self, self, key, keylen, hash, value);
	return err_unimplemented_operator(tp_self, OPERATOR_SETITEM);
}

#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
DEFINE_OPERATOR(int, BoundItem, (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	size_t i;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi) {
			if (nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					size_t size;
					if (DeeObject_AsSize(index, &i))
						goto err;
					if (nsi->nsi_seqlike.nsi_getsize_fast) {
						size = (*nsi->nsi_seqlike.nsi_getsize_fast)(self);
					} else {
						size = (*nsi->nsi_seqlike.nsi_getsize)(self);
						if unlikely(size == (size_t)-1)
							goto err;
					}
					if unlikely(i >= size) {
						/* Bad index */
						return -2;
					}
					result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, i);
					if (!result)
						return 0;
					Dee_Decref(result);
					return 1;
				}
				if (nsi->nsi_seqlike.nsi_getitem) {
					if (DeeObject_AsSize(index, &i))
						goto err;
					result = (*nsi->nsi_seqlike.nsi_getitem)(self, i);
					goto check_result;
				}
			} else if (nsi->nsi_class == TYPE_SEQX_CLASS_MAP) {
				if (nsi->nsi_maplike.nsi_getdefault) {
					result = (*nsi->nsi_maplike.nsi_getdefault)(self, index, ITER_DONE);
					if (result == ITER_DONE)
						return -2;
					goto check_result;
				}
				/* We can't actually substitute with `operator contains', because
				 * if we did that, we may get a false positive for a valid key, but
				 * an unbound key none-the-less. */
			}
		}

		/* Fallback: create an integer object and use it for indexing. */
		result = DeeType_INVOKE_GETITEM(tp_self, self, index);
check_result:
		if (!result) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				return 0;
			if (DeeError_Catch(&DeeError_IndexError) ||
			    DeeError_Catch(&DeeError_KeyError))
				return -2;
			goto err;
		}
		Dee_Decref(result);
		return 1;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
err:
	return -1;
}

DEFINE_OPERATOR(int, BoundItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result, *index_ob;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi) {
			if (nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					size_t size;
					if (nsi->nsi_seqlike.nsi_getsize_fast) {
						size = (*nsi->nsi_seqlike.nsi_getsize_fast)(self);
					} else {
						size = (*nsi->nsi_seqlike.nsi_getsize)(self);
						if unlikely(size == (size_t)-1)
							goto err;
					}
					if unlikely(index >= size)
						return -2; /* Bad index */
					result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, index);
					if (!result)
						return 0;
					Dee_Decref(result);
					return 1;
				}
				if (nsi->nsi_seqlike.nsi_getitem) {
					result = (*nsi->nsi_seqlike.nsi_getitem)(self, index);
					goto check_result;
				}
			} else if (nsi->nsi_class == TYPE_SEQX_CLASS_MAP) {
				if (nsi->nsi_maplike.nsi_getdefault) {
					index_ob = DeeInt_NewSize(index);
					if unlikely(!index_ob)
						goto err;
					result = (*nsi->nsi_maplike.nsi_getdefault)(self, index_ob, ITER_DONE);
					if (result == ITER_DONE) {
						Dee_Decref(index_ob);
						return -2;
					}
					Dee_Decref(index_ob);
					goto check_result;
				}
				/* We can't actually substitute with `operator contains', because
				 * if we did that, we may get a false positive for a valid key, but
				 * an unbound key none-the-less. */
			}
		}

		/* Fallback: create an integer object and use it for indexing. */
		index_ob = DeeInt_NewSize(index);
		if unlikely(!index_ob)
			goto err;
		result = DeeType_INVOKE_GETITEM(tp_self, self, index_ob);
		Dee_Decref(index_ob);
check_result:
		if (!result) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				return 0;
			if (DeeError_Catch(&DeeError_IndexError) ||
			    DeeError_Catch(&DeeError_KeyError))
				return -2;
			goto err;
		}
		Dee_Decref(result);
		return 1;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
err:
	return -1;
}

DEFINE_OPERATOR(int, BoundItemStringHash, (DeeObject *self, char const *key, dhash_t hash)) {
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;

	/* Optimization for specific types. */
again:
	if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeObject_BoundItem(orig_self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_OPERATOR(int, BoundItemStringLenHash,
                (DeeObject *self, char const *key, size_t keylen, dhash_t hash)) {
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;

	/* Optimization for specific types. */
again:
	if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeObject_BoundItem(orig_self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}


/* Check if a given item exists (`deemon.hasitem(self, index)')
 * @return: 1: Does exists
 * @return: 0:  Doesn't exist
 * @return: -1:     Error. */
DEFINE_OPERATOR(int, HasItem, (DeeObject *self, DeeObject *index)) {
	DREF DeeObject *result;
	size_t i;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi) {
			if (nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				size_t size;
				if (DeeObject_AsSize(index, &i))
					goto err;
				if (nsi->nsi_seqlike.nsi_getsize_fast) {
					size = (*nsi->nsi_seqlike.nsi_getsize_fast)(self);
				} else {
					size = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(size == (size_t)-1)
						goto err;
				}
				return i < size;
			} else if (nsi->nsi_class == TYPE_SEQX_CLASS_MAP) {
				if (nsi->nsi_maplike.nsi_getdefault) {
					result = (*nsi->nsi_maplike.nsi_getdefault)(self, index, ITER_DONE);
					if (result == ITER_DONE)
						return 0;
					goto check_result;
				}
				/* We can't actually substitute with `operator contains', because
				 * if we did that, we may get a false positive for a valid key, but
				 * an unbound key none-the-less. */
			}
		}

		/* Fallback: create an integer object and use it for indexing. */
		result = DeeType_INVOKE_GETITEM(tp_self, self, index);
check_result:
		if (!result) {
			if (DeeError_Catch(&DeeError_UnboundItem) ||
			    DeeError_Catch(&DeeError_IndexError) ||
			    DeeError_Catch(&DeeError_KeyError))
				return 0;
			goto err;
		}
		Dee_Decref(result);
		return 1;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
err:
	return -1;
}

DEFINE_OPERATOR(int, HasItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result, *index_ob;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi) {
			if (nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				size_t size;
				if (nsi->nsi_seqlike.nsi_getsize_fast) {
					size = (*nsi->nsi_seqlike.nsi_getsize_fast)(self);
				} else {
					size = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(size == (size_t)-1)
						goto err;
				}
				return index < size;
			} else if (nsi->nsi_class == TYPE_SEQX_CLASS_MAP) {
				if (nsi->nsi_maplike.nsi_getdefault) {
					index_ob = DeeInt_NewSize(index);
					if unlikely(!index_ob)
						goto err;
					result = (*nsi->nsi_maplike.nsi_getdefault)(self, index_ob, ITER_DONE);
					Dee_Decref(index_ob);
					if (result == ITER_DONE)
						return 0;
					goto check_result;
				}
				/* We can't actually substitute with `operator contains', because
				 * if we did that, we may get a false positive for a valid key, but
				 * an unbound key none-the-less. */
			}
		}

		/* Fallback: create an integer object and use it for indexing. */
		index_ob = DeeInt_NewSize(index);
		if unlikely(!index_ob)
			goto err;
		result = DeeType_INVOKE_GETITEM(tp_self, self, index_ob);
		Dee_Decref(index_ob);
check_result:
		if (!result) {
			if (DeeError_Catch(&DeeError_UnboundItem) ||
			    DeeError_Catch(&DeeError_IndexError) ||
			    DeeError_Catch(&DeeError_KeyError))
				return 0;
			goto err;
		}
		Dee_Decref(result);
		return 1;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
err:
	return -1;
}

DEFINE_OPERATOR(int, HasItemStringHash, (DeeObject *self, char const *key, dhash_t hash)) {
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;

	/* Optimization for specific types. */
again:
	if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeObject_HasItem(orig_self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_OPERATOR(int, HasItemStringLenHash, (DeeObject *self, char const *key, size_t keylen, dhash_t hash)) {
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *keyob;
	LOAD_TP_SELF;

	/* Optimization for specific types. */
again:
	if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeObject_HasItem(orig_self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItem, (DeeObject *self, DeeObject *key)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP) {
			if (nsi->nsi_maplike.nsi_getdefault)
				return (*nsi->nsi_maplike.nsi_getdefault)(self, key, ITER_DONE);
		}
		result = DeeType_INVOKE_GETITEM(tp_self, self, key);
		if unlikely(!result) {
			if (DeeError_Catch(&DeeError_KeyError) ||
			    DeeError_Catch(&DeeError_NotImplemented))
				return ITER_DONE;
		}
		return result;
	}
	return ITER_DONE;
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *result;
	DREF DeeObject *index_ob;
	index_ob = DeeInt_NewSize(index);
	if unlikely(!index_ob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TTryGetItem(tp_self, self, index_ob);
#else /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_TryGetItem(self, index_ob);
#endif /* !DEFINE_TYPED_OPERATORS */
	Dee_Decref(index_ob);
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItemStringHash,
                (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TTryGetItem(tp_self, self, keyob);
#else /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_TryGetItem(self, keyob);
#endif /* !DEFINE_TYPED_OPERATORS */
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, TryGetItemStringLenHash,
                (DeeObject *RESTRICT_IF_NOTYPE self, char const *key, size_t keylen, Dee_hash_t hash)) {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TTryGetItem(tp_self, self, keyob);
#else /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_TryGetItem(self, keyob);
#endif /* !DEFINE_TYPED_OPERATORS */
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *index_ob, *result;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getitem) ||
	          unlikely(DeeType_InheritGetItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_getitem)
				return (*nsi->nsi_seqlike.nsi_getitem)(self, index);
		}

		/* Fallback: create an integer object and use it for indexing. */
		index_ob = DeeInt_NewSize(index);
		if unlikely(!index_ob)
			goto err;
		result = DeeType_INVOKE_GETITEM(tp_self, self, index_ob);
		Dee_Decref(index_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
err:
	return NULL;
}

DEFINE_OPERATOR(int, DelItemIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, size_t index)) {
	DREF DeeObject *index_ob;
	int result;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delitem) ||
	          unlikely(DeeType_InheritDelItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_delitem)
				return (*nsi->nsi_seqlike.nsi_delitem)(self, index);
		}

		/* Fallback: create an integer object and use it for indexing. */
		index_ob = DeeInt_NewSize(index);
		if unlikely(!index_ob)
			goto err;
		result = DeeType_INVOKE_DELITEM(tp_self, self, index_ob);
		Dee_Decref(index_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
err:
	return -1;
}

DEFINE_OPERATOR(int, SetItemIndex, (DeeObject *self, size_t index, DeeObject *value)) {
	DREF DeeObject *index_ob;
	int result;
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setitem) ||
	          unlikely(DeeType_InheritSetItem(tp_self))) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_setitem)
				return (*nsi->nsi_seqlike.nsi_setitem)(self, index, value);
		}

		/* Fallback: create an integer object and use it for indexing. */
		index_ob = DeeInt_NewSize(index);
		if unlikely(!index_ob)
			goto err;
		result = DeeType_INVOKE_SETITEM(tp_self, self, index_ob, value);
		Dee_Decref(index_ob);
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
err:
	return -1;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItemStringHash, (DeeObject *self, char const *key, dhash_t hash)) {
	DREF DeeObject *keyob, *result;
	LOAD_TP_SELF;

	/* Optimization for specific types. */
again:
	if (tp_self == &DeeDict_Type) {
		return DeeDict_GetItemStringHash(self, key, hash);
	} else if (tp_self == &DeeRoDict_Type) {
		result = DeeRoDict_GetItemNRStringHash((DeeRoDictObject *)self, key, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		result = DeeKwdsMapping_GetItemNRStringHash((DeeKwdsMappingObject *)self, key, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		result = DeeBlackListKwds_GetItemNRStringHash((DeeBlackListKwdsObject *)self, key, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKw_Type) {
		result = DeeBlackListKw_GetItemNRStringHash((DeeBlackListKwObject *)self, key, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeCachedDict_Type) {
		result = DeeCachedDict_GetItemNRStringHash((DeeCachedDictObject *)self, key, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeObject_TGetItem(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
xincref_result_and_return:
	Dee_XIncref(result);
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItemStringLenHash,
                (DeeObject *self, char const *key, size_t keylen, dhash_t hash)) {
	DREF DeeObject *keyob, *result;
	LOAD_TP_SELF;

	/* Optimization for specific types. */
again:
	if (tp_self == &DeeDict_Type) {
		return DeeDict_GetItemStringLenHash(self, key, keylen, hash);
	} else if (tp_self == &DeeRoDict_Type) {
		result = DeeRoDict_GetItemNRStringLenHash((DeeRoDictObject *)self, key, keylen, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		result = DeeKwdsMapping_GetItemNRStringLenHash((DeeKwdsMappingObject *)self, key, keylen, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		result = DeeBlackListKwds_GetItemNRStringLenHash((DeeBlackListKwdsObject *)self, key, keylen, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKw_Type) {
		result = DeeBlackListKw_GetItemNRStringLenHash((DeeBlackListKwObject *)self, key, keylen, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeCachedDict_Type) {
		result = DeeCachedDict_GetItemNRStringLenHash((DeeCachedDictObject *)self, key, keylen, hash);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeObject_TGetItem(tp_self, self, keyob);
	Dee_Decref(keyob);
	return result;
xincref_result_and_return:
	Dee_XIncref(result);
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR(int, DelItemStringHash, (DeeObject *self, char const *key, dhash_t hash)) {
	DREF DeeObject *keyob;
	int result;
	LOAD_TP_SELF;
	if (tp_self == &DeeDict_Type)
		return DeeDict_DelItemStringHash(self, key, hash);

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TDelItem(tp_self, self, keyob);
#else /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_DelItem(self, keyob);
#endif /* !DEFINE_TYPED_OPERATORS */
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_OPERATOR(int, DelItemStringLenHash,
                (DeeObject *self, char const *key, size_t keylen, dhash_t hash)) {
	DREF DeeObject *keyob;
	int result;
	LOAD_TP_SELF;
	if (tp_self == &DeeDict_Type)
		return DeeDict_DelItemStringLenHash(self, key, keylen, hash);

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TDelItem(tp_self, self, keyob);
#else /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_DelItem(self, keyob);
#endif /* !DEFINE_TYPED_OPERATORS */
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_OPERATOR(int, SetItemStringHash,
                (DeeObject *self, char const *key, dhash_t hash, DeeObject *value)) {
	DREF DeeObject *keyob;
	int result;
	LOAD_TP_SELF;
	if (tp_self == &DeeDict_Type)
		return DeeDict_SetItemStringHash(self, key, hash, value);

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TSetItem(tp_self, self, keyob, value);
#else /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_SetItem(self, keyob, value);
#endif /* !DEFINE_TYPED_OPERATORS */
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}

DEFINE_OPERATOR(int, SetItemStringLenHash,
                (DeeObject *self, char const *key, size_t keylen, dhash_t hash, DeeObject *value)) {
	DREF DeeObject *keyob;
	int result;
	LOAD_TP_SELF;
	if (tp_self == &DeeDict_Type)
		return DeeDict_SetItemStringLenHash(self, key, keylen, hash, value);

	/* Fallback: create a temporary string object and use it for indexing. */
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
#ifdef DEFINE_TYPED_OPERATORS
	result = DeeObject_TSetItem(tp_self, self, keyob, value);
#else /* DEFINE_TYPED_OPERATORS */
	result = DeeObject_SetItem(self, keyob, value);
#endif /* !DEFINE_TYPED_OPERATORS */
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */


#ifndef DEFINE_TYPED_OPERATORS
INTDEF int DCALL none_i1(void *UNUSED(a));

INTERN NONNULL((1)) bool DCALL
DeeType_InheritWith(DeeTypeObject *__restrict self) {
	struct type_with *base_with;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_with = self->tp_with;
	if (base_with) {
		if (base_with->tp_enter) {
			/* Special case: When `tp_enter' is implemented,
			 * a missing `tp_leave' behaves as a no-op. */
			if (base_with->tp_leave == NULL)
				base_with->tp_leave = (int (DCALL *)(DeeObject *__restrict))&none_i1;
			return true;
		} else if (base_with->tp_leave) {
			/* Special case: When `tp_leave' is implemented,
			 * a missing `tp_enter' behaves as a no-op. */
			if (base_with->tp_enter == NULL)
				base_with->tp_enter = (int (DCALL *)(DeeObject *__restrict))&none_i1;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_with = base->tp_with;
		if (base_with == NULL || (base_with->tp_enter == NULL ||
		                          base_with->tp_leave == NULL)) {
			if (!DeeType_InheritWith(base))
				continue;
			base_with = base->tp_with;
		}
		LOG_INHERIT(base, self, "operator <with>");
		if unlikely(self->tp_with) {
			self->tp_with->tp_enter = base_with->tp_enter;
			self->tp_with->tp_leave = base_with->tp_leave;
		} else {
			self->tp_with = base_with;
		}
		return true;
	}
	return false;
}
#endif /* !DEFINE_TYPED_OPERATORS */

/* With-operator invocation:
 * >> with (my_object) {
 * >>     ...
 * >> }
 * Translates to:
 * >> DeeObject_Enter(my_object);
 * >> try {
 * >>     ...
 * >> } finally {
 * >>    DeeObject_Leave(my_object);
 * >> } */
DEFINE_OPERATOR(int, Enter,
               (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_with && tp_self->tp_with->tp_enter) ||
	          unlikely(DeeType_InheritWith(tp_self)))
		return DeeType_INVOKE_ENTER(tp_self, self);
	return err_unimplemented_operator(tp_self, OPERATOR_ENTER);
}

DEFINE_OPERATOR(int, Leave,
               (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_with && tp_self->tp_with->tp_leave) ||
	          unlikely(DeeType_InheritWith(tp_self)))
		return DeeType_INVOKE_LEAVE(tp_self, self);
	return err_unimplemented_operator(tp_self, OPERATOR_LEAVE);
}


#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
DeeType_InheritBuffer(DeeTypeObject *__restrict self) {
	struct type_buffer *base_buffer;
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_buffer = base->tp_buffer;
		if (base_buffer == NULL || !base_buffer->tp_getbuf) {
			if (!DeeType_InheritBuffer(base))
				continue;
			base_buffer = base->tp_buffer;
		}
		LOG_INHERIT(base, self, "<BUFFER>");
		if unlikely(self->tp_buffer) {
			memcpy(self->tp_buffer, base_buffer, sizeof(struct type_buffer));
		} else {
			self->tp_buffer = base_buffer;
		}
		return true;
	}
	return false;
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(int, GetBuf, (DeeObject *RESTRICT_IF_NOTYPE self,
                              DeeBuffer *__restrict info, unsigned int flags)) {
	ASSERTF(!(flags & ~(Dee_BUFFER_FWRITABLE)),
	        "Unknown buffers flags in %x", flags);
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_buffer && tp_self->tp_buffer->tp_getbuf) ||
	          unlikely(DeeType_InheritBuffer(tp_self))) {
		if unlikely((flags & Dee_BUFFER_FWRITABLE) &&
		            (tp_self->tp_buffer->tp_buffer_flags & Dee_BUFFER_TYPE_FREADONLY)) {
			DeeError_Throwf(&DeeError_BufferError,
			                "Cannot write to read-only buffer of type %k",
			                tp_self);
			goto err;
		}
#ifndef __INTELLISENSE__
		info->bb_put = tp_self->tp_buffer->tp_putbuf;
#endif /* !__INTELLISENSE__ */
		return (*tp_self->tp_buffer->tp_getbuf)(self, info, flags);
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETBUF);
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(void, PutBuf,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 DeeBuffer *__restrict info,
                 unsigned int flags)) {
	ASSERTF(!(flags & ~(Dee_BUFFER_FWRITABLE)),
	        "Unknown buffers flags in %x", flags);
#ifdef __INTELLISENSE__
	(void)self;
	(void)info;
	(void)flags;
#elif !defined(DEFINE_TYPED_OPERATORS)
	if (info->bb_put)
		(*info->bb_put)(self, info, flags);
#else /* !DEFINE_TYPED_OPERATORS */
	LOAD_TP_SELF;
	if ((tp_self->tp_buffer && tp_self->tp_buffer->tp_putbuf) ||
	    (DeeType_InheritBuffer(tp_self) && tp_self->tp_buffer->tp_putbuf))
		(*tp_self->tp_buffer->tp_putbuf)(self, info, flags);
#endif /* DEFINE_TYPED_OPERATORS */
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(Dee_ssize_t, Foreach,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_t proc, void *arg)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_foreach) ||
	          unlikely(DeeType_InheritIter(tp_self)))
		return (*tp_self->tp_seq->tp_foreach)(self, proc, arg);
	return err_unimplemented_operator(tp_self, OPERATOR_ITER);
}

DEFINE_OPERATOR(Dee_ssize_t, ForeachPair,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_foreach_pair_t proc, void *arg)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_foreach_pair) ||
	          unlikely(DeeType_InheritIter(tp_self)))
		return (*tp_self->tp_seq->tp_foreach_pair)(self, proc, arg);
	return err_unimplemented_operator(tp_self, OPERATOR_ITER);
}

/* Enumerate valid keys/indices of "self", as well as their current value.
 * @return: * : Sum of return values of `*proc'
 * @return: -1: An error occurred during iteration (or potentially inside of `*proc') */
DEFINE_OPERATOR(Dee_ssize_t, Enumerate,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_t proc, void *arg)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_enumerate) ||
	          unlikely(DeeType_InheritIter(tp_self) && tp_self->tp_seq->tp_enumerate))
		return (*tp_self->tp_seq->tp_enumerate)(self, proc, arg);
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate non-sequence type `%r'",
	                       tp_self);
}

/* Same as `DeeObject_Enumerate()', but only valid when "self" uses integers for indices
 * or is a mapping where all keys are integers. In the former case, [start,end)
 * can be given in order to allow the implementation to only enumerate indices that fall
 * within that range (though an implementation is allowed to simply ignore these arguments)
 * If you want to always enumerate all indices (like is also done by `DeeObject_Enumerate',
 * then simply pass `start = 0, end = (size_t)-1')
 * @return: * : Sum of return values of `*proc'
 * @return: -1: An error occurred during iteration (or potentially inside of `*proc') */
DEFINE_OPERATOR(Dee_ssize_t, EnumerateIndex,
                (DeeObject *RESTRICT_IF_NOTYPE self, Dee_enumerate_index_t proc,
                 void *arg, size_t start, size_t end)) {
	LOAD_TP_SELF;
	if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_enumerate_index) ||
	          unlikely(DeeType_InheritIter(tp_self) && tp_self->tp_seq->tp_enumerate_index))
		return (*tp_self->tp_seq->tp_enumerate_index)(self, proc, arg, start, end);
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Cannot enumerate non-sequence type `%r'",
	                       tp_self);
}


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED ATTR_OUTS(3, 2) NONNULL((1)) int
(DCALL DeeObject_Unpack)(DeeObject *__restrict self, size_t objc,
                         /*out*/ DREF DeeObject **__restrict objv) {
	DREF DeeObject *iterator, *elem;
	size_t fast_size, i;

	/* TODO: Use DeeObject_Foreach() */

	/* Try to make use of the fast-sequence API. */
	fast_size = DeeFastSeq_GetSize_deprecated(self);
	if (fast_size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (objc != fast_size) {
			if (DeeNone_Check(self)) {
				/* Special case: `none' can be unpacked into anything. */
				memsetp(objv, Dee_None, objc);
				Dee_Incref_n(Dee_None, objc);
				return 0;
			}
			return err_invalid_unpack_size(self, objc, fast_size);
		}
		for (i = 0; i < objc; ++i) {
			elem = DeeFastSeq_GetItem_deprecated(self, i);
			if unlikely(!elem)
				goto err_objv;
			objv[i] = elem; /* Inherit reference. */
		}
		return 0;
	}

	/* Fallback: Use an iterator. */
	if ((iterator = DeeObject_Iter(self)) == NULL)
		goto err;
	for (i = 0; i < objc; ++i) {
		elem = DeeObject_IterNext(iterator);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem)
				err_invalid_unpack_size(self, objc, i);
			goto err_iter_objv;
		}
		objv[i] = elem; /* Inherit reference. */
	}

	/* Check to make sure that the iterator actually ends here. */
	elem = DeeObject_IterNext(iterator);
	if unlikely(elem != ITER_DONE) {
		if (elem)
			err_invalid_unpack_iter_size(self, iterator, objc);
		goto err_iter_objv;
	}
	Dee_Decref(iterator);
	return 0;
err_iter_objv:
	Dee_Decref(iterator);
err_objv:
	Dee_Decrefv(objv, i);
err:
	return -1;
}


/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed < key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed > key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_CompareKey)(DeeObject *lhs_keyed,
                             DeeObject *rhs, DeeObject *key) {
	int result;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err;
	result = DeeObject_Compare(lhs_keyed, rhs);
	Dee_Decref(rhs);
	return result;
err:
	return Dee_COMPARE_ERR;
}

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed != key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed != key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_CompareKeyEq)(DeeObject *lhs_keyed,
                               DeeObject *rhs, DeeObject *key) {
	int result;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err;
	result = DeeObject_CompareEq(lhs_keyed, rhs);
	Dee_Decref(rhs);
	return result;
err:
	return Dee_COMPARE_ERR;
}

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed != key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed != key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TryCompareKeyEq)(DeeObject *lhs_keyed,
                                  DeeObject *rhs, DeeObject *key) {
	int result;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err;
	result = DeeObject_TryCompareEq(lhs_keyed, rhs);
	Dee_Decref(rhs);
	return result;
err:
	return Dee_COMPARE_ERR;
}

#endif /* !DEFINE_TYPED_OPERATORS */


#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
DeeType_InheritNSI(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || !base_seq->tp_nsi) {
			if (!DeeType_InheritNSI(base))
				continue;
		}
		if (self->tp_seq != NULL) /* Some other sequence interface has already been implemented! */
			return false;
		LOG_INHERIT(base, self, "<NSI>");
		self->tp_seq = base->tp_seq;
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritNII(DeeTypeObject *__restrict self) {
	struct type_cmp *base_cmp;
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_cmp = base->tp_cmp;
		if (base_cmp == NULL || !base_cmp->tp_nii) {
			if (!DeeType_InheritNSI(base))
				continue;
		}
		if (self->tp_cmp != NULL) /* Some other iterator-compare interface has already been implemented! */
			return false;
		LOG_INHERIT(base, self, "<NII>");
		self->tp_cmp = base->tp_cmp;
		return true;
	}
	return false;
}
#endif /* !DEFINE_TYPED_OPERATORS */

#undef GET_TP_SELF
#undef GET_TP_PSELF
#undef LOAD_ITER
#undef LOAD_ITERP
#undef LOAD_TP_SELF
#undef DEFINE_INTERNAL_OPERATOR
#undef DEFINE_INTERNAL_SEQ_OPERATOR
#undef DEFINE_OPERATOR

#undef RESTRICT_IF_NOTYPE
#undef LOG_INHERIT

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_C */
