/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/code.h>
#include <deemon/object.h>
#include <deemon/gc.h>
#include <deemon/rodict.h>
#include <deemon/float.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/dict.h>
#include <deemon/hashset.h>
#include <deemon/objmethod.h>
#include <deemon/class.h>
#include <deemon/none.h>
#include <deemon/list.h>
#include <deemon/dict.h>
#include <deemon/tuple.h>
#include <deemon/thread.h>
#include <deemon/int.h>
#include <deemon/bool.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/util/string.h>

#include <stdarg.h>
#include <string.h>
#include <limits.h>


#include "runtime_error.h"
#include "strings.h"
#include "../objects/seq/svec.h"
#include "../objects/int_logic.h"

/* Operator invocation. */

DECL_BEGIN

#ifndef DEFINE_OPERATOR
#define DEFINE_OPERATOR(return,name,args) \
        PUBLIC return (DCALL DeeObject_##name) args
#endif /* !DEFINE_OPERATOR */


/* Setup how operator callbacks are invoked.
 * NOTE: When executing operators in a super-context, we must be
 *       careful when invoking class operators, because otherwise
 *       we'd accidentally invoke the overwritten operator:
 * DEEMON:
 * >> class MyClass {
 * >>     operator + (other) {
 * >>         print "MyClass() +",repr other;
 * >>         return this;
 * >>     }
 * >> }
 * >> class MySubClass: MyClass {
 * >>     operator + (other) {
 * >>         print "MySubClass() +",repr other;
 * >>         return this;
 * >>     }
 * >> }
 * >> class MySubSubClass: MySubClass {
 * >> }
 * >> local inst = MySubSubClass();
 * >> (inst as MyClass) + 7;
 * C:
 * >> DeeObject_TAdd(MyClass,inst,7);
 * >> func = MyClass->tp_math->tp_add; // func == &instance_add
 * >> // Invocing `func' directly at this point would result in
 * >> // all information about `MyClass' being referred to, to
 * >> // be lost, resulting in `MySubClass.operator + ' to be
 * >> // invoked instead.
 * >> if (func == &instance_add) {
 * >>     instance_tadd(MyClass,inst,7);
 * >> } else {
 * >>     func(inst,7);
 * >> }
 * >> // The solution to manually check for this case, and invoke
 * >> // the typed class function when that happens.
 * NOTE: This problem only arises in a super-context:
 * >> DeeObject_Add(inst,7); // Same as DeeObject_TAdd(MySubSubClass,inst,7)
 * >> FIND_FRIST_MATCH(tp_math->tp_add); // Found in `MySubClass'
 * >> INHERIT_MATCH();                   // Inherit `operator +' from `MySubClass' into `MyClass'
 * >>                                    // NOTE: This will only inherit the C-wrapper for the operator,
 * >>                                    //       essentially meaning:
 * >>                                    //       >> MySubSubClass->tp_math = MySubClass->tp_math;
 * >>                                    //       Where `tp_math->tp_add == &instance_add'
 * >> // With the tp_math set of operators now inherited from `MySubClass',
 * >> // we can directly invoke the operator from `MySubSubClass', without
 * >> // the need to checking for class operators, because we know that no
 * >> // base type of `inst' before `MySubClass' defined an `operator add'
 * >> MySubSubClass->tp_math->tp_add(inst,7);  // Invokes `instance_add()'
 * >> // `instance_add()' then invokes the following:
 * >> instance_tadd(MySubSubClass,inst,7);
 * >> // This will then invoke:
 * >> DeeClass_GetOperator(MySubSubClass,OPERATOR_ADD);
 * >> // After failing to find `OPERATOR_ADD' as part of `MySubSubClass', this
 * >> // function will then continue to search base-classes for the operator,
 * >> // until it finds it in `MySubClass', following which the associated
 * >> // callback will become cached as part of `MySubSubClass'
 * >> INVOKE_OPERATOR(inst,7);
 * So because of this, we only need to check for class operator callbacks in
 * a super-context, or in other words: when `DEFINE_TYPE_OPERATORS' is defined! */
#ifdef DEFINE_TYPE_OPERATORS
#define DeeType_INVOKE_ASSIGN(tp_self,self,other)              ((tp_self)->tp_init.tp_assign == &instance_assign ? instance_tassign(tp_self,self,other) : (*(tp_self)->tp_init.tp_assign)(self,other))
#define DeeType_INVOKE_MOVEASSIGN(tp_self,self,other)          ((tp_self)->tp_init.tp_move_assign == &instance_moveassign ? instance_tmoveassign(tp_self,self,other) : (*(tp_self)->tp_init.tp_move_assign)(self,other))
#define DeeType_INVOKE_STR(tp_self,self)                       ((tp_self)->tp_cast.tp_str == &instance_str ? instance_tstr(tp_self,self) : (*(tp_self)->tp_cast.tp_str)(self))
#define DeeType_INVOKE_REPR(tp_self,self)                      ((tp_self)->tp_cast.tp_repr == &instance_repr ? instance_trepr(tp_self,self) : (*(tp_self)->tp_cast.tp_repr)(self))
#define DeeType_INVOKE_BOOL(tp_self,self)                      ((tp_self)->tp_cast.tp_bool == &instance_bool ? instance_tbool(tp_self,self) : (*(tp_self)->tp_cast.tp_bool)(self))
#define DeeType_INVOKE_NEXT(tp_self,self)                      ((tp_self)->tp_iter_next == &instance_next ? instance_tnext(tp_self,self) : (*(tp_self)->tp_iter_next)(self))
#define DeeType_INVOKE_CALL(tp_self,self,argc,argv)            ((tp_self)->tp_call == &instance_call ? instance_tcall(tp_self,self,argc,argv) : (*(tp_self)->tp_call)(self,argc,argv))
#define DeeType_INVOKE_CALLKW(tp_self,self,argc,argv,kw)       ((tp_self)->tp_call_kw == &instance_callkw ? instance_tcallkw(tp_self,self,argc,argv,kw) : (*(tp_self)->tp_call_kw)(self,argc,argv,kw))
#define DeeType_INVOKE_INT32(tp_self,self,result)              ((tp_self)->tp_math->tp_int32 == &instance_int32 ? instance_tint32(tp_self,self,result) : (*(tp_self)->tp_math->tp_int32)(self,result))
#define DeeType_INVOKE_INT64(tp_self,self,result)              ((tp_self)->tp_math->tp_int64 == &instance_int64 ? instance_tint64(tp_self,self,result) : (*(tp_self)->tp_math->tp_int64)(self,result))
#define DeeType_INVOKE_DOUBLE(tp_self,self,result)             ((tp_self)->tp_math->tp_double == &instance_double ? instance_tdouble(tp_self,self,result) : (*(tp_self)->tp_math->tp_double)(self,result))
#define DeeType_INVOKE_INT(tp_self,self)                       ((tp_self)->tp_math->tp_int == &instance_int ? instance_tint(tp_self,self) : (*(tp_self)->tp_math->tp_int)(self))
#define DeeType_INVOKE_INV(tp_self,self)                       ((tp_self)->tp_math->tp_inv == &instance_inv ? instance_tinv(tp_self,self) : (*(tp_self)->tp_math->tp_inv)(self))
#define DeeType_INVOKE_POS(tp_self,self)                       ((tp_self)->tp_math->tp_pos == &instance_pos ? instance_tpos(tp_self,self) : (*(tp_self)->tp_math->tp_pos)(self))
#define DeeType_INVOKE_NEG(tp_self,self)                       ((tp_self)->tp_math->tp_neg == &instance_neg ? instance_tneg(tp_self,self) : (*(tp_self)->tp_math->tp_neg)(self))
#define DeeType_INVOKE_ADD(tp_self,self,other)                 ((tp_self)->tp_math->tp_add == &instance_add ? instance_tadd(tp_self,self,other) : (*(tp_self)->tp_math->tp_add)(self,other))
#define DeeType_INVOKE_SUB(tp_self,self,other)                 ((tp_self)->tp_math->tp_sub == &instance_sub ? instance_tsub(tp_self,self,other) : (*(tp_self)->tp_math->tp_sub)(self,other))
#define DeeType_INVOKE_MUL(tp_self,self,other)                 ((tp_self)->tp_math->tp_mul == &instance_mul ? instance_tmul(tp_self,self,other) : (*(tp_self)->tp_math->tp_mul)(self,other))
#define DeeType_INVOKE_DIV(tp_self,self,other)                 ((tp_self)->tp_math->tp_div == &instance_div ? instance_tdiv(tp_self,self,other) : (*(tp_self)->tp_math->tp_div)(self,other))
#define DeeType_INVOKE_MOD(tp_self,self,other)                 ((tp_self)->tp_math->tp_mod == &instance_mod ? instance_tmod(tp_self,self,other) : (*(tp_self)->tp_math->tp_mod)(self,other))
#define DeeType_INVOKE_SHL(tp_self,self,other)                 ((tp_self)->tp_math->tp_shl == &instance_shl ? instance_tshl(tp_self,self,other) : (*(tp_self)->tp_math->tp_shl)(self,other))
#define DeeType_INVOKE_SHR(tp_self,self,other)                 ((tp_self)->tp_math->tp_shr == &instance_shr ? instance_tshr(tp_self,self,other) : (*(tp_self)->tp_math->tp_shr)(self,other))
#define DeeType_INVOKE_AND(tp_self,self,other)                 ((tp_self)->tp_math->tp_and == &instance_and ? instance_tand(tp_self,self,other) : (*(tp_self)->tp_math->tp_and)(self,other))
#define DeeType_INVOKE_OR(tp_self,self,other)                  ((tp_self)->tp_math->tp_or == &instance_or ? instance_tor(tp_self,self,other) : (*(tp_self)->tp_math->tp_or)(self,other))
#define DeeType_INVOKE_XOR(tp_self,self,other)                 ((tp_self)->tp_math->tp_xor == &instance_xor ? instance_txor(tp_self,self,other) : (*(tp_self)->tp_math->tp_xor)(self,other))
#define DeeType_INVOKE_POW(tp_self,self,other)                 ((tp_self)->tp_math->tp_pow == &instance_pow ? instance_tpow(tp_self,self,other) : (*(tp_self)->tp_math->tp_pow)(self,other))
#define DeeType_INVOKE_INC(tp_self,pself)                      ((tp_self)->tp_math->tp_inc == &instance_inc ? instance_tinc(tp_self,pself) : (*(tp_self)->tp_math->tp_inc)(pself))
#define DeeType_INVOKE_DEC(tp_self,pself)                      ((tp_self)->tp_math->tp_dec == &instance_dec ? instance_tdec(tp_self,pself) : (*(tp_self)->tp_math->tp_dec)(pself))
#define DeeType_INVOKE_IADD(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_add == &instance_iadd ? instance_tiadd(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_add)(pself,other))
#define DeeType_INVOKE_ISUB(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_sub == &instance_isub ? instance_tisub(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_sub)(pself,other))
#define DeeType_INVOKE_IMUL(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_mul == &instance_imul ? instance_timul(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_mul)(pself,other))
#define DeeType_INVOKE_IDIV(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_div == &instance_idiv ? instance_tidiv(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_div)(pself,other))
#define DeeType_INVOKE_IMOD(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_mod == &instance_imod ? instance_timod(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_mod)(pself,other))
#define DeeType_INVOKE_ISHL(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_shl == &instance_ishl ? instance_tishl(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_shl)(pself,other))
#define DeeType_INVOKE_ISHR(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_shr == &instance_ishr ? instance_tishr(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_shr)(pself,other))
#define DeeType_INVOKE_IAND(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_and == &instance_iand ? instance_tiand(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_and)(pself,other))
#define DeeType_INVOKE_IOR(tp_self,pself,other)                ((tp_self)->tp_math->tp_inplace_or == &instance_ior ? instance_tior(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_or)(pself,other))
#define DeeType_INVOKE_IXOR(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_xor == &instance_ixor ? instance_tixor(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_xor)(pself,other))
#define DeeType_INVOKE_IPOW(tp_self,pself,other)               ((tp_self)->tp_math->tp_inplace_pow == &instance_ipow ? instance_tipow(tp_self,pself,other) : (*(tp_self)->tp_math->tp_inplace_pow)(pself,other))
#define DeeType_INVOKE_HASH(tp_self,self)                      ((tp_self)->tp_cmp->tp_hash == &instance_hash ? instance_thash(tp_self,self) : (tp_self)->tp_cmp->tp_hash == &instance_builtin_hash ? instance_builtin_thash(tp_self,self) : (*(tp_self)->tp_cmp->tp_hash)(self))
#define DeeType_INVOKE_EQ(tp_self,self,other)                  ((tp_self)->tp_cmp->tp_eq == &instance_eq ? instance_teq(tp_self,self,other) : (tp_self)->tp_cmp->tp_eq == &instance_builtin_eq ? instance_builtin_teq(tp_self,self,other) : (*(tp_self)->tp_cmp->tp_eq)(self,other))
#define DeeType_INVOKE_NE(tp_self,self,other)                  ((tp_self)->tp_cmp->tp_ne == &instance_ne ? instance_tne(tp_self,self,other) : (tp_self)->tp_cmp->tp_ne == &instance_builtin_ne ? instance_builtin_tne(tp_self,self,other) : (*(tp_self)->tp_cmp->tp_ne)(self,other))
#define DeeType_INVOKE_LO(tp_self,self,other)                  ((tp_self)->tp_cmp->tp_lo == &instance_lo ? instance_tlo(tp_self,self,other) : (tp_self)->tp_cmp->tp_lo == &instance_builtin_lo ? instance_builtin_tlo(tp_self,self,other) : (*(tp_self)->tp_cmp->tp_lo)(self,other))
#define DeeType_INVOKE_LE(tp_self,self,other)                  ((tp_self)->tp_cmp->tp_le == &instance_le ? instance_tle(tp_self,self,other) : (tp_self)->tp_cmp->tp_le == &instance_builtin_le ? instance_builtin_tle(tp_self,self,other) : (*(tp_self)->tp_cmp->tp_le)(self,other))
#define DeeType_INVOKE_GR(tp_self,self,other)                  ((tp_self)->tp_cmp->tp_gr == &instance_gr ? instance_tgr(tp_self,self,other) : (tp_self)->tp_cmp->tp_gr == &instance_builtin_gr ? instance_builtin_tgr(tp_self,self,other) : (*(tp_self)->tp_cmp->tp_gr)(self,other))
#define DeeType_INVOKE_GE(tp_self,self,other)                  ((tp_self)->tp_cmp->tp_ge == &instance_ge ? instance_tge(tp_self,self,other) : (tp_self)->tp_cmp->tp_ge == &instance_builtin_ge ? instance_builtin_tge(tp_self,self,other) : (*(tp_self)->tp_cmp->tp_ge)(self,other))
#define DeeType_INVOKE_ITER(tp_self,self)                      ((tp_self)->tp_seq->tp_iter_self == &instance_iter ? instance_titer(tp_self,self) : (*(tp_self)->tp_seq->tp_iter_self)(self))
#define DeeType_INVOKE_SIZE(tp_self,self)                      ((tp_self)->tp_seq->tp_size == &instance_size ? instance_tsize(tp_self,self) : (*(tp_self)->tp_seq->tp_size)(self))
#define DeeType_INVOKE_CONTAINS(tp_self,self,other)            ((tp_self)->tp_seq->tp_contains == &instance_contains ? instance_tcontains(tp_self,self,other) : (*(tp_self)->tp_seq->tp_contains)(self,other))
#define DeeType_INVOKE_GETITEM(tp_self,self,index)             ((tp_self)->tp_seq->tp_get == &instance_getitem ? instance_tgetitem(tp_self,self,index) : (*(tp_self)->tp_seq->tp_get)(self,index))
#define DeeType_INVOKE_DELITEM(tp_self,self,index)             ((tp_self)->tp_seq->tp_del == &instance_delitem ? instance_tdelitem(tp_self,self,index) : (*(tp_self)->tp_seq->tp_del)(self,index))
#define DeeType_INVOKE_SETITEM(tp_self,self,index,value)       ((tp_self)->tp_seq->tp_set == &instance_setitem ? instance_tsetitem(tp_self,self,index,value) : (*(tp_self)->tp_seq->tp_set)(self,index,value))
#define DeeType_INVOKE_GETRANGE(tp_self,self,start,end)        ((tp_self)->tp_seq->tp_range_get == &instance_getrange ? instance_tgetrange(tp_self,self,start,end) : (*(tp_self)->tp_seq->tp_range_get)(self,start,end))
#define DeeType_INVOKE_DELRANGE(tp_self,self,start,end)        ((tp_self)->tp_seq->tp_range_del == &instance_delrange ? instance_tdelrange(tp_self,self,start,end) : (*(tp_self)->tp_seq->tp_range_del)(self,start,end))
#define DeeType_INVOKE_SETRANGE(tp_self,self,start,end,values) ((tp_self)->tp_seq->tp_range_set == &instance_setrange ? instance_tsetrange(tp_self,self,start,end,values) : (*(tp_self)->tp_seq->tp_range_set)(self,start,end,values))
#define DeeType_INVOKE_GETATTR(tp_self,self,name)              ((tp_self)->tp_attr->tp_getattr == &instance_getattr ? instance_tgetattr(tp_self,self,name) : (*(tp_self)->tp_attr->tp_getattr)(self,name))
#define DeeType_INVOKE_DELATTR(tp_self,self,name)              ((tp_self)->tp_attr->tp_delattr == &instance_delattr ? instance_tdelattr(tp_self,self,name) : (*(tp_self)->tp_attr->tp_delattr)(self,name))
#define DeeType_INVOKE_SETATTR(tp_self,self,name,value)        ((tp_self)->tp_attr->tp_setattr == &instance_setattr ? instance_tsetattr(tp_self,self,name,value) : (*(tp_self)->tp_attr->tp_setattr)(self,name,value))
#define DeeType_INVOKE_ENTER(tp_self,self)                     ((tp_self)->tp_with->tp_enter == &instance_enter ? instance_tenter(tp_self,self) : (*(tp_self)->tp_with->tp_enter)(self))
#define DeeType_INVOKE_LEAVE(tp_self,self)                     ((tp_self)->tp_with->tp_leave == &instance_leave ? instance_tleave(tp_self,self) : (*(tp_self)->tp_with->tp_leave)(self))
#else /* DEFINE_TYPE_OPERATORS */
#define DeeType_INVOKE_ASSIGN(tp_self,self,other)              (*(tp_self)->tp_init.tp_assign)(self,other)
#define DeeType_INVOKE_MOVEASSIGN(tp_self,self,other)          (*(tp_self)->tp_init.tp_move_assign)(self,other)
#define DeeType_INVOKE_STR(tp_self,self)                       (*(tp_self)->tp_cast.tp_str)(self)
#define DeeType_INVOKE_REPR(tp_self,self)                      (*(tp_self)->tp_cast.tp_repr)(self)
#define DeeType_INVOKE_BOOL(tp_self,self)                      (*(tp_self)->tp_cast.tp_bool)(self)
#define DeeType_INVOKE_NEXT(tp_self,self)                      (*(tp_self)->tp_iter_next)(self)
#define DeeType_INVOKE_CALL(tp_self,self,argc,argv)            (*(tp_self)->tp_call)(self,argc,argv)
#define DeeType_INVOKE_CALLKW(tp_self,self,argc,argv,kw)       (*(tp_self)->tp_call_kw)(self,argc,argv,kw)
#define DeeType_INVOKE_INT32(tp_self,self,result)              (*(tp_self)->tp_math->tp_int32)(self,result)
#define DeeType_INVOKE_INT64(tp_self,self,result)              (*(tp_self)->tp_math->tp_int64)(self,result)
#define DeeType_INVOKE_DOUBLE(tp_self,self,result)             (*(tp_self)->tp_math->tp_double)(self,result)
#define DeeType_INVOKE_INT(tp_self,self)                       (*(tp_self)->tp_math->tp_int)(self)
#define DeeType_INVOKE_INV(tp_self,self)                       (*(tp_self)->tp_math->tp_inv)(self)
#define DeeType_INVOKE_POS(tp_self,self)                       (*(tp_self)->tp_math->tp_pos)(self)
#define DeeType_INVOKE_NEG(tp_self,self)                       (*(tp_self)->tp_math->tp_neg)(self)
#define DeeType_INVOKE_ADD(tp_self,self,other)                 (*(tp_self)->tp_math->tp_add)(self,other)
#define DeeType_INVOKE_SUB(tp_self,self,other)                 (*(tp_self)->tp_math->tp_sub)(self,other)
#define DeeType_INVOKE_MUL(tp_self,self,other)                 (*(tp_self)->tp_math->tp_mul)(self,other)
#define DeeType_INVOKE_DIV(tp_self,self,other)                 (*(tp_self)->tp_math->tp_div)(self,other)
#define DeeType_INVOKE_MOD(tp_self,self,other)                 (*(tp_self)->tp_math->tp_mod)(self,other)
#define DeeType_INVOKE_SHL(tp_self,self,other)                 (*(tp_self)->tp_math->tp_shl)(self,other)
#define DeeType_INVOKE_SHR(tp_self,self,other)                 (*(tp_self)->tp_math->tp_shr)(self,other)
#define DeeType_INVOKE_AND(tp_self,self,other)                 (*(tp_self)->tp_math->tp_and)(self,other)
#define DeeType_INVOKE_OR(tp_self,self,other)                  (*(tp_self)->tp_math->tp_or)(self,other)
#define DeeType_INVOKE_XOR(tp_self,self,other)                 (*(tp_self)->tp_math->tp_xor)(self,other)
#define DeeType_INVOKE_POW(tp_self,self,other)                 (*(tp_self)->tp_math->tp_pow)(self,other)
#define DeeType_INVOKE_INC(tp_self,pself)                      (*(tp_self)->tp_math->tp_inc)(pself)
#define DeeType_INVOKE_DEC(tp_self,pself)                      (*(tp_self)->tp_math->tp_dec)(pself)
#define DeeType_INVOKE_IADD(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_add)(pself,other)
#define DeeType_INVOKE_ISUB(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_sub)(pself,other)
#define DeeType_INVOKE_IMUL(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_mul)(pself,other)
#define DeeType_INVOKE_IDIV(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_div)(pself,other)
#define DeeType_INVOKE_IMOD(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_mod)(pself,other)
#define DeeType_INVOKE_ISHL(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_shl)(pself,other)
#define DeeType_INVOKE_ISHR(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_shr)(pself,other)
#define DeeType_INVOKE_IAND(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_and)(pself,other)
#define DeeType_INVOKE_IOR(tp_self,pself,other)                (*(tp_self)->tp_math->tp_inplace_or)(pself,other)
#define DeeType_INVOKE_IXOR(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_xor)(pself,other)
#define DeeType_INVOKE_IPOW(tp_self,pself,other)               (*(tp_self)->tp_math->tp_inplace_pow)(pself,other)
#define DeeType_INVOKE_HASH(tp_self,self)                      (*(tp_self)->tp_cmp->tp_hash)(self)
#define DeeType_INVOKE_EQ(tp_self,self,other)                  (*(tp_self)->tp_cmp->tp_eq)(self,other)
#define DeeType_INVOKE_NE(tp_self,self,other)                  (*(tp_self)->tp_cmp->tp_ne)(self,other)
#define DeeType_INVOKE_LO(tp_self,self,other)                  (*(tp_self)->tp_cmp->tp_lo)(self,other)
#define DeeType_INVOKE_LE(tp_self,self,other)                  (*(tp_self)->tp_cmp->tp_le)(self,other)
#define DeeType_INVOKE_GR(tp_self,self,other)                  (*(tp_self)->tp_cmp->tp_gr)(self,other)
#define DeeType_INVOKE_GE(tp_self,self,other)                  (*(tp_self)->tp_cmp->tp_ge)(self,other)
#define DeeType_INVOKE_ITER(tp_self,self)                      (*(tp_self)->tp_seq->tp_iter_self)(self)
#define DeeType_INVOKE_SIZE(tp_self,self)                      (*(tp_self)->tp_seq->tp_size)(self)
#define DeeType_INVOKE_CONTAINS(tp_self,self,other)            (*(tp_self)->tp_seq->tp_contains)(self,other)
#define DeeType_INVOKE_GETITEM(tp_self,self,index)             (*(tp_self)->tp_seq->tp_get)(self,index)
#define DeeType_INVOKE_DELITEM(tp_self,self,index)             (*(tp_self)->tp_seq->tp_del)(self,index)
#define DeeType_INVOKE_SETITEM(tp_self,self,index,value)       (*(tp_self)->tp_seq->tp_set)(self,index,value)
#define DeeType_INVOKE_GETRANGE(tp_self,self,start,end)        (*(tp_self)->tp_seq->tp_range_get)(self,start,end)
#define DeeType_INVOKE_DELRANGE(tp_self,self,start,end)        (*(tp_self)->tp_seq->tp_range_del)(self,start,end)
#define DeeType_INVOKE_SETRANGE(tp_self,self,start,end,values) (*(tp_self)->tp_seq->tp_range_set)(self,start,end,value)
#define DeeType_INVOKE_GETATTR(tp_self,self,name)              (*(tp_self)->tp_attr->tp_getattr)(self,name)
#define DeeType_INVOKE_DELATTR(tp_self,self,name)              (*(tp_self)->tp_attr->tp_delattr)(self,name)
#define DeeType_INVOKE_SETATTR(tp_self,self,name,value)        (*(tp_self)->tp_attr->tp_setattr)(self,name,value)
#define DeeType_INVOKE_ENTER(tp_self,self)                     (*(tp_self)->tp_with->tp_enter)(self)
#define DeeType_INVOKE_LEAVE(tp_self,self)                     (*(tp_self)->tp_with->tp_leave)(self)
#endif /* !DEFINE_TYPE_OPERATORS */

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
#define CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS 1

#ifndef DEFINE_TYPE_OPERATORS
INTERN bool DCALL
type_inherit_constructors(DeeTypeObject *__restrict self) {
 DeeTypeObject *base;
 if (!(self->tp_flags & TP_FINHERITCTOR) ||
      (base = self->tp_base) == NULL)
     return false;
 type_inherit_constructors(base);
 ASSERT((base->tp_flags & TP_FVARIABLE) ==
        (self->tp_flags & TP_FVARIABLE));
 DEE_DPRINTF("Inherit constructors from %q into %q\n",
             base->tp_name,self->tp_name);
 if (self->tp_flags & TP_FVARIABLE) {
  self->tp_init.tp_var.tp_ctor        = base->tp_init.tp_var.tp_ctor;
  self->tp_init.tp_var.tp_copy_ctor   = base->tp_init.tp_var.tp_copy_ctor;
  self->tp_init.tp_var.tp_deep_ctor   = base->tp_init.tp_var.tp_deep_ctor;
  self->tp_init.tp_var.tp_any_ctor    = base->tp_init.tp_var.tp_any_ctor;
  self->tp_init.tp_var.tp_free        = base->tp_init.tp_var.tp_free;
  self->tp_init.tp_var.tp_any_ctor_kw = base->tp_init.tp_var.tp_any_ctor_kw;
 } else {
  if (base->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS
   ASSERT((base->tp_flags&TP_FGC) == (self->tp_flags&TP_FGC));
#else /* !CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
   ASSERTF(!(base->tp_flags&TP_FGC) || (self->tp_flags&TP_FGC),
           "Non-GC object is inheriting its constructors for a GC-enabled object");
#endif /* CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
   self->tp_init.tp_alloc.tp_alloc = base->tp_init.tp_alloc.tp_alloc;
   self->tp_init.tp_alloc.tp_free  = base->tp_init.tp_alloc.tp_free;
  }
  self->tp_init.tp_alloc.tp_ctor        = base->tp_init.tp_alloc.tp_ctor;
  self->tp_init.tp_alloc.tp_copy_ctor   = base->tp_init.tp_alloc.tp_copy_ctor;
  self->tp_init.tp_alloc.tp_deep_ctor   = base->tp_init.tp_alloc.tp_deep_ctor;
  self->tp_init.tp_alloc.tp_any_ctor    = base->tp_init.tp_alloc.tp_any_ctor;
  self->tp_init.tp_alloc.tp_any_ctor_kw = base->tp_init.tp_alloc.tp_any_ctor_kw;
 }
 self->tp_init.tp_assign      = base->tp_init.tp_assign;
 self->tp_init.tp_move_assign = base->tp_init.tp_move_assign;
 self->tp_init.tp_deepload    = base->tp_init.tp_deepload;
 return true;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_NewDefault(DeeTypeObject *__restrict object_type) {
 DREF DeeObject *result;
 ASSERT_OBJECT(object_type);
 ASSERT(DeeType_Check(object_type));
 if (object_type->tp_flags&TP_FVARIABLE) {
  if (object_type->tp_init.tp_var.tp_ctor) {
do_invoke_var_ctor:
   return (*object_type->tp_init.tp_var.tp_ctor)();
  }
  if (object_type->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
   return (*object_type->tp_init.tp_var.tp_any_ctor)(0,NULL);
  }
  if (object_type->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
   return (*object_type->tp_init.tp_var.tp_any_ctor_kw)(0,NULL,NULL);
  }
  if (type_inherit_constructors(object_type)) {
   if (object_type->tp_init.tp_var.tp_ctor)
       goto do_invoke_var_ctor;
   if (object_type->tp_init.tp_var.tp_any_ctor)
       goto do_invoke_var_any_ctor;
   if (object_type->tp_init.tp_var.tp_any_ctor_kw)
       goto do_invoke_var_any_ctor_kw;
  }
 } else {
  int error;
  ASSERT(!(object_type->tp_flags&TP_FVARIABLE));
  if (object_type->tp_init.tp_alloc.tp_free)
   result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
  else if (object_type->tp_flags&TP_FGC)
   result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
  else {
   result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
  }
  if unlikely(!result) goto err;
  DeeObject_Init(result,object_type);
  if (object_type->tp_init.tp_alloc.tp_ctor) {
do_invoke_alloc_ctor:
   error = (*object_type->tp_init.tp_alloc.tp_ctor)(result);
  } else if (object_type->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
   error = (*object_type->tp_init.tp_alloc.tp_any_ctor)(result,0,NULL);
  } else if (object_type->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
   error = (*object_type->tp_init.tp_alloc.tp_any_ctor_kw)(result,0,NULL,NULL);
  } else {
   DeeObject_FreeTracker(result);
   if (object_type->tp_init.tp_alloc.tp_free)
    (*object_type->tp_init.tp_alloc.tp_free)(result);
   else if (object_type->tp_flags & TP_FGC)
    DeeGCObject_Free(result);
   else {
    DeeObject_Free(result);
   }
   if (!type_inherit_constructors(object_type))
        goto err_not_implemented_r;
   if (object_type->tp_init.tp_alloc.tp_free)
    result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
   else if (object_type->tp_flags & TP_FGC)
    result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
   else {
    result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
   }
   if unlikely(!result) goto err_object_type;
   DeeObject_InitNoref(result,object_type);
   if (object_type->tp_init.tp_alloc.tp_ctor)
       goto do_invoke_alloc_ctor;
   if (object_type->tp_init.tp_alloc.tp_any_ctor)
       goto do_invoke_alloc_any_ctor;
   if (object_type->tp_init.tp_alloc.tp_any_ctor_kw)
       goto do_invoke_alloc_any_ctor_kw;
   goto err_not_implemented_r;
  }
  if unlikely(error) goto err_r;
  /* Begin tracking the returned object. */
  if (object_type->tp_flags & TP_FGC)
      DeeGC_Track(result);
  return result;
 }
 err_unimplemented_constructor(object_type,0,NULL);
err:
 return NULL;
err_not_implemented_r:
 err_unimplemented_constructor(object_type,0,NULL);
err_r:
 DeeObject_FreeTracker(result);
 if (object_type->tp_init.tp_alloc.tp_free)
  (*object_type->tp_init.tp_alloc.tp_free)(result);
 else if (object_type->tp_flags & TP_FGC)
  DeeGCObject_Free(result);
 else {
  DeeObject_Free(result);
 }
err_object_type:
 Dee_Decref(object_type);
 goto err;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_New(DeeTypeObject *__restrict object_type,
              size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result;
 ASSERT_OBJECT(object_type);
 ASSERT(DeeType_Check(object_type));
 if (object_type->tp_flags&TP_FVARIABLE) {
  if (object_type->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
   return (*object_type->tp_init.tp_var.tp_any_ctor)(argc,argv);
  }
  if (object_type->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
   return (*object_type->tp_init.tp_var.tp_any_ctor_kw)(argc,argv,NULL);
  }
  if (object_type->tp_init.tp_var.tp_ctor && !argc) {
do_invoke_var_ctor:
   return (*object_type->tp_init.tp_var.tp_ctor)();
  }
  if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
      DeeObject_InstanceOf(argv[0],object_type)) {
do_invoke_var_copy:
   return (*object_type->tp_init.tp_var.tp_copy_ctor)(argv[0]);
  }
  if (type_inherit_constructors(object_type)) {
   if (object_type->tp_init.tp_var.tp_any_ctor)
       goto do_invoke_var_any_ctor;
   if (object_type->tp_init.tp_var.tp_any_ctor_kw)
       goto do_invoke_var_any_ctor_kw;
   if (object_type->tp_init.tp_var.tp_ctor && !argc)
       goto do_invoke_var_ctor;
   if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
       DeeObject_InstanceOf(argv[0],object_type))
       goto do_invoke_var_copy;
  }
 } else {
  int error;
  ASSERT(!(object_type->tp_flags&TP_FVARIABLE));
  if (object_type->tp_init.tp_alloc.tp_free)
   result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
  else if (object_type->tp_flags&TP_FGC)
   result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
  else {
   result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
  }
  if unlikely(!result) goto err;
  DeeObject_Init(result,object_type);
  if (object_type->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
   error = (*object_type->tp_init.tp_alloc.tp_any_ctor)(result,argc,argv);
  } else if (object_type->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
   error = (*object_type->tp_init.tp_alloc.tp_any_ctor_kw)(result,argc,argv,NULL);
  } else if (object_type->tp_init.tp_alloc.tp_ctor && !argc) {
do_invoke_alloc_ctor:
   error = (*object_type->tp_init.tp_alloc.tp_ctor)(result);
  } else if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
             DeeObject_InstanceOf(argv[0],object_type)) {
do_invoke_alloc_copy:
   error = (*object_type->tp_init.tp_alloc.tp_copy_ctor)(result,argv[0]);
  } else {
   DeeObject_FreeTracker(result);
   if (object_type->tp_init.tp_alloc.tp_free)
    (*object_type->tp_init.tp_alloc.tp_free)(result);
   else if (object_type->tp_flags & TP_FGC)
    DeeGCObject_Free(result);
   else {
    DeeObject_Free(result);
   }
   if (!type_inherit_constructors(object_type))
        goto err_not_implemented_r;
   if (object_type->tp_init.tp_alloc.tp_free)
    result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
   else if (object_type->tp_flags & TP_FGC)
    result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
   else {
    result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
   }
   if unlikely(!result) goto err_object_type;
   DeeObject_InitNoref(result,object_type);
   if (object_type->tp_init.tp_alloc.tp_any_ctor)
       goto do_invoke_alloc_any_ctor;
   if (object_type->tp_init.tp_alloc.tp_any_ctor_kw)
       goto do_invoke_alloc_any_ctor_kw;
   if (object_type->tp_init.tp_alloc.tp_ctor && argc == 0)
       goto do_invoke_alloc_ctor;
   if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
       DeeObject_InstanceOf(argv[0],object_type))
       goto do_invoke_alloc_copy;
   goto err_not_implemented_r;
  }
  if unlikely(error) goto err_r;
  /* Begin tracking the returned object. */
  if (object_type->tp_flags & TP_FGC)
      DeeGC_Track(result);
  return result;
 }
 err_unimplemented_constructor(object_type,argc,argv);
