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

#include <hybrid/int128.h>
#include <hybrid/typecore.h>

#include <stdarg.h>

#include "../objects/int_logic.h"
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


#ifndef DEFINE_OPERATOR
#define DEFINE_OPERATOR(return, name, args) \
	PUBLIC return (DCALL DeeObject_##name)args
#endif /* !DEFINE_OPERATOR */
#ifndef DEFINE_INTERNAL_OPERATOR
#define DEFINE_INTERNAL_OPERATOR(return, name, args) \
	INTERN return (DCALL DeeObject_##name)args
#endif /* !DEFINE_INTERNAL_OPERATOR */

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
#define DeeType_INVOKE_ASSIGN              DeeType_InvokeInitAssign
#define DeeType_INVOKE_MOVEASSIGN          DeeType_InvokeInitMoveAssign
#define DeeType_INVOKE_STR                 DeeType_InvokeCastStr
#define DeeType_INVOKE_STR_NODEFAULT       DeeType_InvokeCastStr_NODEFAULT
#define DeeType_INVOKE_PRINT               DeeType_InvokeCastPrint
#define DeeType_INVOKE_PRINT_NODEFAULT     DeeType_InvokeCastPrint_NODEFAULT
#define DeeType_INVOKE_REPR                DeeType_InvokeCastRepr
#define DeeType_INVOKE_REPR_NODEFAULT      DeeType_InvokeCastRepr_NODEFAULT
#define DeeType_INVOKE_PRINTREPR           DeeType_InvokeCastPrintRepr
#define DeeType_INVOKE_PRINTREPR_NODEFAULT DeeType_InvokeCastPrintRepr_NODEFAULT
#define DeeType_INVOKE_BOOL                DeeType_InvokeCastBool
#define DeeType_INVOKE_NEXT                DeeType_InvokeIterNext
#define DeeType_INVOKE_CALL                DeeType_InvokeCall
#define DeeType_INVOKE_CALL_NODEFAULT      DeeType_InvokeCall_NODEFAULT
#define DeeType_INVOKE_CALLKW              DeeType_InvokeCallKw
#define DeeType_INVOKE_CALLKW_NODEFAULT    DeeType_InvokeCallKw_NODEFAULT
#define DeeType_INVOKE_INT32               DeeType_InvokeMathInt32
#define DeeType_INVOKE_INT32_NODEFAULT     DeeType_InvokeMathInt32_NODEFAULT
#define DeeType_INVOKE_INT64               DeeType_InvokeMathInt64
#define DeeType_INVOKE_INT64_NODEFAULT     DeeType_InvokeMathInt64_NODEFAULT
#define DeeType_INVOKE_DOUBLE              DeeType_InvokeMathDouble
#define DeeType_INVOKE_DOUBLE_NODEFAULT    DeeType_InvokeMathDouble_NODEFAULT
#define DeeType_INVOKE_INT                 DeeType_InvokeMathInt
#define DeeType_INVOKE_INT_NODEFAULT       DeeType_InvokeMathInt_NODEFAULT
#define DeeType_INVOKE_INV                 DeeType_InvokeMathInv
#define DeeType_INVOKE_POS                 DeeType_InvokeMathPos
#define DeeType_INVOKE_NEG                 DeeType_InvokeMathNeg
#define DeeType_INVOKE_ADD                 DeeType_InvokeMathAdd
#define DeeType_INVOKE_ADD_NODEFAULT       DeeType_InvokeMathAdd_NODEFAULT
#define DeeType_INVOKE_SUB                 DeeType_InvokeMathSub
#define DeeType_INVOKE_SUB_NODEFAULT       DeeType_InvokeMathSub_NODEFAULT
#define DeeType_INVOKE_MUL                 DeeType_InvokeMathMul
#define DeeType_INVOKE_MUL_NODEFAULT       DeeType_InvokeMathMul_NODEFAULT
#define DeeType_INVOKE_DIV                 DeeType_InvokeMathDiv
#define DeeType_INVOKE_DIV_NODEFAULT       DeeType_InvokeMathDiv_NODEFAULT
#define DeeType_INVOKE_MOD                 DeeType_InvokeMathMod
#define DeeType_INVOKE_MOD_NODEFAULT       DeeType_InvokeMathMod_NODEFAULT
#define DeeType_INVOKE_SHL                 DeeType_InvokeMathShl
#define DeeType_INVOKE_SHL_NODEFAULT       DeeType_InvokeMathShl_NODEFAULT
#define DeeType_INVOKE_SHR                 DeeType_InvokeMathShr
#define DeeType_INVOKE_SHR_NODEFAULT       DeeType_InvokeMathShr_NODEFAULT
#define DeeType_INVOKE_AND                 DeeType_InvokeMathAnd
#define DeeType_INVOKE_AND_NODEFAULT       DeeType_InvokeMathAnd_NODEFAULT
#define DeeType_INVOKE_OR                  DeeType_InvokeMathOr
#define DeeType_INVOKE_OR_NODEFAULT        DeeType_InvokeMathOr_NODEFAULT
#define DeeType_INVOKE_XOR                 DeeType_InvokeMathXor
#define DeeType_INVOKE_XOR_NODEFAULT       DeeType_InvokeMathXor_NODEFAULT
#define DeeType_INVOKE_POW                 DeeType_InvokeMathPow
#define DeeType_INVOKE_POW_NODEFAULT       DeeType_InvokeMathPow_NODEFAULT
#define DeeType_INVOKE_INC                 DeeType_InvokeMathInc
#define DeeType_INVOKE_INC_NODEFAULT       DeeType_InvokeMathInc_NODEFAULT
#define DeeType_INVOKE_DEC                 DeeType_InvokeMathDec
#define DeeType_INVOKE_DEC_NODEFAULT       DeeType_InvokeMathDec_NODEFAULT
#define DeeType_INVOKE_IADD                DeeType_InvokeMathInplaceAdd
#define DeeType_INVOKE_IADD_NODEFAULT      DeeType_InvokeMathInplaceAdd_NODEFAULT
#define DeeType_INVOKE_ISUB                DeeType_InvokeMathInplaceSub
#define DeeType_INVOKE_ISUB_NODEFAULT      DeeType_InvokeMathInplaceSub_NODEFAULT
#define DeeType_INVOKE_IMUL                DeeType_InvokeMathInplaceMul
#define DeeType_INVOKE_IMUL_NODEFAULT      DeeType_InvokeMathInplaceMul_NODEFAULT
#define DeeType_INVOKE_IDIV                DeeType_InvokeMathInplaceDiv
#define DeeType_INVOKE_IDIV_NODEFAULT      DeeType_InvokeMathInplaceDiv_NODEFAULT
#define DeeType_INVOKE_IMOD                DeeType_InvokeMathInplaceMod
#define DeeType_INVOKE_IMOD_NODEFAULT      DeeType_InvokeMathInplaceMod_NODEFAULT
#define DeeType_INVOKE_ISHL                DeeType_InvokeMathInplaceShl
#define DeeType_INVOKE_ISHL_NODEFAULT      DeeType_InvokeMathInplaceShl_NODEFAULT
#define DeeType_INVOKE_ISHR                DeeType_InvokeMathInplaceShr
#define DeeType_INVOKE_ISHR_NODEFAULT      DeeType_InvokeMathInplaceShr_NODEFAULT
#define DeeType_INVOKE_IAND                DeeType_InvokeMathInplaceAnd
#define DeeType_INVOKE_IAND_NODEFAULT      DeeType_InvokeMathInplaceAnd_NODEFAULT
#define DeeType_INVOKE_IOR                 DeeType_InvokeMathInplaceOr
#define DeeType_INVOKE_IOR_NODEFAULT       DeeType_InvokeMathInplaceOr_NODEFAULT
#define DeeType_INVOKE_IXOR                DeeType_InvokeMathInplaceXor
#define DeeType_INVOKE_IXOR_NODEFAULT      DeeType_InvokeMathInplaceXor_NODEFAULT
#define DeeType_INVOKE_IPOW                DeeType_InvokeMathInplacePow
#define DeeType_INVOKE_IPOW_NODEFAULT      DeeType_InvokeMathInplacePow_NODEFAULT
#define DeeType_INVOKE_HASH                DeeType_InvokeCmpHash
#define DeeType_INVOKE_EQ                  DeeType_InvokeCmpEq
#define DeeType_INVOKE_EQ_NODEFAULT        DeeType_InvokeCmpEq_NODEFAULT
#define DeeType_INVOKE_NE                  DeeType_InvokeCmpNe
#define DeeType_INVOKE_NE_NODEFAULT        DeeType_InvokeCmpNe_NODEFAULT
#define DeeType_INVOKE_LO                  DeeType_InvokeCmpLo
#define DeeType_INVOKE_LO_NODEFAULT        DeeType_InvokeCmpLo_NODEFAULT
#define DeeType_INVOKE_LE                  DeeType_InvokeCmpLe
#define DeeType_INVOKE_LE_NODEFAULT        DeeType_InvokeCmpLe_NODEFAULT
#define DeeType_INVOKE_GR                  DeeType_InvokeCmpGr
#define DeeType_INVOKE_GR_NODEFAULT        DeeType_InvokeCmpGr_NODEFAULT
#define DeeType_INVOKE_GE                  DeeType_InvokeCmpGe
#define DeeType_INVOKE_GE_NODEFAULT        DeeType_InvokeCmpGe_NODEFAULT
#define DeeType_INVOKE_ITER                DeeType_InvokeSeqIterSelf
#define DeeType_INVOKE_FOREACH             DeeType_InvokeSeqForeach
#define DeeType_INVOKE_FOREACH_PAIR        DeeType_InvokeSeqForeachPair
#define DeeType_INVOKE_SIZE                DeeType_InvokeSeqSize
#define DeeType_INVOKE_CONTAINS            DeeType_InvokeSeqContains
#define DeeType_INVOKE_GETITEM             DeeType_InvokeSeqGet
#define DeeType_INVOKE_DELITEM             DeeType_InvokeSeqDel
#define DeeType_INVOKE_SETITEM             DeeType_InvokeSeqSet
#define DeeType_INVOKE_GETRANGE            DeeType_InvokeSeqRangeGet
#define DeeType_INVOKE_DELRANGE            DeeType_InvokeSeqRangeDel
#define DeeType_INVOKE_SETRANGE            DeeType_InvokeSeqRangeSet
#define DeeType_INVOKE_GETATTR             DeeType_InvokeAttrGetAttr
#define DeeType_INVOKE_DELATTR             DeeType_InvokeAttrDelAttr
#define DeeType_INVOKE_SETATTR             DeeType_InvokeAttrSetAttr
#define DeeType_INVOKE_ENTER               DeeType_InvokeWithEnter
#define DeeType_INVOKE_LEAVE               DeeType_InvokeWithLeave
#else /* DEFINE_TYPED_OPERATORS */
#define DeeType_INVOKE_ASSIGN(tp_self, self, other)                (*(tp_self)->tp_init.tp_assign)(self, other)
#define DeeType_INVOKE_MOVEASSIGN(tp_self, self, other)            (*(tp_self)->tp_init.tp_move_assign)(self, other)
#define DeeType_INVOKE_STR(tp_self, self)                          (*(tp_self)->tp_cast.tp_str)(self)
#define DeeType_INVOKE_REPR(tp_self, self)                         (*(tp_self)->tp_cast.tp_repr)(self)
#define DeeType_INVOKE_PRINT(tp_self, self, printer, arg)          (*(tp_self)->tp_cast.tp_print)(self, printer, arg)
#define DeeType_INVOKE_PRINTREPR(tp_self, self, printer, arg)      (*(tp_self)->tp_cast.tp_printrepr)(self, printer, arg)
#define DeeType_INVOKE_BOOL(tp_self, self)                         (*(tp_self)->tp_cast.tp_bool)(self)
#define DeeType_INVOKE_NEXT(tp_self, self)                         (*(tp_self)->tp_iter_next)(self)
#define DeeType_INVOKE_CALL(tp_self, self, argc,  argv)            (*(tp_self)->tp_call)(self, argc,  argv)
#define DeeType_INVOKE_CALLKW(tp_self, self, argc,  argv,  kw)     (*(tp_self)->tp_call_kw)(self, argc,  argv,  kw)
#define DeeType_INVOKE_INT32(tp_self, self, result)                (*(tp_self)->tp_math->tp_int32)(self, result)
#define DeeType_INVOKE_INT64(tp_self, self, result)                (*(tp_self)->tp_math->tp_int64)(self, result)
#define DeeType_INVOKE_DOUBLE(tp_self, self, result)               (*(tp_self)->tp_math->tp_double)(self, result)
#define DeeType_INVOKE_INT(tp_self, self)                          (*(tp_self)->tp_math->tp_int)(self)
#define DeeType_INVOKE_INV(tp_self, self)                          (*(tp_self)->tp_math->tp_inv)(self)
#define DeeType_INVOKE_POS(tp_self, self)                          (*(tp_self)->tp_math->tp_pos)(self)
#define DeeType_INVOKE_NEG(tp_self, self)                          (*(tp_self)->tp_math->tp_neg)(self)
#define DeeType_INVOKE_ADD(tp_self, self, other)                   (*(tp_self)->tp_math->tp_add)(self, other)
#define DeeType_INVOKE_SUB(tp_self, self, other)                   (*(tp_self)->tp_math->tp_sub)(self, other)
#define DeeType_INVOKE_MUL(tp_self, self, other)                   (*(tp_self)->tp_math->tp_mul)(self, other)
#define DeeType_INVOKE_DIV(tp_self, self, other)                   (*(tp_self)->tp_math->tp_div)(self, other)
#define DeeType_INVOKE_MOD(tp_self, self, other)                   (*(tp_self)->tp_math->tp_mod)(self, other)
#define DeeType_INVOKE_SHL(tp_self, self, other)                   (*(tp_self)->tp_math->tp_shl)(self, other)
#define DeeType_INVOKE_SHR(tp_self, self, other)                   (*(tp_self)->tp_math->tp_shr)(self, other)
#define DeeType_INVOKE_AND(tp_self, self, other)                   (*(tp_self)->tp_math->tp_and)(self, other)
#define DeeType_INVOKE_OR(tp_self, self, other)                    (*(tp_self)->tp_math->tp_or)(self, other)
#define DeeType_INVOKE_XOR(tp_self, self, other)                   (*(tp_self)->tp_math->tp_xor)(self, other)
#define DeeType_INVOKE_POW(tp_self, self, other)                   (*(tp_self)->tp_math->tp_pow)(self, other)
#define DeeType_INVOKE_INC(tp_self, p_self)                        (*(tp_self)->tp_math->tp_inc)(p_self)
#define DeeType_INVOKE_DEC(tp_self, p_self)                        (*(tp_self)->tp_math->tp_dec)(p_self)
#define DeeType_INVOKE_IADD(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_add)(p_self, other)
#define DeeType_INVOKE_ISUB(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_sub)(p_self, other)
#define DeeType_INVOKE_IMUL(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_mul)(p_self, other)
#define DeeType_INVOKE_IDIV(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_div)(p_self, other)
#define DeeType_INVOKE_IMOD(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_mod)(p_self, other)
#define DeeType_INVOKE_ISHL(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_shl)(p_self, other)
#define DeeType_INVOKE_ISHR(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_shr)(p_self, other)
#define DeeType_INVOKE_IAND(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_and)(p_self, other)
#define DeeType_INVOKE_IOR(tp_self, p_self, other)                 (*(tp_self)->tp_math->tp_inplace_or)(p_self, other)
#define DeeType_INVOKE_IXOR(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_xor)(p_self, other)
#define DeeType_INVOKE_IPOW(tp_self, p_self, other)                (*(tp_self)->tp_math->tp_inplace_pow)(p_self, other)
#define DeeType_INVOKE_HASH(tp_self, self)                         (*(tp_self)->tp_cmp->tp_hash)(self)
#define DeeType_INVOKE_EQ(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_eq)(self, other)
#define DeeType_INVOKE_NE(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_ne)(self, other)
#define DeeType_INVOKE_LO(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_lo)(self, other)
#define DeeType_INVOKE_LE(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_le)(self, other)
#define DeeType_INVOKE_GR(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_gr)(self, other)
#define DeeType_INVOKE_GE(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_ge)(self, other)
#define DeeType_INVOKE_ITER(tp_self, self)                         (*(tp_self)->tp_seq->tp_iter_self)(self)
#define DeeType_INVOKE_FOREACH(tp_self, self, proc, arg)           (*(tp_self)->tp_seq->tp_foreach)(self, proc, arg)
#define DeeType_INVOKE_FOREACH_PAIR(tp_self, self, proc, arg)      (*(tp_self)->tp_seq->tp_foreach_pair)(self, proc, arg)
#define DeeType_INVOKE_SIZE(tp_self, self)                         (*(tp_self)->tp_seq->tp_size)(self)
#define DeeType_INVOKE_CONTAINS(tp_self, self, other)              (*(tp_self)->tp_seq->tp_contains)(self, other)
#define DeeType_INVOKE_GETITEM(tp_self, self, index)               (*(tp_self)->tp_seq->tp_get)(self, index)
#define DeeType_INVOKE_DELITEM(tp_self, self, index)               (*(tp_self)->tp_seq->tp_del)(self, index)
#define DeeType_INVOKE_SETITEM(tp_self, self, index, value)        (*(tp_self)->tp_seq->tp_set)(self, index, value)
#define DeeType_INVOKE_GETRANGE(tp_self, self, start, end)         (*(tp_self)->tp_seq->tp_range_get)(self, start, end)
#define DeeType_INVOKE_DELRANGE(tp_self, self, start, end)         (*(tp_self)->tp_seq->tp_range_del)(self, start, end)
#define DeeType_INVOKE_SETRANGE(tp_self, self, start, end, values) (*(tp_self)->tp_seq->tp_range_set)(self, start, end, values)
#define DeeType_INVOKE_GETATTR(tp_self, self, name)                (*(tp_self)->tp_attr->tp_getattr)(self, name)
#define DeeType_INVOKE_DELATTR(tp_self, self, name)                (*(tp_self)->tp_attr->tp_delattr)(self, name)
#define DeeType_INVOKE_SETATTR(tp_self, self, name, value)         (*(tp_self)->tp_attr->tp_setattr)(self, name, value)
#define DeeType_INVOKE_ENTER(tp_self, self)                        (*(tp_self)->tp_with->tp_enter)(self)
#define DeeType_INVOKE_LEAVE(tp_self, self)                        (*(tp_self)->tp_with->tp_leave)(self)

#define DeeType_INVOKE_STR_NODEFAULT       DeeType_INVOKE_STR
#define DeeType_INVOKE_PRINT_NODEFAULT     DeeType_INVOKE_PRINT
#define DeeType_INVOKE_REPR_NODEFAULT      DeeType_INVOKE_REPR
#define DeeType_INVOKE_PRINTREPR_NODEFAULT DeeType_INVOKE_PRINTREPR
#define DeeType_INVOKE_CALL_NODEFAULT      DeeType_INVOKE_CALL
#define DeeType_INVOKE_CALLKW_NODEFAULT    DeeType_INVOKE_CALLKW
#define DeeType_INVOKE_INT32_NODEFAULT     DeeType_INVOKE_INT32
#define DeeType_INVOKE_INT64_NODEFAULT     DeeType_INVOKE_INT64
#define DeeType_INVOKE_DOUBLE_NODEFAULT    DeeType_INVOKE_DOUBLE
#define DeeType_INVOKE_INT_NODEFAULT       DeeType_INVOKE_INT
#define DeeType_INVOKE_ADD_NODEFAULT       DeeType_INVOKE_ADD
#define DeeType_INVOKE_SUB_NODEFAULT       DeeType_INVOKE_SUB
#define DeeType_INVOKE_MUL_NODEFAULT       DeeType_INVOKE_MUL
#define DeeType_INVOKE_DIV_NODEFAULT       DeeType_INVOKE_DIV
#define DeeType_INVOKE_MOD_NODEFAULT       DeeType_INVOKE_MOD
#define DeeType_INVOKE_SHL_NODEFAULT       DeeType_INVOKE_SHL
#define DeeType_INVOKE_SHR_NODEFAULT       DeeType_INVOKE_SHR
#define DeeType_INVOKE_AND_NODEFAULT       DeeType_INVOKE_AND
#define DeeType_INVOKE_OR_NODEFAULT        DeeType_INVOKE_OR
#define DeeType_INVOKE_XOR_NODEFAULT       DeeType_INVOKE_XOR
#define DeeType_INVOKE_POW_NODEFAULT       DeeType_INVOKE_POW
#define DeeType_INVOKE_INC_NODEFAULT       DeeType_INVOKE_INC
#define DeeType_INVOKE_DEC_NODEFAULT       DeeType_INVOKE_DEC
#define DeeType_INVOKE_IADD_NODEFAULT      DeeType_INVOKE_IADD
#define DeeType_INVOKE_ISUB_NODEFAULT      DeeType_INVOKE_ISUB
#define DeeType_INVOKE_IMUL_NODEFAULT      DeeType_INVOKE_IMUL
#define DeeType_INVOKE_IDIV_NODEFAULT      DeeType_INVOKE_IDIV
#define DeeType_INVOKE_IMOD_NODEFAULT      DeeType_INVOKE_IMOD
#define DeeType_INVOKE_ISHL_NODEFAULT      DeeType_INVOKE_ISHL
#define DeeType_INVOKE_ISHR_NODEFAULT      DeeType_INVOKE_ISHR
#define DeeType_INVOKE_IAND_NODEFAULT      DeeType_INVOKE_IAND
#define DeeType_INVOKE_IOR_NODEFAULT       DeeType_INVOKE_IOR
#define DeeType_INVOKE_IXOR_NODEFAULT      DeeType_INVOKE_IXOR
#define DeeType_INVOKE_IPOW_NODEFAULT      DeeType_INVOKE_IPOW
#define DeeType_INVOKE_EQ_NODEFAULT        DeeType_INVOKE_EQ
#define DeeType_INVOKE_NE_NODEFAULT        DeeType_INVOKE_NE
#define DeeType_INVOKE_LO_NODEFAULT        DeeType_INVOKE_LO
#define DeeType_INVOKE_LE_NODEFAULT        DeeType_INVOKE_LE
#define DeeType_INVOKE_GR_NODEFAULT        DeeType_INVOKE_GR
#define DeeType_INVOKE_GE_NODEFAULT        DeeType_INVOKE_GE
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
PRIVATE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
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
	dssize_t print_error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	LOAD_TP_SELF;
	ASSERT(tp_self->tp_cast.tp_print);
	ASSERT(tp_self->tp_cast.tp_print != &DeeObject_DefaultPrintWithStr);
	print_error = DeeType_INVOKE_PRINTREPR_NODEFAULT(tp_self, self, &unicode_printer_print, &printer);
	ASSERT(tp_self->tp_cast.tp_print &&
	       tp_self->tp_cast.tp_print != &DeeObject_DefaultPrintWithStr);
	print_error = DeeType_INVOKE_PRINT(tp_self, self, &unicode_printer_print, &printer);
	if unlikely(print_error < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultReprWithPrintRepr, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	dssize_t print_error;
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

DEFINE_INTERNAL_OPERATOR(dssize_t, DefaultPrintWithStr, (DeeObject *RESTRICT_IF_NOTYPE self,
                                                         dformatprinter printer, void *arg)) {
	dssize_t result;
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

DEFINE_INTERNAL_OPERATOR(dssize_t, DefaultPrintReprWithRepr, (DeeObject *RESTRICT_IF_NOTYPE self,
                                                              dformatprinter printer, void *arg)) {
	dssize_t result;
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
DeeType_InheritBool(DeeTypeObject *__restrict self) {
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		if (!base->tp_cast.tp_bool) {
			if (!DeeType_InheritBool(base))
				continue;
		}
		LOG_INHERIT(base, self, "operator bool");
		self->tp_cast.tp_bool = base->tp_cast.tp_bool;
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

DEFINE_OPERATOR(dssize_t, Print, (DeeObject *RESTRICT_IF_NOTYPE self,
                                  dformatprinter printer, void *arg)) {
	dssize_t result;
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

DEFINE_OPERATOR(dssize_t, PrintRepr, (DeeObject *RESTRICT_IF_NOTYPE self,
                                      dformatprinter printer, void *arg)) {
	dssize_t result;
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
	          DeeType_InheritHash(tp_self)) {
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
		int temp = DeeObject_Bool(ob);
		Dee_Decref(ob);
		if likely(temp >= 0) {
			ob = DeeBool_For(!temp);
			Dee_Incref(ob);
		} else {
			ob = NULL;
		}
	}
	return ob;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultEqWithNe,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_NE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultNeWithEq,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_NE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLoWithGe,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_GE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultLeWithGr,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_GR_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGrWithLe,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_LE_NODEFAULT(tp_self, self, other));
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultGeWithLo,
                         (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	return xinvoke_not(DeeType_INVOKE_LO_NODEFAULT(tp_self, self, other));
}


/* Default wrappers for implementing OPERATOR_ITER via `tp_iter_self <===> tp_foreach <===> tp_foreach_pair' */
DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIterSelfWithForeach, (DeeObject *__restrict self)) {
	LOAD_TP_SELF;
	/* TODO: Custom iterator type that uses "tp_foreach" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(DREF DeeObject *, DefaultIterSelfWithForeachPair,
                         (DeeObject *__restrict self)) {
	LOAD_TP_SELF;
	/* TODO: Custom iterator type that uses "tp_foreach_pair" */
	(void)tp_self;
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachWithIterSelf,
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

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachPairWithIterSelf,
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
	return DeeType_INVOKE_FOREACH_PAIR(tp_self, self, &default_foreach_with_foreach_pair_cb, &data);
}

struct default_foreach_pair_with_foreach_data {
	Dee_foreach_pair_t dfpwf_proc; /* [1..1] Underlying callback. */
	void              *dfpwf_arg;  /* Cookie for `dfpwf_proc' */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_foreach_cb(void *arg, DeeObject *elem);

#ifndef DEFINE_TYPED_OPERATORS
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
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
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_INTERNAL_OPERATOR(Dee_ssize_t, DefaultForeachPairWithForeach,
                         (DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg)) {
	struct default_foreach_pair_with_foreach_data data;
	LOAD_TP_SELF;
	data.dfpwf_proc = proc;
	data.dfpwf_arg  = arg;
	return DeeType_INVOKE_FOREACH(tp_self, self, &default_foreach_pair_with_foreach_cb, &data);
}






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
	if likely((tp_self->tp_math && tp_self->tp_math->tp_inv) ||
	          DeeType_InheritInv(tp_self))
		return DeeType_INVOKE_INV(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_INV);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Pos, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_pos) ||
	          DeeType_InheritPos(tp_self))
		return DeeType_INVOKE_POS(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_POS);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Neg, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_neg) ||
	          DeeType_InheritNeg(tp_self))
		return DeeType_INVOKE_NEG(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_NEG);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Add, (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_add) ||
	          (DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_add))
		return DeeType_INVOKE_ADD(tp_self, self, some_object);
	err_unimplemented_operator(tp_self, OPERATOR_ADD);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, Sub, (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_sub) ||
	          (DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_sub))
		return DeeType_INVOKE_SUB(tp_self, self, some_object);
	err_unimplemented_operator(tp_self, OPERATOR_SUB);
	return NULL;
}

