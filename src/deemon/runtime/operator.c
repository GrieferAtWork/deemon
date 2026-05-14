/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_C 1

#include <deemon/api.h>

#include <deemon/error-rt.h>       /* DeeRT_ErrNoBufferInterface */
#include <deemon/error.h>          /* DeeError_BufferError, DeeError_Throwf */
#include <deemon/int.h>            /* DeeIntObject, DeeInt_* */
#include <deemon/list.h>           /* DeeList_* */
#include <deemon/object.h>         /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE_A, ASSERT_OBJECT_TYPE_EXACT_OPT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BUFFER_FWRITABLE, Dee_Decref, Dee_Decrefv, Dee_HAS_ERR, Dee_Increfv, Dee_TYPE, Dee_buffer, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, return_reference_ */
#include <deemon/operator-hints.h> /* DeeType_Inherit*, DeeType_InvokeCastPrint, DeeType_InvokeCastPrintRepr, DeeType_InvokeCastRepr, DeeType_InvokeCastStr, DeeType_InvokeCmpHash */
#include <deemon/seq.h>            /* DeeSharedVector_Decref, DeeSharedVector_NewShared */
#include <deemon/string.h>         /* DeeString_PrintAscii, DeeString_Type */
#include <deemon/thread.h>         /* DeeThreadObject, DeeThread_Self, Dee_repr_frame, Dee_trepr_frame */
#include <deemon/tuple.h>          /* DeeTuple* */
#include <deemon/type.h>           /* ASSERT_OBJECT_TYPE_A, DeeType_InheritBuffer, DeeType_IsGC, Dee_BUFFER_TYPE_FREADONLY, OPERATOR_* */
#include <deemon/util/hash.h>      /* DeeObject_HashGeneric, Dee_HASHOF_RECURSIVE_ITEM */

#include "../objects/int_logic.h"
#include "runtime_error.h"
#include "strings.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* int8_t, uint8_t, uint32_t, uint64_t */

/************************************************************************/
/* Operator invocation.                                                 */
/************************************************************************/


DECL_BEGIN

#define do_fix_negative_range_index(index, size) \
	((size) - ((size_t)(-(index)) % (size)))

