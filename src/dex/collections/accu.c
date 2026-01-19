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
#ifndef GUARD_DEX_COLLECTIONS_ACCU_C
#define GUARD_DEX_COLLECTIONS_ACCU_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/api.h>

#include <deemon/accu.h>
#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/types.h>

#include "accu.h"

DECL_BEGIN

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_accu_already_acquired(AccuObject *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_RuntimeError, "Reentrant calls to `collections.Accu' are not allowed");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
accu_acquire(AccuObject *__restrict self) {
	int status = Dee_nrshared_lock_acquire(&self->a_lock);
	if (status == Dee_NRLOCK_OK)
		return 0;
	if (status == Dee_NRLOCK_ALREADY)
		return err_accu_already_acquired(self);
	return -1;
}

#define accu_release(self) \
	Dee_nrshared_lock_release(&(self)->a_lock)


/* @return: ITER_DONE: Accumulator is unbound */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
accu_trygetvalue(AccuObject *__restrict self) {
	DREF DeeObject *result;
	if unlikely(accu_acquire(self))
		goto err;
	result = Dee_accu_pack(&self->a_accu, ITER_DONE);
	if (ITER_ISOK(result)) {
		Dee_accu_init_with_first(&self->a_accu, result);
	} else {
		Dee_accu_init(&self->a_accu);
	}
	accu_release(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
accu_getvalue(AccuObject *__restrict self) {
	DREF DeeObject *result = accu_trygetvalue(self);
	if (result == ITER_DONE)
		result = DeeRT_ErrUnboundAttrCStr(self, "value");
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
accu_delvalue(AccuObject *__restrict self) {
	if unlikely(accu_acquire(self))
		goto err;
	Dee_accu_fini(&self->a_accu);
	Dee_accu_init(&self->a_accu);
	accu_release(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
accu_setvalue(AccuObject *self, DeeObject *value) {
	if unlikely(accu_acquire(self))
		goto err;
	Dee_accu_fini(&self->a_accu);
	Dee_accu_init_with_first(&self->a_accu, value);
	accu_release(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
accu_boundvalue(AccuObject *self) {
	int result;
	if unlikely(accu_acquire(self))
		goto err;
	result = Dee_BOUND_FROMBOOL(self->a_accu.acu_mode != Dee_ACCU_FIRST);
	accu_release(self);
	return result;
err:
	return Dee_BOUND_ERR;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
accu_ctor(AccuObject *__restrict self) {
	Dee_nrshared_lock_init(&self->a_lock);
	Dee_accu_init(&self->a_accu);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
accu_init(AccuObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *init;
	DeeArg_Unpack1(err, argc, argv, "Accu", &init);
	Dee_nrshared_lock_init(&self->a_lock);
	Dee_accu_init_with_first(&self->a_accu, init);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
accu_copy(AccuObject *__restrict self, AccuObject *__restrict other) {
	DREF DeeObject *value = accu_trygetvalue(other);
	if (value == NULL)
		goto err;
	if (value == ITER_DONE) {
		Dee_accu_init(&self->a_accu);
	} else {
		Dee_accu_init_with_first_inherited(&self->a_accu, value);
	}
	Dee_nrshared_lock_init(&self->a_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
accu_deep(AccuObject *__restrict self, AccuObject *__restrict other) {
	DREF DeeObject *value = accu_trygetvalue(other);
	if (value == NULL)
		goto err;
	if (value == ITER_DONE) {
		Dee_accu_init(&self->a_accu);
	} else {
		if unlikely(DeeObject_InplaceDeepCopy(&value))
			goto err_value;
		Dee_accu_init_with_first_inherited(&self->a_accu, value);
	}
	Dee_nrshared_lock_init(&self->a_lock);
	return 0;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
accu_fini(AccuObject *__restrict self) {
	Dee_accu_fini(&self->a_accu);
}

PRIVATE NONNULL((1, 2)) void DCALL
accu_visit(AccuObject *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_accu_visit(&self->a_accu, proc, arg);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
accu_bool(AccuObject *__restrict self) {
	int result;
	if unlikely(accu_acquire(self))
		goto err;
	result = self->a_accu.acu_mode != Dee_ACCU_FIRST;
	accu_release(self);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
accu_print(AccuObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DREF DeeObject *value = accu_trygetvalue(self);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return DeeFormat_PRINT(printer, arg, "<empty accumulation>");
	return DeeFormat_Printf(printer, arg, "<accumulation with %K>", value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
accu_printrepr(AccuObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DREF DeeObject *value = accu_trygetvalue(self);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return DeeFormat_PRINT(printer, arg, "Accu()");
	return DeeFormat_Printf(printer, arg, "Accu(%r)", value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF AccuObject *DCALL
accu_add(AccuObject *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *value;
	Dee_ssize_t status;
	DeeArg_Unpack1(err, argc, argv, "add", &value);
	if unlikely(accu_acquire(self))
		goto err;
	status = Dee_accu_add(&self->a_accu, value);
	accu_release(self);
	if unlikely(status < 0)
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF AccuObject *DCALL
accu_addall(AccuObject *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *values;
	Dee_ssize_t status;
	DeeArg_Unpack1(err, argc, argv, "add", &values);
	if unlikely(accu_acquire(self))
		goto err;
	status = Dee_accu_addall(&self->a_accu, values);
	accu_release(self);
	if unlikely(status < 0)
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
accu_hash(AccuObject *__restrict self) {
	(void)self; /* TODO */
	return (Dee_hash_t)42;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
accu_compare_impl(AccuObject *lhs, AccuObject *rhs,
                  int (DCALL *compare)(DeeObject *lhs, DeeObject *rhs)) {
	int result;
	DREF DeeObject *lhs_ob, *rhs_ob;
	lhs_ob = accu_trygetvalue(lhs);
	if (lhs_ob == NULL)
		goto err;
	rhs_ob = accu_trygetvalue(rhs);
	if (rhs_ob == NULL)
		goto err_lhs;
	result = (*compare)(lhs_ob, rhs_ob);
	Dee_Decref(rhs_ob);
	Dee_Decref(lhs_ob);
	return result;
err_lhs:
	Dee_Decref(lhs_ob);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
accu_compare_eq(AccuObject *lhs, AccuObject *rhs) {
	if (DeeObject_AssertType(rhs, &Accu_Type))
		goto err;
	return accu_compare_impl(lhs, rhs, &DeeObject_TryCompareEq);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
accu_trycompare_eq(AccuObject *lhs, AccuObject *rhs) {
	if (!DeeObject_InstanceOf(rhs, &Accu_Type))
		return Dee_COMPARE_NE;
	return accu_compare_impl(lhs, rhs, &DeeObject_TryCompareEq);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
accu_compare(AccuObject *lhs, AccuObject *rhs) {
	if (DeeObject_AssertType(rhs, &Accu_Type))
		goto err;
	return accu_compare_impl(lhs, rhs, &DeeObject_Compare);
err:
	return Dee_COMPARE_ERR;
}


PRIVATE struct type_cmp accu_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&accu_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&accu_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&accu_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&accu_trycompare_eq,
};

PRIVATE struct type_method tpconst accu_methods[] = {
	TYPE_METHOD("add", &accu_add,
	            "(ob)->?.\n"
	            "Add @ob to the accumulator before re-returning it.\n"
	            "${"
	            /**/ "function add(item: Object): Accu {\n"
	            /**/ "	// Not the actual implementation (actual impl is atomic in regards\n"
	            /**/ "	// to other threads also adding items, prevents reentrant calls, and\n"
	            /**/ "	// has various optimizations for different objects being accumulated\n"
	            /**/ "	this.value = this.value is bound ? (this.value + item) : item;\n"
	            /**/ "	return this;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("addall", &accu_addall,
	            "(objs:?S?O)->?.\n"
	            "Iterate @objs and add its elements to the accumulator before re-returning it.\n"
	            "${"
	            /**/ "function addall(objs: {Object...}): Accu {\n"
	            /**/ "	for (local ob: objs)\n"
	            /**/ "		this.add(ob);\n"
	            /**/ "	return this;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst accu_getsets[] = {
	TYPE_GETSET_BOUND("value", &accu_getvalue, &accu_delvalue, &accu_setvalue, &accu_boundvalue,
	                  "Result of the accumulation so far (lazily materialized into an object "
	                  /**/ "on access, meaning reading this getset the first time after adding "
	                  /**/ "something to the accumulator can result in OOM and the like)"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject Accu_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Accu",
	/* .tp_doc      = */ DOC("Object accumulator (via ${operator +}). This type can be used to efficiently "
	                         /**/ "sum up an arbitrary number of objects (in order of them being added), while "
	                         /**/ "doing so much more efficiently when adding certain types of objects (such as "
	                         /**/ "strings or integers):\n"
	                         "${"
	                         /**/ "/* Any code like this: ... */\n"
	                         /**/ "local result = a + b + c + d + e + f + g;\n"
	                         /**/ "\\\n"
	                         /**/ "/* ... becomes more efficient when written as (one of) */\n"
	                         /**/ "local result = Accu(a).add(b).add(c).add(d).add(e).add(f).add(g).value;\n"
	                         /**/ "local result = Accu().add(a).add(b).add(c).add(d).add(e).add(f).add(g).value;\n"
	                         /**/ "local result = Accu().addall({ a, b, c, d, e, f, g }).value;"
	                         "}\n"
	                         "The current implementation of ?. offers dedicated optimizations for building instances of:"
	                         "#L-"
	                         /**/ "{?Dstring"
	                         /**/ "|?Dint"
	                         /**/ "|?Dfloat"
	                         /**/ "|?DTuple"
	                         /**/ "|?DList"
	                         /**/ "|?DBytes"
	                         /**/ "|?N"
	                         /**/ "|All other types are accumulated by directly invoking their ${operator +}"
	                         "}\n"
	                         "\n"
	                         "(init?)\n"
	                         "\n"
	                         "bool->\n"
	                         "Returns true if ?#value is bound (at least "
	                         /**/ "1 element was added to the accumulator)\n"
	                         "\n"
	                         "==->\n"
	                         "!=->\n"
	                         "<->\n"
	                         "<=->\n"
	                         ">->\n"
	                         ">=->\n"
	                         "Compare ?#value of @this and @other"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ AccuObject,
			/* tp_ctor:        */ &accu_ctor,
			/* tp_copy_ctor:   */ &accu_copy,
			/* tp_deep_ctor:   */ &accu_deep,
			/* tp_any_ctor:    */ &accu_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&accu_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&accu_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&accu_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&accu_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&accu_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &accu_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ accu_methods,
	/* .tp_getsets       = */ accu_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_ACCU_C */
