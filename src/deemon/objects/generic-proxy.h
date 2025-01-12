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
#ifndef GUARD_DEEMON_OBJECTS_GENERIC_PROXY_H
#define GUARD_DEEMON_OBJECTS_GENERIC_PROXY_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *po_obj; /* [1..1][const] The object being proxied */
} ProxyObject;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *po_obj1; /* [1..1][const] The first object being proxied */
	DREF DeeObject *po_obj2; /* [1..1][const] The second object being proxied */
} ProxyObject2;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *po_obj1; /* [1..1][const] The first object being proxied */
	DREF DeeObject *po_obj2; /* [1..1][const] The second object being proxied */
	DREF DeeObject *po_obj3; /* [1..1][const] The third object being proxied */
} ProxyObject3;

#define PROXY_OBJECT_HEAD_EX(Tobj, po_obj) \
	OBJECT_HEAD                            \
	DREF Tobj *po_obj; /* [1..1][const] The object being proxied */
#define PROXY_OBJECT_HEAD2_EX(Tobj1, po_obj1, Tobj2, po_obj2)               \
	OBJECT_HEAD                                                             \
	DREF Tobj1 *po_obj1; /* [1..1][const] The first object being proxied */ \
	DREF Tobj2 *po_obj2; /* [1..1][const] The second object being proxied */
#define PROXY_OBJECT_HEAD3_EX(Tobj1, po_obj1, Tobj2, po_obj2, Tobj3, po_obj3) \
	OBJECT_HEAD                                                               \
	DREF Tobj1 *po_obj1; /* [1..1][const] The first object being proxied */   \
	DREF Tobj2 *po_obj2; /* [1..1][const] The second object being proxied */  \
	DREF Tobj3 *po_obj3; /* [1..1][const] The third object being proxied */
#define PROXY_OBJECT_HEAD(po_obj) \
	PROXY_OBJECT_HEAD_EX(DeeObject, po_obj)
#define PROXY_OBJECT_HEAD2(po_obj1, po_obj2) \
	PROXY_OBJECT_HEAD2_EX(DeeObject, po_obj1, DeeObject, po_obj2)
#define PROXY_OBJECT_HEAD3(po_obj1, po_obj2, po_obj3) \
	PROXY_OBJECT_HEAD3_EX(DeeObject, po_obj1, DeeObject, po_obj2, DeeObject, po_obj3)

INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_copy_alias(ProxyObject *__restrict self, ProxyObject *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_copy_recursive(ProxyObject *__restrict self, ProxyObject *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_deepcopy(ProxyObject *__restrict self, ProxyObject *__restrict other);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_init(ProxyObject *__restrict self, size_t argc, DeeObject *const *argv); /* (obj) */

INTDEF NONNULL((1, 2)) void DCALL generic_proxy_visit(ProxyObject *__restrict self, dvisit_t proc, void *arg);
INTDEF NONNULL((1)) void DCALL generic_proxy_fini(ProxyObject *__restrict self);
#define generic_proxy_fini_likely   generic_proxy_fini
#define generic_proxy_fini_unlikely generic_proxy_fini

INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2_copy_alias12(ProxyObject2 *__restrict self, ProxyObject2 *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2_copy_recursive1_alias2(ProxyObject2 *__restrict self, ProxyObject2 *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2_deepcopy(ProxyObject2 *__restrict self, ProxyObject2 *__restrict other);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy2_init(ProxyObject2 *__restrict self, size_t argc, DeeObject *const *argv); /* (obj1,obj2) */

INTDEF NONNULL((1, 2)) void DCALL generic_proxy2_visit(ProxyObject2 *__restrict self, dvisit_t proc, void *arg);
INTDEF NONNULL((1)) void DCALL generic_proxy2_fini(ProxyObject2 *__restrict self);
#define generic_proxy2_fini_normal_likely     generic_proxy2_fini
#define generic_proxy2_fini_normal_unlikely   generic_proxy2_fini
#define generic_proxy2_fini_likely_normal     generic_proxy2_fini
#define generic_proxy2_fini_likely_likely     generic_proxy2_fini
#define generic_proxy2_fini_likely_unlikely   generic_proxy2_fini
#define generic_proxy2_fini_unlikely_normal   generic_proxy2_fini
#define generic_proxy2_fini_unlikely_likely   generic_proxy2_fini
#define generic_proxy2_fini_unlikely_unlikely generic_proxy2_fini

INTDEF NONNULL((1)) int DCALL generic_proxy2_bool_1or2(ProxyObject2 *__restrict self);

STATIC_ASSERT_MSG(offsetof(ProxyObject2, po_obj1) == offsetof(ProxyObject, po_obj),
                  "You're allowed to use everything below with `ProxyObject2', "
                  /**/ "and have the runtime only make use of `po_obj1'");

INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_bool(ProxyObject *__restrict self);                         /* DeeObject_Bool(self->po_obj) */
INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy_iter_advance(ProxyObject *__restrict self, size_t step); /* DeeObject_IterAdvance(self->po_obj, step) */

INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy_size(ProxyObject *self);                                                  /* DeeObject_Size(self->po_obj) */
INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy_size_fast(ProxyObject *self);                                             /* DeeObject_SizeFast(self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy_sizeob(ProxyObject *self);                                       /* DeeObject_SizeOb(self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy_iterkeys(ProxyObject *self);                                     /* DeeObject_IterKeys(self->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy_getitem(ProxyObject *self, DeeObject *key_or_index);          /* DeeObject_GetItem(self->po_obj, key_or_index) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_delitem(ProxyObject *self, DeeObject *key_or_index);                      /* DeeObject_DelItem(self->po_obj, key_or_index) */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_proxy_setitem(ProxyObject *self, DeeObject *key_or_index, DeeObject *value); /* DeeObject_SetItem(self->po_obj, key_or_index, value) */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_proxy_delrange(ProxyObject *self, DeeObject *start, DeeObject *end);         /* DeeObject_DelRange(self->po_obj, start, end) */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL generic_proxy_setrange(ProxyObject *self, DeeObject *start, DeeObject *end, DeeObject *values); /* DeeObject_SetRange(self->po_obj, start, end, values) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_hasitem(ProxyObject *self, DeeObject *key_or_index);                      /* DeeObject_HasItem(self->po_obj, key_or_index) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_bounditem(ProxyObject *self, DeeObject *key_or_index);                    /* DeeObject_BoundItem(self->po_obj, key_or_index) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_delitem_index(ProxyObject *__restrict self, size_t index);                   /* DeeObject_DelItemIndex(self->po_obj, index) */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL generic_proxy_setitem_index(ProxyObject *__restrict self, size_t index, DeeObject *value); /* DeeObject_SetItemIndex(self->po_obj, index, value) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_delrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end);       /* DeeObject_DelRangeIndex(self->po_obj, start, end) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_delrange_index_n(ProxyObject *self, Dee_ssize_t start);                      /* DeeObject_DelRangeIndexN(self->po_obj, start) */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL generic_proxy_setrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values); /* DeeObject_SetRangeIndex(self->po_obj, start, end, values) */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL generic_proxy_setrange_index_n(ProxyObject *self, Dee_ssize_t start, DeeObject *values); /* DeeObject_SetRangeIndexN(self->po_obj, start, values) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_hasitem_index(ProxyObject *__restrict self, size_t index);                   /* DeeObject_HasItemIndex(self->po_obj, index) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_bounditem_index(ProxyObject *__restrict self, size_t index);                 /* DeeObject_BoundItemIndex(self->po_obj, index) */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy_contains(ProxyObject *self, DeeObject *item);                 /* DeeObject_Contains(self->po_obj, item) */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL generic_proxy_getrange(ProxyObject *self, DeeObject *start, DeeObject *end); /* DeeObject_GetRange(self->po_obj, start, end) */

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy_getitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash);                             /* DeeObject_GetItemStringHash(self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_delitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash);                                         /* DeeObject_DelItemStringHash(self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL generic_proxy_setitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash, DeeObject *value);                    /* DeeObject_SetItemStringHash(self->po_obj, key, hash, value) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_hasitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash);                                         /* DeeObject_HasItemStringHash(self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_bounditem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash);                                       /* DeeObject_BoundItemStringHash(self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy_getitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash);          /* DeeObject_GetItemStringLenHash(self->po_obj, key, keylen, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_delitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash);                      /* DeeObject_DelItemStringLenHash(self->po_obj, key, keylen, hash) */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL generic_proxy_setitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value); /* DeeObject_SetItemStringHash(Lenself->po_obj, key, hash, keylen, value) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_hasitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash);                      /* DeeObject_HasItemStringLenHash(self->po_obj, key, keylen, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_bounditem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash);                    /* DeeObject_BoundItemStringLenHash(self->po_obj, key, keylen, hash) */

INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy_seq_bool(ProxyObject *__restrict self); /* DeeSeq_OperatorBool(self->po_obj) */
INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy_seq_size(ProxyObject *__restrict self); /* DeeSeq_OperatorSize(self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy_seq_sizeob(ProxyObject *__restrict self); /* DeeSeq_OperatorSizeOb(self->po_obj) */

/*INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_proxy_hash_id(ProxyObject *self);*/

INTDEF struct type_cmp generic_proxy_cmp_recursive;
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_proxy_hash_recursive(ProxyObject *__restrict self);               /* DeeObject_Hash(self->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_trycompare_eq_recursive(ProxyObject *self, ProxyObject *other); /* DeeObject_TryCompareEq(self->po_obj, other->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_compare_eq_recursive(ProxyObject *self, ProxyObject *other);    /* DeeObject_CompareEq(self->po_obj, other->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_compare_recursive(ProxyObject *self, ProxyObject *other);       /* DeeObject_Compare(self->po_obj, other->po_obj) */

INTDEF struct type_cmp generic_proxy2_cmp_recursive_ordered;
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_proxy2_hash_recursive_ordered(ProxyObject2 *__restrict self);      /* Dee_HashCombine(DeeObject_Hash(po_obj1), DeeObject_Hash(po_obj2)) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2_trycompare_eq_recursive(ProxyObject2 *self, ProxyObject2 *other); /* DeeObject_TryCompareEq(self->po_obj1, other->po_obj1) ?: DeeObject_TryCompareEq(self->po_obj2, other->po_obj2) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2_compare_eq_recursive(ProxyObject2 *self, ProxyObject2 *other);    /* DeeObject_CompareEq(self->po_obj1, other->po_obj1) ?: DeeObject_CompareEq(self->po_obj2, other->po_obj2) */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_GENERIC_PROXY_H */
