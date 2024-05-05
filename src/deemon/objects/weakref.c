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
#ifndef GUARD_DEEMON_OBJECTS_WEAKREF_C
#define GUARD_DEEMON_OBJECTS_WEAKREF_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>
#include <deemon/weakref.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeWeakRefObject WeakRef;
typedef DeeWeakRefAbleObject WeakRefAble;


PRIVATE NONNULL((1)) void DCALL
ob_weakref_fini(WeakRef *__restrict self) {
	Dee_weakref_fini(&self->wr_ref);
	Dee_XDecref(self->wr_del);
}

PRIVATE NONNULL((1, 2)) void DCALL
ob_weakref_visit(WeakRef *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->wr_del);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ob_weakref_ctor(WeakRef *__restrict self) {
	Dee_weakref_null(&self->wr_ref);
	self->wr_del = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ob_weakref_copy(WeakRef *__restrict self,
                WeakRef *__restrict other) {
	self->wr_del = other->wr_del;
	Dee_XIncref(self->wr_del);
	Dee_weakref_copy(&self->wr_ref, &other->wr_ref);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ob_weakref_deep(WeakRef *__restrict self,
                WeakRef *__restrict other) {
	self->wr_del = NULL;
	if (other->wr_del) {
		self->wr_del = DeeObject_DeepCopy(other->wr_del);
		if unlikely(!self->wr_del)
			goto err;
	}
	Dee_weakref_copy(&self->wr_ref, &other->wr_ref);
	return 0;
err:
	return -1;
}

PRIVATE void DCALL
ob_weakref_invoke_callback(struct weakref *__restrict self) {
	DREF WeakRef *me;
	me = COMPILER_CONTAINER_OF(self, WeakRef, wr_ref);
	if (!Dee_IncrefIfNotZero(me)) {
		/* Controller has already died. */
		DeeWeakref_UnlockCallback(self);
	} else {
		DREF DeeObject *result;
		DeeWeakref_UnlockCallback(self);
		/* Invoke the controller deletion callback. */
		ASSERT(me->wr_del);
		result = DeeObject_Call(me->wr_del, 1, (DeeObject **)&me);
		Dee_Decref(me);
		if likely(result) {
			Dee_Decref(result);
		} else {
			DeeError_Print("Unhandled exception in WeakRef callback",
			               ERROR_PRINT_DOHANDLE);
		}
	}
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
ob_weakref_init(WeakRef *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeObject *obj;
	self->wr_del = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:WeakRef", &obj, &self->wr_del))
		goto err;
	if (self->wr_del) {
		Dee_Incref(self->wr_del);
		if (!Dee_weakref_init(&self->wr_ref, obj, &ob_weakref_invoke_callback))
			goto err_nosupport_del;
	} else {
		if (!Dee_weakref_init(&self->wr_ref, obj, NULL))
			goto err_nosupport;
	}
	return 0;
err_nosupport_del:
	Dee_Decref(self->wr_del);
err_nosupport:
	err_cannot_weak_reference(obj);
err:
	return -1;
}

PRIVATE int DCALL
ob_weakref_init_kw(WeakRef *__restrict self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	DeeObject *obj;
	PRIVATE DEFINE_KWLIST(kwlist, { K(obj), K(callback), KEND });
	self->wr_del = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|o:WeakRef", &obj, &self->wr_del))
		goto err;
	if (self->wr_del) {
		Dee_Incref(self->wr_del);
		if (!Dee_weakref_init(&self->wr_ref, obj, &ob_weakref_invoke_callback))
			goto err_nosupport_del;
	} else {
		if (!Dee_weakref_init(&self->wr_ref, obj, NULL))
			goto err_nosupport;
	}
	return 0;
err_nosupport_del:
	Dee_Decref(self->wr_del);
err_nosupport:
	err_cannot_weak_reference(obj);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ob_weakref_assign(WeakRef *__restrict self,
                  DeeObject *__restrict other) {
	if (DeeWeakRef_Check(other)) {
		Dee_weakref_copyassign(&self->wr_ref,
		                       &((WeakRef *)other)->wr_ref);
	} else {
		/* Assign the given other to our weak reference. */
		if (!Dee_weakref_set(&self->wr_ref, other))
			return err_cannot_weak_reference(other);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ob_weakref_moveassign(WeakRef *__restrict self,
                      WeakRef *__restrict other) {
	Dee_weakref_moveassign(&self->wr_ref, &other->wr_ref);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ob_weakref_print(WeakRef *__restrict self,
                 dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *refobj;
	refobj = Dee_weakref_lock(&self->wr_ref);
	if (refobj) {
		result = DeeFormat_Printf(printer, arg, "<weakref to %k>", refobj);
		Dee_Decref_unlikely(refobj);
	} else {
		result = DeeFormat_PRINT(printer, arg, "<empty weakref>");
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ob_weakref_printrepr(WeakRef *__restrict self,
                     dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *refobj;
	refobj = Dee_weakref_lock(&self->wr_ref);
	if (refobj) {
		result = DeeFormat_Printf(printer, arg, "WeakRef(%r)", refobj);
		Dee_Decref_unlikely(refobj);
	} else {
		result = DeeFormat_PRINT(printer, arg, "WeakRef()");
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ob_weakref_bool(WeakRef *__restrict self) {
	return Dee_weakref_bound(&self->wr_ref);
}

#define LAZY_GETOBJ(x) atomic_read(&(x)->wr_ref.wr_obj)

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
ob_weakref_hash(WeakRef *__restrict self) {
	return Dee_HashPointer(LAZY_GETOBJ(self));
}

#define DEFINE_WEAKREF_COMPARE(name, op)                            \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL           \
	name(WeakRef *self, WeakRef *other) {                           \
		if (DeeNone_Check(other))                                   \
			return_bool((void *)LAZY_GETOBJ(self) op(void *) NULL); \
		if (DeeObject_AssertTypeExact(other, &DeeWeakRef_Type))     \
			goto err;                                               \
		return_bool(LAZY_GETOBJ(self) op LAZY_GETOBJ(other));       \
	err:                                                            \
		return NULL;                                                \
	}
DEFINE_WEAKREF_COMPARE(ob_weakref_eq, ==)
DEFINE_WEAKREF_COMPARE(ob_weakref_ne, !=)
DEFINE_WEAKREF_COMPARE(ob_weakref_lo, <)
DEFINE_WEAKREF_COMPARE(ob_weakref_le, <=)
DEFINE_WEAKREF_COMPARE(ob_weakref_gr, >)
DEFINE_WEAKREF_COMPARE(ob_weakref_ge, >=)
#undef DEFINE_WEAKREF_COMPARE

PRIVATE struct type_cmp ob_weakref_cmp = {
	/* .tp_hash       = */ (dhash_t (DCALL *)(DeeObject *__restrict))&ob_weakref_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ NULL,
	/* .tp_eq         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ob_weakref_eq,
	/* .tp_ne         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ob_weakref_ne,
	/* .tp_lo         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ob_weakref_lo,
	/* .tp_le         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ob_weakref_le,
	/* .tp_gr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ob_weakref_gr,
	/* .tp_ge         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ob_weakref_ge
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ob_weakref_get(WeakRef *__restrict self) {
	DREF DeeObject *result;
	result = Dee_weakref_lock(&self->wr_ref);
	if (!result)
		err_cannot_lock_weakref();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ob_weakref_del(WeakRef *__restrict self) {
	Dee_weakref_clear(&self->wr_ref);
	/* Don't throw an error if the reference wasn't bound to prevent
	 * a race condition between someone trying to delete the weakref
	 * and someone else destroying the pointed-to object. */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ob_weakref_set(WeakRef *__restrict self,
               DeeObject *__restrict value) {
	if unlikely(!Dee_weakref_set(&self->wr_ref, value))
		return err_cannot_weak_reference(value);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ob_weakref_lock(WeakRef *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *alt = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:lock", &alt))
		goto err;
	result = Dee_weakref_lock(&self->wr_ref);
	if (!result) {
		if ((result = alt) == NULL) {
			err_cannot_lock_weakref();
		} else {
			Dee_Incref(result);
		}
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ob_weakref_alive(WeakRef *__restrict self) {
	return_bool(Dee_weakref_bound(&self->wr_ref));
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ob_weakref_try_lock(WeakRef *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":try_lock"))
		goto err;
	result = Dee_weakref_lock(&self->wr_ref);
	if (!result) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
	return result;
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_method tpconst ob_weakref_methods[] = {
	TYPE_METHOD_F("lock", &ob_weakref_lock, METHOD_FNOREFESCAPE,
	              "->\n"
	              "(def)->\n"
	              "#tReferenceError{The weak reference is no longer bound and no @def was given}"
	              "Lock the weak reference and return the pointed-to object"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD_F("try_lock", &ob_weakref_try_lock, METHOD_FNOREFESCAPE,
	              "()\n"
	              "->\n"
	              "Deprecated alias for ?#lock with passing ?N (${this.lock(none)})"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst ob_weakref_getsets[] = {
	TYPE_GETSET_BOUND_F("value", &ob_weakref_get, &ob_weakref_del, &ob_weakref_set, &ob_weakref_bool, METHOD_FNOREFESCAPE,
	                    "#tReferenceError{Attempted to get the value after the reference has been unbound}"
	                    "#tValueError{Attempted to set an object that does not support weak referencing}"
	                    "Access to the referenced object"),
	TYPE_GETTER_F("alive", &ob_weakref_alive, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Alias for ?#{op:bool}"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ob_weakref_members[] = {
	TYPE_MEMBER_FIELD_DOC("callback", STRUCT_OBJECT, offsetof(WeakRef, wr_del),
	                      "->?DCallable\n"
	                      "#tUnboundAttribute{No callback has been specified}"
	                      "The second argument passed to the constructor, specifying "
	                      /**/ "an optional callback to-be executed when the bound "
	                      /**/ "object dies on its own\n"
	                      "When invoked, the callback is passed a reference to @this WeakRef object"),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeWeakRef_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_WeakRef),
	/* .tp_doc      = */ DOC("A weak reference to another object implementing WeakRef functionality\n"
	                         "\n"

	                         "()\n"
	                         "Construct an unbound weak reference\n"
	                         "\n"

	                         "(obj,callback?:?DCallable)\n"
	                         "#tTypeError{The given object @obj does not implement weak referencing support}"
	                         "Construct a weak reference bound to @obj, that will be notified once said "
	                         /**/ "object is supposed to be destroyed. With that in mind, weak references don't "
	                         /**/ "actually hold references at all, but rather allow the user to test if an "
	                         /**/ "object is still allocated at runtime.\n"
	                         "If given, @callback is invoked with a single argument passed as "
	                         /**/ "@this WeakRef object once the then bound object dies on its own\n"
	                         "Note however that the bound callback does not influence anything "
	                         /**/ "else, such as comparison operators, which only compare the ids of "
	                         /**/ "bound objects, even after those objects have died\n"
	                         "Also note that at the time that @callback is invoked, @this weak reference "
	                         /**/ "will have already been unbound and no longer point to the object that is "
	                         /**/ "being destroyed, so-as to prevent a possible infinite loop caused by the "
	                         /**/ "object being revived when accessed (@callback is only invoked once the bound "
	                         /**/ "object has passed the point of no return and can no longer be revived)\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns true if the weak reference is currently bound. Note however that this information "
	                         /**/ "is volatile and may not longer be up-to-date by the time the operator returns\n"
	                         "\n"

	                         "==(other:?X2?.?N)->\n"
	                         "!=(other:?X2?.?N)->\n"
	                         "<(other:?X2?.?N)->\n"
	                         "<=(other:?X2?.?N)->\n"
	                         ">(other:?X2?.?N)->\n"
	                         ">=(other:?X2?.?N)->\n"
	                         "When compared with ?N, test for the pointed-to object being bound. "
	                         /**/ "Otherwise, compare the pointed-to object of @this weak reference "
	                         /**/ "to that of @other\n"
	                         "\n"

	                         ":=(other:?X2?.?O)->\n"
	                         "#tTypeError{The given @other does not implement weak referencing support}"
	                         "Assign the value of @other to @this WeakRef object\n"
	                         "\n"

	                         "move:=->\n"
	                         "Override @this weak reference with the value referenced by @other, "
	                         /**/ "while atomically clearing the weak reference from @other\n"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT|TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (dfunptr_t)&ob_weakref_ctor,
				/* .tp_copy_ctor   = */ (dfunptr_t)&ob_weakref_copy,
				/* .tp_deep_ctor   = */ (dfunptr_t)&ob_weakref_deep,
				/* .tp_any_ctor    = */ (dfunptr_t)&ob_weakref_init,
				TYPE_FIXED_ALLOCATOR(WeakRef),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&ob_weakref_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ob_weakref_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&ob_weakref_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&ob_weakref_moveassign,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&ob_weakref_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ob_weakref_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ob_weakref_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ob_weakref_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ob_weakref_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ob_weakref_methods,
	/* .tp_getsets       = */ ob_weakref_getsets,
	/* .tp_members       = */ ob_weakref_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
weakrefable_init(WeakRefAble *__restrict self) {
	weakref_support_init(self);
	return 0;
}

PRIVATE int DCALL
weakrefable_copy(WeakRefAble *__restrict self,
                 WeakRefAble *__restrict UNUSED(other)) {
	weakref_support_init(self);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
weakrefable_fini(WeakRefAble *__restrict self) {
	weakref_support_fini(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
weakrefable_assign(WeakRefAble *__restrict self,
                   WeakRefAble *__restrict other) {
	(void)self;
	return DeeObject_AssertType(other, &DeeWeakRefAble_Type);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
weakrefable_moveassign(WeakRefAble *__restrict self,
                       WeakRefAble *__restrict other) {
	(void)self;
	(void)other;
	return 0;
}


PUBLIC DeeTypeObject DeeWeakRefAble_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_WeakRefAble),
	/* .tp_doc      = */ DOC("An base class that user-defined classes can be "
	                         /**/ "derived from to become weakly referenceable"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(WeakRefAble),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&weakrefable_init,
				/* .tp_copy_ctor = */ (dfunptr_t)&weakrefable_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&weakrefable_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(WeakRefAble)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&weakrefable_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&weakrefable_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&weakrefable_moveassign,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_WEAKREF_C */