#define DEFINE_MATH_OPERATOR2(name, xxx, operator_name, invoke, inherit) \
	DEFINE_OPERATOR(DREF DeeObject *, name,                              \
	                (DeeObject *self, DeeObject *some_object)) {         \
		LOAD_TP_SELF;                                                    \
		ASSERT_OBJECT(some_object);                                      \
		if likely((tp_self->tp_math && tp_self->tp_math->tp_##xxx) ||    \
		          inherit(tp_self))                                      \
			return invoke(tp_self, self, some_object);                   \
		err_unimplemented_operator(tp_self, operator_name);              \
		return NULL;                                                     \
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
	if likely((tp_self->tp_math && tp_self->tp_math->tp_inc) ||
	          (DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_inc))
		return DeeType_INVOKE_INC(tp_self, p_self);
	return err_unimplemented_operator(tp_self, OPERATOR_INC);
}

DEFINE_OPERATOR(int, Dec, (DREF DeeObject **__restrict p_self)) {
	LOAD_TP_SELFP;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_dec) ||
	          (DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_dec))
		return DeeType_INVOKE_DEC(tp_self, p_self);
	return err_unimplemented_operator(tp_self, OPERATOR_DEC);
}

DEFINE_OPERATOR(int, InplaceAdd, (DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object)) {
	LOAD_TP_SELFP;
	ASSERT_OBJECT(some_object);
	if likely((tp_self->tp_math && tp_self->tp_math->tp_inplace_add) ||
	          (DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_inplace_add))
		return DeeType_INVOKE_IADD(tp_self, p_self, some_object);
	return err_unimplemented_operator(tp_self, OPERATOR_INPLACE_ADD);
}