err:
 return NULL;
err_not_implemented_r:
 err_unimplemented_constructor(object_type,argc,argv);
err_r:
 DeeObject_FreeTracker(result);
 if (object_type->tp_init.tp_alloc.tp_free)
  (*object_type->tp_init.tp_alloc.tp_free)(result);
 else if (object_type->tp_flags & TP_FGC)
  DeeGCObject_Free(result);
 else {
  DeeObject_Free(result);
 }
err_object_type:
 Dee_Decref(object_type);
 goto err;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_NewKw(DeeTypeObject *__restrict object_type,
                size_t argc, DeeObject **__restrict argv,
                DeeObject *kw) {
 DREF DeeObject *result;
 ASSERT_OBJECT(object_type);
 ASSERT(DeeType_Check(object_type));
 if (object_type->tp_flags&TP_FVARIABLE) {
  if (object_type->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
   return (*object_type->tp_init.tp_var.tp_any_ctor_kw)(argc,argv,kw);
  }
  if (object_type->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t kw_size = DeeObject_Size(kw);
     if unlikely(kw_size == (size_t)-1) goto err;
     if (kw_size != 0)
         goto err_no_keywords;
    }
   }
   return (*object_type->tp_init.tp_var.tp_any_ctor)(argc,argv);
  }
  if (object_type->tp_init.tp_var.tp_ctor && !argc) {
do_invoke_var_ctor:
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t kw_size = DeeObject_Size(kw);
     if unlikely(kw_size == (size_t)-1) goto err;
     if (kw_size != 0)
         goto err_no_keywords;
    }
   }
   return (*object_type->tp_init.tp_var.tp_ctor)();
  }
  if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
      DeeObject_InstanceOf(argv[0],object_type)) {
do_invoke_var_copy:
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t kw_size = DeeObject_Size(kw);
     if unlikely(kw_size == (size_t)-1) goto err;
     if (kw_size != 0)
         goto err_no_keywords;
    }
   }
   return (*object_type->tp_init.tp_var.tp_copy_ctor)(argv[0]);
  }
  if (type_inherit_constructors(object_type)) {
   if (object_type->tp_init.tp_var.tp_any_ctor_kw)
       goto do_invoke_var_any_ctor_kw;
   if (object_type->tp_init.tp_var.tp_any_ctor)
       goto do_invoke_var_any_ctor;
   if (object_type->tp_init.tp_var.tp_ctor && !argc)
       goto do_invoke_var_ctor;
   if (object_type->tp_init.tp_var.tp_copy_ctor && argc == 1 &&
       DeeObject_InstanceOf(argv[0],object_type))
       goto do_invoke_var_copy;
  }
 } else {
  int error;
  ASSERT(!(object_type->tp_flags&TP_FVARIABLE));
  if (object_type->tp_init.tp_alloc.tp_free)
   result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
  else if (object_type->tp_flags&TP_FGC)
   result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
  else {
   result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
  }
  if unlikely(!result) goto err;
  DeeObject_Init(result,object_type);
  if (object_type->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
   error = (*object_type->tp_init.tp_alloc.tp_any_ctor_kw)(result,argc,argv,kw);
  } else if (object_type->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords_r;
    } else {
     size_t kw_size = DeeObject_Size(kw);
     if unlikely(kw_size == (size_t)-1) goto err_r;
     if (kw_size != 0)
         goto err_no_keywords_r;
    }
   }
   error = (*object_type->tp_init.tp_alloc.tp_any_ctor)(result,argc,argv);
  } else if (object_type->tp_init.tp_alloc.tp_ctor && !argc) {
do_invoke_alloc_ctor:
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords_r;
    } else {
     size_t kw_size = DeeObject_Size(kw);
     if unlikely(kw_size == (size_t)-1) goto err_r;
     if (kw_size != 0)
         goto err_no_keywords_r;
    }
   }
   error = (*object_type->tp_init.tp_alloc.tp_ctor)(result);
  } else if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
             DeeObject_InstanceOf(argv[0],object_type)) {
do_invoke_alloc_copy:
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords_r;
    } else {
     size_t kw_size = DeeObject_Size(kw);
     if unlikely(kw_size == (size_t)-1) goto err_r;
     if (kw_size != 0)
         goto err_no_keywords_r;
    }
   }
   error = (*object_type->tp_init.tp_alloc.tp_copy_ctor)(result,argv[0]);
  } else {
   DeeObject_FreeTracker(result);
   if (object_type->tp_init.tp_alloc.tp_free)
    (*object_type->tp_init.tp_alloc.tp_free)(result);
   else if (object_type->tp_flags & TP_FGC)
    DeeGCObject_Free(result);
   else {
    DeeObject_Free(result);
   }
   if (!type_inherit_constructors(object_type))
        goto err_not_implemented_r;
   if (object_type->tp_init.tp_alloc.tp_free)
    result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
   else if (object_type->tp_flags & TP_FGC)
    result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
   else {
    result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
   }
   if unlikely(!result) goto err_object_type;
   DeeObject_InitNoref(result,object_type);
   if (object_type->tp_init.tp_alloc.tp_any_ctor_kw)
       goto do_invoke_alloc_any_ctor_kw;
   if (object_type->tp_init.tp_alloc.tp_any_ctor)
       goto do_invoke_alloc_any_ctor;
   if (object_type->tp_init.tp_alloc.tp_ctor && argc == 0)
       goto do_invoke_alloc_ctor;
   if (object_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 &&
       DeeObject_InstanceOf(argv[0],object_type))
       goto do_invoke_alloc_copy;
   goto err_not_implemented_r;
  }
  if unlikely(error) goto err_r;
  /* Begin tracking the returned object. */
  if (object_type->tp_flags & TP_FGC)
      DeeGC_Track(result);
  return result;
 }
 err_unimplemented_constructor(object_type,argc,argv);