#ifndef DEFINE_OPERATOR
#define DEFINE_OPERATOR(return, name, args) \
	PUBLIC return (DCALL DeeObject_##name)args
#endif /* !DEFINE_OPERATOR */

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
#define DeeType_INVOKE_STR       DeeType_InvokeCastStr
#define DeeType_INVOKE_PRINT     DeeType_InvokeCastPrint
#define DeeType_INVOKE_REPR      DeeType_InvokeCastRepr
#define DeeType_INVOKE_PRINTREPR DeeType_InvokeCastPrintRepr
#define DeeType_INVOKE_HASH      DeeType_InvokeCmpHash
#else /* DEFINE_TYPED_OPERATORS */
#define DeeType_INVOKE_STR(tp_self, self)                     (*(tp_self)->tp_cast.tp_str)(self)
#define DeeType_INVOKE_REPR(tp_self, self)                    (*(tp_self)->tp_cast.tp_repr)(self)
#define DeeType_INVOKE_PRINT(tp_self, self, printer, arg)     (*(tp_self)->tp_cast.tp_print)(self, printer, arg)
#define DeeType_INVOKE_PRINTREPR(tp_self, self, printer, arg) (*(tp_self)->tp_cast.tp_printrepr)(self, printer, arg)
#define DeeType_INVOKE_HASH(tp_self, self)                    (*(tp_self)->tp_cmp->tp_hash)(self)
#endif /* !DEFINE_TYPED_OPERATORS */

#ifdef DEFINE_TYPED_OPERATORS
#define LOAD_TP_SELF  ASSERT_OBJECT_TYPE_A(self, tp_self)
#define GET_TP_SELF() tp_self
#else /* DEFINE_TYPED_OPERATORS */
#define LOAD_TP_SELF  DeeTypeObject *tp_self; \
                      ASSERT_OBJECT(self);    \
                      tp_self = Dee_TYPE(self)
#define GET_TP_SELF() Dee_TYPE(self)
#endif /* !DEFINE_TYPED_OPERATORS */

#ifdef DEFINE_TYPED_OPERATORS
LOCAL WUNUSED bool DCALL
repr_contains(struct Dee_trepr_frame *chain, DeeTypeObject *tp, DeeObject *ob)
#else /* DEFINE_TYPED_OPERATORS */
LOCAL WUNUSED bool DCALL
repr_contains(struct Dee_repr_frame *chain, DeeObject *__restrict ob)
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
STATIC_ASSERT(offsetof(struct Dee_trepr_frame, rf_prev) ==
              offsetof(struct Dee_repr_frame, rf_prev));
STATIC_ASSERT(offsetof(struct Dee_trepr_frame, rf_obj) ==
              offsetof(struct Dee_repr_frame, rf_obj));

#ifdef DEFINE_TYPED_OPERATORS
#define Xrepr_frame Dee_trepr_frame
#else /* DEFINE_TYPED_OPERATORS */
#define Xrepr_frame Dee_repr_frame
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_OPERATOR(DREF DeeObject *, Str, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_str && !DeeType_InheritStr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if unlikely(DeeType_IsGC(tp_self)) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __str__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_str_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj          = self;
		opframe.rf_type         = tp_self;
		this_thread->t_str_curr = (struct Dee_repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_str_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_STR(tp_self, self);
		this_thread->t_str_curr = (struct Dee_repr_frame *)opframe.rf_prev;
		ASSERT_OBJECT_TYPE_EXACT_OPT(result, &DeeString_Type);
		return result;
	}

	/* Non-gc object (much simpler) */
	return DeeType_INVOKE_STR(tp_self, self);
missing:
	err_unimplemented_operator(tp_self, OPERATOR_STR);
	return NULL;
recursion:
	return_reference_(Dee_AsObject(&str_dots));
}

DEFINE_OPERATOR(DREF DeeObject *, Repr, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_repr && !DeeType_InheritRepr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if (DeeType_IsGC(tp_self)) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __repr__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_repr_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj           = self;
		opframe.rf_type          = tp_self;
		this_thread->t_repr_curr = (struct Dee_repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_repr_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_REPR(tp_self, self);
		this_thread->t_repr_curr = (struct Dee_repr_frame *)opframe.rf_prev;
		ASSERT_OBJECT_TYPE_EXACT_OPT(result, &DeeString_Type);
		return result;
	}

	/* Non-gc object (much simpler) */
	return DeeType_INVOKE_REPR(tp_self, self);
missing:
	err_unimplemented_operator(tp_self, OPERATOR_REPR);
	return NULL;
recursion:
	return_reference_(Dee_AsObject(&str_dots));
}

DEFINE_OPERATOR(Dee_ssize_t, Print, (DeeObject *RESTRICT_IF_NOTYPE self,
                                     Dee_formatprinter_t printer, void *arg)) {
	Dee_ssize_t result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_print && !DeeType_InheritStr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if unlikely(DeeType_IsGC(tp_self)) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __str__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_str_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj          = self;
		opframe.rf_type         = tp_self;
		this_thread->t_str_curr = (struct Dee_repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_str_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_PRINT(tp_self, self, printer, arg);
		this_thread->t_str_curr = (struct Dee_repr_frame *)opframe.rf_prev;
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
                                         Dee_formatprinter_t printer, void *arg)) {
	Dee_ssize_t result;
	LOAD_TP_SELF;
	if unlikely(!tp_self->tp_cast.tp_printrepr && !DeeType_InheritRepr(tp_self))
		goto missing;

	/* Handle string-repr recursion for GC objects. */
	if (DeeType_IsGC(tp_self)) {
		struct Xrepr_frame opframe;
		DeeThreadObject *this_thread = DeeThread_Self();

		/* Trace objects for which __repr__ is being invoked. */
		opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_repr_curr;
#ifdef DEFINE_TYPED_OPERATORS
		if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
			goto recursion;
		opframe.rf_obj           = self;
		opframe.rf_type          = tp_self;
		this_thread->t_repr_curr = (struct Dee_repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
		if unlikely(repr_contains(opframe.rf_prev, self))
			goto recursion;
		opframe.rf_obj = self;
		this_thread->t_repr_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
		result = DeeType_INVOKE_PRINTREPR(tp_self, self, printer, arg);
		this_thread->t_repr_curr = (struct Dee_repr_frame *)opframe.rf_prev;
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


#ifdef DEFINE_TYPED_OPERATORS
#define Xrepr_frame Dee_trepr_frame
#else /* DEFINE_TYPED_OPERATORS */
#define Xrepr_frame Dee_repr_frame
#endif /* !DEFINE_TYPED_OPERATORS */

WUNUSED /*ATTR_PURE*/
DEFINE_OPERATOR(Dee_hash_t, Hash, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_cmp && tp_self->tp_cmp->tp_hash) ||
	          (DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_hash)) {
		if likely(!DeeType_IsGC(tp_self)) {
			return DeeType_INVOKE_HASH(tp_self, self);
		} else {
			/* Handle hash recursion for GC objects. */
			Dee_hash_t result;
			struct Xrepr_frame opframe;
			DeeThreadObject *this_thread = DeeThread_Self();

			/* Trace objects for which __hash__ is being invoked. */
			opframe.rf_prev = (struct Xrepr_frame *)this_thread->t_hash_curr;
#ifdef DEFINE_TYPED_OPERATORS
			if unlikely(repr_contains(opframe.rf_prev, tp_self, self))
				goto recursion;
			opframe.rf_obj           = self;
			opframe.rf_type          = tp_self;
			this_thread->t_hash_curr = (struct Dee_repr_frame *)&opframe;
#else /* DEFINE_TYPED_OPERATORS */
			if unlikely(repr_contains(opframe.rf_prev, self))
				goto recursion;
			opframe.rf_obj = self;
			this_thread->t_hash_curr = &opframe;
#endif /* !DEFINE_TYPED_OPERATORS */
			result = DeeType_INVOKE_HASH(tp_self, self);
			this_thread->t_hash_curr = (struct Dee_repr_frame *)opframe.rf_prev;
			return result;
		}
	}
	return DeeObject_HashGeneric(self);
recursion:
	return Dee_HASHOF_RECURSIVE_ITEM;
}

#undef Xrepr_frame

#ifndef DEFINE_TYPED_OPERATORS
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_AddInt8(DeeObject *__restrict self, int8_t val) {
	DREF DeeObject *val_ob, *result;
	/* Optimization for `int' */
	if (DeeInt_Check(self))
		return Dee_AsObject(DeeInt_AddSDigit((DeeIntObject *)self, val));
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
		return Dee_AsObject(DeeInt_SubSDigit((DeeIntObject *)self, val));
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
		return Dee_AsObject(DeeInt_AddUInt32((DeeIntObject *)self, val));
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
		return Dee_AsObject(DeeInt_SubUInt32((DeeIntObject *)self, val));
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


#define DEFINE_COMPARE_ASBOOL_OPERATOR(name_asbool, name)  \
	PUBLIC WUNUSED NONNULL((1, 2)) int DCALL               \
	name_asbool(DeeObject *self, DeeObject *some_object) { \
		DREF DeeObject *result;                            \
		result = name(self, some_object);                  \
		if unlikely(!result)                               \
			goto err;                                      \
		return DeeObject_BoolInherited(result);            \
	err:                                                   \
		return Dee_HAS_ERR;                                \
	}
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpEqAsBool, DeeObject_CmpEq)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpNeAsBool, DeeObject_CmpNe)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpLoAsBool, DeeObject_CmpLo)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpLeAsBool, DeeObject_CmpLe)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpGrAsBool, DeeObject_CmpGr)
DEFINE_COMPARE_ASBOOL_OPERATOR(DeeObject_CmpGeAsBool, DeeObject_CmpGe)
#undef DEFINE_COMPARE_ASBOOL_OPERATOR

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_ConcatInherited(/*inherit(always)*/ DREF DeeObject *self, DeeObject *other) {
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
DeeObject_ExtendInherited(/*inherit(always)*/ DREF DeeObject *self, size_t argc,
                          /*inherit(always)*/ DREF DeeObject *const *argv) {
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


DEFINE_OPERATOR(int, GetBuf, (DeeObject *RESTRICT_IF_NOTYPE self,
                              struct Dee_buffer *__restrict info, unsigned int flags)) {
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
		return (*tp_self->tp_buffer->tp_getbuf)(self, info, flags);
	}
	DeeRT_ErrNoBufferInterface(tp_self);
err:
	return -1;
}

#undef GET_TP_SELF
#undef LOAD_TP_SELF
#undef DEFINE_OPERATOR
#undef RESTRICT_IF_NOTYPE

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_C */
