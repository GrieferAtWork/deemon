/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_MAP_C
#define GUARD_DEEMON_OBJECTS_MAP_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/thread.h>
#include <deemon/format.h>
#include <deemon/string.h>
#include <deemon/error.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

typedef struct {
    OBJECT_HEAD
    DREF DeeObject  *mpi_iter; /* [1..1][const] The iterator for enumerating `mpi_map'. */
    DREF DeeObject  *mpi_map;  /* [1..1][const] The mapping object for which this is a proxy. */
    struct type_nsi *mpi_nsi;  /* [0..1][const][->nsi_class == TYPE_SEQX_CLASS_MAP] If available, the NSI interface of the map. */
} MapProxyIterator;

PRIVATE int DCALL
proxy_iterator_ctor(MapProxyIterator *__restrict self) {
 self->mpi_iter = DeeObject_IterSelf(Dee_EmptyMapping);
 if unlikely(!self->mpi_iter) return -1;
 self->mpi_map  = Dee_EmptyMapping;
 Dee_Incref(self->mpi_map);
 self->mpi_nsi  = NULL;
 return 0;
}
PRIVATE int DCALL
proxy_iterator_init(MapProxyIterator *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 self->mpi_map = Dee_EmptyMapping;
 if (DeeArg_Unpack(argc,argv,"|o:_mappingproxy.iterator",&self->mpi_map))
     return -1;
 self->mpi_iter = DeeObject_IterSelf(self->mpi_map);
 if unlikely(!self->mpi_iter) return -1;
 Dee_Incref(self->mpi_map);
 self->mpi_nsi = DeeType_NSI(Dee_TYPE(self->mpi_map));
 if (self->mpi_nsi && self->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
     self->mpi_nsi = NULL;
 return 0;
}
PRIVATE int DCALL
proxy_iterator_copy(MapProxyIterator *__restrict self,
                    MapProxyIterator *__restrict other) {
 self->mpi_iter = DeeObject_Copy(other->mpi_iter);
 if unlikely(!self->mpi_iter) return -1;
 self->mpi_map = other->mpi_map;
 Dee_Incref(self->mpi_map);
 self->mpi_nsi = other->mpi_nsi;
 return 0;
}
PRIVATE void DCALL
proxy_iterator_fini(MapProxyIterator *__restrict self) {
 Dee_Decref(self->mpi_iter);
 Dee_Decref(self->mpi_map);
}
PRIVATE void DCALL
proxy_iterator_visit(MapProxyIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->mpi_iter);
 Dee_Visit(self->mpi_map);
}
PRIVATE struct type_member proxy_iterator_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(MapProxyIterator,mpi_map)),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
proxy_iterator_next_key(MapProxyIterator *__restrict self) {
 DREF DeeObject *pair;
 DREF DeeObject *key_and_value[2];
 int error;
 /* Optimize using NSI */
 if (self->mpi_nsi && self->mpi_nsi->nsi_maplike.nsi_nextkey)
     return (*self->mpi_nsi->nsi_maplike.nsi_nextkey)(self->mpi_iter);
 pair = DeeObject_IterNext(self->mpi_iter);
 if (pair == ITER_DONE) return ITER_DONE;
 if unlikely(!pair) goto err;
 error = DeeObject_Unpack(pair,2,key_and_value);
 Dee_Decref(pair);
 if unlikely(error) goto err;
 Dee_Decref(key_and_value[1]);
 return key_and_value[0];
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
proxy_iterator_next_value(MapProxyIterator *__restrict self) {
 DREF DeeObject *pair;
 DREF DeeObject *key_and_value[2];
 int error;
 /* Optimize using NSI */
 if (self->mpi_nsi && self->mpi_nsi->nsi_maplike.nsi_nextkey)
     return (*self->mpi_nsi->nsi_maplike.nsi_nextkey)(self->mpi_iter);
 pair = DeeObject_IterNext(self->mpi_iter);
 if (pair == ITER_DONE) return ITER_DONE;
 if unlikely(!pair) goto err;
 error = DeeObject_Unpack(pair,2,key_and_value);
 Dee_Decref(pair);
 if unlikely(error) goto err;
 Dee_Decref(key_and_value[0]);
 return key_and_value[1];
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
proxy_iterator_next_item(MapProxyIterator *__restrict self) {
 return DeeObject_IterNext(self->mpi_iter);
}


PRIVATE DeeTypeObject DeeMapKeysIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_mappingkeys.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict))&proxy_iterator_ctor,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&proxy_iterator_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict,size_t,DeeObject **__restrict))&proxy_iterator_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(MapProxyIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&proxy_iterator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&proxy_iterator_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_key,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */proxy_iterator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};
PRIVATE DeeTypeObject DeeMapValuesIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_mappingvalues.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(int(DCALL *)(DeeObject *__restrict))&proxy_iterator_ctor,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&proxy_iterator_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(int(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&proxy_iterator_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(MapProxyIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&proxy_iterator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&proxy_iterator_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_value,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */proxy_iterator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};
PRIVATE DeeTypeObject DeeMapItemsIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_mappingitems.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(int(DCALL *)(DeeObject *__restrict))&proxy_iterator_ctor,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&proxy_iterator_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(int(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&proxy_iterator_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(MapProxyIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&proxy_iterator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&proxy_iterator_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_item,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */proxy_iterator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};






typedef struct {
    OBJECT_HEAD
    DREF DeeObject *mp_map; /* [1..1][const] The mapping object for which this is a proxy. */
} MapProxy;

PRIVATE int DCALL
proxy_ctor(MapProxy *__restrict self) {
 self->mp_map = Dee_EmptyMapping;
 Dee_Incref(self->mp_map);
 return 0;
}
PRIVATE int DCALL
proxy_init(MapProxy *__restrict self, size_t argc,
           DeeObject **__restrict argv) {
 self->mp_map = Dee_EmptyMapping;
 if (DeeArg_Unpack(argc,argv,"|o:proxy",&self->mp_map))
     return -1;
 Dee_Incref(self->mp_map);
 return 0;
}
PRIVATE int DCALL
proxy_copy(MapProxy *__restrict self,
           MapProxy *__restrict other) {
 self->mp_map = other->mp_map;
 Dee_Incref(self->mp_map);
 return 0;
}
PRIVATE void DCALL
proxy_fini(MapProxy *__restrict self) {
 Dee_Decref(self->mp_map);
}
PRIVATE void DCALL
proxy_visit(MapProxy *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->mp_map);
}
PRIVATE DREF DeeObject *DCALL
proxy_size(MapProxy *__restrict self) {
 return DeeObject_SizeObject(self->mp_map);
}
PRIVATE DREF DeeObject *DCALL
proxy_contains_key(MapProxy *__restrict self, DeeObject *__restrict key) {
 return DeeObject_ContainsObject(self->mp_map,key);
}

PRIVATE DREF MapProxyIterator *DCALL
proxy_iterself(MapProxy *__restrict self, DeeTypeObject *__restrict result_type) {
 DREF MapProxyIterator *result;
 result = DeeObject_MALLOC(MapProxyIterator);
 if unlikely(!result) goto done;
 result->mpi_iter = DeeObject_IterSelf(self->mp_map);
 if unlikely(!result->mpi_iter) {
  DeeObject_Free(result);
  result = NULL;
  goto done;
 }
 result->mpi_nsi = DeeType_NSI(Dee_TYPE(self->mp_map));
 if (result->mpi_nsi && result->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
     result->mpi_nsi = NULL;
 result->mpi_map = self->mp_map;
 Dee_Incref(result->mpi_map);
 DeeObject_Init(result,result_type);
done:
 return result;
}

PRIVATE DREF MapProxyIterator *DCALL
proxy_iterself_keys(MapProxy *__restrict self) {
 return proxy_iterself(self,&DeeMapKeysIterator_Type);
}
PRIVATE DREF MapProxyIterator *DCALL
proxy_iterself_values(MapProxy *__restrict self) {
 return proxy_iterself(self,&DeeMapValuesIterator_Type);
}
PRIVATE DREF MapProxyIterator *DCALL
proxy_iterself_items(MapProxy *__restrict self) {
 return proxy_iterself(self,&DeeMapItemsIterator_Type);
}



PRIVATE struct type_seq proxykeys_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_keys,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self,DeeObject *__restrict))&proxy_contains_key
};
PRIVATE struct type_seq proxyvalues_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_values,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_size
};
PRIVATE struct type_seq proxyitems_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_items,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_size
};
PRIVATE struct type_member proxykeys_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeMapKeysIterator_Type),
    TYPE_MEMBER_END
};
PRIVATE struct type_member proxyvalues_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeMapValuesIterator_Type),
    TYPE_MEMBER_END
};
PRIVATE struct type_member proxyitems_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeMapItemsIterator_Type),
    TYPE_MEMBER_END
};



