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
#ifndef GUARD_DEEMON_OBJECTS_CELL_C
#define GUARD_DEEMON_OBJECTS_CELL_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/cell.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/util/lock.h>

#include "../runtime/strings.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

/* Construct a new Cell object. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_New(DeeObject *__restrict item) {
	DREF DeeCellObject *result;
	ASSERT_OBJECT(item);
	result = DeeGCObject_MALLOC(DeeCellObject);
	if unlikely(!result)
		goto err;
	/* Initialize and fill in the new Cell. */
	DeeObject_Init(result, &DeeCell_Type);
	Dee_Incref(item);
	result->c_item = item;
	Dee_atomic_rwlock_init(&result->c_lock);
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_ctor(DeeCellObject *__restrict self) {
	self->c_item = NULL;
	Dee_atomic_rwlock_init(&self->c_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cell_copy(DeeCellObject *__restrict self,
          DeeCellObject *__restrict other) {
	DeeCell_LockRead(other);
	self->c_item = other->c_item;
	Dee_XIncref(self->c_item);
	DeeCell_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->c_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_init(DeeCellObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "Cell", &self->c_item);
	Dee_Incref(self->c_item);
	Dee_atomic_rwlock_init(&self->c_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
cell_fini(DeeCellObject *__restrict self) {
	Dee_XDecref(self->c_item);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cell_serialize(DeeCellObject *__restrict self,
               DeeSerial *__restrict writer,
               Dee_seraddr_t addr) {
	DREF DeeObject *value = DeeCell_TryGet(Dee_AsObject(self));
	DeeCellObject *out = DeeSerial_Addr2Mem(writer, addr, DeeCellObject);
	Dee_atomic_rwlock_init(&out->c_lock);
	return DeeSerial_XPutObjectInherited(writer, addr + offsetof(DeeCellObject, c_item), value);
}

PRIVATE NONNULL((1, 2)) int DCALL
cell_assign(DeeCellObject *self,
            DeeCellObject *other) {
	DREF DeeObject *old_obj, *new_obj;
	if (DeeObject_AssertType(other, &DeeCell_Type))
		goto err;
	DeeCell_LockRead(other);
	new_obj = other->c_item;
	Dee_XIncref(new_obj);
	DeeCell_LockEndRead(other);
	DeeCell_LockWrite(self);
	old_obj = self->c_item;
	self->c_item = new_obj;
	DeeCell_LockEndWrite(self);
	Dee_XDecref(old_obj);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) int DCALL
cell_moveassign(DeeCellObject *__restrict self,
                DeeCellObject *__restrict other) {
	DREF DeeObject *old_obj, *new_obj;
	ASSERT_OBJECT_TYPE(other, &DeeCell_Type);
	DeeCell_LockWrite(other);
	new_obj = other->c_item;
	other->c_item = NULL; /* Steal */
	DeeCell_LockEndWrite(other);
	DeeCell_LockWrite(self);
	old_obj = self->c_item;
	self->c_item = new_obj;
	DeeCell_LockEndWrite(self);
	Dee_XDecref(old_obj);
	return 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
cell_visit(DeeCellObject *__restrict self,
           Dee_visit_t proc, void *arg) {
	DeeCell_LockRead(self);
	Dee_XVisit(self->c_item);
	DeeCell_LockEndRead(self);
}

PRIVATE NONNULL((1)) void DCALL
cell_clear(DeeCellObject *__restrict self) {
	DREF DeeObject *old_obj;
	DeeCell_LockWrite(self);
	old_obj      = self->c_item;
	self->c_item = NULL;
	DeeCell_LockEndWrite(self);
	Dee_XDecref(old_obj);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_deepload(DeeCellObject *__restrict self) {
	return DeeObject_XInplaceDeepCopyWithRWLock(&self->c_item,
	                                            &self->c_lock);
}


/* Get/Del/Set the value associated with a given Cell.
 * HINT: These are the getset callbacks used for `Cell.item' (or its deprecated name `Cell.value').
 *       With that in mind, `DeeCell_Del()' and `DeeCell_Set()'
 *       always return `0' indicative of a successful callback.
 * NOTE: `DeeCell_Get' will return `NULL' and throw an `UnboundAttribute' if the `self' is
 *       empty, whereas `DeeCell_TryGet()' will do the same, but never throw any error. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_TryGet(DeeObject *__restrict self) {
	DeeCellObject *me = (DeeCellObject *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(me, &DeeCell_Type);
	DeeCell_LockRead(me);
	result = me->c_item;
	Dee_XIncref(result);
	DeeCell_LockEndRead(me);
	return result;
}

PRIVATE ATTR_COLD int DCALL err_empty_cell(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "The cell is empty");
}



PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_Get(DeeObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeCell_TryGet(self);
	if unlikely(!result) {
		DeeError_Throwf(&DeeError_UnboundAttribute,
		                "The cell is empty");
		/* No mitochondria here... */
	}
	return result;
}

/* Exchange the Cell's value.
 * NOTE: `DeeCell_XchIfNotNull()' will only set the new value when the old was non-NULL. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_Xch(DeeObject *self, DeeObject *value) {
	DeeCellObject *me = (DeeCellObject *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(me, &DeeCell_Type);
	ASSERT_OBJECT_OPT(value);

	/* Exchange the Cell's value. */
	Dee_XIncref(value);
	DeeCell_LockWrite(me);
	result = me->c_item;
	me->c_item = value;
	DeeCell_LockEndWrite(me);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_XchIfNotNull(DeeObject *self, DeeObject *value) {
	DeeCellObject *me = (DeeCellObject *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(me, &DeeCell_Type);
	ASSERT_OBJECT_OPT(value);

	/* Exchange the Cell's value. */
	Dee_XIncref(value);
	DeeCell_LockWrite(me);
	result = me->c_item;
	if unlikely(!result) {
		DeeCell_LockEndWrite(me);
		Dee_XDecrefNokill(value);
	} else {
		me->c_item = value;
		DeeCell_LockEndWrite(me);
	}
	return result;
}

/* Perform a compare-exchange, returning the old value of the Cell. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_CmpXch(DeeObject *self,
               DeeObject *old_value,
               DeeObject *new_value) {
	DeeCellObject *me = (DeeCellObject *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(me, &DeeCell_Type);
	ASSERT_OBJECT_OPT(old_value);
	ASSERT_OBJECT_OPT(new_value);

	/* Compare-and-exchange the Cell's value. */
	DeeCell_LockWrite(me);
	result = me->c_item;
	if (result == old_value) {
		Dee_XIncref(new_value);
		me->c_item = new_value;
	} else {
		Dee_XIncref(result);
	}
	DeeCell_LockEndWrite(me);
	return result;
}

PUBLIC NONNULL((1)) int DCALL
DeeCell_Del(DeeObject *__restrict self) {
	DeeCellObject *me = (DeeCellObject *)self;
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(me, &DeeCell_Type);

	/* Extract the Cell's value. */
	DeeCell_LockWrite(me);
	old_value  = me->c_item;
	me->c_item = NULL;
	DeeCell_LockEndWrite(me);

	/* Decref() the old value. */
	Dee_XDecref(old_value);
	return 0;
}

PUBLIC NONNULL((1)) int DCALL
DeeCell_Set(DeeObject *self, DeeObject *value) {
	DeeCellObject *me = (DeeCellObject *)self;
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(me, &DeeCell_Type);
	ASSERT_OBJECT(value);
	Dee_Incref(value);

	/* Exchange the Cell's value. */
	DeeCell_LockWrite(me);
	old_value  = me->c_item;
	me->c_item = value;
	DeeCell_LockEndWrite(me);

	/* Decref() the old value. */
	Dee_XDecref(old_value);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cell_print(DeeCellObject *__restrict self,
           Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *item;
	item = DeeCell_TryGet(Dee_AsObject(self));
	if (item) {
		result = DeeFormat_Printf(printer, arg, "<cell with %k>", item);
		Dee_Decref(item);
	} else {
		result = DeeFormat_PRINT(printer, arg, "<empty cell>");
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cell_printrepr(DeeCellObject *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeObject *item;
	item = DeeCell_TryGet(Dee_AsObject(self));
	if (item) {
		result = DeeFormat_Printf(printer, arg, "Cell(%r)", item);
		Dee_Decref(item);
	} else {
		result = DeeFormat_PRINT(printer, arg, "Cell()");
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_bool(DeeCellObject *__restrict self) {
	return DeeCell_IsBound(self) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_value_bound(DeeCellObject *__restrict self) {
	bool isbound = DeeCell_IsBound(self);
	return Dee_BOUND_FROMBOOL(isbound);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
cell_hash(DeeCellObject *__restrict self) {
	return DeeCell_GetHash(self);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cell_compare(DeeCellObject *self, DeeCellObject *other) {
	if (DeeObject_AssertType(other, &DeeCell_Type))
		goto err;
	Dee_return_compareT(uintptr_t, DeeObject_Id(DeeCell_GetItemPointer(self)),
	                    /*      */ DeeObject_Id(DeeCell_GetItemPointer(other)));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cell_trycompare_eq(DeeCellObject *self, DeeCellObject *other) {
	uintptr_t lhs_id, rhs_id;
	if unlikely(!DeeCell_Check(other))
		return -1;
	lhs_id = DeeObject_Id(DeeCell_GetItemPointer(self));
	rhs_id = DeeObject_Id(DeeCell_GetItemPointer(other));
	return lhs_id == rhs_id ? 0 : 1;
}

PRIVATE struct type_cmp cell_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&cell_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&cell_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&cell_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&cell_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE struct type_getset tpconst cell_getsets[] = {
	TYPE_GETSET_BOUND_F("value", &DeeCell_Get, &DeeCell_Del, &DeeCell_Set, &cell_value_bound,
	                    METHOD_FNOREFESCAPE,
	                    "#tUnboundAttribute{Attempted to read from an empty Cell}"
	                    "Read/write access to the underlying, contained ?O"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_get(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *def = NULL, *result;
	DeeArg_Unpack0Or1(err, argc, argv, "get", &def);
	result = DeeCell_TryGet(Dee_AsObject(self));
	if (!result) {
		result = def;
		if (!result)
			goto err_empty;
		Dee_Incref(def);
	}
	return result;
err_empty:
	err_empty_cell();
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_delete(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval;
	DeeArg_Unpack0(err, argc, argv, "delete");
	oldval = DeeCell_Xch(Dee_AsObject(self), NULL);
	if (!oldval)
		return_false;
	Dee_Decref(oldval);
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_pop(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval, *def = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, "pop", &def);
	oldval = DeeCell_Xch(Dee_AsObject(self), NULL);
	if (!oldval) {
		if (def)
			return_reference_(def);
		goto err_empty;
	}
	return oldval;
err_empty:
	err_empty_cell();
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_set(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *newval;
	DeeArg_Unpack1(err, argc, argv, "set", &newval);
	newval = DeeCell_Xch(Dee_AsObject(self), newval);
	if (!newval)
		return_false;
	Dee_Decref(newval);
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_xch(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *value, *def = NULL, *result;
	DeeArg_Unpack1Or2(err, argc, argv, "o|o:xch", &value, &def);
	if (def) {
		result = DeeCell_Xch(Dee_AsObject(self), value);
		if (!result) {
			Dee_Incref(def);
			result = def;
		}
	} else {
		result = DeeCell_XchIfNotNull(Dee_AsObject(self), value);
		if (!result)
			goto err_empty;
	}
	return result;
err_empty:
	err_empty_cell();
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_cmpdel(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval, *result;
	DeeArg_Unpack1(err, argc, argv, "o:cmpdel", &oldval);
	result = DeeCell_CmpXch(Dee_AsObject(self), oldval, NULL);
	Dee_XDecref(result);
	return_bool(result == oldval);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_cmpxch(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("cmpxch", params: """
	DeeObject *oldval;
	DeeObject *newval = NULL;
	DeeObject *def = NULL;
""");]]]*/
	struct {
		DeeObject *oldval;
		DeeObject *newval;
		DeeObject *def;
	} args;
	args.newval = NULL;
	args.def = NULL;
	DeeArg_UnpackStruct1Or2Or3(err, argc, argv, "cmpxch", &args, &args.oldval, &args.newval, &args.def);
/*[[[end]]]*/
	if (args.newval) {
		result = DeeCell_CmpXch(Dee_AsObject(self), args.oldval, args.newval);
		if (!result) {
			if (!args.def)
				goto err_empty;
			Dee_Incref(args.def);
			result = args.def;
		}
	} else {
		result = DeeCell_CmpXch(Dee_AsObject(self), NULL, args.oldval);
		if (!result)
			return_true;
		Dee_Decref(result);
		result = DeeBool_NewFalse();
	}
	return result;
err_empty:
	err_empty_cell();
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_cmpset(DeeCellObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval, *newval = NULL, *result;
	DeeArg_Unpack1Or2(err, argc, argv, "cmpset", &oldval, &newval);
	result = DeeCell_CmpXch(Dee_AsObject(self), oldval, newval);
	Dee_XDecref(result);
	return_bool(result == oldval);
err:
	return NULL;
}


PRIVATE struct type_method tpconst cell_methods[] = {
	TYPE_METHOD_F(STR_get, &cell_get, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#tValueError{@this ?. is empty}"
	              "Returns the contained value of the ?.\n"
	              "\n"

	              "(def)->\n"
	              "Returns the contained value of the ?. or @def when it is empty"),
	TYPE_METHOD_F("delete", &cell_delete, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Delete the value stored in @this ?., returning ?t if "
	              /**/ "the ?. wasn't empty before, or ?f if it already was"),
	TYPE_METHOD_F(STR_pop, &cell_pop, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#tValueError{The ?. was empty}"

	              "\n"
	              "(def)->\n"
	              "Pop and return the previously contained object, @def, or throw a :ValueError"),
	TYPE_METHOD_F(STR_set, &cell_set, METHOD_FNOREFESCAPE,
	              "(value)->?Dbool\n"
	              "Set (override) @this ?.'s value, returning ?t if a previous value "
	              /**/ "has been overwritten, or ?f if no value had been set before"),
	TYPE_METHOD_F("xch", &cell_xch, METHOD_FNOREFESCAPE,
	              "(value)->\n"
	              "#tValueError{@this ?. is empty}"
	              "Overwrite the ?.'s value and return the old value or throw an error when it was empty\n"

	              "\n"
	              "(value,def)->\n"
	              "Returns the contained value of the ?. or @def when it is empty"),
	TYPE_METHOD_F("cmpdel", &cell_cmpdel, METHOD_FNOREFESCAPE,
	              "(old_value)->?Dbool\n"
	              "Atomically check if the stored object's id matches @{old_value}. If this is "
	              /**/ "the case, delete the stored object and return ?t. Otherwise, return ?f"),
	TYPE_METHOD_F("cmpxch", &cell_cmpxch, METHOD_FNOREFESCAPE,
	              "(old_value,new_value)->\n"
	              "#tValueError{@this ?. is empty}"
	              "\n"

	              "(old_value,new_value,def)->\n"
	              "Do an id-compare of the stored object against @old_value and overwrite "
	              /**/ "that object with @new_value when they match. Otherwise, don't modify the "
	              /**/ "stored object. In both cases, return the previously stored object, @def, "
	              /**/ "or throw a :{ValueError}.\n"
	              "This is equivalent to the atomic execution of the following:\n"
	              "${"
	              /**/ "local result = this.value;\n"
	              /**/ "if (this && result === old_value)\n"
	              /**/ "	this.value = new_value;\n"
	              /**/ "return result;"
	              "}\n"
	              "\n"

	              "(new_value)->?Dbool\n"
	              "Return ?t and atomically set @new_value as stored object only "
	              /**/ "if no object had been set before. Otherwise, return ?f"),
	TYPE_METHOD_F("cmpset", &cell_cmpset, METHOD_FNOREFESCAPE,
	              "(old_value)->?Dbool\n"
	              "(old_value,new_value)->?Dbool\n"
	              "Atomically check if the stored value equals @old_value and return ?t "
	              /**/ "alongside storing @new_value if this is the case. Otherwise, return ?f\n"
	              "When @new_value is omit, the function behaves identical to ?#cmpdel"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD_F("del", &cell_delete, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Deprecated alias for ?#delete"),
	TYPE_METHOD_F("exchange", &cell_xch, METHOD_FNOREFESCAPE,
	              "(value)->\n"
	              "(value,def)->\n"
	              "Deprecated alias for ?#xch"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};

PRIVATE struct type_gc tpconst cell_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&cell_clear
};

PUBLIC DeeTypeObject DeeCell_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Cell),
	/* .tp_doc      = */ DOC("A 1-layer indirection allowing for referral to another object\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(obj)\n"
	                         "Create a new ?. containing @obj"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeeCellObject,
			/* tp_ctor:        */ &cell_ctor,
			/* tp_copy_ctor:   */ &cell_copy,
			/* tp_deep_ctor:   */ &cell_copy,
			/* tp_any_ctor:    */ &cell_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &cell_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cell_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&cell_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&cell_moveassign,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&cell_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cell_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cell_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cell_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cell_visit,
	/* .tp_gc            = */ &cell_gc,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &cell_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cell_methods,
	/* .tp_getsets       = */ cell_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CELL_C */
