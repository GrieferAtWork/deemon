/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
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
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* _Exit(), abort() */
#include <deemon/thread.h>

#include <hybrid/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

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


PRIVATE WUNUSED NONNULL((1, 2)) char const *DCALL
typeobject_find_objmethod(DeeTypeObject *__restrict self,
                          dobjmethod_t meth) {
	do {
		struct type_method *iter;
		iter = self->tp_class_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == meth)
					return iter->m_name;
			}
	} while ((self = DeeType_Base(self)) != NULL);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) char const *DCALL
typeobject_find_objmethod_doc(DeeTypeObject *__restrict self,
                              dobjmethod_t meth) {
	do {
		struct type_method *iter;
		iter = self->tp_class_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == meth)
					return iter->m_doc;
			}
	} while ((self = DeeType_Base(self)) != NULL);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeTypeObject *DCALL
typeobject_find_objmethod_type(DeeTypeObject *__restrict self,
                               dobjmethod_t meth) {
	do {
		struct type_method *iter;
		iter = self->tp_class_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == meth)
					return self;
			}
	} while ((self = DeeType_Base(self)) != NULL);
	return NULL;
}

PUBLIC char const *DCALL
DeeObjMethod_GetName(DeeObject *__restrict self) {
	dobjmethod_t func;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	ASSERT(DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self));
	func    = DeeObjMethod_FUNC(self);
	tp_self = Dee_TYPE(DeeObjMethod_SELF(self));
	do {
		struct type_method *iter;
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
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
	return NULL;
}

PUBLIC char const *DCALL
DeeObjMethod_GetDoc(DeeObject *__restrict self) {
	dobjmethod_t func;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	ASSERT(DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self));
	func    = DeeObjMethod_FUNC(self);
	tp_self = Dee_TYPE(DeeObjMethod_SELF(self));
	do {
		struct type_method *iter;
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
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
	return NULL;
}

PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeObjMethod_GetType(DeeObject *__restrict self) {
	dobjmethod_t func;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	ASSERT(DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self));
	func    = DeeObjMethod_FUNC(self);
	tp_self = Dee_TYPE(DeeObjMethod_SELF(self));
	do {
		struct type_method *iter;
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
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
	return Dee_TYPE(DeeObjMethod_SELF(self));
}