/* Base class for proxy views for mapping sequences. */
PRIVATE DeeTypeObject DeeMapProxy_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_mappingproxy",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict))&proxy_ctor,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&proxy_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(int(DCALL *)(DeeTypeObject *__restrict,DeeObject *__restrict,size_t,DeeObject **__restrict))&proxy_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(MapProxy)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&proxy_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&proxy_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&proxyitems_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */proxyitems_class_members
};
PRIVATE DeeTypeObject DeeMapKeys_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_mappingkeys",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapProxy_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(int(DCALL *)(DeeObject *__restrict))&proxy_ctor,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&proxy_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(int(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&proxy_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(MapProxy)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&proxykeys_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */proxykeys_class_members
};
PRIVATE DeeTypeObject DeeMapValues_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_mappingvalues",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapProxy_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(int(DCALL *)(DeeObject *__restrict))&proxy_ctor,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&proxy_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(int(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&proxy_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(MapProxy)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&proxyvalues_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */proxyvalues_class_members
};
PRIVATE DeeTypeObject DeeMapItems_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_mappingitems",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapProxy_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(int(DCALL *)(DeeObject *__restrict))&proxy_ctor,
                /* .tp_copy_ctor = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&proxy_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(int(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&proxy_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(MapProxy)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&proxyitems_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */proxyitems_class_members
};









