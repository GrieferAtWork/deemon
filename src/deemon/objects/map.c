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
#ifndef GUARD_DEEMON_OBJECTS_MAP_C
#define GUARD_DEEMON_OBJECTS_MAP_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/byattr.h"
#include "seq/each.h"
#include "seq/hashfilter.h"

DECL_BEGIN

INTDEF struct keyword seq_byhash_kwlist[];

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_byhash(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_byhash_kwlist, "o:byhash", &template_))
		goto err;
	return DeeMap_HashFilter(self, DeeObject_Hash(template_));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_get(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return DeeObject_GetItemDef(self, key, def);
err:
	return NULL;
}




DOC_DEF(map_get_doc,
        "(key,def=!N)->\n"
        "@return The value associated with @key or @def when @key has no value associated");

DOC_DEF(map_byhash_doc,
        "(template:?O)->?S?T2?O?O\n"
        "@param template The object who's hash should be used to search for collisions\n"
        "Same as ?Abyhash?DSequence, but rather than comparing the hashes of the "
        /**/ "key-value pairs, search for pairs where the key matches the hash of @template");

INTDEF struct type_method tpconst map_methods[];
INTERN_TPCONST struct type_method tpconst map_methods[] = {
	TYPE_METHOD(STR_get, &map_get, DOC_GET(map_get_doc)),
	TYPE_KWMETHOD("byhash", &map_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_END
};



typedef struct {
	OBJECT_HEAD
	DREF DeeObject  *mpi_iter; /* [1..1][const] The iterator for enumerating `mpi_map'. */
	DREF DeeObject  *mpi_map;  /* [1..1][const] The mapping object for which this is a proxy. */
	struct type_nsi const *mpi_nsi;  /* [0..1][const][->nsi_class == TYPE_SEQX_CLASS_MAP] If available, the NSI interface of the map. */
} MapProxyIterator;

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_iterator_ctor(MapProxyIterator *__restrict self) {
	self->mpi_iter = DeeObject_IterSelf(Dee_EmptyMapping);
	if unlikely(!self->mpi_iter)
		goto err;
	self->mpi_map = Dee_EmptyMapping;
	Dee_Incref(self->mpi_map);
	self->mpi_nsi = NULL;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_iterator_init(MapProxyIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_mappingproxy.Iterator", &self->mpi_map))
		goto err;
	self->mpi_iter = DeeObject_IterSelf(self->mpi_map);
	if unlikely(!self->mpi_iter)
		goto err;
	Dee_Incref(self->mpi_map);
	self->mpi_nsi = DeeType_NSI(Dee_TYPE(self->mpi_map));
	if (self->mpi_nsi && self->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
		self->mpi_nsi = NULL;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_iterator_copy(MapProxyIterator *__restrict self,
                    MapProxyIterator *__restrict other) {
	self->mpi_iter = DeeObject_Copy(other->mpi_iter);
	if unlikely(!self->mpi_iter)
		goto err;
	self->mpi_map = other->mpi_map;
	Dee_Incref(self->mpi_map);
	self->mpi_nsi = other->mpi_nsi;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_iterator_deepcopy(MapProxyIterator *__restrict self,
                        MapProxyIterator *__restrict other) {
	self->mpi_iter = DeeObject_DeepCopy(other->mpi_iter);
	if unlikely(!self->mpi_iter)
		goto err;
	self->mpi_map = DeeObject_DeepCopy(other->mpi_map);
	if unlikely(!self->mpi_iter)
		goto err_iter;
	self->mpi_nsi = other->mpi_nsi;
	return 0;
err_iter:
	Dee_Decref(self->mpi_iter);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_iterator_fini(MapProxyIterator *__restrict self) {
	Dee_Decref(self->mpi_iter);
	Dee_Decref(self->mpi_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_iterator_visit(MapProxyIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mpi_iter);
	Dee_Visit(self->mpi_map);
}

PRIVATE struct type_member tpconst proxy_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapProxyIterator, mpi_map), "->?DMapping"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(MapProxyIterator, mpi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_iterator_next_key(MapProxyIterator *__restrict self) {
	DREF DeeObject *pair;
	DREF DeeObject *key_and_value[2];
	int error;
	/* Optimize using NSI */
	if (self->mpi_nsi && self->mpi_nsi->nsi_maplike.nsi_nextkey)
		return (*self->mpi_nsi->nsi_maplike.nsi_nextkey)(self->mpi_iter);
	pair = DeeObject_IterNext(self->mpi_iter);
	if (pair == ITER_DONE)
		return ITER_DONE;
	if unlikely(!pair)
		goto err;
	error = DeeObject_Unpack(pair, 2, key_and_value);
	Dee_Decref(pair);
	if unlikely(error)
		goto err;
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_iterator_next_value(MapProxyIterator *__restrict self) {
	DREF DeeObject *pair;
	DREF DeeObject *key_and_value[2];
	int error;
	/* Optimize using NSI */
	if (self->mpi_nsi && self->mpi_nsi->nsi_maplike.nsi_nextvalue)
		return (*self->mpi_nsi->nsi_maplike.nsi_nextvalue)(self->mpi_iter);
	pair = DeeObject_IterNext(self->mpi_iter);
	if (pair == ITER_DONE)
		return ITER_DONE;
	if unlikely(!pair)
		goto err;
	error = DeeObject_Unpack(pair, 2, key_and_value);
	Dee_Decref(pair);
	if unlikely(error)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_iterator_next_item(MapProxyIterator *__restrict self) {
	return DeeObject_IterNext(self->mpi_iter);
}


PRIVATE DeeTypeObject DeeMappingProxyIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingProxyIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_iterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_iterator_visit,
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
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeTypeObject DeeMappingKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingKeysIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
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
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_key,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeTypeObject DeeMappingValuesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingValuesIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
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
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_value,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeTypeObject DeeMappingItemsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingItemsIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
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
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_item,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};






typedef struct {
	OBJECT_HEAD
	DREF DeeObject *mp_map; /* [1..1][const] The mapping object for which this is a proxy. */
} MapProxy;

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_ctor(MapProxy *__restrict self) {
	self->mp_map = Dee_EmptyMapping;
	Dee_Incref(self->mp_map);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_init(MapProxy *__restrict self, size_t argc,
           DeeObject *const *argv) {
	self->mp_map = Dee_EmptyMapping;
	if (DeeArg_Unpack(argc, argv, "|o:_MappingProxy", &self->mp_map))
		goto err;
	Dee_Incref(self->mp_map);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_copy(MapProxy *__restrict self,
           MapProxy *__restrict other) {
	self->mp_map = other->mp_map;
	Dee_Incref(self->mp_map);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_deepcopy(MapProxy *__restrict self,
               MapProxy *__restrict other) {
	self->mp_map = DeeObject_DeepCopy(other->mp_map);
	if unlikely(!self->mp_map)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_fini(MapProxy *__restrict self) {
	Dee_Decref(self->mp_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_visit(MapProxy *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mp_map);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_size(MapProxy *__restrict self) {
	return DeeObject_SizeObject(self->mp_map);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
proxy_contains_key(MapProxy *self, DeeObject *key) {
	return DeeObject_ContainsObject(self->mp_map, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF MapProxyIterator *DCALL
proxy_iterself(MapProxy *__restrict self, DeeTypeObject *__restrict result_type) {
	DREF MapProxyIterator *result;
	result = DeeObject_MALLOC(MapProxyIterator);
	if unlikely(!result)
		goto done;
	result->mpi_iter = DeeObject_IterSelf(self->mp_map);
	if unlikely(!result->mpi_iter)
		goto err_r;
	result->mpi_nsi = DeeType_NSI(Dee_TYPE(self->mp_map));
	if (result->mpi_nsi && result->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
		result->mpi_nsi = NULL;
	result->mpi_map = self->mp_map;
	Dee_Incref(result->mpi_map);
	DeeObject_Init(result, result_type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
proxy_iterself_keys(MapProxy *__restrict self) {
	return proxy_iterself(self, &DeeMappingKeysIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
proxy_iterself_values(MapProxy *__restrict self) {
	return proxy_iterself(self, &DeeMappingValuesIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
proxy_iterself_items(MapProxy *__restrict self) {
	return proxy_iterself(self, &DeeMappingItemsIterator_Type);
}


PRIVATE struct type_seq proxykeys_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_keys,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict))&proxy_contains_key
};

PRIVATE struct type_seq proxyvalues_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_values,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_size
};

PRIVATE struct type_seq proxyitems_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_items,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_size
};

PRIVATE struct type_member tpconst proxykeys_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingKeysIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxyvalues_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingValuesIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxyitems_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingItemsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingProxyIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxyitems_byhash(MapProxy *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_byhash_kwlist, "o:byhash", &template_))
		goto err;
	/* Invoke byhash() on the underlying mapping. */
	return DeeObject_CallAttrStringf(self->mp_map, "byhash", PCKuSIZ,
	                                 DeeObject_Hash(template_));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxykeys_byhash(MapProxy *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	result = proxyitems_byhash(self, argc, argv, kw);
	if likely(result) {
		if (DeeTuple_Check(result)) {
			if (DeeTuple_SIZE(result) == 0)
				goto done;
			if (DeeTuple_SIZE(result) == 1) {
				DREF DeeObject *key_and_value[2];
				/* Unpack the pointed-to pair */
				if (DeeObject_Unpack(DeeTuple_GET(result, 0), 2, key_and_value))
					goto err_r;
				Dee_Decref(result);
				Dee_Decref(key_and_value[1]);
				/* Return the pair's first element (which is the key) */
				result = DeeTuple_NewUninitialized(1);
				if unlikely(!result) {
					Dee_Decref(key_and_value[0]);
					goto err;
				}
				DeeTuple_SET(result, 0, key_and_value[0]);
				goto done;
			}
		}
#ifndef __OPTIMIZE_SIZE__
		else if (Dee_TYPE(result) == &MapHashFilter_Type &&
		         ((HashFilter *)result)->f_seq == self->mp_map) {
			/* The mapping doesn't implement a proper byhash() function.
			 * In this case, we might as well just return a regular, old
			 * sequence hash filter object for ourself (the mapping's key
			 * view), rather than going through the each-wrapper below. */
			HashFilter *filter;
			filter = (HashFilter *)result;
			if unlikely(DeeObject_IsShared(filter)) {
				/* The filter is being shared, so we must create a new filter. */
				Dee_hash_t hash;
				hash = filter->f_hash;
				Dee_Decref_unlikely(filter);
				result = DeeSeq_HashFilter((DeeObject *)self, hash);
			} else {
				/* The filter isn't being shared. -> Can modify in-place */
				Dee_DecrefNokill(&MapHashFilter_Type);
				Dee_DecrefNokill(filter->f_seq);
				Dee_Incref(&SeqHashFilter_Type);
				Dee_Incref(self);
				filter->f_seq = (DREF DeeObject *)self;
				filter->ob_type = &SeqHashFilter_Type;
			}
			goto done;
		}
#endif /* !__OPTIMIZE_SIZE__ */

		/* Fallback: byhash() didn't return a tuple, or that tuple's length is >= 2
		 * In this case, we return the equivalent of `return byhash(template).each[0] as Sequence' */
		{
			DREF SeqEachOperator *each;
			each = SeqEachOperator_MALLOC(1);
			if unlikely(!each)
				goto err_r;
			each->so_opname    = OPERATOR_GETITEM;
			each->so_opargc    = 1;
			each->se_seq       = result; /* inherit reference */
			each->so_opargv[0] = &DeeInt_Zero;
			Dee_Incref(&DeeInt_Zero);
			DeeObject_Init(each, &SeqEachOperator_Type);
			/* Wrap the each-operator in a super-view for sequences, thus preventing
			 * the caller from accidentally extending the each-expression any further. */
			result = DeeSuper_New(&DeeSeq_Type, (DeeObject *)each);
			Dee_Decref_unlikely(each);
		}
	}
done:
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


DOC_REF(seq_byhash_doc);

INTDEF struct type_method tpconst proxykeys_methods[];
INTERN_TPCONST struct type_method tpconst proxykeys_methods[] = {
	TYPE_KWMETHOD("byhash", &proxykeys_byhash, DOC_GET(seq_byhash_doc)),
	TYPE_METHOD_END
};

INTDEF struct type_method tpconst proxyitems_methods[];
INTERN_TPCONST struct type_method tpconst proxyitems_methods[] = {
	TYPE_KWMETHOD("byhash", &proxyitems_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_END
};



/* Base class for proxy views for mapping sequences. */
PRIVATE DeeTypeObject DeeMappingProxy_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingProxy",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxyitems_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_class_members
};

PRIVATE DeeTypeObject DeeMappingKeys_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingKeys",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
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
	/* .tp_seq           = */ &proxykeys_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxykeys_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxykeys_class_members
};

PRIVATE DeeTypeObject DeeMappingValues_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingValues",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
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
	/* .tp_seq           = */ &proxyvalues_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxyvalues_class_members
};

PRIVATE DeeTypeObject DeeMappingItems_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingItems",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
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
	/* .tp_seq           = */ &proxyitems_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxyitems_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxyitems_class_members
};









INTDEF WUNUSED DREF DeeObject *DCALL new_empty_sequence_iterator(void);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_iterself(DeeObject *__restrict self) {
	if unlikely(Dee_TYPE(self) == &DeeMapping_Type) {
		/* Special case: Create an empty iterator.
		 * >> This can happen when someone tries to iterate a symbolic empty-mapping object. */
		return new_empty_sequence_iterator();
	}
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERSELF);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
map_tpcontains(DeeObject *self, DeeObject *key) {
	DREF DeeObject *value;
	value = DeeObject_GetItem(self, key);
	if (!value) {
		if (DeeError_Catch(&DeeError_KeyError))
			return_false;
		return NULL;
	}
	Dee_Decref(value);
	return_true;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
map_getitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *iter, *item;
	DREF DeeObject *item_key_and_value[2];
	dhash_t key_hash = DeeObject_Hash(key);
	/* Very inefficient: iterate the mapping to search for a matching key-item pair. */
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		int unpack_error, temp;
		unpack_error = DeeObject_Unpack(item, 2, item_key_and_value);
		Dee_Decref(item);
		if unlikely(unpack_error)
			goto err_iter;
		/* Check if this is the key we're looking for. */
		if (DeeObject_Hash(item_key_and_value[0]) != key_hash) {
			Dee_Decref(item_key_and_value[0]);
		} else {
			temp = DeeObject_CompareEq(key, item_key_and_value[0]);
			Dee_Decref(item_key_and_value[0]);
			if (temp != 0) {
				if unlikely(temp < 0)
					Dee_Clear(item_key_and_value[1]);
				/* Found it! */
				Dee_Decref(iter);
				return item_key_and_value[1];
			}
		}
		Dee_Decref(item_key_and_value[1]);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	if (item == ITER_DONE)
		err_unknown_key(self, key);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
map_getrange(DeeObject *self,
             DeeObject *UNUSED(begin),
             DeeObject *UNUSED(end)) {
	/* Override the getrange operator of `sequence' as not-implemented. */
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETRANGE);
	return NULL;
}



PRIVATE struct type_math map_math = {
	/* .tp_int32  = */ NULL,
	/* .tp_int64  = */ NULL,
	/* .tp_double = */ NULL,
	/* .tp_int    = */ NULL,
	/* .tp_inv    = */ NULL,
	/* .tp_pos    = */ NULL,
	/* .tp_neg    = */ NULL,
	/* .tp_add    = */ NULL, /* TODO: map_concat (similar to concat for sequences, but also includes map operations) */
	/* .tp_sub    = */ NULL,
	/* .tp_mul    = */ NULL,
	/* .tp_div    = */ NULL,
	/* .tp_mod    = */ NULL,
	/* .tp_shl    = */ NULL,
	/* .tp_shr    = */ NULL,
	/* .tp_and    = */ NULL, /* TODO: &DeeMap_Share */
	/* .tp_or     = */ NULL, /* TODO: &DeeMap_Union */
	/* .tp_xor    = */ NULL,
	/* .tp_pow    = */ NULL
};

PRIVATE struct type_seq map_seq = {
	/* .tp_iter_self = */ &map_iterself,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ &map_tpcontains,
	/* .tp_get       = */ &map_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ &map_getrange,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
map_eq_impl(DeeObject *__restrict self,
            DeeObject *__restrict other) {
	size_t pair_count = 0, other_size;
	int temp;
	DREF DeeObject *iter, *elem, *pair[2];
	DREF DeeObject *other_value;
	/* Check of all keys from `self' have the same value within `other' */
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		if (DeeObject_Unpack(elem, 2, pair))
			goto err_elem;
		Dee_Decref(elem);
		other_value = DeeObject_GetItemDef(other, pair[0], ITER_DONE);
		Dee_Decref(pair[0]);
		if (!ITER_ISOK(other_value)) {
			Dee_Decref(pair[1]);
			Dee_Decref(iter);
			if unlikely(!other_value)
				goto err;
			return 0; /* Key wasn't provided by `other' */
		}
		temp = DeeObject_CompareEq(pair[1], other_value);
		Dee_Decref(other_value);
		Dee_Decref(pair[1]);
		if (temp <= 0) {
			Dee_Decref(iter);
			return temp;
		}
		/* Track the number of pairs found in `self' */
		++pair_count;
	}
	if unlikely(!elem)
		goto err_iter;
	Dee_Decref(iter);
	/* Make sure that `other' has the same size as `self' */
	other_size = DeeObject_Size(other);
	if unlikely(other_size == (size_t)-1)
		goto err;
	return pair_count == other_size;
err_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
map_eq(DeeObject *self,
       DeeObject *other) {
	int error = map_eq_impl(self, other);
	if unlikely(error < 0)
		goto err;
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
map_ne(DeeObject *self,
       DeeObject *other) {
	int error = map_eq_impl(self, other);
	if unlikely(error < 0)
		goto err;
	return_bool_(!error);
err:
	return NULL;
}

PRIVATE struct type_cmp map_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ &map_eq,
	/* .tp_ne   = */ &map_ne,
	/* .tp_lo   = */ NULL, /* XXX: Sub-set */
	/* .tp_le   = */ NULL, /* XXX: Sub-set, or same */
	/* .tp_gr   = */ NULL, /* XXX: Super-set */
	/* .tp_ge   = */ NULL, /* XXX: Super-set, or same */
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_repr(DeeObject *__restrict self) {
	struct unicode_printer p = UNICODE_PRINTER_INIT;
	DREF DeeObject *iterator = DeeObject_IterSelf(self);
	DREF DeeObject *elem;
	bool is_first = true;
	if unlikely(!iterator)
		goto err;
	if unlikely(UNICODE_PRINTER_PRINT(&p, "{ ") < 0)
		goto err_p_iterator;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		dssize_t print_error;
		DREF DeeObject *elem_key_and_value[2];
		if (DeeObject_Unpack(elem, 2, elem_key_and_value))
			goto err_p_iterator_elem;
		Dee_Decref(elem);
		print_error = unicode_printer_printf(&p, "%s%r: %r",
		                                     is_first ? "" : ", ",
		                                     elem_key_and_value[0],
		                                     elem_key_and_value[1]);
		Dee_Decref(elem_key_and_value[1]);
		Dee_Decref(elem_key_and_value[0]);
		if (print_error < 0)
			goto err_p_iterator;
		is_first = false;
		if (DeeThread_CheckInterrupt())
			goto err_p_iterator;
	}
	if unlikely(!elem)
		goto err_p_iterator;
	if unlikely((is_first ? unicode_printer_putascii(&p, '}')
	                      : UNICODE_PRINTER_PRINT(&p, " }")) < 0)
		goto err_p_iterator;
	Dee_Decref(iterator);
	return unicode_printer_pack(&p);
err_p_iterator_elem:
	Dee_Decref(elem);
err_p_iterator:
	Dee_Decref(iterator);
	unicode_printer_fini(&p);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF MapProxy *DCALL
map_new_proxy(DeeObject *__restrict self, DeeTypeObject *__restrict result_type) {
	DREF MapProxy *result;
	result = DeeObject_MALLOC(MapProxy);
	if unlikely(!result)
		goto done;
	result->mp_map = self;
	Dee_Incref(self);
	DeeObject_Init(result, result_type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF MapProxyIterator *DCALL
map_new_proxyiter(DeeObject *__restrict self, DeeTypeObject *__restrict result_type) {
	DREF MapProxyIterator *result;
	result = DeeObject_MALLOC(MapProxyIterator);
	if unlikely(!result)
		goto done;
	result->mpi_iter = DeeObject_IterSelf(self);
	if unlikely(!result->mpi_iter)
		goto err_r;
	result->mpi_map = self;
	Dee_Incref(self);
	if (result_type != &DeeMappingItemsIterator_Type) {
		/* Search for an NSI descriptor defined by the mapping type. */
		result->mpi_nsi = DeeType_NSI(Dee_TYPE(self));
		if (result->mpi_nsi && result->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
			result->mpi_nsi = NULL;
	}
	DeeObject_Init(result, result_type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxy *DCALL
map_keys(DeeObject *__restrict self) {
	return map_new_proxy(self, &DeeMappingKeys_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxy *DCALL
map_values(DeeObject *__restrict self) {
	return map_new_proxy(self, &DeeMappingValues_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxy *DCALL
map_items(DeeObject *__restrict self) {
	return map_new_proxy(self, &DeeMappingItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
map_iterkeys(DeeObject *__restrict self) {
	return map_new_proxyiter(self, &DeeMappingKeysIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
map_itervalues(DeeObject *__restrict self) {
	return map_new_proxyiter(self, &DeeMappingValuesIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
map_iteritems(DeeObject *__restrict self) {
	return map_new_proxyiter(self, &DeeMappingItemsIterator_Type);
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapiter_next_key(DeeObject *self,
                 DeeObject *iter) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	DREF DeeObject *content[2];
	int error;
	tp_self = Dee_TYPE(self);
	do {
		if (tp_self->tp_seq) {
			struct type_nsi const *nsi;
			nsi = tp_self->tp_seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP &&
			    nsi->nsi_maplike.nsi_nextkey)
				return (*nsi->nsi_maplike.nsi_nextkey)(iter);
			break;
		}
	} while (type_inherit_nsi(tp_self));
	result = DeeObject_IterNext(iter);
	if (!ITER_ISOK(result)) {
		if unlikely(!result)
			goto err;
		return ITER_DONE;
	}
	error = DeeObject_Unpack(result, 2, content);
	Dee_Decref(result);
	if unlikely(error)
		goto err;
	Dee_Decref(content[1]);
	return content[0];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_GetFirst(DeeObject *__restrict self) {
	DREF DeeObject *iter, *result;
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (result == ITER_DONE)
		goto err_empty;
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeMap_DelFirst(DeeObject *__restrict self) {
	DREF DeeObject *iter, *key;
	int result;
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	key = mapiter_next_key(self, iter);
	Dee_Decref(iter);
	if (!ITER_ISOK(key)) {
		if unlikely(!key)
			goto err;
		goto err_empty;
	}
	result = DeeObject_DelItem(self, key);
	Dee_Decref(key);
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_GetLast(DeeObject *__restrict self) {
	DREF DeeObject *iter, *result, *next;
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterNext(iter);
	if (!ITER_ISOK(result)) {
		Dee_Decref(iter);
		if (result == ITER_DONE)
			goto err_empty;
		goto err;
	}
	for (;;) {
		next = DeeObject_IterNext(iter);
		if (!ITER_ISOK(next))
			break;
		Dee_Decref(result);
		result = next;
		if (DeeThread_CheckInterrupt())
			goto err_result;
	}
	Dee_Decref(iter);
	if unlikely(!next)
		Dee_Clear(result);
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return NULL;
err_result:
	Dee_Decref(result);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeMap_DelLast(DeeObject *__restrict self) {
	DREF DeeObject *iter, *key, *next;
	int result;
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	key = mapiter_next_key(self, iter);
	if (!ITER_ISOK(key)) {
		Dee_Decref(iter);
		if (key == ITER_DONE)
			goto err_empty;
		goto err;
	}
	for (;;) {
		next = mapiter_next_key(self, iter);
		if (!ITER_ISOK(next))
			break;
		Dee_Decref(key);
		key = next;
	}
	Dee_Decref(iter);
	if unlikely(!next) {
		Dee_Decref(key);
		goto err;
	}
	result = DeeObject_DelItem(self, key);
	Dee_Decref(key);
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return -1;
}

PRIVATE struct type_getset tpconst map_getsets[] = {
	TYPE_GETTER("keys", &map_keys,
	            "->?#Keys\n"
	            "Returns a ?DSequence that can be enumerated to view only the keys of @this Mapping"),
	TYPE_GETTER("values", &map_values,
	            "->?#Values\n"
	            "Returns a ?DSequence that can be enumerated to view only the values of @this Mapping"),
	TYPE_GETTER("items", &map_items,
	            "->?#Items\n"
	            "Returns a ?DSequence that can be enumerated to view the key-item "
	            /**/ "pairs as 2-element sequences, the same way they could be viewed "
	            /**/ "if @this Mapping itself was being iterated\n"
	            "Note however that the returned ?DSequence is pure, meaning that it "
	            /**/ "implements a index-based getitem and getrange operators, the same "
	            /**/ "way one would expect of any regular object implementing the sequence "
	            /**/ "protocol"),
	TYPE_GETTER("iterkeys", &map_iterkeys,
	            "->?AIterator?#Keys\n"
	            "Returns an iterator for ?#{keys}. Same as ${this.keys.operator iter()}"),
	TYPE_GETTER("itervalues", &map_itervalues,
	            "->?AIterator?#Values\n"
	            "Returns an iterator for ?#{values}. Same as ${this.values.operator iter()}"),
	TYPE_GETTER("iteritems", &map_iteritems,
	            "->?AIterator?#Items\n"
	            "Returns an iterator for ?#{items}. Same as ${this.items.operator iter()}"),
	TYPE_GETSET_NODOC(STR_first, &DeeMap_GetFirst, &DeeMap_DelFirst, NULL),
	TYPE_GETSET_NODOC(STR_last, &DeeMap_GetLast, &DeeMap_DelLast, NULL),
	TYPE_GETTER("byattr", &MapByAttr_New,
	            "->?Ert:MappingByAttr\n"
	            "Construct a wrapper for @this mapping that behaves like a generic class object, "
	            /**/ "such that any attribute address ${this.byattr.foo} behaves like ${this[\"foo\"]} "
	            /**/ "(during all of $get, $del and $set).\n"
	            "Note that the returned object doesn't implement the ?DSequence- or ?DMapping "
	            /**/ "interfaces, but instead simply behaves like a completely generic object.\n"
	            "This attribute only makes sense if @this mapping behaves like ${{string: Object}}."),
	TYPE_GETTER("frozen", &DeeRoDict_FromSequence,
	            "->?DMapping\n"
	            "Returns a read-only (frozen) copy of @this Mapping"),
	TYPE_GETSET_END
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_iterator_get(DeeTypeObject *__restrict self) {
	if (self == &DeeMapping_Type)
		return_reference_(&DeeIterator_Type);
	err_unknown_attribute(self, STR_Iterator, ATTR_ACCESS_GET);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_frozen_get(DeeTypeObject *__restrict self) {
	int error;
	DREF DeeTypeObject *result;
	struct attribute_info info;
	struct attribute_lookup_rules rules;
	rules.alr_name       = "Frozen";
	rules.alr_hash       = Dee_HashPtr("Frozen", COMPILER_STRLEN("Frozen"));
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = ATTR_PERMGET | ATTR_IMEMBER;
	rules.alr_perm_value = ATTR_PERMGET | ATTR_IMEMBER;
	error = DeeAttribute_Lookup(Dee_TYPE(self),
	                            (DeeObject *)self,
	                            &info,
	                            &rules);
	if unlikely(error < 0)
		goto err;
	if (error != 0)
		return_reference_(&DeeRoDict_Type);
	if (info.a_attrtype) {
		result = info.a_attrtype;
		Dee_Incref(result);
	} else if (info.a_decl == (DeeObject *)&DeeMapping_Type) {
		result = &DeeRoDict_Type;
		Dee_Incref(&DeeRoDict_Type);
	} else {
		if (info.a_doc) {
			/* TODO: Use doc meta-data to determine the return type! */
		}
		/* Fallback: just tell the caller what they already know: a Mapping will be returned... */
		result = &DeeMapping_Type;
		Dee_Incref(&DeeMapping_Type);
	}
	attribute_info_fini(&info);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst map_class_getsets[] = {
	TYPE_GETTER(STR_Iterator, &map_iterator_get,
	            "->?DType\n"
	            "Returns the iterator class used by instances of @this Mapping type\n"
	            "This member must be overwritten by sub-classes of ?DMapping"),
	TYPE_GETTER("Frozen", &map_frozen_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the #i:frozen property"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst map_class_members[] = {
	TYPE_MEMBER_CONST_DOC("Proxy", &DeeMappingProxy_Type,
	                      "->?DType\n"
	                      "The common base-class of ?#Keys, ?#Values and ?#Items"),
	TYPE_MEMBER_CONST_DOC("Keys", &DeeMappingKeys_Type,
	                      "->?DType\n"
	                      "The return type of the ?#keys member function"),
	TYPE_MEMBER_CONST_DOC("Values", &DeeMappingValues_Type,
	                      "->?DType\n"
	                      "The return type of the ?#values member function"),
	TYPE_MEMBER_CONST_DOC("Items", &DeeMappingItems_Type,
	                      "->?DType\n"
	                      "The return type of the ?#items member function"),
	TYPE_MEMBER_END
};

INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));

/* `Mapping from deemon' */
PUBLIC DeeTypeObject DeeMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Mapping),
	/* .tp_doc      = */ DOC("A recommended abstract base class for any Mapping "
	                         "type that wishes to implement a key-value protocol\n"
	                         "An object derived from this class must implement ${operator iter}, "
	                         /**/ "and preferrably (but optionally) or ${operator []} (getitem)\n"
	                         /**/ "The abstract declaration of a mapping-like sequence is ${{{object, object}...}}\n"
	                         "\n"

	                         "()\n"
	                         "A no-op default constructor that is implicitly called by sub-classes\n"
	                         "When invoked manually, a general-purpose, empty Mapping is returned\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the representation of all sequence elements, "
	                         /**/ "using abstract mapping syntax\n"
	                         "e.g.: ${{ \"foo\": 10, \"bar\": \"baz\" }}\n"
	                         "\n"

	                         "[:]->!D\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating key-value pairs as 2-elements sequences"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ &map_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &map_math,
	/* .tp_cmp           = */ &map_cmp,
	/* .tp_seq           = */ &map_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ map_methods,
	/* .tp_getsets       = */ map_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ map_class_getsets,
	/* .tp_class_members = */ map_class_members
};


/* An empty instance of a generic mapping object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeMapping_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeMapping_Type' should be considered stub/empty. */
PUBLIC DeeObject DeeMapping_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeMapping_Type)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_MAP_C */