err:
 return NULL;
err_no_keywords_r:
 err_keywords_ctor_not_accepted(object_type,kw);
 goto err_r;
err_not_implemented_r:
 err_unimplemented_constructor(object_type,argc,argv);
err_r:
 DeeObject_FreeTracker(result);
 if (object_type->tp_init.tp_alloc.tp_free)
  (*object_type->tp_init.tp_alloc.tp_free)(result);
 else if (object_type->tp_flags & TP_FGC)
  DeeGCObject_Free(result);
 else {
  DeeObject_Free(result);
 }
err_object_type:
 Dee_Decref(object_type);
 goto err;
err_no_keywords:
 err_keywords_ctor_not_accepted(object_type,kw);
 goto err;
}


#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VNewPack,12),
                    ASSEMBLY_NAME(DeeObject_New,12));
#else
PUBLIC DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *__restrict object_type,
                   size_t argc, va_list args) {
 return DeeObject_New(object_type,argc,(DeeObject **)args);
}
#endif
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *__restrict object_type,
                   size_t argc, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VPackSymbolic(argc,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_New(object_type,argc,DeeTuple_ELEM(args_tuple));
 DeeTuple_DecrefSymbolic(args_tuple);
 return result;
}
#endif /* !CONFIG_VA_LIST_IS_STACK_POINTER */

PUBLIC DREF DeeObject *DCALL
DeeObject_VNewf(DeeTypeObject *__restrict object_type,
                char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VNewf(format,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_New(object_type,
                        DeeTuple_SIZE(args_tuple),
                        DeeTuple_ELEM(args_tuple));
 Dee_Decref(args_tuple);
 return result;
}
PUBLIC ATTR_SENTINEL DREF DeeObject *
DeeObject_NewPack(DeeTypeObject *__restrict object_type,
                  size_t argc, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,argc);
 result = DeeObject_VNewPack(object_type,argc,args);
 va_end(args);
 return result;
}
PUBLIC DREF DeeObject *
DeeObject_Newf(DeeTypeObject *__restrict object_type,
               char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeObject_VNewf(object_type,format,args);
 va_end(args);
 return result;
}
#endif /* DEFINE_TYPE_OPERATORS */

#ifdef DEFINE_TYPE_OPERATORS
#define LOAD_TP_SELF   ASSERT_OBJECT_TYPE(self,tp_self)
#define LOAD_TP_SELFP  ASSERT(pself); ASSERT_OBJECT_TYPE(*pself,tp_self)
#define LOAD_ITER      DeeTypeObject *iter = tp_self
#define LOAD_ITERP     DeeTypeObject *iter = tp_self
#define GET_TP_SELF()  tp_self
#define GET_TP_PSELF() tp_self
#else
#define LOAD_TP_SELF   DeeTypeObject *tp_self; \
                       ASSERT_OBJECT(self); \
                       tp_self = Dee_TYPE(self)
#define LOAD_TP_SELFP  DeeTypeObject *tp_self; \
                       ASSERT(pself); \
                       ASSERT_OBJECT(*pself); \
                       tp_self = Dee_TYPE(*pself)
#define LOAD_ITER      DeeTypeObject *iter; \
                       ASSERT_OBJECT(self); \
                       iter = Dee_TYPE(self)
#define LOAD_ITERP     DeeTypeObject *iter; \
                       ASSERT(pself); \
                       ASSERT_OBJECT(*pself); \
                       iter = Dee_TYPE(*pself)
#define GET_TP_SELF()  Dee_TYPE(self)
#define GET_TP_PSELF() Dee_TYPE(*pself)
#endif
 


STATIC_ASSERT(COMPILER_OFFSETOF(DeeTypeObject,tp_init.tp_alloc.tp_deep_ctor) ==
              COMPILER_OFFSETOF(DeeTypeObject,tp_init.tp_var.tp_deep_ctor));

DEFINE_OPERATOR(DREF DeeObject *,DeepCopy,(DeeObject *__restrict self)) {
 DREF DeeObject *result;
 DeeThreadObject *thread_self = DeeThread_Self();
 LOAD_TP_SELF;
 /* Check to make sure that a deepcopy construction is implemented by this type.
  * Note that the variable-and fixed-length constructors are located at the same
  * offset in the type structure, meaning that we only need to check one address. */
 if unlikely(!tp_self->tp_init.tp_alloc.tp_deep_ctor) {
  if (!tp_self->tp_init.tp_alloc.tp_copy_ctor) {
   if (type_inherit_constructors(tp_self)) {
    if (tp_self->tp_init.tp_alloc.tp_deep_ctor)
        goto got_deep_copy;
    if (tp_self->tp_init.tp_alloc.tp_copy_ctor)
        goto got_normal_copy;
   }
   /* when neither a deepcopy, nor a regular copy operator are present
    * assume that the object is immutable and re-return the object itself. */
   return_reference_(self);
  }
got_normal_copy:
  /* There isn't a deepcopy operator, but there is a copy operator.
   * Now, if there also is a deepload operator, then we can invoke that one! */
  if unlikely(!tp_self->tp_init.tp_deepload) {
   err_unimplemented_operator(tp_self,OPERATOR_DEEPCOPY);
   return NULL;
  }
 }
got_deep_copy:

 /* Check if this object is already been constructed. */
 result = deepcopy_lookup(thread_self,self,tp_self);
 if (result) return_reference_(result);
 deepcopy_begin(thread_self);
 /* Allocate an to basic construction of the deepcopy object. */
 if (tp_self->tp_flags & TP_FVARIABLE) {
  /* Variable-length object. */
  result = tp_self->tp_init.tp_var.tp_deep_ctor
         ? (*tp_self->tp_init.tp_var.tp_deep_ctor)(self)
         : (*tp_self->tp_init.tp_var.tp_copy_ctor)(self)
         ;
  if unlikely(!result) goto done_endcopy;
 } else {
  ASSERT(!(tp_self->tp_flags&TP_FVARIABLE));
  /* Static-length object. */
  if (tp_self->tp_init.tp_alloc.tp_free)
   result = (DREF DeeObject *)(*tp_self->tp_init.tp_alloc.tp_alloc)();
  else if (tp_self->tp_flags&TP_FGC)
   result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
  else {
   result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
  }
  if unlikely(!result) goto done_endcopy;
  /* Perform basic object initialization. */
  DeeObject_Init(result,tp_self);
  /* Invoke the deepcopy constructor first. */
  if unlikely(tp_self->tp_init.tp_alloc.tp_deep_ctor
          ? (*tp_self->tp_init.tp_alloc.tp_deep_ctor)(result,self)
          : (*tp_self->tp_init.tp_alloc.tp_copy_ctor)(result,self)) {
   /* Undo allocating and base-initializing the new object. */
   DeeObject_FreeTracker(result);
   if (tp_self->tp_init.tp_alloc.tp_free)
    (*tp_self->tp_init.tp_alloc.tp_free)(result);
   else if (tp_self->tp_flags&TP_FGC)
    DeeGCObject_Free(result);
   else {
    DeeObject_Free(result);
   }
   Dee_Decref(tp_self);
   result = NULL;
   goto done_endcopy;
  }
  /* Begin tracking the returned object if this is a GC type. */
  if (tp_self->tp_flags&TP_FGC)
      DeeGC_Track(result);
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
  if (Dee_DeepCopyAddAssoc(result,self))
      goto err_result_endcopy;
  if unlikely((*tp_self->tp_init.tp_deepload)(result))
     goto done_endcopy;
done_endcopy:
  deepcopy_end(thread_self);
 } else {
  /* Optimization: In the event that this is the first-level deepcopy call,
   *               yet the type does not implement the deepload protocol
   *              (as could be the case for immutable sequence types like
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
   if (Dee_DeepCopyAddAssoc(result,self))
       goto err_result;
  }
 }
done:
 return result;
err_result_endcopy: Dee_Clear(result);
err_result: Dee_Clear(result); goto done;
}
DEFINE_OPERATOR(DREF DeeObject *,Copy,(DeeObject *__restrict self)) {
 DREF DeeObject *result;
 LOAD_TP_SELF;
 if (tp_self->tp_flags&TP_FVARIABLE) {
  if (tp_self->tp_init.tp_var.tp_copy_ctor) {
do_invoke_var_copy:
   return (*tp_self->tp_init.tp_var.tp_copy_ctor)(self);
  }
  if (tp_self->tp_init.tp_var.tp_deep_ctor) {
do_invoke_var_deep:
#ifdef DEFINE_TYPE_OPERATORS
   return DeeObject_TDeepCopy(tp_self,self);
#else
   return DeeObject_DeepCopy(self);
#endif
  }
  if (tp_self->tp_init.tp_var.tp_any_ctor) {
do_invoke_var_any_ctor:
   return (*tp_self->tp_init.tp_var.tp_any_ctor)(1,(DeeObject **)&self);
  }
  if (tp_self->tp_init.tp_var.tp_any_ctor_kw) {
do_invoke_var_any_ctor_kw:
   return (*tp_self->tp_init.tp_var.tp_any_ctor_kw)(1,(DeeObject **)&self,NULL);
  }
  if (type_inherit_constructors(tp_self)) {
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
  if (tp_self->tp_init.tp_alloc.tp_free)
   result = (DREF DeeObject *)(*tp_self->tp_init.tp_alloc.tp_alloc)();
  else if (tp_self->tp_flags&TP_FGC)
   result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
  else {
   result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
  }
  if unlikely(!result) goto err;
  DeeObject_Init(result,tp_self);
  error = (*tp_self->tp_init.tp_alloc.tp_copy_ctor)(result,self);
  if unlikely(error) goto err_r;
  /* Begin tracking the returned object. */
  if (tp_self->tp_flags & TP_FGC)
      DeeGC_Track(result);
  return result;
 } else if (tp_self->tp_init.tp_alloc.tp_deep_ctor) {
  goto do_invoke_var_deep;
 } else {
  int error;
  ASSERT(!(tp_self->tp_flags&TP_FVARIABLE));
  if (tp_self->tp_init.tp_alloc.tp_free)
   result = (DREF DeeObject *)(*tp_self->tp_init.tp_alloc.tp_alloc)();
  else if (tp_self->tp_flags&TP_FGC)
   result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
  else {
   result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
  }
  if unlikely(!result) goto err;
  DeeObject_Init(result,tp_self);
  if (tp_self->tp_init.tp_alloc.tp_any_ctor) {
do_invoke_alloc_any_ctor:
   error = (*tp_self->tp_init.tp_alloc.tp_any_ctor)(result,1,(DeeObject **)&self);
  } else if (tp_self->tp_init.tp_alloc.tp_any_ctor_kw) {
do_invoke_alloc_any_ctor_kw:
   error = (*tp_self->tp_init.tp_alloc.tp_any_ctor_kw)(result,1,(DeeObject **)&self,NULL);
  } else {
   DeeObject_FreeTracker(result);
   if (tp_self->tp_init.tp_alloc.tp_free)
    (*tp_self->tp_init.tp_alloc.tp_free)(result);
   else if (tp_self->tp_flags & TP_FGC)
    DeeGCObject_Free(result);
   else {
    DeeObject_Free(result);
   }
   if (!type_inherit_constructors(tp_self))
        goto err_not_implemented_r;
   if (tp_self->tp_init.tp_alloc.tp_copy_ctor) {
    Dee_DecrefNokill(tp_self);
    goto do_invoke_alloc_copy;
   }
   if (tp_self->tp_init.tp_alloc.tp_deep_ctor) {
    Dee_DecrefNokill(tp_self);
    goto do_invoke_var_deep;
   }
   if (tp_self->tp_init.tp_alloc.tp_free)
    result = (DREF DeeObject *)(*tp_self->tp_init.tp_alloc.tp_alloc)();
   else if (tp_self->tp_flags & TP_FGC)
    result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
   else {
    result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
   }
   if unlikely(!result) goto err_object_type;
   DeeObject_InitNoref(result,tp_self);
   if (tp_self->tp_init.tp_alloc.tp_any_ctor)
       goto do_invoke_alloc_any_ctor;
   if (tp_self->tp_init.tp_alloc.tp_any_ctor_kw)
       goto do_invoke_alloc_any_ctor_kw;
   goto err_not_implemented_r;
  }
  if unlikely(error) goto err_r;
  /* Begin tracking the returned object. */
  if (tp_self->tp_flags & TP_FGC)
      DeeGC_Track(result);
  return result;
 }
 err_unimplemented_constructor(tp_self,0,NULL);
err:
 return NULL;
err_not_implemented_r:
 err_unimplemented_constructor(tp_self,0,NULL);
err_r:
 DeeObject_FreeTracker(result);
 if (tp_self->tp_init.tp_alloc.tp_free)
  (*tp_self->tp_init.tp_alloc.tp_free)(result);
 else if (tp_self->tp_flags & TP_FGC)
  DeeGCObject_Free(result);
 else {
  DeeObject_Free(result);
 }
err_object_type:
 Dee_Decref(tp_self);
 goto err;
}


#ifndef DEFINE_TYPE_OPERATORS
PUBLIC int (DCALL DeeObject_InplaceDeepCopy)(DREF DeeObject **__restrict pself) {
 DeeObject *objcopy,*old_object;
 ASSERT(pself);
 old_object = *pself;
 ASSERT_OBJECT(old_object);
 objcopy = DeeObject_DeepCopy(old_object);
 if unlikely(!objcopy) return -1;
 Dee_Decref(old_object);
 *pself = objcopy;
 return 0;
}
#ifndef CONFIG_NO_THREADS
PUBLIC int DCALL
DeeObject_InplaceDeepCopyWithLock(DREF DeeObject **__restrict pself,
                                  rwlock_t *__restrict plock) {
 DREF DeeObject *temp,*copy;
 /* Step #1: Extract the existing object. */
 rwlock_read(plock);
 temp = *pself;
 Dee_Incref(temp);
 rwlock_endread(plock);
 /* Step #2: Create a deep copy for it. */
 copy = DeeObject_DeepCopy(temp);
 Dee_Decref(temp);
 if unlikely(!copy)
    goto err;
 /* Step #3: Write back the newly created deep copy. */
 rwlock_write(plock);
 temp   = *pself; /* Inherit */
 *pself = copy;   /* Inherit */
 rwlock_endwrite(plock);
 Dee_Decref(temp);
 return 0;
err:
 return -1;
}
PUBLIC int DCALL
DeeObject_InplaceXDeepCopyWithLock(DREF DeeObject **__restrict pself,
                                   rwlock_t *__restrict plock) {
 DREF DeeObject *temp,*copy;
 /* Step #1: Extract the existing object. */
 rwlock_read(plock);
 temp = *pself;
 if (!temp) {
  rwlock_endread(plock);
  goto done;
 }
 Dee_Incref(temp);
 rwlock_endread(plock);
 /* Step #2: Create a deep copy for it. */
 copy = DeeObject_DeepCopy(temp);
 Dee_Decref(temp);
 if unlikely(!copy)
    goto err;
 /* Step #3: Write back the newly created deep copy. */
 rwlock_write(plock);
 temp   = *pself; /* Inherit */
 *pself = copy;   /* Inherit */
 rwlock_endwrite(plock);
 Dee_XDecref(temp);
done: return 0;
err:  return -1;
}
#endif
#endif /* !DEFINE_TYPE_OPERATORS */

DEFINE_OPERATOR(int,Assign,(DeeObject *__restrict self, DeeObject *__restrict some_object)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(some_object);
 if (tp_self->tp_init.tp_assign) {
do_assign:
  return DeeType_INVOKE_ASSIGN(tp_self,self,some_object);
 }
 if (type_inherit_constructors(tp_self)) {
  if (tp_self->tp_init.tp_assign)
      goto do_assign;
 }
 return err_unimplemented_operator(GET_TP_SELF(),OPERATOR_ASSIGN);
}
DEFINE_OPERATOR(int,MoveAssign,(DeeObject *__restrict self, DeeObject *__restrict other)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(other);
 if (!(tp_self->tp_flags & TP_FMOVEANY) &&
       DeeObject_AssertType(other,tp_self))
       goto err;
 if (tp_self->tp_init.tp_move_assign) {
do_move_assign:
  return DeeType_INVOKE_MOVEASSIGN(tp_self,self,other);
 }
 if (tp_self->tp_init.tp_assign) {
do_assign:
  return DeeType_INVOKE_ASSIGN(tp_self,self,other);
 }
 if (type_inherit_constructors(tp_self)) {
  if (tp_self->tp_init.tp_move_assign)
      goto do_move_assign;
  if (tp_self->tp_init.tp_assign)
      goto do_assign;
 }
 return err_unimplemented_operator(GET_TP_SELF(),OPERATOR_MOVEASSIGN);
err:
 return -1;
}


#ifdef DEFINE_TYPE_OPERATORS
LOCAL bool DCALL
repr_contains(struct trepr_frame *chain,
              DeeTypeObject *__restrict tp,
              DeeObject *__restrict ob)
#else
LOCAL bool DCALL
repr_contains(struct repr_frame *chain,
              DeeObject *__restrict ob)
#endif
{
 for (; chain; chain = chain->rf_prev) {
#ifdef DEFINE_TYPE_OPERATORS
  if (chain->rf_obj == ob &&
      chain->rf_type == tp)
      return true;
#else
  if (chain->rf_obj == ob)
      return true;
#endif
 }
 return false;
}

/* Make sure the repr-frame offsets match. */
STATIC_ASSERT(COMPILER_OFFSETOF(struct trepr_frame,rf_prev) ==
              COMPILER_OFFSETOF(struct repr_frame,rf_prev));
STATIC_ASSERT(COMPILER_OFFSETOF(struct trepr_frame,rf_obj) ==
              COMPILER_OFFSETOF(struct repr_frame,rf_obj));

#ifdef DEFINE_TYPE_OPERATORS
#define Xrepr_frame trepr_frame
#else
#define Xrepr_frame repr_frame
#endif

#ifndef DEFINE_TYPE_OPERATORS
#define DEFINE_TYPE_INHERIT_FUNCTION(name,opname,field) \
INTERN bool DCALL \
name(DeeTypeObject *__restrict self) { \
 DeeTypeObject *base = DeeType_Base(self); \
 if (!base || \
    (!base->field && !name(base))) \
     return false; \
 DEE_DPRINTF("Inherit `" opname "' from %q into %q\n", \
             base->tp_name,self->tp_name); \
 self->field = base->field; \
 return true; \
}
#define DEFINE_TYPE_INHERIT_FUNCTION2(name,opname,field,field2) \
INTERN bool DCALL \
name(DeeTypeObject *__restrict self) { \
 DeeTypeObject *base = DeeType_Base(self); \
 if (!base || \
    (!base->field && !base->field2 && !name(base))) \
     return false; \
 DEE_DPRINTF("Inherit `" opname "' from %q into %q\n", \
             base->tp_name,self->tp_name); \
 self->field  = base->field; \
 self->field2 = base->field2; \
 return true; \
}
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_str,"operator str",tp_cast.tp_str)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_repr,"operator repr",tp_cast.tp_repr)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_bool,"operator bool",tp_cast.tp_bool)
DEFINE_TYPE_INHERIT_FUNCTION2(type_inherit_call,"operator call",tp_call,tp_call_kw)
#undef DEFINE_TYPE_INHERIT_FUNCTION2
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif


