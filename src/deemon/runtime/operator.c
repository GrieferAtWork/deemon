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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/list.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/operator-hints.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/int128.h>
#include <hybrid/limitcore.h> /* __INT32_MAX__, __INT64_MAX__ */

#include "../objects/int_logic.h"
#include "runtime_error.h"
#include "strings.h"
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */
#include <stdint.h> /* intN_t, uintN_t */

#ifndef INT32_MAX
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */
#ifndef INT64_MAX
#define INT64_MAX __INT64_MAX__
#endif /* !INT64_MAX */

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
#define DeeType_INVOKE_STR                               DeeType_InvokeCastStr
#define DeeType_INVOKE_PRINT                             DeeType_InvokeCastPrint
#define DeeType_INVOKE_REPR                              DeeType_InvokeCastRepr
#define DeeType_INVOKE_PRINTREPR                         DeeType_InvokeCastPrintRepr
#define DeeType_INVOKE_BOOL                              DeeType_InvokeCastBool
#define DeeType_INVOKE_CALL                              DeeType_InvokeCall
#define DeeType_INVOKE_CALLKW                            DeeType_InvokeCallKw
#define DeeType_INVOKE_INT32                             DeeType_InvokeMathInt32
#define DeeType_INVOKE_INT64                             DeeType_InvokeMathInt64
#define DeeType_INVOKE_DOUBLE                            DeeType_InvokeMathDouble
#define DeeType_INVOKE_INT                               DeeType_InvokeMathInt
#define DeeType_INVOKE_HASH                              DeeType_InvokeCmpHash
#define DeeType_INVOKE_GETRANGE                          DeeType_InvokeSeqGetRange
#define DeeType_INVOKE_DELRANGE                          DeeType_InvokeSeqDelRange
#define DeeType_INVOKE_SETRANGE                          DeeType_InvokeSeqSetRange
#define DeeType_INVOKE_GETRANGEINDEX                     DeeType_InvokeSeqGetRangeIndex
#define DeeType_INVOKE_DELRANGEINDEX                     DeeType_InvokeSeqDelRangeIndex
#define DeeType_INVOKE_SETRANGEINDEX                     DeeType_InvokeSeqSetRangeIndex
#define DeeType_INVOKE_GETRANGEINDEXN                    DeeType_InvokeSeqGetRangeIndexN
#define DeeType_INVOKE_DELRANGEINDEXN                    DeeType_InvokeSeqDelRangeIndexN
#define DeeType_INVOKE_SETRANGEINDEXN                    DeeType_InvokeSeqSetRangeIndexN
#else /* DEFINE_TYPED_OPERATORS */
#define DeeType_INVOKE_STR(tp_self, self)                                            (*(tp_self)->tp_cast.tp_str)(self)
#define DeeType_INVOKE_REPR(tp_self, self)                                           (*(tp_self)->tp_cast.tp_repr)(self)
#define DeeType_INVOKE_PRINT(tp_self, self, printer, arg)                            (*(tp_self)->tp_cast.tp_print)(self, printer, arg)
#define DeeType_INVOKE_PRINTREPR(tp_self, self, printer, arg)                        (*(tp_self)->tp_cast.tp_printrepr)(self, printer, arg)
#define DeeType_INVOKE_BOOL(tp_self, self)                                           (*(tp_self)->tp_cast.tp_bool)(self)
#define DeeType_INVOKE_CALL(tp_self, self, argc, argv)                               (*(tp_self)->tp_call)(self, argc, argv)
#define DeeType_INVOKE_CALLKW(tp_self, self, argc, argv, kw)                         (*(tp_self)->tp_call_kw)(self, argc, argv, kw)
#define DeeType_INVOKE_INT32(tp_self, self, result)                                  (*(tp_self)->tp_math->tp_int32)(self, result)
#define DeeType_INVOKE_INT64(tp_self, self, result)                                  (*(tp_self)->tp_math->tp_int64)(self, result)
#define DeeType_INVOKE_DOUBLE(tp_self, self, result)                                 (*(tp_self)->tp_math->tp_double)(self, result)
#define DeeType_INVOKE_INT(tp_self, self)                                            (*(tp_self)->tp_math->tp_int)(self)
#define DeeType_INVOKE_HASH(tp_self, self)                                           (*(tp_self)->tp_cmp->tp_hash)(self)
#define DeeType_INVOKE_GETRANGE(tp_self, self, start, end)                           (*(tp_self)->tp_seq->tp_getrange)(self, start, end)
#define DeeType_INVOKE_DELRANGE(tp_self, self, start, end)                           (*(tp_self)->tp_seq->tp_delrange)(self, start, end)
#define DeeType_INVOKE_SETRANGE(tp_self, self, start, end, values)                   (*(tp_self)->tp_seq->tp_setrange)(self, start, end, values)
#define DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start, end)                      (*(tp_self)->tp_seq->tp_getrange_index)(self, start, end)
#define DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start, end)                      (*(tp_self)->tp_seq->tp_delrange_index)(self, start, end)
#define DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start, end, value)               (*(tp_self)->tp_seq->tp_setrange_index)(self, start, end, value)
#define DeeType_INVOKE_GETRANGEINDEXN(tp_self, self, start)                          (*(tp_self)->tp_seq->tp_getrange_index_n)(self, start)
#define DeeType_INVOKE_DELRANGEINDEXN(tp_self, self, start)                          (*(tp_self)->tp_seq->tp_delrange_index_n)(self, start)
#define DeeType_INVOKE_SETRANGEINDEXN(tp_self, self, start, value)                   (*(tp_self)->tp_seq->tp_setrange_index_n)(self, start, value)
#endif /* !DEFINE_TYPED_OPERATORS */