DEFINE_OPERATOR(int, InplaceSub, (DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object)) {
	LOAD_TP_SELFP;
	ASSERT_OBJECT(some_object);
	if likely((tp_self->tp_math && tp_self->tp_math->tp_inplace_sub) ||
	          (DeeType_InheritAdd(tp_self) && tp_self->tp_math->tp_inplace_sub))
		return DeeType_INVOKE_ISUB(tp_self, p_self, some_object);
	return err_unimplemented_operator(tp_self, OPERATOR_INPLACE_SUB);
}

#define DEFINE_MATH_INPLACE_OPERATOR2(name, xxx, operator_name, invoke_inplace, inherit) \
	DEFINE_OPERATOR(int, name, (DREF DeeObject **__restrict p_self,                      \
	                            DeeObject *some_object)) {                               \
		LOAD_TP_SELFP;                                                                   \
		ASSERT_OBJECT(some_object);                                                      \
		if likely((tp_self->tp_math && tp_self->tp_math->tp_inplace_##xxx) ||            \
		          inherit(tp_self))                                                      \
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


#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
DeeType_InheritHash(DeeTypeObject *__restrict self) {
	struct type_cmp *base_cmp;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_cmp = self->tp_cmp;
	if (base_cmp) {
		if (base_cmp->tp_hash)
			return true;
		if (base_cmp->tp_eq) {
			if (base_cmp->tp_ne == NULL)
				base_cmp->tp_ne = &DeeObject_DefaultNeWithEq;
			return false; /* Cannot inherit "operator hash" when "operator ==" is defined. */
		} else if (base_cmp->tp_ne) {
			if (base_cmp->tp_eq == NULL)
				base_cmp->tp_eq = &DeeObject_DefaultEqWithNe;
			return false; /* Cannot inherit "operator hash" when "operator !=" is defined. */
		}
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_cmp = base->tp_cmp;
		if (base_cmp == NULL) {
			if (!base_cmp->tp_hash) {
				if (!DeeType_InheritHash(base))
					continue;
			} else {
				if (base_cmp->tp_eq || base_cmp->tp_ne)
					return false; /* Cannot inherit hash in this case */
			}
			base_cmp = base->tp_cmp;
		}
		ASSERT(base_cmp->tp_hash);
		LOG_INHERIT(base, self, "operator hash");
		if (self->tp_cmp) {
			DeeTypeObject *origin = DeeType_GetCmpOrigin(self);
			if unlikely(origin)
				return DeeType_InheritHash(origin);
			if (self->tp_cmp->tp_hash == NULL)
				self->tp_cmp->tp_hash = base_cmp->tp_hash;
			if (self->tp_cmp->tp_eq == NULL && base_cmp->tp_eq)
				self->tp_cmp->tp_eq = base_cmp->tp_eq;
			if (self->tp_cmp->tp_ne == NULL && base_cmp->tp_ne)
				self->tp_cmp->tp_ne = base_cmp->tp_ne;
		} else {
			self->tp_cmp = base_cmp;
		}
		return true;
	}
	return false;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritCompare(DeeTypeObject *__restrict self) {
	struct type_cmp *base_cmp;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_cmp = self->tp_cmp;
	if (base_cmp) {
		bool ok = false;
		if (base_cmp->tp_eq) {
			if (base_cmp->tp_ne == NULL)
				base_cmp->tp_ne = &DeeObject_DefaultNeWithEq;
			ok = true;
		} else if (base_cmp->tp_ne) {
			if (base_cmp->tp_eq == NULL)
				base_cmp->tp_eq = &DeeObject_DefaultEqWithNe;
			ok = true;
		}
		if (base_cmp->tp_lo) {
			if (base_cmp->tp_ge == NULL)
				base_cmp->tp_ge = &DeeObject_DefaultGeWithLo;
			ok = true;
		} else if (base_cmp->tp_ge) {
			if (base_cmp->tp_lo == NULL)
				base_cmp->tp_lo = &DeeObject_DefaultLoWithGe;
			ok = true;
		}
		if (base_cmp->tp_le) {
			if (base_cmp->tp_gr == NULL)
				base_cmp->tp_gr = &DeeObject_DefaultGrWithLe;
			ok = true;
		} else if (base_cmp->tp_le) {
			if (base_cmp->tp_gr == NULL)
				base_cmp->tp_gr = &DeeObject_DefaultLeWithGr;
			ok = true;
		}
		if (ok)
			return true;
	}

	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_cmp = base->tp_cmp;
		if (base_cmp == NULL ||
		    (!base_cmp->tp_eq && !base_cmp->tp_ne &&
		     !base_cmp->tp_lo && !base_cmp->tp_le &&
		     !base_cmp->tp_gr && !base_cmp->tp_ge)) {
			if (!DeeType_InheritCompare(base))
				continue;
			base_cmp = base->tp_cmp;
		}
		LOG_INHERIT(base, self, "operator <compare>");
		if (self->tp_cmp) {
			DeeTypeObject *origin = DeeType_GetCmpOrigin(self);
			if unlikely(origin)
				return DeeType_InheritCompare(origin);
			if (self->tp_cmp->tp_eq == NULL)
				self->tp_cmp->tp_eq = base_cmp->tp_eq;
			if (self->tp_cmp->tp_ne == NULL)
				self->tp_cmp->tp_ne = base_cmp->tp_ne;
			if (self->tp_cmp->tp_lo == NULL)
				self->tp_cmp->tp_lo = base_cmp->tp_lo;
			if (self->tp_cmp->tp_le == NULL)
				self->tp_cmp->tp_le = base_cmp->tp_le;
			if (self->tp_cmp->tp_gr == NULL)
				self->tp_cmp->tp_gr = base_cmp->tp_gr;
			if (self->tp_cmp->tp_ge == NULL)
				self->tp_cmp->tp_ge = base_cmp->tp_ge;
		} else {
			self->tp_cmp = base_cmp;
		}
		return true;
	}
	return false;
}
#endif /* !DEFINE_TYPED_OPERATORS */

#define DEFINE_OBJECT_COMPARE_OPERATOR(name, cmp, operator_name, invoke)          \
	DEFINE_OPERATOR(DREF DeeObject *, name,                                       \
	                (DeeObject *self, DeeObject *some_object)) {                  \
		LOAD_TP_SELF;                                                             \
		ASSERT_OBJECT(some_object);                                               \
		if likely((tp_self->tp_cmp && tp_self->tp_cmp->tp_##cmp) ||               \
		          (DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_##cmp)) \
			return invoke(tp_self, self, some_object);                            \
		err_unimplemented_operator(tp_self, operator_name);                       \
		return NULL;                                                              \
	}
DEFINE_OBJECT_COMPARE_OPERATOR(CompareEqObject, eq, OPERATOR_EQ, DeeType_INVOKE_EQ)
DEFINE_OBJECT_COMPARE_OPERATOR(CompareNeObject, ne, OPERATOR_NE, DeeType_INVOKE_NE)
DEFINE_OBJECT_COMPARE_OPERATOR(CompareLoObject, lo, OPERATOR_LO, DeeType_INVOKE_LO)
DEFINE_OBJECT_COMPARE_OPERATOR(CompareLeObject, le, OPERATOR_LE, DeeType_INVOKE_LE)
DEFINE_OBJECT_COMPARE_OPERATOR(CompareGrObject, gr, OPERATOR_GR, DeeType_INVOKE_GR)
DEFINE_OBJECT_COMPARE_OPERATOR(CompareGeObject, ge, OPERATOR_GE, DeeType_INVOKE_GE)
#undef DEFINE_OBJECT_COMPARE_OPERATOR


#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_COMPARE_OPERATOR(name, fwd, bck, error) \
	PUBLIC WUNUSED NONNULL((1, 2)) int DCALL           \
	name(DeeObject *self, DeeObject *some_object) {    \
		DeeObject *val;                                \
		int result;                                    \
		val = name##Object(self, some_object);         \
		if unlikely(!val)                              \
			goto err;                                  \
		result = DeeObject_Bool(val);                  \
		Dee_Decref(val);                               \
		return result;                                 \
	err:                                               \
		return -1;                                     \
	}
DEFINE_COMPARE_OPERATOR(DeeObject_CompareLo, lo, ge, OPERATOR_LO)
DEFINE_COMPARE_OPERATOR(DeeObject_CompareLe, le, gr, OPERATOR_LE)
DEFINE_COMPARE_OPERATOR(DeeObject_CompareGr, gr, lo, OPERATOR_GR)
DEFINE_COMPARE_OPERATOR(DeeObject_CompareGe, ge, le, OPERATOR_GE)
#undef DEFINE_COMPARE_OPERATOR

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_CompareEq(DeeObject *self, DeeObject *some_object) {
	DeeObject *val;
	int result;
	val = DeeObject_CompareEqObject(self, some_object);
	if unlikely(!val) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			return self == some_object;
		return -1;
	}
	result = DeeObject_Bool(val);
	Dee_Decref(val);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_CompareNe(DeeObject *self, DeeObject *some_object) {
	DeeObject *val;
	int result;
	val = DeeObject_CompareNeObject(self, some_object);
	if unlikely(!val) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			return self != some_object;
		return -1;
	}
	result = DeeObject_Bool(val);
	Dee_Decref(val);
	return result;
}

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL /* From "../objects/seq.c" */
seq_lo(DeeObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* From "../objects/seq.c" */
DeeSeq_Compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL /* From "../objects/tuple.c" */
tuple_lo(DeeTupleObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL /* From "../objects/list.c" */
list_lo(DeeListObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* From "../objects/list.c" */
DeeList_CompareS(DeeListObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* From "../objects/string.c" */
string_compare(DeeStringObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL /* From "../objects/string.c" */
string_lo(DeeStringObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* From "../objects/bytes.c" */
bytes_compare(DeeBytesObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL /* From "../objects/bytes.c" */
bytes_lo(DeeBytesObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
super_lo(DeeSuperObject *self, DeeObject *some_object);


/* @return: == -2: An error occurred.
 * @return: == -1: `lhs < rhs'
 * @return: == 0:  `lhs == rhs'
 * @return: == 1:  `lhs > rhs' */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_Compare(DeeObject *lhs, DeeObject *rhs) {
#if 1
	DeeTypeObject *tp_lhs;
	ASSERT_OBJECT(lhs);
	ASSERT_OBJECT(rhs);
	tp_lhs = Dee_TYPE(lhs);
again:
	if (tp_lhs->tp_cmp || DeeType_InheritCompare(tp_lhs)) {
		int opval;
		DREF DeeObject *opres;
		DREF DeeObject *(DCALL *tp_lo)(DeeObject *self, DeeObject *some_object);
		tp_lo = tp_lhs->tp_cmp->tp_lo;

		/* Check for special operators for which we provide optimizations. */
		if (tp_lo == (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tuple_lo)
			return DeeSeq_CompareVS(DeeTuple_ELEM(lhs), DeeTuple_SIZE(lhs), rhs);
		if (tp_lo == (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&list_lo)
			return DeeList_CompareS((DeeListObject *)lhs, rhs);
		if (tp_lo == (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seq_lo)
			return DeeSeq_Compare(lhs, rhs);
		if (tp_lo == (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&string_lo)
			return string_compare((DeeStringObject *)lhs, rhs);
		if (tp_lo == (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_lo)
			return bytes_compare((DeeBytesObject *)lhs, rhs);
		if (tp_lo == (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&super_lo) {
			/* Unwind super wrappers */
			tp_lhs = DeeSuper_TYPE(lhs);
			lhs    = DeeSuper_SELF(lhs);
			goto again;
		}

		/* Fallback: do 1/2 operator invocations, based on those operators that are available. */
		if (tp_lo) {
			opres = (*tp_lo)(lhs, rhs);
		} else if (tp_lhs->tp_cmp->tp_ge) {
			opres = xinvoke_not((*tp_lhs->tp_cmp->tp_ge)(lhs, rhs));
		} else {
			goto no_lo;
		}
		if unlikely(!opres)
			goto err;
		opval = DeeObject_Bool(opres);
		Dee_Decref(opres);
		if (opval != 0)
			return unlikely(opval < 0) ? -2 : -1;

		/* At this point we know that: "!(lhs < rhs)"
		 * -> Now check if "lhs == rhs" */
		if (tp_lhs->tp_cmp->tp_eq) {
			opres = (*tp_lhs->tp_cmp->tp_eq)(lhs, rhs);
		} else if (tp_lhs->tp_cmp->tp_ne) {
			opres = xinvoke_not((*tp_lhs->tp_cmp->tp_ne)(lhs, rhs));
		} else {
			goto no_lo;
		}
		if unlikely(!opres)
			goto err;
		opval = DeeObject_Bool(opres);
		Dee_Decref(opres);
		if likely(opval >= 0)
			opval = !opval;
		return opval;
	}
no_lo:
	err_unimplemented_operator(tp_lhs, OPERATOR_LO);
err:
	return -2;
#else
	/* Fallback: use the normal compare operators.
	 * For this purpose, we always use "operator <" and "operator ==" */
	result = DeeObject_CompareLo(self, some_object);
	if (result != 0)
		return unlikely(result < 0) ? -2 : -1;
	result = DeeObject_CompareEq(self, some_object);
	if likely(result >= 0)
		result = !result;
	return result;
#endif
}

#endif /* !DEFINE_TYPED_OPERATORS */

#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field)                \
	INTERN NONNULL((1)) bool DCALL                                       \
	name(DeeTypeObject *__restrict self) {                               \
		struct type_seq *base_seq;                                       \
		DeeTypeMRO mro;                                                  \
		DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);               \
		while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) { \
			base_seq = base->tp_seq;                                     \
			if (base_seq == NULL || !base_seq->field) {                  \
				if (!name(base))                                         \
					continue;                                            \
			}                                                            \
			base_seq = base->tp_seq;                                     \
			LOG_INHERIT(base, self, opname);                             \
			if (self->tp_seq) {                                          \
				DeeTypeObject *origin = DeeType_GetSeqOrigin(self);      \
				if unlikely(origin)                                      \
					return name(origin);                                 \
				self->tp_seq->field = base_seq->field;                   \
			} else {                                                     \
				self->tp_seq = base_seq;                                 \
			}                                                            \
			return true;                                                 \
		}                                                                \
		return false;                                                    \
	}

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

INTERN NONNULL((1)) bool DCALL
DeeType_InheritIterSelf(DeeTypeObject *__restrict self) {
	struct type_seq *base_seq;
	DeeTypeMRO mro;
	DeeTypeObject *base;
	base_seq = self->tp_seq;
	if (base_seq) {
		if (base_seq->tp_iter_self) {
			if (base_seq->tp_foreach == NULL)
				base_seq->tp_foreach = &DeeObject_DefaultForeachWithIterSelf;
			if (base_seq->tp_foreach_pair == NULL)
				base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithIterSelf;
			return true;
		} else if (base_seq->tp_foreach) {
			base_seq->tp_iter_self = &DeeObject_DefaultIterSelfWithForeach;
			if (base_seq->tp_foreach_pair == NULL)
				base_seq->tp_foreach_pair = &DeeObject_DefaultForeachPairWithForeach;
			return true;
		} else if (base_seq->tp_foreach_pair) {
			base_seq->tp_iter_self = &DeeObject_DefaultIterSelfWithForeachPair;
			base_seq->tp_foreach   = &DeeObject_DefaultForeachWithForeachPair;
			return true;
		}
	}
	base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_seq = base->tp_seq;
		if (base_seq == NULL || (!base_seq->tp_iter_self ||
		                         !base_seq->tp_foreach ||
		                         !base_seq->tp_foreach_pair)) {
			if (!DeeType_InheritIterSelf(base))
				continue;
		}
		base_seq = base->tp_seq;
		LOG_INHERIT(base, self, "operator iterself");
		if (self->tp_seq) {
			DeeTypeObject *origin = DeeType_GetSeqOrigin(self);
			if unlikely(origin)
				return DeeType_InheritIterSelf(origin);
			self->tp_seq->tp_iter_self    = base_seq->tp_iter_self;
			self->tp_seq->tp_foreach      = base_seq->tp_foreach;
			self->tp_seq->tp_foreach_pair = base_seq->tp_foreach_pair;
		} else {
			self->tp_seq = base_seq;
		}
		return true;
	}
	return false;
}

DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritSize, "operator size", tp_size)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritContains, "operator contains", tp_contains)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritGetItem, "operator getitem", tp_get)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritDelItem, "operator delitem", tp_del)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritSetItem, "operator setitem", tp_set)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritGetRange, "operator getrange", tp_range_get)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritDelRange, "operator delrange", tp_range_del)
DEFINE_TYPE_INHERIT_FUNCTION(DeeType_InheritSetRange, "operator setrange", tp_range_set)
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, IterSelf, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_iter_self) ||
	          DeeType_InheritIterSelf(tp_self))
		return DeeType_INVOKE_ITER(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_ITERSELF);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, IterNext, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely(tp_self->tp_iter_next || DeeType_InheritIterNext(tp_self))
		return DeeType_INVOKE_NEXT(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_ITERNEXT);
	return NULL;
}



#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(size_t, Size, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *sizeob;
	size_t result;
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_size) ||
	          DeeType_InheritSize(tp_self)) {
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi) {
			ASSERT(nsi->nsi_common.nsi_getsize);
			return (*nsi->nsi_common.nsi_getsize)(self);
		}
		sizeob = DeeType_INVOKE_SIZE(tp_self, self);
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
}

DEFINE_OPERATOR(int, Contains,
                (DeeObject *self, DeeObject *some_object)) {
	DREF DeeObject *resultob;
	int result;
	resultob = DeeObject_ContainsObject(self, some_object);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_Bool(resultob);
	Dee_Decref(resultob);
	return result;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, SizeObject, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_size) ||
	          DeeType_InheritSize(tp_self))
		return DeeType_INVOKE_SIZE(tp_self, self);
	err_unimplemented_operator(tp_self, OPERATOR_SIZE);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, ContainsObject,
                (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_contains) ||
	          DeeType_InheritContains(tp_self))
		return DeeType_INVOKE_CONTAINS(tp_self, self, some_object);
	err_unimplemented_operator(tp_self, OPERATOR_CONTAINS);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItem,
                (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_get) ||
	          DeeType_InheritGetItem(tp_self))
		return DeeType_INVOKE_GETITEM(tp_self, self, index);
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(int, DelItem,
                (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_del) ||
	          DeeType_InheritDelItem(tp_self))
		return DeeType_INVOKE_DELITEM(tp_self, self, index);
	return err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
}