DEFINE_OPERATOR(DREF DeeObject *,Str,(DeeObject *__restrict self)) {
 DREF DeeObject *result;
 LOAD_TP_SELF;
 if unlikely(!tp_self->tp_cast.tp_str &&
             !type_inherit_str(tp_self))
    goto missing;
 /* Handle string-repr recursion for GC objects. */
 if (tp_self->tp_flags&TP_FGC) {
  struct Xrepr_frame opframe;
  DeeThreadObject *this_thread;
  this_thread = DeeThread_Self();
  /* Trace objects for which __str__ is being invoked. */
  opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_str_curr;
#ifdef DEFINE_TYPE_OPERATORS
  if unlikely(repr_contains(opframe.rf_prev,tp_self,self)) goto recursion;
  opframe.rf_obj  = self;
  opframe.rf_type = tp_self;
  this_thread->t_str_curr = (struct repr_frame *)&opframe;
#else /* DEFINE_TYPE_OPERATORS */
  if unlikely(repr_contains(opframe.rf_prev,self)) goto recursion;
  opframe.rf_obj  = self;
  this_thread->t_str_curr = &opframe;
#endif /* !DEFINE_TYPE_OPERATORS */
  result = DeeType_INVOKE_STR(tp_self,self);
  this_thread->t_str_curr = (struct repr_frame *)opframe.rf_prev;
  ASSERT_OBJECT_TYPE_EXACT_OPT(result,&DeeString_Type);
  return result;
 }
 /* Non-gc object (much simpler) */
 return DeeType_INVOKE_STR(tp_self,self);
missing:
 err_unimplemented_operator(tp_self,OPERATOR_STR);
 return NULL;
recursion:
 return_reference_(&str_dots);
}
DEFINE_OPERATOR(DREF DeeObject *,Repr,(DeeObject *__restrict self)) {
 DREF DeeObject *result;
 LOAD_TP_SELF;
 if unlikely(!tp_self->tp_cast.tp_repr &&
             !type_inherit_repr(tp_self))
    goto missing;
 /* Handle string-repr recursion for GC objects. */
 if (tp_self->tp_flags&TP_FGC) {
  struct Xrepr_frame opframe;
  DeeThreadObject *this_thread;
  this_thread = DeeThread_Self();
  /* Trace objects for which __repr__ is being invoked. */
  opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_repr_curr;
#ifdef DEFINE_TYPE_OPERATORS
  if unlikely(repr_contains(opframe.rf_prev,tp_self,self)) goto recursion;
  opframe.rf_obj  = self;
  opframe.rf_type = tp_self;
  this_thread->t_repr_curr = (struct repr_frame *)&opframe;
#else /* DEFINE_TYPE_OPERATORS */
  if unlikely(repr_contains(opframe.rf_prev,self)) goto recursion;
  opframe.rf_obj  = self;
  this_thread->t_repr_curr = &opframe;
#endif /* !DEFINE_TYPE_OPERATORS */
  result = DeeType_INVOKE_REPR(tp_self,self);
  this_thread->t_repr_curr = (struct repr_frame *)opframe.rf_prev;
  ASSERT_OBJECT_TYPE_EXACT_OPT(result,&DeeString_Type);
  return result;
 }
 /* Non-gc object (much simpler) */
 return DeeType_INVOKE_REPR(tp_self,self);
missing:
 err_unimplemented_operator(tp_self,OPERATOR_REPR);
 return NULL;
recursion:
 return_reference_(&str_dots);
}

#undef Xrepr_frame

DEFINE_OPERATOR(int,Bool,(DeeObject *__restrict self)) {
 /* _very_ likely case: `self' is one of the boolean constants
  *  -> In this case, we return the result immediately! */
 if (self == Dee_True) return 1;
 if (self == Dee_False) return 0;
 LOAD_TP_SELF;
 if likely(tp_self->tp_cast.tp_bool ||
           type_inherit_bool(tp_self))
    return DeeType_INVOKE_BOOL(tp_self,self);
 return err_unimplemented_operator(tp_self,OPERATOR_BOOL);
}

DEFINE_OPERATOR(DREF DeeObject *,Call,
               (DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_call)
      return DeeType_INVOKE_CALL(tp_self,self,argc,argv);
  if (tp_self->tp_call_kw)
      return DeeType_INVOKE_CALLKW(tp_self,self,argc,argv,NULL);
 } while (type_inherit_call(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_CALL);
 return NULL;
}

#ifndef DEFINE_TYPE_OPERATORS
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
DEFINE_OPERATOR(DREF DeeObject *,CallTuple,
               (DeeObject *__restrict self,
                DeeObject *__restrict args)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT_TYPE_EXACT(args,&DeeTuple_Type);
 if (tp_self == &DeeFunction_Type)
     return DeeFunction_CallTuple((DeeFunctionObject *)self,args);
 do {
  if (tp_self->tp_call)
      return DeeType_INVOKE_CALL(tp_self,self,
                                 DeeTuple_SIZE(args),
                                 DeeTuple_ELEM(args));
  if (tp_self->tp_call_kw)
      return DeeType_INVOKE_CALLKW(tp_self,self,
                                   DeeTuple_SIZE(args),
                                   DeeTuple_ELEM(args),
                                   NULL);
 } while (type_inherit_call(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_CALL);
 return NULL;
}
DEFINE_OPERATOR(DREF DeeObject *,CallTupleKw,
               (DeeObject *__restrict self,
                DeeObject *__restrict args,
                DeeObject *kw)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT_TYPE_EXACT(args,&DeeTuple_Type);
 if (tp_self == &DeeFunction_Type)
     return DeeFunction_CallTupleKw((DeeFunctionObject *)self,args,kw);
 do {
  if (tp_self->tp_call_kw)
      return DeeType_INVOKE_CALLKW(tp_self,self,
                                   DeeTuple_SIZE(args),
                                   DeeTuple_ELEM(args),
                                   kw);
  if (tp_self->tp_call) {
   /* Object doesn't support keyword arguments. */
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t kw_length;
     kw_length = DeeObject_Size(kw);
     if unlikely(kw_length == (size_t)-1) return NULL;
     if (kw_length != 0) goto err_no_keywords;
    }
   }
   return DeeType_INVOKE_CALL(tp_self,self,
                              DeeTuple_SIZE(args),
                              DeeTuple_ELEM(args));
  }
 } while (type_inherit_call(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_CALL);
 return NULL;
err_no_keywords:
 err_keywords_not_accepted(tp_self,kw);
 return NULL;
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
#endif /* !DEFINE_TYPE_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *,CallKw,
               (DeeObject *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_call_kw)
      return DeeType_INVOKE_CALLKW(tp_self,self,argc,argv,kw);
  if (tp_self->tp_call) {
   /* Object doesn't support keyword arguments. */
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t kw_length;
     kw_length = DeeObject_Size(kw);
     if unlikely(kw_length == (size_t)-1) return NULL;
     if (kw_length != 0) goto err_no_keywords;
    }
   }
   return DeeType_INVOKE_CALL(tp_self,self,argc,argv);
  }
 } while (type_inherit_call(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_CALL);
 return NULL;
err_no_keywords:
 err_keywords_not_accepted(tp_self,kw);
 return NULL;
}

#ifndef DEFINE_TYPE_OPERATORS
INTERN DREF DeeObject *DCALL
DeeObject_TThisCall(DeeTypeObject *__restrict tp_self,
                    DeeObject *__restrict self,
                    DeeObject *__restrict this_arg,
                    size_t argc, DeeObject **__restrict argv);
INTERN DREF DeeObject *DCALL
DeeObject_TThisCallKw(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      DeeObject *__restrict this_arg,
                      size_t argc, DeeObject **__restrict argv,
                      DeeObject *kw);
#endif

DEFINE_OPERATOR(DREF DeeObject *,ThisCall,
               (DeeObject *__restrict self,
                DeeObject *__restrict this_arg,
                size_t argc, DeeObject **__restrict argv)) {
 DREF DeeObject *full_args,*result;
 ASSERT_OBJECT(self);
#ifndef DEFINE_TYPE_OPERATORS
 if (DeeSuper_Check(self)) {
  return DeeObject_TThisCall(DeeSuper_TYPE(self),
                             DeeSuper_SELF(self),
                             this_arg,argc,argv);
 }
#endif

 /* Check for special callback optimizations. */
 if (GET_TP_SELF() == &DeeFunction_Type)
     return DeeFunction_ThisCall((DeeFunctionObject *)self,this_arg,argc,argv);
 if (GET_TP_SELF() == &DeeClsMethod_Type) {
  /* Must ensure proper typing of the this-argument. */
  if (DeeObject_AssertType(this_arg,((DeeClsMethodObject *)self)->cm_type))
      return NULL;
  return (*((DeeClsMethodObject *)self)->cm_func)(this_arg,argc,argv);
 }
 if (GET_TP_SELF() == &DeeKwClsMethod_Type) {
  /* Must ensure proper typing of the this-argument. */
  if (DeeObject_AssertType(this_arg,((DeeKwClsMethodObject *)self)->cm_type))
      return NULL;
  return (*((DeeKwClsMethodObject *)self)->cm_func)(this_arg,argc,argv,NULL);
 }
 /* sigh... Looks like we need to create a temporary argument tuple... */
 full_args = DeeTuple_NewUninitialized(1+argc);
 if unlikely(!full_args) return NULL;
 /* Lazily alias arguments in the `full_args' tuple. */
 DeeTuple_SET(full_args,0,this_arg);
 memcpy(&DeeTuple_ELEM(full_args)[1],argv,argc*sizeof(DeeObject *));
#ifdef DEFINE_TYPE_OPERATORS
 result = DeeObject_TCall(tp_self,self,
                          DeeTuple_SIZE(full_args),
                          DeeTuple_ELEM(full_args));
#else
 result = DeeObject_Call(self,
                         DeeTuple_SIZE(full_args),
                         DeeTuple_ELEM(full_args));
#endif
 DeeTuple_DecrefSymbolic(full_args);
 return result;
}

DEFINE_OPERATOR(DREF DeeObject *,ThisCallKw,
               (DeeObject *__restrict self,
                DeeObject *__restrict this_arg,
                size_t argc, DeeObject **__restrict argv,
                DeeObject *kw)) {
 DREF DeeObject *full_args,*result;
 ASSERT_OBJECT(self);
#ifndef DEFINE_TYPE_OPERATORS
 if (DeeSuper_Check(self)) {
  return DeeObject_TThisCallKw(DeeSuper_TYPE(self),
                               DeeSuper_SELF(self),
                               this_arg,argc,argv,kw);
 }
#endif

 /* Check for special callback optimizations. */
 if (GET_TP_SELF() == &DeeFunction_Type)
     return DeeFunction_ThisCallKw((DeeFunctionObject *)self,this_arg,argc,argv,kw);
 if (GET_TP_SELF() == &DeeKwClsMethod_Type) {
  /* Must ensure proper typing of the this-argument. */
  if (DeeObject_AssertType(this_arg,((DeeKwClsMethodObject *)self)->cm_type))
      return NULL;
  return (*((DeeKwClsMethodObject *)self)->cm_func)(this_arg,argc,argv,kw);
 }
 if (GET_TP_SELF() == &DeeClsMethod_Type) {
  /* Must ensure proper typing of the this-argument. */
  if (DeeObject_AssertType(this_arg,((DeeClsMethodObject *)self)->cm_type))
      return NULL;
  if (kw) {
   if (DeeKwds_Check(kw)) {
    if (DeeKwds_SIZE(kw) != 0)
        goto err_no_keywords;
   } else {
    size_t kw_length;
    kw_length = DeeObject_Size(kw);
    if unlikely(kw_length == (size_t)-1) return NULL;
    if (kw_length != 0) goto err_no_keywords;
   }
  }
  return (*((DeeClsMethodObject *)self)->cm_func)(this_arg,argc,argv);
 }
 /* sigh... Looks like we need to create a temporary argument tuple... */
 full_args = DeeTuple_NewUninitialized(1+argc);
 if unlikely(!full_args) return NULL;
 /* Lazily alias arguments in the `full_args' tuple. */
 DeeTuple_SET(full_args,0,this_arg);
 memcpy(&DeeTuple_ELEM(full_args)[1],argv,argc*sizeof(DeeObject *));
#ifdef DEFINE_TYPE_OPERATORS
 result = DeeObject_TCallKw(tp_self,self,
                            DeeTuple_SIZE(full_args),
                            DeeTuple_ELEM(full_args),
                            kw);
#else
 result = DeeObject_CallKw(self,
                           DeeTuple_SIZE(full_args),
                           DeeTuple_ELEM(full_args),
                           kw);
#endif
 DeeTuple_DecrefSymbolic(full_args);
 return result;
err_no_keywords:
 err_keywords_not_accepted(GET_TP_SELF(),kw);
 return NULL;
}


#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
#ifndef DEFINE_TYPE_OPERATORS
DEFINE_OPERATOR(DREF DeeObject *,ThisCallTuple,
               (DeeObject *__restrict self,
                DeeObject *__restrict this_arg,
                DeeObject *__restrict args)) {
 /* Check for special callback optimizations. */
 if (GET_TP_SELF() == &DeeFunction_Type)
     return DeeFunction_ThisCallTuple((DeeFunctionObject *)self,this_arg,args);
 return DeeObject_ThisCall(self,
                           this_arg,
                           DeeTuple_SIZE(args),
                           DeeTuple_ELEM(args));
}

DEFINE_OPERATOR(DREF DeeObject *,ThisCallTupleKw,
               (DeeObject *__restrict self,
                DeeObject *__restrict this_arg,
                DeeObject *__restrict args,
                DeeObject *kw)) {
 /* Check for special callback optimizations. */
 if (GET_TP_SELF() == &DeeFunction_Type)
     return DeeFunction_ThisCallTupleKw((DeeFunctionObject *)self,this_arg,args,kw);
 return DeeObject_ThisCallKw(self,
                             this_arg,
                             DeeTuple_SIZE(args),
                             DeeTuple_ELEM(args),
                             kw);
}
#endif /* !DEFINE_TYPE_OPERATORS */
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */


#ifndef DEFINE_TYPE_OPERATORS
#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallPack,12),
                    ASSEMBLY_NAME(DeeObject_Call,12));
#else
PUBLIC DREF DeeObject *DCALL
DeeObject_VCallPack(DeeObject *__restrict self,
                    size_t argc, va_list args) {
 return DeeObject_Call(self,argc,(DeeObject **)args);
}
#endif
#else
PUBLIC DREF DeeObject *DCALL
DeeObject_VCallPack(DeeObject *__restrict self,
                    size_t argc, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VPackSymbolic(argc,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_Call(self,argc,DeeTuple_ELEM(args_tuple));
 DeeTuple_DecrefSymbolic(args_tuple);
 return result;
}
#endif
PUBLIC DREF DeeObject *DCALL
DeeObject_VCallf(DeeObject *__restrict self,
                 char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VNewf(format,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_Call(self,
                         DeeTuple_SIZE(args_tuple),
                         DeeTuple_ELEM(args_tuple));
 Dee_Decref(args_tuple);
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_VThisCallf(DeeObject *__restrict self,
                     DeeObject *__restrict this_arg,
                     char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VNewf(format,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_ThisCall(self,this_arg,
                             DeeTuple_SIZE(args_tuple),
                             DeeTuple_ELEM(args_tuple));
 Dee_Decref(args_tuple);
 return result;
}

PUBLIC ATTR_SENTINEL DREF DeeObject *
DeeObject_CallPack(DeeObject *__restrict self, size_t argc, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,argc);
 result = DeeObject_VCallPack(self,argc,args);
 va_end(args);
 return result;
}
PUBLIC DREF DeeObject *
DeeObject_Callf(DeeObject *__restrict self,
                char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeObject_VCallf(self,format,args);
 va_end(args);
 return result;
}
PUBLIC DREF DeeObject *
DeeObject_ThisCallf(DeeObject *__restrict self,
                    DeeObject *__restrict this_arg,
                    char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeObject_VThisCallf(self,this_arg,format,args);
 va_end(args);
 return result;
}
#endif /* !DEFINE_TYPE_OPERATORS */

#ifndef DEFINE_TYPE_OPERATORS
INTERN bool DCALL type_inherit_hash(DeeTypeObject *__restrict self) {
 DeeTypeObject *base = DeeType_Base(self);
 struct type_cmp *base_cmp;
 if (!base ||
   (((base_cmp = base->tp_cmp) == NULL ||
     !base_cmp->tp_hash) &&
     !type_inherit_hash(base)))
      return false;
 base_cmp = base->tp_cmp;
 if (self->tp_cmp) {
  self->tp_cmp->tp_hash = base_cmp->tp_hash;
 } else {
  self->tp_cmp = base_cmp;
 }
 return true;
}
#endif /* !DEFINE_TYPE_OPERATORS */


DEFINE_OPERATOR(dhash_t,Hash,(DeeObject *__restrict self)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_cmp && tp_self->tp_cmp->tp_hash)
      return DeeType_INVOKE_HASH(tp_self,self);
 } while (type_inherit_hash(tp_self));
 return DeeObject_HashGeneric(self);
}

#ifndef DEFINE_TYPE_OPERATORS
PUBLIC void DCALL
DeeObject_Visit(DeeObject *__restrict self,
                dvisit_t proc, void *arg) {
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_visit) {
   if (tp_self->tp_visit == &instance_visit) {
    /* Required to prevent redundancy in class instances.
     * Without this, all instance levels would be visited more
     * than once by the number of recursive user-types, when
     * one visit (as implemented here) is already enough. */
    instance_tvisit(tp_self,self,proc,arg);
   } else {
    (*tp_self->tp_visit)(self,proc,arg);
   }
  }
 } while ((tp_self = DeeType_Base(tp_self)) != NULL);
}
PUBLIC void DCALL
DeeObject_VisitAll(DeeObject *__restrict self,
                   dvisit_t proc, void *arg) {
 DeeObject_Visit(self,proc,arg);
 /* Only visit heap-allocated types. */
#if 1
 if (Dee_TYPE(self)->tp_flags & TP_FHEAP)
    (*proc)((DeeObject *)Dee_TYPE(self),arg);
#else
 if (Dee_TYPE(self)->tp_flags & TP_FHEAP)
     DeeObject_Visit((DeeObject *)Dee_TYPE(self),proc,arg);
#endif
}

PUBLIC void DCALL
DeeObject_Clear(DeeObject *__restrict self) {
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_gc && tp_self->tp_gc->tp_clear) {
   if (tp_self->tp_gc->tp_clear == &instance_clear) {
    /* Same deal as with visit above: Reduce
     * overhead from recursive redundancies. */
    instance_tclear(tp_self,self);
   } else {
    (*tp_self->tp_gc->tp_clear)(self);
   }
  }
 } while ((tp_self = DeeType_Base(tp_self)) != NULL);
}
PUBLIC void DCALL
DeeObject_PClear(DeeObject *__restrict self,
                 unsigned int gc_priority) {
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 if unlikely(gc_priority == GC_PRIORITY_LATE) {
  DeeObject_Clear(self);
  return;
 }
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_gc &&
      tp_self->tp_gc->tp_pclear) {
   if (tp_self->tp_gc->tp_pclear == &instance_pclear) {
    /* Same deal as with visit above: Reduce
     * overhead from recursive redundancies. */
    instance_tpclear(tp_self,self,gc_priority);
   } else {
    (*tp_self->tp_gc->tp_pclear)(self,gc_priority);
   }
  }
 } while ((tp_self = DeeType_Base(tp_self)) != NULL);
}
#endif /* !DEFINE_TYPE_OPERATORS */


#ifndef DEFINE_TYPE_OPERATORS
INTERN bool DCALL type_inherit_int(DeeTypeObject *__restrict self) {
 DeeTypeObject *base = DeeType_Base(self);
 struct type_math *base_math;
 if (!base ||
   (((base_math = base->tp_math) == NULL ||
    (!base_math->tp_int && !base_math->tp_int32 &&
     !base_math->tp_int64 && !base_math->tp_double)) &&
     !type_inherit_int(base)))
     return false;
 DEE_DPRINTF("Inherit `operator int' from %q into %q\n",
             base->tp_name,self->tp_name);
 base_math = base->tp_math;
 if (!self->tp_math) {
  self->tp_math = base_math;
 } else {
  self->tp_math->tp_int32  = base_math->tp_int32;
  self->tp_math->tp_int64  = base_math->tp_int64;
  self->tp_math->tp_double = base_math->tp_double;
  self->tp_math->tp_int    = base_math->tp_int;
 }
 return true;
}
#endif /* !DEFINE_TYPE_OPERATORS */



