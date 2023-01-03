/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
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
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
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

#include <stdarg.h>

#include "../objects/int_logic.h"
#include "../objects/seq/each.h"
#include "../objects/seq/svec.h"
#include "../objects/seq/varkwds.h"
#include "runtime_error.h"
#include "strings.h"

/* Operator invocation. */

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
#endif

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
#define DeeType_INVOKE_ASSIGN(tp_self, self, other)                ((tp_self)->tp_init.tp_assign == &instance_assign ? instance_tassign(tp_self, self, other) : (*(tp_self)->tp_init.tp_assign)(self, other))
#define DeeType_INVOKE_MOVEASSIGN(tp_self, self, other)            ((tp_self)->tp_init.tp_move_assign == &instance_moveassign ? instance_tmoveassign(tp_self, self, other) : (*(tp_self)->tp_init.tp_move_assign)(self, other))
#define DeeType_INVOKE_STR(tp_self, self)                          ((tp_self)->tp_cast.tp_str == &instance_str ? instance_tstr(tp_self, self) : (*(tp_self)->tp_cast.tp_str)(self))
#ifdef CLASS_TP_FAUTOINIT
#define DeeType_INVOKE_REPR(tp_self, self)                         ((tp_self)->tp_cast.tp_repr == &instance_repr ? instance_trepr(tp_self, self) : \
                                                                   (tp_self)->tp_cast.tp_repr == &instance_builtin_auto_repr ? instance_builtin_auto_trepr(tp_self, self) : \
                                                                   (*(tp_self)->tp_cast.tp_repr)(self))