DEFINE_OPERATOR(int, SetItem,
                (DeeObject *self, DeeObject *index, DeeObject *value)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_set) ||
	          DeeType_InheritSetItem(tp_self))
		return DeeType_INVOKE_SETITEM(tp_self, self, index, value);
	return err_unimplemented_operator(tp_self, OPERATOR_SETITEM);
}

DEFINE_OPERATOR(DREF DeeObject *, GetRange,
                (DeeObject *self, DeeObject *begin, DeeObject *end)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_get) ||
	          DeeType_InheritGetRange(tp_self))
		return DeeType_INVOKE_GETRANGE(tp_self, self, begin, end);
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


PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *
(DCALL DeeObject_GetRangeBeginIndex)(DeeObject *self, dssize_t begin, DeeObject *end) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_get) ||
	          DeeType_InheritGetRange(tp_self)) {
		dssize_t end_index;
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
	err_unimplemented_operator(GET_TP_SELF(), OPERATOR_GETRANGE);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_GetRangeEndIndex)(DeeObject *self, DeeObject *begin, dssize_t end) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_get) ||
	          DeeType_InheritGetRange(tp_self)) {
		dssize_t begin_index;
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
	err_unimplemented_operator(GET_TP_SELF(), OPERATOR_GETRANGE);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_GetRangeIndex)(DeeObject *__restrict self,
                                dssize_t begin, dssize_t end) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_get) ||
	          DeeType_InheritGetRange(tp_self)) {
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

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_DelRangeBeginIndex)(DeeObject *self,
                                     dssize_t begin, DeeObject *end) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_del) ||
	          DeeType_InheritSetRange(tp_self)) {
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
				dssize_t end_index;
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

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelRangeEndIndex)(DeeObject *self,
                                   DeeObject *begin, dssize_t end) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_del) ||
	          DeeType_InheritSetRange(tp_self)) {
		int result;
		DREF DeeObject *end_ob;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_delrange) {
				dssize_t start_index;
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

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_DelRangeIndex)(DeeObject *self,
                                dssize_t begin, dssize_t end) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_del) ||
	          DeeType_InheritSetRange(tp_self)) {
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

PUBLIC WUNUSED NONNULL((1, 3, 4)) int
(DCALL DeeObject_SetRangeBeginIndex)(DeeObject *self,
                                     dssize_t begin, DeeObject *end,
                                     DeeObject *values) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	ASSERT_OBJECT(values);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_set) ||
	          DeeType_InheritSetRange(tp_self)) {
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
				dssize_t end_index;
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

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_SetRangeEndIndex)(DeeObject *self,
                                   DeeObject *begin, dssize_t end,
                                   DeeObject *values) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(values);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_set) ||
	          DeeType_InheritSetRange(tp_self)) {
		int result;
		DREF DeeObject *end_ob;
		struct type_nsi const *nsi;
		/* NSI optimizations. */
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
			if (nsi->nsi_seqlike.nsi_setrange) {
				dssize_t start_index;
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

PUBLIC WUNUSED NONNULL((1, 4)) int
(DCALL DeeObject_SetRangeIndex)(DeeObject *self,
                                dssize_t begin, dssize_t end,
                                DeeObject *values) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(values);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_set) ||
	          DeeType_InheritSetRange(tp_self)) {
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
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(int, DelRange,
                (DeeObject *self, DeeObject *begin, DeeObject *end)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(end);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_del) ||
	          DeeType_InheritDelRange(tp_self))
		return DeeType_INVOKE_DELRANGE(tp_self, self, begin, end);
	return err_unimplemented_operator(tp_self, OPERATOR_DELRANGE);
}