DEFINE_OPERATOR(int,GetInt32,
               (DeeObject *__restrict self,
                int32_t *__restrict result)) {
 int error;
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int32)
       return DeeType_INVOKE_INT32(tp_self,self,result);
   if (tp_self->tp_math->tp_int64) {
    int64_t val64;
    error = DeeType_INVOKE_INT64(tp_self,self,&val64);
    if unlikely(error < 0) return -1;
    if (error == INT_SIGNED) {
     if unlikely(val64 < INT32_MIN || val64 > INT32_MAX) {
      if (val64 > 0) {
       *result = (int32_t)((uint32_t)(uint64_t)val64);
       return INT_UNSIGNED;
      }
      if (tp_self->tp_flags&TP_FTRUNCATE) {
       *result = (int32_t)val64;
       return val64 < INT32_MIN ? INT_SIGNED : INT_UNSIGNED;
      }
      return err_integer_overflow(self,32,val64 > 0);
     }
    } else {
     if unlikely((uint64_t)val64 > UINT32_MAX) {
      if (tp_self->tp_flags&TP_FTRUNCATE) {
       *result = (uint32_t)val64;
       return INT_UNSIGNED;
      }
      return err_integer_overflow(self,32,true);
     }
    }
    *result = (int32_t)(uint64_t)val64;
    return error;
   }
   if (tp_self->tp_math->tp_int) {
    /* Cast to integer, then read its value. */
    DREF DeeObject *intob;
    intob = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!intob) return -1;
    error = DeeInt_As32(intob,result);
    Dee_Decref(intob);
    return error;
   }
   if (tp_self->tp_math->tp_double) {
    double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error < 0) return -1;
    if unlikely(resflt < INT32_MIN || resflt > UINT32_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      if (resflt < 0) {
       *result = (int32_t)resflt;
       return INT_SIGNED;
      } else {
       *result = (uint32_t)resflt;
       return INT_UNSIGNED;
      }
     }
     return err_integer_overflow(self,32,resflt > 0);
    }
    if (resflt < 0) { *result = (int32_t)resflt; return INT_SIGNED; }
    *result = (int32_t)(uint32_t)resflt;
    return INT_UNSIGNED;
   }
  }
 } while (type_inherit_int(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_INT);
}
DEFINE_OPERATOR(int,GetInt64,
               (DeeObject *__restrict self,
                int64_t *__restrict result)) {
 int error;
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int64)
       return DeeType_INVOKE_INT64(tp_self,self,result);
   if (tp_self->tp_math->tp_int32) {
    int32_t val32;
    error = DeeType_INVOKE_INT32(tp_self,self,&val32);
    if unlikely(error < 0) return -1;
    if (error == INT_SIGNED) {
     *result = (int64_t)val32;
    } else {
     *result = (int64_t)(uint64_t)(uint32_t)val32;
    }
    return error;
   }
   if (tp_self->tp_math->tp_int) {
    /* Cast to integer, then read its value. */
    DREF DeeObject *intob;
    intob = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!intob) return -1;
    error = DeeInt_As64(intob,result);
    Dee_Decref(intob);
    return error;
   }
   if (tp_self->tp_math->tp_double) {
    double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error) return -1;
    if unlikely(resflt < INT64_MIN || resflt > UINT64_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      if (resflt < 0) {
       *result = (int64_t)resflt;
       return INT_SIGNED;
      } else {
       *result = (uint64_t)resflt;
       return INT_UNSIGNED;
      }
     }
     return err_integer_overflow(self,64,resflt > 0);
    }
    if (resflt < 0) { *result = (int64_t)resflt; return INT_SIGNED; }
    *result = (int64_t)(uint64_t)resflt;
    return INT_UNSIGNED;
   }
  }
 } while (type_inherit_int(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_INT);
}

#ifndef DEFINE_TYPE_OPERATORS
DEFINE_OPERATOR(int,GetInt128,
               (DeeObject *__restrict self,
                dint128_t *__restrict result)) {
 int error;
 LOAD_TP_SELF;
 if (tp_self == &DeeInt_Type)
     return DeeInt_As128(self,result);
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int) {
    /* Cast to integer, then read its value. */
    DREF DeeObject *intob;
    intob = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!intob) return -1;
    error = DeeInt_As128(intob,result);
    Dee_Decref(intob);
    return error;
   }
   if (tp_self->tp_math->tp_int64) {
    DUINT128_GETS64(*result)[DEE_INT128_MS64] = 0;
    error = DeeType_INVOKE_INT64(tp_self,self,&DUINT128_GETS64(*result)[DEE_INT128_LS64]);
    if (error == INT_SIGNED && DUINT128_GETS64(*result)[DEE_INT128_LS64] < 0)
        DUINT128_GETS64(*result)[DEE_INT128_MS64] = -1;
    return error;
   }
   if (tp_self->tp_math->tp_int32) {
    int32_t val32;
    DUINT128_GETS64(*result)[DEE_INT128_MS64] = 0;
    error = DeeType_INVOKE_INT32(tp_self,self,&val32);
    if unlikely(error < 0) return -1;
    if (error == INT_SIGNED) {
     if (val32 < 0)
         DUINT128_GETS64(*result)[DEE_INT128_MS64] = -1;
     DUINT128_GETS64(*result)[DEE_INT128_LS64] = (int64_t)val32;
    } else {
     DUINT128_GETS64(*result)[DEE_INT128_LS64] = (int64_t)(uint64_t)(uint32_t)val32;
    }
    return error;
   }
   if (tp_self->tp_math->tp_double) {
    double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error < 0) return -1;
    DUINT128_GETS64(*result)[DEE_INT128_MS64] = 0;
    if unlikely(resflt < INT64_MIN || resflt > UINT64_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      if (resflt < 0) {
       DUINT128_GETS64(*result)[DEE_INT128_LS64] = (int64_t)resflt;
       DUINT128_GETS64(*result)[DEE_INT128_MS64] = -1;
       return INT_SIGNED;
      } else {
       DUINT128_GETS64(*result)[DEE_INT128_LS64] = (uint64_t)resflt;
       return INT_UNSIGNED;
      }
     }
     return err_integer_overflow(self,64,resflt > 0);
    }
    if (resflt < 0) {
     DUINT128_GETS64(*result)[DEE_INT128_LS64] = (int64_t)resflt;
     DUINT128_GETS64(*result)[DEE_INT128_MS64] = -1;
     return INT_SIGNED;
    }
    DUINT128_GETS64(*result)[DEE_INT128_LS64] = (int64_t)(uint64_t)resflt;
    return INT_UNSIGNED;
   }
  }
 } while (type_inherit_int(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_INT);
}
#endif


#ifndef DEFINE_TYPE_OPERATORS
PUBLIC int (DCALL DeeObject_AsUInt32)(DeeObject *__restrict self,
                                      uint32_t *__restrict result) {
 int error;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int32) {
    error = DeeType_INVOKE_INT32(tp_self,self,(int32_t *)result);
    if unlikely(error < 0) return -1;
    if unlikely(error == INT_SIGNED && *(int32_t *)result < 0) {
     if (tp_self->tp_flags&TP_FTRUNCATE) return 0;
neg_overflow:
     return err_integer_overflow(self,32,0);
    }
    return 0;
   }
   if (tp_self->tp_math->tp_int64) {
    int64_t val64;
    error = DeeType_INVOKE_INT64(tp_self,self,&val64);
    if (error < 0) return -1;
    if unlikely(error == INT_SIGNED && val64 < 0) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
return_trunc64: *result = (uint32_t)val64; return 0;
     }
     goto neg_overflow;
    }
    if unlikely((uint64_t)val64 > (uint64_t)UINT32_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) goto return_trunc64;
     return err_integer_overflow(self,32,1);
    }
    *result = (int32_t)(uint64_t)val64;
    return 0;
   }
   if (tp_self->tp_math->tp_int) {
    /* Cast to integer, then read its value. */
    DREF DeeObject *intob;
    intob = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!intob) return -1;
    error = DeeInt_AsU32(intob,result);
    Dee_Decref(intob);
    return error;
   }
   if (tp_self->tp_math->tp_double) {
    double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error < 0) return -1;
    if unlikely(resflt < 0 || resflt > UINT32_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      *result = (uint32_t)resflt;
      return 0;
     }
     return err_integer_overflow(self,32,resflt > 0);
    }
    *result = (uint32_t)resflt;
    return 0;
   }
  }
 } while (type_inherit_int(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_INT);
}

PUBLIC int (DCALL DeeObject_AsInt32)(DeeObject *__restrict self,
                                     int32_t *__restrict result) {
 int error;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int32) {
    error = DeeType_INVOKE_INT32(tp_self,self,result);
    if unlikely(error < 0) return -1;
    if unlikely(error == INT_UNSIGNED && (uint32_t)*result > INT32_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) return 0;
     return err_integer_overflow(self,32,1);
    }
    return 0;
   }
   if (tp_self->tp_math->tp_int64) {
    int64_t val64;
    error = DeeType_INVOKE_INT64(tp_self,self,&val64);
    if unlikely(error < 0) return -1;
    if (error == INT_SIGNED) {
     if unlikely(val64 < INT32_MIN || val64 > INT32_MAX) {
      if (tp_self->tp_flags&TP_FTRUNCATE) {
return_trunc64:
       *result = (int32_t)val64;
       return 0;
      }
      return err_integer_overflow(self,32,val64 > 0);
     }
     *result = (int32_t)val64;
    } else {
     if unlikely((uint64_t)val64 > INT32_MAX) {
      if (tp_self->tp_flags&TP_FTRUNCATE) goto return_trunc64;
      return err_integer_overflow(self,32,1);
     }
     *result = (int32_t)(uint64_t)val64;
    }
    return 0;
   }
   if (tp_self->tp_math->tp_int) {
    /* Cast to integer, then read its value. */
    DREF DeeObject *intob;
    intob = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!intob) return -1;
    error = DeeInt_AsS32(intob,result);
    Dee_Decref(intob);
    return error;
   }
   if (tp_self->tp_math->tp_double) {
    double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error < 0) return -1;
    if unlikely(resflt < INT32_MIN || resflt > INT32_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      *result = (int32_t)resflt;
      return 0;
     }
     return err_integer_overflow(self,32,resflt > 0);
    }
    *result = (int32_t)resflt;
    return 0;
   }
  }
 } while (type_inherit_int(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_INT);
}

PUBLIC int (DCALL DeeObject_AsUInt64)(DeeObject *__restrict self,
                                      uint64_t *__restrict result) {
 int error;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int64) {
    error = DeeType_INVOKE_INT64(tp_self,self,(int64_t *)result);
    if unlikely(error < 0) return -1;
    if unlikely(error == INT_SIGNED && *(int64_t *)result < 0) {
     if (tp_self->tp_flags&TP_FTRUNCATE) return 0;
neg_overflow:
     return err_integer_overflow(self,64,false);
    }
    return 0;
   }
   if (tp_self->tp_math->tp_int32) {
    int32_t val32;
    error = DeeType_INVOKE_INT32(tp_self,self,&val32);
    if unlikely(error < 0) goto err;
    if unlikely(error == INT_SIGNED && val32 < 0) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      *result = (uint64_t)(int64_t)val32;
      return 0;
     }
     goto neg_overflow;
    }
    *result = (uint64_t)(uint32_t)val32;
    return 0;
   }
   if (tp_self->tp_math->tp_int) {
    /* Cast to integer, then read its value. */
    DREF DeeObject *intob;
    intob = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!intob) goto err;
    error = DeeInt_AsU64(intob,result);
    Dee_Decref(intob);
    return error;
   }
   if (tp_self->tp_math->tp_double) {
    double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error) goto err;
    if unlikely(resflt < 0 || resflt > UINT64_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      *result = (uint64_t)resflt;
      return 0;
     }
     return err_integer_overflow(self,64,resflt > 0);
    }
    *result = (int64_t)resflt;
    return 0;
   }
  }
 } while (type_inherit_int(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_INT);
err:
 return -1;
}

PUBLIC int (DCALL DeeObject_AsInt64)(DeeObject *__restrict self,
                                     int64_t *__restrict result) {
 int error;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int64) {
    error = DeeType_INVOKE_INT64(tp_self,self,result);
    if unlikely(error < 0) return -1;
    if unlikely(error == INT_UNSIGNED && (uint64_t)*result > INT64_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) return 0;
     return err_integer_overflow(self,64,true);
    }
    return 0;
   }
   if (tp_self->tp_math->tp_int32) {
    int32_t val32;
    error = DeeType_INVOKE_INT32(tp_self,self,&val32);
    if (error < 0) return -1;
    if (error == INT_SIGNED)
         *result = (int64_t)val32;
    else *result = (int64_t)((uint64_t)(uint32_t)val32);
    return 0;
   }
   if (tp_self->tp_math->tp_int) {
    /* Cast to integer, then read its value. */
    DREF DeeObject *intob;
    intob = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!intob) return -1;
    error = DeeInt_AsS64(intob,result);
    Dee_Decref(intob);
    return error;
   }
   if (tp_self->tp_math->tp_double) {
    double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error < 0) return -1;
    if unlikely(resflt < INT64_MIN || resflt > INT64_MAX) {
     if (tp_self->tp_flags&TP_FTRUNCATE) {
      *result = (int64_t)resflt;
      return 0;
     }
     return err_integer_overflow(self,64,resflt > 0);
    }
    *result = (int64_t)resflt;
    return 0;
   }
  }
 } while (type_inherit_int(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_INT);
}
PUBLIC int (DCALL DeeObject_AsInt128)(DeeObject *__restrict self,
                                      dint128_t *__restrict result) {
 int error = DeeObject_GetInt128(self,result);
 if (error == INT_UNSIGNED && DSINT128_ISNEG(*result))
     return err_integer_overflow(self,128,true);
 return 0;
}
PUBLIC int (DCALL DeeObject_AsUInt128)(DeeObject *__restrict self,
                                       duint128_t *__restrict result) {
 int error = DeeObject_GetInt128(self,(dint128_t *)result);
 if (error == INT_SIGNED && DSINT128_ISNEG(*result))
     return err_integer_overflow(self,128,false);
 return 0;
}
#endif /* !DEFINE_TYPE_OPERATORS */

DEFINE_OPERATOR(int,AsDouble,
               (DeeObject *__restrict self,
                double *__restrict result)) {
 union {
     int32_t res32;
     int64_t res64;
 } res;
 int error;
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_double)
       return DeeType_INVOKE_DOUBLE(tp_self,self,result);
   if (tp_self->tp_math->tp_int64) {
    error = DeeType_INVOKE_INT64(tp_self,self,&res.res64);
    if (error == INT_UNSIGNED)
         *result = (double)(uint64_t)res.res64;
    else *result = (double)res.res64;
    return error < 0 ? -1 : 0;
   }
   if (tp_self->tp_math->tp_int32) {
    error = DeeType_INVOKE_INT32(tp_self,self,&res.res32);
    if (error == INT_UNSIGNED)
         *result = (double)(uint32_t)res.res32;
    else *result = (double)res.res32;
    return error < 0 ? -1 : 0;
   }
   if (tp_self->tp_math->tp_int) {
    DREF DeeObject *temp_int;
    temp_int = DeeType_INVOKE_INT(tp_self,self);
    if unlikely(!temp_int) goto err;
    error = DeeInt_As64(temp_int,&res.res64);
    Dee_Decref(temp_int);
    if (error == INT_UNSIGNED)
         *result = (double)(uint64_t)res.res64;
    else *result = (double)res.res64;
    return error < 0 ? -1 : 0;
   }
  }
 } while (type_inherit_int(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_FLOAT);
err:
 return -1;
}


#ifndef DEFINE_TYPE_OPERATORS
INTERN DeeTypeObject *DCALL
type_get_int_caster(DeeTypeObject *__restrict start) {
 while (!DeeType_HasOperator(start,OPERATOR_INT))
         start = DeeType_Base(start);
 return start;
}

PUBLIC int DCALL
DeeObject_GetInt8(DeeObject *__restrict self,
                  int8_t *__restrict result) {
 int32_t val32;
 int error = DeeObject_GetInt32(self,&val32);
 if unlikely(error < 0) return -1;
 if (error == INT_SIGNED) {
  if (val32 < INT8_MIN || val32 > INT8_MAX) {
   if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE) {
    *result = (int8_t)val32;
    return *result < 0 ? INT_SIGNED : INT_UNSIGNED;
   }
   return err_integer_overflow(self,8,val32 > 0);
  }
  *result = (int8_t)val32;
 } else {
  if ((uint32_t)val32 > UINT8_MAX) {
   if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE) {
    *result = (uint8_t)val32;
    return INT_UNSIGNED;
   }
   return err_integer_overflow(self,8,true);
  }
  *result = (int8_t)(uint8_t)val32;
 }
 return error;
}
PUBLIC int DCALL
DeeObject_GetInt16(DeeObject *__restrict self,
                   int16_t *__restrict result) {
 int32_t val32;
 int error = DeeObject_GetInt32(self,&val32);
 if unlikely(error < 0) return -1;
 if (error == INT_SIGNED) {
  if (val32 < INT16_MIN || val32 > INT16_MAX) {
   if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE) {
    *result = (int16_t)val32;
    return *result < 0 ? INT_SIGNED : INT_UNSIGNED;
   }
   return err_integer_overflow(self,16,val32 > 0);
  }
  *result = (int16_t)val32;
 } else {
  if ((uint32_t)val32 > UINT16_MAX) {
   if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE) {
    *result = (uint16_t)val32;
    return INT_UNSIGNED;
   }
   return err_integer_overflow(self,16,true);
  }
  *result = (int16_t)(uint16_t)val32;
 }
 return error;
}

PUBLIC int (DCALL DeeObject_AsInt8)(DeeObject *__restrict self,
                                  int8_t *__restrict result) {
 int32_t val32;
 int error = DeeObject_AsInt32(self,&val32);
 if unlikely(error < 0) return -1;
 if (val32 < INT8_MIN || val32 > INT8_MAX) {
  if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE)
      goto return_value;
  return err_integer_overflow(self,8,val32 > 0);
 }
return_value:
 *result = (int8_t)val32;
 return error;
}
PUBLIC int (DCALL DeeObject_AsInt16)(DeeObject *__restrict self,
                                   int16_t *__restrict result) {
 int32_t val32;
 int error = DeeObject_AsInt32(self,&val32);
 if unlikely(error < 0) return -1;
 if (val32 < INT16_MIN || val32 > INT16_MAX) {
  if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE)
      goto return_value;
  return err_integer_overflow(self,16,val32 > 0);
 }
return_value:
 *result = (int8_t)val32;
 return 0;
}
PUBLIC int (DCALL DeeObject_AsUInt8)(DeeObject *__restrict self,
                                   uint8_t *__restrict result) {
 uint32_t val32;
 int error = DeeObject_AsUInt32(self,&val32);
 if unlikely(error < 0) return -1;
 if (val32 > UINT8_MAX) {
  if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE)
      goto return_value;
  return err_integer_overflow(self,8,true);
 }
return_value:
 *result = (uint8_t)val32;
 return error;
}
PUBLIC int (DCALL DeeObject_AsUInt16)(DeeObject *__restrict self,
                                      uint16_t *__restrict result) {
 uint32_t val32;
 int error = DeeObject_AsUInt32(self,&val32);
 if unlikely(error < 0) return -1;
 if (val32 > UINT16_MAX) {
  if (type_get_int_caster(Dee_TYPE(self))->tp_flags&TP_FTRUNCATE)
      goto return_value;
  return err_integer_overflow(self,16,true);
 }
return_value:
 *result = (uint16_t)val32;
 return 0;
}
#endif /* !DEFINE_TYPE_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *,Int,(DeeObject *__restrict self)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_math) {
   if (tp_self->tp_math->tp_int)
       return DeeType_INVOKE_INT(tp_self,self);
   if (tp_self->tp_math->tp_int64) {
    int64_t val64; int error;
    error = DeeType_INVOKE_INT64(tp_self,self,&val64);
    if unlikely(error < 0) goto err;
    if (error == INT_SIGNED)
        return DeeInt_NewS64(val64);
    return DeeInt_NewU64((uint64_t)val64);
   }
   if (tp_self->tp_math->tp_int32) {
    int32_t val32; int error;
    error = DeeType_INVOKE_INT32(tp_self,self,&val32);
    if unlikely(error < 0) goto err;
    if (error == INT_SIGNED)
        return DeeInt_NewS32(val32);
    return DeeInt_NewU32((uint32_t)val32);
   }
   if (tp_self->tp_math->tp_double) {
    int error; double resflt;
    error = DeeType_INVOKE_DOUBLE(tp_self,self,&resflt);
    if unlikely(error < 0) return NULL;
    if (resflt < INT64_MIN || resflt > UINT64_MAX) {
     if (!(tp_self->tp_flags&TP_FTRUNCATE)) {
      err_integer_overflow(self,64,resflt > 0);
      return NULL;
     }
    }
    return DeeInt_NewS64((int64_t)resflt);
   }
  }
 } while (type_inherit_int(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_INT);
err:
 return NULL;
}