#else /* CLASS_TP_FAUTOINIT */
#define DeeType_INVOKE_REPR(tp_self, self)                         ((tp_self)->tp_cast.tp_repr == &instance_repr ? instance_trepr(tp_self, self) : (*(tp_self)->tp_cast.tp_repr)(self))
#endif /* !CLASS_TP_FAUTOINIT */
#define DeeType_INVOKE_BOOL(tp_self, self)                         ((tp_self)->tp_cast.tp_bool == &instance_bool ? instance_tbool(tp_self, self) : (*(tp_self)->tp_cast.tp_bool)(self))
#define DeeType_INVOKE_NEXT(tp_self, self)                         ((tp_self)->tp_iter_next == &instance_next ? instance_tnext(tp_self, self) : (*(tp_self)->tp_iter_next)(self))
#define DeeType_INVOKE_CALL(tp_self, self, argc,  argv)            ((tp_self)->tp_call == &instance_call ? instance_tcall(tp_self, self, argc,  argv) : (*(tp_self)->tp_call)(self, argc,  argv))
#define DeeType_INVOKE_CALLKW(tp_self, self, argc,  argv,  kw)     ((tp_self)->tp_call_kw == &instance_callkw ? instance_tcallkw(tp_self, self, argc,  argv,  kw) : (*(tp_self)->tp_call_kw)(self, argc,  argv,  kw))
#define DeeType_INVOKE_INT32(tp_self, self, result)                ((tp_self)->tp_math->tp_int32 == &instance_int32 ? instance_tint32(tp_self, self, result) : (*(tp_self)->tp_math->tp_int32)(self, result))
#define DeeType_INVOKE_INT64(tp_self, self, result)                ((tp_self)->tp_math->tp_int64 == &instance_int64 ? instance_tint64(tp_self, self, result) : (*(tp_self)->tp_math->tp_int64)(self, result))
#define DeeType_INVOKE_DOUBLE(tp_self, self, result)               ((tp_self)->tp_math->tp_double == &instance_double ? instance_tdouble(tp_self, self, result) : (*(tp_self)->tp_math->tp_double)(self, result))
#define DeeType_INVOKE_INT(tp_self, self)                          ((tp_self)->tp_math->tp_int == &instance_int ? instance_tint(tp_self, self) : (*(tp_self)->tp_math->tp_int)(self))
#define DeeType_INVOKE_INV(tp_self, self)                          ((tp_self)->tp_math->tp_inv == &instance_inv ? instance_tinv(tp_self, self) : (*(tp_self)->tp_math->tp_inv)(self))
#define DeeType_INVOKE_POS(tp_self, self)                          ((tp_self)->tp_math->tp_pos == &instance_pos ? instance_tpos(tp_self, self) : (*(tp_self)->tp_math->tp_pos)(self))
#define DeeType_INVOKE_NEG(tp_self, self)                          ((tp_self)->tp_math->tp_neg == &instance_neg ? instance_tneg(tp_self, self) : (*(tp_self)->tp_math->tp_neg)(self))
#define DeeType_INVOKE_ADD(tp_self, self, other)                   ((tp_self)->tp_math->tp_add == &instance_add ? instance_tadd(tp_self, self, other) : (*(tp_self)->tp_math->tp_add)(self, other))
#define DeeType_INVOKE_SUB(tp_self, self, other)                   ((tp_self)->tp_math->tp_sub == &instance_sub ? instance_tsub(tp_self, self, other) : (*(tp_self)->tp_math->tp_sub)(self, other))
#define DeeType_INVOKE_MUL(tp_self, self, other)                   ((tp_self)->tp_math->tp_mul == &instance_mul ? instance_tmul(tp_self, self, other) : (*(tp_self)->tp_math->tp_mul)(self, other))
#define DeeType_INVOKE_DIV(tp_self, self, other)                   ((tp_self)->tp_math->tp_div == &instance_div ? instance_tdiv(tp_self, self, other) : (*(tp_self)->tp_math->tp_div)(self, other))
#define DeeType_INVOKE_MOD(tp_self, self, other)                   ((tp_self)->tp_math->tp_mod == &instance_mod ? instance_tmod(tp_self, self, other) : (*(tp_self)->tp_math->tp_mod)(self, other))
#define DeeType_INVOKE_SHL(tp_self, self, other)                   ((tp_self)->tp_math->tp_shl == &instance_shl ? instance_tshl(tp_self, self, other) : (*(tp_self)->tp_math->tp_shl)(self, other))
#define DeeType_INVOKE_SHR(tp_self, self, other)                   ((tp_self)->tp_math->tp_shr == &instance_shr ? instance_tshr(tp_self, self, other) : (*(tp_self)->tp_math->tp_shr)(self, other))
#define DeeType_INVOKE_AND(tp_self, self, other)                   ((tp_self)->tp_math->tp_and == &instance_and ? instance_tand(tp_self, self, other) : (*(tp_self)->tp_math->tp_and)(self, other))
#define DeeType_INVOKE_OR(tp_self, self, other)                    ((tp_self)->tp_math->tp_or == &instance_or ? instance_tor(tp_self, self, other) : (*(tp_self)->tp_math->tp_or)(self, other))
#define DeeType_INVOKE_XOR(tp_self, self, other)                   ((tp_self)->tp_math->tp_xor == &instance_xor ? instance_txor(tp_self, self, other) : (*(tp_self)->tp_math->tp_xor)(self, other))
#define DeeType_INVOKE_POW(tp_self, self, other)                   ((tp_self)->tp_math->tp_pow == &instance_pow ? instance_tpow(tp_self, self, other) : (*(tp_self)->tp_math->tp_pow)(self, other))
#define DeeType_INVOKE_INC(tp_self, pself)                         ((tp_self)->tp_math->tp_inc == &instance_inc ? instance_tinc(tp_self, pself) : (*(tp_self)->tp_math->tp_inc)(pself))
#define DeeType_INVOKE_DEC(tp_self, pself)                         ((tp_self)->tp_math->tp_dec == &instance_dec ? instance_tdec(tp_self, pself) : (*(tp_self)->tp_math->tp_dec)(pself))
#define DeeType_INVOKE_IADD(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_add == &instance_iadd ? instance_tiadd(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_add)(pself, other))
#define DeeType_INVOKE_ISUB(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_sub == &instance_isub ? instance_tisub(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_sub)(pself, other))
#define DeeType_INVOKE_IMUL(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_mul == &instance_imul ? instance_timul(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_mul)(pself, other))
#define DeeType_INVOKE_IDIV(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_div == &instance_idiv ? instance_tidiv(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_div)(pself, other))
#define DeeType_INVOKE_IMOD(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_mod == &instance_imod ? instance_timod(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_mod)(pself, other))
#define DeeType_INVOKE_ISHL(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_shl == &instance_ishl ? instance_tishl(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_shl)(pself, other))
#define DeeType_INVOKE_ISHR(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_shr == &instance_ishr ? instance_tishr(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_shr)(pself, other))
#define DeeType_INVOKE_IAND(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_and == &instance_iand ? instance_tiand(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_and)(pself, other))
#define DeeType_INVOKE_IOR(tp_self, pself, other)                  ((tp_self)->tp_math->tp_inplace_or == &instance_ior ? instance_tior(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_or)(pself, other))
#define DeeType_INVOKE_IXOR(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_xor == &instance_ixor ? instance_tixor(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_xor)(pself, other))
#define DeeType_INVOKE_IPOW(tp_self, pself, other)                 ((tp_self)->tp_math->tp_inplace_pow == &instance_ipow ? instance_tipow(tp_self, pself, other) : (*(tp_self)->tp_math->tp_inplace_pow)(pself, other))
#define DeeType_INVOKE_HASH(tp_self, self)                         ((tp_self)->tp_cmp->tp_hash == &instance_hash ? instance_thash(tp_self, self) : (tp_self)->tp_cmp->tp_hash == &instance_builtin_hash ? instance_builtin_thash(tp_self, self) : (*(tp_self)->tp_cmp->tp_hash)(self))
#define DeeType_INVOKE_EQ(tp_self, self, other)                    ((tp_self)->tp_cmp->tp_eq == &instance_eq ? instance_teq(tp_self, self, other) : (tp_self)->tp_cmp->tp_eq == &instance_builtin_eq ? instance_builtin_teq(tp_self, self, other) : (*(tp_self)->tp_cmp->tp_eq)(self, other))
#define DeeType_INVOKE_NE(tp_self, self, other)                    ((tp_self)->tp_cmp->tp_ne == &instance_ne ? instance_tne(tp_self, self, other) : (tp_self)->tp_cmp->tp_ne == &instance_builtin_ne ? instance_builtin_tne(tp_self, self, other) : (*(tp_self)->tp_cmp->tp_ne)(self, other))
#define DeeType_INVOKE_LO(tp_self, self, other)                    ((tp_self)->tp_cmp->tp_lo == &instance_lo ? instance_tlo(tp_self, self, other) : (tp_self)->tp_cmp->tp_lo == &instance_builtin_lo ? instance_builtin_tlo(tp_self, self, other) : (*(tp_self)->tp_cmp->tp_lo)(self, other))
#define DeeType_INVOKE_LE(tp_self, self, other)                    ((tp_self)->tp_cmp->tp_le == &instance_le ? instance_tle(tp_self, self, other) : (tp_self)->tp_cmp->tp_le == &instance_builtin_le ? instance_builtin_tle(tp_self, self, other) : (*(tp_self)->tp_cmp->tp_le)(self, other))
#define DeeType_INVOKE_GR(tp_self, self, other)                    ((tp_self)->tp_cmp->tp_gr == &instance_gr ? instance_tgr(tp_self, self, other) : (tp_self)->tp_cmp->tp_gr == &instance_builtin_gr ? instance_builtin_tgr(tp_self, self, other) : (*(tp_self)->tp_cmp->tp_gr)(self, other))
#define DeeType_INVOKE_GE(tp_self, self, other)                    ((tp_self)->tp_cmp->tp_ge == &instance_ge ? instance_tge(tp_self, self, other) : (tp_self)->tp_cmp->tp_ge == &instance_builtin_ge ? instance_builtin_tge(tp_self, self, other) : (*(tp_self)->tp_cmp->tp_ge)(self, other))
#define DeeType_INVOKE_ITER(tp_self, self)                         ((tp_self)->tp_seq->tp_iter_self == &instance_iter ? instance_titer(tp_self, self) : (*(tp_self)->tp_seq->tp_iter_self)(self))
#define DeeType_INVOKE_SIZE(tp_self, self)                         ((tp_self)->tp_seq->tp_size == &instance_size ? instance_tsize(tp_self, self) : (*(tp_self)->tp_seq->tp_size)(self))
#define DeeType_INVOKE_CONTAINS(tp_self, self, other)              ((tp_self)->tp_seq->tp_contains == &instance_contains ? instance_tcontains(tp_self, self, other) : (*(tp_self)->tp_seq->tp_contains)(self, other))
#define DeeType_INVOKE_GETITEM(tp_self, self, index)               ((tp_self)->tp_seq->tp_get == &instance_getitem ? instance_tgetitem(tp_self, self, index) : (*(tp_self)->tp_seq->tp_get)(self, index))
#define DeeType_INVOKE_DELITEM(tp_self, self, index)               ((tp_self)->tp_seq->tp_del == &instance_delitem ? instance_tdelitem(tp_self, self, index) : (*(tp_self)->tp_seq->tp_del)(self, index))
#define DeeType_INVOKE_SETITEM(tp_self, self, index, value)        ((tp_self)->tp_seq->tp_set == &instance_setitem ? instance_tsetitem(tp_self, self, index, value) : (*(tp_self)->tp_seq->tp_set)(self, index, value))
#define DeeType_INVOKE_GETRANGE(tp_self, self, start, end)         ((tp_self)->tp_seq->tp_range_get == &instance_getrange ? instance_tgetrange(tp_self, self, start, end) : (*(tp_self)->tp_seq->tp_range_get)(self, start, end))
#define DeeType_INVOKE_DELRANGE(tp_self, self, start, end)         ((tp_self)->tp_seq->tp_range_del == &instance_delrange ? instance_tdelrange(tp_self, self, start, end) : (*(tp_self)->tp_seq->tp_range_del)(self, start, end))
#define DeeType_INVOKE_SETRANGE(tp_self, self, start, end, values) ((tp_self)->tp_seq->tp_range_set == &instance_setrange ? instance_tsetrange(tp_self, self, start, end, values) : (*(tp_self)->tp_seq->tp_range_set)(self, start, end, values))
#define DeeType_INVOKE_GETATTR(tp_self, self, name)                ((tp_self)->tp_attr->tp_getattr == &instance_getattr ? instance_tgetattr(tp_self, self, name) : (*(tp_self)->tp_attr->tp_getattr)(self, name))
#define DeeType_INVOKE_DELATTR(tp_self, self, name)                ((tp_self)->tp_attr->tp_delattr == &instance_delattr ? instance_tdelattr(tp_self, self, name) : (*(tp_self)->tp_attr->tp_delattr)(self, name))
#define DeeType_INVOKE_SETATTR(tp_self, self, name, value)         ((tp_self)->tp_attr->tp_setattr == &instance_setattr ? instance_tsetattr(tp_self, self, name, value) : (*(tp_self)->tp_attr->tp_setattr)(self, name, value))
#define DeeType_INVOKE_ENTER(tp_self, self)                        ((tp_self)->tp_with->tp_enter == &instance_enter ? instance_tenter(tp_self, self) : (*(tp_self)->tp_with->tp_enter)(self))
#define DeeType_INVOKE_LEAVE(tp_self, self)                        ((tp_self)->tp_with->tp_leave == &instance_leave ? instance_tleave(tp_self, self) : (*(tp_self)->tp_with->tp_leave)(self))
#else /* DEFINE_TYPED_OPERATORS */
#define DeeType_INVOKE_ASSIGN(tp_self, self, other)                (*(tp_self)->tp_init.tp_assign)(self, other)
#define DeeType_INVOKE_MOVEASSIGN(tp_self, self, other)            (*(tp_self)->tp_init.tp_move_assign)(self, other)
#define DeeType_INVOKE_STR(tp_self, self)                          (*(tp_self)->tp_cast.tp_str)(self)
#define DeeType_INVOKE_REPR(tp_self, self)                         (*(tp_self)->tp_cast.tp_repr)(self)
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
#define DeeType_INVOKE_INC(tp_self, pself)                         (*(tp_self)->tp_math->tp_inc)(pself)
#define DeeType_INVOKE_DEC(tp_self, pself)                         (*(tp_self)->tp_math->tp_dec)(pself)
#define DeeType_INVOKE_IADD(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_add)(pself, other)
#define DeeType_INVOKE_ISUB(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_sub)(pself, other)
#define DeeType_INVOKE_IMUL(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_mul)(pself, other)
#define DeeType_INVOKE_IDIV(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_div)(pself, other)
#define DeeType_INVOKE_IMOD(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_mod)(pself, other)
#define DeeType_INVOKE_ISHL(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_shl)(pself, other)
#define DeeType_INVOKE_ISHR(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_shr)(pself, other)
#define DeeType_INVOKE_IAND(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_and)(pself, other)
#define DeeType_INVOKE_IOR(tp_self, pself, other)                  (*(tp_self)->tp_math->tp_inplace_or)(pself, other)
#define DeeType_INVOKE_IXOR(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_xor)(pself, other)
#define DeeType_INVOKE_IPOW(tp_self, pself, other)                 (*(tp_self)->tp_math->tp_inplace_pow)(pself, other)
#define DeeType_INVOKE_HASH(tp_self, self)                         (*(tp_self)->tp_cmp->tp_hash)(self)
#define DeeType_INVOKE_EQ(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_eq)(self, other)
#define DeeType_INVOKE_NE(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_ne)(self, other)
#define DeeType_INVOKE_LO(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_lo)(self, other)
#define DeeType_INVOKE_LE(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_le)(self, other)
#define DeeType_INVOKE_GR(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_gr)(self, other)
#define DeeType_INVOKE_GE(tp_self, self, other)                    (*(tp_self)->tp_cmp->tp_ge)(self, other)
#define DeeType_INVOKE_ITER(tp_self, self)                         (*(tp_self)->tp_seq->tp_iter_self)(self)
#define DeeType_INVOKE_SIZE(tp_self, self)                         (*(tp_self)->tp_seq->tp_size)(self)
#define DeeType_INVOKE_CONTAINS(tp_self, self, other)              (*(tp_self)->tp_seq->tp_contains)(self, other)
#define DeeType_INVOKE_GETITEM(tp_self, self, index)               (*(tp_self)->tp_seq->tp_get)(self, index)
#define DeeType_INVOKE_DELITEM(tp_self, self, index)               (*(tp_self)->tp_seq->tp_del)(self, index)
#define DeeType_INVOKE_SETITEM(tp_self, self, index, value)        (*(tp_self)->tp_seq->tp_set)(self, index, value)
#define DeeType_INVOKE_GETRANGE(tp_self, self, start, end)         (*(tp_self)->tp_seq->tp_range_get)(self, start, end)
#define DeeType_INVOKE_DELRANGE(tp_self, self, start, end)         (*(tp_self)->tp_seq->tp_range_del)(self, start, end)
#define DeeType_INVOKE_SETRANGE(tp_self, self, start, end, values) (*(tp_self)->tp_seq->tp_range_set)(self, start, end, value)
#define DeeType_INVOKE_GETATTR(tp_self, self, name)                (*(tp_self)->tp_attr->tp_getattr)(self, name)
#define DeeType_INVOKE_DELATTR(tp_self, self, name)                (*(tp_self)->tp_attr->tp_delattr)(self, name)
#define DeeType_INVOKE_SETATTR(tp_self, self, name, value)         (*(tp_self)->tp_attr->tp_setattr)(self, name, value)
#define DeeType_INVOKE_ENTER(tp_self, self)                        (*(tp_self)->tp_with->tp_enter)(self)
#define DeeType_INVOKE_LEAVE(tp_self, self)                        (*(tp_self)->tp_with->tp_leave)(self)
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
INTERN NONNULL((1)) bool DCALL
type_inherit_constructors(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	if (!(self->tp_flags & TP_FINHERITCTOR) ||
	    (base = self->tp_base) == NULL)
		return false;
	type_inherit_constructors(base);
	ASSERT((base->tp_flags & TP_FVARIABLE) ==
	       (self->tp_flags & TP_FVARIABLE));
	Dee_DPRINTF("[RT] Inherit constructors from %q into %q\n",
	            base->tp_name, self->tp_name);
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
	self->tp_init.tp_assign      = base->tp_init.tp_assign;
	self->tp_init.tp_move_assign = base->tp_init.tp_move_assign;
	self->tp_init.tp_deepload    = base->tp_init.tp_deepload;
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
		ASSERT(!(object_type->tp_flags & TP_FVARIABLE));
		if (object_type->tp_init.tp_alloc.tp_free)
			result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
		else if (object_type->tp_flags & TP_FGC)
			result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
		else {
			result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
		}
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
			if (object_type->tp_init.tp_alloc.tp_free)
				(*object_type->tp_init.tp_alloc.tp_free)(result);
			else if (object_type->tp_flags & TP_FGC)
				DeeGCObject_Free(result);
			else {
				DeeObject_Free(result);
			}
			if (!type_inherit_constructors(object_type))
				goto err_not_implemented;
			if (object_type->tp_init.tp_alloc.tp_free)
				result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
			else if (object_type->tp_flags & TP_FGC)
				result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
			else {
				result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
			}
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
			DeeGC_Track(result);
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

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
		if (type_inherit_constructors(object_type)) {
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
		if (object_type->tp_init.tp_alloc.tp_free)
			result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
		else if (object_type->tp_flags & TP_FGC)
			result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
		else {
			result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
		}
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
			if (object_type->tp_init.tp_alloc.tp_free)
				(*object_type->tp_init.tp_alloc.tp_free)(result);
			else if (object_type->tp_flags & TP_FGC)
				DeeGCObject_Free(result);
			else {
				DeeObject_Free(result);
			}
			if (!type_inherit_constructors(object_type))
				goto err_not_implemented;
			if (object_type->tp_init.tp_alloc.tp_free)
				result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
			else if (object_type->tp_flags & TP_FGC)
				result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
			else {
				result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
			}
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
			DeeGC_Track(result);
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

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_NewKw(DeeTypeObject *object_type, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	ASSERT_OBJECT(object_type);
	ASSERT(DeeType_Check(object_type));
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
		if (type_inherit_constructors(object_type)) {
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
		if (object_type->tp_init.tp_alloc.tp_free)
			result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
		else if (object_type->tp_flags & TP_FGC)
			result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
		else {
			result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
		}
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
			if (object_type->tp_init.tp_alloc.tp_free)
				(*object_type->tp_init.tp_alloc.tp_free)(result);
			else if (object_type->tp_flags & TP_FGC)
				DeeGCObject_Free(result);
			else {
				DeeObject_Free(result);
			}
			if (!type_inherit_constructors(object_type))
				goto err_not_implemented;
			if (object_type->tp_init.tp_alloc.tp_free)
				result = (DREF DeeObject *)(*object_type->tp_init.tp_alloc.tp_alloc)();
			else if (object_type->tp_flags & TP_FGC)
				result = (DREF DeeObject *)DeeGCObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
			else {
				result = (DREF DeeObject *)DeeObject_Malloc(object_type->tp_init.tp_alloc.tp_instance_size);
			}
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
			DeeGC_Track(result);
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
	err_keywords_ctor_not_accepted(object_type, kw);
	goto err;
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
#define LOAD_TP_SELFP  ASSERT_OBJECT_TYPE_A(*pself, tp_self)
#define LOAD_ITER      DeeTypeObject *iter = tp_self
#define LOAD_ITERP     DeeTypeObject *iter = tp_self
#define GET_TP_SELF()  tp_self
#define GET_TP_PSELF() tp_self
#else /* DEFINE_TYPED_OPERATORS */
#define LOAD_TP_SELF   DeeTypeObject *tp_self; \
                       ASSERT_OBJECT(self);    \
                       tp_self = Dee_TYPE(self)
#define LOAD_TP_SELFP  DeeTypeObject *tp_self; \
                       ASSERT_OBJECT(*pself);  \
                       tp_self = Dee_TYPE(*pself)
#define LOAD_ITER      DeeTypeObject *iter; \
                       ASSERT_OBJECT(self); \
                       iter = Dee_TYPE(self)
#define LOAD_ITERP     DeeTypeObject *iter;   \
                       ASSERT_OBJECT(*pself); \
                       iter = Dee_TYPE(*pself)
#define GET_TP_SELF()  Dee_TYPE(self)
#define GET_TP_PSELF() Dee_TYPE(*pself)
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
			if (type_inherit_constructors(tp_self)) {
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
		if (tp_self->tp_init.tp_alloc.tp_free)
			result = (DREF DeeObject *)(*tp_self->tp_init.tp_alloc.tp_alloc)();
		else if (tp_self->tp_flags & TP_FGC)
			result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
		else {
			result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
		}
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
			if (tp_self->tp_init.tp_alloc.tp_free)
				(*tp_self->tp_init.tp_alloc.tp_free)(result);
			else if (tp_self->tp_flags & TP_FGC)
				DeeGCObject_Free(result);
			else {
				DeeObject_Free(result);
			}
			Dee_Decref(tp_self);
			result = NULL;
			goto done_endcopy;
		}
		/* Begin tracking the returned object if this is a GC type. */
		if (tp_self->tp_flags & TP_FGC)
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
		if (Dee_DeepCopyAddAssoc(result, self))
			goto err_result_endcopy;
		if unlikely((*tp_self->tp_init.tp_deepload)(result))
			goto err_result_endcopy;
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
		else if (tp_self->tp_flags & TP_FGC)
			result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
		else {
			result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
		}
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, tp_self);
		error = (*tp_self->tp_init.tp_alloc.tp_copy_ctor)(result, self);
		if unlikely(error)
			goto err_r;
		/* Begin tracking the returned object. */
		if (tp_self->tp_flags & TP_FGC)
			DeeGC_Track(result);
		return result;
	} else if (tp_self->tp_init.tp_alloc.tp_deep_ctor) {
		goto do_invoke_var_deep;
	} else {
		int error;
		ASSERT(!(tp_self->tp_flags & TP_FVARIABLE));
		if (tp_self->tp_init.tp_alloc.tp_free)
			result = (DREF DeeObject *)(*tp_self->tp_init.tp_alloc.tp_alloc)();
		else if (tp_self->tp_flags & TP_FGC)
			result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
		else {
			result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
		}
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
			if (tp_self->tp_init.tp_alloc.tp_free)
				(*tp_self->tp_init.tp_alloc.tp_free)(result);
			else if (tp_self->tp_flags & TP_FGC)
				DeeGCObject_Free(result);
			else {
				DeeObject_Free(result);
			}
			if (!type_inherit_constructors(tp_self))
				goto err_not_implemented;
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
			DeeGC_Track(result);
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


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_InplaceDeepCopy)(DREF DeeObject **__restrict pself) {
	DeeObject *objcopy, *old_object;
	old_object = *pself;
	ASSERT_OBJECT(old_object);
	objcopy = DeeObject_DeepCopy(old_object);
	if unlikely(!objcopy)
		goto err;
	Dee_Decref(old_object);
	*pself = objcopy;
	return 0;
err:
	return -1;
}

#ifndef CONFIG_NO_THREADS
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_InplaceDeepCopyWithLock(DREF DeeObject **__restrict pself,
                                  Dee_atomic_rwlock_t *__restrict plock) {
	DREF DeeObject *temp, *copy;

	/* Step #1: Extract the existing object. */
	atomic_rwlock_read(plock);
	temp = *pself;
	Dee_Incref(temp);
	atomic_rwlock_endread(plock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	atomic_rwlock_write(plock);
	temp   = *pself; /* Inherit */
	*pself = copy;   /* Inherit */
	atomic_rwlock_endwrite(plock);
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_XInplaceDeepCopyWithLock(DREF DeeObject **__restrict pself,
                                   Dee_atomic_rwlock_t *__restrict plock) {
	DREF DeeObject *temp, *copy;

	/* Step #1: Extract the existing object. */
	atomic_rwlock_read(plock);
	temp = *pself;
	if (!temp) {
		rwlock_endread(plock);
		goto done;
	}
	Dee_Incref(temp);
	atomic_rwlock_endread(plock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	atomic_rwlock_write(plock);
	temp   = *pself; /* Inherit */
	*pself = copy;   /* Inherit */
	atomic_rwlock_endwrite(plock);
	Dee_XDecref(temp);
done:
	return 0;
err:
	return -1;
}
#endif
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(int, Assign, (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(some_object);
	if (tp_self->tp_init.tp_assign) {
do_assign:
		return DeeType_INVOKE_ASSIGN(tp_self, self, some_object);
	}
	if (type_inherit_constructors(tp_self)) {
		if (tp_self->tp_init.tp_assign)
			goto do_assign;
	}
	return err_unimplemented_operator(GET_TP_SELF(), OPERATOR_ASSIGN);
}

DEFINE_OPERATOR(int, MoveAssign, (DeeObject *self, DeeObject *other)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(other);
	if (!(tp_self->tp_flags & TP_FMOVEANY) &&
	    DeeObject_AssertType(other, tp_self))
		goto err;
	if (tp_self->tp_init.tp_move_assign) {
do_move_assign:
		return DeeType_INVOKE_MOVEASSIGN(tp_self, self, other);
	}
	if (tp_self->tp_init.tp_assign) {
do_assign:
		return DeeType_INVOKE_ASSIGN(tp_self, self, other);
	}
	if (type_inherit_constructors(tp_self)) {
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
LOCAL bool DCALL
repr_contains(struct trepr_frame *chain, DeeTypeObject *tp, DeeObject *ob)
#else /* DEFINE_TYPED_OPERATORS */
LOCAL bool DCALL
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

#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field)          \
	INTERN NONNULL((1)) bool DCALL                                 \
	name(DeeTypeObject *__restrict self) {                         \
		DeeTypeObject *base = DeeType_Base(self);                  \
		if (!base ||                                               \
		    (!base->field && !name(base)))                         \
			return false;                                          \
		Dee_DPRINTF("[RT] Inherit `" opname "' from %q into %q\n", \
		            base->tp_name, self->tp_name);                 \
		self->field = base->field;                                 \
		return true;                                               \
	}
#define DEFINE_TYPE_INHERIT_FUNCTION2(name, opname, field, field2) \
	INTERN NONNULL((1)) bool DCALL                                 \
	name(DeeTypeObject *__restrict self) {                         \
		DeeTypeObject *base = DeeType_Base(self);                  \
		if (!base ||                                               \
		    (!base->field && !base->field2 && !name(base)))        \
			return false;                                          \
		Dee_DPRINTF("[RT] Inherit `" opname "' from %q into %q\n", \
		            base->tp_name, self->tp_name);                 \
		self->field  = base->field;                                \
		self->field2 = base->field2;                               \
		return true;                                               \
	}
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_str, "operator str", tp_cast.tp_str)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_repr, "operator repr", tp_cast.tp_repr)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_bool, "operator bool", tp_cast.tp_bool)
DEFINE_TYPE_INHERIT_FUNCTION2(type_inherit_call, "operator call", tp_call, tp_call_kw)
#undef DEFINE_TYPE_INHERIT_FUNCTION2
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(DREF DeeObject *, Str, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_str &&
		         !type_inherit_str(tp_self))
	goto missing;
	/* Handle string-repr recursion for GC objects. */
	if (tp_self->tp_flags & TP_FGC) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread;
		this_thread = DeeThread_Self();
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
		result                  = DeeType_INVOKE_STR(tp_self, self);
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
	if unlikely(!tp_self->tp_cast.tp_repr &&
	            !type_inherit_repr(tp_self))
		goto missing;
	/* Handle string-repr recursion for GC objects. */
	if (tp_self->tp_flags & TP_FGC) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread;
		this_thread = DeeThread_Self();
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
		result                   = DeeType_INVOKE_REPR(tp_self, self);
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

#undef Xrepr_frame

DEFINE_OPERATOR(int, Bool, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	/* _very_ likely case: `self' is one of the boolean constants
	 *  -> In this case, we return the result immediately! */
	if (self == Dee_True)
		return 1;
	if (self == Dee_False)
		return 0;
	LOAD_TP_SELF;
	if likely(tp_self->tp_cast.tp_bool ||
	          type_inherit_bool(tp_self))
		return DeeType_INVOKE_BOOL(tp_self, self);
	return err_unimplemented_operator(tp_self, OPERATOR_BOOL);
}

DEFINE_OPERATOR(DREF DeeObject *, Call,
               (DeeObject *self, size_t argc, DeeObject *const *argv)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_call)
			return DeeType_INVOKE_CALL(tp_self, self, argc, argv);
		if (tp_self->tp_call_kw)
			return DeeType_INVOKE_CALLKW(tp_self, self, argc, argv, NULL);
	} while (type_inherit_call(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
DEFINE_OPERATOR(DREF DeeObject *, CallTuple,
               (DeeObject *self, DeeObject *args)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT_TYPE_EXACT(args, &DeeTuple_Type);
	if (tp_self == &DeeFunction_Type)
		return DeeFunction_CallTuple((DeeFunctionObject *)self, args);
	do {
		if (tp_self->tp_call)
			return DeeType_INVOKE_CALL(tp_self, self,
			                           DeeTuple_SIZE(args),
			                           DeeTuple_ELEM(args));
		if (tp_self->tp_call_kw)
			return DeeType_INVOKE_CALLKW(tp_self, self,
			                             DeeTuple_SIZE(args),
			                             DeeTuple_ELEM(args),
			                             NULL);
	} while (type_inherit_call(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, CallTupleKw,
               (DeeObject *self, DeeObject *args, DeeObject *kw)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT_TYPE_EXACT(args, &DeeTuple_Type);
	if (tp_self == &DeeFunction_Type)
		return DeeFunction_CallTupleKw((DeeFunctionObject *)self, args, kw);
	do {
		if (tp_self->tp_call_kw)
			return DeeType_INVOKE_CALLKW(tp_self, self,
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
					if unlikely(kw_length == (size_t)-1)
						goto err;
					if (kw_length != 0)
						goto err_no_keywords;
				}
			}
			return DeeType_INVOKE_CALL(tp_self, self,
			                           DeeTuple_SIZE(args),
			                           DeeTuple_ELEM(args));
		}
	} while (type_inherit_call(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
err:
	return NULL;
err_no_keywords:
	err_keywords_not_accepted(tp_self, kw);
	goto err;
}

#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
DEFINE_OPERATOR(DREF DeeObject *, CallTuple,
                (DeeObject *self, DeeObject *args)) {
	return DeeObject_Call(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}
DEFINE_OPERATOR(DREF DeeObject *, CallTupleKw,
                (DeeObject *self, DeeObject *args, DeeObject *kw)) {
	return DeeObject_CallKw(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, CallKw,
                (DeeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_call_kw)
			return DeeType_INVOKE_CALLKW(tp_self, self, argc, argv, kw);
		if (tp_self->tp_call) {
			/* Object doesn't support keyword arguments. */
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
			return DeeType_INVOKE_CALL(tp_self, self, argc, argv);
		}
	} while (type_inherit_call(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_CALL);
err:
	return NULL;
err_no_keywords:
	err_keywords_not_accepted(tp_self, kw);
	goto err;
}

#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED DREF DeeObject *DCALL
DeeObject_TThisCall(DeeTypeObject *tp_self,
                    DeeObject *self, DeeObject *this_arg,
                    size_t argc, DeeObject *const *argv);
INTERN WUNUSED DREF DeeObject *DCALL
DeeObject_TThisCallKw(DeeTypeObject *tp_self,
                      DeeObject *self, DeeObject *this_arg,
                      size_t argc, DeeObject *const *argv,
                      DeeObject *kw);
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, ThisCall,
                (DeeObject *self, DeeObject *this_arg,
                 size_t argc, DeeObject *const *argv)) {
	DREF DeeObject *full_args, *result;
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
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertType(this_arg, ((DeeClsMethodObject *)self)->cm_type))
			goto err;
		return (*((DeeClsMethodObject *)self)->cm_func)(this_arg, argc, argv);
	}
	if (GET_TP_SELF() == &DeeKwClsMethod_Type) {
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertType(this_arg, ((DeeKwClsMethodObject *)self)->cm_type))
			goto err;
		return (*((DeeKwClsMethodObject *)self)->cm_func)(this_arg, argc, argv, NULL);
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
	result = DeeObject_Call(self,
	                        DeeTuple_SIZE(full_args),
	                        DeeTuple_ELEM(full_args));
#endif /* !DEFINE_TYPED_OPERATORS */
	DeeTuple_DecrefSymbolic(full_args);
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, ThisCallKw,
                (DeeObject *self, DeeObject *this_arg,
                 size_t argc, DeeObject *const *argv,
                 DeeObject *kw)) {
	DREF DeeObject *full_args, *result;
	ASSERT_OBJECT(self);
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
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertType(this_arg, ((DeeKwClsMethodObject *)self)->cm_type))
			goto err;
		return (*((DeeKwClsMethodObject *)self)->cm_func)(this_arg, argc, argv, kw);
	}
	if (GET_TP_SELF() == &DeeClsMethod_Type) {
		/* Must ensure proper typing of the this-argument. */
		if (DeeObject_AssertType(this_arg, ((DeeClsMethodObject *)self)->cm_type))
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
		return (*((DeeClsMethodObject *)self)->cm_func)(this_arg, argc, argv);
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
	result = DeeObject_CallKw(self,
	                          DeeTuple_SIZE(full_args),
	                          DeeTuple_ELEM(full_args),
	                          kw);
#endif /* !DEFINE_TYPED_OPERATORS */
	DeeTuple_DecrefSymbolic(full_args);
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
DeeObject_Callf(DeeObject *__restrict self,
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

#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
type_inherit_hash(DeeTypeObject *__restrict self) {
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
#endif /* !DEFINE_TYPED_OPERATORS */


WUNUSED /*ATTR_PURE*/
DEFINE_OPERATOR(dhash_t, Hash, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_cmp && tp_self->tp_cmp->tp_hash)
			return DeeType_INVOKE_HASH(tp_self, self);
	} while (type_inherit_hash(tp_self));
	return DeeObject_HashGeneric(self);
}

#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED /*ATTR_PURE*/ NONNULL((1)) Dee_hash_t
(DCALL DeeObject_Hashv)(DeeObject *const *__restrict object_vector,
                        size_t object_count) {
	size_t i;
	Dee_hash_t result = 0;
	for (i = 0; i < object_count; ++i) {
		result ^= DeeObject_Hash(object_vector[i]);
	}
	return result;
}
#endif /* DEFINE_TYPED_OPERATORS */


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC NONNULL((1, 2)) void DCALL
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

PUBLIC NONNULL((1)) void DCALL
DeeObject_Clear(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
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

PUBLIC void DCALL
DeeObject_PClear(DeeObject *__restrict self,
                 unsigned int gc_priority) {
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	if unlikely(gc_priority == Dee_GC_PRIORITY_LATE) {
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
				instance_tpclear(tp_self, self, gc_priority);
			} else {
				(*tp_self->tp_gc->tp_pclear)(self, gc_priority);
			}
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
}
#endif /* !DEFINE_TYPED_OPERATORS */


#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
type_inherit_int(DeeTypeObject *__restrict self) {
	DeeTypeObject *base = DeeType_Base(self);
	struct type_math *base_math;
	if (!base ||
	    (((base_math = base->tp_math) == NULL ||
	      (!base_math->tp_int && !base_math->tp_int32 &&
	       !base_math->tp_int64 && !base_math->tp_double)) &&
	     !type_inherit_int(base)))
		return false;
	Dee_DPRINTF("[RT] Inherit `operator int' from %q into %q\n",
	            base->tp_name, self->tp_name);
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
#endif /* !DEFINE_TYPED_OPERATORS */



DEFINE_OPERATOR(int, GetInt32,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 int32_t *__restrict result)) {
	int error;
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_math) {
			if (tp_self->tp_math->tp_int32)
				return DeeType_INVOKE_INT32(tp_self, self, result);
			if (tp_self->tp_math->tp_int64) {
				int64_t val64;
				error = DeeType_INVOKE_INT64(tp_self, self, &val64);
				if unlikely(error < 0)
					goto err;
				if (error == INT_SIGNED) {
					if unlikely(val64 < INT32_MIN || val64 > INT32_MAX) {
						if (val64 > 0) {
							*result = (int32_t)((uint32_t)(uint64_t)val64);
							return INT_UNSIGNED;
						}
						if (tp_self->tp_flags & TP_FTRUNCATE) {
							*result = (int32_t)val64;
							return val64 < INT32_MIN ? INT_SIGNED : INT_UNSIGNED;
						}
						return err_integer_overflow(self, 32, val64 > 0);
					}
				} else {
					if unlikely((uint64_t)val64 > UINT32_MAX) {
						if (tp_self->tp_flags & TP_FTRUNCATE) {
							*result = (uint32_t)val64;
							return INT_UNSIGNED;
						}
						return err_integer_overflow(self, 32, true);
					}
				}
				*result = (int32_t)(uint64_t)val64;
				return error;
			}
			if (tp_self->tp_math->tp_int) {
				/* Cast to integer, then read its value. */
				DREF DeeObject *intob;
				intob = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!intob)
					goto err;
				error = DeeInt_As32(intob, result);
				Dee_Decref(intob);
				return error;
			}
			if (tp_self->tp_math->tp_double) {
				double resflt;
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error < 0)
					goto err;
				if unlikely(resflt < INT32_MIN || resflt > UINT32_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
						if (resflt < 0) {
							*result = (int32_t)resflt;
							return INT_SIGNED;
						} else {
							*result = (uint32_t)resflt;
							return INT_UNSIGNED;
						}
					}
					return err_integer_overflow(self, 32, resflt > 0);
				}
				if (resflt < 0) {
					*result = (int32_t)resflt;
					return INT_SIGNED;
				}
				*result = (int32_t)(uint32_t)resflt;
				return INT_UNSIGNED;
			}
		}
	} while (type_inherit_int(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}

DEFINE_OPERATOR(int, GetInt64,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 int64_t *__restrict result)) {
	int error;
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_math) {
			if (tp_self->tp_math->tp_int64)
				return DeeType_INVOKE_INT64(tp_self, self, result);
			if (tp_self->tp_math->tp_int32) {
				int32_t val32;
				error = DeeType_INVOKE_INT32(tp_self, self, &val32);
				if unlikely(error < 0)
					goto err;
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
				intob = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!intob)
					goto err;
				error = DeeInt_As64(intob, result);
				Dee_Decref(intob);
				return error;
			}
			if (tp_self->tp_math->tp_double) {
				double resflt;
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error)
					goto err;
				if unlikely(resflt < INT64_MIN || resflt > UINT64_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
						if (resflt < 0) {
							*result = (int64_t)resflt;
							return INT_SIGNED;
						} else {
							*result = (uint64_t)resflt;
							return INT_UNSIGNED;
						}
					}
					return err_integer_overflow(self, 64, resflt > 0);
				}
				if (resflt < 0) {
					*result = (int64_t)resflt;
					return INT_SIGNED;
				}
				*result = (int64_t)(uint64_t)resflt;
				return INT_UNSIGNED;
			}
		}
	} while (type_inherit_int(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(int, GetInt128,
                (DeeObject *RESTRICT_IF_NOTYPE self,
                 dint128_t *__restrict result)) {
	int error;
	LOAD_TP_SELF;
	if (tp_self == &DeeInt_Type)
		return DeeInt_As128(self, result);
	do {
		if (tp_self->tp_math) {
			if (tp_self->tp_math->tp_int) {
				/* Cast to integer, then read its value. */
				DREF DeeObject *intob;
				intob = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!intob)
					goto err;
				error = DeeInt_As128(intob, result);
				Dee_Decref(intob);
				return error;
			}
			if (tp_self->tp_math->tp_int64) {
				DUINT128_GETS64(*result)[DEE_INT128_MS64] = 0;
				error = DeeType_INVOKE_INT64(tp_self, self, &DUINT128_GETS64(*result)[DEE_INT128_LS64]);
				if (error == INT_SIGNED && DUINT128_GETS64(*result)[DEE_INT128_LS64] < 0)
					DUINT128_GETS64(*result)[DEE_INT128_MS64] = -1;
				return error;
			}
			if (tp_self->tp_math->tp_int32) {
				int32_t val32;
				DUINT128_GETS64(*result)[DEE_INT128_MS64] = 0;
				error = DeeType_INVOKE_INT32(tp_self, self, &val32);
				if unlikely(error < 0)
					goto err;
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
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error < 0)
					goto err;
				DUINT128_GETS64(*result)[DEE_INT128_MS64] = 0;
				if unlikely(resflt < INT64_MIN || resflt > UINT64_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
						if (resflt < 0) {
							DUINT128_GETS64(*result)[DEE_INT128_LS64] = (int64_t)resflt;
							DUINT128_GETS64(*result)[DEE_INT128_MS64] = -1;
							return INT_SIGNED;
						} else {
							DUINT128_GETS64(*result)[DEE_INT128_LS64] = (uint64_t)resflt;
							return INT_UNSIGNED;
						}
					}
					return err_integer_overflow(self, 64, resflt > 0);
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
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}
#endif


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC int (DCALL DeeObject_AsUInt32)(DeeObject *__restrict self,
                                      uint32_t *__restrict result) {
	int error;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	do {
		if (tp_self->tp_math) {
			if (tp_self->tp_math->tp_int32) {
				error = DeeType_INVOKE_INT32(tp_self, self, (int32_t *)result);
				if unlikely(error < 0)
					goto err;
				if unlikely(error == INT_SIGNED && *(int32_t *)result < 0) {
					if (tp_self->tp_flags & TP_FTRUNCATE)
						return 0;
neg_overflow:
					return err_integer_overflow(self, 32, 0);
				}
				return 0;
			}
			if (tp_self->tp_math->tp_int64) {
				int64_t val64;
				error = DeeType_INVOKE_INT64(tp_self, self, &val64);
				if (error < 0)
					goto err;
				if unlikely(error == INT_SIGNED && val64 < 0) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
return_trunc64:
						*result = (uint32_t)val64;
						return 0;
					}
					goto neg_overflow;
				}
				if unlikely((uint64_t)val64 > (uint64_t)UINT32_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE)
						goto return_trunc64;
					return err_integer_overflow(self, 32, 1);
				}
				*result = (int32_t)(uint64_t)val64;
				return 0;
			}
			if (tp_self->tp_math->tp_int) {
				/* Cast to integer, then read its value. */
				DREF DeeObject *intob;
				intob = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!intob)
					goto err;
				error = DeeInt_AsU32(intob, result);
				Dee_Decref(intob);
				return error;
			}
			if (tp_self->tp_math->tp_double) {
				double resflt;
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error < 0)
					goto err;
				if unlikely(resflt < 0 || resflt > UINT32_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
						*result = (uint32_t)resflt;
						return 0;
					}
					return err_integer_overflow(self, 32, resflt > 0);
				}
				*result = (uint32_t)resflt;
				return 0;
			}
		}
	} while (type_inherit_int(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
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
				error = DeeType_INVOKE_INT32(tp_self, self, result);
				if unlikely(error < 0)
					goto err;
				if unlikely(error == INT_UNSIGNED && (uint32_t)*result > INT32_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE)
						return 0;
					return err_integer_overflow(self, 32, 1);
				}
				return 0;
			}
			if (tp_self->tp_math->tp_int64) {
				int64_t val64;
				error = DeeType_INVOKE_INT64(tp_self, self, &val64);
				if unlikely(error < 0)
					goto err;
				if (error == INT_SIGNED) {
					if unlikely(val64 < INT32_MIN || val64 > INT32_MAX) {
						if (tp_self->tp_flags & TP_FTRUNCATE) {
return_trunc64:
							*result = (int32_t)val64;
							return 0;
						}
						return err_integer_overflow(self, 32, val64 > 0);
					}
					*result = (int32_t)val64;
				} else {
					if unlikely((uint64_t)val64 > INT32_MAX) {
						if (tp_self->tp_flags & TP_FTRUNCATE)
							goto return_trunc64;
						return err_integer_overflow(self, 32, 1);
					}
					*result = (int32_t)(uint64_t)val64;
				}
				return 0;
			}
			if (tp_self->tp_math->tp_int) {
				/* Cast to integer, then read its value. */
				DREF DeeObject *intob;
				intob = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!intob)
					goto err;
				error = DeeInt_AsS32(intob, result);
				Dee_Decref(intob);
				return error;
			}
			if (tp_self->tp_math->tp_double) {
				double resflt;
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error < 0)
					goto err;
				if unlikely(resflt < INT32_MIN || resflt > INT32_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
						*result = (int32_t)resflt;
						return 0;
					}
					return err_integer_overflow(self, 32, resflt > 0);
				}
				*result = (int32_t)resflt;
				return 0;
			}
		}
	} while (type_inherit_int(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
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
				error = DeeType_INVOKE_INT64(tp_self, self, (int64_t *)result);
				if unlikely(error < 0)
					goto err;
				if unlikely(error == INT_SIGNED && *(int64_t *)result < 0) {
					if (tp_self->tp_flags & TP_FTRUNCATE)
						return 0;
neg_overflow:
					return err_integer_overflow(self, 64, false);
				}
				return 0;
			}
			if (tp_self->tp_math->tp_int32) {
				int32_t val32;
				error = DeeType_INVOKE_INT32(tp_self, self, &val32);
				if unlikely(error < 0)
					goto err;
				if unlikely(error == INT_SIGNED && val32 < 0) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
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
				intob = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!intob)
					goto err;
				error = DeeInt_AsU64(intob, result);
				Dee_Decref(intob);
				return error;
			}
			if (tp_self->tp_math->tp_double) {
				double resflt;
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error)
					goto err;
				if unlikely(resflt < 0 || resflt > UINT64_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
						*result = (uint64_t)resflt;
						return 0;
					}
					return err_integer_overflow(self, 64, resflt > 0);
				}
				*result = (int64_t)resflt;
				return 0;
			}
		}
	} while (type_inherit_int(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_INT);
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
				error = DeeType_INVOKE_INT64(tp_self, self, result);
				if unlikely(error < 0)
					goto err;
				if unlikely(error == INT_UNSIGNED && (uint64_t)*result > INT64_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE)
						return 0;
					return err_integer_overflow(self, 64, true);
				}
				return 0;
			}
			if (tp_self->tp_math->tp_int32) {
				int32_t val32;
				error = DeeType_INVOKE_INT32(tp_self, self, &val32);
				if (error < 0)
					goto err;
				if (error == INT_SIGNED)
					*result = (int64_t)val32;
				else {
					*result = (int64_t)((uint64_t)(uint32_t)val32);
				}
				return 0;
			}
			if (tp_self->tp_math->tp_int) {
				/* Cast to integer, then read its value. */
				DREF DeeObject *intob;
				intob = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!intob)
					goto err;
				error = DeeInt_AsS64(intob, result);
				Dee_Decref(intob);
				return error;
			}
			if (tp_self->tp_math->tp_double) {
				double resflt;
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error < 0)
					goto err;
				if unlikely(resflt < INT64_MIN || resflt > INT64_MAX) {
					if (tp_self->tp_flags & TP_FTRUNCATE) {
						*result = (int64_t)resflt;
						return 0;
					}
					return err_integer_overflow(self, 64, resflt > 0);
				}
				*result = (int64_t)resflt;
				return 0;
			}
		}
	} while (type_inherit_int(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return -1;
}

PUBLIC int (DCALL DeeObject_AsInt128)(DeeObject *__restrict self,
                                      dint128_t *__restrict result) {
	int error = DeeObject_GetInt128(self, result);
	if (error == INT_UNSIGNED && DSINT128_ISNEG(*result))
		return err_integer_overflow(self, 128, true);
	return 0;
}

PUBLIC int (DCALL DeeObject_AsUInt128)(DeeObject *__restrict self,
                                       duint128_t *__restrict result) {
	int error = DeeObject_GetInt128(self, (dint128_t *)result);
	if (error == INT_SIGNED && DSINT128_ISNEG(*result))
		return err_integer_overflow(self, 128, false);
	return 0;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(int, AsDouble,
                (DeeObject *RESTRICT_IF_NOTYPE self,
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
				return DeeType_INVOKE_DOUBLE(tp_self, self, result);
			if (tp_self->tp_math->tp_int64) {
				error = DeeType_INVOKE_INT64(tp_self, self, &res.res64);
				if (error == INT_UNSIGNED)
					*result = (double)(uint64_t)res.res64;
				else {
					*result = (double)res.res64;
				}
				return error < 0 ? -1 : 0;
			}
			if (tp_self->tp_math->tp_int32) {
				error = DeeType_INVOKE_INT32(tp_self, self, &res.res32);
				if (error == INT_UNSIGNED)
					*result = (double)(uint32_t)res.res32;
				else {
					*result = (double)res.res32;
				}
				return error < 0 ? -1 : 0;
			}
			if (tp_self->tp_math->tp_int) {
				DREF DeeObject *temp_int;
				temp_int = DeeType_INVOKE_INT(tp_self, self);
				if unlikely(!temp_int)
					goto err;
				error = DeeInt_As64(temp_int, &res.res64);
				Dee_Decref(temp_int);
				if (error == INT_UNSIGNED)
					*result = (double)(uint64_t)res.res64;
				else {
					*result = (double)res.res64;
				}
				return error < 0 ? -1 : 0;
			}
		}
	} while (type_inherit_int(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_FLOAT);
err:
	return -1;
}


#ifndef DEFINE_TYPED_OPERATORS
INTERN WUNUSED NONNULL((1)) DeeTypeObject *DCALL
type_get_int_caster(DeeTypeObject *__restrict start) {
	while (!DeeType_HasPrivateOperator(start, OPERATOR_INT))
		start = DeeType_Base(start);
	return start;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_GetInt8(DeeObject *__restrict self,
                  int8_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_GetInt32(self, &val32);
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

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_GetInt16(DeeObject *__restrict self,
                   int16_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_GetInt32(self, &val32);
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

PUBLIC int (DCALL DeeObject_AsInt8)(DeeObject *__restrict self,
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

PUBLIC int (DCALL DeeObject_AsInt16)(DeeObject *__restrict self,
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
	*result = (int8_t)val32;
	return 0;
err:
	return -1;
}

PUBLIC int (DCALL DeeObject_AsUInt8)(DeeObject *__restrict self,
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

PUBLIC int (DCALL DeeObject_AsUInt16)(DeeObject *__restrict self,
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

DEFINE_OPERATOR(DREF DeeObject *, Int, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_math) {
			if (tp_self->tp_math->tp_int)
				return DeeType_INVOKE_INT(tp_self, self);
			if (tp_self->tp_math->tp_int64) {
				int64_t val64;
				int error;
				error = DeeType_INVOKE_INT64(tp_self, self, &val64);
				if unlikely(error < 0)
					goto err;
				if (error == INT_SIGNED)
					return DeeInt_NewS64(val64);
				return DeeInt_NewU64((uint64_t)val64);
			}
			if (tp_self->tp_math->tp_int32) {
				int32_t val32;
				int error;
				error = DeeType_INVOKE_INT32(tp_self, self, &val32);
				if unlikely(error < 0)
					goto err;
				if (error == INT_SIGNED)
					return DeeInt_NewS32(val32);
				return DeeInt_NewU32((uint32_t)val32);
			}
			if (tp_self->tp_math->tp_double) {
				int error;
				double resflt;
				error = DeeType_INVOKE_DOUBLE(tp_self, self, &resflt);
				if unlikely(error < 0)
					goto err;
				if (resflt < INT64_MIN || resflt > UINT64_MAX) {
					if (!(tp_self->tp_flags & TP_FTRUNCATE)) {
						err_integer_overflow(self, 64, resflt > 0);
						goto err;
					}
				}
				return DeeInt_NewS64((int64_t)resflt);
			}
		}
	} while (type_inherit_int(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_INT);
err:
	return NULL;
}



#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field)                      \
	INTERN NONNULL((1)) bool DCALL                                             \
	name(DeeTypeObject *__restrict self) {                                     \
		struct type_math *base_math;                                           \
		DeeTypeObject *base = DeeType_Base(self);                              \
		if (!base ||                                                           \
		    (((base_math = base->tp_math) == NULL ||                           \
		      (!base_math->tp_##field &&                                       \
		       !base_math->tp_inplace_##field)) &&                             \
		     !name(base)))                                                     \
			return false;                                                      \
		Dee_DPRINTF("[RT] Inherit `" opname "' from %q into %q\n",             \
		            base->tp_name, self->tp_name);                             \
		if (self->tp_math) {                                                   \
			self->tp_math->tp_##field         = base_math->tp_##field;         \
			self->tp_math->tp_inplace_##field = base_math->tp_inplace_##field; \
		} else {                                                               \
			self->tp_math = base_math;                                         \
		}                                                                      \
		return true;                                                           \
	}
#define DEFINE_TYPE_INHERIT_FUNCTION1(name, opname, field)         \
	INTERN NONNULL((1)) bool DCALL                                 \
	name(DeeTypeObject *__restrict self) {                         \
		struct type_math *base_math;                               \
		DeeTypeObject *base = DeeType_Base(self);                  \
		if (!base ||                                               \
		    (((base_math = base->tp_math) == NULL ||               \
		      !base_math->tp_##field) &&                           \
		     !name(base)))                                         \
			return false;                                          \
		Dee_DPRINTF("[RT] Inherit `" opname "' from %q into %q\n", \
		            base->tp_name, self->tp_name);                 \
		base_math = base->tp_math;                                 \
		if (self->tp_math) {                                       \
			self->tp_math->tp_##field = base_math->tp_##field;     \
		} else {                                                   \
			self->tp_math = base_math;                             \
		}                                                          \
		return true;                                               \
	}
DEFINE_TYPE_INHERIT_FUNCTION1(type_inherit_inv, "operator inv", inv)
DEFINE_TYPE_INHERIT_FUNCTION1(type_inherit_pos, "operator pos", pos)
DEFINE_TYPE_INHERIT_FUNCTION1(type_inherit_neg, "operator neg", neg)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_mul, "operator mul", mul)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_div, "operator div", div)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_mod, "operator mod", mod)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_shl, "operator shl", shl)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_shr, "operator shr", shr)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_and, "operator and", and)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_or, "operator or", or)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_xor, "operator xor", xor)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_pow, "operator pow", pow)

/* inc, dec, add, sub, iadd & isub are all apart of the same operator group. */
INTERN NONNULL((1)) bool DCALL type_inherit_add(DeeTypeObject *__restrict self) {
	struct type_math *base_math;
	DeeTypeObject *base = DeeType_Base(self);
	if (!base ||
	    (((base_math = base->tp_math) == NULL ||
	      (!base_math->tp_add && !base_math->tp_inplace_add &&
	       !base_math->tp_sub && !base_math->tp_inplace_sub &&
	       !base_math->tp_inc && !base_math->tp_dec)) &&
	     !type_inherit_add(base)))
		return false;
	Dee_DPRINTF("[RT] Inherit `operator add' from %q into %q\n",
	            base->tp_name, self->tp_name);
	base_math = base->tp_math;
	if (self->tp_math) {
		self->tp_math->tp_inc         = base_math->tp_inc;
		self->tp_math->tp_dec         = base_math->tp_dec;
		self->tp_math->tp_add         = base_math->tp_add;
		self->tp_math->tp_sub         = base_math->tp_sub;
		self->tp_math->tp_inplace_add = base_math->tp_inplace_add;
		self->tp_math->tp_inplace_sub = base_math->tp_inplace_sub;
	} else {
		self->tp_math = base_math;
	}
	return true;
}
#undef DEFINE_TYPE_INHERIT_FUNCTION1
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif /* !DEFINE_TYPED_OPERATORS */


#ifdef DEFINE_TYPED_OPERATORS
#define COPY_SELF() DeeObject_TCopy(tp_self, self)
#else /* DEFINE_TYPED_OPERATORS */
#define COPY_SELF() DeeObject_Copy(self)
#endif /* !DEFINE_TYPED_OPERATORS */

#define DEFINE_MATH_OPERATOR1(name, xxx, operator_name, invoke)                     \
	DEFINE_OPERATOR(DREF DeeObject *, name, (DeeObject *RESTRICT_IF_NOTYPE self)) { \
		LOAD_TP_SELF;                                                               \
		do {                                                                        \
			if (tp_self->tp_math && tp_self->tp_math->tp_##xxx)                     \
				return invoke(tp_self, self);                                       \
		} while (type_inherit_##xxx(tp_self));                                      \
		err_unimplemented_operator(tp_self, operator_name);                         \
		return NULL;                                                                \
	}
#define DEFINE_MATH_OPERATOR2(name, xxx, operator_name, invoke, invoke_inplace)        \
	DEFINE_OPERATOR(DREF DeeObject *, name,                                            \
	                (DeeObject *self, DeeObject *some_object)) {                       \
		LOAD_TP_SELF;                                                                  \
		ASSERT_OBJECT(some_object);                                                    \
		do {                                                                           \
			struct type_math *math;                                                    \
			if ((math = tp_self->tp_math) != NULL) {                                   \
				if (tp_self->tp_math->tp_##xxx)                                        \
					return invoke(tp_self, self, some_object);                         \
				if (tp_self->tp_math->tp_inplace_##xxx) {                              \
					int error;                                                         \
					if unlikely((self = COPY_SELF()) == NULL)                          \
						goto err;                                                      \
					error = invoke_inplace(tp_self, (DeeObject **)&self, some_object); \
					if unlikely(error)                                                 \
						Dee_Clear(self);                                               \
					return self;                                                       \
				}                                                                      \
			}                                                                          \
		} while (type_inherit_##xxx(tp_self));                                         \
		err_unimplemented_operator(tp_self, operator_name);                            \
	err:                                                                               \
		return NULL;                                                                   \
	}
DEFINE_MATH_OPERATOR1(Inv, inv, OPERATOR_INV, DeeType_INVOKE_INV)
DEFINE_MATH_OPERATOR1(Pos, pos, OPERATOR_POS, DeeType_INVOKE_POS)
DEFINE_MATH_OPERATOR1(Neg, neg, OPERATOR_NEG, DeeType_INVOKE_NEG)
DEFINE_MATH_OPERATOR2(Mul, mul, OPERATOR_MUL, DeeType_INVOKE_MUL, DeeType_INVOKE_IMUL)
DEFINE_MATH_OPERATOR2(Div, div, OPERATOR_DIV, DeeType_INVOKE_DIV, DeeType_INVOKE_IDIV)
DEFINE_MATH_OPERATOR2(Mod, mod, OPERATOR_MOD, DeeType_INVOKE_MOD, DeeType_INVOKE_IMOD)
DEFINE_MATH_OPERATOR2(Shl, shl, OPERATOR_SHL, DeeType_INVOKE_SHL, DeeType_INVOKE_ISHL)
DEFINE_MATH_OPERATOR2(Shr, shr, OPERATOR_SHR, DeeType_INVOKE_SHR, DeeType_INVOKE_ISHR)
DEFINE_MATH_OPERATOR2(And, and, OPERATOR_AND, DeeType_INVOKE_AND, DeeType_INVOKE_IAND)
DEFINE_MATH_OPERATOR2(Or, or, OPERATOR_OR, DeeType_INVOKE_OR, DeeType_INVOKE_IOR)
DEFINE_MATH_OPERATOR2(Xor, xor, OPERATOR_XOR, DeeType_INVOKE_XOR, DeeType_INVOKE_IXOR)
DEFINE_MATH_OPERATOR2(Pow, pow, OPERATOR_POW, DeeType_INVOKE_POW, DeeType_INVOKE_IPOW)

DEFINE_OPERATOR(DREF DeeObject *, Add,
               (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(some_object);
	do {
		struct type_math *math = tp_self->tp_math;
		if (math) {
			if (math->tp_add)
				return DeeType_INVOKE_ADD(tp_self, self, some_object);
			if (math->tp_inplace_add) {
				if unlikely((self = COPY_SELF()) == NULL)
					goto err;
				if unlikely(DeeType_INVOKE_IADD(tp_self, (DeeObject **)&self, some_object))
					goto err_self;
				return self;
			}
			if (math->tp_sub)
				break; /* XXX: `x + y --> x - (-y)' */
			if (math->tp_inplace_sub)
				break; /* XXX: `x += y --> x -= (-y)' */
			if (math->tp_inc)
				break;
			if (math->tp_dec)
				break;
		}
	} while (type_inherit_add(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_ADD);
err:
	return NULL;
err_self:
	Dee_Decref(self);
	goto err;
}

DEFINE_OPERATOR(DREF DeeObject *, Sub,
                (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(some_object);
	do {
		struct type_math *math = tp_self->tp_math;
		if (math) {
			if (math->tp_sub)
				return DeeType_INVOKE_SUB(tp_self, self, some_object);
			if (math->tp_inplace_sub) {
				if unlikely((self = COPY_SELF()) == NULL)
					goto err;
				if unlikely(DeeType_INVOKE_ISUB(tp_self, (DeeObject **)&self, some_object))
					goto err_self;
				return self;
			}
			if (math->tp_add)
				break; /* XXX: `x - y --> x + (-y)' */
			if (math->tp_inplace_add)
				break; /* XXX: `x -= y --> x += (-y)' */
			if (math->tp_inc)
				break;
			if (math->tp_dec)
				break;
		}
	} while (type_inherit_add(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_SUB);
err:
	return NULL;
err_self:
	Dee_Decref(self);
	goto err;
}

#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AddS8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return DeeInt_AddSDigit((DeeIntObject *)self, val);
	val_ob = DeeInt_NewS8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Add(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_SubS8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return DeeInt_SubSDigit((DeeIntObject *)self, val);
	val_ob = DeeInt_NewS8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Sub(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AddInt(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return DeeInt_AddU32((DeeIntObject *)self, val);
	val_ob = DeeInt_NewU32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Add(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_SubInt(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return DeeInt_SubU32((DeeIntObject *)self, val);
	val_ob = DeeInt_NewU32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Sub(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_MulInt(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewS8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Mul(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_DivInt(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewS8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Div(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_ModInt(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewS8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Mod(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_ShlInt(DeeObject *__restrict self, uint8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewU8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Shl(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_ShrInt(DeeObject *__restrict self, uint8_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewU8(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Shr(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AndInt(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewU32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_And(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_OrInt(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewU32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Or(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_XorInt(DeeObject *__restrict self, uint32_t val) {
	DREF DeeObject *val_ob, *result;
	/* TODO: Optimization for `int' */
	val_ob = DeeInt_NewU32(val);
	if unlikely(!val_ob)
		goto err;
	result = DeeObject_Xor(self, val_ob);
	Dee_Decref(val_ob);
	return result;
err:
	return NULL;
}
#endif /* !DEFINE_TYPED_OPERATORS */

#undef DEFINE_MATH_OPERATOR2
#undef DEFINE_MATH_OPERATOR1
#undef COPY_SELF


DEFINE_OPERATOR(int, Inc, (DeeObject **__restrict pself)) {
	LOAD_TP_SELFP;
	do {
		struct type_math *math;
		if ((math = tp_self->tp_math) != NULL) {
			if (math->tp_inc)
				return DeeType_INVOKE_INC(tp_self, pself);
			if (math->tp_inplace_add)
				return DeeType_INVOKE_IADD(tp_self, pself, &DeeInt_One);
			if (math->tp_add) {
				DREF DeeObject *temp;
				temp = DeeType_INVOKE_ADD(tp_self, *pself, &DeeInt_One);
				if unlikely(!temp)
					goto err;
				Dee_Decref(*pself);
				*pself = temp;
				return 0;
			}
			if (math->tp_inplace_sub)
				return DeeType_INVOKE_ISUB(tp_self, pself, &DeeInt_MinusOne);
			if (math->tp_sub) {
				DREF DeeObject *temp;
				temp = DeeType_INVOKE_SUB(tp_self, *pself, &DeeInt_MinusOne);
				if unlikely(!temp)
					goto err;
				Dee_Decref(*pself);
				*pself = temp;
				return 0;
			}
			if (math->tp_dec)
				break;
		}
	} while (type_inherit_add(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_INC);
err:
	return -1;
}

DEFINE_OPERATOR(int, Dec, (DeeObject **__restrict pself)) {
	LOAD_TP_SELFP;
	do {
		struct type_math *math;
		if ((math = tp_self->tp_math) != NULL) {
			if (math->tp_dec)
				return DeeType_INVOKE_DEC(tp_self, pself);
			if (math->tp_inplace_sub)
				return DeeType_INVOKE_ISUB(tp_self, pself, &DeeInt_One);
			if (math->tp_sub) {
				DREF DeeObject *temp;
				temp = DeeType_INVOKE_SUB(tp_self, *pself, &DeeInt_One);
				if unlikely(!temp)
					goto err;
				Dee_Decref(*pself);
				*pself = temp;
				return 0;
			}
			if (math->tp_inplace_add)
				return DeeType_INVOKE_IADD(tp_self, pself, &DeeInt_MinusOne);
			if (math->tp_add) {
				DREF DeeObject *temp;
				temp = DeeType_INVOKE_ADD(tp_self, *pself, &DeeInt_MinusOne);
				if unlikely(!temp)
					goto err;
				Dee_Decref(*pself);
				*pself = temp;
				return 0;
			}
			if (math->tp_inc)
				break;
		}
	} while (type_inherit_add(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_DEC);
err:
	return -1;
}

#define DEFINE_MATH_INPLACE_OPERATOR2(name, xxx, operator_name, invoke, invoke_inplace)  \
	DEFINE_OPERATOR(int, name, (DeeObject **__restrict pself, DeeObject *some_object)) { \
		LOAD_TP_SELFP;                                                                   \
		ASSERT_OBJECT(some_object);                                                      \
		do {                                                                             \
			if (tp_self->tp_math) {                                                      \
				if (tp_self->tp_math->tp_inplace_##xxx) {                                \
					return invoke_inplace(tp_self, pself, some_object);                  \
				}                                                                        \
				if (tp_self->tp_math->tp_##xxx) {                                        \
					DREF DeeObject *temp;                                                \
					temp = invoke(tp_self, *pself, some_object);                         \
					if unlikely(!temp)                                                   \
						goto err;                                                        \
					Dee_Decref(*pself);                                                  \
					*pself = temp;                                                       \
					return 0;                                                            \
				}                                                                        \
			}                                                                            \
		} while (type_inherit_##xxx(tp_self));                                           \
		err_unimplemented_operator(tp_self, operator_name);                              \
	err:                                                                                 \
		return -1;                                                                       \
	}
DEFINE_MATH_INPLACE_OPERATOR2(InplaceMul, mul, OPERATOR_INPLACE_MUL, DeeType_INVOKE_MUL, DeeType_INVOKE_IMUL)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceDiv, div, OPERATOR_INPLACE_DIV, DeeType_INVOKE_DIV, DeeType_INVOKE_IDIV)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceMod, mod, OPERATOR_INPLACE_MOD, DeeType_INVOKE_MOD, DeeType_INVOKE_IMOD)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceShl, shl, OPERATOR_INPLACE_SHL, DeeType_INVOKE_SHL, DeeType_INVOKE_ISHL)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceShr, shr, OPERATOR_INPLACE_SHR, DeeType_INVOKE_SHR, DeeType_INVOKE_ISHR)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceAnd, and, OPERATOR_INPLACE_AND, DeeType_INVOKE_AND, DeeType_INVOKE_IAND)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceOr, or, OPERATOR_INPLACE_OR, DeeType_INVOKE_OR, DeeType_INVOKE_IOR)
DEFINE_MATH_INPLACE_OPERATOR2(InplaceXor, xor, OPERATOR_INPLACE_XOR, DeeType_INVOKE_XOR, DeeType_INVOKE_IXOR)
DEFINE_MATH_INPLACE_OPERATOR2(InplacePow, pow, OPERATOR_INPLACE_POW, DeeType_INVOKE_POW, DeeType_INVOKE_IPOW)

DEFINE_OPERATOR(int, InplaceAdd,
                (DeeObject **__restrict pself, DeeObject *some_object)) {
	LOAD_TP_SELFP;
	ASSERT_OBJECT(some_object);
	do {
		struct type_math *math = tp_self->tp_math;
		if (math) {
			if (math->tp_inplace_add)
				return DeeType_INVOKE_IADD(tp_self, pself, some_object);
			if (math->tp_add) {
				DREF DeeObject *temp;
				temp = DeeType_INVOKE_ADD(tp_self, *pself, some_object);
				if unlikely(!temp)
					goto err;
				Dee_Decref(*pself);
				*pself = temp;
				return 0;
			}
			if (math->tp_sub)
				break; /* XXX: `x - y --> x + (-y)' */
			if (math->tp_inplace_sub)
				break; /* XXX: `x -= y --> x += (-y)' */
			if (math->tp_inc)
				break;
			if (math->tp_dec)
				break;
		}
	} while (type_inherit_add(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_INPLACE_ADD);
err:
	return -1;
}
DEFINE_OPERATOR(int, InplaceSub,
                (DeeObject **__restrict pself, DeeObject *some_object)) {
	LOAD_TP_SELFP;
	ASSERT_OBJECT(some_object);
	do {
		struct type_math *math = tp_self->tp_math;
		if (math) {
			if (math->tp_inplace_sub)
				return DeeType_INVOKE_ISUB(tp_self, pself, some_object);
			if (math->tp_sub) {
				DREF DeeObject *temp;
				temp = DeeType_INVOKE_SUB(tp_self, *pself, some_object);
				if unlikely(!temp)
					goto err;
				Dee_Decref(*pself);
				*pself = temp;
				return 0;
			}
			if (math->tp_add)
				break; /* XXX: `x + y --> x - (-y)' */
			if (math->tp_inplace_add)
				break; /* XXX: `x += y --> x -= (-y)' */
			if (math->tp_inc)
				break;
			if (math->tp_dec)
				break;
		}
	} while (type_inherit_add(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_INPLACE_ADD);
err:
	return -1;
}

#undef DEFINE_MATH_INPLACE_OPERATOR2

#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceXXX, reg, DeeInt_NewXXX, intX_t, operator_name) \
	PUBLIC int DCALL                                                                                      \
	DeeObject_InplaceXXX(DREF DeeObject **__restrict pself, intX_t val) {                                 \
		DREF DeeObject *temp;                                                                             \
		int result;                                                                                       \
		/* TODO: Optimization for `int' */                                                                \
		temp = DeeInt_NewXXX(val);                                                                        \
		if unlikely(!temp)                                                                                \
			goto err;                                                                                     \
		result = reg(pself, temp);                                                                        \
		Dee_Decref(temp);                                                                                 \
		return result;                                                                                    \
	err:                                                                                                  \
		return -1;                                                                                        \
	}
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAddS8, DeeObject_InplaceAdd, DeeInt_NewS8, int8_t, OPERATOR_INPLACE_ADD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceSubS8, DeeObject_InplaceSub, DeeInt_NewS8, int8_t, OPERATOR_INPLACE_SUB)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAddInt, DeeObject_InplaceAdd, DeeInt_NewU32, uint32_t, OPERATOR_INPLACE_ADD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceSubInt, DeeObject_InplaceSub, DeeInt_NewU32, uint32_t, OPERATOR_INPLACE_SUB)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceMulInt, DeeObject_InplaceMul, DeeInt_NewS8, int8_t, OPERATOR_INPLACE_MUL)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceDivInt, DeeObject_InplaceDiv, DeeInt_NewS8, int8_t, OPERATOR_INPLACE_DIV)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceModInt, DeeObject_InplaceMod, DeeInt_NewS8, int8_t, OPERATOR_INPLACE_MOD)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceShlInt, DeeObject_InplaceShl, DeeInt_NewU8, uint8_t, OPERATOR_INPLACE_SHL)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceShrInt, DeeObject_InplaceShr, DeeInt_NewU8, uint8_t, OPERATOR_INPLACE_SHR)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceAndInt, DeeObject_InplaceAnd, DeeInt_NewU32, uint32_t, OPERATOR_INPLACE_AND)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceOrInt, DeeObject_InplaceOr, DeeInt_NewU32, uint32_t, OPERATOR_INPLACE_OR)
DEFINE_MATH_INPLACE_INT_OPERATOR(DeeObject_InplaceXorInt, DeeObject_InplaceXor, DeeInt_NewU32, uint32_t, OPERATOR_INPLACE_XOR)
#undef DEFINE_MATH_INPLACE_INT_OPERATOR
#endif /* !DEFINE_TYPED_OPERATORS */


LOCAL WUNUSED DREF DeeObject *DCALL
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



#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
type_inherit_compare(DeeTypeObject *__restrict self) {
	struct type_cmp *base_cmp;
	DeeTypeObject *base = DeeType_Base(self);
	if (!base ||
	    (((base_cmp = base->tp_cmp) == NULL ||
	      (!base_cmp->tp_eq && !base_cmp->tp_ne &&
	       !base_cmp->tp_lo && !base_cmp->tp_le &&
	       !base_cmp->tp_gr && !base_cmp->tp_ge)) &&
	     !type_inherit_compare(base)))
		return false;
	Dee_DPRINTF("[RT] Inherit `operator <compare>' from %q into %q\n",
	            base->tp_name, self->tp_name);
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
#endif /* !DEFINE_TYPED_OPERATORS */

#define DEFINE_COMPARE_OPERATOR(name, fwd, bck, operator_name, invoke_fwd, invoke_bck) \
	DEFINE_OPERATOR(DREF DeeObject *, name,                                            \
	                (DeeObject *self, DeeObject *some_object)) {                       \
		LOAD_TP_SELF;                                                                  \
		ASSERT_OBJECT(some_object);                                                    \
		do {                                                                           \
			if (tp_self->tp_cmp) {                                                     \
				if (tp_self->tp_cmp->tp_##fwd)                                         \
					return invoke_fwd(tp_self, self, some_object);                     \
				if (tp_self->tp_cmp->tp_##bck)                                         \
					return invoke_not(invoke_bck(tp_self, self, some_object));         \
			}                                                                          \
		} while (type_inherit_compare(tp_self));                                       \
		err_unimplemented_operator(tp_self, operator_name);                            \
		return NULL;                                                                   \
	}
DEFINE_COMPARE_OPERATOR(CompareEqObject, eq, ne, OPERATOR_EQ, DeeType_INVOKE_EQ, DeeType_INVOKE_NE)
DEFINE_COMPARE_OPERATOR(CompareNeObject, ne, eq, OPERATOR_NE, DeeType_INVOKE_NE, DeeType_INVOKE_EQ)
DEFINE_COMPARE_OPERATOR(CompareLoObject, lo, ge, OPERATOR_LO, DeeType_INVOKE_LO, DeeType_INVOKE_GE)
DEFINE_COMPARE_OPERATOR(CompareLeObject, le, gr, OPERATOR_LE, DeeType_INVOKE_LE, DeeType_INVOKE_GR)
DEFINE_COMPARE_OPERATOR(CompareGrObject, gr, lo, OPERATOR_GR, DeeType_INVOKE_GR, DeeType_INVOKE_LO)
DEFINE_COMPARE_OPERATOR(CompareGeObject, ge, le, OPERATOR_GE, DeeType_INVOKE_GE, DeeType_INVOKE_LE)
#undef DEFINE_COMPARE_OPERATOR


#ifndef DEFINE_TYPED_OPERATORS
#define DEFINE_COMPARE_OPERATOR(name, fwd, bck, error) \
	PUBLIC WUNUSED NONNULL((1, 2)) int DCALL           \
	name(DeeObject *self, DeeObject *some_object) {    \
		DeeObject *val;                                \
		int result;                                    \
		val = name##Object(self, some_object);         \
		if unlikely(!val)                              \
			return -1;                                 \
		result = DeeObject_Bool(val);                  \
		Dee_Decref(val);                               \
		return result;                                 \
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
 * @return: == 0:  Objects compare as equal
 * @return: == 1:  `lhs > rhs' */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_Compare(DeeObject *lhs, DeeObject *rhs) {
#if 1
	DeeTypeObject *tp_lhs;
	ASSERT_OBJECT(lhs);
	ASSERT_OBJECT(rhs);
	tp_lhs = Dee_TYPE(lhs);
again:
	do {
		if (tp_lhs->tp_cmp) {
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
				opres = invoke_not((*tp_lhs->tp_cmp->tp_ge)(lhs, rhs));
			} else {
				break;
			}
			if unlikely(!opres)
				return -2;
			opval = DeeObject_Bool(opres);
			Dee_Decref(opres);
			if (opval != 0)
				return unlikely(opval < 0) ? -2 : -1;

			/* At this point we know that: "!(lhs < rhs)"
			 * -> Now check if "lhs == rhs" */
			if (tp_lhs->tp_cmp->tp_eq) {
				opres = (*tp_lhs->tp_cmp->tp_eq)(lhs, rhs);
			} else if (tp_lhs->tp_cmp->tp_ne) {
				opres = invoke_not((*tp_lhs->tp_cmp->tp_ne)(lhs, rhs));
			} else {
				err_unimplemented_operator(tp_lhs, OPERATOR_LO);
				return -2;
			}
			if unlikely(!opres)
				return -2;
			opval = DeeObject_Bool(opres);
			Dee_Decref(opres);
			if likely(opval >= 0)
				opval = !opval;
			return opval;
		}
	} while (type_inherit_compare(tp_lhs));
	err_unimplemented_operator(tp_lhs, OPERATOR_LO);
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
#define DEFINE_TYPE_INHERIT_FUNCTION(name, field)  \
	INTERN NONNULL((1)) bool DCALL                 \
	name(DeeTypeObject *__restrict self) {         \
		DeeTypeObject *base = DeeType_Base(self);  \
		struct type_seq *base_seq;                 \
		if (!base ||                               \
		    (((base_seq = base->tp_seq) == NULL || \
		      (!base_seq->field)) &&               \
		     !name(base)))                         \
			return false;                          \
		base_seq = base->tp_seq;                   \
		if (self->tp_seq) {                        \
			self->tp_seq->field = base_seq->field; \
		} else {                                   \
			self->tp_seq = base_seq;               \
		}                                          \
		return true;                               \
	}

INTERN NONNULL((1)) bool DCALL
type_inherit_iternext(DeeTypeObject *__restrict self) {
	DeeTypeObject *base = DeeType_Base(self);
	if (!base ||
	    (!base->tp_iter_next &&
	     !type_inherit_iternext(base)))
		return false;
	self->tp_iter_next = base->tp_iter_next;
	return true;
}
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_iterself, tp_iter_self)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_size, tp_size)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_contains, tp_contains)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_getitem, tp_get)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_delitem, tp_del)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_setitem, tp_set)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_getrange, tp_range_get)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_delrange, tp_range_del)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_setrange, tp_range_set)
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, IterSelf, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	do {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq && seq->tp_iter_self)
			return DeeType_INVOKE_ITER(tp_self, self);
	} while (type_inherit_iterself(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_ITERSELF);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, IterNext, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_iter_next)
			return DeeType_INVOKE_NEXT(tp_self, self);
	} while (type_inherit_iternext(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_ITERNEXT);
	return NULL;
}




#ifndef DEFINE_TYPED_OPERATORS

DEFINE_OPERATOR(size_t, Size, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *sizeob;
	size_t result;
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_size) {
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
	} while (type_inherit_size(tp_self));
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_size)
			return DeeType_INVOKE_SIZE(tp_self, self);
	} while (type_inherit_size(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_SIZE);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, ContainsObject,
                (DeeObject *self, DeeObject *some_object)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_contains)
			return DeeType_INVOKE_CONTAINS(tp_self, self, some_object);
	} while (type_inherit_contains(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_CONTAINS);
	return NULL;
}

DEFINE_OPERATOR(DREF DeeObject *, GetItem,
                (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_get)
			return DeeType_INVOKE_GETITEM(tp_self, self, index);
	} while (type_inherit_getitem(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

DEFINE_OPERATOR(int, DelItem,
                (DeeObject *self, DeeObject *index)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_del)
			return DeeType_INVOKE_DELITEM(tp_self, self, index);
	} while (type_inherit_delitem(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
}

DEFINE_OPERATOR(int, SetItem,
                (DeeObject *self, DeeObject *index, DeeObject *value)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_set)
			return DeeType_INVOKE_SETITEM(tp_self, self, index, value);
	} while (type_inherit_setitem(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_SETITEM);
}

DEFINE_OPERATOR(DREF DeeObject *, GetRange,
                (DeeObject *self, DeeObject *begin, DeeObject *end)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get)
			return DeeType_INVOKE_GETRANGE(tp_self, self, begin, end);
	} while (type_inherit_getrange(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
	return NULL;
}

#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_ConcatInherited(DREF DeeObject *self, DeeObject *other) {
	DREF DeeObject *result;
	if (DeeTuple_CheckExact(other)) {
		size_t i, count = DeeTuple_SIZE(other);
		for (i = 0; i < count; ++i)
			Dee_Incref(DeeTuple_GET(other, i));
		result = DeeObject_ExtendInherited(self, count, DeeTuple_ELEM(other));
		if unlikely(!result) {
			for (i = 0; i < count; ++i)
				Dee_Decref(DeeTuple_GET(other, i));
		}
		return result;
	}
	if (DeeTuple_CheckExact(self))
		return DeeTuple_ConcatInherited(self, other);
	if (DeeList_CheckExact(self))
		return DeeList_Concat(self, other);
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
	do {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq && seq->tp_range_get) {
			dssize_t end_index;
			struct type_nsi const *nsi;
			DREF DeeObject *begin_ob, *result;
			/* NSI optimizations. */
			nsi = seq->tp_nsi;
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
	} while (type_inherit_getrange(tp_self));
	err_unimplemented_operator(GET_TP_SELF(), OPERATOR_GETRANGE);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_GetRangeEndIndex)(DeeObject *self, DeeObject *begin, dssize_t end) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
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
	} while (type_inherit_getrange(tp_self));
	err_unimplemented_operator(GET_TP_SELF(), OPERATOR_GETRANGE);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_GetRangeIndex)(DeeObject *__restrict self,
                                dssize_t begin, dssize_t end) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
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
	} while (type_inherit_getrange(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_GETRANGE);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 3, 4)) int
(DCALL DeeObject_SetRangeBeginIndex)(DeeObject *self,
                                     dssize_t begin, DeeObject *end,
                                     DeeObject *value) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(end);
	ASSERT_OBJECT(value);
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
			int result;
			DREF DeeObject *begin_ob;
			struct type_nsi const *nsi;
			/* NSI optimizations. */
			nsi = tp_self->tp_seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				if (DeeNone_Check(end)) {
					if (nsi->nsi_seqlike.nsi_setrange_n)
						return (*nsi->nsi_seqlike.nsi_setrange_n)(self, begin, value);
				} else if (nsi->nsi_seqlike.nsi_setrange) {
					dssize_t end_index;
					if (DeeObject_AsSSize(end, &end_index))
						goto err;
					return (*nsi->nsi_seqlike.nsi_setrange)(self, begin, end_index, value);
				}
			}
			begin_ob = DeeInt_NewSSize(begin);
			if unlikely(!begin_ob)
				goto err;
			result = DeeType_INVOKE_SETRANGE(tp_self, self, begin_ob, end, value);
			Dee_Decref(begin_ob);
			return result;
		}
	} while (type_inherit_setrange(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_SetRangeEndIndex)(DeeObject *self,
                                   DeeObject *begin, dssize_t end,
                                   DeeObject *value) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(value);
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_get) {
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
					return (*nsi->nsi_seqlike.nsi_setrange)(self, start_index, end, value);
				}
			}
			end_ob = DeeInt_NewSSize(end);
			if unlikely(!end_ob)
				goto err;
			result = DeeType_INVOKE_SETRANGE(tp_self, self, begin, end_ob, value);
			Dee_Decref(end_ob);
			return result;
		}
	} while (type_inherit_setrange(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_SETRANGE);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 4)) int
(DCALL DeeObject_SetRangeIndex)(DeeObject *self,
                                dssize_t begin, dssize_t end,
                                DeeObject *value) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(value);
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_set) {
			DREF DeeObject *begin_ob, *end_ob;
			int result;
			struct type_nsi const *nsi;
			/* NSI optimizations. */
			nsi = tp_self->tp_seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				if (nsi->nsi_seqlike.nsi_setrange)
					return (*nsi->nsi_seqlike.nsi_setrange)(self, begin, end, value);
			}
			begin_ob = DeeInt_NewSSize(begin);
			if unlikely(!begin_ob)
				goto err;
			end_ob = DeeInt_NewSSize(end);
			if unlikely(!end_ob) {
				Dee_Decref(begin_ob);
				goto err;
			}
			result = DeeType_INVOKE_SETRANGE(tp_self, self, begin_ob, end_ob, value);
			Dee_Decref(end_ob);
			Dee_Decref(begin_ob);
			return result;
		}
	} while (type_inherit_setrange(tp_self));
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_del)
			return DeeType_INVOKE_DELRANGE(tp_self, self, begin, end);
	} while (type_inherit_delrange(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_DELRANGE);
}

DEFINE_OPERATOR(int, SetRange,
                (DeeObject *self, DeeObject *begin,
                 DeeObject *end, DeeObject *value)) {
	LOAD_TP_SELF;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(end);
	ASSERT_OBJECT(value);
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_range_set)
			return DeeType_INVOKE_SETRANGE(tp_self, self, begin, end, value);
	} while (type_inherit_setrange(tp_self));
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
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
			/* Fallback create an integer object and use it for indexing. */
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
	} while (type_inherit_getitem(tp_self));
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
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
			/* Fallback create an integer object and use it for indexing. */
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
	} while (type_inherit_getitem(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItemString)(DeeObject *__restrict self,
                                  char const *__restrict key,
                                  dhash_t hash,
                                  bool allow_missing) {
	int result;
	DREF DeeObject *key_ob;
	ASSERT_OBJECT(self);
	/* Optimization for specific types. */
	if (DeeDict_CheckExact(self)) {
		result = DeeDict_HasItemString(self, key, hash);
		goto handle_bool;
	}
	if (DeeRoDict_CheckExact(self)) {
		result = DeeRoDict_HasItemString(self, key, hash);
		goto handle_bool;
	}
	if (DeeKwdsMapping_CheckExact(self)) {
		result = DeeKwdsMapping_HasItemString(self, key, hash);
		goto handle_bool;
	}
	if (BlackListVarkwds_CheckExact(self)) {
		result = BlackListVarkwds_HasItemString((BlackListVarkwds *)self, key, hash);
		goto handle_bool;
	}
	if (BlackListMapping_CheckExact(self))
		return BlackListMapping_BoundItemString((BlackListMapping *)self, key, hash, allow_missing);
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_BoundItem(self, key_ob, allow_missing);
	Dee_Decref(key_ob);
	return result;
handle_bool:
	if (!result) {
		if (!allow_missing) {
			err_unknown_key_str(self, key);
			goto err;
		}
		result = -2;
	}
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItemStringLen)(DeeObject *__restrict self,
                                     char const *__restrict key,
                                     size_t keylen, dhash_t hash,
                                     bool allow_missing) {
	int result;
	DREF DeeObject *key_ob;
	ASSERT_OBJECT(self);
	/* Optimization for specific types. */
	if (DeeDict_CheckExact(self)) {
		result = DeeDict_HasItemStringLen(self, key, keylen, hash);
		goto handle_bool;
	}
	if (DeeRoDict_CheckExact(self)) {
		result = DeeRoDict_HasItemStringLen(self, key, keylen, hash);
		goto handle_bool;
	}
	if (DeeKwdsMapping_CheckExact(self)) {
		result = DeeKwdsMapping_HasItemStringLen(self, key, keylen, hash);
		goto handle_bool;
	}
	if (BlackListVarkwds_CheckExact(self)) {
		result = BlackListVarkwds_HasItemStringLen((BlackListVarkwds *)self, key, keylen, hash);
		goto handle_bool;
	}
	if (BlackListMapping_CheckExact(self))
		return BlackListMapping_BoundItemStringLen((BlackListMapping *)self, key, keylen, hash, allow_missing);
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_BoundItem(self, key_ob, allow_missing);
	Dee_Decref(key_ob);
	return result;
handle_bool:
	if (!result) {
		if (!allow_missing) {
			err_unknown_key_str_len(self, key, keylen);
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
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
			/* Fallback create an integer object and use it for indexing. */
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
	} while (type_inherit_getitem(tp_self));
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
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
			/* Fallback create an integer object and use it for indexing. */
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
	} while (type_inherit_getitem(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItemString)(DeeObject *__restrict self,
                                char const *__restrict key,
                                dhash_t hash) {
	int result;
	DREF DeeObject *key_ob;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_HasItemString(self, key, hash) ? 1 : 0;
	if (DeeRoDict_CheckExact(self))
		return DeeRoDict_HasItemString(self, key, hash) ? 1 : 0;
	if (DeeKwdsMapping_CheckExact(self))
		return DeeKwdsMapping_HasItemString(self, key, hash) ? 1 : 0;
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_HasItem(self, key_ob);
	Dee_Decref(key_ob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItemStringLen)(DeeObject *__restrict self,
                                   char const *__restrict key,
                                   size_t keylen, dhash_t hash) {
	int result;
	DREF DeeObject *key_ob;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_HasItemStringLen(self, key, keylen, hash) ? 1 : 0;
	if (DeeRoDict_CheckExact(self))
		return DeeRoDict_HasItemStringLen(self, key, keylen, hash) ? 1 : 0;
	if (DeeKwdsMapping_CheckExact(self))
		return DeeKwdsMapping_HasItemStringLen(self, key, keylen, hash) ? 1 : 0;
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_HasItem(self, key_ob);
	Dee_Decref(key_ob);
	return result;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObject_GetItemDef(DeeObject *self, DeeObject *key, DeeObject *def) {
	DREF DeeObject *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
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
	} while (type_inherit_getitem(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_GETITEM);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_GetItemIndex(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *index_ob, *result;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_get) {
			struct type_nsi const *nsi;
			nsi = tp_self->tp_seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				if (nsi->nsi_seqlike.nsi_getitem)
					return (*nsi->nsi_seqlike.nsi_getitem)(self, index);
			}
			/* Fallback create an integer object and use it for indexing. */
			index_ob = DeeInt_NewSize(index);
			if unlikely(!index_ob)
				goto err;
			result = DeeType_INVOKE_GETITEM(tp_self, self, index_ob);
			Dee_Decref(index_ob);
			return result;
		}
	} while (type_inherit_getitem(tp_self));
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_del) {
			struct type_nsi const *nsi;
			nsi = tp_self->tp_seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				if (nsi->nsi_seqlike.nsi_delitem)
					return (*nsi->nsi_seqlike.nsi_delitem)(self, index);
			}
			/* Fallback create an integer object and use it for indexing. */
			index_ob = DeeInt_NewSize(index);
			if unlikely(!index_ob)
				goto err;
			result = DeeType_INVOKE_DELITEM(tp_self, self, index_ob);
			Dee_Decref(index_ob);
			return result;
		}
	} while (type_inherit_delitem(tp_self));
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
	do {
		if (tp_self->tp_seq && tp_self->tp_seq->tp_set) {
			struct type_nsi const *nsi;
			nsi = tp_self->tp_seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ) {
				if (nsi->nsi_seqlike.nsi_setitem)
					return (*nsi->nsi_seqlike.nsi_setitem)(self, index, value);
			}
			/* Fallback create an integer object and use it for indexing. */
			index_ob = DeeInt_NewSize(index);
			if unlikely(!index_ob)
				goto err;
			result = DeeType_INVOKE_SETITEM(tp_self, self, index_ob, value);
			Dee_Decref(index_ob);
			return result;
		}
	} while (type_inherit_setitem(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_DELITEM);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_GetItemString(DeeObject *__restrict self,
                        char const *__restrict key,
                        dhash_t hash) {
	DREF DeeObject *key_ob, *result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_GetItemString(self, key, hash);
	if (DeeRoDict_CheckExact(self))
		return DeeRoDict_GetItemString(self, key, hash);
	if (DeeKwdsMapping_CheckExact(self))
		return DeeKwdsMapping_GetItemString(self, key, hash);
	if (BlackListVarkwds_CheckExact(self))
		return BlackListVarkwds_GetItemString((BlackListVarkwds *)self, key, hash);
	if (BlackListMapping_CheckExact(self))
		return BlackListMapping_GetItemString((BlackListMapping *)self, key, hash);
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_GetItem(self, key_ob);
	Dee_Decref(key_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_GetItemStringLen(DeeObject *__restrict self,
                           char const *__restrict key,
                           size_t keylen,
                           dhash_t hash) {
	DREF DeeObject *key_ob, *result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_GetItemStringLen(self, key, keylen, hash);
	if (DeeRoDict_CheckExact(self))
		return DeeRoDict_GetItemStringLen(self, key, keylen, hash);
	if (DeeKwdsMapping_CheckExact(self))
		return DeeKwdsMapping_GetItemStringLen(self, key, keylen, hash);
	if (BlackListVarkwds_CheckExact(self))
		return BlackListVarkwds_GetItemStringLen((BlackListVarkwds *)self, key, keylen, hash);
	if (BlackListMapping_CheckExact(self))
		return BlackListMapping_GetItemStringLen((BlackListMapping *)self, key, keylen, hash);
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_GetItem(self, key_ob);
	Dee_Decref(key_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeObject_GetItemStringDef(DeeObject *self,
                           char const *__restrict key,
                           dhash_t hash,
                           DeeObject *def) {
	DREF DeeObject *key_ob, *result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_GetItemStringDef(self, key, hash, def);
	if (DeeRoDict_CheckExact(self))
		return DeeRoDict_GetItemStringDef(self, key, hash, def);
	if (DeeKwdsMapping_CheckExact(self))
		return DeeKwdsMapping_GetItemStringDef(self, key, hash, def);
	if (BlackListVarkwds_CheckExact(self))
		return BlackListVarkwds_GetItemStringDef((BlackListVarkwds *)self, key, hash, def);
	if (BlackListMapping_CheckExact(self))
		return BlackListMapping_GetItemStringDef((BlackListMapping *)self, key, hash, def);
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewWithHash(key, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_GetItemDef(self, key_ob, def);
	Dee_Decref(key_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeObject_GetItemStringLenDef(DeeObject *self,
                              char const *__restrict key,
                              size_t keylen, dhash_t hash,
                              DeeObject *def) {
	DREF DeeObject *key_ob, *result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_GetItemStringLenDef(self, key, keylen, hash, def);
	if (DeeRoDict_CheckExact(self))
		return DeeRoDict_GetItemStringLenDef(self, key, keylen, hash, def);
	if (DeeKwdsMapping_CheckExact(self))
		return DeeKwdsMapping_GetItemStringLenDef(self, key, keylen, hash, def);
	if (BlackListVarkwds_CheckExact(self))
		return BlackListVarkwds_GetItemStringLenDef((BlackListVarkwds *)self, key, keylen, hash, def);
	if (BlackListMapping_CheckExact(self))
		return BlackListMapping_GetItemStringLenDef((BlackListMapping *)self, key, keylen, hash, def);
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_GetItemDef(self, key_ob, def);
	Dee_Decref(key_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelItemString)(DeeObject *__restrict self,
                                char const *__restrict key,
                                dhash_t hash) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_DelItemString(self, key, hash);
	/* Fallback: create a string object and use it for indexing. */
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
(DCALL DeeObject_DelItemStringLen)(DeeObject *__restrict self,
                                   char const *__restrict key,
                                   size_t keylen, dhash_t hash) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	if (DeeDict_CheckExact(self))
		return DeeDict_DelItemStringLen(self, key, keylen, hash);
	/* Fallback: create a string object and use it for indexing. */
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
(DCALL DeeObject_SetItemString)(DeeObject *self,
                                char const *__restrict key,
                                dhash_t hash, DeeObject *value) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(value);
	if (DeeDict_CheckExact(self))
		return DeeDict_SetItemString(self, key, hash, value);
	/* Fallback: create a string object and use it for indexing. */
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
(DCALL DeeObject_SetItemStringLen)(DeeObject *self,
                                   char const *__restrict key,
                                   size_t keylen, dhash_t hash,
                                   DeeObject *value) {
	DREF DeeObject *key_ob;
	int result;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(value);
	if (DeeDict_CheckExact(self))
		return DeeDict_SetItemStringLen(self, key, keylen, hash, value);
	/* Fallback: create a string object and use it for indexing. */
	key_ob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!key_ob)
		goto err;
	result = DeeObject_SetItem(self, key_ob, value);
	Dee_Decref(key_ob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeObject_Print(DeeObject *__restrict self,
                dformatprinter printer, void *arg) {
	DREF DeeObject *ob_str;
	dssize_t result;
	DeeTypeObject *typ;
	typ = Dee_TYPE(self);
	if (typ == &DeeString_Type)
		return DeeString_PrintUtf8(self, printer, arg);
	if (typ == &DeeInt_Type)
		return DeeInt_Print(self, DEEINT_PRINT_DEC, 0, printer, arg);
	if (typ == &DeeTuple_Type)
		return DeeTuple_Print(self, printer, arg);
	if (typ == &DeeBytes_Type)
		return DeeBytes_PrintUtf8(self, printer, arg);
	if (typ == &DeeFloat_Type)
		return DeeFormat_Printf(printer, arg, "%f", DeeFloat_VALUE(self));
	if (typ == &DeeError_CompilerError ||
	    typ == &DeeError_SyntaxError ||
	    typ == &DeeError_SymbolError)
		return DeeCompilerError_Print(self, printer, arg);
	/* Fallback: print the object's __str__ operator result. */
	ob_str = DeeObject_Str(self);
	if unlikely(!ob_str)
		goto err;
	result = DeeString_PrintUtf8(ob_str, printer, arg);
	Dee_Decref(ob_str);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeObject_PrintRepr(DeeObject *__restrict self,
                    dformatprinter printer, void *arg) {
	DREF DeeObject *ob_repr;
	dssize_t result;
	DeeTypeObject *typ;
	typ = Dee_TYPE(self);
	if (typ == &DeeString_Type)
		return DeeString_PrintRepr(self, printer, arg, FORMAT_QUOTE_FNORMAL);
	if (typ == &DeeInt_Type)
		return DeeInt_Print(self, DEEINT_PRINT_DEC, 0, printer, arg);
	if (typ == &DeeBytes_Type)
		return DeeBytes_PrintRepr(self, printer, arg);
	if (typ == &DeeFloat_Type)
		return DeeFormat_Printf(printer, arg, "%f", DeeFloat_VALUE(self));
	if (typ == &DeeList_Type)
		return DeeList_PrintRepr(self, printer, arg);
	if (typ == &DeeError_CompilerError ||
	    typ == &DeeError_SyntaxError ||
	    typ == &DeeError_SymbolError)
		return DeeCompilerError_Print(self, printer, arg);

	/* Fallback: print the object __repr__ operator result. */
	ob_repr = DeeObject_Repr(self);
	if unlikely(!ob_repr)
		goto err;
	result = DeeString_PrintUtf8(ob_repr, printer, arg);
	Dee_Decref(ob_repr);
	return result;
err:
	return -1;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, GetAttr,
                (DeeObject *self, /*String*/ DeeObject *attr_name)) {
	DREF DeeObject *result;
	dhash_t hash;
	DeeTypeObject *iter;
	LOAD_TP_SELF;
	iter = tp_self;
	ASSERT_OBJECT_TYPE_EXACT(attr_name, &DeeString_Type);
	if (iter->tp_attr)
		goto do_iter_attr;
	hash = DeeString_Hash(attr_name);
	/* Search through the cache for the requested attribute. */
	if ((result = DeeType_GetCachedAttr(iter, self, DeeString_STR(attr_name), hash)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *attr;
			if ((attr = DeeType_QueryAttributeWithHash(tp_self, iter, attr_name, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this attr. */
				if (!class_attribute_mayaccess(attr, iter)) {
					err_class_protected_member(iter, attr);
					return NULL;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_GetAttribute(desc, DeeInstance_DESC(desc, self), self, attr);
			}
		} else {
			if (iter->tp_methods &&
			    (result = DeeType_GetMethodAttr(tp_self, iter, self, DeeString_STR(attr_name), hash)) != ITER_DONE)
				goto done;
			if (iter->tp_getsets &&
			    (result = DeeType_GetGetSetAttr(tp_self, iter, self, DeeString_STR(attr_name), hash)) != ITER_DONE)
				goto done;
			if (iter->tp_members &&
			    (result = DeeType_GetMemberAttr(tp_self, iter, self, DeeString_STR(attr_name), hash)) != ITER_DONE)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr)
				return DeeType_INVOKE_GETATTR(iter, self, attr_name);
			/* Don't consider attributes from lower levels for custom attr access. */
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

DEFINE_OPERATOR(int, DelAttr,
                (DeeObject *self, /*String*/ DeeObject *attr_name)) {
	int temp;
	dhash_t hash;
	DeeTypeObject *iter;
	LOAD_TP_SELF;
	iter = tp_self;
	ASSERT_OBJECT_TYPE_EXACT(attr_name, &DeeString_Type);
	if (iter->tp_attr)
		goto do_iter_attr;
	hash = DeeString_Hash(attr_name);
	/* Search through the cache for the requested attribute. */
	if ((temp = DeeType_DelCachedAttr(iter, self, DeeString_STR(attr_name), hash)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *attr;
			if ((attr = DeeType_QueryAttributeWithHash(tp_self, iter, attr_name, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this attr. */
				if (!class_attribute_mayaccess(attr, iter)) {
					err_class_protected_member(iter, attr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_DelAttribute(desc, DeeInstance_DESC(desc, self), self, attr);
			}
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttr(tp_self, iter, DeeString_STR(attr_name), hash))
				goto noaccess;
			if (iter->tp_getsets &&
			    (temp = DeeType_DelGetSetAttr(tp_self, iter, self, DeeString_STR(attr_name), hash)) <= 0)
				goto done;
			if (iter->tp_members &&
			    (temp = DeeType_DelMemberAttr(tp_self, iter, self, DeeString_STR(attr_name), hash)) <= 0)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_delattr)
				return DeeType_INVOKE_DELATTR(iter, self, attr_name);
			/* Don't consider attributes from lower levels for custom attr access. */
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

DEFINE_OPERATOR(int, SetAttr,
                (DeeObject *self, /*String*/ DeeObject *attr_name, DeeObject *value)) {
	int temp;
	dhash_t hash;
	DeeTypeObject *iter;
	LOAD_TP_SELF;
	iter = tp_self;
	ASSERT_OBJECT_TYPE_EXACT(attr_name, &DeeString_Type);
	ASSERT_OBJECT(value);
	if (iter->tp_attr)
		goto do_iter_attr;
	hash = DeeString_Hash(attr_name);
	/* Search through the cache for the requested attribute. */
	if ((temp = DeeType_SetCachedAttr(iter, self, DeeString_STR(attr_name), hash, value)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *attr;
			if ((attr = DeeType_QueryAttributeWithHash(tp_self, iter, attr_name, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this attr. */
				if (!class_attribute_mayaccess(attr, iter)) {
					err_class_protected_member(iter, attr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_SetAttribute(desc, DeeInstance_DESC(desc, self), self, attr, value);
			}
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttr(tp_self, iter, DeeString_STR(attr_name), hash))
				goto noaccess;
			if (iter->tp_getsets &&
			    (temp = DeeType_SetGetSetAttr(tp_self, iter, self, DeeString_STR(attr_name), hash, value)) <= 0)
				goto done;
			if (iter->tp_members &&
			    (temp = DeeType_SetMemberAttr(tp_self, iter, self, DeeString_STR(attr_name), hash, value)) <= 0)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_setattr)
				return DeeType_INVOKE_SETATTR(iter, self, attr_name, value);
			/* Don't consider attributes from lower levels for custom attr access. */
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

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_getattr(DeeObject *self, DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL type_callattr(DeeObject *self, DeeObject *name, size_t argc, DeeObject *const *argv);
#ifndef DEFINE_TYPED_OPERATORS
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_getattr(DeeObject *self, DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *DCALL
DeeObject_TCallAttr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *name, size_t argc, DeeObject *const *argv);
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(DREF DeeObject *, CallAttr,
                (DeeObject *self, /*String*/ DeeObject *attr_name,
                 size_t argc, DeeObject *const *argv)) {
	DREF DeeObject *result;
	dhash_t hash;
	DeeTypeObject *iter;
	LOAD_TP_SELF;
	iter = tp_self;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(attr_name);
	ASSERT(DeeString_Check(attr_name));
	if (iter->tp_attr)
		goto do_iter_attr;
	hash = DeeString_Hash(attr_name);
	if ((result = DeeType_CallCachedAttr(iter, self, DeeString_STR(attr_name), hash, argc, argv)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *attr;
			if ((attr = DeeType_QueryAttributeWithHash(tp_self, iter, attr_name, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this attr. */
				if (!class_attribute_mayaccess(attr, iter)) {
					err_class_protected_member(iter, attr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttribute(desc,
				                                 DeeInstance_DESC(desc,
				                                                  self),
				                                 self, attr, argc, argv);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttr(tp_self, iter, self, DeeString_STR(attr_name), hash, argc, argv)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttr(tp_self, iter, self, DeeString_STR(attr_name), hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttr(tp_self, iter, self, DeeString_STR(attr_name), hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *getattr)(DeeObject *, /*String*/ DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return type_callattr(self, attr_name, argc, argv);
#ifndef DEFINE_TYPED_OPERATORS
			if (getattr == &super_getattr)
				return DeeObject_TCallAttr(DeeSuper_TYPE(self), DeeSuper_SELF(self), attr_name, argc, argv);
#endif /* !DEFINE_TYPED_OPERATORS */
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttr(((SeqEachBase *)self)->se_seq, attr_name, argc, argv);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttr(self, attr_name, argc, argv);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			result = (*getattr)(self, attr_name);
			goto done_invoke;
		}
	}
	err_unknown_attribute(DeeObject_Class(self),
	                      DeeString_STR(attr_name),
	                      ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_Call(result, argc, argv);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}

#ifndef DEFINE_TYPED_OPERATORS
#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallAttrPack, 16),
                    ASSEMBLY_NAME(DeeObject_CallAttr, 16));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VCallAttrPack(DeeObject *self,
                        /*String*/ DeeObject *attr_name,
                        size_t argc, va_list args) {
	return DeeObject_CallAttr(self, attr_name, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VCallAttrPack(DeeObject *self,
                        /*String*/ DeeObject *attr_name,
                        size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_CallAttr(self, attr_name, argc,
	                            DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !CONFIG_VA_LIST_IS_STACK_POINTER */
#endif /* !DEFINE_TYPED_OPERATORS */


#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
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
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(int, Enter,
               (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_with) {
			if (tp_self->tp_with->tp_enter)
				return DeeType_INVOKE_ENTER(tp_self, self);
			/* Special case: When `tp_leave' is implemented,
			 * a missing `tp_enter' behaves as a no-op. */
			if (tp_self->tp_with->tp_leave)
				return 0;
		}
	} while (type_inherit_with(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_ENTER);
}

DEFINE_OPERATOR(int, Leave,
               (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_with) {
			if (tp_self->tp_with->tp_leave)
				return DeeType_INVOKE_LEAVE(tp_self, self);
			/* Special case: When `tp_enter' is implemented,
			 * a missing `tp_leave' behaves as a no-op. */
			if (tp_self->tp_with->tp_enter)
				return 0;
		}
	} while (type_inherit_with(tp_self));
	return err_unimplemented_operator(tp_self, OPERATOR_LEAVE);
}


#ifndef DEFINE_TYPED_OPERATORS
INTERN NONNULL((1)) bool DCALL
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
		memcpy(self->tp_buffer, base_buffer,
		       sizeof(struct type_buffer));
	} else {
		self->tp_buffer = base_buffer;
	}
	return true;
}
#endif /* !DEFINE_TYPED_OPERATORS */


DEFINE_OPERATOR(int, GetBuf,
               (DeeObject *RESTRICT_IF_NOTYPE self,
                DeeBuffer *__restrict info,
                unsigned int flags)) {
	ASSERTF(!(flags & ~(Dee_BUFFER_FWRITABLE)),
	        "Unknown buffers flags in %x", flags);
	LOAD_TP_SELF;
	do {
		struct type_buffer *buf = tp_self->tp_buffer;
		if (buf && buf->tp_getbuf) {
			if unlikely((flags & Dee_BUFFER_FWRITABLE) &&
			            (buf->tp_buffer_flags & Dee_BUFFER_TYPE_FREADONLY)) {
				DeeError_Throwf(&DeeError_BufferError,
				                "Cannot write to read-only buffer of type %k",
				                tp_self);
				goto err;
			}
#ifndef __INTELLISENSE__
			info->bb_put = buf->tp_putbuf;
#endif /* !__INTELLISENSE__ */
			return (*buf->tp_getbuf)(self, info, flags);
		}
	} while (type_inherit_buffer(tp_self));
	err_unimplemented_operator(tp_self, OPERATOR_GETBUF);
err:
	return -1;
}

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
#elif 1
#ifdef DEFINE_TYPED_OPERATORS
	(void)tp_self;
#endif /* DEFINE_TYPED_OPERATORS */
	if (info->bb_put)
		(*info->bb_put)(self, info, flags);
#else
	LOAD_TP_SELF;
	do {
		if (tp_self->tp_buffer && tp_self->tp_buffer->tp_getbuf) {
			if (tp_self->tp_buffer->tp_putbuf)
				(*tp_self->tp_buffer->tp_putbuf)(self, info, flags);
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
#undef DEFINE_INTERNAL_OPERATOR
#undef DEFINE_OPERATOR


#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_Unpack)(DeeObject *__restrict self, size_t objc,
                         /*out*/ DREF DeeObject **__restrict objv) {
	DREF DeeObject *iterator, *elem;
	size_t fast_size, i = 0;
	/* Try to make use of the fast-sequence API. */
	fast_size = DeeFastSeq_GetSize(self);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		if (objc != fast_size)
			return err_invalid_unpack_size(self, objc, fast_size);
		for (; i < objc; ++i) {
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err;
			objv[i] = elem; /* Inherit reference. */
		}
		return 0;
	}
	if (DeeNone_Check(self)) {
		/* Special case: `none' can be unpacked into anything. */
		memsetp(objv, Dee_None, objc);
		Dee_Incref_n(Dee_None, objc);
		return 0;
	}
	/* Fallback: Use an iterator. */
	if ((iterator = DeeObject_IterSelf(self)) == NULL)
		goto err;
	for (; i < objc; ++i) {
		elem = DeeObject_IterNext(iterator);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem)
				err_invalid_unpack_size(self, objc, i);
			goto err_iter;
		}
		objv[i] = elem; /* Inherit reference. */
	}
	/* Check to make sure that the iterator actually ends here. */
	elem = DeeObject_IterNext(iterator);
	if unlikely(elem != ITER_DONE) {
		if (elem)
			err_invalid_unpack_iter_size(self, iterator, objc);
		goto err_iter;
	}
	Dee_Decref(iterator);
	return 0;
err_iter:
	Dee_Decref(iterator);
err:
	Dee_Decrefv(objv, i);
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeObject_Foreach(DeeObject *__restrict self,
                  dforeach_t proc, void *arg) {
	dssize_t temp, result = 0;
	DREF DeeObject *elem;
	size_t fast_size;
	fast_size = DeeFastSeq_GetSize(self);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		/* Optimization for fast-sequence object. */
		for (i = 0; i < fast_size; ++i) {
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0) {
				result = temp;
				break;
			}
			result += temp;
		}
		return result;
	}
	/* Fallback: Use an iterator. */
	if ((self = DeeObject_IterSelf(self)) == NULL)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0) {
			result = temp;
			break;
		}
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

/* Compare a pre-keyed `keyed_search_item' with `elem' using the given (optional) `key' function
 * @return: == -2: An error occurred.
 * @return: == -1: `lhs < key(rhs)'
 * @return: == 0:  Objects compare as equal
 * @return: == 1:  `lhs > key(rhs)' */
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
#endif /* !DEFINE_TYPED_OPERATORS */

#undef RESTRICT_IF_NOTYPE

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_C */
