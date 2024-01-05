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
#ifndef GUARD_DEEMON_OBJECTS_OBJMETHOD_C
#define GUARD_DEEMON_OBJECTS_OBJMETHOD_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* _Exit(), abort() */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

/* Construct a new `_ObjMethod' object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObjMethod_New(dobjmethod_t func, DeeObject *__restrict self) {
	DREF DeeObjMethodObject *result;
	ASSERT_OBJECT(self);
	result = DeeObject_MALLOC(DeeObjMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeObjMethod_Type);
	result->om_func = func;
	result->om_this = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwObjMethod_New(dkwobjmethod_t func, DeeObject *__restrict self) {
	DREF DeeKwObjMethodObject *result;
	ASSERT_OBJECT(self);
	result = DeeObject_MALLOC(DeeKwObjMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeKwObjMethod_Type);
	result->om_func = func;
	result->om_this = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}


PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) char const *DCALL
typeobject_find_objmethod(DeeTypeObject *__restrict self,
                          dobjmethod_t meth) {
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, self);
	do {
		struct type_method const *iter;
		iter = self->tp_class_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == meth)
					return iter->m_name;
			}
	} while ((self = DeeTypeMRO_Next(&mro, self)) != NULL);
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) char const *DCALL
typeobject_find_objmethod_doc(DeeTypeObject *__restrict self,
                              dobjmethod_t meth) {
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, self);
	do {
		struct type_method const *iter;
		iter = self->tp_class_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == meth)
					return iter->m_doc;
			}
	} while ((self = DeeTypeMRO_Next(&mro, self)) != NULL);
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) DeeTypeObject *DCALL
typeobject_find_objmethod_type(DeeTypeObject *__restrict self,
                               dobjmethod_t meth) {
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, self);
	do {
		struct type_method const *iter;
		iter = self->tp_class_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == meth)
					return self;
			}
	} while ((self = DeeTypeMRO_Next(&mro, self)) != NULL);
	return NULL;
}

/* Returns the name of the function bound by the given
 * `_ObjMethod', or `NULL' if the name could not be determined. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeObjMethod_GetName(DeeObject const *__restrict self) {
	dobjmethod_t func;
	DeeTypeObject *tp_self;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	ASSERT(DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self));
	func    = DeeObjMethod_FUNC(self);
	tp_self = Dee_TYPE(DeeObjMethod_SELF(self));
	DeeTypeMRO_Init(&mro, tp_self);
	do {
		struct type_method const *iter;
		if (tp_self == &DeeType_Type) {
			char const *result;
			result = typeobject_find_objmethod((DeeTypeObject *)DeeObjMethod_SELF(self), func);
			if (result)
				return result;
		}
		iter = tp_self->tp_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == func)
					return iter->m_name;
			}
	} while ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) != NULL);
	return NULL;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeObjMethod_GetDoc(DeeObject const *__restrict self) {
	dobjmethod_t func;
	DeeTypeObject *tp_self;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	ASSERT(DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self));
	func    = DeeObjMethod_FUNC(self);
	tp_self = Dee_TYPE(DeeObjMethod_SELF(self));
	DeeTypeMRO_Init(&mro, tp_self);
	do {
		struct type_method const *iter;
		if (tp_self == &DeeType_Type) {
			char const *result;
			result = typeobject_find_objmethod_doc((DeeTypeObject *)DeeObjMethod_SELF(self), func);
			if (result)
				return result;
		}
		iter = tp_self->tp_methods;
		if (iter) {
			for (; iter->m_name; ++iter) {
				if (iter->m_func == func)
					return iter->m_doc;
			}
		}
	} while ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) != NULL);
	return NULL;
}

/* Returns the type that is implementing the bound method. */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeObjMethod_GetType(DeeObject const *__restrict self) {
	dobjmethod_t func;
	DeeTypeObject *tp_self;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	ASSERT(DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self));
	func    = DeeObjMethod_FUNC(self);
	tp_self = Dee_TYPE(DeeObjMethod_SELF(self));
	DeeTypeMRO_Init(&mro, tp_self);
	do {
		struct type_method const *iter;
		if (tp_self == &DeeType_Type) {
			DeeTypeObject *result;
			result = typeobject_find_objmethod_type((DeeTypeObject *)DeeObjMethod_SELF(self), func);
			if (result)
				return result;
		}
		iter = tp_self->tp_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == func)
					return tp_self;
			}
	} while ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) != NULL);
	return Dee_TYPE(DeeObjMethod_SELF(self));
}

