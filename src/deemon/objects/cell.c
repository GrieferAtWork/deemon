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
#ifndef GUARD_DEEMON_OBJECTS_CELL_C
#define GUARD_DEEMON_OBJECTS_CELL_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/cell.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeCellObject Cell;

/* Construct a new Cell object. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_New(DeeObject *__restrict item) {
	DREF Cell *result;
	ASSERT_OBJECT(item);
	result = DeeGCObject_MALLOC(Cell);
	if unlikely(!result)
		goto done;
	/* Initialize and fill in the new Cell. */
	DeeObject_Init(result, &DeeCell_Type);
	Dee_Incref(item);
	result->c_item = item;
	Dee_atomic_rwlock_init(&result->c_lock);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_ctor(Cell *__restrict self) {
	self->c_item = NULL;
	Dee_atomic_rwlock_init(&self->c_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cell_copy(Cell *__restrict self,
          Cell *__restrict other) {
	DeeCell_LockRead(other);
	self->c_item = other->c_item;
	Dee_XIncref(self->c_item);
	DeeCell_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->c_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_init(Cell *__restrict self,
          size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:Cell", &self->c_item))
		goto err;
	Dee_Incref(self->c_item);
	Dee_atomic_rwlock_init(&self->c_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
cell_fini(Cell *__restrict self) {
	Dee_XDecref(self->c_item);
}

PRIVATE NONNULL((1, 2)) void DCALL
cell_visit(Cell *__restrict self, dvisit_t proc, void *arg) {
	DeeCell_LockRead(self);
	Dee_XVisit(self->c_item);
	DeeCell_LockEndRead(self);
}

PRIVATE NONNULL((1)) void DCALL
cell_clear(Cell *__restrict self) {
	DREF DeeObject *old_obj;
	DeeCell_LockWrite(self);
	old_obj      = self->c_item;
	self->c_item = NULL;
	DeeCell_LockEndWrite(self);
	Dee_XDecref(old_obj);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_deepload(Cell *__restrict self) {
	return DeeObject_XInplaceDeepCopyWithLock(&self->c_item,
	                                          &self->c_lock);
}


/* Get/Del/Set the value associated with a given Cell.
 * HINT:  These are the getset callbacks used for `Cell.item' (or its deprecated name `Cell.value').
 *        With that in mind, `DeeCell_Del()' and `DeeCell_Set()'
 *        always return `0' indicative of a successful callback.
 * NOTE: `DeeCell_Get' will return `NULL' and throw an `AttributeError' if the `self' is
 *        empty, whereas `DeeCell_TryGet()' will do the same, but never throw any error. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_TryGet(DeeObject *__restrict self) {
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, &DeeCell_Type);
	DeeCell_LockRead(self);
	result = DeeCell_Item(self);
	Dee_XIncref(result);
	DeeCell_LockEndRead(self);
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
 * NOTE: `DeeCell_XchNonNull()' will only set the new value when the old was non-NULL. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_Xch(DeeObject *self, DeeObject *value) {
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, &DeeCell_Type);
	ASSERT_OBJECT_OPT(value);
	/* Exchange the Cell's value. */
	Dee_XIncref(value);
	DeeCell_LockWrite(self);
	result = DeeCell_Item(self);
	DeeCell_Item(self) = value;
	DeeCell_LockEndWrite(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_XchNonNull(DeeObject *self, DeeObject *value) {
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, &DeeCell_Type);
	ASSERT_OBJECT_OPT(value);
	/* Exchange the Cell's value. */
	Dee_XIncref(value);
	DeeCell_LockWrite(self);
	result = DeeCell_Item(self);
	if unlikely(!result) {
		DeeCell_LockEndWrite(self);
		Dee_DecrefNokill(value);
	} else {
		DeeCell_Item(self) = value;
		DeeCell_LockEndWrite(self);
	}
	return result;
}

/* Perform a compare-exchange, returning the old value of the Cell. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_CmpXch(DeeObject *self,
               DeeObject *old_value,
               DeeObject *new_value) {
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, &DeeCell_Type);
	ASSERT_OBJECT_OPT(old_value);
	ASSERT_OBJECT_OPT(new_value);
	/* Extract the Cell's value. */
	DeeCell_LockWrite(self);
	result = DeeCell_Item(self);
	if (result == old_value) {
		Dee_XIncref(new_value);
		DeeCell_Item(self) = new_value;
	} else {
		Dee_XIncref(result);
	}
	DeeCell_LockEndWrite(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeCell_Del(DeeObject *__restrict self) {
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(self, &DeeCell_Type);
	/* Extract the Cell's value. */
	DeeCell_LockWrite(self);
	old_value          = DeeCell_Item(self);
	DeeCell_Item(self) = NULL;
	DeeCell_LockEndWrite(self);
	/* Decref() the old value. */
	Dee_XDecref(old_value);
	return 0;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeCell_Set(DeeObject *self, DeeObject *value) {
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(self, &DeeCell_Type);
	ASSERT_OBJECT(value);
	Dee_Incref(value);
	/* Exchange the Cell's value. */
	DeeCell_LockWrite(self);
	old_value          = DeeCell_Item(self);
	DeeCell_Item(self) = value;
	DeeCell_LockEndWrite(self);
	/* Decref() the old value. */
	Dee_XDecref(old_value);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cell_print(Cell *__restrict self,
           dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *item;
	item = DeeCell_TryGet((DeeObject *)self);
	if (item) {
		result = DeeFormat_Printf(printer, arg, "<cell with %k>", item);
		Dee_Decref(item);
	} else {
		result = DeeFormat_PRINT(printer, arg, "<empty cell>");
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cell_printrepr(Cell *__restrict self,
               dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *item;
	item = DeeCell_TryGet((DeeObject *)self);
	if (item) {
		result = DeeFormat_Printf(printer, arg, "Cell(%r)", item);
		Dee_Decref(item);
	} else {
		result = DeeFormat_PRINT(printer, arg, "Cell()");
	}
	return result;
}

#define CELL_READITEM(x) atomic_read(&(x)->c_item)

PRIVATE WUNUSED NONNULL((1)) int DCALL
cell_bool(Cell *__restrict self) {
	return CELL_READITEM(self) != NULL;
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
cell_hash(Cell *__restrict self) {
	return DeeObject_HashGeneric(CELL_READITEM(self));
}


#define DEFINE_CELL_COMPARE(name, op)                     \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(Cell *self, Cell *other) {                       \
		if (DeeObject_AssertType(other, &DeeCell_Type))   \
			return NULL;                                  \
		return_bool(DeeObject_Id(CELL_READITEM(self)) op  \
		            DeeObject_Id(CELL_READITEM(other)));  \
	}
DEFINE_CELL_COMPARE(cell_eq, ==)
DEFINE_CELL_COMPARE(cell_ne, !=)
DEFINE_CELL_COMPARE(cell_lo, <)
DEFINE_CELL_COMPARE(cell_le, <=)
DEFINE_CELL_COMPARE(cell_gr, >)
DEFINE_CELL_COMPARE(cell_ge, >=)
#undef DEFINE_CELL_COMPARE

PRIVATE struct type_cmp cell_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&cell_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cell_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cell_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cell_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cell_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cell_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cell_ge
};


PRIVATE struct type_getset tpconst cell_getsets[] = {
	TYPE_GETSET("value", &DeeCell_Get, &DeeCell_Del, &DeeCell_Set,
	            "@throw UnboundAttribute Attempted to read from an empty Cell\n"
	            "Read/write access to the underlying, contained ?O"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_get(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *def = NULL, *result;
	if (DeeArg_Unpack(argc, argv, "|o:get", &def))
		goto err;
	result = DeeCell_TryGet((DeeObject *)self);
	if (!result) {
		result = def;
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
cell_delete(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval;
	if (DeeArg_Unpack(argc, argv, ":delete"))
		goto err;
	oldval = DeeCell_Xch((DeeObject *)self, NULL);
	if (!oldval)
		return_false;
	Dee_Decref(oldval);
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_pop(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:pop", &def))
		goto err;
	oldval = DeeCell_Xch((DeeObject *)self, NULL);
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
cell_set(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *newval;
	if (DeeArg_Unpack(argc, argv, "o:set", &newval))
		goto err;
	newval = DeeCell_Xch((DeeObject *)self, newval);
	if (!newval)
		return_false;
	Dee_Decref(newval);
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_xch(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *value, *def = NULL, *result;
	if (DeeArg_Unpack(argc, argv, "o|o:xch", &value, &def))
		goto err;
	if (def) {
		result = DeeCell_Xch((DeeObject *)self, value);
		if (!result) {
			Dee_Incref(def);
			result = def;
		}
	} else {
		result = DeeCell_XchNonNull((DeeObject *)self, value);
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
cell_cmpdel(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval, *result;
	if (DeeArg_Unpack(argc, argv, "o:cmpdel", &oldval))
		goto err;
	result = DeeCell_CmpXch((DeeObject *)self, oldval, NULL);
	Dee_Decref(result);
	return_bool_(result == oldval);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_cmpxch(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval, *newval = NULL, *def = NULL, *result;
	if (DeeArg_Unpack(argc, argv, "o|oo:cmpxch", &oldval, &newval, &def))
		goto err;
	if (newval) {
		result = DeeCell_CmpXch((DeeObject *)self, oldval, newval);
		if (!result) {
			if (!def)
				goto err_empty;
			Dee_Incref(def);
			result = def;
		}
	} else {
		result = DeeCell_CmpXch((DeeObject *)self, NULL, oldval);
		if (!result)
			return_true;
		Dee_Incref(Dee_False);
		Dee_Decref(result);
		result = Dee_False;
	}
	return result;
err_empty:
	err_empty_cell();
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
cell_cmpset(Cell *self, size_t argc, DeeObject *const *argv) {
	DeeObject *oldval, *newval = NULL, *result;
	if (DeeArg_Unpack(argc, argv, "o|o:cmpset", &oldval, &newval))
		goto err;
	result = DeeCell_CmpXch((DeeObject *)self, oldval, newval);
	Dee_XDecref(result);
	return_bool_(result == oldval);
err:
	return NULL;
}


PRIVATE struct type_method tpconst cell_methods[] = {
	TYPE_METHOD(STR_get, &cell_get,
	            "->\n"
	            "@throw ValueError @this Cell is empty\n"
	            "Returns the contained value of the Cell\n"
	            "\n"

	            "(def)->\n"
	            "Returns the contained value of the Cell or @def when it is empty"),
	TYPE_METHOD("delete", &cell_delete,
	            "->?Dbool\n"
	            "Delete the value stored in @this Cell, returning ?t if "
	            /**/ "the Cell wasn't empty before, or ?f if it already was"),
	TYPE_METHOD(STR_pop, &cell_pop,
	            "->\n"
	            "@throw ValueError The Cell was empty\n"

	            "\n"
	            "(def)->\n"
	            "Pop and return the previously contained object, @def, or throw a :ValueError"),
	TYPE_METHOD(STR_set, &cell_set,
	            "(value)->?Dbool\n"
	            "Set (override) @this Cell's value, returning ?t if a previous value "
	            /**/ "has been overwritten, or ?f if no value had been set before"),
	TYPE_METHOD(STR_xch, &cell_xch,
	            "(value)->\n"
	            "@throw ValueError @this Cell is empty\n"
	            "Overwrite the Cell's value and return the old value or throw an error when it was empty\n"

	            "\n"
	            "(value,def)->\n"
	            "Returns the contained value of the Cell or @def when it is empty"),
	TYPE_METHOD("cmpdel", &cell_cmpdel,
	            "(old_value)->?Dbool\n"
	            "Atomically check if the stored object's id matches @{old_value}. If this is "
	            /**/ "the case, delete the stored object and return ?t. Otherwise, return ?f"),
	TYPE_METHOD("cmpxch", &cell_cmpxch,
	            "(old_value,new_value)->\n"
	            "@throw ValueError @this Cell is empty\n"
	            "\n"

	            "(old_value,new_value,def)->\n"
	            "Do an id-compare of the stored object against @old_value and overwrite "
	            /**/ "that object with @new_value when they match. Otherwise, don't modify the "
	            /**/ "stored object. In both cases, return the previously stored object, @def, or throw a :{ValueError}.\n"
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
	TYPE_METHOD("cmpset", &cell_cmpset,
	            "(old_value)->?Dbool\n"
	            "(old_value,new_value)->?Dbool\n"
	            "Atomically check if the stored value equals @old_value and return ?t "
	            /**/ "alongside storing @new_value if this is the case. Otherwise, return ?f\n"
	            "When @new_value is omit, the function behaves identical to ?#cmpdel"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("del", &cell_delete,
	            "->?Dbool\n"
	            "Deprecated alias for ?#delete"),
	TYPE_METHOD("exchange", &cell_xch,
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
	                         "Create a new, empty Cell\n"
	                         "\n"

	                         "(obj)\n"
	                         "Create a new Cell containing @obj"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&cell_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&cell_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&cell_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&cell_init,
				TYPE_FIXED_ALLOCATOR_GC(Cell)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cell_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&cell_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cell_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cell_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cell_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cell_visit,
	/* .tp_gc            = */ &cell_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &cell_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cell_methods,
	/* .tp_getsets       = */ cell_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CELL_C */