#ifndef DEFINE_TYPED_OPERATORS
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
			goto err_object_type_r_not_implemented;
		}
		if unlikely(error)
			goto err_object_type_r;
		/* Begin tracking the returned object. */
		if (object_type->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
		return result;
	}
err_not_implemented:
	err_unimplemented_constructor(object_type, 0, NULL);
err:
	return NULL;
err_object_type_r_not_implemented:
	err_unimplemented_constructor(object_type, 0, NULL);
err_object_type_r:
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
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VNewPack, 12),
                    DCALL_ASSEMBLY_NAME(DeeObject_New, 12));
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
#define LOAD_TP_SELF  ASSERT_OBJECT_TYPE_A(self, tp_self)
#define GET_TP_SELF() tp_self
#else /* DEFINE_TYPED_OPERATORS */
#define LOAD_TP_SELF  DeeTypeObject *tp_self; \
                      ASSERT_OBJECT(self);    \
                      tp_self = Dee_TYPE(self)
#define GET_TP_SELF() Dee_TYPE(self)
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
(DCALL DeeObject_InplaceDeepCopyWithLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
                                          Dee_atomic_lock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_lock_acquire(p_lock);
	temp = *p_self;
	Dee_Incref(temp);
	Dee_atomic_lock_release(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_lock_acquire(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_lock_release(p_lock);
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_XInplaceDeepCopyWithLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
                                           Dee_atomic_lock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_lock_acquire(p_lock);
	temp = *p_self;
	if (!temp) {
		Dee_atomic_lock_release(p_lock);
		goto done;
	}
	Dee_Incref(temp);
	Dee_atomic_lock_release(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_lock_acquire(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_lock_release(p_lock);
	Dee_XDecref(temp);
done:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceDeepCopyWithRWLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
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
(DCALL DeeObject_XInplaceDeepCopyWithRWLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
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
                                     Dee_formatprinter_t printer, void *arg)) {
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
                                         Dee_formatprinter_t printer, void *arg)) {
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
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VCallPack, 12),
                    DCALL_ASSEMBLY_NAME(DeeObject_Call, 12));
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
DEFINE_OPERATOR(Dee_hash_t, Hash, (DeeObject *RESTRICT_IF_NOTYPE self)) {
	LOAD_TP_SELF;
	if likely((tp_self->tp_cmp && tp_self->tp_cmp->tp_hash) ||
	          (DeeType_InheritCompare(tp_self) && tp_self->tp_cmp->tp_hash)) {
		if likely(!(tp_self->tp_flags & TP_FGC)) {
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
PUBLIC WUNUSED /*ATTR_PURE*/ ATTR_INS(1, 2) Dee_hash_t
(DCALL DeeObject_Hashv)(DeeObject *const *__restrict object_vector,
                        size_t object_count) {
	size_t i;
	Dee_hash_t result;
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
		Dee_hash_t hsitem;
		hsitem = DeeObject_Hash(object_vector[i]);
		result = Dee_HashCombine(result, hsitem);
	}
	return result;
}

PUBLIC WUNUSED /*ATTR_PURE*/ ATTR_INS(1, 2) Dee_hash_t
(DCALL DeeObject_XHashv)(DeeObject *const *__restrict object_vector,
                         size_t object_count) {
	size_t i;
	Dee_hash_t result;
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
	{
		DeeObject *item = object_vector[0];
		result = item ? DeeObject_Hash(item) : DEE_HASHOF_UNBOUND_ITEM;
	}
	for (i = 1; i < object_count; ++i) {
		Dee_hash_t hsitem;
		DeeObject *item = object_vector[i];
		hsitem = item ? DeeObject_Hash(item) : DEE_HASHOF_UNBOUND_ITEM;
		result = Dee_HashCombine(result, hsitem);
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

DEFINE_OPERATOR(DREF DeeObject *, GetRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t start, DeeObject *end)) {
	Dee_ssize_t end_index;
	LOAD_TP_SELF;
	if (DeeInt_Check(end) && DeeInt_TryAsSSize(end, &end_index)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index) ||
		          unlikely(DeeType_InheritGetRange(tp_self)))
			return DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start, end_index);
	} else if (DeeNone_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index_n) ||
		          unlikely(DeeType_InheritGetRange(tp_self)))
			return DeeType_INVOKE_GETRANGEINDEXN(tp_self, self, start);
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
	Dee_ssize_t start_index;
	LOAD_TP_SELF;
	if (DeeInt_Check(start) && DeeInt_TryAsSSize(start, &start_index)) {
do_get_with_start_index:
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange_index) ||
		          unlikely(DeeType_InheritGetRange(tp_self)))
			return DeeType_INVOKE_GETRANGEINDEX(tp_self, self, start_index, end);
	} else if (DeeNone_Check(start)) {
		start_index = 0;
		goto do_get_with_start_index;
	} else {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_getrange) ||
		          unlikely(DeeType_InheritGetRange(tp_self))) {
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

DEFINE_OPERATOR(int, DelRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t start, DeeObject *end)) {
	Dee_ssize_t end_index;
	LOAD_TP_SELF;
	if (DeeInt_Check(end) && DeeInt_TryAsSSize(end, &end_index)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index) ||
		          unlikely(DeeType_InheritDelRange(tp_self)))
			return DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start, end_index);
	} else if (DeeNone_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index_n) ||
		          unlikely(DeeType_InheritDelRange(tp_self)))
			return DeeType_INVOKE_DELRANGEINDEXN(tp_self, self, start);
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
	Dee_ssize_t start_index;
	LOAD_TP_SELF;
	if (DeeInt_Check(start) && DeeInt_TryAsSSize(start, &start_index)) {
do_del_with_start_index:
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_delrange_index) ||
		          unlikely(DeeType_InheritDelRange(tp_self)))
			return DeeType_INVOKE_DELRANGEINDEX(tp_self, self, start_index, end);
	} else if (DeeNone_Check(start)) {
		start_index = 0;
		goto do_del_with_start_index;
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

DEFINE_OPERATOR(int, SetRangeBeginIndex,
                (DeeObject *self, Dee_ssize_t start, DeeObject *end, DeeObject *values)) {
	Dee_ssize_t end_index;
	LOAD_TP_SELF;
	if (DeeInt_Check(end) && DeeInt_TryAsSSize(end, &end_index)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index) ||
		          unlikely(DeeType_InheritSetRange(tp_self)))
			return DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start, end_index, values);
	} else if (DeeNone_Check(end)) {
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index_n) ||
		          unlikely(DeeType_InheritSetRange(tp_self)))
			return DeeType_INVOKE_SETRANGEINDEXN(tp_self, self, start, values);
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
	Dee_ssize_t start_index;
	LOAD_TP_SELF;
	if (DeeInt_Check(start) && DeeInt_TryAsSSize(start, &start_index)) {
do_set_with_start_index:
		if likely(likely(tp_self->tp_seq && tp_self->tp_seq->tp_setrange_index) ||
		          unlikely(DeeType_InheritSetRange(tp_self)))
			return DeeType_INVOKE_SETRANGEINDEX(tp_self, self, start_index, end, values);
	} else if (DeeNone_Check(start)) {
		start_index = 0;
		goto do_set_with_start_index;
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














#ifndef DEFINE_TYPED_OPERATORS
DEFINE_OPERATOR(DREF DeeObject *, IntInherited, (/*inherit(always)*/ DREF DeeObject *RESTRICT_IF_NOTYPE self)) {
#ifdef __OPTIMIZE_SIZE__
	DREF DeeObject *result;
	result = DeeObject_Int(self);
	Dee_Decref(self);
	return result;
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *result;
	LOAD_TP_SELF;
	if (tp_self == &DeeInt_Type)
		return self;
	if likely((tp_self->tp_math && tp_self->tp_math->tp_int) ||
	          DeeType_InheritInt(tp_self)) {
		result = DeeType_INVOKE_INT(tp_self, self);
	} else {
		err_unimplemented_operator(tp_self, OPERATOR_INT);
		result = NULL;
	}
	Dee_Decref(self);
	return result;
#endif /* !__OPTIMIZE_SIZE__ */
}

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

/* All of these return (T)-1 on error. When the object's actual value is `(T)-1', throw `IntegerOverflow' */
PUBLIC WUNUSED NONNULL((1)) uint8_t
(DCALL DeeObject_AsDirectUInt8)(DeeObject *__restrict self) {
	uint8_t result;
	if unlikely(DeeObject_AsUInt8(self, &result))
		goto err;
	if unlikely(result == (uint8_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(8, true);
err:
	return (uint8_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) uint16_t
(DCALL DeeObject_AsDirectUInt16)(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_AsUInt16(self, &result))
		goto err;
	if unlikely(result == (uint16_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(16, true);
err:
	return (uint16_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t
(DCALL DeeObject_AsDirectUInt32)(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_AsUInt32(self, &result))
		goto err;
	if unlikely(result == (uint32_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(32, true);
err:
	return (uint32_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) uint64_t
(DCALL DeeObject_AsDirectUInt64)(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_AsUInt64(self, &result))
		goto err;
	if unlikely(result == (uint64_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(64, true);
err:
	return (uint64_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) uint8_t
(DCALL DeeObject_AsDirectUInt8Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint8_t result;
	result = DeeObject_AsDirectUInt8(self);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint16_t
(DCALL DeeObject_AsDirectUInt16Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint16_t result;
	result = DeeObject_AsDirectUInt16(self);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t
(DCALL DeeObject_AsDirectUInt32Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint32_t result;
	result = DeeObject_AsDirectUInt32(self);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint64_t
(DCALL DeeObject_AsDirectUInt64Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint64_t result;
	result = DeeObject_AsDirectUInt64(self);
	Dee_Decref(self);
	return result;
}


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
PUBLIC WUNUSED NONNULL((1, 2)) int /* DEPRECATED! */
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
PUBLIC WUNUSED NONNULL((1, 2)) int /* DEPRECATED! */
(DCALL DeeObject_TryCmpKeyEqAsBool)(DeeObject *keyed_search_item,
                                    DeeObject *elem, /*nullable*/ DeeObject *key) {
	int result = key ? DeeObject_TryCompareKeyEq(keyed_search_item, elem, key)
	                 : DeeObject_TryCompareEq(keyed_search_item, elem);
	if unlikely(result == Dee_COMPARE_ERR)
		return -1;
	return result == 0 ? 1 : 0;
}

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

#undef GET_TP_SELF
#undef LOAD_TP_SELF
#undef DEFINE_OPERATOR
#undef RESTRICT_IF_NOTYPE

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_C */