#ifndef DEFINE_TYPE_OPERATORS
#define DEFINE_TYPE_INHERIT_FUNCTION(name,opname,field) \
INTERN bool DCALL \
name(DeeTypeObject *__restrict self) { \
 struct type_math *base_math; \
 DeeTypeObject *base = DeeType_Base(self); \
 if (!base || \
   (((base_math = base->tp_math) == NULL || \
    (!base_math->tp_##field && \
     !base_math->tp_inplace_##field)) && !name(base))) \
      return false; \
 DEE_DPRINTF("Inherit `" opname "' from %q into %q\n", \
             base->tp_name,self->tp_name); \
 if (self->tp_math) { \
  self->tp_math->tp_##field         = base_math->tp_##field; \
  self->tp_math->tp_inplace_##field = base_math->tp_inplace_##field; \
 } else { \
  self->tp_math = base_math; \
 } \
 return true; \
}
#define DEFINE_TYPE_INHERIT_FUNCTION1(name,opname,field) \
INTERN bool DCALL \
name(DeeTypeObject *__restrict self) { \
 struct type_math *base_math; \
 DeeTypeObject *base = DeeType_Base(self); \
 if (!base || \
   (((base_math = base->tp_math) == NULL || \
     !base_math->tp_##field) && !name(base))) \
      return false; \
 DEE_DPRINTF("Inherit `" opname "' from %q into %q\n", \
             base->tp_name,self->tp_name); \
 base_math = base->tp_math; \
 if (self->tp_math) { \
  self->tp_math->tp_##field = base_math->tp_##field; \
 } else { \
  self->tp_math = base_math; \
 } \
 return true; \
}
DEFINE_TYPE_INHERIT_FUNCTION1(type_inherit_inv,"operator inv",inv)
DEFINE_TYPE_INHERIT_FUNCTION1(type_inherit_pos,"operator pos",pos)
DEFINE_TYPE_INHERIT_FUNCTION1(type_inherit_neg,"operator neg",neg)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_mul,"operator mul",mul)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_div,"operator div",div)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_mod,"operator mod",mod)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_shl,"operator shl",shl)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_shr,"operator shr",shr)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_and,"operator and",and)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_or, "operator or", or)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_xor,"operator xor",xor)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_pow,"operator pow",pow)

/* inc,dec,add,sub,iadd & isub are all apart of the same operator group. */
INTERN bool DCALL type_inherit_add(DeeTypeObject *__restrict self) {
 struct type_math *base_math;
 DeeTypeObject *base = DeeType_Base(self);
 if (!base ||
   (((base_math = base->tp_math) == NULL ||
    (!base_math->tp_add && !base_math->tp_inplace_add &&
     !base_math->tp_sub && !base_math->tp_inplace_sub &&
     !base_math->tp_inc && !base_math->tp_dec)) && !type_inherit_add(base)))
      return false;
 DEE_DPRINTF("Inherit `operator add' %q from into %q\n",
             base->tp_name,self->tp_name);
 base_math = base->tp_math;
 if (self->tp_math) {
  self->tp_math->tp_inc = base_math->tp_inc;
  self->tp_math->tp_dec = base_math->tp_dec;
  self->tp_math->tp_add = base_math->tp_add;
  self->tp_math->tp_sub = base_math->tp_sub;
  self->tp_math->tp_inplace_add = base_math->tp_inplace_add;
  self->tp_math->tp_inplace_sub = base_math->tp_inplace_sub;
 } else {
  self->tp_math = base_math;
 }
 return true;
}
#undef DEFINE_TYPE_INHERIT_FUNCTION1
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif


#ifdef DEFINE_TYPE_OPERATORS
#define COPY_SELF() DeeObject_TCopy(tp_self,self)
#else
#define COPY_SELF() DeeObject_Copy(self)
#endif

#define DEFINE_MATH_OPERATOR1(name,xxx,operator_name,invoke) \
DEFINE_OPERATOR(DREF DeeObject *,name,(DeeObject *__restrict self)) { \
 LOAD_TP_SELF; \
 do { \
  if (tp_self->tp_math && tp_self->tp_math->tp_##xxx) \
      return invoke(tp_self,self); \
 } while (type_inherit_##xxx(tp_self)); \
 err_unimplemented_operator(tp_self,operator_name); \
 return NULL; \
}
#define DEFINE_MATH_OPERATOR2(name,xxx,operator_name,invoke,invoke_inplace) \
DEFINE_OPERATOR(DREF DeeObject *,name,\
               (DeeObject *__restrict self, \
                DeeObject *__restrict some_object)) { \
 LOAD_TP_SELF; \
 ASSERT_OBJECT(some_object); \
 do { \
  struct type_math *math; \
  if ((math = tp_self->tp_math) != NULL) { \
   if (tp_self->tp_math->tp_##xxx) \
       return invoke(tp_self,self,some_object); \
   if (tp_self->tp_math->tp_inplace_##xxx) { \
    int error; \
    if unlikely((self = COPY_SELF()) == NULL) goto err; \
    error = invoke_inplace(tp_self,(DeeObject **)&self,some_object); \
    if unlikely(error) Dee_Clear(self); \
    return self; \
   } \
  } \
 } while (type_inherit_##xxx(tp_self)); \
 err_unimplemented_operator(tp_self,operator_name); \
err: \
 return NULL; \
}
DEFINE_MATH_OPERATOR1(Inv,inv,OPERATOR_INV,DeeType_INVOKE_INV)
DEFINE_MATH_OPERATOR1(Pos,pos,OPERATOR_POS,DeeType_INVOKE_POS)
DEFINE_MATH_OPERATOR1(Neg,neg,OPERATOR_NEG,DeeType_INVOKE_NEG)
DEFINE_MATH_OPERATOR2(Mul,mul,OPERATOR_MUL,DeeType_INVOKE_MUL,DeeType_INVOKE_IMUL)
DEFINE_MATH_OPERATOR2(Div,div,OPERATOR_DIV,DeeType_INVOKE_DIV,DeeType_INVOKE_IDIV)
DEFINE_MATH_OPERATOR2(Mod,mod,OPERATOR_MOD,DeeType_INVOKE_MOD,DeeType_INVOKE_IMOD)
DEFINE_MATH_OPERATOR2(Shl,shl,OPERATOR_SHL,DeeType_INVOKE_SHL,DeeType_INVOKE_ISHL)
DEFINE_MATH_OPERATOR2(Shr,shr,OPERATOR_SHR,DeeType_INVOKE_SHR,DeeType_INVOKE_ISHR)
DEFINE_MATH_OPERATOR2(And,and,OPERATOR_AND,DeeType_INVOKE_AND,DeeType_INVOKE_IAND)
DEFINE_MATH_OPERATOR2(Or, or, OPERATOR_OR, DeeType_INVOKE_OR, DeeType_INVOKE_IOR)
DEFINE_MATH_OPERATOR2(Xor,xor,OPERATOR_XOR,DeeType_INVOKE_XOR,DeeType_INVOKE_IXOR)
DEFINE_MATH_OPERATOR2(Pow,pow,OPERATOR_POW,DeeType_INVOKE_POW,DeeType_INVOKE_IPOW)

DEFINE_OPERATOR(DREF DeeObject *,Add,
               (DeeObject *__restrict self,
                DeeObject *__restrict some_object)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(some_object);
 do {
  struct type_math *math = tp_self->tp_math;
  if (math) {
   if (math->tp_add)
       return DeeType_INVOKE_ADD(tp_self,self,some_object);
   if (math->tp_inplace_add) {
    if unlikely((self = COPY_SELF()) == NULL) goto err;
    if unlikely(DeeType_INVOKE_IADD(tp_self,(DeeObject **)&self,some_object))
       goto err_self;
    return self;
   }
   if (math->tp_sub) break;         /* XXX: `x + y --> x - (-y)' */
   if (math->tp_inplace_sub) break; /* XXX: `x += y --> x -= (-y)' */
   if (math->tp_inc) break;
   if (math->tp_dec) break;
  }
 } while (type_inherit_add(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_ADD);
err:
 return NULL;
err_self:
 Dee_Decref(self);
 goto err;
}
DEFINE_OPERATOR(DREF DeeObject *,Sub,
               (DeeObject *__restrict self,
                DeeObject *__restrict some_object)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(some_object);
 do {
  struct type_math *math = tp_self->tp_math;
  if (math) {
   if (math->tp_sub)
       return DeeType_INVOKE_SUB(tp_self,self,some_object);
   if (math->tp_inplace_sub) {
    if unlikely((self = COPY_SELF()) == NULL) goto err;
    if unlikely(DeeType_INVOKE_ISUB(tp_self,(DeeObject **)&self,some_object))
       goto err_self;
    return self;
   }
   if (math->tp_add) break;         /* XXX: `x - y --> x + (-y)' */
   if (math->tp_inplace_add) break; /* XXX: `x -= y --> x += (-y)' */
   if (math->tp_inc) break;
   if (math->tp_dec) break;
  }
 } while (type_inherit_add(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_SUB);
err:
 return NULL;
err_self:
 Dee_Decref(self);
 goto err;
}

#ifndef DEFINE_TYPE_OPERATORS
PUBLIC DREF DeeObject *DCALL
DeeObject_AddS8(DeeObject *__restrict self, int8_t val) {
 DREF DeeObject *val_ob,*result;
 /* Optimization for `int' */
 if (DeeInt_Check(self))
     return DeeInt_AddSDigit((DeeIntObject *)self,val);
 val_ob = DeeInt_NewS8(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Add(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_SubS8(DeeObject *__restrict self, int8_t val) {
 DREF DeeObject *val_ob,*result;
 /* Optimization for `int' */
 if (DeeInt_Check(self))
     return DeeInt_SubSDigit((DeeIntObject *)self,val);
 val_ob = DeeInt_NewS8(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Sub(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_AddInt(DeeObject *__restrict self, uint32_t val) {
 DREF DeeObject *val_ob,*result;
 /* Optimization for `int' */
 if (DeeInt_Check(self))
     return DeeInt_AddU32((DeeIntObject *)self,val);
 val_ob = DeeInt_NewU32(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Add(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_SubInt(DeeObject *__restrict self, uint32_t val) {
 DREF DeeObject *val_ob,*result;
 /* Optimization for `int' */
 if (DeeInt_Check(self))
     return DeeInt_SubU32((DeeIntObject *)self,val);
 val_ob = DeeInt_NewU32(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Sub(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_MulInt(DeeObject *__restrict self, int8_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewS8(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Mul(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_DivInt(DeeObject *__restrict self, int8_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewS8(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Div(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_ModInt(DeeObject *__restrict self, int8_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewS8(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Mod(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_ShlInt(DeeObject *__restrict self, uint8_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewU8(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Shl(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_ShrInt(DeeObject *__restrict self, uint8_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewU8(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Shr(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_AndInt(DeeObject *__restrict self, uint32_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewU32(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_And(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_OrInt(DeeObject *__restrict self, uint32_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewU32(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Or(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_XorInt(DeeObject *__restrict self, uint32_t val) {
 DREF DeeObject *val_ob,*result;
 /* TODO: Optimization for `int' */
 val_ob = DeeInt_NewU32(val);
 if unlikely(!val_ob) goto err;
 result = DeeObject_Xor(self,val_ob);
 Dee_Decref(val_ob);
 return result;
err:
 return NULL;
}
#endif /* !DEFINE_TYPE_OPERATORS */

#undef DEFINE_MATH_OPERATOR2
#undef DEFINE_MATH_OPERATOR1
#undef COPY_SELF


DEFINE_OPERATOR(int,Inc,(DeeObject **__restrict pself)) {
 LOAD_TP_SELFP;
 do {
  struct type_math *math;
  if ((math = tp_self->tp_math) != NULL) {
   if (math->tp_inc)
       return DeeType_INVOKE_INC(tp_self,pself);
   if (math->tp_inplace_add)
       return DeeType_INVOKE_IADD(tp_self,pself,&DeeInt_One);
   if (math->tp_add) {
    DREF DeeObject *temp;
    temp = DeeType_INVOKE_ADD(tp_self,*pself,&DeeInt_One);
    if unlikely(!temp) goto err;
    Dee_Decref(*pself);
    *pself = temp;
    return 0;
   }
   if (math->tp_inplace_sub)
       return DeeType_INVOKE_ISUB(tp_self,pself,&DeeInt_MinusOne);
   if (math->tp_sub) {
    DREF DeeObject *temp;
    temp = DeeType_INVOKE_SUB(tp_self,*pself,&DeeInt_MinusOne);
    if unlikely(!temp) goto err;
    Dee_Decref(*pself);
    *pself = temp;
    return 0;
   }
   if (math->tp_dec)
       break;
  }
 } while (type_inherit_add(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_INC);
err:
 return -1;
}

DEFINE_OPERATOR(int,Dec,(DeeObject **__restrict pself)) {
 LOAD_TP_SELFP;
 do {
  struct type_math *math;
  if ((math = tp_self->tp_math) != NULL) {
   if (math->tp_dec)
       return DeeType_INVOKE_DEC(tp_self,pself);
   if (math->tp_inplace_sub)
       return DeeType_INVOKE_ISUB(tp_self,pself,&DeeInt_One);
   if (math->tp_sub) {
    DREF DeeObject *temp;
    temp = DeeType_INVOKE_SUB(tp_self,*pself,&DeeInt_One);
    if unlikely(!temp) goto err;
    Dee_Decref(*pself);
    *pself = temp;
    return 0;
   }
   if (math->tp_inplace_add)
       return DeeType_INVOKE_IADD(tp_self,pself,&DeeInt_MinusOne);
   if (math->tp_add) {
    DREF DeeObject *temp;
    temp = DeeType_INVOKE_ADD(tp_self,*pself,&DeeInt_MinusOne);
    if unlikely(!temp) goto err;
    Dee_Decref(*pself);
    *pself = temp;
    return 0;
   }
   if (math->tp_inc)
       break;
  }
 } while (type_inherit_add(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_DEC);
err:
 return -1;
}

#define DEFINE_MATH_INPLACE_OPERATOR2(name,xxx,operator_name,invoke,invoke_inplace) \
DEFINE_OPERATOR(int,name,(DeeObject **__restrict pself, \
                          DeeObject *__restrict some_object)) { \
 LOAD_TP_SELFP; \
 ASSERT_OBJECT(some_object); \
 do { \
  if (tp_self->tp_math) { \
   if (tp_self->tp_math->tp_inplace_##xxx) \
   { \
    return invoke_inplace(tp_self,pself,some_object); \
   } \
   if (tp_self->tp_math->tp_##xxx) \
   { \
    DREF DeeObject *temp; \
    temp = invoke(tp_self,*pself,some_object); \
    if unlikely(!temp) goto err; \
    Dee_Decref(*pself); \
    *pself = temp; \
    return 0; \
   } \
  } \
 } while (type_inherit_##xxx(tp_self)); \
 err_unimplemented_operator(tp_self,operator_name); \
err: \
 return -1; \
}
DEFINE_MATH_INPLACE_OPERATOR2(InplaceMul,mul,OPERATOR_INPLACE_MUL,DeeType_INVOKE_MUL,DeeType_INVOKE_IMUL)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceDiv,div,OPERATOR_INPLACE_DIV,DeeType_INVOKE_DIV,DeeType_INVOKE_IDIV)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceMod,mod,OPERATOR_INPLACE_MOD,DeeType_INVOKE_MOD,DeeType_INVOKE_IMOD)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceShl,shl,OPERATOR_INPLACE_SHL,DeeType_INVOKE_SHL,DeeType_INVOKE_ISHL)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceShr,shr,OPERATOR_INPLACE_SHR,DeeType_INVOKE_SHR,DeeType_INVOKE_ISHR)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceAnd,and,OPERATOR_INPLACE_AND,DeeType_INVOKE_AND,DeeType_INVOKE_IAND)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceOr, or, OPERATOR_INPLACE_OR, DeeType_INVOKE_OR, DeeType_INVOKE_IOR)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceXor,xor,OPERATOR_INPLACE_XOR,DeeType_INVOKE_XOR,DeeType_INVOKE_IXOR)
DEFINE_MATH_INPLACE_OPERATOR2(InplacePow,pow,OPERATOR_INPLACE_POW,DeeType_INVOKE_POW,DeeType_INVOKE_IPOW)

DEFINE_OPERATOR(int,InplaceAdd,
               (DeeObject **__restrict pself,
                DeeObject *__restrict some_object)) {
 LOAD_TP_SELFP;
 ASSERT_OBJECT(some_object);
 do {
  struct type_math *math = tp_self->tp_math;
  if (math) {
   if (math->tp_inplace_add)
       return DeeType_INVOKE_IADD(tp_self,pself,some_object);
   if (math->tp_add) {
    DREF DeeObject *temp;
    temp = DeeType_INVOKE_ADD(tp_self,*pself,some_object);
    if unlikely(!temp) goto err;
    Dee_Decref(*pself);
    *pself = temp;
    return 0;
   }
   if (math->tp_sub) break;         /* XXX: `x - y --> x + (-y)' */
   if (math->tp_inplace_sub) break; /* XXX: `x -= y --> x += (-y)' */
   if (math->tp_inc) break;
   if (math->tp_dec) break;
  }
 } while (type_inherit_add(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_INPLACE_ADD);
err:
 return -1;
}
DEFINE_OPERATOR(int,InplaceSub,
               (DeeObject **__restrict pself,
                DeeObject *__restrict some_object)) {
 LOAD_TP_SELFP;
 ASSERT_OBJECT(some_object);
 do {
  struct type_math *math = tp_self->tp_math;
  if (math) {
   if (math->tp_inplace_sub)
       return DeeType_INVOKE_ISUB(tp_self,pself,some_object);
   if (math->tp_sub) {
    DREF DeeObject *temp;
    temp = DeeType_INVOKE_SUB(tp_self,*pself,some_object);
    if unlikely(!temp) goto err;
    Dee_Decref(*pself);
    *pself = temp;
    return 0;
   }
   if (math->tp_add) break;         /* XXX: `x + y --> x - (-y)' */
   if (math->tp_inplace_add) break; /* XXX: `x += y --> x -= (-y)' */
   if (math->tp_inc) break;
   if (math->tp_dec) break;
  }
 } while (type_inherit_add(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_INPLACE_ADD);
err:
 return -1;
}

#undef DEFINE_MATH_INPLACE_OPERATOR2

#ifndef DEFINE_TYPE_OPERATORS
#define DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceXXX,reg,DeeInt_NewXXX,intX_t,operator_name) \
PUBLIC int DCALL \
DeeObject_InplaceXXX(DREF DeeObject **__restrict pself, intX_t val) { \
 DREF DeeObject *temp; int result; \
 temp = DeeInt_NewXXX(val); \
 if unlikely(!temp) goto err; \
 result = reg(pself,temp); \
 Dee_Decref(temp); \
 return result; \
err: \
 return -1; \
}
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAddS8,DeeObject_InplaceAdd,DeeInt_NewS8,int8_t,OPERATOR_INPLACE_ADD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceSubS8,DeeObject_InplaceSub,DeeInt_NewS8,int8_t,OPERATOR_INPLACE_SUB)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAddInt,DeeObject_InplaceAdd,DeeInt_NewU32,uint32_t,OPERATOR_INPLACE_ADD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceSubInt,DeeObject_InplaceSub,DeeInt_NewU32,uint32_t,OPERATOR_INPLACE_SUB)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceMulInt,DeeObject_InplaceMul,DeeInt_NewS8,int8_t,OPERATOR_INPLACE_MUL)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceDivInt,DeeObject_InplaceDiv,DeeInt_NewS8,int8_t,OPERATOR_INPLACE_DIV)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceModInt,DeeObject_InplaceMod,DeeInt_NewS8,int8_t,OPERATOR_INPLACE_MOD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceShlInt,DeeObject_InplaceShl,DeeInt_NewU8,uint8_t,OPERATOR_INPLACE_SHL)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceShrInt,DeeObject_InplaceShr,DeeInt_NewU8,uint8_t,OPERATOR_INPLACE_SHR)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAndInt,DeeObject_InplaceAnd,DeeInt_NewU32,uint32_t,OPERATOR_INPLACE_AND)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceOrInt,DeeObject_InplaceOr,DeeInt_NewU32,uint32_t,OPERATOR_INPLACE_OR)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceXorInt,DeeObject_InplaceXor,DeeInt_NewU32,uint32_t,OPERATOR_INPLACE_XOR)
#undef DEFINE_MATH_INPLACE_INT_OPERATOR
#endif /* !DEFINE_TYPE_OPERATORS */


LOCAL DREF DeeObject *DCALL
invoke_not(DREF DeeObject *ob) {
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



#ifndef DEFINE_TYPE_OPERATORS
INTERN bool DCALL type_inherit_compare(DeeTypeObject *__restrict self) {
 struct type_cmp *base_cmp;
 DeeTypeObject *base = DeeType_Base(self);
 if (!base ||
   (((base_cmp = base->tp_cmp) == NULL ||
    (!base_cmp->tp_eq && !base_cmp->tp_ne &&
     !base_cmp->tp_lo && !base_cmp->tp_le &&
     !base_cmp->tp_gr && !base_cmp->tp_ge)) &&
     !type_inherit_compare(base)))
      return false;
 DEE_DPRINTF("Inherit `operator <compare>' %q from into %q\n",
             base->tp_name,self->tp_name);
 base_cmp = base->tp_cmp;
 if (self->tp_cmp) {
  self->tp_cmp->tp_eq = base_cmp->tp_eq;
  self->tp_cmp->tp_ne = base_cmp->tp_ne;
  self->tp_cmp->tp_lo = base_cmp->tp_lo;
  self->tp_cmp->tp_le = base_cmp->tp_le;
  self->tp_cmp->tp_gr = base_cmp->tp_gr;
  self->tp_cmp->tp_ge = base_cmp->tp_ge;
 } else {
  self->tp_cmp = base_cmp;
 }
 return true;
}
#endif /* !DEFINE_TYPE_OPERATORS */

#define DEFINE_COMPARE_OPERATOR(name,fwd,bck,operator_name,invoke_fwd,invoke_bck) \
DEFINE_OPERATOR(DREF DeeObject *,name, \
               (DeeObject *__restrict self, \
                DeeObject *__restrict some_object)) { \
 LOAD_TP_SELF; \
 ASSERT_OBJECT(some_object); \
 do { \
 if (tp_self->tp_cmp) { \
  if (tp_self->tp_cmp->tp_##fwd) \
      return invoke_fwd(tp_self,self,some_object); \
  if (tp_self->tp_cmp->tp_##bck) \
      return invoke_not(invoke_bck(tp_self,self,some_object)); \
 } \
 } while (type_inherit_compare(tp_self)); \
 err_unimplemented_operator(tp_self,operator_name); \
 return NULL; \
}
DEFINE_COMPARE_OPERATOR(CompareEqObject,eq,ne,OPERATOR_EQ,DeeType_INVOKE_EQ,DeeType_INVOKE_NE)
DEFINE_COMPARE_OPERATOR(CompareNeObject,ne,eq,OPERATOR_NE,DeeType_INVOKE_NE,DeeType_INVOKE_EQ)
DEFINE_COMPARE_OPERATOR(CompareLoObject,lo,ge,OPERATOR_LO,DeeType_INVOKE_LO,DeeType_INVOKE_GE)
DEFINE_COMPARE_OPERATOR(CompareLeObject,le,gr,OPERATOR_LE,DeeType_INVOKE_LE,DeeType_INVOKE_GR)
DEFINE_COMPARE_OPERATOR(CompareGrObject,gr,lo,OPERATOR_GR,DeeType_INVOKE_GR,DeeType_INVOKE_LO)
DEFINE_COMPARE_OPERATOR(CompareGeObject,ge,le,OPERATOR_GE,DeeType_INVOKE_GE,DeeType_INVOKE_LE)
#undef DEFINE_COMPARE_OPERATOR


#ifndef DEFINE_TYPE_OPERATORS
#define DEFINE_COMPARE_OPERATOR(name,fwd,bck,error) \
PUBLIC int DCALL \
name(DeeObject *__restrict self, \
     DeeObject *__restrict some_object) { \
 DeeObject *val; int result; \
 val = name##Object(self,some_object); \
 if unlikely(!val) return -1; \
 result = DeeObject_Bool(val); \
 Dee_Decref(val); \
 return result; \
}
DEFINE_COMPARE_OPERATOR(DeeObject_CompareLo,lo,ge,OPERATOR_LO)
DEFINE_COMPARE_OPERATOR(DeeObject_CompareLe,le,gr,OPERATOR_LE)
DEFINE_COMPARE_OPERATOR(DeeObject_CompareGr,gr,lo,OPERATOR_GR)
DEFINE_COMPARE_OPERATOR(DeeObject_CompareGe,ge,le,OPERATOR_GE)
#undef DEFINE_COMPARE_OPERATOR

PUBLIC int DCALL
DeeObject_CompareEq(DeeObject *__restrict self,
                    DeeObject *__restrict some_object) {
 DeeObject *val; int result;
 val = DeeObject_CompareEqObject(self,some_object);
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

PUBLIC int DCALL
DeeObject_CompareNe(DeeObject *__restrict self,
                    DeeObject *__restrict some_object) {
 DeeObject *val; int result;
 val = DeeObject_CompareNeObject(self,some_object);
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
#endif /* !DEFINE_TYPE_OPERATORS */

#ifndef DEFINE_TYPE_OPERATORS
#define DEFINE_TYPE_INHERIT_FUNCTION(name,field) \
INTERN bool DCALL \
name(DeeTypeObject *__restrict self) { \
 DeeTypeObject *base = DeeType_Base(self); \
 struct type_seq *base_seq; \
 if (!base || \
   (((base_seq = base->tp_seq) == NULL || \
    (!base_seq->field)) && !name(base))) \
      return false; \
 base_seq = base->tp_seq; \
 if (self->tp_seq) { \
  self->tp_seq->field = base_seq->field; \
 } else { \
  self->tp_seq = base_seq; \
 } \
 return true; \
}
INTERN bool DCALL
type_inherit_iternext(DeeTypeObject *__restrict self) {
 DeeTypeObject *base = DeeType_Base(self);
 if (!base ||
    (!base->tp_iter_next &&
     !type_inherit_iternext(base)))
      return false;
 self->tp_iter_next = base->tp_iter_next;
 return true;
}
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_iterself,tp_iter_self)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_size,tp_size)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_contains,tp_contains)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_getitem,tp_get)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_delitem,tp_del)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_setitem,tp_set)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_getrange,tp_range_get)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_delrange,tp_range_del)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_setrange,tp_range_set)
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif /* !DEFINE_TYPE_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *,IterSelf,(DeeObject *__restrict self)) {
 LOAD_TP_SELF;
 do {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq && seq->tp_iter_self)
      return DeeType_INVOKE_ITER(tp_self,self);
 } while (type_inherit_iterself(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_ITERSELF);
 return NULL;
}
DEFINE_OPERATOR(DREF DeeObject *,IterNext,(DeeObject *__restrict self)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_iter_next)
      return DeeType_INVOKE_NEXT(tp_self,self);
 } while (type_inherit_iternext(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_ITERNEXT);
 return NULL;
}




#ifndef DEFINE_TYPE_OPERATORS

DEFINE_OPERATOR(size_t,Size,(DeeObject *__restrict self)) {
 DREF DeeObject *sizeob;
 size_t result;
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_size) {
   struct type_nsi *nsi;
   /* NSI optimizations. */
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi) {
    ASSERT(nsi->nsi_common.nsi_getsize);
    return (*nsi->nsi_common.nsi_getsize)(self);
   }
   sizeob = DeeType_INVOKE_SIZE(tp_self,self);
   if unlikely(!sizeob) goto err;
   if (DeeObject_AsSize(sizeob,&result))
       goto err_ob;
   Dee_Decref(sizeob);
   /* Deal with negative-one */
   if unlikely(result == (size_t)-1)
      return err_integer_overflow_i(sizeof(size_t)*8,true);
   return result;
  }
 } while (type_inherit_size(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_SIZE);
 return (size_t)-1;
err_ob:
 Dee_Decref(sizeob);
err:
 return (size_t)-1;
}

DEFINE_OPERATOR(int,Contains,
               (DeeObject *__restrict self,
                DeeObject *__restrict some_object)) {
 DREF DeeObject *resultob; int result;
 resultob = DeeObject_ContainsObject(self,some_object);
 if unlikely(!resultob) goto err;
 result = DeeObject_Bool(resultob);
 Dee_Decref(resultob);
 return result;
err:
 return -1;
}
#endif

DEFINE_OPERATOR(DREF DeeObject *,SizeObject,(DeeObject *__restrict self)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_size)
      return DeeType_INVOKE_SIZE(tp_self,self);
 } while (type_inherit_size(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_SIZE);
 return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *,ContainsObject,
               (DeeObject *__restrict self,
                DeeObject *__restrict some_object)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_contains)
      return DeeType_INVOKE_CONTAINS(tp_self,self,some_object);
 } while (type_inherit_contains(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_CONTAINS);
 return NULL;
}
DEFINE_OPERATOR(DREF DeeObject *,GetItem,
               (DeeObject *__restrict self,
                DeeObject *__restrict index)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_get)
      return DeeType_INVOKE_GETITEM(tp_self,self,index);
 } while (type_inherit_getitem(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_GETITEM);
 return NULL;
}
DEFINE_OPERATOR(int,DelItem,
               (DeeObject *__restrict self,
                DeeObject *__restrict index)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_del)
      return DeeType_INVOKE_DELITEM(tp_self,self,index);
 } while (type_inherit_delitem(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_DELITEM);
}
DEFINE_OPERATOR(int,SetItem,(DeeObject *__restrict self,
                             DeeObject *__restrict index,
                             DeeObject *__restrict value)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_set)
      return DeeType_INVOKE_SETITEM(tp_self,self,index,value);
 } while (type_inherit_setitem(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_SETITEM);
}
DEFINE_OPERATOR(DREF DeeObject *,GetRange,
               (DeeObject *__restrict self,
                DeeObject *__restrict begin,
                DeeObject *__restrict end)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get)
      return DeeType_INVOKE_GETRANGE(tp_self,self,begin,end);
 } while (type_inherit_getrange(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_GETRANGE);
 return NULL;
}
#ifndef DEFINE_TYPE_OPERATORS

INTERN DREF DeeObject *DCALL
DeeObject_ConcatInherited(DREF DeeObject *__restrict self,
                          DeeObject *__restrict other) {
 DREF DeeObject *result;
 if (DeeTuple_CheckExact(other)) {
  size_t i,count = DeeTuple_SIZE(other);
  for (i = 0; i < count; ++i)
      Dee_Incref(DeeTuple_GET(other,i));
  result = DeeObject_ExtendInherited(self,count,DeeTuple_ELEM(other));
  if unlikely(!result) {
   for (i = 0; i < count; ++i)
       Dee_Decref(DeeTuple_GET(other,i));
  }
  return result;
 }
 if (DeeTuple_CheckExact(self))
     return DeeTuple_ConcatInherited(self,other);
 if (DeeList_CheckExact(self))
     return DeeList_Concat(self,other);
 /* Fallback: perform an arithmetic add operation. */
 result = DeeObject_Add(self,other);
 Dee_Decref(self);
 return result;
}
INTERN DREF DeeObject *DCALL
DeeObject_ExtendInherited(DREF DeeObject *__restrict self,
                          size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result;
 DREF SharedVector *other;
 if (DeeTuple_CheckExact(self))
     return DeeTuple_ExtendInherited(self,argc,argv);
 if (DeeList_CheckExact(self))
     return DeeList_ExtendInherited(self,argc,argv);
 /* Fallback: perform an arithmetic add operation. */
 other = SharedVector_NewShared(argc,argv);
 if unlikely(!other) return NULL;
 result = DeeObject_Add(self,(DeeObject *)other);
 SharedVector_Decref(other);
 Dee_Decref(self);
 return result;
}


PUBLIC DREF DeeObject *
(DCALL DeeObject_GetRangeBeginIndex)(DeeObject *__restrict self,
                                     dssize_t begin,
                                     DeeObject *__restrict end) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(end);
 do {
  struct type_seq *seq = tp_self->tp_seq;
  if (seq && seq->tp_range_get) {
   dssize_t end_index;
   struct type_nsi *nsi;
   DREF DeeObject *begin_ob,*result;
   /* NSI optimizations. */
   nsi = seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (DeeNone_Check(end)) {
     if (nsi->nsi_seqlike.nsi_getrange_n)
         return (*nsi->nsi_seqlike.nsi_getrange_n)(self,begin);
    } else if (nsi->nsi_seqlike.nsi_getrange) {
     if (DeeObject_AsSSize(end,&end_index)) goto err;
     return (*nsi->nsi_seqlike.nsi_getrange)(self,begin,end_index);
    }
   }
   begin_ob = DeeInt_NewSSize(begin);
   if unlikely(!begin_ob) goto err;
   result = DeeType_INVOKE_GETRANGE(tp_self,self,begin_ob,end);
   Dee_Decref(begin_ob);
   return result;
  }
 } while (type_inherit_getrange(tp_self));
 err_unimplemented_operator(GET_TP_SELF(),OPERATOR_GETRANGE);
err:
 return NULL;
}
PUBLIC DREF DeeObject *
(DCALL DeeObject_GetRangeEndIndex)(DeeObject *__restrict self,
                                   DeeObject *__restrict begin, dssize_t end) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(begin);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
   dssize_t begin_index;
   DREF DeeObject *end_ob,*result;
   struct type_nsi *nsi;
   /* NSI optimizations. */
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (nsi->nsi_seqlike.nsi_getrange) {
     if (DeeObject_AsSSize(begin,&begin_index)) goto err;
     return (*nsi->nsi_seqlike.nsi_getrange)(self,begin_index,end);
    }
   }
   end_ob = DeeInt_NewSSize(end);
   if unlikely(!end_ob) goto err;
   result = DeeType_INVOKE_GETRANGE(tp_self,self,begin,end_ob);
   Dee_Decref(end_ob);
   return result;
  }
 } while (type_inherit_getrange(tp_self));
 err_unimplemented_operator(GET_TP_SELF(),OPERATOR_GETRANGE);
err:
 return NULL;
}

PUBLIC DREF DeeObject *
(DCALL DeeObject_GetRangeIndex)(DeeObject *__restrict self,
                                dssize_t begin, dssize_t end) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
   DREF DeeObject *begin_ob,*end_ob,*result;
   struct type_nsi *nsi;
   /* NSI optimizations. */
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (nsi->nsi_seqlike.nsi_getrange)
        return (*nsi->nsi_seqlike.nsi_getrange)(self,begin,end);
   }
   begin_ob = DeeInt_NewSSize(begin);
   if unlikely(!begin_ob) goto err;
   end_ob   = DeeInt_NewSSize(end);
   if unlikely(!end_ob) { Dee_Decref(begin_ob); goto err; }
   result = DeeType_INVOKE_GETRANGE(tp_self,self,begin_ob,end_ob);
   Dee_Decref(end_ob);
   Dee_Decref(begin_ob);
   return result;
  }
 } while (type_inherit_getrange(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_GETRANGE);
err:
 return NULL;
}
PUBLIC int
(DCALL DeeObject_SetRangeBeginIndex)(DeeObject *__restrict self,
                                     dssize_t begin, DeeObject *__restrict end,
                                     DeeObject *__restrict value) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(end);
 ASSERT_OBJECT(value);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
   int result; DREF DeeObject *begin_ob;
   struct type_nsi *nsi;
   /* NSI optimizations. */
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (DeeNone_Check(end)) {
     if (nsi->nsi_seqlike.nsi_setrange_n)
         return (*nsi->nsi_seqlike.nsi_setrange_n)(self,begin,value);
    } else if (nsi->nsi_seqlike.nsi_setrange) {
     dssize_t end_index;
     if (DeeObject_AsSSize(end,&end_index)) goto err;
     return (*nsi->nsi_seqlike.nsi_setrange)(self,begin,end_index,value);
    }
   }
   begin_ob = DeeInt_NewSSize(begin);
   if unlikely(!begin_ob) goto err;
   result = DeeType_INVOKE_SETRANGE(tp_self,self,begin_ob,end,value);
   Dee_Decref(begin_ob);
   return result;
  }
 } while (type_inherit_setrange(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_SETRANGE);
err:
 return -1;
}
PUBLIC int
(DCALL DeeObject_SetRangeEndIndex)(DeeObject *__restrict self,
                                   DeeObject *__restrict begin, dssize_t end,
                                   DeeObject *__restrict value) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(begin);
 ASSERT_OBJECT(value);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
   int result; DREF DeeObject *end_ob;
   struct type_nsi *nsi;
   /* NSI optimizations. */
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (nsi->nsi_seqlike.nsi_setrange) {
     dssize_t start_index;
     if (DeeObject_AsSSize(begin,&start_index)) goto err;
     return (*nsi->nsi_seqlike.nsi_setrange)(self,start_index,end,value);
    }
   }
   end_ob = DeeInt_NewSSize(end);
   if unlikely(!end_ob) goto err;
   result = DeeType_INVOKE_SETRANGE(tp_self,self,begin,end_ob,value);
   Dee_Decref(end_ob);
   return result;
  }
 } while (type_inherit_setrange(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_SETRANGE);
err:
 return -1;
}
PUBLIC int
(DCALL DeeObject_SetRangeIndex)(DeeObject *__restrict self,
                                dssize_t begin, dssize_t end,
                                DeeObject *__restrict value) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(value);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_set) {
   DREF DeeObject *begin_ob,*end_ob; int result;
   struct type_nsi *nsi;
   /* NSI optimizations. */
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (nsi->nsi_seqlike.nsi_setrange)
        return (*nsi->nsi_seqlike.nsi_setrange)(self,begin,end,value);
   }
   begin_ob = DeeInt_NewSSize(begin);
   if unlikely(!begin_ob) goto err;
   end_ob   = DeeInt_NewSSize(end);
   if unlikely(!end_ob) { Dee_Decref(begin_ob); goto err; }
   result = DeeType_INVOKE_SETRANGE(tp_self,self,begin_ob,end_ob,value);
   Dee_Decref(end_ob);
   Dee_Decref(begin_ob);
   return result;
  }
 } while (type_inherit_setrange(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_SETRANGE);
err:
 return -1;
}
#endif