INTDEF DREF DeeObject *DCALL new_empty_sequence_iterator(void);
PRIVATE DREF DeeObject *DCALL
map_iterself(DeeObject *__restrict self) {
 if unlikely(Dee_TYPE(self) == &DeeMapping_Type) {
  /* Special case: Create an empty iterator.
   * >> This can happen when someone tries to iterate a symbolic empty-mapping object. */
  return new_empty_sequence_iterator();
 }
 err_unimplemented_operator(Dee_TYPE(self),OPERATOR_ITERSELF);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
map_tpcontains(DeeObject *__restrict self, DeeObject *__restrict key) {
 DREF DeeObject *value;
 value = DeeObject_GetItem(self,key);
 if (!value) {
  if (DeeError_Catch(&DeeError_KeyError))
      return_false;
  return NULL;
 }
 Dee_Decref(value);
 return_true;
}

PRIVATE DREF DeeObject *DCALL
map_getitem(DeeObject *__restrict self, DeeObject *__restrict key) {
 DREF DeeObject *iter,*item;
 DREF DeeObject *item_key_and_value[2];
 dhash_t key_hash = DeeObject_Hash(key);
 /* Very inefficient: iterate the mapping to search for a matching key-item pair. */
 iter = DeeObject_IterSelf(self);
 if unlikely(!iter) goto err;
 while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
  int unpack_error,temp;
  unpack_error = DeeObject_Unpack(item,2,item_key_and_value);
  Dee_Decref(item);
  if unlikely(unpack_error) goto err_iter;
  /* Check if this is the key we're looking for. */
  if (DeeObject_Hash(item_key_and_value[0]) != key_hash) {
   Dee_Decref(item_key_and_value[0]);
  } else {
   temp = DeeObject_CompareEq(key,item_key_and_value[0]);
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
     err_unknown_key(self,key);
err_iter:
 Dee_Decref(iter);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
map_getrange(DeeObject *__restrict self,
             DeeObject *__restrict UNUSED(begin),
             DeeObject *__restrict UNUSED(end)) {
 /* Override the getrange operator of `sequence' as not-implemented. */
 err_unimplemented_operator(Dee_TYPE(self),OPERATOR_GETRANGE);
 return NULL;
}



PRIVATE struct type_math map_math = {
    /* .tp_int32  = */NULL,
    /* .tp_int64  = */NULL,
    /* .tp_double = */NULL,
    /* .tp_int    = */NULL,
    /* .tp_inv    = */NULL,
    /* .tp_pos    = */NULL,
    /* .tp_neg    = */NULL,
    /* .tp_add    = */NULL, /* TODO: map_concat (similar to concat for sequences, but also includes map operations) */
    /* .tp_sub    = */NULL,
    /* .tp_mul    = */NULL,
    /* .tp_div    = */NULL,
    /* .tp_mod    = */NULL,
    /* .tp_shl    = */NULL,
    /* .tp_shr    = */NULL,
    /* .tp_and    = */NULL, /* TODO: &DeeMap_Share */
    /* .tp_or     = */NULL, /* TODO: &DeeMap_Union */
    /* .tp_xor    = */NULL,
    /* .tp_pow    = */NULL
};

PRIVATE struct type_seq map_seq = {
    /* .tp_iter_self = */&map_iterself,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */&map_tpcontains,
    /* .tp_get       = */&map_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */&map_getrange,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};


PRIVATE DREF DeeObject *DCALL
map_repr(DeeObject *__restrict self) {
 struct unicode_printer p = UNICODE_PRINTER_INIT;
 DREF DeeObject *iterator = DeeObject_IterSelf(self);
 DREF DeeObject *elem; bool is_first = true;
 if unlikely(!iterator) return NULL;
 if unlikely(UNICODE_PRINTER_PRINT(&p,"{ ") < 0) goto err1;
 while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
  dssize_t print_error;
  DREF DeeObject *elem_key_and_value[2];
  if (DeeObject_Unpack(elem,2,elem_key_and_value)) goto err2;
  Dee_Decref(elem);
  print_error = unicode_printer_printf(&p,"%s%r: %r",
                                       is_first ? "" : ", ",
                                       elem_key_and_value[0],
                                       elem_key_and_value[1]);
  Dee_Decref(elem_key_and_value[1]);
  Dee_Decref(elem_key_and_value[0]);
  if (print_error < 0) goto err1;
  is_first = false;
  if (DeeThread_CheckInterrupt())
      goto err1;
 }
 if unlikely(!elem) goto err1;
 if unlikely((is_first ? unicode_printer_putascii(&p,'}')
                       : UNICODE_PRINTER_PRINT(&p," }")) < 0)
    goto err1;
 Dee_Decref(iterator);
 return unicode_printer_pack(&p);
err2: Dee_Decref(elem);
err1: Dee_Decref(iterator);
 unicode_printer_fini(&p);
 return NULL;
}

PRIVATE DREF MapProxy *DCALL
map_new_proxy(DeeObject *__restrict self, DeeTypeObject *__restrict result_type) {
 DREF MapProxy *result;
 result = DeeObject_MALLOC(MapProxy);
 if unlikely(!result) goto done;
 result->mp_map = self;
 Dee_Incref(self);
 DeeObject_Init(result,result_type);
done:
 return result;
}

PRIVATE DREF MapProxyIterator *DCALL
map_new_proxyiter(DeeObject *__restrict self, DeeTypeObject *__restrict result_type) {
 DREF MapProxyIterator *result;
 result = DeeObject_MALLOC(MapProxyIterator);
 if unlikely(!result) goto done;
 result->mpi_iter = DeeObject_IterSelf(self);
 if unlikely(!result->mpi_iter) {
  DeeObject_Free(result);
  return NULL;
 }
 result->mpi_map = self;
 Dee_Incref(self);
 if (result_type != &DeeMapItemsIterator_Type) {
  /* Search for an NSI descriptor defined by the mapping type. */
  result->mpi_nsi = DeeType_NSI(Dee_TYPE(self));
  if (result->mpi_nsi && result->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
      result->mpi_nsi = NULL;
 }
 DeeObject_Init(result,result_type);
done:
 return result;
}

PRIVATE DREF MapProxy *DCALL
map_keys(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":keys")) return NULL;
 return map_new_proxy(self,&DeeMapKeys_Type);
}
PRIVATE DREF MapProxy *DCALL
map_values(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":values")) return NULL;
 return map_new_proxy(self,&DeeMapValues_Type);
}
PRIVATE DREF MapProxy *DCALL
map_items(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":items")) return NULL;
 return map_new_proxy(self,&DeeMapItems_Type);
}
PRIVATE DREF MapProxyIterator *DCALL
map_iterkeys(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":iterkeys")) return NULL;
 return map_new_proxyiter(self,&DeeMapKeysIterator_Type);
}
PRIVATE DREF MapProxyIterator *DCALL
map_itervalues(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":itervalues")) return NULL;
 return map_new_proxyiter(self,&DeeMapValuesIterator_Type);
}
PRIVATE DREF MapProxyIterator *DCALL
map_iteritems(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":iteritems")) return NULL;
 return map_new_proxyiter(self,&DeeMapItemsIterator_Type);
}