/* Returns the name of the function bound by the given
 * `_ClsMethod', or `NULL' if the name could not be determined. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeClsMethod_GetName(DeeObject const *__restrict self) {
	dobjmethod_t func;
	struct type_method const *iter;
	ASSERT_OBJECT(self);
	ASSERT(DeeClsMethod_Check(self) || DeeKwClsMethod_Check(self));
	func = DeeClsMethod_FUNC(self);
	iter = DeeClsMethod_TYPE(self)->tp_methods;
	if (iter) {
		for (; iter->m_name; ++iter) {
			if (iter->m_func == func)
				return iter->m_name;
		}
	}
	return NULL;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeClsMethod_GetDoc(DeeObject const *__restrict self) {
	dobjmethod_t func;
	struct type_method const *iter;
	ASSERT_OBJECT(self);
	ASSERT(DeeClsMethod_Check(self) || DeeKwClsMethod_Check(self));
	func = DeeClsMethod_FUNC(self);
	iter = DeeClsMethod_TYPE(self)->tp_methods;
	if (iter) {
		for (; iter->m_name; ++iter) {
			if (iter->m_func == func)
				return iter->m_doc;
		}
	}
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
objmethod_fini(DeeObjMethodObject *__restrict self) {
	Dee_Decref(self->om_this);
}

PRIVATE NONNULL((1, 2)) void DCALL
objmethod_visit(DeeObjMethodObject *__restrict self,
                dvisit_t proc, void *arg) {
	Dee_Visit(((DeeObjMethodObject *)self)->om_this);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_call(DeeObjMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeObjMethod_CallFunc(self->om_func, self->om_this, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
objmethod_hash(DeeObjMethodObject *__restrict self) {
	return DeeObject_Hash(self->om_this) ^
	       Dee_HashPointer(self->om_func);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
objmethod_eq(DeeObjMethodObject *self,
             DeeObjMethodObject *other) {
	if (DeeObject_AssertType(other, &DeeObjMethod_Type))
		goto err;
	if (self->om_func != other->om_func)
		return_false;
	return DeeObject_CompareEqObject(self->om_this, other->om_this);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
objmethod_ne(DeeObjMethodObject *self,
             DeeObjMethodObject *other) {
	if (DeeObject_AssertType(other, &DeeObjMethod_Type))
		goto err;
	if (self->om_func != other->om_func)
		return_true;
	return DeeObject_CompareNeObject(self->om_this, other->om_this);
err:
	return NULL;
}

PRIVATE struct type_cmp objmethod_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&objmethod_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&objmethod_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&objmethod_ne
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_func(DeeObjMethodObject *__restrict self) {
	DeeTypeObject *typ;
	typ = DeeObjMethod_GetType((DeeObject *)self);
	return DeeClsMethod_New(typ, self->om_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_name(DeeObjMethodObject *__restrict self) {
	char const *name;
	name = DeeObjMethod_GetName((DeeObject *)self);
	if unlikely(!name)
		return_none;
	return DeeString_NewAuto(name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_doc(DeeObjMethodObject *__restrict self) {
	char const *doc;
	doc = DeeObjMethod_GetDoc((DeeObject *)self);
	if unlikely(!doc)
		return_none;
	return DeeString_NewAutoUtf8(doc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
objmethod_get_type(DeeObjMethodObject *__restrict self) {
	DeeTypeObject *result;
	result = DeeObjMethod_GetType((DeeObject *)self);
	Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_module(DeeObjMethodObject *__restrict self) {
	DeeTypeObject *type;
	DREF DeeObject *result;
	type   = DeeObjMethod_GetType((DeeObject *)self);
	result = DeeType_GetModule(type);
	if (!result)
		return_none;
	return result;
}

DOC_DEF(objmethod_get_func_doc,
        "->?DCallable\n"
        "The unbound class-function that is being bound by this object-method");
DOC_DEF(objmethod_get_name_doc,
        "->?X2?Dstring?N\n"
        "The name of the function, or ?N if unknown");
DOC_DEF(objmethod_get_doc_doc,
        "->?X2?Dstring?N\n"
        "The documentation string of the function being bound, or ?N if unknown");
DOC_DEF(objmethod_get_kwds_doc,
        "->?S?Dstring\n"
        "Returns a sequence of keyword argument names accepted by @this function\n"
        "If @this function doesn't accept keyword arguments, an empty sequence is returned");
DOC_DEF(objmethod_get_type_doc,
        "->?DType\n"
        "The type implementing the function that is being bound");
DOC_DEF(objmethod_get_module_doc,
        "->?X2?DModule?N\n"
        "The module implementing the function that is being bound, or ?N if unknown");

PRIVATE struct type_getset tpconst objmethod_getsets[] = {
	TYPE_GETTER_F("__func__", &objmethod_get_func, TYPE_GETSET_FNOREFESCAPE, DOC_GET(objmethod_get_func_doc)),
	TYPE_GETTER_F(STR___name__, &objmethod_get_name, TYPE_GETSET_FNOREFESCAPE, DOC_GET(objmethod_get_name_doc)),
	TYPE_GETTER_F(STR___doc__, &objmethod_get_doc, TYPE_GETSET_FNOREFESCAPE, DOC_GET(objmethod_get_doc_doc)),
	TYPE_GETTER_F(STR___type__, &objmethod_get_type, TYPE_GETSET_FNOREFESCAPE, DOC_GET(objmethod_get_type_doc)),
	TYPE_GETTER_F(STR___module__, &objmethod_get_module, TYPE_GETSET_FNOREFESCAPE, DOC_GET(objmethod_get_module_doc)),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst objmethod_members[] = {
	TYPE_MEMBER_FIELD_DOC("__this__", STRUCT_OBJECT, offsetof(DeeObjMethodObject, om_this),
	                      "The object to which @this object-method is bound"),
#define OBJMETHOD_MEMBERS_INDEXOF_KWDS 1
	TYPE_MEMBER_CONST_DOC(STR___kwds__, Dee_EmptySeq, objmethod_get_kwds_doc), /* NOTE: _MUST_ always come last! */
	TYPE_MEMBER_END
};

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
objmethod_getname(DeeObjMethodObject *__restrict self) {
	char const *result = DeeObjMethod_GetName((DeeObject *)self);
	if unlikely(result == NULL)
		result = "<unknown>";
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
objmethod_print(DeeObjMethodObject *__restrict self,
                dformatprinter printer, void *arg) {
	char const *name    = objmethod_getname(self);
	DeeTypeObject *type = DeeObjMethod_GetType((DeeObject *)self);
	if (!type) {
		return DeeFormat_Printf(printer, arg,
		                        "<object method <unknown>.%s, bound to %r>",
		                        name, self->om_this);
	} else {
		return DeeFormat_Printf(printer, arg,
		                        "<object method %k.%s, bound to %r>",
		                        type, name, self->om_this);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
objmethod_printrepr(DeeObjMethodObject *__restrict self,
                    dformatprinter printer, void *arg) {
	char const *name = objmethod_getname(self);
	return DeeFormat_Printf(printer, arg, "%r.%s", self->om_this, name);
}


PUBLIC DeeTypeObject DeeObjMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ObjMethod",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObjMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&objmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&objmethod_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&objmethod_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&objmethod_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&objmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &objmethod_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ objmethod_getsets,
	/* .tp_members       = */ objmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


typedef struct {
	OBJECT_HEAD
	DREF DeeObject *dk_owner; /* [1..1][const] The owner of `dk_start'. */
	char const     *dk_start; /* [1..1][const] Doc string. */
} DocKwds;

typedef struct {
	OBJECT_HEAD
	DREF DocKwds     *dki_kwds; /* [1..1][const] The associated sequence. */
	DWEAK char const *dki_iter; /* [1..1] Iterator position (start of next keyword name). */
} DocKwdsIterator;
#define DOCKWDSITER_RDITER(self) atomic_read(&(self)->dki_iter)

INTDEF DeeTypeObject DocKwds_Type;
INTDEF DeeTypeObject DocKwdsIterator_Type;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dockwdsiter_next(DocKwdsIterator *__restrict self) {
	char const *pos, *newpos;
	char const *name_end;
	bool is_escaped;
	for (;;) {
		pos        = atomic_read(&self->dki_iter);
		newpos     = pos;
		is_escaped = false;
		for (;; ++newpos) {
			char ch = *newpos;
			if (!ch)
				break;
			if (ch == '\\') {
				++newpos;
				if (!*newpos)
					break;
				is_escaped = true;
				continue;
			}
			if (ch == ',' || ch == ')' ||
			    ch == ':' || ch == '!' ||
			    ch == '?')
				break;
		}
		name_end = newpos;
		if (*newpos == '?') {
			++newpos;
		} else if (*newpos == '!') {
			++newpos;
			if (*newpos == '!')
				++newpos;
		}
		if (*newpos == ':') {
			++newpos;
			for (;; ++newpos) {
				char ch = *newpos;
				if (ch == '\\') {
					++newpos;
					if (!*newpos)
						break;
					continue;
				}
				if (ch == ',') {
					++newpos;
					break;
				}
				if (ch == ')')
					break;
			}
		}
		if (atomic_cmpxch_weak_or_write(&self->dki_iter, pos, newpos))
			break;
	}
	if (pos >= name_end)
		return ITER_DONE;
	if (is_escaped) {
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		char const *iter, *flush_start;
		iter = flush_start = pos;
		for (; iter < name_end; ++iter) {
			if (*iter != '\\')
				continue;
			if (unicode_printer_print(&printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err_printer;
			flush_start = ++iter;
			ASSERT(iter <= name_end);
			if (iter >= name_end)
				break;
		}
		if (unicode_printer_print(&printer, flush_start, (size_t)(name_end - flush_start)) < 0)
			goto err_printer;
		return unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
		return NULL;
	}
	return DeeString_NewUtf8(pos,
	                         (size_t)(name_end - pos),
	                         STRING_ERROR_FIGNORE);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dockwdsiter_copy(DocKwdsIterator *__restrict self,
                 DocKwdsIterator *__restrict other) {
	self->dki_kwds = other->dki_kwds;
	self->dki_iter = DOCKWDSITER_RDITER(other);
	Dee_Incref(self->dki_kwds);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
dockwdsiter_fini(DocKwdsIterator *__restrict self) {
	Dee_Decref(self->dki_kwds);
}

PRIVATE NONNULL((1, 2)) void DCALL
dockwdsiter_visit(DocKwdsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->dki_kwds);
}

#define DEFINE_DOCKWDSITERATOR_COMPARE(name, op)                            \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                   \
	name(DocKwdsIterator *self, DocKwdsIterator *other) {                   \
		if (DeeObject_AssertTypeExact(other, &DocKwdsIterator_Type))        \
			goto err;                                                       \
		return_bool(DOCKWDSITER_RDITER(self) op DOCKWDSITER_RDITER(other)); \
	err:                                                                    \
		return NULL;                                                        \
	}
DEFINE_DOCKWDSITERATOR_COMPARE(dockwdsiter_eq, ==)
DEFINE_DOCKWDSITERATOR_COMPARE(dockwdsiter_ne, !=)
DEFINE_DOCKWDSITERATOR_COMPARE(dockwdsiter_lo, <)
DEFINE_DOCKWDSITERATOR_COMPARE(dockwdsiter_le, <=)
DEFINE_DOCKWDSITERATOR_COMPARE(dockwdsiter_gr, >)
DEFINE_DOCKWDSITERATOR_COMPARE(dockwdsiter_ge, >=)
#undef DEFINE_DOCKWDSITERATOR_COMPARE

PRIVATE struct type_cmp dockwdsiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dockwdsiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dockwdsiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dockwdsiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dockwdsiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dockwdsiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dockwdsiter_ge,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dockwdsiter_getdocstr(DocKwdsIterator *__restrict self) {
	return DeeString_New(self->dki_kwds->dk_start);
}

PRIVATE struct type_getset tpconst dockwdsiter_getsets[] = {
	TYPE_GETTER_F("__doc__", &dockwdsiter_getdocstr, TYPE_GETSET_FNOREFESCAPE, "->?Dstring"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst dockwdsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DocKwdsIterator, dki_kwds), "->?Ert:DocKwds"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DocKwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DocKwdsIterator",
	/* .tp_doc      = */ DOC("next->?Dstring"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&dockwdsiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DocKwdsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dockwdsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dockwdsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &dockwdsiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dockwdsiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ dockwdsiter_getsets,
	/* .tp_members       = */ dockwdsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


STATIC_ASSERT(offsetof(DocKwds, dk_owner) == offsetof(DocKwdsIterator, dki_kwds));
#define dockwds_fini  dockwdsiter_fini
#define dockwds_visit dockwdsiter_visit

PRIVATE WUNUSED NONNULL((1)) DREF DocKwdsIterator *DCALL
dockwds_iter(DocKwds *__restrict self) {
	DREF DocKwdsIterator *result;
	result = DeeObject_MALLOC(DocKwdsIterator);
	if unlikely(!result)
		goto done;
	ASSERT(self->dk_start);
	ASSERT(self->dk_start[0] == '(');
	Dee_Incref(self);
	result->dki_kwds = self;
	result->dki_iter = self->dk_start + 1;
	DeeObject_Init(result, &DocKwdsIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq dockwds_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dockwds_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
};

PRIVATE struct type_member tpconst dockwds_members[] = {
	TYPE_MEMBER_FIELD("__docstr__", STRUCT_CONST | STRUCT_CSTR, offsetof(DocKwds, dk_start)),
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT, offsetof(DocKwds, dk_owner)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst dockwds_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DocKwdsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
dockwds_ctor(DocKwds *__restrict self) {
	self->dk_owner = DeeString_New("()");
	if unlikely(!self->dk_owner)
		goto err;
	self->dk_start = DeeString_STR(self->dk_owner);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dockwds_copy(DocKwds *__restrict self,
             DocKwds *__restrict other) {
	self->dk_owner = other->dk_owner;
	self->dk_start = other->dk_start;
	Dee_Incref(self->dk_owner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dockwds_init(DocKwds *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeObject *text;
	if (DeeArg_Unpack(argc, argv, "o:_DocKwds", &text))
		goto err;
	if (DeeObject_AssertTypeExact(text, &DeeString_Type))
		goto err;
	if (DeeString_STR(text)[0] != '(') {
		DeeError_Throwf(&DeeError_ValueError,
		                "The given string %r does not start with `('");
		goto err;
	}
	Dee_Incref(text);
	self->dk_owner = text;
	self->dk_start = DeeString_STR(text);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
dockwds_print(DocKwds *__restrict self,
              dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<keywords for %k>", self->dk_owner);
}


INTERN DeeTypeObject DocKwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DocKwds",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)dockwds_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)dockwds_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)dockwds_init,
				TYPE_FIXED_ALLOCATOR(DocKwds)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dockwds_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&dockwds_print,
		/* .tp_printrepr = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dockwds_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dockwds_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dockwds_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dockwds_class_members
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
doc_decode_kwds(DeeObject *owner, char const *doc) {
	DREF DocKwds *result;
	if (!doc)
		goto no_kwds;
	if (doc[0] != '(')
		goto no_kwds;
	result = DeeObject_MALLOC(DocKwds);
	if unlikely(!result)
		goto done;
	Dee_Incref(owner);
	result->dk_owner = owner;
	result->dk_start = doc;
	DeeObject_Init(result, &DocKwds_Type);
done:
	return (DREF DeeObject *)result;
no_kwds:
	return_empty_seq;
}




STATIC_ASSERT(offsetof(DeeObjMethodObject, om_this) ==
              offsetof(DeeKwObjMethodObject, om_this));
STATIC_ASSERT(offsetof(DeeObjMethodObject, om_func) ==
              offsetof(DeeKwObjMethodObject, om_func));
#define kwobjmethod_fini  objmethod_fini
#define kwobjmethod_visit objmethod_visit
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_call(DeeKwObjMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeKwObjMethod_CallFunc(self->om_func, self->om_this, argc, argv, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_call_kw(DeeKwObjMethodObject *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	return DeeKwObjMethod_CallFunc(self->om_func, self->om_this, argc, argv, kw);
}
#define kwobjmethod_cmp objmethod_cmp

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_get_func(DeeKwObjMethodObject *__restrict self) {
	DeeTypeObject *type;
	type = DeeObjMethod_GetType((DeeObject *)self);
	if (!type)
		type = Dee_TYPE(self->om_this);
	return DeeKwClsMethod_New(type, self->om_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_get_kwds(DeeKwObjMethodObject *__restrict self) {
	return doc_decode_kwds((DeeObject *)self,
	                       DeeObjMethod_GetDoc((DeeObject *)self));
}

#define kwobjmethod_get_name       objmethod_get_name
#define kwobjmethod_get_doc        objmethod_get_doc
#define kwobjmethod_get_type       objmethod_get_type
#define kwobjmethod_get_module     objmethod_get_module
#define kwobjmethod_get_func_doc   objmethod_get_func_doc
#define kwobjmethod_get_name_doc   objmethod_get_name_doc
#define kwobjmethod_get_doc_doc    objmethod_get_doc_doc
#define kwobjmethod_get_kwds_doc   objmethod_get_kwds_doc
#define kwobjmethod_get_type_doc   objmethod_get_type_doc
#define kwobjmethod_get_module_doc objmethod_get_module_doc

PRIVATE struct type_getset tpconst kwobjmethod_getsets[] = {
	TYPE_GETTER_F("__func__", &kwobjmethod_get_func, TYPE_GETSET_FNOREFESCAPE, DOC_GET(kwobjmethod_get_func_doc)),
	TYPE_GETTER_F(STR___name__, &kwobjmethod_get_name, TYPE_GETSET_FNOREFESCAPE, DOC_GET(kwobjmethod_get_name_doc)),
	TYPE_GETTER_F(STR___doc__, &kwobjmethod_get_doc, TYPE_GETSET_FNOREFESCAPE, DOC_GET(kwobjmethod_get_doc_doc)),
	TYPE_GETTER_F(STR___kwds__, &kwobjmethod_get_kwds, TYPE_GETSET_FNOREFESCAPE, DOC_GET(kwobjmethod_get_kwds_doc)),
	TYPE_GETTER_F(STR___type__, &kwobjmethod_get_type, TYPE_GETSET_FNOREFESCAPE, DOC_GET(kwobjmethod_get_type_doc)),
	TYPE_GETTER_F(STR___module__, &kwobjmethod_get_module, TYPE_GETSET_FNOREFESCAPE, DOC_GET(kwobjmethod_get_module_doc)),
	TYPE_GETSET_END
};
#define kwobjmethod_members   objmethod_members
#define kwobjmethod_print     objmethod_print
#define kwobjmethod_printrepr objmethod_printrepr

PUBLIC DeeTypeObject DeeKwObjMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwObjMethod",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwObjMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwobjmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&kwobjmethod_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&kwobjmethod_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&kwobjmethod_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kwobjmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &kwobjmethod_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ kwobjmethod_getsets,
	/* .tp_members       = */ kwobjmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&kwobjmethod_call_kw,
};


/* Construct a new `_ClassMethod' object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*ClsMethod*/ DeeObject *DCALL
DeeClsMethod_New(DeeTypeObject *__restrict type,
                 dobjmethod_t func) {
	DeeClsMethodObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeClsMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeClsMethod_Type);
	result->cm_type = type;
	result->cm_func = func;
	Dee_Incref(type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF /*KwClsMethod*/ DeeObject *DCALL
DeeKwClsMethod_New(DeeTypeObject *__restrict type,
                   dkwobjmethod_t func) {
	DeeKwClsMethodObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeKwClsMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeKwClsMethod_Type);
	result->cm_type = type;
	result->cm_func = func;
	Dee_Incref(type);
done:
	return (DREF DeeObject *)result;
}


PRIVATE ATTR_COLD int DCALL err_classmethod_argc_zero(void) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "ClassMethod objects must be called "
	                       /**/ "with at least 1 argument");
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_call(DeeClsMethodObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_argc_zero;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cm_type)) {
		if (DeeObject_AssertType(argv[0], self->cm_type))
			goto err;
	}
	/* Use the first argument as the this-argument. */
	return (*self->cm_func)(argv[0], argc - 1, argv + 1);
err_argc_zero:
	err_classmethod_argc_zero();
err:
	return NULL;
}


#if 1
STATIC_ASSERT(offsetof(DeeClsMethodObject, cm_type) ==
              offsetof(DeeObjMethodObject, om_this));
STATIC_ASSERT(offsetof(DeeClsMethodObject, cm_func) ==
              offsetof(DeeObjMethodObject, om_func));
#define clsmethod_fini  objmethod_fini
#define clsmethod_visit objmethod_visit
#else
PRIVATE NONNULL((1)) void DCALL
clsmethod_fini(DeeClsMethodObject *__restrict self) {
	Dee_Decref(self->cm_type);
}

PRIVATE NONNULL((1, 2)) void DCALL
clsmethod_visit(DeeClsMethodObject *__restrict self,
                dvisit_t proc, void *arg) {
	Dee_Visit(self->cm_type);
}
#endif

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
clsmethod_getname(DeeClsMethodObject *__restrict self) {
	char const *result = DeeClsMethod_GetName((DeeObject *)self);
	if unlikely(result == NULL)
		result = "<unknown>";
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
clsmethod_print(DeeClsMethodObject *__restrict self,
                dformatprinter printer, void *arg) {
	char const *name = clsmethod_getname(self);
	return DeeFormat_Printf(printer, arg,
	                        "<class method %k.%s>",
	                        self->cm_type, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
clsmethod_printrepr(DeeClsMethodObject *__restrict self,
                    dformatprinter printer, void *arg) {
	char const *name = clsmethod_getname(self);
	return DeeFormat_Printf(printer, arg, "%r.%s",
	                        self->cm_type, name);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
clsmethod_hash(DeeClsMethodObject *__restrict self) {
	return Dee_HashPointer(self->cm_func);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_eq(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func == other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_ne(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func != other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_lo(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func < other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_le(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func <= other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_gr(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func > other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_ge(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func >= other->cm_func);
err:
	return NULL;
}

PRIVATE struct type_cmp clsmethod_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&clsmethod_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmethod_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmethod_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmethod_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmethod_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmethod_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmethod_ge
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_get_name(DeeClsMethodObject *__restrict self) {
	char const *name;
	name = DeeClsMethod_GetName((DeeObject *)self);
	if unlikely(!name)
		return_none;
	return DeeString_NewAuto(name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_get_doc(DeeClsMethodObject *__restrict self) {
	char const *doc;
	doc = DeeClsMethod_GetDoc((DeeObject *)self);
	if (!doc)
		return_none;
	return DeeString_NewAutoUtf8(doc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwclsmethod_get_kwds(DeeClsMethodObject *__restrict self) {
	return doc_decode_kwds((DeeObject *)self,
	                       DeeClsMethod_GetDoc((DeeObject *)self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_get_module(DeeClsMethodObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cm_type);
	if (!result)
		return_none;
	return result;
}

#define clsmethod_getsets (kwclsmethod_getsets + 1)
PRIVATE struct type_getset tpconst kwclsmethod_getsets[] = {
	TYPE_GETTER_F(STR___kwds__, &kwclsmethod_get_kwds, TYPE_GETSET_FNOREFESCAPE,
	              DOC_GET(objmethod_get_kwds_doc)),
	TYPE_GETTER_F(STR___name__, &clsmethod_get_name, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "The name of @this method, or ?N if unknown"),
	TYPE_GETTER_F(STR___doc__, &clsmethod_get_doc, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "The documentation string of @this method, or ?N if unknown"),
	TYPE_GETTER_F(STR___module__, &clsmethod_get_module, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?DModule?N\n"
	              "The module implementing @this method, or ?N if unknown"),
	TYPE_GETSET_END
};
#define clsmethod_get_kwds_doc objmethod_get_kwds_doc


PRIVATE struct type_member tpconst kwclsmethod_members[] = {
	TYPE_MEMBER_CONST_DOC(STR___kwds__, Dee_EmptySeq, clsmethod_get_kwds_doc),
#define clsmethod_members (kwclsmethod_members + 1)
	TYPE_MEMBER_FIELD_DOC(STR___type__, STRUCT_OBJECT, offsetof(DeeClsMethodObject, cm_type),
	                      "->?DType\n"
	                      "The type implementing @this method"),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeClsMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassMethod",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&clsmethod_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&clsmethod_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsmethod_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&clsmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &clsmethod_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ clsmethod_getsets,
	/* .tp_members       = */ clsmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


STATIC_ASSERT(offsetof(DeeClsMethodObject, cm_func) ==
              offsetof(DeeKwClsMethodObject, cm_func));
STATIC_ASSERT(offsetof(DeeClsMethodObject, cm_type) ==
              offsetof(DeeKwClsMethodObject, cm_type));

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwclsmethod_call(DeeKwClsMethodObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_argc_zero;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cm_type)) {
		if (DeeObject_AssertType(argv[0], self->cm_type))
			goto err;
	}
	/* Use the first argument as the this-argument. */
	return (*self->cm_func)(argv[0], argc - 1, argv + 1, NULL);
err_argc_zero:
	err_classmethod_argc_zero();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwclsmethod_call_kw(DeeKwClsMethodObject *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	if unlikely(!argc)
		goto err_argc_zero;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cm_type)) {
		if (DeeObject_AssertType(argv[0], self->cm_type))
			goto err;
	}
	/* Use the first argument as the this-argument. */
	return (*self->cm_func)(argv[0], argc - 1, argv + 1, kw);
err_argc_zero:
	err_classmethod_argc_zero();
err:
	return NULL;
}

#define kwclsmethod_fini      clsmethod_fini
#define kwclsmethod_print     clsmethod_print
#define kwclsmethod_printrepr clsmethod_printrepr
#define kwclsmethod_visit     clsmethod_visit
#define kwclsmethod_hash      clsmethod_hash
#define kwclsmethod_cmp       clsmethod_cmp

PUBLIC DeeTypeObject DeeKwClsMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwClassMethod",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwClsMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwclsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&kwclsmethod_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&kwclsmethod_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&kwclsmethod_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kwclsmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &kwclsmethod_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ kwclsmethod_getsets,
	/* .tp_members       = */ kwclsmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&kwclsmethod_call_kw
};



/* Returns the name of the function bound by the given
 * `_ClassProperty', or `NULL' if the name could not be determined. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeClsProperty_GetName(DeeObject const *__restrict self) {
	struct type_getset const *iter;
	ASSERT_OBJECT(self);
	ASSERT(DeeClsProperty_Check(self));
	iter = DeeClsProperty_TYPE(self)->tp_getsets;
	if (iter) {
		for (; iter->gs_name; ++iter) {
			if (iter->gs_get == DeeClsProperty_GET(self) &&
			    iter->gs_del == DeeClsProperty_DEL(self) &&
			    iter->gs_set == DeeClsProperty_SET(self))
				return iter->gs_name;
		}
	}
	return NULL;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeClsProperty_GetDoc(DeeObject const *__restrict self) {
	struct type_getset const *iter;
	ASSERT_OBJECT(self);
	ASSERT(DeeClsProperty_Check(self));
	iter = DeeClsProperty_TYPE(self)->tp_getsets;
	if (iter) {
		for (; iter->gs_name; ++iter) {
			if (iter->gs_get == DeeClsProperty_GET(self) &&
			    iter->gs_del == DeeClsProperty_DEL(self) &&
			    iter->gs_set == DeeClsProperty_SET(self))
				return iter->gs_doc;
		}
	}
	return NULL;
}


/* Create a new unbound class property object. */
PUBLIC WUNUSED NONNULL((1)) DREF /*ClsProperty*/ DeeObject *DCALL
DeeClsProperty_New(DeeTypeObject *__restrict type,
                   dgetmethod_t get,
                   ddelmethod_t del,
                   dsetmethod_t set) {
	DeeClsPropertyObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeClsPropertyObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeClsProperty_Type);
	result->cp_type = type;
	Dee_Incref(type);
	result->cp_get = get;
	result->cp_del = del;
	result->cp_set = set;
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
clsproperty_hash(DeeClsPropertyObject *__restrict self) {
	return (Dee_HashPointer(self->cp_get) ^
	        Dee_HashPointer(self->cp_del) ^
	        Dee_HashPointer(self->cp_set));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsproperty_eq(DeeClsPropertyObject *self,
               DeeClsPropertyObject *other) {
	if (DeeObject_AssertTypeExact(other, &DeeClsProperty_Type))
		goto err;
	return_bool(self->cp_get == other->cp_get &&
	            self->cp_del == other->cp_del &&
	            self->cp_set == other->cp_set);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsproperty_ne(DeeClsPropertyObject *self,
               DeeClsPropertyObject *other) {
	if (DeeObject_AssertTypeExact(other, &DeeClsProperty_Type))
		goto err;
	return_bool(self->cp_get != other->cp_get ||
	            self->cp_del != other->cp_del ||
	            self->cp_set != other->cp_set);
err:
	return NULL;
}

PRIVATE struct type_cmp clsproperty_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&clsproperty_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsproperty_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsproperty_ne
};

INTERN struct keyword getter_kwlist[] = { K(thisarg), KEND };

PRIVATE struct keyword setter_kwlist[] = { K(thisarg), K(value), KEND };

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get(DeeClsPropertyObject *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	if (!self->cp_get) {
		err_cant_access_attribute_string(&DeeClsProperty_Type, STR_get, ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_Unpack(argc, argv, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cp_type)) {
		if (DeeObject_AssertType(thisarg, self->cp_type))
			goto err;
	}
	return (*self->cp_get)(thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_kw(DeeClsPropertyObject *__restrict self,
                   size_t argc, DeeObject *const *argv,
                   DeeObject *kw) {
	DeeObject *thisarg;
	if unlikely(!self->cp_get) {
		err_cant_access_attribute_string(&DeeClsProperty_Type, STR_get, ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cp_type)) {
		if (DeeObject_AssertType(thisarg, self->cp_type))
			goto err;
	}
	return (*self->cp_get)(thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_delete(DeeClsPropertyObject *__restrict self,
                   size_t argc, DeeObject *const *argv,
                   DeeObject *kw) {
	DeeObject *thisarg;
	if unlikely(!self->cp_del) {
		err_cant_access_attribute_string(&DeeClsProperty_Type, "delete", ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "o:delete", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cp_type)) {
		if (DeeObject_AssertType(thisarg, self->cp_type))
			goto err;
	}
	if unlikely((*self->cp_del)(thisarg))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_set(DeeClsPropertyObject *__restrict self,
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	DeeObject *thisarg, *value;
	if unlikely(!self->cp_set) {
		err_cant_access_attribute_string(&DeeClsProperty_Type, STR_set, ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_UnpackKw(argc, argv, kw, setter_kwlist, "oo:set", &thisarg, &value))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cp_type)) {
		if (DeeObject_AssertType(thisarg, self->cp_type))
			goto err;
	}
	if unlikely((*self->cp_set)(thisarg, value))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst clsproperty_methods[] = {
	TYPE_KWMETHOD_F(STR_get, &clsproperty_get_kw, TYPE_METHOD_FNOREFESCAPE, "(thisarg)->"),
	TYPE_KWMETHOD_F("delete", &clsproperty_delete, TYPE_METHOD_FNOREFESCAPE, "(thisarg)"),
	TYPE_KWMETHOD_F(STR_set, &clsproperty_set, TYPE_METHOD_FNOREFESCAPE, "(thisarg,value)"),
	TYPE_KWMETHOD_F("getter", &clsproperty_get_kw, TYPE_METHOD_FNOREFESCAPE, "(thisarg)->\nAlias for ?#get"),
	TYPE_KWMETHOD_F("setter", &clsproperty_set, TYPE_METHOD_FNOREFESCAPE, "(thisarg,value)\nAlias for ?#set"),
	TYPE_METHOD_END
};

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
clsproperty_getname(DeeClsPropertyObject *__restrict self) {
	char const *result = DeeClsProperty_GetName((DeeObject *)self);
	if unlikely(result == NULL)
		result = "<unknown>";
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
clsproperty_print(DeeClsPropertyObject *__restrict self,
                  dformatprinter printer, void *arg) {
	char permissions[COMPILER_LENOF(",get,del,set")], *p = permissions;
	char const *name = clsproperty_getname(self);
	if (self->cp_get)
		p = stpcpy(p, ",get");
	if (self->cp_del)
		p = stpcpy(p, ",del");
	if (self->cp_set)
		p = stpcpy(p, ",set");
	if (p == permissions)
		strcpy(p, ",-");
	return DeeFormat_Printf(printer, arg,
	                        "<class property %k.%s (%s)>",
	                        self->cp_type, name,
	                        permissions + 1);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
clsproperty_printrepr(DeeClsPropertyObject *__restrict self,
                      dformatprinter printer, void *arg) {
	char const *name = clsproperty_getname(self);
	return DeeFormat_Printf(printer, arg, "%r.%s", self->cp_type, name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_name(DeeClsPropertyObject *__restrict self) {
	char const *name;
	name = DeeClsProperty_GetName((DeeObject *)self);
	if unlikely(!name)
		return_none;
	return DeeString_NewAuto(name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_doc(DeeClsPropertyObject *__restrict self) {
	char const *doc;
	doc = DeeClsProperty_GetDoc((DeeObject *)self);
	if (!doc)
		return_none;
	return DeeString_NewAutoUtf8(doc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_module(DeeClsPropertyObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cp_type);
	if unlikely(!result)
		return_none;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_canget(DeeClsPropertyObject *__restrict self) {
	return_bool_(self->cp_get != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_candel(DeeClsPropertyObject *__restrict self) {
	return_bool_(self->cp_del != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_canset(DeeClsPropertyObject *__restrict self) {
	return_bool_(self->cp_set != NULL);
}

PRIVATE struct type_getset tpconst clsproperty_getsets[] = {
	TYPE_GETTER_F("canget", &clsproperty_canget, TYPE_GETSET_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this property has a getter callback"),
	TYPE_GETTER_F("candel", &clsproperty_candel, TYPE_GETSET_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this property has a delete callback"),
	TYPE_GETTER_F("canset", &clsproperty_canset, TYPE_GETSET_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this property has a setter callback"),
	TYPE_GETTER_F(STR___name__, &clsproperty_get_name, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "The name of @this property, or ?N if unknown"),
	TYPE_GETTER_F(STR___doc__, &clsproperty_get_doc, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "The documentation string of @this property, or ?N if unknown"),
	TYPE_GETTER_F(STR___module__, &clsproperty_get_module, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?DModule?N\n"
	              "The module implementing @this property, or ?N if unknown"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst clsproperty_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___type__, STRUCT_OBJECT, offsetof(DeeClsPropertyObject, cp_type),
	                      "->?DType\n"
	                      "The type implementing @this property"),
	TYPE_MEMBER_END
};

/* Make sure that we're allowed to re-use operators from ClassMethod. */
STATIC_ASSERT(offsetof(DeeClsPropertyObject, cp_type) ==
              offsetof(DeeClsMethodObject, cm_type));

PUBLIC DeeTypeObject DeeClsProperty_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassProperty",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsPropertyObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&clsproperty_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&clsproperty_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_get,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&clsmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &clsproperty_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ clsproperty_methods,
	/* .tp_getsets       = */ clsproperty_getsets,
	/* .tp_members       = */ clsproperty_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&clsproperty_get_kw,
};



/* Create a new unbound class member object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*ClsMember*/ DeeObject *DCALL
DeeClsMember_New(DeeTypeObject *__restrict type,
                 struct type_member const *desc) {
	DREF DeeClsMemberObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeClsMemberObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeClsMember_Type);
	result->cm_memb = *desc;
	result->cm_type = type;
	Dee_Incref(type);
done:
	return (DREF DeeObject *)result;
}

PRIVATE NONNULL((1)) void DCALL
clsmember_fini(DeeClsMemberObject *__restrict self) {
	Dee_Decref(self->cm_type);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
clsmember_print(DeeClsMemberObject *__restrict self,
                dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg,
	                        "<class member %k.%s>",
	                        self->cm_type,
	                        self->cm_memb.m_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
clsmember_printrepr(DeeClsMemberObject *__restrict self,
                    dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.%s",
	                        self->cm_type,
	                        self->cm_memb.m_name);
}

PRIVATE NONNULL((1, 2)) void DCALL
clsmember_visit(DeeClsMemberObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->cm_type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_get(DeeClsMemberObject *self, size_t argc,
              DeeObject *const *argv) {
	DeeObject *thisarg;
	if (DeeArg_Unpack(argc, argv, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cm_type)) {
		if (DeeObject_AssertType(thisarg, self->cm_type))
			goto err;
	}
	return type_member_get(&self->cm_memb, thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_get_kw(DeeClsMemberObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg;
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cm_type)) {
		if (DeeObject_AssertType(thisarg, self->cm_type))
			goto err;
	}
	return type_member_get(&self->cm_memb, thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_delete(DeeClsMemberObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg;
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "o:delete", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cm_type)) {
		if (DeeObject_AssertType(thisarg, self->cm_type))
			goto err;
	}
	if (type_member_del(&self->cm_memb, thisarg))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_set(DeeClsMemberObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg, *value;
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "oo:set", &thisarg, &value))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!DeeType_IsAbstract(self->cm_type)) {
		if (DeeObject_AssertType(thisarg, self->cm_type))
			goto err;
	}
	if (type_member_set(&self->cm_memb, thisarg, value))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst clsmember_methods[] = {
	TYPE_KWMETHOD_F(STR_get, &clsmember_get_kw, TYPE_METHOD_FNOREFESCAPE, "(thisarg)->"),
	TYPE_KWMETHOD_F("delete", &clsmember_delete, TYPE_METHOD_FNOREFESCAPE, "(thisarg)"),
	TYPE_KWMETHOD_F(STR_set, &clsmember_set, TYPE_METHOD_FNOREFESCAPE, "(thisarg,value)"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
clsmember_hash(DeeClsMemberObject *__restrict self) {
	return (Dee_HashPointer(self->cm_type) ^
	        Dee_HashPointer(self->cm_memb.m_name) ^
	        Dee_HashPointer(self->cm_memb.m_const));
}
#define CLSMEMBER_CMP(a, b) \
	memcmp(&(a)->cm_memb, &(b)->cm_memb, sizeof(DeeClsMemberObject) - offsetof(DeeClsMemberObject, cm_memb))
#define CLSMEMBER_BCMP(a, b) \
	bcmp(&(a)->cm_memb, &(b)->cm_memb, sizeof(DeeClsMemberObject) - offsetof(DeeClsMemberObject, cm_memb))
#define DEFINE_CLSMEMBER_COMPARE(name, CLSMEMBER_CMP, op)       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL       \
	name(DeeClsMemberObject *self, DeeClsMemberObject *other) { \
		if (DeeObject_AssertType(other, &DeeClsMember_Type))    \
			goto err;                                           \
		return_bool(CLSMEMBER_CMP(self, other) op 0);           \
	err:                                                        \
		return NULL;                                            \
	}
DEFINE_CLSMEMBER_COMPARE(clsmember_eq, CLSMEMBER_BCMP, ==)
DEFINE_CLSMEMBER_COMPARE(clsmember_ne, CLSMEMBER_BCMP, !=)
DEFINE_CLSMEMBER_COMPARE(clsmember_lo, CLSMEMBER_CMP, <)
DEFINE_CLSMEMBER_COMPARE(clsmember_le, CLSMEMBER_CMP, <=)
DEFINE_CLSMEMBER_COMPARE(clsmember_gr, CLSMEMBER_CMP, >)
DEFINE_CLSMEMBER_COMPARE(clsmember_ge, CLSMEMBER_CMP, >=)
#undef DEFINE_CLSMEMBER_COMPARE
#undef CLSMEMBER_BCMP
#undef CLSMEMBER_CMP

PRIVATE struct type_cmp clsmember_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&clsmember_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmember_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmember_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmember_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmember_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmember_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&clsmember_ge
};

PRIVATE struct type_member tpconst clsmember_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___type__, STRUCT_OBJECT, offsetof(DeeClsMemberObject, cm_type),
	                      "->?DType\n"
	                      "The type implementing @this member"),
	TYPE_MEMBER_FIELD_DOC(STR___name__, STRUCT_CONST | STRUCT_CSTR, offsetof(DeeClsMemberObject, cm_memb.m_name),
	                      "The name of @this member"),
	TYPE_MEMBER_FIELD_DOC(STR___doc__, STRUCT_CONST | STRUCT_CSTR_OPT, offsetof(DeeClsMemberObject, cm_memb.m_doc),
	                      "->?X2?Dstring?N\n"
	                      "The documentation string of @this member, or ?N if unknown"),
	TYPE_MEMBER_CONST_DOC("canget", Dee_True, "Always evaluates to ?t"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_canset(DeeClsMemberObject *__restrict self) {
	if (TYPE_MEMBER_ISCONST(&self->cm_memb))
		return_false;
	return_bool_(!(self->cm_memb.m_field.m_type & STRUCT_CONST));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_get_module(DeeClsMemberObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cm_type);
	if (!result)
		return_none;
	return result;
}

PRIVATE struct type_getset tpconst clsmember_getsets[] = {
	TYPE_GETTER_F("candel", &clsmember_canset, TYPE_GETSET_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Alias for #canset"),
	TYPE_GETTER_F("canset", &clsmember_canset, TYPE_GETSET_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this member can be modified"),
	TYPE_GETTER_F(STR___module__, &clsmember_get_module, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?DModule?N\n"
	              "The module implementing @this member, or ?N if unknown"),
	TYPE_GETSET_END
};

PUBLIC DeeTypeObject DeeClsMember_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassMember",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsMemberObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmember_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&clsmember_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&clsmember_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsmember_get,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&clsmember_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &clsmember_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ clsmember_methods,
	/* .tp_getsets       = */ clsmember_getsets,
	/* .tp_members       = */ clsmember_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&clsmember_get_kw
};



/* Make sure that we can re-use some functions from `ClassMethod' */
STATIC_ASSERT(offsetof(DeeCMethodObject, cm_func) ==
              offsetof(DeeClsMethodObject, cm_func));


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_call(DeeCMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeCMethod_CallFunc(self->cm_func, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
cmethod_getmodsym(DeeModuleObject *__restrict mod,
                  dcmethod_t func_ptr) {
	uint16_t addr;
	struct module_symbol *result;
	DeeModule_LockRead(mod);
	for (addr = 0; addr < mod->mo_globalc; ++addr) {
		DeeObject *glob = mod->mo_globalv[addr];
		if (!glob)
			continue;
		if (!DeeCMethod_Check(glob) &&
		    !DeeKwCMethod_Check(glob))
			continue;
		if (DeeCMethod_FUNC(glob) != func_ptr)
			continue;
		DeeModule_LockEndRead(mod);
		result = DeeModule_GetSymbolID(mod, addr);
		if (result)
			return result;
		DeeModule_LockRead(mod);
	}
	DeeModule_LockEndRead(mod);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member const *DCALL
type_member_search_cmethod(DeeTypeObject *type,
                           struct type_member const *chain,
                           DeeTypeObject **__restrict ptarget_type,
                           dcmethod_t func_ptr) {
	for (; chain->m_name; ++chain) {
		if (!TYPE_MEMBER_ISCONST(chain))
			continue;
		if (DeeType_Check(chain->m_const)) {
			/* XXX: Recursively search sub-types? (would require keeping a working set to prevent recursion) */
		} else if (DeeCMethod_Check(chain->m_const) ||
		           DeeKwCMethod_Check(chain->m_const)) {
			if (DeeCMethod_FUNC(chain->m_const) == func_ptr)
				goto gotit;
		}
	}
	return NULL;
gotit:
	*ptarget_type = type;
	return chain;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member const *DCALL
type_seach_cmethod(DeeTypeObject *self, dcmethod_t func_ptr,
                   DeeTypeObject **__restrict ptarget_type) {
	struct type_member const *result;
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, self);
	do {
		if (self->tp_class_members &&
		    (result = type_member_search_cmethod(self, self->tp_class_members, ptarget_type, func_ptr)) != NULL)
			goto gotit;
		if (self->tp_members &&
		    (result = type_member_search_cmethod(self, self->tp_members, ptarget_type, func_ptr)) != NULL)
			goto gotit;
	} while ((self = DeeTypeMRO_Next(&mro, self)) != NULL);
	return NULL;
gotit:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member const *DCALL
cmethod_gettypefield(DeeModuleObject *mod,
                     DREF DeeTypeObject **__restrict ptype,
                     dcmethod_t func_ptr) {
	uint16_t addr;
	struct type_member const *result;
	DeeModule_LockRead(mod);
	for (addr = 0; addr < mod->mo_globalc; ++addr) {
		DeeObject *glob = mod->mo_globalv[addr];
		if (!glob)
			continue;
		/* Check for cmethod objects defined as TYPE_MEMBER_CONST() fields of exported types. */
		if (!DeeType_Check(glob))
			continue;
		Dee_Incref(glob);
		DeeModule_LockEndRead(mod);
		result = type_seach_cmethod((DeeTypeObject *)glob, func_ptr, ptype);
		if (result) {
			Dee_Incref(*ptype);
			Dee_Decref_unlikely(glob);
			return result;
		}
		Dee_Decref_unlikely(glob);
		DeeModule_LockRead(mod);
	}
	DeeModule_LockEndRead(mod);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_module(DeeCMethodObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (!result)
		return_none;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_get_kwds(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *mod;
	DREF DeeObject *result;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod) {
		struct module_symbol *symbol;
		struct type_member const *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol) {
			if (symbol->ss_doc) {
				result = doc_decode_kwds((DeeObject *)mod,
				                         MODULE_SYMBOL_GETDOCSTR(symbol));
				Dee_Decref(mod);
				return result;
			}
		} else {
			member = cmethod_gettypefield(mod, &type, self->cm_func);
			if (member) {
				if (member->m_doc) {
					result = doc_decode_kwds((DeeObject *)mod,
					                         member->m_doc);
				} else {
					result = Dee_EmptySeq;
					Dee_Incref(Dee_EmptySeq);
				}
				Dee_Decref(mod);
				Dee_Decref(type);
				return result;
			}
		}
		Dee_Decref(mod);
	}
	return_empty_seq;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_name(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *mod;
	DREF DeeObject *result;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod) {
		struct module_symbol *symbol;
		struct type_member const *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol) {
			result = (DREF DeeObject *)module_symbol_getnameobj(symbol);
			Dee_Decref(mod);
			return result;
		}
		member = cmethod_gettypefield(mod, &type, self->cm_func);
		if (member) {
			result = DeeString_NewAuto(member->m_name);
			Dee_Decref(type);
			Dee_Decref(mod);
			return result;
		}
		Dee_Decref(mod);
	}
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_type(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *mod;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod) {
		struct module_symbol *symbol;
		struct type_member const *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol) {
			Dee_Decref(mod);
			return_none;
		}
		member = cmethod_gettypefield(mod, &type, self->cm_func);
		if (member) {
			Dee_Decref(mod);
			return (DREF DeeObject *)type;
		}
		Dee_Decref(mod);
	}
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_doc(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *mod;
	DREF DeeObject *result;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod) {
		struct module_symbol *symbol;
		struct type_member const *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol) {
			if (symbol->ss_doc) {
				result = (DREF DeeObject *)module_symbol_getdocobj(symbol);
				Dee_Decref(mod);
				return result;
			}
		} else {
			member = cmethod_gettypefield(mod, &type, self->cm_func);
			if (member) {
				if (member->m_doc) {
					result = DeeString_NewAutoUtf8(member->m_doc);
				} else {
					result = Dee_None;
					Dee_Incref(Dee_None);
				}
				Dee_Decref(mod);
				Dee_Decref(type);
				return result;
			}
		}
		Dee_Decref(mod);
	}
	return_none;
}

#define cmethod_get_kwds_doc objmethod_get_kwds_doc

#define cmethod_members (objmethod_members + OBJMETHOD_MEMBERS_INDEXOF_KWDS)
#define cmethod_getsets (kwcmethod_getsets + 1)
PRIVATE struct type_getset tpconst kwcmethod_getsets[] = {
	TYPE_GETTER_F(STR___kwds__, &kwcmethod_get_kwds, TYPE_GETSET_FNOREFESCAPE,
	              DOC_GET(cmethod_get_kwds_doc)),
	TYPE_GETTER_F(STR___module__, &cmethod_get_module, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?DModule?N\n"
	              "Returns the module defining @this method, or ?N if that module could not be determined"),
	TYPE_GETTER_F(STR___type__, &cmethod_get_type, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?DType?N\n"
	              "Returns the type as part of which @this method was declared, or ?N "
	              /**/ "if @this method was declared as part of a module, or if the type could "
	              /**/ "not be located"),
	TYPE_GETTER_F(STR___name__, &cmethod_get_name, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "Returns the name of @this method, or ?N if unknown"),
	TYPE_GETTER_F(STR___doc__, &cmethod_get_doc, TYPE_GETSET_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "Returns the documentation string of @this method, or ?N if unknown"),
	TYPE_GETSET_END
};


PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
cmethod_get_print_typename(DeeCMethodObject *__restrict self) {
	char const *result = "c-method";
	if (DeeKwCMethod_Check(self))
		result = "kw-c-method";
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cmethod_print(DeeCMethodObject *__restrict self,
              dformatprinter printer, void *arg) {
	DREF DeeModuleObject *mod;
	char const *type_name = cmethod_get_print_typename(self);
	dssize_t result;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod != NULL) {
		DREF DeeTypeObject *type;
		struct type_member const *member;
		struct module_symbol *symbol;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol != NULL) {
			result = DeeFormat_Printf(printer, arg, "<%s %k.%s>",
			                          type_name, mod, symbol->ss_name);
			Dee_Decref(mod);
			goto done;
		}
		member = cmethod_gettypefield(mod, &type, self->cm_func);
		if (member) {
			result = DeeFormat_Printf(printer, arg, "<%s %k.%k.%s>",
			                          type_name, mod, type, member->m_name);
			Dee_Decref(type);
			Dee_Decref(mod);
			goto done;
		}
		result = DeeFormat_Printf(printer, arg,
		                          "<%s %k.<unknown>>",
		                          type_name, mod);
		Dee_Decref(mod);
	} else {
		result = DeeFormat_Printf(printer, arg,
		                          "<%s <unknown>>",
		                          type_name);
	}
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cmethod_printrepr(DeeCMethodObject *__restrict self,
                  dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeModuleObject *mod;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod) {
		struct module_symbol *symbol;
		struct type_member const *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol) {
			result = DeeFormat_Printf(printer, arg, "%r.%s",
			                          mod, symbol->ss_name);
			Dee_Decref(mod);
			return result;
		}
		member = cmethod_gettypefield(mod, &type, self->cm_func);
		if (member) {
			result = DeeFormat_Printf(printer, arg, "%r.%k.%s",
			                          mod, type, member->m_name);
			Dee_Decref(type);
			Dee_Decref(mod);
			return result;
		}
		result = DeeFormat_Printf(printer, arg, "%r.<unknown %s>",
		                          mod, cmethod_get_print_typename(self));
		Dee_Decref(mod);
	} else {
		result = DeeFormat_Printf(printer, arg, "<unknown %s>",
		                          cmethod_get_print_typename(self));
	}
	return result;
}

PUBLIC DeeTypeObject DeeCMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CMethod",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCMethodObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cmethod_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cmethod_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&cmethod_call,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &clsmethod_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cmethod_getsets,
	/* .tp_members       = */ cmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


/* Make sure that we can re-use some functions from `CMethod' */
STATIC_ASSERT(offsetof(DeeKwCMethodObject, cm_func) ==
              offsetof(DeeCMethodObject, cm_func));


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_call(DeeKwCMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeKwCMethod_CallFunc(self->cm_func, argc, argv, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_call_kw(DeeKwCMethodObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	return DeeKwCMethod_CallFunc(self->cm_func, argc, argv, kw);
}

#define kwcmethod_print     cmethod_print
#define kwcmethod_printrepr cmethod_printrepr

PUBLIC DeeTypeObject DeeKwCMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwCMethod",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwCMethodObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&kwcmethod_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&kwcmethod_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&kwcmethod_call,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &clsmethod_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ kwcmethod_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&kwcmethod_call_kw
};


/* Helpers for dynamically creating C method wrapper objects.
 * You really shouldn't use these (unless you *really* need to
 * wrap functions dynamically with no way of creating CMETHOD
 * objects statically). These are really only here to allow
 * for portable bindings of the deemon API in languages other
 * than C.
 *
 * If at all possible, use `Dee_DEFINE_CMETHOD' instead! */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCMethod_New(Dee_cmethod_t func) {
	DREF DeeCMethodObject *result;
	result = DeeObject_MALLOC(DeeCMethodObject);
	if likely(result) {
		result->cm_func = func;
		DeeObject_Init(result, &DeeCMethod_Type);
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwCMethod_New(Dee_kwcmethod_t func) {
	DREF DeeKwCMethodObject *result;
	result = DeeObject_MALLOC(DeeKwCMethodObject);
	if likely(result) {
		result->cm_func = func;
		DeeObject_Init(result, &DeeKwCMethod_Type);
	}
	return (DREF DeeObject *)result;
}


/* Try to figure out doc information about `func' */
PUBLIC NONNULL((1, 2)) void DCALL
DeeCMethod_DocInfo(Dee_cmethod_t func, struct Dee_cmethod_docinfo *__restrict result) {
	bzero(result, sizeof(*result));
	result->dmdi_mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&func);
	if likely(result->dmdi_mod) {
		result->dmdi_modsym = cmethod_getmodsym(result->dmdi_mod, func);
		if (result->dmdi_modsym) {
			result->dmdi_name = result->dmdi_modsym->ss_name;
			result->dmdi_doc  = result->dmdi_modsym->ss_doc;
		} else {
			result->dmdi_typmem = cmethod_gettypefield(result->dmdi_mod, &result->dmdi_typ, func);
			if (result->dmdi_typmem) {
				result->dmdi_name = result->dmdi_typmem->m_name;
				result->dmdi_doc  = result->dmdi_typmem->m_doc;
			}
		}
	}
}




/* Invoke a given c-function callback.
 * Since this is the main way through which external functions are invoked,
 * we use this point to add some debug checks for proper use of exceptions.
 * That means we assert that the exception depth is manipulated as follows:
 * >> Dee_ASSERT(old_except_depth == new_except_depth + (return == NULL ? 1 : 0));
 * This way we can quickly determine improper use of exceptions in most cases,
 * even before the interpreter would crash because it tried to handle exceptions
 * when there were none.
 * A very common culprit for improper exception propagation is the return value
 * of printf-style format functions. Usually, code can just check for a non-zero
 * return value to determine if an integer-returning function has failed.
 * However, printf-style format functions return a negative value when that
 * happens, but return the positive sum of all printer calls on success (which
 * unless nothing was printed, is also non-zero).
 * This can easily be fixed by replacing:
 * >> if (DeeFormat_Printf(...)) goto err;
 * with this:
 * >> if (DeeFormat_Printf(...) < 0) goto err;
 */
#ifndef NDEBUG
PRIVATE ATTR_NORETURN void DCALL
fatal_invalid_except(DeeObject *__restrict return_value,
                     uint16_t excepted, void *callback_addr) {
	Dee_DPRINT_SET_ENABLED(true); /* We're about to crash, so don't hide any details */
	Dee_DPRINTF("\n\n"
	            "FATAL ERROR: Exception depth was improperly modified:\n"
	            "After a return value %p from C-function %p, the exception "
	            /**/ "depth should have been %u, but was actually %u\n"
	            "For details, see the C documentation of `DeeCMethod_CallFunc'",
	            return_value, callback_addr, excepted, DeeThread_Self()->t_exceptsz);
	/* TODO: Dump deemon call-stack trace. */
	Dee_BREAKPOINT();
	abort();
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCMethod_CallFunc_d(dcmethod_t fun, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*fun)(argc, argv);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObjMethod_CallFunc_d(dobjmethod_t fun, DeeObject *self,
                        size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*fun)(self, argc, argv);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwCMethod_CallFunc_d(dkwcmethod_t fun, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*fun)(argc, argv, kw);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwObjMethod_CallFunc_d(dkwobjmethod_t fun, DeeObject *self,
                          size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*fun)(self, argc, argv, kw);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}
#endif /* !NDEBUG */


INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeKwObjMethod_VCallFuncf(Dee_kwobjmethod_t funptr, DeeObject *__restrict thisarg,
                          char const *__restrict format, va_list args) {
	/* XXX: Optimizations? */
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeKwObjMethod_CallFunc(funptr, thisarg,
	                                 DeeTuple_SIZE(args_tuple),
	                                 DeeTuple_ELEM(args_tuple),
	                                 NULL);
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObjMethod_VCallFuncf(Dee_objmethod_t funptr, DeeObject *__restrict thisarg,
                        char const *__restrict format, va_list args) {
	/* XXX: Optimizations? */
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObjMethod_CallFunc(funptr, thisarg,
	                               DeeTuple_SIZE(args_tuple),
	                               DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJMETHOD_C */