DEFINE_OPERATOR(int,DelRange,(DeeObject *__restrict self,
                              DeeObject *__restrict begin,
                              DeeObject *__restrict end)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(begin);
 ASSERT_OBJECT(end);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_del)
      return DeeType_INVOKE_DELRANGE(tp_self,self,begin,end);
 } while (type_inherit_delrange(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_DELRANGE);
}
DEFINE_OPERATOR(int,SetRange,(DeeObject *__restrict self,
                              DeeObject *__restrict begin,
                              DeeObject *__restrict end,
                              DeeObject *__restrict value)) {
 LOAD_TP_SELF;
 ASSERT_OBJECT(begin);
 ASSERT_OBJECT(end);
 ASSERT_OBJECT(value);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_range_set)
      return DeeType_INVOKE_SETRANGE(tp_self,self,begin,end,value);
 } while (type_inherit_setrange(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_SETRANGE);
}



#ifndef DEFINE_TYPE_OPERATORS
PUBLIC DREF DeeObject *DCALL
DeeObject_GetItemDef(DeeObject *__restrict self,
                     DeeObject *__restrict key,
                     DeeObject *__restrict def) {
 DREF DeeObject *result;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
   struct type_nsi *nsi;
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP) {
    if (nsi->nsi_maplike.nsi_getdefault)
        return (*nsi->nsi_maplike.nsi_getdefault)(self,key,def);
   }
   result = DeeType_INVOKE_GETITEM(tp_self,self,key);
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
 } while (type_inherit_getitem(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_GETITEM);
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_GetItemIndex(DeeObject *__restrict self,
                       size_t index) {
 DREF DeeObject *index_ob,*result;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
   struct type_nsi *nsi;
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (nsi->nsi_seqlike.nsi_getitem)
        return (*nsi->nsi_seqlike.nsi_getitem)(self,index);
   }
   /* Fallback create an integer object and use it for indexing. */
   index_ob = DeeInt_NewSize(index);
   if unlikely(!index_ob) goto err;
   result = DeeType_INVOKE_GETITEM(tp_self,self,index_ob);
   Dee_Decref(index_ob);
   return result;
  }
 } while (type_inherit_getitem(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_GETITEM);
err:
 return NULL;
}

