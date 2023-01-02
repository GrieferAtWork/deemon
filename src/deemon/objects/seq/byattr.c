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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_BYATTR_C
#define GUARD_DEEMON_OBJECTS_SEQ_BYATTR_C 1

#include "byattr.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include <stddef.h>

#include "../../runtime/strings.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
byattr_ctor(MapByAttr *__restrict self) {
	self->mba_map = Dee_EmptyMapping;
	Dee_Incref(Dee_EmptyMapping);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
byattr_copy(MapByAttr *__restrict self,
            MapByAttr *__restrict other) {
	self->mba_map = other->mba_map;
	Dee_Incref(self->mba_map);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
byattr_deep(MapByAttr *__restrict self,
            MapByAttr *__restrict other) {
	self->mba_map = DeeObject_DeepCopy(other->mba_map);
	if unlikely(!self->mba_map)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
byattr_init(MapByAttr *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_MappingByAttr", &self->mba_map))
		goto err;
	Dee_Incref(self->mba_map);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
byattr_fini(MapByAttr *__restrict self) {
	Dee_Decref(self->mba_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
byattr_visit(MapByAttr *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mba_map);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
byattr_getattr(MapByAttr *self, /*String*/ DeeObject *name) {
	return DeeObject_GetItem(self->mba_map, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
byattr_delattr(MapByAttr *self, /*String*/ DeeObject *name) {
	return DeeObject_DelItem(self->mba_map, name);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
byattr_setattr(MapByAttr *self, /*String*/ DeeObject *name, DeeObject *value) {
	return DeeObject_SetItem(self->mba_map, name, value);
}

struct byattr_enumattr_foreach_data {
	MapByAttr *self;
	Dee_enum_t proc;
	void      *arg;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
byattr_enumattr_foreach(void *arg, DeeObject *__restrict elem) {
	struct byattr_enumattr_foreach_data *cookie;
	DREF DeeObject *key_and_value[2];
	Dee_ssize_t result;
	cookie = (struct byattr_enumattr_foreach_data *)arg;
	if (DeeObject_Unpack(elem, 2, key_and_value))
		goto err;
	result = 0;
	Dee_Decref(key_and_value[1]);
	if (DeeString_Check(key_and_value[0])) {
		result = (*cookie->proc)((DeeObject *)cookie->self,
		                         DeeString_STR(key_and_value[0]),
		                         NULL,
		                         ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET |
		                         ATTR_IMEMBER | ATTR_PROPERTY | ATTR_NAMEOBJ,
		                         NULL, cookie->arg);
	}
	Dee_Decref(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
byattr_enumattr(DeeTypeObject *tp_self, MapByAttr *self,
                Dee_enum_t proc, void *arg) {
	struct byattr_enumattr_foreach_data cookie;
	(void)tp_self;
	cookie.self = self;
	cookie.proc = proc;
	cookie.arg  = arg;
	return DeeObject_Foreach(self->mba_map,
	                         &byattr_enumattr_foreach,
	                         &cookie);
}


PRIVATE struct type_attr tpconst byattr_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&byattr_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, DeeObject *))&byattr_delattr,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&byattr_setattr,
	/* .tp_enumattr = */ (Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_enum_t, void *))&byattr_enumattr
};

PRIVATE struct type_member tpconst byattr_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map__", STRUCT_OBJECT,
	                      offsetof(MapByAttr, mba_map),
	                      "->?DMapping\nUnderlying mapping"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
byattr_repr(MapByAttr *__restrict self) {
	return DeeString_Newf("%r.byattr", self->mba_map);
}



PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
byattr_hash(MapByAttr *__restrict self) {
	return DeeObject_Hash(self->mba_map);
}

#define DEFINE_BYATTR_COMPARE(name, Name)                                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                      \
	byattr_##name(MapByAttr *self, MapByAttr *other) {                         \
		if unlikely(DeeObject_AssertType(other, &MapByAttr_Type))              \
			goto err;                                                          \
		return DeeObject_Compare##Name##Object(self->mba_map, other->mba_map); \
err:                                                                           \
		return NULL;                                                           \
	}
DEFINE_BYATTR_COMPARE(eq, Eq)
DEFINE_BYATTR_COMPARE(ne, Ne)
DEFINE_BYATTR_COMPARE(lo, Lo)
DEFINE_BYATTR_COMPARE(le, Le)
DEFINE_BYATTR_COMPARE(gr, Gr)
DEFINE_BYATTR_COMPARE(ge, Ge)
#undef DEFINE_BYATTR_COMPARE

PRIVATE struct type_cmp byattr_cmp = {
	/* .tp_hash = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&byattr_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *))&byattr_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *))&byattr_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *))&byattr_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *))&byattr_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *))&byattr_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *))&byattr_ge,
};


INTERN DeeTypeObject MapByAttr_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingByAttr",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&byattr_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&byattr_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&byattr_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&byattr_init,
				TYPE_FIXED_ALLOCATOR(MapByAttr)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&byattr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&byattr_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&byattr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &byattr_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ &byattr_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ byattr_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


/* Create a new byattr proxy for `map' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
MapByAttr_New(DeeObject *__restrict map) {
	DREF MapByAttr *result;
	result = DeeObject_MALLOC(MapByAttr);
	if likely(result) {
		DeeObject_Init(result, &MapByAttr_Type);
		Dee_Incref(map);
		result->mba_map = map;
	}
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_BYATTR_C */