DEFINE_OPERATOR(int, SetRange,
                (DeeObject *self, DeeObject *begin,
                 DeeObject *end, DeeObject *value)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(end);
	ASSERT_OBJECT(value);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_range_set) ||
	          DeeType_InheritSetRange(tp_self))
		return DeeType_INVOKE_SETRANGE(tp_self, self, begin, end, value);
	return err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
}



#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItem)(DeeObject *self,
                            DeeObject *index,
                            bool allow_missing) {
	DREF DeeObject *result;
	DeeTypeObject *tp_self;
	size_t i;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_get) ||
	          DeeType_InheritGetItem(tp_self)) {
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
						if (allow_missing)
							return -2;
						return err_index_out_of_bounds(self, i, size);
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
					if (result == ITER_DONE) {
						if (allow_missing)
							return -2;
						return err_unknown_key(self, index);
					}
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
			if (allow_missing &&
			    (DeeError_Catch(&DeeError_IndexError) ||
			     DeeError_Catch(&DeeError_KeyError)))
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

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_BoundItemIndex)(DeeObject *__restrict self,
                                 size_t index, bool allow_missing) {
	DREF DeeObject *result, *index_ob;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_get) ||
	          DeeType_InheritGetItem(tp_self)) {
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
					if unlikely(index >= size) {
						if (allow_missing)
							return -2; /* Bad index */
						return err_index_out_of_bounds(self, index, size);
					}
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
						if (allow_missing) {
							Dee_Decref(index_ob);
							return -2;
						}
						err_unknown_key(self, index_ob);
						Dee_Decref(index_ob);
						return -1;
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
			if (allow_missing &&
			    (DeeError_Catch(&DeeError_IndexError) ||
			     DeeError_Catch(&DeeError_KeyError)))
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

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItemStringHash)(DeeObject *__restrict self,
                                      char const *__restrict key,
                                      dhash_t hash,
                                      bool allow_missing) {
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *key_ob;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
again:
	if (tp_self == &DeeDict_Type) {
		result = DeeDict_HasItemStringHash(self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeRoDict_Type) {
		result = DeeRoDict_HasItemStringHash((DeeRoDictObject *)self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		result = DeeKwdsMapping_HasItemStringHash((DeeKwdsMappingObject *)self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		result = DeeBlackListKwds_HasItemStringHash((DeeBlackListKwdsObject *)self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKw_Type) {
		return DeeBlackListKw_BoundItemStringHash((DeeBlackListKwObject *)self, key, hash, allow_missing);
	} else if (tp_self == &DeeCachedDict_Type) {
		return DeeCachedDict_BoundItemStringHash((DeeCachedDictObject *)self, key, hash, allow_missing);
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_BoundItem(orig_self, key_ob, allow_missing);
	Dee_Decref(key_ob);
	return result;
handle_bool:
	if (!result) {
		if (!allow_missing) {
			err_unknown_key_str(orig_self, key);
			goto err;
		}
		result = -2;
	}
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItemStringLenHash)(DeeObject *__restrict self,
                                         char const *__restrict key,
                                         size_t keylen, dhash_t hash,
                                         bool allow_missing) {
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *key_ob;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
again:
	if (tp_self == &DeeDict_Type) {
		result = DeeDict_HasItemStringLenHash(self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeRoDict_Type) {
		result = DeeRoDict_HasItemStringLenHash((DeeRoDictObject *)self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		result = DeeKwdsMapping_HasItemStringLenHash((DeeKwdsMappingObject *)self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		result = DeeBlackListKwds_HasItemStringLenHash((DeeBlackListKwdsObject *)self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKw_Type) {
		return DeeBlackListKw_BoundItemStringLenHash((DeeBlackListKwObject *)self, key, keylen, hash, allow_missing);
	} else if (tp_self == &DeeCachedDict_Type) {
		return DeeCachedDict_BoundItemStringLenHash((DeeCachedDictObject *)self, key, keylen, hash, allow_missing);
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_BoundItem(orig_self, key_ob, allow_missing);
	Dee_Decref(key_ob);
	return result;
handle_bool:
	if (!result) {
		if (!allow_missing) {
			err_unknown_key_str_len(orig_self, key, keylen);
			goto err;
		}
		result = -2;
	}
	return result;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItem)(DeeObject *self, DeeObject *index) {
	DREF DeeObject *result;
	DeeTypeObject *tp_self;
	size_t i;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_get) ||
	          DeeType_InheritGetItem(tp_self)) {
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

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_HasItemIndex)(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result, *index_ob;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_get) ||
	          DeeType_InheritGetItem(tp_self)) {
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

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItemStringHash)(DeeObject *__restrict self,
                                    char const *__restrict key,
                                    dhash_t hash) {
	bool b_result;
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *key_ob;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
again:
	if (tp_self == &DeeDict_Type) {
		b_result = DeeDict_HasItemStringHash(self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeRoDict_Type) {
		b_result = DeeRoDict_HasItemStringHash((DeeRoDictObject *)self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		b_result = DeeKwdsMapping_HasItemStringHash((DeeKwdsMappingObject *)self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		b_result = DeeBlackListKwds_HasItemStringHash((DeeBlackListKwdsObject *)self, key, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKw_Type) {
		return DeeBlackListKw_HasItemStringHash((DeeBlackListKwObject *)self, key, hash);
	} else if (tp_self == &DeeCachedDict_Type) {
		return DeeCachedDict_HasItemStringHash((DeeCachedDictObject *)self, key, hash);
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_HasItem(orig_self, key_ob);
	Dee_Decref(key_ob);
	return result;
handle_bool:
	return b_result ? 1 : 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItemStringLenHash)(DeeObject *__restrict self,
                                       char const *__restrict key,
                                       size_t keylen, dhash_t hash) {
	bool b_result;
	int result;
	DeeObject *orig_self = self;
	DREF DeeObject *key_ob;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
again:
	if (tp_self == &DeeDict_Type) {
		b_result = DeeDict_HasItemStringLenHash(self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeRoDict_Type) {
		b_result = DeeRoDict_HasItemStringLenHash((DeeRoDictObject *)self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		b_result = DeeKwdsMapping_HasItemStringLenHash((DeeKwdsMappingObject *)self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		b_result = DeeBlackListKwds_HasItemStringLenHash((DeeBlackListKwdsObject *)self, key, keylen, hash);
		goto handle_bool;
	} else if (tp_self == &DeeBlackListKw_Type) {
		return DeeBlackListKw_HasItemStringLenHash((DeeBlackListKwObject *)self, key, keylen, hash);
	} else if (tp_self == &DeeCachedDict_Type) {
		return DeeCachedDict_HasItemStringLenHash((DeeCachedDictObject *)self, key, keylen, hash);
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_HasItem(orig_self, key_ob);
	Dee_Decref(key_ob);
	return result;
handle_bool:
	return b_result ? 1 : 0;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObject_GetItemDef(DeeObject *self, DeeObject *key, DeeObject *def) {
	DREF DeeObject *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_get) ||
	          DeeType_InheritGetItem(tp_self)) {
		struct type_nsi const *nsi;
		nsi = tp_self->tp_seq->tp_nsi;
		if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP) {
			if (nsi->nsi_maplike.nsi_getdefault)
				return (*nsi->nsi_maplike.nsi_getdefault)(self, key, def);
		}
		result = DeeType_INVOKE_GETITEM(tp_self, self, key);
		if unlikely(!result) {
			if (DeeError_Catch(&DeeError_KeyError) ||
			    DeeError_Catch(&DeeError_NotImplemented)) {
				if (def != ITER_DONE)
					Dee_Incref(def);
				return def;
			}
		}
		return result;
	}
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_GetItemIndex(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *index_ob, *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_get) ||
	          DeeType_InheritGetItem(tp_self)) {
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

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_DelItemIndex)(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *index_ob;
	int result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_del) ||
	          DeeType_InheritDelItem(tp_self)) {
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

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_SetItemIndex)(DeeObject *self, size_t index,
                               DeeObject *value) {
	DREF DeeObject *index_ob;
	int result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_set) ||
	          DeeType_InheritSetItem(tp_self)) {
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

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_GetItemStringHash(DeeObject *__restrict self,
                            char const *__restrict key,
                            dhash_t hash) {
	DREF DeeObject *key_ob, *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
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
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_TGetItem(tp_self, self, key_ob);
	Dee_Decref(key_ob);
	return result;
xincref_result_and_return:
	Dee_XIncref(result);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_GetItemStringLenHash(DeeObject *__restrict self,
                               char const *__restrict key,
                               size_t keylen, dhash_t hash) {
	DREF DeeObject *key_ob, *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
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
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_TGetItem(tp_self, self, key_ob);
	Dee_Decref(key_ob);
	return result;
xincref_result_and_return:
	Dee_XIncref(result);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeObject_GetItemStringHashDef(DeeObject *self,
                               char const *__restrict key,
                               dhash_t hash, DeeObject *def) {
	DeeObject *orig_self = self;
	DREF DeeObject *key_ob, *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
again:
	if (tp_self == &DeeDict_Type) {
		return DeeDict_GetItemStringHashDef(self, key, hash, def);
	} else if (tp_self == &DeeRoDict_Type) {
		result = DeeRoDict_GetItemNRStringHashDef((DeeRoDictObject *)self, key, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		result = DeeKwdsMapping_GetItemNRStringHashDef((DeeKwdsMappingObject *)self, key, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		result = DeeBlackListKwds_GetItemNRStringHashDef((DeeBlackListKwdsObject *)self, key, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKw_Type) {
		result = DeeBlackListKw_GetItemNRStringHashDef((DeeBlackListKwObject *)self, key, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeCachedDict_Type) {
		result = DeeCachedDict_GetItemNRStringHashDef((DeeCachedDictObject *)self, key, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_GetItemDef(orig_self, key_ob, def);
	Dee_Decref(key_ob);
	return result;
xincref_result_and_return:
	Dee_XIncref(result);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeObject_GetItemStringLenHashDef(DeeObject *self,
                                  char const *__restrict key,
                                  size_t keylen, dhash_t hash,
                                  DeeObject *def) {
	DeeObject *orig_self = self;
	DREF DeeObject *key_ob, *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);

	/* Optimization for specific types. */
	tp_self = Dee_TYPE(self);
again:
	if (tp_self == &DeeDict_Type) {
		return DeeDict_GetItemStringLenHashDef(self, key, keylen, hash, def);
	} else if (tp_self == &DeeRoDict_Type) {
		result = DeeRoDict_GetItemNRStringLenHashDef((DeeRoDictObject *)self, key, keylen, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeKwdsMapping_Type) {
		result = DeeKwdsMapping_GetItemNRStringLenHashDef((DeeKwdsMappingObject *)self, key, keylen, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKwds_Type) {
		result = DeeBlackListKwds_GetItemNRStringLenHashDef((DeeBlackListKwdsObject *)self, key, keylen, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeBlackListKw_Type) {
		result = DeeBlackListKw_GetItemNRStringLenHashDef((DeeBlackListKwObject *)self, key, keylen, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeCachedDict_Type) {
		result = DeeCachedDict_GetItemNRStringLenHashDef((DeeCachedDictObject *)self, key, keylen, hash, def);
		goto xincref_result_and_return;
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	}

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_GetItemDef(orig_self, key_ob, def);
	Dee_Decref(key_ob);
	return result;
xincref_result_and_return:
	Dee_XIncref(result);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelItemStringHash)(DeeObject *__restrict self,
                                    char const *__restrict key,
                                    dhash_t hash) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_DelItemStringHash(self, key, hash);

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_DelItem(self, key_ob);
	Dee_Decref(key_ob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelItemStringLenHash)(DeeObject *__restrict self,
                                       char const *__restrict key,
                                       size_t keylen, dhash_t hash) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_DelItemStringLenHash(self, key, keylen, hash);

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_DelItem(self, key_ob);
	Dee_Decref(key_ob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_SetItemStringHash)(DeeObject *self,
                                    char const *__restrict key,
                                    dhash_t hash, DeeObject *value) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(value);
	if (DeeDict_CheckExact(self))
		return DeeDict_SetItemStringHash(self, key, hash, value);

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_SetItem(self, key_ob, value);
	Dee_Decref(key_ob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) int
(DCALL DeeObject_SetItemStringLenHash)(DeeObject *self,
                                       char const *__restrict key,
                                       size_t keylen, dhash_t hash,
                                       DeeObject *value) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(value);
	if (DeeDict_CheckExact(self))
		return DeeDict_SetItemStringLenHash(self, key, keylen, hash, value);

	/* Fallback: create a temporary string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_SetItem(self, key_ob, value);
	Dee_Decref(key_ob);
	return result;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */



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
	if likely((tp_self->tp_with && tp_self->tp_with->tp_enter) || DeeType_InheritWith(tp_self))
		return DeeType_INVOKE_ENTER(tp_self, self);
	return err_unimplemented_operator(tp_self, OPERATOR_ENTER);
}

DEFINE_OPERATOR(int, Leave,
               (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_with && tp_self->tp_with->tp_leave) || DeeType_InheritWith(tp_self))
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
	if likely((tp_self->tp_buffer && tp_self->tp_buffer->tp_getbuf) ||
	          DeeType_InheritBuffer(tp_self)) {
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


#undef GET_TP_SELF
#undef GET_TP_PSELF
#undef LOAD_ITER
#undef LOAD_ITERP
#undef LOAD_TP_SELF
#undef DEFINE_INTERNAL_OPERATOR
#undef DEFINE_OPERATOR


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED ATTR_OUTS(3, 2) NONNULL((1)) int
(DCALL DeeObject_Unpack)(DeeObject *__restrict self, size_t objc,
                         /*out*/ DREF DeeObject **__restrict objv) {
	DREF DeeObject *iterator, *elem;
	size_t fast_size, i;

	/* Try to make use of the fast-sequence API. */
	fast_size = DeeFastSeq_GetSize(self);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
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
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err_objv;
			objv[i] = elem; /* Inherit reference. */
		}
		return 0;
	}

	/* Fallback: Use an iterator. */
	if ((iterator = DeeObject_IterSelf(self)) == NULL)
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


PUBLIC WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeObject_Foreach(DeeObject *__restrict self,
                  Dee_foreach_t proc, void *arg) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_foreach) ||
	          DeeType_InheritIterSelf(tp_self))
		return (*tp_self->tp_seq->tp_foreach)(self, proc, arg);
	return err_unimplemented_operator(tp_self, OPERATOR_ITERSELF);
}

PUBLIC WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeObject_ForeachPair(DeeObject *__restrict self,
                      Dee_foreach_pair_t proc, void *arg) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if likely((tp_self->tp_seq && tp_self->tp_seq->tp_foreach_pair) ||
	          DeeType_InheritIterSelf(tp_self))
		return (*tp_self->tp_seq->tp_foreach_pair)(self, proc, arg);
	return err_unimplemented_operator(tp_self, OPERATOR_ITERSELF);
}

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given (optional) `key' function
 * @return: == -2: An error occurred.
 * @return: == -1: `lhs_keyed < key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed > key(rhs)' */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_CompareKey)(DeeObject *lhs_keyed,
                             DeeObject *rhs, /*nullable*/ DeeObject *key) {
	int result;
	if (!key)
		return DeeObject_Compare(lhs_keyed, rhs);
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err;
	result = DeeObject_Compare(lhs_keyed, rhs);
	Dee_Decref(rhs);
	return result;
err:
	return -2;
}


/* Compare a pre-keyed `keyed_search_item' with `elem' using the given (optional) `key' function
 * @return:  > 0: `keyed_search_item == key(elem)'
 * @return: == 0: `keyed_search_item != key(elem)'
 * @return:  < 0: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_CompareKeyEq)(DeeObject *keyed_search_item,
                               DeeObject *elem, /*nullable*/ DeeObject *key) {
	int result;
	if (!key)
		return DeeObject_CompareEq(keyed_search_item, elem);

	/* TODO: Special optimizations for specific keys (e.g. `string.lower') */
	elem = DeeObject_Call(key, 1, (DeeObject **)&elem);
	if unlikely(!elem)
		goto err;
	result = DeeObject_CompareEq(keyed_search_item, elem);
	Dee_Decref(elem);
	return result;
err:
	return -1;
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

#undef RESTRICT_IF_NOTYPE
#undef LOG_INHERIT

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_C */