PUBLIC int (DCALL DeeObject_DelItemIndex)(DeeObject *__restrict self,
                                          size_t index) {
 DREF DeeObject *index_ob; int result;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_del) {
   struct type_nsi *nsi;
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (nsi->nsi_seqlike.nsi_delitem)
        return (*nsi->nsi_seqlike.nsi_delitem)(self,index);
   }
   /* Fallback create an integer object and use it for indexing. */
   index_ob = DeeInt_NewSize(index);
   if unlikely(!index_ob) goto err;
   result = DeeType_INVOKE_DELITEM(tp_self,self,index_ob);
   Dee_Decref(index_ob);
   return result;
  }
 } while (type_inherit_getitem(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_DELITEM);
err:
 return -1;
}
PUBLIC int (DCALL DeeObject_SetItemIndex)(DeeObject *__restrict self,
                                          size_t index,
                                          DeeObject *__restrict value) {
 DREF DeeObject *index_ob; int result;
 DeeTypeObject *tp_self;
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_seq && tp_self->tp_seq->tp_set) {
   struct type_nsi *nsi;
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
    if (nsi->nsi_seqlike.nsi_setitem)
        return (*nsi->nsi_seqlike.nsi_setitem)(self,index,value);
   }
   /* Fallback create an integer object and use it for indexing. */
   index_ob = DeeInt_NewSize(index);
   if unlikely(!index_ob) goto err;
   result = DeeType_INVOKE_SETITEM(tp_self,self,index_ob,value);
   Dee_Decref(index_ob);
   return result;
  }
 } while (type_inherit_getitem(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_DELITEM);
err:
 return -1;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_GetItemString(DeeObject *__restrict self,
                        char const *__restrict key,
                        dhash_t hash) {
 DREF DeeObject *key_ob,*result;
 ASSERT_OBJECT(self);
 if (DeeDict_CheckExact(self))
     return DeeDict_GetItemString(self,key,hash);
 if (DeeKwdsMapping_CheckExact(self))
     return DeeKwdsMapping_GetItemString(self,key,hash);
 /* Fallback: create a string object and use it for indexing. */
 key_ob = DeeString_New(key);
 if unlikely(!key_ob) return NULL;
 result = DeeObject_GetItem(self,key_ob);
 Dee_Decref(key_ob);
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_GetItemStringDef(DeeObject *__restrict self,
                           char const *__restrict key, dhash_t hash,
                           DeeObject *__restrict def) {
 DREF DeeObject *key_ob,*result;
 ASSERT_OBJECT(self);
 if (DeeDict_CheckExact(self))
     return DeeDict_GetItemStringDef(self,key,hash,def);
 if (DeeKwdsMapping_CheckExact(self))
     return DeeKwdsMapping_GetItemStringDef(self,key,hash,def);
 /* Fallback: create a string object and use it for indexing. */
 key_ob = DeeString_NewWithHash(key,hash);
 if unlikely(!key_ob) return NULL;
 result = DeeObject_GetItemDef(self,key_ob,def);
 Dee_Decref(key_ob);
 return result;
}
PUBLIC int (DCALL DeeObject_DelItemString)(DeeObject *__restrict self,
                                           char const *__restrict key,
                                           dhash_t hash) {
 DREF DeeObject *key_ob; int result;
 ASSERT_OBJECT(self);
 if (DeeDict_CheckExact(self))
     return DeeDict_DelItemString(self,key,hash);
 /* Fallback: create a string object and use it for indexing. */
 key_ob = DeeString_NewWithHash(key,hash);
 if unlikely(!key_ob) return -1;
 result = DeeObject_DelItem(self,key_ob);
 Dee_Decref(key_ob);
 return result;
}
PUBLIC int (DCALL DeeObject_SetItemString)(DeeObject *__restrict self,
                                           char const *__restrict key,
                                           dhash_t hash,
                                           DeeObject *__restrict value) {
 DREF DeeObject *key_ob; int result;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(value);
 if (DeeDict_CheckExact(self))
     return DeeDict_SetItemString(self,key,hash,value);
 /* Fallback: create a string object and use it for indexing. */
 key_ob = DeeString_NewWithHash(key,hash);
 if unlikely(!key_ob) return -1;
 result = DeeObject_SetItem(self,key_ob,value);
 Dee_Decref(key_ob);
 return result;
}

INTDEF dssize_t DCALL
comerr_print(DeeCompilerErrorObject *__restrict self,
             dformatprinter printer, void *arg);

PUBLIC dssize_t DCALL
DeeObject_Print(DeeObject *__restrict self,
                dformatprinter printer, void *arg) {
 DREF DeeObject *ob_str; dssize_t result;
 if (DeeInt_Check(self))
     return DeeInt_Print(self,DEEINT_PRINT_DEC,printer,arg);
 if (DeeString_Check(self))
     return DeeString_PrintUtf8(self,printer,arg);
 if (DeeBytes_Check(self))
     return DeeBytes_PrintUtf8(self,printer,arg);
 if (Dee_TYPE(self) == &DeeError_CompilerError ||
     Dee_TYPE(self) == &DeeError_SyntaxError ||
     Dee_TYPE(self) == &DeeError_SymbolError)
     return comerr_print((DeeCompilerErrorObject *)self,printer,arg);
 /* Fallback: print the object __str__ operator result. */
 ob_str = DeeObject_Str(self);
 if unlikely(!ob_str) return -1;
 result = DeeString_PrintUtf8(ob_str,printer,arg);
 Dee_Decref(ob_str);
 return result;
}

INTDEF dssize_t DCALL
bytes_print_repr(DeeObject *__restrict self, dformatprinter printer, void *arg);

PUBLIC dssize_t DCALL
DeeObject_PrintRepr(DeeObject *__restrict self,
                    dformatprinter printer, void *arg) {
 DREF DeeObject *ob_repr; dssize_t result;
 if (DeeInt_Check(self))
     return DeeInt_Print(self,DEEINT_PRINT_DEC,printer,arg);
 if (DeeString_Check(self))
     return DeeString_PrintRepr(self,printer,arg,FORMAT_QUOTE_FNORMAL);
 if (DeeBytes_Check(self))
     return bytes_print_repr(self,printer,arg);
 if (Dee_TYPE(self) == &DeeError_CompilerError)
     return comerr_print((DeeCompilerErrorObject *)self,printer,arg);

 /* Fallback: print the object __repr__ operator result. */
 ob_repr = DeeObject_Repr(self);
 if unlikely(!ob_repr) return -1;
 result = DeeString_PrintUtf8(ob_repr,printer,arg);
 Dee_Decref(ob_repr);
 return result;
}
#endif /* !DEFINE_TYPE_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *,GetAttr,
               (DeeObject *__restrict self,
                /*String*/DeeObject *__restrict attr_name)) {
 struct membercache *cache;
 DREF DeeObject *result;
 dhash_t hash;
 LOAD_ITER;
 ASSERT_OBJECT_TYPE_EXACT(attr_name,&DeeString_Type);
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 hash = DeeString_Hash(attr_name);
 /* Search through the cache for the requested attribute. */
 if ((result = membercache_getattr(cache,self,
                                   DeeString_STR(attr_name),
                                   hash)) != ITER_DONE)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct class_attribute *member;
   struct class_desc *desc;
   desc = DeeClass_DESC(iter);
   if ((member = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this member. */
    if (!member_mayaccess(iter,member)) {
     err_protected_member(iter,member);
     return NULL;
    }
    return member_get(iter,DeeInstance_DESC(desc,self),self,member);
   }
  } else {
   if (iter->tp_methods &&
      (result = type_method_getattr(cache,iter->tp_methods,self,
                                    DeeString_STR(attr_name),hash)) != ITER_DONE)
       goto done;
   if (iter->tp_getsets &&
      (result = type_getset_getattr(cache,iter->tp_getsets,self,
                                    DeeString_STR(attr_name),hash)) != ITER_DONE)
       goto done;
   if (iter->tp_members &&
      (result = type_member_getattr(cache,iter->tp_members,self,
                                    DeeString_STR(attr_name),hash)) != ITER_DONE)
       goto done;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if likely(iter->tp_attr->tp_getattr)
      return DeeType_INVOKE_GETATTR(iter,self,attr_name);
   /* Don't consider attributes from lower levels for custom member access. */
   break;
  }
 }
 err_unknown_attribute(GET_TP_SELF(),
                       DeeString_STR(attr_name),
                       ATTR_ACCESS_GET);
 return NULL;
done:
 return result;
}
DEFINE_OPERATOR(int,DelAttr,
               (DeeObject *__restrict self,
                /*String*/DeeObject *__restrict attr_name)) {
 struct membercache *cache;
 int temp; dhash_t hash;
 LOAD_ITER;
 ASSERT_OBJECT_TYPE_EXACT(attr_name,&DeeString_Type);
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 hash = DeeString_Hash(attr_name);
 /* Search through the cache for the requested attribute. */
 if ((temp = membercache_delattr(cache,self,
                                 DeeString_STR(attr_name),
                                 hash)) <= 0)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct class_attribute *member;
   struct class_desc *desc;
   desc = DeeClass_DESC(iter);
   if ((member = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this member. */
    if (!member_mayaccess(iter,member)) {
     err_protected_member(iter,member);
     goto err;
    }
    return member_del(iter,DeeInstance_DESC(desc,self),self,member);
   }
  } else {
   if (iter->tp_methods &&
       type_method_hasattr(cache,iter->tp_methods,DeeString_STR(attr_name),hash))
       goto noaccess;
   if (iter->tp_getsets &&
      (temp = type_getset_delattr(cache,iter->tp_getsets,self,DeeString_STR(attr_name),hash)) <= 0)
       goto done;
   if (iter->tp_members &&
      (temp = type_member_delattr(cache,iter->tp_members,self,DeeString_STR(attr_name),hash)) <= 0)
       goto done;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if likely(iter->tp_attr->tp_delattr)
      return DeeType_INVOKE_DELATTR(iter,self,attr_name);
   /* Don't consider attributes from lower levels for custom member access. */
   break;
  }
 }
 err_unknown_attribute(GET_TP_SELF(),
                       DeeString_STR(attr_name),
                       ATTR_ACCESS_DEL);
 goto err;
noaccess:
 err_cant_access_attribute(iter,
                           DeeString_STR(attr_name),
                           ATTR_ACCESS_DEL);
err:
 return -1;
done:
 return temp;
}

DEFINE_OPERATOR(int,SetAttr,
               (DeeObject *__restrict self,
                /*String*/DeeObject *__restrict attr_name,
                DeeObject *__restrict value)) {
 struct membercache *cache;
 int temp; dhash_t hash;
 LOAD_ITER;
 ASSERT_OBJECT_TYPE_EXACT(attr_name,&DeeString_Type);
 ASSERT_OBJECT(value);
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 hash = DeeString_Hash(attr_name);
 /* Search through the cache for the requested attribute. */
 if ((temp = membercache_setattr(cache,self,
                                 DeeString_STR(attr_name),
                                 hash,value)) <= 0)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct class_attribute *member;
   struct class_desc *desc;
   desc = DeeClass_DESC(iter);
   if ((member = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this member. */
    if (!member_mayaccess(iter,member)) {
     err_protected_member(iter,member);
     goto err;
    }
    return member_set(iter,DeeInstance_DESC(desc,self),self,member,value);
   }
  } else {
   if (iter->tp_methods &&
       type_method_hasattr(cache,iter->tp_methods,DeeString_STR(attr_name),hash))
       goto noaccess;
   if (iter->tp_getsets &&
      (temp = type_getset_setattr(cache,iter->tp_getsets,self,DeeString_STR(attr_name),hash,value)) <= 0)
       goto done;
   if (iter->tp_members &&
      (temp = type_member_setattr(cache,iter->tp_members,self,DeeString_STR(attr_name),hash,value)) <= 0)
       goto done;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if likely(iter->tp_attr->tp_setattr)
      return DeeType_INVOKE_SETATTR(iter,self,attr_name,value);
   /* Don't consider attributes from lower levels for custom member access. */
   break;
  }
 }
 err_unknown_attribute(GET_TP_SELF(),
                       DeeString_STR(attr_name),
                       ATTR_ACCESS_SET);
 goto err;
noaccess:
 err_cant_access_attribute(iter,
                           DeeString_STR(attr_name),
                           ATTR_ACCESS_SET);
err:
 return -1;
done:
 return temp;
}


#ifndef DEFINE_TYPE_OPERATORS
INTERN bool DCALL
type_inherit_with(DeeTypeObject *__restrict self) {
 DeeTypeObject *base = DeeType_Base(self);
 struct type_with *base_with;
 if (!base ||
   (((base_with = base->tp_with) == NULL ||
    (!base_with->tp_enter && !base_with->tp_leave)) &&
     !type_inherit_with(base)))
      return false;
 base_with = base->tp_with;
 if unlikely(self->tp_with) {
  self->tp_with->tp_enter = base_with->tp_enter;
  self->tp_with->tp_leave = base_with->tp_leave;
 } else {
  self->tp_with = base_with;
 }
 return true;
}
#endif /* !DEFINE_TYPE_OPERATORS */

DEFINE_OPERATOR(int,Enter,
               (DeeObject *__restrict self)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_with) {
   if (tp_self->tp_with->tp_enter)
       return DeeType_INVOKE_ENTER(tp_self,self);
   /* Special case: When `tp_leave' is implemented,
    * a missing `tp_enter' behaves as a no-op. */
   if (tp_self->tp_with->tp_leave)
       return 0;
  }
 } while (type_inherit_with(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_ENTER);
}

DEFINE_OPERATOR(int,Leave,
               (DeeObject *__restrict self)) {
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_with) {
   if (tp_self->tp_with->tp_leave)
       return DeeType_INVOKE_LEAVE(tp_self,self);
   /* Special case: When `tp_enter' is implemented,
    * a missing `tp_leave' behaves as a no-op. */
   if (tp_self->tp_with->tp_enter)
       return 0;
  }
 } while (type_inherit_with(tp_self));
 return err_unimplemented_operator(tp_self,OPERATOR_LEAVE);
}


#ifndef DEFINE_TYPE_OPERATORS
INTERN bool DCALL
type_inherit_buffer(DeeTypeObject *__restrict self) {
 DeeTypeObject *base = DeeType_Base(self);
 struct type_buffer *base_buffer;
 if (!base ||
   (((base_buffer = base->tp_buffer) == NULL ||
    (!base_buffer->tp_getbuf)) &&
     !type_inherit_buffer(base)))
      return false;
 base_buffer = base->tp_buffer;
 if unlikely(self->tp_buffer) {
  memcpy(self->tp_buffer,base_buffer,
         sizeof(struct type_buffer));
 } else {
  self->tp_buffer = base_buffer;
 }
 return true;
}
#endif /* !DEFINE_TYPE_OPERATORS */


DEFINE_OPERATOR(int,GetBuf,
               (DeeObject *__restrict self,
                DeeBuffer *__restrict info,
                unsigned int flags)) {
 ASSERTF(!(flags & ~(DEE_BUFFER_FWRITABLE)),
          "Unknown buffers flags in %x",flags);
 LOAD_TP_SELF;
 do {
  struct type_buffer *buf = tp_self->tp_buffer;
  if (buf && buf->tp_getbuf) {
   if unlikely((flags & DEE_BUFFER_FWRITABLE) &&
               (buf->tp_buffer_flags & DEE_BUFFER_TYPE_FREADONLY)) {
    DeeError_Throwf(&DeeError_BufferError,
                    "Cannot write to read-only buffer of type %k",
                    tp_self);
    goto err;
   }
#ifndef __INTELLISENSE__
   info->bb_put = buf->tp_putbuf;
#endif
   return (*buf->tp_getbuf)(self,info,flags);
  }
 } while (type_inherit_buffer(tp_self));
 err_unimplemented_operator(tp_self,OPERATOR_GETBUF);
err:
 return -1;
}

DEFINE_OPERATOR(void,PutBuf,
               (DeeObject *__restrict self,
                DeeBuffer *__restrict info,
                unsigned int flags)) {
 ASSERTF(!(flags & ~(DEE_BUFFER_FWRITABLE)),
          "Unknown buffers flags in %x",flags);
#ifdef __INTELLISENSE__
 (void)self;
 (void)info;
 (void)flags;
#elif 1
#ifdef DEFINE_TYPE_OPERATORS
 (void)tp_self;
#endif
 if (info->bb_put)
   (*info->bb_put)(self,info,flags);
#else
 LOAD_TP_SELF;
 do {
  if (tp_self->tp_buffer && tp_self->tp_buffer->tp_getbuf) {
   if (tp_self->tp_buffer->tp_putbuf)
     (*tp_self->tp_buffer->tp_putbuf)(self,info,flags);
   break;
  }
 } while (type_inherit_buffer(tp_self));
#endif
}


#undef GET_TP_SELF
#undef GET_TP_PSELF
#undef LOAD_ITER
#undef LOAD_ITERP
#undef LOAD_TP_SELF
#undef DEFINE_OPERATOR


#ifndef DEFINE_TYPE_OPERATORS
PUBLIC DREF /*String*/DeeObject *DCALL
DeeObject_Doc(DeeObject *__restrict self) {
 DREF /*String*/DeeObject *result;
 /* Special case: Lookup documentation of `none' yields documentation of `DeeNone_Type' */
 if (self == Dee_None)
     self = (DeeObject *)&DeeNone_Type;
 if (DeeType_Check(self)) {
  DeeTypeObject *me = (DeeTypeObject *)self;
  if (!me->tp_doc) goto nodoc;
  if (me->tp_flags&TP_FDOCOBJECT) {
   return_reference_((DeeObject *)COMPILER_CONTAINER_OF(me->tp_doc,DeeStringObject,s_str));
  }
  return DeeString_NewUtf8(me->tp_doc,
                           strlen(me->tp_doc),
                           STRING_ERROR_FIGNORE);
 }
 /* Fallback: Look for an attribute `__doc__' */
 result = DeeObject_GetAttrString(self,"__doc__");
 if (result) {
  /* Make sure that it's a string. */
  if (DeeObject_AssertTypeExact(result,&DeeString_Type))
      Dee_Clear(result);
  return result;
 }
 if (!DeeError_Catch(&DeeError_AttributeError) &&
     !DeeError_Catch(&DeeError_NotImplemented))
      return NULL;
nodoc:
 DeeError_Throwf(&DeeError_ValueError,
                 "No documentation found for `%k'",
                 self);
 return NULL;
}

PUBLIC int
(DCALL DeeObject_Unpack)(DeeObject *__restrict self,
                         size_t objc,
                         DREF DeeObject **__restrict objv) {
 DREF DeeObject *iterator,*elem;
 size_t fast_size,i = 0;
 /* Try to make use of the fast-sequence API. */
 fast_size = DeeFastSeq_GetSize(self);
 if (fast_size != DEE_FASTSEQ_NOTFAST) {
  if (objc != fast_size)
      return err_invalid_unpack_size(self,objc,fast_size);
  for (; i < objc; ++i) {
   elem = DeeFastSeq_GetItem(self,i);
   if unlikely(!elem) goto err;
   objv[i] = elem; /* Inherit reference. */
  }
  return 0;
 }
 if (DeeNone_Check(self)) {
  /* Special case: `none' can be unpacked into anything. */
  MEMFIL_PTR(objv,Dee_None,objc);
  Dee_Incref_n(Dee_None,objc);
  return 0;
 }
 /* Fallback: Use an iterator. */
 if ((iterator = DeeObject_IterSelf(self)) == NULL)
      goto err;
 for (; i < objc; ++i) {
  elem = DeeObject_IterNext(iterator);
  if unlikely(!ITER_ISOK(elem)) {
   if (elem) err_invalid_unpack_size(self,objc,i);
   Dee_Decref(iterator);
   goto err;
  }
  objv[i] = elem; /* Inherit reference. */
 }
 /* Check to make sure that the iterator actually ends here. */
 elem = DeeObject_IterNext(iterator);
 if unlikely(elem != ITER_DONE) {
  if (elem) err_invalid_unpack_iter_size(self,iterator,objc);
  Dee_Decref(iterator);
  goto err;
 }
 Dee_Decref(iterator);
 return 0;
err:
 while (i--) Dee_Decref(objv[i]);
 return -1;
}


PUBLIC dssize_t DCALL
DeeObject_Foreach(DeeObject *__restrict self,
                  dforeach_t proc, void *arg) {
 dssize_t temp,result = 0;
 DREF DeeObject *elem; size_t fast_size;
 ASSERT(proc);
 fast_size = DeeFastSeq_GetSize(self);
 if (fast_size != DEE_FASTSEQ_NOTFAST) {
  size_t i;
  /* Optimization for fast-sequence object. */
  for (i = 0; i < fast_size; ++i) {
   elem = DeeFastSeq_GetItem(self,i);
   if unlikely(!elem) goto err;
   temp = (*proc)(arg,elem);
   Dee_Decref(elem);
   if unlikely(temp < 0) { result = temp; break; }
   result += temp;
  }
  return result;
 }
 /* Fallback: Use an iterator. */
 if ((self = DeeObject_IterSelf(self)) == NULL)
      goto err;
 while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
  temp = (*proc)(arg,elem);
  Dee_Decref(elem);
  if unlikely(temp < 0) { result = temp; break; }
  result += temp; /* Propagate return values by summarizing them. */
  if (DeeThread_CheckInterrupt())
      goto err_self;
 }
 if unlikely(!elem)
    goto err_self;
 Dee_Decref(self);
 return result;
err_self:
 Dee_Decref(self);
err:
 return -1;
}
#endif


#ifndef DEFINE_TYPE_OPERATORS
INTERN bool DCALL
type_inherit_nsi(DeeTypeObject *__restrict self) {
 DeeTypeObject *base = DeeType_Base(self);
 struct type_seq *base_seq;
 if (!base ||
   (((base_seq = base->tp_seq) == NULL ||
     (base_seq->tp_nsi == NULL)) &&
     !type_inherit_nsi(base)))
     return false;
 if (self->tp_seq != NULL) /* Some other sequence interface has already been implemented! */
     return false;
 self->tp_seq = base->tp_seq;
 return true;
}
#endif


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_C */