PRIVATE DREF DeeObject *DCALL
map_get(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
 DeeObject *key,*def = Dee_None;
 if (DeeArg_Unpack(argc,argv,"o|o:get",&key,&def))
     return NULL;
 return DeeObject_GetItemDef(self,key,def);
}


INTERN struct type_method map_methods[] = {
    { DeeString_STR(&str_get), (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&map_get,
      DOC("(object key,object def=none)->object\n"
          "@return The value associated with @key or @def when @key has no value associated") },
    { "keys", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&map_keys,
      DOC("->sequence\nReturns a :sequence that can be enumerated to view only the keys of @this mapping") },
    { "values", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&map_values,
      DOC("->sequence\nReturns a :sequence that can be enumerated to view only the values of @this mapping") },
    { "items", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&map_items,
      DOC("->sequence\n"
          "Returns a :sequence that can be enumerated to view the key-item "
          "pairs as 2-element sequences, the same way they could be viewed "
          "if @this mapping itself was being iterated\n"
          "Note however that the returned :sequence is pure, meaning that it "
          "implements a index-based getitem and getrange operators, the same "
          "way one would expect of any regular object implementing the sequence "
          "protocol") },
    { "iterkeys", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&map_iterkeys,
      DOC("->iterator\nReturns an iterator for #{keys}. Same as ${this.keys().operator iter()}") },
    { "itervalues", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&map_itervalues,
      DOC("->iterator\nReturns an iterator for #{values}. Same as ${this.values().operator iter()}") },
    { "iteritems", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&map_iteritems,
      DOC("->iterator\nReturns an iterator for #{items}. Same as ${this.items().operator iter()}") },
    { NULL }
};


PRIVATE DREF DeeObject *DCALL
mapiter_next_key(DeeObject *__restrict self,
                 DeeObject *__restrict iter) {
 DeeTypeObject *tp_self;
 DREF DeeObject *result;
 DREF DeeObject *content[2];
 int error;
 tp_self = Dee_TYPE(self);
 do {
  if (tp_self->tp_seq) {
   struct type_nsi *nsi;
   nsi = tp_self->tp_seq->tp_nsi;
   if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP &&
       nsi->nsi_maplike.nsi_nextkey)
       return (*nsi->nsi_maplike.nsi_nextkey)(iter);
   break;
  }
 } while (type_inherit_nsi(tp_self));
 result = DeeObject_IterNext(iter);
 if (!ITER_ISOK(result)) {
  if unlikely(!result) goto err;
  return ITER_DONE;
 }
 error = DeeObject_Unpack(result,2,content);
 Dee_Decref(result);
 if unlikely(error) goto err;
 Dee_Decref(content[1]);
 return content[0];
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
DeeMap_GetFirst(DeeObject *__restrict self) {
 DREF DeeObject *iter,*result;
 iter = DeeObject_IterSelf(self);
 if unlikely(!iter) goto err;
 result = DeeObject_IterNext(iter);
 Dee_Decref(iter);
 if (result == ITER_DONE) goto err_empty;
 return result;
err_empty:
 err_empty_sequence(self);
err:
 return NULL;
}

PRIVATE int DCALL
DeeMap_DelFirst(DeeObject *__restrict self) {
 DREF DeeObject *iter,*key; int result;
 iter = DeeObject_IterSelf(self);
 if unlikely(!iter) goto err;
 key = mapiter_next_key(self,iter);
 Dee_Decref(iter);
 if (!ITER_ISOK(key)) {
  if unlikely(!key) goto err;
  goto err_empty;
 }
 result = DeeObject_DelItem(self,key);
 Dee_Decref(key);
 return result;
err_empty:
 err_empty_sequence(self);
err:
 return -1;
}

PRIVATE DREF DeeObject *DCALL
DeeMap_GetLast(DeeObject *__restrict self) {
 DREF DeeObject *iter,*result,*next;
 iter = DeeObject_IterSelf(self);
 if unlikely(!iter) goto err;
 result = DeeObject_IterNext(iter);
 if (!ITER_ISOK(result)) {
  Dee_Decref(iter);
  if (result == ITER_DONE)
      goto err_empty;
  goto err;
 }
 for (;;) {
  next = DeeObject_IterNext(iter);
  if (!ITER_ISOK(next)) break;
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

PRIVATE int DCALL
DeeMap_DelLast(DeeObject *__restrict self) {
 DREF DeeObject *iter,*key,*next; int result;
 iter = DeeObject_IterSelf(self);
 if unlikely(!iter) goto err;
 key = mapiter_next_key(self,iter);
 if (!ITER_ISOK(key)) {
  Dee_Decref(iter);
  if (key == ITER_DONE)
      goto err_empty;
  goto err;
 }
 for (;;) {
  next = mapiter_next_key(self,iter);
  if (!ITER_ISOK(next)) break;
  Dee_Decref(key);
  key = next;
 }
 Dee_Decref(iter);
 if unlikely(!next) { Dee_Decref(key); goto err; }
 result = DeeObject_DelItem(self,key);
 Dee_Decref(key);
 return result;
err_empty:
 err_empty_sequence(self);
err:
 return -1;
}

PRIVATE struct type_getset map_getsets[] = {
    { DeeString_STR(&str_first),
      &DeeMap_GetFirst,
      &DeeMap_DelFirst,
      NULL,
      DOC("->object") },
    { DeeString_STR(&str_last),
      &DeeMap_GetLast,
      &DeeMap_DelLast,
      NULL,
      DOC("->object") },
    { NULL }
};



PRIVATE DREF DeeTypeObject *DCALL
map_iterator_get(DeeTypeObject *__restrict self) {
 if (self == &DeeMapping_Type)
     return_reference_(&DeeIterator_Type);
 err_unknown_attribute(self,
                       DeeString_STR(&str_iterator),
                       ATTR_ACCESS_GET);
 return NULL;
}

PRIVATE struct type_getset map_class_getsets[] = {
    { DeeString_STR(&str_iterator), (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&map_iterator_get, NULL, NULL,
      DOC("->type\nReturns the iterator class used by instances of @this mapping type\n"
          "This member must be overwritten by sub-classes of :mapping") },
    { NULL }
};
PRIVATE struct type_member map_class_members[] = {
    TYPE_MEMBER_CONST_DOC("proxy",&DeeMapProxy_Type,"->type\nThe common base-class of #c:keys, #c:values and #c:items"),
    TYPE_MEMBER_CONST_DOC("keys",&DeeMapKeys_Type,"->type\nThe return type of the #i:keys member function"),
    TYPE_MEMBER_CONST_DOC("values",&DeeMapValues_Type,"->type\nThe return type of the #i:values member function"),
    TYPE_MEMBER_CONST_DOC("items",&DeeMapItems_Type,"->type\nThe return type of the #i:items member function"),
    TYPE_MEMBER_END
};

INTDEF int DCALL
seq_ctor(DeeObject *__restrict UNUSED(self));

PUBLIC DeeTypeObject DeeMapping_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_mapping),
    /* .tp_doc      = */DOC("A recommended abstract base class for any mapping "
                            "type that wishes to implement a key-value protocol\n"
                            "An object derived from this class must implement ${operator iter}, "
                            "and preferrably (but optionally) or ${operator []} (getitem)\n"
                            "The abstract declaration of a mapping-like sequence is ${{{object,object}...}}\n"
                            "\n"
                            "()\n"
                            "A no-op default constructor that is implicitly called by sub-classes\n"
                            "When invoked manually, a general-purpose, empty mapping is returned\n"
                            "\n"
                            "repr()\n"
                            "Returns the representation of all sequence elements, using "
                            "abstract mapping syntax\n"
                            "e.g.: ${{ \"foo\": 10, \"bar\": \"baz\" }}\n"
                            "\n"
                            "[:](int start,int end)\n"
                            "Always throws a :NotImplemented error\n"
                            "\n"
                            "iter->\n"
                            "Returns an iterator for enumerating key-value pairs as 2-elements sequences"
                            ),
    /* .tp_flags    = */TP_FNORMAL|TP_FABSTRACT|TP_FNAMEOBJECT, /* Generic base class type. */
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&seq_ctor, /* Allow default-construction of sequence objects. */
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeObject)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */&map_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */&map_math,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&map_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */map_methods,
    /* .tp_getsets       = */map_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */map_class_getsets,
    /* .tp_class_members = */map_class_members
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