PUBLIC WUNUSED NONNULL((1)) char const *DCALL
DeeClsMethod_GetName(DeeObject *__restrict self) {
	dobjmethod_t func;
	struct type_method *iter;
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

PUBLIC WUNUSED NONNULL((1)) char const *DCALL
DeeClsMethod_GetDoc(DeeObject *__restrict self) {
	dobjmethod_t func;
	struct type_method *iter;
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
	if (DeeObject_AssertType((DeeObject *)other, &DeeObjMethod_Type))
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
	if (DeeObject_AssertType((DeeObject *)other, &DeeObjMethod_Type))
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

PRIVATE struct type_getset objmethod_getsets[] = {
	{ "__func__",
	  (DREF DeeObject *(DCALL *)(DeeObject *))&objmethod_get_func, NULL, NULL,
	  DOC_GET(objmethod_get_func_doc) },
	{ DeeString_STR(&str___name__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&objmethod_get_name, NULL, NULL,
	  DOC_GET(objmethod_get_name_doc) },
	{ DeeString_STR(&str___doc__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&objmethod_get_doc, NULL, NULL,
	  DOC_GET(objmethod_get_doc_doc) },
	{ DeeString_STR(&str___type__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&objmethod_get_type, NULL, NULL,
	  DOC_GET(objmethod_get_type_doc) },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&objmethod_get_module, NULL, NULL,
	  DOC_GET(objmethod_get_module_doc) },
	{ NULL }
};

PRIVATE struct type_member objmethod_members[] = {
	TYPE_MEMBER_FIELD_DOC("__this__", STRUCT_OBJECT, offsetof(DeeObjMethodObject, om_this),
	                      "The object to which @this object-method is bound"),
#define OBJMETHOD_MEMBERS_INDEXOF_KWDS 1
	TYPE_MEMBER_CONST_DOC("__kwds__", Dee_EmptySeq, objmethod_get_kwds_doc), /* NOTE: _MUST_ always come last! */
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_str(DeeObjMethodObject *__restrict self) {
	char const *name    = DeeObjMethod_GetName((DeeObject *)self);
	DeeTypeObject *type = DeeObjMethod_GetType((DeeObject *)self);
	if unlikely(!name)
		name = "<unknown>";
	if (!type)
		return DeeString_Newf("<object method <unknown>.%s, bound to %r>", name, self->om_this);
	return DeeString_Newf("<object method %k.%s, bound to %r>", type, name, self->om_this);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_repr(DeeObjMethodObject *__restrict self) {
	char const *name;
	name = DeeObjMethod_GetName((DeeObject *)self);
	if unlikely(!name)
		name = "?";
	return DeeString_Newf("%r.%s", self->om_this, name);
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeObjMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&objmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&objmethod_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&objmethod_repr,
		/* .tp_bool = */ NULL
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

#ifdef CONFIG_NO_THREADS
#define DOCKWDSITER_RDITER(self)            ((self)->dki_iter)
#else /* CONFIG_NO_THREADS */
#define DOCKWDSITER_RDITER(self) ATOMIC_READ((self)->dki_iter)
#endif /* !CONFIG_NO_THREADS */

INTDEF DeeTypeObject DocKwds_Type;
INTDEF DeeTypeObject DocKwdsIterator_Type;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dockwdsiter_next(DocKwdsIterator *__restrict self) {
	char const *pos, *newpos;
	char const *name_end;
	bool is_escaped;
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifndef CONFIG_NO_THREADS
		pos = newpos = ATOMIC_READ(self->dki_iter);
#else /* !CONFIG_NO_THREADS */
		pos = newpos  = self->dki_iter;
#endif /* CONFIG_NO_THREADS */
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
#ifndef CONFIG_NO_THREADS
		if (ATOMIC_CMPXCH_WEAK(self->dki_iter, pos, newpos))
			break;
#else /* !CONFIG_NO_THREADS */
		self->dki_iter = newpos;
#endif /* CONFIG_NO_THREADS */
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

#define DEFINE_DOCKWDSITER_COMPARE(name, op)                                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                         \
	name(DocKwdsIterator *self, DocKwdsIterator *other) {                         \
		if (DeeObject_AssertTypeExact(other, &DocKwdsIterator_Type)) \
			goto err;                                                             \
		return_bool(DOCKWDSITER_RDITER(self) op DOCKWDSITER_RDITER(other));       \
	err:                                                                          \
		return NULL;                                                              \
	}
DEFINE_DOCKWDSITER_COMPARE(dockwdsiter_eq, ==)
DEFINE_DOCKWDSITER_COMPARE(dockwdsiter_ne, !=)
DEFINE_DOCKWDSITER_COMPARE(dockwdsiter_lo, <)
DEFINE_DOCKWDSITER_COMPARE(dockwdsiter_le, <=)
DEFINE_DOCKWDSITER_COMPARE(dockwdsiter_gr, >)
DEFINE_DOCKWDSITER_COMPARE(dockwdsiter_ge, >=)
#undef DEFINE_COTI_COMPARE

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

PRIVATE struct type_getset dockwdsiter_getsets[] = {
	{ "__docstr__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dockwdsiter_getdocstr, NULL, NULL,
	  DOC("->?Dstring") },
	{ NULL }
};

PRIVATE struct type_member dockwdsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(DocKwdsIterator, dki_kwds), "->?Ert:_DocKwds"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DocKwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DocKwdsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ (void *)&dockwdsiter_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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

PRIVATE struct type_member dockwds_members[] = {
	TYPE_MEMBER_FIELD("__docstr__", STRUCT_CONST | STRUCT_CSTR, offsetof(DocKwds, dk_start)),
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT, offsetof(DocKwds, dk_owner)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member dockwds_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DocKwdsIterator_Type),
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
				/* .tp_ctor      = */ (void *)dockwds_ctor,
				/* .tp_copy_ctor = */ (void *)dockwds_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)dockwds_init,
				TYPE_FIXED_ALLOCATOR(DocKwds)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dockwds_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
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




STATIC_ASSERT(COMPILER_OFFSETOF(DeeObjMethodObject, om_this) ==
              COMPILER_OFFSETOF(DeeKwObjMethodObject, om_this));
STATIC_ASSERT(COMPILER_OFFSETOF(DeeObjMethodObject, om_func) ==
              COMPILER_OFFSETOF(DeeKwObjMethodObject, om_func));
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

PRIVATE struct type_getset kwobjmethod_getsets[] = {
	{ "__func__",
	  (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_func, NULL, NULL,
	  DOC_GET(kwobjmethod_get_func_doc) },
	{ DeeString_STR(&str___name__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_name, NULL, NULL,
	  DOC_GET(kwobjmethod_get_name_doc) },
	{ DeeString_STR(&str___doc__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_doc, NULL, NULL,
	  DOC_GET(kwobjmethod_get_doc_doc) },
	{ DeeString_STR(&str___kwds__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_kwds, NULL, NULL,
	  DOC_GET(kwobjmethod_get_kwds_doc) },
	{ DeeString_STR(&str___type__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_type, NULL, NULL,
	  DOC_GET(kwobjmethod_get_type_doc) },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&kwobjmethod_get_module, NULL, NULL,
	  DOC_GET(kwobjmethod_get_module_doc) },
	{ NULL }
};
#define kwobjmethod_members objmethod_members
#define kwobjmethod_str     objmethod_str
#define kwobjmethod_repr    objmethod_repr

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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwObjMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwobjmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwobjmethod_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwobjmethod_repr,
		/* .tp_bool = */ NULL
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
	                       "with at least 1 argument");
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_call(DeeClsMethodObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_argc_zero;
	/* Allow non-instance objects for generic types. */
	if (!(self->cm_type->tp_flags & TP_FABSTRACT)) {
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
STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsMethodObject, cm_type) ==
              COMPILER_OFFSETOF(DeeObjMethodObject, om_this));
STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsMethodObject, cm_func) ==
              COMPILER_OFFSETOF(DeeObjMethodObject, om_func));
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_str(DeeClsMethodObject *__restrict self) {
	char const *name;
	name = DeeClsMethod_GetName((DeeObject *)self);
	if unlikely(!name)
		name = "<unknown>";
	return DeeString_Newf("<class method %r.%s>", self->cm_type, name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_repr(DeeClsMethodObject *__restrict self) {
	char const *name;
	name = DeeClsMethod_GetName((DeeObject *)self);
	if unlikely(!name)
		name = "?";
	return DeeString_Newf("%r.%s", self->cm_type, name);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
clsmethod_hash(DeeClsMethodObject *__restrict self) {
	return Dee_HashPointer(self->cm_func);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_eq(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func == other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_ne(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func != other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_lo(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func < other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_le(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func <= other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_gr(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, Dee_TYPE(self)))
		goto err;
	return_bool_(self->cm_func > other->cm_func);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_ge(DeeClsMethodObject *self,
             DeeClsMethodObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, Dee_TYPE(self)))
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
PRIVATE struct type_getset kwclsmethod_getsets[] = {
	{ DeeString_STR(&str___kwds__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwclsmethod_get_kwds, NULL, NULL,
	  DOC_GET(objmethod_get_kwds_doc) },
	{ DeeString_STR(&str___name__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&clsmethod_get_name, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "The name of @this method, or ?N if unknown") },
	{ DeeString_STR(&str___doc__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&clsmethod_get_doc, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "The documentation string of @this method, or ?N if unknown") },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&clsmethod_get_module, NULL, NULL,
	  DOC("->?X2?DModule?N\n"
	      "The module implementing @this method, or ?N if unknown") },
	{ NULL }
};
#define clsmethod_get_kwds_doc objmethod_get_kwds_doc


PRIVATE struct type_member kwclsmethod_members[] = {
	TYPE_MEMBER_CONST_DOC("__kwds__", Dee_EmptySeq, clsmethod_get_kwds_doc),
#define clsmethod_members (kwclsmethod_members + 1)
	TYPE_MEMBER_FIELD_DOC("__type__", STRUCT_OBJECT, offsetof(DeeClsMethodObject, cm_type),
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmethod_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmethod_repr,
		/* .tp_bool = */ NULL
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


STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsMethodObject, cm_func) ==
              COMPILER_OFFSETOF(DeeKwClsMethodObject, cm_func));
STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsMethodObject, cm_type) ==
              COMPILER_OFFSETOF(DeeKwClsMethodObject, cm_type));

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwclsmethod_call(DeeKwClsMethodObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_argc_zero;
	/* Allow non-instance objects for generic types. */
	if (!(self->cm_type->tp_flags & TP_FABSTRACT)) {
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
	if (!(self->cm_type->tp_flags & TP_FABSTRACT)) {
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

#define kwclsmethod_fini    clsmethod_fini
#define kwclsmethod_str     clsmethod_str
#define kwclsmethod_repr    clsmethod_repr
#define kwclsmethod_visit   clsmethod_visit
#define kwclsmethod_hash    clsmethod_hash
#define kwclsmethod_cmp     clsmethod_cmp

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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwClsMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwclsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwclsmethod_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwclsmethod_repr,
		/* .tp_bool = */ NULL
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



PUBLIC WUNUSED NONNULL((1)) char const *DCALL
DeeClsProperty_GetName(DeeObject *__restrict self) {
	struct type_getset *iter;
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

PUBLIC WUNUSED NONNULL((1)) char const *DCALL
DeeClsProperty_GetDoc(DeeObject *__restrict self) {
	struct type_getset *iter;
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

PRIVATE WUNUSED DREF DeeObject *DCALL
clsproperty_get_nokw(DeeClsPropertyObject *__restrict self,
                     size_t argc, DREF DeeObject *const *argv) {
	DeeObject *thisarg;
	if (!self->cp_get) {
		err_cant_access_attribute(&DeeClsProperty_Type,
		                          DeeString_STR(&str_get),
		                          ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_Unpack(argc, argv, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!(self->cp_type->tp_flags & TP_FABSTRACT) &&
	    DeeObject_AssertType(thisarg, self->cp_type))
		goto err;
	return (*self->cp_get)(thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
clsproperty_get(DeeClsPropertyObject *__restrict self,
                size_t argc, DREF DeeObject *const *argv,
                DeeObject *kw) {
	DeeObject *thisarg;
	if unlikely(!self->cp_get) {
		err_cant_access_attribute(&DeeClsProperty_Type,
		                          DeeString_STR(&str_get),
		                          ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!(self->cp_type->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(thisarg, self->cp_type))
			goto err;
	}
	return (*self->cp_get)(thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
clsproperty_delete(DeeClsPropertyObject *__restrict self,
                   size_t argc, DREF DeeObject *const *argv,
                   DeeObject *kw) {
	DeeObject *thisarg;
	if unlikely(!self->cp_del) {
		err_cant_access_attribute(&DeeClsProperty_Type, "delete", ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "o:delete", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!(self->cp_type->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(thisarg, self->cp_type))
			goto err;
	}
	if unlikely((*self->cp_del)(thisarg))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
clsproperty_set(DeeClsPropertyObject *__restrict self,
                size_t argc, DREF DeeObject *const *argv,
                DeeObject *kw) {
	DeeObject *thisarg, *value;
	if unlikely(!self->cp_set) {
		err_cant_access_attribute(&DeeClsProperty_Type,
		                          DeeString_STR(&str_set),
		                          ATTR_ACCESS_GET);
		goto err;
	}
	if (DeeArg_UnpackKw(argc, argv, kw, setter_kwlist, "oo:set", &thisarg, &value))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!(self->cp_type->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(thisarg, self->cp_type))
			goto err;
	}
	if unlikely((*self->cp_set)(thisarg, value))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method clsproperty_methods[] = {
	{ DeeString_STR(&str_get),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_get,
	  DOC("(thisarg)->"),
	  TYPE_METHOD_FKWDS },
	{ "delete",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_delete,
	  DOC("(thisarg)"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_set),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_set,
	  DOC("(thisarg,value)"),
	  TYPE_METHOD_FKWDS },
	{ "getter",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_get,
	  DOC("(thisarg)->\n"
	      "Alias for ?#get"),
	  TYPE_METHOD_FKWDS },
	{ "setter",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_set,
	  DOC("(thisarg,value)\n"
	      "Alias for ?#set"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_str(DeeClsPropertyObject *__restrict self) {
	char const *name;
	name = DeeClsProperty_GetName((DeeObject *)self);
	if unlikely(!name)
		name = "<unknown>";
	return DeeString_Newf("<class property %r.%s (%c%c%c)>",
	                      self->cp_type, name,
	                      self->cp_get ? 'G' : '-',
	                      self->cp_del ? 'D' : '-',
	                      self->cp_set ? 'S' : '-');
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_repr(DeeClsPropertyObject *__restrict self) {
	char const *name;
	name = DeeClsProperty_GetName((DeeObject *)self);
	if unlikely(!name)
		name = "?";
	return DeeString_Newf("%r.%s", self->cp_type, name);
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

PRIVATE struct type_getset clsproperty_getsets[] = {
	{ "canget",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsproperty_canget, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this property has a getter callback") },
	{ "candel",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsproperty_candel, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this property has a delete callback") },
	{ "canset",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsproperty_canset, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this property has a setter callback") },
	{ DeeString_STR(&str___name__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&clsproperty_get_name, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "The name of @this property, or ?N if unknown") },
	{ DeeString_STR(&str___doc__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&clsproperty_get_doc, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "The documentation string of @this property, or ?N if unknown") },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&clsproperty_get_module, NULL, NULL,
	  DOC("->?X2?DModule?N\n"
	      "The module implementing @this property, or ?N if unknown") },
	{ NULL }
};

PRIVATE struct type_member clsproperty_members[] = {
	TYPE_MEMBER_FIELD_DOC("__type__", STRUCT_OBJECT, offsetof(DeeClsPropertyObject, cp_type),
	                      "->?DType\n"
	                      "The type implementing @this property"),
	TYPE_MEMBER_END
};

/* Make sure that we're allowed to re-use operators from classmethod. */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeClsPropertyObject, cp_type) ==
              COMPILER_OFFSETOF(DeeClsMethodObject, cm_type));

PUBLIC DeeTypeObject DeeClsProperty_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassProperty",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsPropertyObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsproperty_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsproperty_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_get_nokw,
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
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&clsproperty_get,
};



/* Create a new unbound class property object. */
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_str(DeeClsMemberObject *__restrict self) {
	return DeeString_Newf("<class member %k.%s>",
	                      self->cm_type,
	                      self->cm_memb.m_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_repr(DeeClsMemberObject *__restrict self) {
	return DeeString_Newf("%k.%s", self->cm_type, self->cm_memb.m_name);
}

PRIVATE NONNULL((1, 2)) void DCALL
clsmember_visit(DeeClsMemberObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->cm_type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_get(DeeClsMemberObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg;
	if (DeeArg_UnpackKw(argc, argv, kw, getter_kwlist, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (!(self->cm_type->tp_flags & TP_FABSTRACT)) {
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
	if (!(self->cm_type->tp_flags & TP_FABSTRACT)) {
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
	if (!(self->cm_type->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(thisarg, self->cm_type))
			goto err;
	}
	if (type_member_set(&self->cm_memb, thisarg, value))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method clsmember_methods[] = {
	{ DeeString_STR(&str_get),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsmember_get,
	  DOC("(thisarg)->"),
	  TYPE_METHOD_FKWDS },
	{ "delete",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsmember_delete,
	  DOC("(thisarg)"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_set),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsmember_set,
	  DOC("(thisarg,value)"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
clsmember_hash(DeeClsMemberObject *__restrict self) {
	return (Dee_HashPointer(self->cm_type) ^
	        Dee_HashPointer(self->cm_memb.m_name) ^
	        Dee_HashPointer(self->cm_memb.m_const));
}
#define CLSMEMBER_CMP(a, b)              \
	memcmp(&(a)->cm_memb, &(b)->cm_memb, \
	       sizeof(DeeClsMemberObject) -  \
	       offsetof(DeeClsMemberObject, cm_memb))

#define DEFINE_CLSMEMBER_CMP(name, op)                                    \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                 \
	name(DeeClsMemberObject *self, DeeClsMemberObject *other) {           \
		if (DeeObject_AssertType((DeeObject *)other, &DeeClsMember_Type)) \
			goto err;                                                     \
		return_bool(CLSMEMBER_CMP(self, other) op 0);                     \
	err:                                                                  \
		return NULL;                                                      \
	}
DEFINE_CLSMEMBER_CMP(clsmember_eq, ==)
DEFINE_CLSMEMBER_CMP(clsmember_ne, !=)
DEFINE_CLSMEMBER_CMP(clsmember_lo, <)
DEFINE_CLSMEMBER_CMP(clsmember_le, <=)
DEFINE_CLSMEMBER_CMP(clsmember_gr, >)
DEFINE_CLSMEMBER_CMP(clsmember_ge, >=)
#undef DEFINE_CLSMEMBER_CMP
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

PRIVATE struct type_member clsmember_members[] = {
	TYPE_MEMBER_FIELD_DOC("__type__", STRUCT_OBJECT, offsetof(DeeClsMemberObject, cm_type),
	                      "->?DType\n"
	                      "The type implementing @this member"),
	TYPE_MEMBER_FIELD_DOC("__name__", STRUCT_CONST | STRUCT_CSTR, offsetof(DeeClsMemberObject, cm_memb.m_name),
	                      "The name of @this member"),
	TYPE_MEMBER_FIELD_DOC("__doc__", STRUCT_CONST | STRUCT_CSTR_OPT, offsetof(DeeClsMemberObject, cm_memb.m_doc),
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

PRIVATE struct type_getset clsmember_getsets[] = {
	{ "candel", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmember_canset, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Alias for #canset") },
	{ "canset", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmember_canset, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this member can be modified") },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&clsmember_get_module, NULL, NULL,
	  DOC("->?X2?DModule?N\n"
	      "The module implementing @this member, or ?N if unknown") },
	{ NULL }
};

PUBLIC DeeTypeObject DeeClsMember_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassMember",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsMemberObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmember_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmember_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&clsmember_repr,
		/* .tp_bool = */ NULL
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
	/* .tp_class_members = */ NULL
};



/* Make sure that we can re-use some functions from `classmethod' */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeCMethodObject, cm_func) ==
              COMPILER_OFFSETOF(DeeClsMethodObject, cm_func));


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_call(DeeCMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeCMethod_CallFunc(self->cm_func, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
cmethod_getmodsym(DeeModuleObject *__restrict module,
                  dcmethod_t func_ptr) {
	uint16_t addr;
	struct module_symbol *result;
	rwlock_read(&module->mo_lock);
	for (addr = 0; addr < module->mo_globalc; ++addr) {
		DeeObject *glob = module->mo_globalv[addr];
		if (!glob)
			continue;
		if (!DeeCMethod_Check(glob) &&
		    !DeeKwCMethod_Check(glob))
			continue;
		if (DeeCMethod_FUNC(glob) != func_ptr)
			continue;
		rwlock_endread(&module->mo_lock);
		result = DeeModule_GetSymbolID(module, addr);
		if (result)
			return result;
		rwlock_read(&module->mo_lock);
	}
	rwlock_endread(&module->mo_lock);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member *DCALL
type_member_search_cmethod(DeeTypeObject *type,
                           struct type_member *chain,
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

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member *DCALL
type_seach_cmethod(DeeTypeObject *self,
                   dcmethod_t func_ptr,
                   DeeTypeObject **__restrict ptarget_type) {
	struct type_member *result;
	do {
		if (self->tp_class_members &&
		    (result = type_member_search_cmethod(self, self->tp_class_members, ptarget_type, func_ptr)) != NULL)
			goto gotit;
		if (self->tp_members &&
		    (result = type_member_search_cmethod(self, self->tp_members, ptarget_type, func_ptr)) != NULL)
			goto gotit;
	} while ((self = DeeType_Base(self)) != NULL);
	return NULL;
gotit:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member *DCALL
cmethod_gettypefield(DeeModuleObject *module,
                     DREF DeeTypeObject **__restrict ptype,
                     dcmethod_t func_ptr) {
	uint16_t addr;
	struct type_member *result;
	rwlock_read(&module->mo_lock);
	for (addr = 0; addr < module->mo_globalc; ++addr) {
		DeeObject *glob = module->mo_globalv[addr];
		if (!glob)
			continue;
		/* Check for cmethod objects defined as TYPE_MEMBER_CONST() fields of exported types. */
		if (!DeeType_Check(glob))
			continue;
		Dee_Incref(glob);
		rwlock_endread(&module->mo_lock);
		result = type_seach_cmethod((DeeTypeObject *)glob, func_ptr, ptype);
		if (result) {
			Dee_Incref(*ptype);
			Dee_Decref_unlikely(glob);
			return result;
		}
		Dee_Decref_unlikely(glob);
		rwlock_read(&module->mo_lock);
	}
	rwlock_endread(&module->mo_lock);
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
		struct type_member *member;
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
	DREF DeeModuleObject *module;
	DREF DeeObject *result;
	module = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (module) {
		struct module_symbol *symbol;
		struct type_member *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(module, self->cm_func);
		if (symbol) {
			result = (DREF DeeObject *)module_symbol_getnameobj(symbol);
			Dee_Decref(module);
			return result;
		}
		member = cmethod_gettypefield(module, &type, self->cm_func);
		if (member) {
			result = DeeString_NewAuto(member->m_name);
			Dee_Decref(type);
			Dee_Decref(module);
			return result;
		}
		Dee_Decref(module);
	}
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_type(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *module;
	module = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (module) {
		struct module_symbol *symbol;
		struct type_member *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(module, self->cm_func);
		if (symbol) {
			Dee_Decref(module);
			return_none;
		}
		member = cmethod_gettypefield(module, &type, self->cm_func);
		if (member) {
			Dee_Decref(module);
			return (DREF DeeObject *)type;
		}
		Dee_Decref(module);
	}
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_doc(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *module;
	DREF DeeObject *result;
	module = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (module) {
		struct module_symbol *symbol;
		struct type_member *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(module, self->cm_func);
		if (symbol) {
			if (symbol->ss_doc) {
				result = (DREF DeeObject *)module_symbol_getdocobj(symbol);
				Dee_Decref(module);
				return result;
			}
		} else {
			member = cmethod_gettypefield(module, &type, self->cm_func);
			if (member) {
				if (member->m_doc) {
					result = DeeString_NewAutoUtf8(member->m_doc);
				} else {
					result = Dee_None;
					Dee_Incref(Dee_None);
				}
				Dee_Decref(module);
				Dee_Decref(type);
				return result;
			}
		}
		Dee_Decref(module);
	}
	return_none;
}

#define cmethod_get_kwds_doc objmethod_get_kwds_doc

#define cmethod_members (objmethod_members + OBJMETHOD_MEMBERS_INDEXOF_KWDS)
#define cmethod_getsets (kwcmethod_getsets + 1)
PRIVATE struct type_getset kwcmethod_getsets[] = {
	{ DeeString_STR(&str___kwds__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwcmethod_get_kwds, NULL, NULL,
	  DOC_GET(objmethod_get_kwds_doc) },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cmethod_get_module, NULL, NULL,
	  DOC("->?X2?DModule?N\n"
	      "Returns the module defining @this method, or ?N if that module could not be determined") },
	{ DeeString_STR(&str___type__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cmethod_get_type, NULL, NULL,
	  DOC("->?X2?DType?N\n"
	      "Returns the type as part of which @this method was declared, or ?N "
	      "if @this method was declared as part of a module, or if the type could "
	      "not be located") },
	{ DeeString_STR(&str___name__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cmethod_get_name, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "Returns the name of @this method, or ?N if unknown") },
	{ DeeString_STR(&str___doc__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cmethod_get_doc, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "Returns the documentation string of @this method, or ?N if unknown") },
	{ NULL }
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_str(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *module;
	DREF DeeObject *result;
	char const *name = Dee_TYPE(self)->tp_name + 1;
	module           = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (module) {
		struct module_symbol *symbol;
		struct type_member *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(module, self->cm_func);
		if (symbol) {
			result = DeeString_Newf("<%s %k.%s at %p>", name, module, symbol->ss_name, *(void **)&self->cm_func);
			Dee_Decref(module);
			return result;
		}
		member = cmethod_gettypefield(module, &type, self->cm_func);
		if (member) {
			result = DeeString_Newf("<%s %k.%k.%s at %p>", name, module, type, member->m_name, *(void **)&self->cm_func);
			Dee_Decref(type);
			Dee_Decref(module);
			return result;
		}
		Dee_Decref(module);
	}
	return DeeString_Newf("<%s at %p>", name, *(void **)&self->cm_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_repr(DeeCMethodObject *__restrict self) {
	DREF DeeModuleObject *module;
	DREF DeeObject *result;
	module = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (module) {
		struct module_symbol *symbol;
		struct type_member *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(module, self->cm_func);
		if (symbol) {
			result = DeeString_Newf("%k.%s", module, symbol->ss_name);
			Dee_Decref(module);
			return result;
		}
		member = cmethod_gettypefield(module, &type, self->cm_func);
		if (member) {
			result = DeeString_Newf("%k.%k.%s", module, type, member->m_name);
			Dee_Decref(type);
			Dee_Decref(module);
			return result;
		}
		Dee_Decref(module);
	}
	return DeeString_Newf("<%s at %p>",
	                      Dee_TYPE(self)->tp_name + 1,
	                      *(void **)&self->cm_func);
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeCMethodObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cmethod_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cmethod_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&cmethod_call,
	/* .tp_visit         = */ NULL, /* TODO: Visit the associated DEX */
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


/* Make sure that we can re-use some functions from `classmethod' */
STATIC_ASSERT(COMPILER_OFFSETOF(DeeKwCMethodObject, cm_func) ==
              COMPILER_OFFSETOF(DeeCMethodObject, cm_func));


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_call(DeeKwCMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeKwCMethod_CallFunc(self->cm_func, argc, argv, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_call_kw(DeeKwCMethodObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	return DeeKwCMethod_CallFunc(self->cm_func, argc, argv, kw);
}

#define kwcmethod_str cmethod_str
#define kwcmethod_repr cmethod_repr

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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwCMethodObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwcmethod_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwcmethod_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&kwcmethod_call,
	/* .tp_visit         = */ NULL, /* TODO: Visit the associated DEX */
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


#ifndef NDEBUG
PRIVATE ATTR_NORETURN void DCALL
fatal_invalid_except(DeeObject *__restrict return_value,
                     uint16_t excepted, void *callback_addr) {
	DEE_DPRINTF("Exception depth was improperly modified:\n"
	            "After a return value %p from C-function %p, the exception "
	            "depth should have been %u, but was actually %u\n"
	            "For details, see the C documentation of `DeeCMethod_CallFunc'",
	            return_value, callback_addr, excepted, DeeThread_Self()->t_exceptsz);
	BREAKPOINT();
#ifdef CONFIG_HAVE__Exit
	_Exit(EXIT_FAILURE);
#else /* CONFIG_HAVE__Exit */
	abort();
#endif /* !CONFIG_HAVE__Exit */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCMethod_CallFunc_d(dcmethod_t fun, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeThreadObject *caller = DeeThread_Self();
	uint16_t old_depth      = caller->t_exceptsz;
	result                  = (*fun)(argc, argv);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObjMethod_CallFunc_d(dobjmethod_t fun, DeeObject *self,
                        size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeThreadObject *caller = DeeThread_Self();
	uint16_t old_depth      = caller->t_exceptsz;
	result                  = (*fun)(self, argc, argv);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwCMethod_CallFunc_d(dkwcmethod_t fun, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeThreadObject *caller = DeeThread_Self();
	uint16_t old_depth      = caller->t_exceptsz;
	result                  = (*fun)(argc, argv, kw);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwObjMethod_CallFunc_d(dkwobjmethod_t fun, DeeObject *self,
                          size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeThreadObject *caller = DeeThread_Self();
	uint16_t old_depth      = caller->t_exceptsz;
	result                  = (*fun)(self, argc, argv, kw);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)fun);
	return result;
}
#endif /* !NDEBUG */


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJMETHOD_C */
