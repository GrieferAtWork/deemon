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
#ifndef GUARD_DEEMON_OBJECTS_GENERIC_PROXY_H
#define GUARD_DEEMON_OBJECTS_GENERIC_PROXY_H 1

#include <deemon/api.h>

#include <deemon/object.h>

#include <hybrid/typecore.h>

#include <stddef.h> /* offsetof, size_t */

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

struct Dee_serial;

INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__copy_alias(ProxyObject *__restrict self, ProxyObject *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__copy_recursive(ProxyObject *__restrict self, ProxyObject *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__deepcopy(ProxyObject *__restrict self, ProxyObject *__restrict other);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__init(ProxyObject *__restrict self, size_t argc, DeeObject *const *argv); /* (obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__serialize(ProxyObject *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);

INTDEF NONNULL((1, 2)) void DCALL generic_proxy__visit(ProxyObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1)) void DCALL generic_proxy__fini(ProxyObject *__restrict self);
#define generic_proxy__fini_likely   generic_proxy__fini
#define generic_proxy__fini_unlikely generic_proxy__fini

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__getobj(ProxyObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2__copy_alias12(ProxyObject2 *__restrict self, ProxyObject2 *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2__copy_recursive1_alias2(ProxyObject2 *__restrict self, ProxyObject2 *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2__deepcopy(ProxyObject2 *__restrict self, ProxyObject2 *__restrict other);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy2__init(ProxyObject2 *__restrict self, size_t argc, DeeObject *const *argv); /* (obj1,obj2) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2__serialize(ProxyObject2 *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);

INTDEF NONNULL((1, 2)) void DCALL generic_proxy2__visit(ProxyObject2 *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1)) void DCALL generic_proxy2__fini(ProxyObject2 *__restrict self);
#define generic_proxy2__fini_normal_likely     generic_proxy2__fini
#define generic_proxy2__fini_normal_unlikely   generic_proxy2__fini
#define generic_proxy2__fini_likely_normal     generic_proxy2__fini
#define generic_proxy2__fini_likely_likely     generic_proxy2__fini
#define generic_proxy2__fini_likely_unlikely   generic_proxy2__fini
#define generic_proxy2__fini_unlikely_normal   generic_proxy2__fini
#define generic_proxy2__fini_unlikely_likely   generic_proxy2__fini
#define generic_proxy2__fini_unlikely_unlikely generic_proxy2__fini

INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy3__copy_alias123(ProxyObject3 *__restrict self, ProxyObject3 *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy3__deepcopy(ProxyObject3 *__restrict self, ProxyObject3 *__restrict other);
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy3__init(ProxyObject3 *__restrict self, size_t argc, DeeObject *const *argv); /* (obj1,obj2,obj3) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy3__serialize(ProxyObject3 *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);

INTDEF NONNULL((1, 2)) void DCALL generic_proxy3__visit(ProxyObject3 *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1)) void DCALL generic_proxy3__fini(ProxyObject3 *__restrict self);


/* Same as `generic_proxy__serialize()', but look at "tp_instance_size" (or
 * "tp_alloc") and memcpy all memory that is located after "ProxyObject". */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__serialize_and_memcpy(ProxyObject *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2__serialize_and_memcpy(ProxyObject2 *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy3__serialize_and_memcpy(ProxyObject3 *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
#define generic_proxy__serialize_and_wordcopy_atomic8  generic_proxy__serialize_and_memcpy
#ifdef CONFIG_NO_THREADS
#define generic_proxy__serialize_and_wordcopy_atomic16 generic_proxy__serialize_and_memcpy
#define generic_proxy__serialize_and_wordcopy_atomic32 generic_proxy__serialize_and_memcpy
#define generic_proxy__serialize_and_wordcopy_atomic64 generic_proxy__serialize_and_memcpy
#else /* CONFIG_NO_THREADS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__serialize_and_wordcopy_atomic16(ProxyObject *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__serialize_and_wordcopy_atomic32(ProxyObject *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
#if __SIZEOF_POINTER__ >= 8
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__serialize_and_wordcopy_atomic64(ProxyObject *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
#endif /* __SIZEOF_POINTER__ >= 8 */
#endif /* !CONFIG_NO_THREADS */

typedef struct {
	PROXY_OBJECT_HEAD(po_obj) /* [1..1] Wrapped object */
	void             *po_ptr; /* [0..1][lock(ATOMIC)] Pointer into some struct kept alive by "mo_obj" */
} ProxyObjectWithPointer;
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy_with_xpointer__serialize_atomic(ProxyObjectWithPointer *__restrict self, struct Dee_serial *__restrict writer, Dee_seraddr_t addr);
#define generic_proxy_with_pointer__serialize        generic_proxy_with_xpointer__serialize_atomic
#define generic_proxy_with_xpointer__serialize       generic_proxy_with_xpointer__serialize_atomic
#define generic_proxy_with_pointer__serialize_atomic generic_proxy_with_xpointer__serialize_atomic
#define generic_proxy_with_funcpointer__serialize    generic_proxy_with_xpointer__serialize_atomic
#define generic_proxy_with_xfuncpointer__serialize   generic_proxy_with_xpointer__serialize_atomic


#define _generic_proxy__serialize_and_wordcopy_atomic_N1 generic_proxy__serialize_and_wordcopy_atomic8
#define _generic_proxy__serialize_and_wordcopy_atomic_N2 generic_proxy__serialize_and_wordcopy_atomic16
#define _generic_proxy__serialize_and_wordcopy_atomic_N4 generic_proxy__serialize_and_wordcopy_atomic32
#if __SIZEOF_POINTER__ >= 8
#define _generic_proxy__serialize_and_wordcopy_atomic_N8 generic_proxy__serialize_and_wordcopy_atomic64
#endif /* __SIZEOF_POINTER__ >= 8 */
#define _generic_proxy__serialize_and_wordcopy_atomic_N(word_size) \
	_generic_proxy__serialize_and_wordcopy_atomic_N##word_size
#define generic_proxy__serialize_and_wordcopy_atomic(word_size) \
	_generic_proxy__serialize_and_wordcopy_atomic_N(word_size)



STATIC_ASSERT_MSG(offsetof(ProxyObject2, po_obj1) == offsetof(ProxyObject, po_obj),
                  "You're allowed to use everything below with `ProxyObject2', "
                  /**/ "and have the runtime only make use of `po_obj1'");

INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__bool(ProxyObject *__restrict self);                         /* DeeObject_Bool(self->po_obj) */
INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy__iter_advance(ProxyObject *__restrict self, size_t step); /* DeeObject_IterAdvance(self->po_obj, step) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__iter_next(ProxyObject *__restrict self);        /* DeeObject_IterNext(self->po_obj) */

INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy__size_fast(ProxyObject *self); /* DeeObject_SizeFast(self->po_obj) */

INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__hasattr_string_hash(ProxyObject *self, char const *attr, Dee_hash_t hash);                       /* DeeObject_HasAttrStringHash(self->po_obj, attr, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__boundattr_string_hash(ProxyObject *self, char const *attr, Dee_hash_t hash);                     /* DeeObject_BoundAttrStringHash(self->po_obj, attr, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__hasattr_string_len_hash(ProxyObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);   /* DeeObject_HasAttrStringLenHash(self->po_obj, attr, attrlen, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__boundattr_string_len_hash(ProxyObject *self, char const *attr, size_t attrlen, Dee_hash_t hash); /* DeeObject_BoundAttrStringLenHash(self->po_obj, attr, attrlen, hash) */

/*[[[deemon
import printProxyObjectMethodHintWrapper from "..method-hints.method-hints";
printProxyObjectMethodHintWrapper("seq_operator_bool");
printProxyObjectMethodHintWrapper("seq_operator_size");
printProxyObjectMethodHintWrapper("seq_operator_sizeob");
printProxyObjectMethodHintWrapper("seq_operator_bounditem");
printProxyObjectMethodHintWrapper("seq_operator_bounditem_index");
printProxyObjectMethodHintWrapper("seq_operator_hasitem");
printProxyObjectMethodHintWrapper("seq_operator_hasitem_index");
printProxyObjectMethodHintWrapper("seq_operator_delitem");
printProxyObjectMethodHintWrapper("seq_operator_delrange");
printProxyObjectMethodHintWrapper("seq_operator_delitem_index");
printProxyObjectMethodHintWrapper("seq_operator_delrange_index");
printProxyObjectMethodHintWrapper("seq_operator_delrange_index_n");
printProxyObjectMethodHintWrapper("seq_operator_setitem");
printProxyObjectMethodHintWrapper("seq_operator_setitem_index");
printProxyObjectMethodHintWrapper("seq_operator_setrange");
printProxyObjectMethodHintWrapper("seq_operator_setrange_index");
printProxyObjectMethodHintWrapper("seq_operator_setrange_index_n");
printProxyObjectMethodHintWrapper("seq_operator_contains");
printProxyObjectMethodHintWrapper("seq_clear");
printProxyObjectMethodHintWrapper("seq_contains");
printProxyObjectMethodHintWrapper("seq_boundfirst");
printProxyObjectMethodHintWrapper("seq_delfirst");
printProxyObjectMethodHintWrapper("seq_boundlast");
printProxyObjectMethodHintWrapper("seq_dellast");
printProxyObjectMethodHintWrapper("set_operator_iter");
printProxyObjectMethodHintWrapper("set_operator_size");
printProxyObjectMethodHintWrapper("set_operator_sizeob");
printProxyObjectMethodHintWrapper("map_operator_getitem");
printProxyObjectMethodHintWrapper("map_operator_delitem");
printProxyObjectMethodHintWrapper("map_operator_setitem");
printProxyObjectMethodHintWrapper("map_operator_hasitem");
printProxyObjectMethodHintWrapper("map_operator_bounditem");
printProxyObjectMethodHintWrapper("map_operator_getitem_string_hash");
printProxyObjectMethodHintWrapper("map_operator_delitem_string_hash");
printProxyObjectMethodHintWrapper("map_operator_setitem_string_hash");
printProxyObjectMethodHintWrapper("map_operator_hasitem_string_hash");
printProxyObjectMethodHintWrapper("map_operator_bounditem_string_hash");
printProxyObjectMethodHintWrapper("map_operator_getitem_string_len_hash");
printProxyObjectMethodHintWrapper("map_operator_delitem_string_len_hash");
printProxyObjectMethodHintWrapper("map_operator_setitem_string_len_hash");
printProxyObjectMethodHintWrapper("map_operator_hasitem_string_len_hash");
printProxyObjectMethodHintWrapper("map_operator_bounditem_string_len_hash");
printProxyObjectMethodHintWrapper("map_operator_contains");
printProxyObjectMethodHintWrapper("map_operator_size");
printProxyObjectMethodHintWrapper("map_operator_sizeob");
printProxyObjectMethodHintWrapper("map_iterkeys");
printProxyObjectMethodHintWrapper("map_itervalues");
printProxyObjectMethodHintWrapper("map_remove");
printProxyObjectMethodHintWrapper("map_removekeys");
]]]*/
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_operator_bool(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_operator_bool, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy__seq_operator_size(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_operator_size, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__seq_operator_sizeob(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_operator_sizeob, self->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__seq_operator_bounditem(ProxyObject *self, DeeObject *index); /* DeeObject_InvokeMethodHint(seq_operator_bounditem, self->po_obj, index) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_operator_bounditem_index(ProxyObject *__restrict self, size_t index); /* DeeObject_InvokeMethodHint(seq_operator_bounditem_index, self->po_obj, index) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__seq_operator_hasitem(ProxyObject *self, DeeObject *index); /* DeeObject_InvokeMethodHint(seq_operator_hasitem, self->po_obj, index) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_operator_hasitem_index(ProxyObject *__restrict self, size_t index); /* DeeObject_InvokeMethodHint(seq_operator_hasitem_index, self->po_obj, index) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__seq_operator_delitem(ProxyObject *self, DeeObject *index); /* DeeObject_InvokeMethodHint(seq_operator_delitem, self->po_obj, index) */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_proxy__seq_operator_delrange(ProxyObject *self, DeeObject *start, DeeObject *end); /* DeeObject_InvokeMethodHint(seq_operator_delrange, self->po_obj, start, end) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_operator_delitem_index(ProxyObject *__restrict self, size_t index); /* DeeObject_InvokeMethodHint(seq_operator_delitem_index, self->po_obj, index) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_operator_delrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end); /* DeeObject_InvokeMethodHint(seq_operator_delrange_index, self->po_obj, start, end) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_operator_delrange_index_n(ProxyObject *self, Dee_ssize_t start); /* DeeObject_InvokeMethodHint(seq_operator_delrange_index_n, self->po_obj, start) */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_proxy__seq_operator_setitem(ProxyObject *self, DeeObject *index, DeeObject *value); /* DeeObject_InvokeMethodHint(seq_operator_setitem, self->po_obj, index, value) */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL generic_proxy__seq_operator_setitem_index(ProxyObject *self, size_t index, DeeObject *value); /* DeeObject_InvokeMethodHint(seq_operator_setitem_index, self->po_obj, index, value) */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL generic_proxy__seq_operator_setrange(ProxyObject *self, DeeObject *start, DeeObject *end, DeeObject *items); /* DeeObject_InvokeMethodHint(seq_operator_setrange, self->po_obj, start, end, items) */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL generic_proxy__seq_operator_setrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items); /* DeeObject_InvokeMethodHint(seq_operator_setrange_index, self->po_obj, start, end, items) */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL generic_proxy__seq_operator_setrange_index_n(ProxyObject *self, Dee_ssize_t start, DeeObject *items); /* DeeObject_InvokeMethodHint(seq_operator_setrange_index_n, self->po_obj, start, items) */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy__seq_operator_contains(ProxyObject *self, DeeObject *item); /* DeeObject_InvokeMethodHint(seq_operator_contains, self->po_obj, item) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_clear(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_clear, self->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__seq_contains(ProxyObject *self, DeeObject *item); /* DeeObject_InvokeMethodHint(seq_contains, self->po_obj, item) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_boundfirst(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_boundfirst, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_delfirst(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_delfirst, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_boundlast(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_boundlast, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__seq_dellast(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(seq_dellast, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__set_operator_iter(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(set_operator_iter, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy__set_operator_size(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(set_operator_size, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__set_operator_sizeob(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(set_operator_sizeob, self->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy__map_operator_getitem(ProxyObject *self, DeeObject *key); /* DeeObject_InvokeMethodHint(map_operator_getitem, self->po_obj, key) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_operator_delitem(ProxyObject *self, DeeObject *key); /* DeeObject_InvokeMethodHint(map_operator_delitem, self->po_obj, key) */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL generic_proxy__map_operator_setitem(ProxyObject *self, DeeObject *key, DeeObject *value); /* DeeObject_InvokeMethodHint(map_operator_setitem, self->po_obj, key, value) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_operator_hasitem(ProxyObject *self, DeeObject *key); /* DeeObject_InvokeMethodHint(map_operator_hasitem, self->po_obj, key) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_operator_bounditem(ProxyObject *self, DeeObject *key); /* DeeObject_InvokeMethodHint(map_operator_bounditem, self->po_obj, key) */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy__map_operator_getitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_getitem_string_hash, self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_operator_delitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_delitem_string_hash, self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL generic_proxy__map_operator_setitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash, DeeObject *value); /* DeeObject_InvokeMethodHint(map_operator_setitem_string_hash, self->po_obj, key, hash, value) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_operator_hasitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_hasitem_string_hash, self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_operator_bounditem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_bounditem_string_hash, self->po_obj, key, hash) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__map_operator_getitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_getitem_string_len_hash, self->po_obj, key, keylen, hash) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__map_operator_delitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_delitem_string_len_hash, self->po_obj, key, keylen, hash) */
INTDEF WUNUSED NONNULL((1, 5)) int DCALL generic_proxy__map_operator_setitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value); /* DeeObject_InvokeMethodHint(map_operator_setitem_string_len_hash, self->po_obj, key, keylen, hash, value) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__map_operator_hasitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_hasitem_string_len_hash, self->po_obj, key, keylen, hash) */
INTDEF WUNUSED NONNULL((1)) int DCALL generic_proxy__map_operator_bounditem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash); /* DeeObject_InvokeMethodHint(map_operator_bounditem_string_len_hash, self->po_obj, key, keylen, hash) */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_proxy__map_operator_contains(ProxyObject *self, DeeObject *key); /* DeeObject_InvokeMethodHint(map_operator_contains, self->po_obj, key) */
INTDEF WUNUSED NONNULL((1)) size_t DCALL generic_proxy__map_operator_size(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(map_operator_size, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__map_operator_sizeob(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(map_operator_sizeob, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__map_iterkeys(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(map_iterkeys, self->po_obj) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_proxy__map_itervalues(ProxyObject *__restrict self); /* DeeObject_InvokeMethodHint(map_itervalues, self->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_remove(ProxyObject *self, DeeObject *key); /* DeeObject_InvokeMethodHint(map_remove, self->po_obj, key) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__map_removekeys(ProxyObject *self, DeeObject *keys); /* DeeObject_InvokeMethodHint(map_removekeys, self->po_obj, keys) */
/*[[[end]]]*/

/*INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_proxy__hash_id(ProxyObject *self);*/

INTDEF struct type_cmp generic_proxy__cmp_recursive;
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_proxy__hash_recursive(ProxyObject *__restrict self);               /* DeeObject_Hash(self->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__trycompare_eq_recursive(ProxyObject *self, ProxyObject *other); /* DeeObject_TryCompareEq(self->po_obj, other->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__compare_eq_recursive(ProxyObject *self, ProxyObject *other);    /* DeeObject_CompareEq(self->po_obj, other->po_obj) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy__compare_recursive(ProxyObject *self, ProxyObject *other);       /* DeeObject_Compare(self->po_obj, other->po_obj) */

INTDEF struct type_cmp generic_proxy2__cmp_recursive_ordered;
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL generic_proxy2__hash_recursive_ordered(ProxyObject2 *__restrict self);      /* Dee_HashCombine(DeeObject_Hash(po_obj1), DeeObject_Hash(po_obj2)) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2__trycompare_eq_recursive(ProxyObject2 *self, ProxyObject2 *other); /* DeeObject_TryCompareEq(self->po_obj1, other->po_obj1) ?: DeeObject_TryCompareEq(self->po_obj2, other->po_obj2) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_proxy2__compare_eq_recursive(ProxyObject2 *self, ProxyObject2 *other);    /* DeeObject_CompareEq(self->po_obj1, other->po_obj1) ?: DeeObject_CompareEq(self->po_obj2, other->po_obj2) */


/* Wrap the given object "self" as a Sequence, Set, or Mapping */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_obj__asseq(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_obj__asset(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL generic_obj__asmap(DeeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_GENERIC_PROXY_H */
