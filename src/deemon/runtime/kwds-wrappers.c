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
#ifndef GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_C
#define GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>

#include "../objects/generic-proxy.h"
#include "kwlist.h"
#include "runtime_error.h"
#include "strings.h"

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

/* ======================== DeeBlackListKwdsObject ======================== */

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeBlackListKwdsObject, blki_map); /* [1..1][const] The associated keywords mapping. */
	DWEAK struct kwds_entry                     *blki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry                           *blki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
} DeeBlackListKwdsIterator;

INTDEF DeeTypeObject DeeBlackListKwdsIterator_Type;

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#define BLVI_GETITER(self) atomic_read(&(self)->blki_iter)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blvi_copy(DeeBlackListKwdsIterator *__restrict self,
          DeeBlackListKwdsIterator *__restrict other) {
	self->blki_iter = BLVI_GETITER(other);
	self->blki_end  = other->blki_end;
	self->blki_map  = other->blki_map;
	Dee_Incref(self->blki_map);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blvi_deep(DeeBlackListKwdsIterator *__restrict self,
          DeeBlackListKwdsIterator *__restrict other) {
	self->blki_iter = BLVI_GETITER(other);
	self->blki_end  = other->blki_end;
	self->blki_map  = (DREF DeeBlackListKwdsObject *)DeeObject_DeepCopy((DeeObject *)other->blki_map);
	if unlikely(!self->blki_map)
		goto err;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(DeeBlackListKwdsIterator, blki_map) == offsetof(ProxyObject, po_obj));
#define blvi_fini  generic_proxy__fini
#define blvi_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) struct kwds_entry *DCALL
blvi_nextiter(DeeBlackListKwdsIterator *__restrict self) {
	struct kwds_entry *iter;
	struct kwds_entry *old_iter;
again:
	iter     = BLVI_GETITER(self);
	old_iter = iter;
	for (;;) {
		if (iter >= self->blki_end)
			goto nope;
		if (iter->ke_name) {
			/* Skip values which have been black-listed. */
			if (!DeeBlackListKwds_IsBlackListed(self->blki_map, (DeeObject *)iter->ke_name))
				break;
		}
		++iter;
	}
	if (!atomic_cmpxch_weak_or_write(&self->blki_iter, old_iter, iter + 1))
		goto again;
	return iter;
nope:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blvi_nextpair(DeeBlackListKwdsIterator *__restrict self,
              DREF DeeObject *key_and_value[2]) {
	DREF DeeObject *value;
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return 1;
	DeeBlackListKwds_LockRead(self->blki_map);
	if unlikely(!self->blki_map->blkd_argv) {
		DeeBlackListKwds_LockEndRead(self->blki_map);
		return 1;
	}
	value = self->blki_map->blkd_argv[ent->ke_index];
	Dee_Incref(value);
	DeeBlackListKwds_LockEndRead(self->blki_map);
	Dee_Incref(ent->ke_name);
	key_and_value[0] = (DREF DeeObject *)ent->ke_name;
	key_and_value[1] = value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
blvi_nextkey(DeeBlackListKwdsIterator *__restrict self) {
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return (DREF DeeStringObject *)ITER_DONE;
	return_reference_(ent->ke_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blvi_nextvalue(DeeBlackListKwdsIterator *__restrict self) {
	DREF DeeObject *result;
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return ITER_DONE;
	DeeBlackListKwds_LockRead(self->blki_map);
	if unlikely(!self->blki_map->blkd_argv) {
		DeeBlackListKwds_LockEndRead(self->blki_map);
		return ITER_DONE;
	}
	result = self->blki_map->blkd_argv[ent->ke_index];
	Dee_Incref(result);
	DeeBlackListKwds_LockEndRead(self->blki_map);
	return result;
}

PRIVATE struct type_iterator blvi_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&blvi_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blvi_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blvi_nextvalue,
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextkey),
};


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
blvi_hash(DeeBlackListKwdsIterator *self) {
	return Dee_HashPointer(BLVI_GETITER(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blvi_compare(DeeBlackListKwdsIterator *self, DeeBlackListKwdsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &DeeBlackListKwdsIterator_Type))
		goto err;
	Dee_return_compareT(struct kwds_entry *, BLVI_GETITER(self),
	                    /*                */ BLVI_GETITER(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp blvi_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&blvi_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&blvi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_member tpconst blvi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DeeBlackListKwdsIterator, blki_map),
	                      "->?Ert:BlackListKwds"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeBlackListKwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListKwdsIterator",
	/* .tp_doc      = */ DOC("next->?T2?Dstring?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&blvi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&blvi_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeBlackListKwdsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blvi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ NULL /* XXX: Could easily be implemented... */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blvi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &blvi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &blvi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ blvi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};





INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKwds_IsBlackListed(DeeBlackListKwdsObject *__restrict self,
                               /*string*/ DeeObject *__restrict name) {
	Dee_hash_t i, perturb;
	DeeStringObject *str;
	Dee_hash_t hash;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
again:
	i = perturb = hash & self->blkd_mask;
	for (;; DeeBlackListKwds_BLCKNEXT(i, perturb)) {
		str = self->blkd_blck[i & self->blkd_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_SIZE((DeeObject *)str) != DeeString_SIZE(name))
			continue;
		if (bcmpc(DeeString_STR(str),
		          DeeString_STR(name),
		          DeeString_SIZE(name),
		          sizeof(char)) != 0)
			continue;
		return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blkd_load) < self->blkd_ckwc) {
		size_t index;
		DeeBlackListKwds_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blkd_load);
		while (index < self->blkd_ckwc) {
			Dee_hash_t hashof_str;
			str = self->blkd_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blkd_mask;
			for (;; DeeBlackListKwds_BLCKNEXT(i, perturb)) {
				DeeBlackListKwdsEntry *ent;
				ent = &self->blkd_blck[i & self->blkd_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash &&
			    DeeString_SIZE(str) == DeeString_SIZE(name) &&
			    bcmpc(DeeString_STR(str), DeeString_STR(name),
			          DeeString_SIZE(name), sizeof(char)) == 0) {
				self->blkd_load = index;
				DeeBlackListKwds_LockEndWrite(self);
				return true;
			}
		}
		self->blkd_load = index;
		DeeBlackListKwds_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKwds_IsBlackListedStringHash(DeeBlackListKwdsObject *__restrict self,
                                         char const *__restrict name,
                                         Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blkd_mask;
	for (;; DeeBlackListKwds_BLCKNEXT(i, perturb)) {
		str = self->blkd_blck[i & self->blkd_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (strcmp(DeeString_STR(str), name) != 0)
			continue;
		return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blkd_load) < self->blkd_ckwc) {
		size_t index;
		DeeBlackListKwds_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blkd_load);
		while (index < self->blkd_ckwc) {
			Dee_hash_t hashof_str;
			str = self->blkd_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blkd_mask;
			for (;; DeeBlackListKwds_BLCKNEXT(i, perturb)) {
				DeeBlackListKwdsEntry *ent;
				ent = &self->blkd_blck[i & self->blkd_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash &&
			    strcmp(DeeString_STR(str), name) == 0) {
				self->blkd_load = index;
				DeeBlackListKwds_LockEndWrite(self);
				return true;
			}
		}
		self->blkd_load = index;
		DeeBlackListKwds_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKwds_IsBlackListedStringLenHash(DeeBlackListKwdsObject *__restrict self,
                                            char const *__restrict name,
                                            size_t namelen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blkd_mask;
	for (;; DeeBlackListKwds_BLCKNEXT(i, perturb)) {
		str = self->blkd_blck[i & self->blkd_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_EqualsBuf(str, name, namelen))
			return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blkd_load) < self->blkd_ckwc) {
		size_t index;
		DeeBlackListKwds_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blkd_load);
		while (index < self->blkd_ckwc) {
			Dee_hash_t hashof_str;
			str = self->blkd_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blkd_mask;
			for (;; DeeBlackListKwds_BLCKNEXT(i, perturb)) {
				DeeBlackListKwdsEntry *ent;
				ent = &self->blkd_blck[i & self->blkd_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash && DeeString_SIZE(str) == namelen &&
			    bcmpc(DeeString_STR(str), name, namelen, sizeof(char)) == 0) {
				self->blkd_load = index;
				DeeBlackListKwds_LockEndWrite(self);
				return true;
			}
		}
		self->blkd_load = index;
		DeeBlackListKwds_LockEndWrite(self);
		goto again;
	}
	return false;
}

PRIVATE NONNULL((1)) void DCALL
blv_fini(DeeBlackListKwdsObject *__restrict self) {
	size_t argc = DeeKwds_SIZE(self->blkd_kwds);
	Dee_Decrefv(self->blkd_argv, argc);
	Dee_Decref(self->blkd_code);
	Dee_Decref(self->blkd_kwds);
}

PRIVATE NONNULL((1, 2)) void DCALL
blv_visit(DeeBlackListKwdsObject *__restrict self, dvisit_t proc, void *arg) {
	size_t argc;
	DeeBlackListKwds_LockRead(self);
	argc = DeeKwds_SIZE(self->blkd_kwds);
	Dee_Visitv(self->blkd_argv, argc);
	DeeBlackListKwds_LockEndRead(self);
	Dee_Visit(self->blkd_code);
	Dee_Visit(self->blkd_kwds);
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
blv_bool(DeeBlackListKwdsObject *__restrict self) {
	size_t i;
	DeeKwdsObject *kw = self->blkd_kwds;
	for (i = 0; i <= kw->kw_mask; ++i) {
		DeeStringObject *name;
		name = kw->kw_map[i].ke_name;
		if (!name)
			continue;

		/* Check if this keyword has been black-listed. */
		if (!DeeBlackListKwds_IsBlackListed(self, (DeeObject *)name))
			return 1; /* non-empty. */
	}
	return 0; /* empty */
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
blv_size(DeeBlackListKwdsObject *__restrict self) {
	size_t i, result = 0;
	DeeKwdsObject *kw = self->blkd_kwds;
	for (i = 0; i <= kw->kw_mask; ++i) {
		DeeStringObject *name;
		name = kw->kw_map[i].ke_name;
		if (!name)
			continue;

		/* Check if this keyword has been black-listed. */
		if (!DeeBlackListKwds_IsBlackListed(self, (DeeObject *)name))
			++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwdsIterator *DCALL
blv_iter(DeeBlackListKwdsObject *__restrict self) {
	DREF DeeBlackListKwdsIterator *result;
	result = DeeObject_MALLOC(DeeBlackListKwdsIterator);
	if unlikely(!result)
		goto done;
	result->blki_iter = self->blkd_kwds->kw_map;
	result->blki_end  = result->blki_iter + self->blkd_kwds->kw_mask + 1;
	result->blki_map  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeBlackListKwdsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_contains(DeeBlackListKwdsObject *self, DeeObject *key) {
	size_t index;
	if unlikely(!DeeString_Check(key))
		goto nope;
	index = DeeKwds_IndexOf((DeeObject *)self->blkd_kwds, key);
	if (index == (size_t)-1)
		goto nope;
	if (DeeBlackListKwds_IsBlackListed(self, key))
		goto nope;
	return_true;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blv_getitemnr(DeeBlackListKwdsObject *__restrict self,
              /*string*/ DeeObject *__restrict name) {
	DeeObject *result;
	size_t index = DeeKwds_IndexOf((DeeObject *)self->blkd_kwds, name);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(DeeBlackListKwds_IsBlackListed(self, name))
		goto missing;
	DeeBlackListKwds_LockRead(self);
	result = self->blkd_argv[index];
	DeeBlackListKwds_LockEndRead(self);
	return result;
missing:
	err_unknown_key((DeeObject *)self, name);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blv_getitemnr_string_hash(DeeBlackListKwdsObject *__restrict self,
                          char const *__restrict name, Dee_hash_t hash) {
	DeeObject *result;
	size_t index = DeeKwds_IndexOfStringHash((DeeObject *)self->blkd_kwds, name, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(DeeBlackListKwds_IsBlackListedStringHash(self, name, hash))
		goto missing;
	DeeBlackListKwds_LockRead(self);
	result = self->blkd_argv[index];
	DeeBlackListKwds_LockEndRead(self);
	return result;
missing:
	err_unknown_key_str((DeeObject *)self, name);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blv_getitemnr_string_len_hash(DeeBlackListKwdsObject *__restrict self,
                              char const *__restrict name,
                              size_t namelen, Dee_hash_t hash) {
	DeeObject *result;
	size_t index = DeeKwds_IndexOfStringLenHash((DeeObject *)self->blkd_kwds, name, namelen, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(DeeBlackListKwds_IsBlackListedStringLenHash(self, name, namelen, hash))
		goto missing;
	DeeBlackListKwds_LockRead(self);
	result = self->blkd_argv[index];
	DeeBlackListKwds_LockEndRead(self);
	return result;
missing:
	err_unknown_key_str_len((DeeObject *)self, name, namelen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blv_trygetitemnr(DeeBlackListKwdsObject *__restrict self,
                 /*string*/ DeeObject *__restrict name) {
	DeeObject *result;
	size_t index = DeeKwds_IndexOf((DeeObject *)self->blkd_kwds, name);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(DeeBlackListKwds_IsBlackListed(self, name))
		goto missing;
	DeeBlackListKwds_LockRead(self);
	result = self->blkd_argv[index];
	DeeBlackListKwds_LockEndRead(self);
	return result;
missing:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blv_trygetitemnr_string_hash(DeeBlackListKwdsObject *__restrict self,
                             char const *__restrict name, Dee_hash_t hash) {
	DeeObject *result;
	size_t index = DeeKwds_IndexOfStringHash((DeeObject *)self->blkd_kwds, name, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(DeeBlackListKwds_IsBlackListedStringHash(self, name, hash))
		goto missing;
	DeeBlackListKwds_LockRead(self);
	result = self->blkd_argv[index];
	DeeBlackListKwds_LockEndRead(self);
	return result;
missing:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blv_trygetitemnr_string_len_hash(DeeBlackListKwdsObject *__restrict self,
                                 char const *__restrict name,
                                 size_t namelen, Dee_hash_t hash) {
	DeeObject *result;
	size_t index = DeeKwds_IndexOfStringLenHash((DeeObject *)self->blkd_kwds, name, namelen, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(DeeBlackListKwds_IsBlackListedStringLenHash(self, name, namelen, hash))
		goto missing;
	DeeBlackListKwds_LockRead(self);
	result = self->blkd_argv[index];
	DeeBlackListKwds_LockEndRead(self);
	return result;
missing:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_getitem(DeeBlackListKwdsObject *self, DeeObject *key) {
	DREF DeeObject *result;
	if unlikely(!DeeString_Check(key)) {
		err_unknown_key((DeeObject *)self, key);
		return NULL;
	}
	result = blv_getitemnr(self, key);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_getitem_string_hash(DeeBlackListKwdsObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = blv_getitemnr_string_hash(self, key, hash);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_getitem_string_len_hash(DeeBlackListKwdsObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = blv_getitemnr_string_len_hash(self, key, keylen, hash);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_trygetitem(DeeBlackListKwdsObject *self, DeeObject *key) {
	DREF DeeObject *result;
	if unlikely(!DeeString_Check(key))
		return ITER_DONE;
	result = blv_trygetitemnr(self, key);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_trygetitem_string_hash(DeeBlackListKwdsObject *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = blv_trygetitemnr_string_hash(self, key, hash);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_trygetitem_string_len_hash(DeeBlackListKwdsObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = blv_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blv_hasitem(DeeBlackListKwdsObject *self, DeeObject *key) {
	size_t index;
	if unlikely(!DeeString_Check(key))
		goto nope;
	index = DeeKwds_IndexOf((DeeObject *)self->blkd_kwds, key);
	if (index == (size_t)-1)
		goto nope;
	if (DeeBlackListKwds_IsBlackListed(self, key))
		goto nope;
	return 1;
nope:
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blv_hasitem_string_hash(DeeBlackListKwdsObject *self, char const *key, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash((DeeObject *)self->blkd_kwds, key, hash);
	if (index == (size_t)-1)
		goto nope;
	if (DeeBlackListKwds_IsBlackListedStringHash(self, key, hash))
		goto nope;
	return 1;
nope:
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blv_hasitem_string_len_hash(DeeBlackListKwdsObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash((DeeObject *)self->blkd_kwds, key, keylen, hash);
	if (index == (size_t)-1)
		goto nope;
	if (DeeBlackListKwds_IsBlackListedStringLenHash(self, key, keylen, hash))
		goto nope;
	return 1;
nope:
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
blv_foreach_pair(DeeBlackListKwdsObject *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_hash_t i;
	Dee_ssize_t temp, result = 0;
	DeeKwdsObject *kwds = self->blkd_kwds;
	for (i = 0; i <= kwds->kw_mask; ++i) {
		DeeObject *value;
		struct kwds_entry *entry = &kwds->kw_map[i];
		if (!entry->ke_name)
			continue;
		if (DeeBlackListKwds_IsBlackListed(self, (DeeObject *)entry->ke_name))
			continue;
		DeeBlackListKwds_LockRead(self);
		value = self->blkd_argv[entry->ke_index];
		DeeBlackListKwds_LockEndRead(self);
		temp = (*proc)(arg, (DeeObject *)entry->ke_name, value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_seq blv_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blv_iter,
	/* .tp_sizeob                       = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blv_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blv_getitem,
	/* .tp_delitem                      = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                      = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                     = */ NULL,
	/* .tp_delrange                     = */ NULL,
	/* .tp_setrange                     = */ NULL,
	/* .tp_foreach                      = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&blv_foreach_pair,
	/* .tp_enumerate                    = */ NULL,
	/* .tp_enumerate_index              = */ NULL,
	/* .tp_iterkeys                     = */ NULL,
	/* .tp_bounditem                    = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&blv_hasitem,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&blv_size,
	/* .tp_size_fast                    = */ NULL, /*(size_t (DCALL *)(DeeObject *__restrict))&blv_size,*/ /* Does not run in O(1) */
	/* .tp_getitem_index                = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index                = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index              = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index                = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index               = */ NULL,
	/* .tp_delrange_index               = */ NULL,
	/* .tp_setrange_index               = */ NULL,
	/* .tp_getrange_index_n             = */ NULL,
	/* .tp_delrange_index_n             = */ NULL,
	/* .tp_setrange_index_n             = */ NULL,
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blv_trygetitem,
	/* .tp_trygetitem_index             = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&blv_trygetitem_string_hash,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&blv_getitem_string_hash,
	/* .tp_delitem_string_hash          = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash          = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash        = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&blv_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&blv_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&blv_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash      = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash    = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&blv_hasitem_string_len_hash,
	/* .tp_asvector                     = */ NULL,
	/* .tp_asvector_nothrow             = */ NULL,
	/* .tp_unpack                       = */ NULL,
	/* .tp_unpack_ex                    = */ NULL,
	/* .tp_unpack_ub                    = */ NULL,
	/* .tp_getitemnr                    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&blv_getitemnr,
	/* .tp_getitemnr_string_hash        = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&blv_getitemnr_string_hash,
	/* .tp_getitemnr_string_len_hash    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&blv_getitemnr_string_len_hash,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&blv_trygetitemnr,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&blv_trygetitemnr_string_hash,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&blv_trygetitemnr_string_len_hash,
};

PRIVATE struct type_getset tpconst blv_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst blv_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeBlackListKwdsIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeBlackListKwds_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED DREF DeeBlackListKwdsObject *DCALL
blv_init_kw(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeCodeObject *code;
	DeeTupleObject *kwargs;
	DeeKwdsObject *kwds;
	size_t positional;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__code_positional_kwargs_kwds,
	                    "o" UNPuSIZ "oo:_BlackListKwds",
	                    &code, &positional, &kwargs, &kwds))
		goto err;
	if (DeeObject_AssertTypeExact(code, &DeeCode_Type))
		goto err;
	if (DeeObject_AssertTypeExact(kwargs, &DeeTuple_Type))
		goto err;
	if (DeeObject_AssertTypeExact(kwds, &DeeKwds_Type))
		goto err;

	/* Validate arguments. */
	if unlikely(DeeKwds_SIZE(kwds) <= 0) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Empty keyword list given");
		goto err;
	}
	if unlikely(DeeKwds_SIZE(kwds) != DeeTuple_SIZE(kwargs)) {
		DeeError_Throwf(&DeeError_ValueError,
		                "`kwargs' tuple has %" PRFuSIZ " elements when "
		                "`kwds' map requires exactly %" PRFuSIZ " values",
		                DeeTuple_SIZE(kwargs), DeeKwds_SIZE(kwds));
		goto err;
	}
	if unlikely(!code->co_keywords) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Code object does not define keyword arguments");
		goto err;
	}
	if unlikely(code->co_argc_max <= positional) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Code object takes at most %" PRFu16 " positional "
		                "arguments when %" PRFuSIZ " were supposedly given",
		                code->co_argc_max, positional);
		goto err;
	}

	result = DeeBlackListKwds_New(code, positional, DeeTuple_ELEM(kwargs), kwds);
	if unlikely(!result)
		goto err;
	/* Force unshare (hacky, but works and doesn't require extra code) */
	Dee_Incref(result);
	DeeBlackListKwds_Decref(result);
	return (DREF DeeBlackListKwdsObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwdsObject *DCALL
blv_copy(DeeBlackListKwdsObject *__restrict self) {
	size_t count = self->blkd_kwds->kw_size;
	size_t sizeof_blkd_blck = ((self->blkd_mask + 1) * sizeof(DeeBlackListKwdsEntry));
	DREF DeeBlackListKwdsObject *result;
	result = (DREF DeeBlackListKwdsObject *)DeeObject_Mallocc(offsetof(DeeBlackListKwdsObject, blkd_blck) +
	                                                          sizeof_blkd_blck,
	                                                          count, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->blkd_argv = (DREF DeeObject **)((byte_t *)result->blkd_blck + sizeof_blkd_blck);
	DeeBlackListKwds_LockRead(self);
	Dee_Movrefv(result->blkd_argv, self->blkd_argv, count);
	result->blkd_load = self->blkd_load;
	memcpyc(result->blkd_blck,
	        self->blkd_blck,
	        self->blkd_mask + 1,
	        sizeof(DeeBlackListKwdsEntry));
	DeeBlackListKwds_LockEndRead(self);
	Dee_atomic_rwlock_init(&result->blkd_lock);
	result->blkd_code = self->blkd_code;
	Dee_Incref(result->blkd_code);
	result->blkd_ckwc = self->blkd_ckwc;
	result->blkd_ckwv = self->blkd_ckwv;
	result->blkd_kwds = self->blkd_kwds;
	Dee_Incref(result->blkd_kwds);
	result->blkd_mask = self->blkd_mask;
	DeeObject_Init(result, &DeeBlackListKwds_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwdsObject *DCALL
blv_deep(DeeBlackListKwdsObject *__restrict self) {
	size_t count = self->blkd_kwds->kw_size;
	size_t sizeof_blkd_blck = ((self->blkd_mask + 1) * sizeof(DeeBlackListKwdsEntry));
	DREF DeeBlackListKwdsObject *result;
	result = (DREF DeeBlackListKwdsObject *)DeeObject_Mallocc(offsetof(DeeBlackListKwdsObject, blkd_blck) +
	                                                          sizeof_blkd_blck,
	                                                          count, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->blkd_argv = (DREF DeeObject **)((byte_t *)result->blkd_blck + sizeof_blkd_blck);
	DeeBlackListKwds_LockRead(self);
	Dee_Movrefv(result->blkd_argv, self->blkd_argv, count);
	result->blkd_load = self->blkd_load;
	memcpyc(result->blkd_blck,
	        self->blkd_blck,
	        self->blkd_mask + 1,
	        sizeof(DeeBlackListKwdsEntry));
	DeeBlackListKwds_LockEndRead(self);

	/* Construct deep copies of all of the arguments. */
	if (DeeObject_InplaceDeepCopyv(result->blkd_argv, count))
		goto err_r_argv;
	Dee_atomic_rwlock_init(&result->blkd_lock);
	result->blkd_code = self->blkd_code;
	Dee_Incref(result->blkd_code);
	result->blkd_ckwc = self->blkd_ckwc;
	result->blkd_ckwv = self->blkd_ckwv;
	result->blkd_kwds = self->blkd_kwds;
	Dee_Incref(result->blkd_kwds);
	result->blkd_mask = self->blkd_mask;
	DeeObject_Init(result, &DeeBlackListKwds_Type);
done:
	return result;
err_r_argv:
	Dee_Decrefv(result->blkd_argv, count);
/*err_r:*/
	DeeObject_Free(result);
	return NULL;
}


PUBLIC DeeTypeObject DeeBlackListKwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListKwds",
	/* .tp_doc      = */ DOC("A ${{string: Object}}-like mapping that is used to exclude positional "
	                         /**/ "keyword arguments for a variable-keywords user-code function, when that "
	                         /**/ "function is invoked with regular keywords being passed:\n"
	                         "${"
	                         /**/ "function foo(a, **kwds) {\n"
	                         /**/ "	print type kwds; /* _BlackListKwds */\n"
	                         /**/ "}\n"
	                         /**/ "foo(10, b: 20);\n"
	                         /**/ "foo(a: 10, b: 20);"
	                         "}\n"
	                         "\n"
	                         "(code:?Ert:Code,positional:?Dint,kwargs:?DTuple,kwds:?Ert:Kwds)\n"
	                         "Construct a new ?. object that blacklists the first @positional "
	                         /**/ "arguments from @code. Keyword argument values are then taken "
	                         /**/ "from @kwargs and @kwds"
	),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)&blv_copy,
				/* .tp_deep_ctor   = */ (dfunptr_t)&blv_deep,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				/* .tp_free        = */ (dfunptr_t)NULL,
				/* .tp_alloc       = */ { (dfunptr_t)NULL },
				/* .tp_any_ctor_kw = */ (dfunptr_t)&blv_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&blv_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__667432E5904B49F8),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__40D3D60A1F18CAE2),
	/* .tp_seq           = */ &blv_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ blv_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ blv_class_members,
};


/* Construct a new mapping for keywords that follows the black-listing scheme.
 * The caller must decref the returned object using `DeeBlackListKwds_Decref()'
 * -> This function is used to filter keyword arguments from varkwds when 
 *    kwargs argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListKwds { "something_else" : "foobar" }'
 *    >> foo(x: 10, something_else: "foobar"); */
PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeBlackListKwds_New(struct code_object *__restrict code,
                     size_t positional_argc, DeeObject *const *kw_argv,
                     DeeKwdsObject *__restrict kwds) {
	DREF DeeBlackListKwdsObject *result;
	size_t mask, argc = code->co_argc_max;
	ASSERT(DeeKwds_SIZE(kwds) > 0);
	ASSERT(code->co_keywords);
	ASSERT(argc > positional_argc);

	/* Calculate an appropriate mask for the blacklist hash-set. */
	for (mask = 3; mask <= argc; mask = (mask << 1) | 1)
		;
	result = (DREF DeeBlackListKwdsObject *)DeeObject_Callocc(offsetof(DeeBlackListKwdsObject, blkd_blck) +
	                                                          ((mask + 1) * sizeof(DeeBlackListKwdsEntry)),
	                                                          kwds->kw_size, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	Dee_atomic_rwlock_cinit(&result->blkd_lock);
	result->blkd_code = code; /* Weakly referenced. */
	result->blkd_ckwc = argc - positional_argc;
	result->blkd_ckwv = code->co_keywords + positional_argc;
	result->blkd_kwds = kwds;                                     /* Weakly referenced. */
	result->blkd_argv = (DREF DeeObject **)(DeeObject **)kw_argv; /* Weakly referenced. */
	result->blkd_mask = mask;
	DeeObject_Init(result, &DeeBlackListKwds_Type);
done:
	return (DREF DeeObject *)result;
}


/* Unshare the argument vector from a blacklist-varkwds object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
PUBLIC NONNULL((1)) void DCALL
DeeBlackListKwds_Decref(DREF DeeObject *__restrict self) {
	DREF DeeBlackListKwdsObject *me;
	size_t kwdc, sizeof_blkd_blck;
	DREF DeeObject **argv;
	me = (DREF DeeBlackListKwdsObject *)self;
	if (!DeeObject_IsShared(me)) {
		/* Simple case: user-code didn't share the keyword mapping with its caller,
		 *              so our cleanup process has been greatly simplified. */
		/*Dee_Decref(me->blkd_code);*/ /* Not actually referenced */
		/*Dee_Decref(me->blkd_kwds);*/ /* Not actually referenced */
		Dee_DecrefNokill(&DeeBlackListKwds_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
		return;
	}

	/* Must transform the object such that it can continue to exist without causing problems. */
	sizeof_blkd_blck = ((me->blkd_mask + 1) * sizeof(DeeBlackListKwdsEntry));
	kwdc = DeeKwds_SIZE(me->blkd_kwds);
	argv = (DREF DeeObject **)((byte_t *)me->blkd_blck + sizeof_blkd_blck);
	argv = Dee_Movrefv(argv, me->blkd_argv, kwdc);

	/* Override the old argv such that the object holds its own copy. */
	DeeBlackListKwds_LockWrite(me);
	me->blkd_argv = argv; /* Inherit */
	DeeBlackListKwds_LockEndWrite(me);

	/* Construct references to pointed-to objects (done now, so we could
	 * skip that step within the `!DeeObject_IsShared(me)' path above) */
	Dee_Incref(me->blkd_code);
	Dee_Incref(me->blkd_kwds);

	/* Drop our own reference (which should still be shared right now) */
	Dee_Decref_unlikely(self);
}














/* ======================== DeeBlackListKwObject ======================== */
typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,            mi_iter, /* [1..1][const] An iterator for the underlying `mi_map->blkw_kw'. */
	                      DeeBlackListKwObject, mi_map); /* [1..1][const] The general-purpose blacklist mapping being iterated. */
} DeeBlackListKwIterator;

INTDEF DeeTypeObject DeeBlackListKwIterator_Type;

STATIC_ASSERT(offsetof(DeeBlackListKwIterator, mi_iter) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(DeeBlackListKwIterator, mi_map) == offsetof(ProxyObject2, po_obj2));
#define blmi_copy  generic_proxy2__copy_recursive1_alias2
#define blmi_fini  generic_proxy2__fini
#define blmi_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blmi_nextpair(DeeBlackListKwIterator *__restrict self,
              DREF DeeObject *key_and_value[2]) {
	int result;
again:
	result = DeeObject_IterNextPair(self->mi_iter, key_and_value);
	if (result == 0) {
		if (DeeString_Check(key_and_value[0]) &&
		    DeeBlackListKw_IsBlackListed(self->mi_map, key_and_value[0])) {
			Dee_Decref(key_and_value[0]);
			Dee_Decref(key_and_value[1]);
			goto again;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blmi_nextkey(DeeBlackListKwIterator *__restrict self) {
	DREF DeeObject *result;
again:
	result = DeeObject_IterNextKey(self->mi_iter);
	if (ITER_ISOK(result)) {
		if (DeeString_Check(result) &&
		    DeeBlackListKw_IsBlackListed(self->mi_map, result)) {
			Dee_Decref(result);
			goto again;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blmi_nextvalue(DeeBlackListKwIterator *__restrict self) {
	int result;
	DREF DeeObject *pair[2];
again:
	result = DeeObject_IterNextPair(self->mi_iter, pair);
	if unlikely(result != 0) {
		if (result > 0)
			return ITER_DONE;
		goto err;
	}
	if (DeeString_Check(pair[0]) &&
	    DeeBlackListKw_IsBlackListed(self->mi_map, pair[0])) {
		Dee_Decref(pair[1]);
		Dee_Decref(pair[0]);
		goto again;
	}
	Dee_Decref(pair[0]);
	return pair[1];
err:
	return NULL;
}

PRIVATE struct type_iterator blmi_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&blmi_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blmi_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blmi_nextvalue,
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextkey),
};

STATIC_ASSERT(offsetof(DeeBlackListKwIterator, mi_iter) == offsetof(ProxyObject, po_obj));
#define blmi_hash          generic_proxy__hash_recursive
#define blmi_compare       generic_proxy__compare_recursive
#define blmi_compare_eq    generic_proxy__compare_eq_recursive
#define blmi_trycompare_eq generic_proxy__trycompare_eq_recursive
#define blmi_cmp           generic_proxy__cmp_recursive

PRIVATE struct type_member tpconst blmi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DeeBlackListKwIterator, mi_map), "->?Ert:BlackListKw"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(DeeBlackListKwIterator, mi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeBlackListKwIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListKwIterator",
	/* .tp_doc      = */ DOC("next->?T2?Dstring?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&blmi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeBlackListKwIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blmi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blmi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &blmi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &blmi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ blmi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKw_IsBlackListed(DeeBlackListKwObject *__restrict self,
                             /*String*/ DeeObject *name) {
	Dee_hash_t i, perturb;
	DeeStringObject *str;
	Dee_hash_t hash;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
again:
	i = perturb = hash & self->blkw_mask;
	for (;; DeeBlackListKw_BLCKNEXT(i, perturb)) {
		str = self->blkw_blck[i & self->blkw_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_EqualsSTR(str, name))
			return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blkw_load) < self->blkw_ckwc) {
		size_t index;
		DeeBlackListKw_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blkw_load);
		while (index < self->blkw_ckwc) {
			Dee_hash_t hashof_str;
			str = self->blkw_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blkw_mask;
			for (;; DeeBlackListKw_BLCKNEXT(i, perturb)) {
				DeeBlackListKwdsEntry *ent;
				ent = &self->blkw_blck[i & self->blkw_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash &&
			    DeeString_SIZE(str) == DeeString_SIZE(name) &&
			    bcmpc(DeeString_STR(str), DeeString_STR(name),
			          DeeString_SIZE(name), sizeof(char)) == 0) {
				self->blkw_load = index;
				DeeBlackListKw_LockEndWrite(self);
				return true;
			}
		}
		self->blkw_load = index;
		DeeBlackListKw_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKw_IsBlackListedStringHash(DeeBlackListKwObject *__restrict self,
                                       char const *__restrict name,
                                       Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blkw_mask;
	for (;; DeeBlackListKw_BLCKNEXT(i, perturb)) {
		str = self->blkw_blck[i & self->blkw_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (strcmp(DeeString_STR(str), name) != 0)
			continue;
		return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blkw_load) < self->blkw_ckwc) {
		size_t index;
		DeeBlackListKw_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blkw_load);
		while (index < self->blkw_ckwc) {
			Dee_hash_t hashof_str;
			str = self->blkw_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blkw_mask;
			for (;; DeeBlackListKw_BLCKNEXT(i, perturb)) {
				DeeBlackListKwdsEntry *ent;
				ent = &self->blkw_blck[i & self->blkw_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash &&
			    strcmp(DeeString_STR(str), name) == 0) {
				self->blkw_load = index;
				DeeBlackListKw_LockEndWrite(self);
				return true;
			}
		}
		self->blkw_load = index;
		DeeBlackListKw_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKw_IsBlackListedStringLenHash(DeeBlackListKwObject *__restrict self,
                                          char const *__restrict name,
                                          size_t namelen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blkw_mask;
	for (;; DeeBlackListKw_BLCKNEXT(i, perturb)) {
		str = self->blkw_blck[i & self->blkw_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_EqualsBuf(str, name, namelen))
			return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blkw_load) < self->blkw_ckwc) {
		size_t index;
		DeeBlackListKw_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blkw_load);
		while (index < self->blkw_ckwc) {
			Dee_hash_t hashof_str;
			str = self->blkw_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blkw_mask;
			for (;; DeeBlackListKw_BLCKNEXT(i, perturb)) {
				DeeBlackListKwdsEntry *ent;
				ent = &self->blkw_blck[i & self->blkw_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash && DeeString_SIZE(str) == namelen &&
			    bcmpc(DeeString_STR(str), name, namelen, sizeof(char)) == 0) {
				self->blkw_load = index;
				DeeBlackListKw_LockEndWrite(self);
				return true;
			}
		}
		self->blkw_load = index;
		DeeBlackListKw_LockEndWrite(self);
		goto again;
	}
	return false;
}



PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blkw_getitemnr(DeeBlackListKwObject *__restrict self,
               /*string*/ DeeObject *__restrict name) {
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if likely(!DeeBlackListKw_IsBlackListed(self, name))
		return DeeKw_GetItemNR(DeeBlackListKw_KW(self), name);
	err_unknown_key((DeeObject *)self, name);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blkw_getitemnr_string_hash(DeeBlackListKwObject *__restrict self,
                           char const *__restrict name, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeKw_GetItemNRStringHash(DeeBlackListKw_KW(self), name, hash);
	err_unknown_key_str((DeeObject *)self, name);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blkw_getitemnr_string_len_hash(DeeBlackListKwObject *__restrict self,
                               char const *__restrict name,
                               size_t namelen, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeKw_GetItemNRStringLenHash(DeeBlackListKw_KW(self), name, namelen, hash);
	err_unknown_key_str_len((DeeObject *)self, name, namelen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blkw_trygetitemnr(DeeBlackListKwObject *__restrict self,
                  /*string*/ DeeObject *__restrict name) {
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if likely(!DeeBlackListKw_IsBlackListed(self, name))
		return DeeKw_TryGetItemNR(DeeBlackListKw_KW(self), name);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blkw_trygetitemnr_string_hash(DeeBlackListKwObject *__restrict self,
                              char const *__restrict name, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeKw_TryGetItemNRStringHash(DeeBlackListKw_KW(self), name, hash);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
blkw_trygetitemnr_string_len_hash(DeeBlackListKwObject *__restrict self,
                                  char const *__restrict name,
                                  size_t namelen, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeKw_TryGetItemNRStringLenHash(DeeBlackListKw_KW(self), name, namelen, hash);
	return ITER_DONE;
}

STATIC_ASSERT(offsetof(DeeBlackListKwObject, blkw_code) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DeeBlackListKwObject, blkw_code) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(DeeBlackListKwObject, blkw_kw) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DeeBlackListKwObject, blkw_kw) == offsetof(ProxyObject2, po_obj2));
#define blkw_fini  generic_proxy2__fini
#define blkw_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
blkw_bool_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	DeeBlackListKwObject *self = (DeeBlackListKwObject *)arg;
	(void)value;
	if (!DeeString_Check(key) ||
	    !DeeBlackListKw_IsBlackListed(self, key)) {
		Dee_Decref(key);
		return -2; /* Success indicator. */
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
blkw_bool(DeeBlackListKwObject *__restrict self) {
	int status = (int)DeeObject_ForeachPair(self->blkw_kw, &blkw_bool_foreach_cb, self);
	ASSERT(status == 0 || status == -1 || status == -2);
	if (status == -2)
		status = 1;
	return status;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
blkw_size_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	DeeBlackListKwObject *self = (DeeBlackListKwObject *)arg;
	(void)value;
	if (!DeeString_Check(key) ||
	    !DeeBlackListKw_IsBlackListed(self, key))
		return 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
blkw_size(DeeBlackListKwObject *__restrict self) {
	return (size_t)DeeObject_ForeachPair(self->blkw_kw, &blkw_size_foreach_cb, self);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwIterator *DCALL
blkw_iter(DeeBlackListKwObject *__restrict self) {
	DREF DeeBlackListKwIterator *result;
	result = DeeObject_MALLOC(DeeBlackListKwIterator);
	if unlikely(!result)
		goto done;
	result->mi_iter = DeeObject_Iter(self->blkw_kw);
	if unlikely(!result->mi_iter)
		goto err_r;
	result->mi_map = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeBlackListKwIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_contains(DeeBlackListKwObject *self,
             DeeObject *key) {
	if (DeeString_Check(key)) {
		if (DeeBlackListKw_IsBlackListed(self, key))
			return_false;
	}
	return DeeObject_Contains(self->blkw_kw, key);
}

struct blkw_foreach_pair_data {
	DeeBlackListKwObject *bfp_self; /* [1..1] The blacklist controller. */
	Dee_foreach_pair_t    bfp_proc; /* [1..1] Wrapped callback. */
	void                 *bfp_arg;  /* Cookie for `bfp_proc' */
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
blkw_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct blkw_foreach_pair_data *data;
	data = (struct blkw_foreach_pair_data *)arg;
	if (DeeString_Check(key) && !DeeBlackListKw_IsBlackListed(data->bfp_self, key))
		return (*data->bfp_proc)(data->bfp_arg, key, value);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
blkw_foreach_pair(DeeBlackListKwObject *self, Dee_foreach_pair_t proc, void *arg) {
	struct blkw_foreach_pair_data data;
	data.bfp_self = self;
	data.bfp_proc = proc;
	data.bfp_arg  = arg;
	return DeeObject_ForeachPair(DeeBlackListKw_KW(self), &blkw_foreach_pair_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_getitem(DeeBlackListKwObject *self, DeeObject *key) {
	if likely(DeeString_Check(key) && !DeeBlackListKw_IsBlackListed(self, key))
		return DeeObject_GetItem(DeeBlackListKw_KW(self), key);
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_getitem_string_hash(DeeBlackListKwObject *__restrict self, char const *key, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, key, hash))
		return DeeObject_GetItemStringHash(DeeBlackListKw_KW(self), key, hash);
	err_unknown_key_str((DeeObject *)self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_getitem_string_len_hash(DeeBlackListKwObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, key, keylen, hash))
		return DeeObject_GetItemStringLenHash(DeeBlackListKw_KW(self), key, keylen, hash);
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_trygetitem(DeeBlackListKwObject *self, DeeObject *key) {
	if likely(DeeString_Check(key) && !DeeBlackListKw_IsBlackListed(self, key))
		return DeeObject_TryGetItem(DeeBlackListKw_KW(self), key);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_trygetitem_string_hash(DeeBlackListKwObject *__restrict self, char const *key, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, key, hash))
		return DeeObject_TryGetItemStringHash(DeeBlackListKw_KW(self), key, hash);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_trygetitem_string_len_hash(DeeBlackListKwObject *__restrict self, char const *key, size_t keylen, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, key, keylen, hash))
		return DeeObject_TryGetItemStringLenHash(DeeBlackListKw_KW(self), key, keylen, hash);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blkw_bounditem(DeeBlackListKwObject *self, DeeObject *key) {
	if likely(DeeString_Check(key) && !DeeBlackListKw_IsBlackListed(self, key))
		return DeeObject_BoundItem(DeeBlackListKw_KW(self), key);
	return Dee_BOUND_MISSING;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blkw_bounditem_string_hash(DeeBlackListKwObject *__restrict self,
                           char const *__restrict name, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeObject_BoundItemStringHash(DeeBlackListKw_KW(self), name, hash);
	return Dee_BOUND_MISSING;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blkw_bounditem_string_len_hash(DeeBlackListKwObject *__restrict self,
                               char const *__restrict name,
                               size_t namelen, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeObject_BoundItemStringLenHash(DeeBlackListKw_KW(self), name, namelen, hash);
	return Dee_BOUND_MISSING;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blkw_hasitem(DeeBlackListKwObject *self, DeeObject *key) {
	if likely(DeeString_Check(key) && !DeeBlackListKw_IsBlackListed(self, key))
		return DeeObject_HasItem(DeeBlackListKw_KW(self), key);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blkw_hasitem_string_hash(DeeBlackListKwObject *__restrict self,
                         char const *__restrict name, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeObject_HasItemStringHash(DeeBlackListKw_KW(self), name, hash);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blkw_hasitem_string_len_hash(DeeBlackListKwObject *__restrict self,
                             char const *__restrict name,
                             size_t namelen, Dee_hash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeObject_HasItemStringLenHash(DeeBlackListKw_KW(self), name, namelen, hash);
	return 0;
}


PRIVATE struct type_seq blkw_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blkw_iter,
	/* .tp_sizeob                       = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blkw_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blkw_getitem,
	/* .tp_delitem                      = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                      = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                     = */ NULL,
	/* .tp_delrange                     = */ NULL,
	/* .tp_setrange                     = */ NULL,
	/* .tp_foreach                      = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&blkw_foreach_pair,
	/* .tp_enumerate                    = */ NULL,
	/* .tp_enumerate_index              = */ NULL,
	/* .tp_iterkeys                     = */ NULL,
	/* .tp_bounditem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&blkw_bounditem,
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&blkw_hasitem,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&blkw_size,
	/* .tp_size_fast                    = */ NULL,
	/* .tp_getitem_index                = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index                = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index              = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index                = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index               = */ NULL,
	/* .tp_delrange_index               = */ NULL,
	/* .tp_setrange_index               = */ NULL,
	/* .tp_getrange_index_n             = */ NULL,
	/* .tp_delrange_index_n             = */ NULL,
	/* .tp_setrange_index_n             = */ NULL,
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blkw_trygetitem,
	/* .tp_trygetitem_index             = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&blkw_trygetitem_string_hash,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&blkw_getitem_string_hash,
	/* .tp_delitem_string_hash          = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash          = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&blkw_bounditem_string_hash,
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&blkw_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&blkw_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&blkw_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash      = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&blkw_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&blkw_hasitem_string_len_hash,
	/* .tp_asvector                     = */ NULL,
	/* .tp_asvector_nothrow             = */ NULL,
	/* .tp_unpack                       = */ NULL,
	/* .tp_unpack_ex                    = */ NULL,
	/* .tp_unpack_ub                    = */ NULL,
	/* .tp_getitemnr                    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&blkw_getitemnr,
	/* .tp_getitemnr_string_hash        = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&blkw_getitemnr_string_hash,
	/* .tp_getitemnr_string_len_hash    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&blkw_getitemnr_string_len_hash,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&blkw_trygetitemnr,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&blkw_trygetitemnr_string_hash,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&blkw_trygetitemnr_string_len_hash,
};

PRIVATE WUNUSED DREF DeeBlackListKwObject *DCALL
blkw_init_kw(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeCodeObject *code;
	DeeObject *kwds;
	size_t positional;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__code_positional_kwds,
	                    "o" UNPuSIZ "o:_BlackListKw",
	                    &code, &positional, &kwds))
		goto err;
	if (DeeObject_AssertTypeExact(code, &DeeCode_Type))
		goto err;

	/* Validate arguments. */
	if unlikely(!code->co_keywords) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Code object does not define keyword arguments");
		goto err;
	}
	if unlikely(code->co_argc_max <= positional) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Code object takes at most %" PRFu16 " positional "
		                "arguments when %" PRFuSIZ " were supposedly given",
		                code->co_argc_max, positional);
		goto err;
	}

	result = DeeBlackListKw_New(code, positional, kwds);
	return (DREF DeeBlackListKwObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwObject *DCALL
blkw_copy(DeeBlackListKwObject *__restrict self) {
	DREF DeeObject *result_kw;
	DREF DeeBlackListKwObject *result;
	/* Create a copy of the original keywords, since we're acting as a proxy. */
	result_kw = DeeObject_Copy(self->blkw_kw);
	if unlikely(!result_kw)
		goto err;
	if (result_kw == self->blkw_kw) {
		Dee_DecrefNokill(result_kw);
		return_reference_(self);
	}

	result = (DREF DeeBlackListKwObject *)DeeObject_Mallocc(offsetof(DeeBlackListKwObject, blkw_blck),
	                                                        self->blkw_mask + 1, sizeof(DeeBlackListKwdsEntry));
	if unlikely(!result)
		goto err_result_kw;
	result->blkw_kw = result_kw; /* Inherit reference */
	Dee_atomic_rwlock_init(&result->blkw_lock);
	result->blkw_code = self->blkw_code;
	Dee_Incref(result->blkw_code);
	result->blkw_ckwc = self->blkw_ckwc;
	result->blkw_ckwv = self->blkw_ckwv;
	result->blkw_mask = self->blkw_mask;
	DeeBlackListKw_LockRead(self);
	result->blkw_load = self->blkw_load;
	memcpyc(result->blkw_blck,
	        self->blkw_blck,
	        result->blkw_mask + 1,
	        sizeof(DeeBlackListKwdsEntry));
	DeeBlackListKw_LockEndRead(self);
	DeeObject_Init(result, &DeeBlackListKw_Type);
	return result;
err_result_kw:
	Dee_Decref(result_kw);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwObject *DCALL
blkw_deep(DeeBlackListKwObject *__restrict self) {
	DREF DeeBlackListKwObject *result;
	result = (DREF DeeBlackListKwObject *)DeeObject_Mallocc(offsetof(DeeBlackListKwObject, blkw_blck),
	                                                        self->blkw_mask + 1, sizeof(DeeBlackListKwdsEntry));
	if unlikely(!result)
		goto done;
	result->blkw_kw = DeeObject_DeepCopy(self->blkw_kw);
	if unlikely(!result->blkw_kw)
		goto err_r;
	Dee_atomic_rwlock_init(&result->blkw_lock);
	result->blkw_code = self->blkw_code; /* Immutable, so no copy required */
	Dee_Incref(result->blkw_code);
	result->blkw_ckwc = self->blkw_ckwc;
	result->blkw_ckwv = self->blkw_ckwv;
	result->blkw_mask = self->blkw_mask;
	DeeBlackListKw_LockRead(self);
	result->blkw_load = self->blkw_load;
	memcpyc(result->blkw_blck,
	        self->blkw_blck,
	        result->blkw_mask + 1,
	        sizeof(DeeBlackListKwdsEntry));
	DeeBlackListKw_LockEndRead(self);
	DeeObject_Init(result, &DeeBlackListKw_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwObject *DCALL
blkw_get_frozen(DeeBlackListKwObject *__restrict self) {
	DREF DeeObject *frozen_kw;
	DREF DeeBlackListKwObject *result;
	frozen_kw = DeeObject_GetAttr(self->blkw_kw, (DeeObject *)&str_frozen);
	if unlikely(!frozen_kw)
		goto err;
	if (frozen_kw == self->blkw_kw) {
		Dee_DecrefNokill(frozen_kw);
		return_reference_(self);
	}

	result = (DREF DeeBlackListKwObject *)DeeObject_Mallocc(offsetof(DeeBlackListKwObject, blkw_blck),
	                                                        self->blkw_mask + 1, sizeof(DeeBlackListKwdsEntry));
	if unlikely(!result)
		goto err_frozen_kw;

	result->blkw_kw = frozen_kw; /* Inherit reference */
	Dee_atomic_rwlock_init(&result->blkw_lock);
	result->blkw_code = self->blkw_code;
	Dee_Incref(result->blkw_code);
	result->blkw_ckwc = self->blkw_ckwc;
	result->blkw_ckwv = self->blkw_ckwv;
	result->blkw_mask = self->blkw_mask;
	DeeBlackListKw_LockRead(self);
	result->blkw_load = self->blkw_load;
	memcpyc(result->blkw_blck,
	        self->blkw_blck,
	        result->blkw_mask + 1,
	        sizeof(DeeBlackListKwdsEntry));
	DeeBlackListKw_LockEndRead(self);
	DeeObject_Init(result, &DeeBlackListKw_Type);
	return result;
err_frozen_kw:
	Dee_Decref(frozen_kw);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst blkw_getsets[] = {
	TYPE_GETTER(STR_frozen, &blkw_get_frozen, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst blkw_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeBlackListKwIterator_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeBlackListKw_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListKw",
	/* .tp_doc      = */ DOC("A ${{string: Object}}-like mapping that is similar to ?GDeeBlackListKwds, "
	                         /**/ "however gets used when the function is invoked using a custom keyword "
	                         /**/ "protocol, rather than conventional keyword arguments that store their "
	                         /**/ "values as part of the argument vector:\n"
	                         "${"
	                         /**/ "function foo(a, **kwds) {\n"
	                         /**/ "	print type kwds; /* _BlackListKw */\n"
	                         /**/ "}\n"
	                         /**/ "foo(**{ \"a\": 10, \"b\": 20});"
	                         "}\n"
	                         "\n"
	                         "(code:?Ert:Code,positional:?Dint,kwds:?Ert:Kwds)\n"
	                         "Construct a new ?. object that blacklists the first @positional "
	                         /**/ "arguments from @code. Keyword argument values are then taken "
	                         /**/ "from @kwds"
	),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)&blkw_copy,
				/* .tp_deep_ctor   = */ (dfunptr_t)&blkw_deep,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				/* .tp_free        = */ (dfunptr_t)NULL,
				/* .tp_alloc       = */ { (dfunptr_t)NULL },
				/* .tp_any_ctor_kw = */ (dfunptr_t)&blkw_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blkw_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&blkw_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blkw_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__667432E5904B49F8),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__40D3D60A1F18CAE2),
	/* .tp_seq           = */ &blkw_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ blkw_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ blkw_class_members,
};


/* Construct a new mapping for a general-purpose mapping that follows the black-listing scheme.
 * -> The returned objects can be used for any kind of mapping, such that in the
 *    case of kwmappings, `DeeBlackListKw_New(code, DeeKwdsMapping_New(kwds, argv))'
 *    would produce the semantically equivalent of `DeeBlackListKwds_New(code, kwds, argv)'
 * -> This function is used to filter keyword arguments from varkwds when the general
 *    purpose keyword argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListKw { "something_else" : "foobar" }'
 *    >> foo(**{ "x" : 10, "something_else" : "foobar" }); */
PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeBlackListKw_New(struct code_object *__restrict code,
                   size_t positional_argc,
                   DeeObject *__restrict kw) {
	DREF DeeBlackListKwObject *result;
	size_t mask, argc = code->co_argc_max;
	ASSERT(argc > positional_argc);
	ASSERT(code->co_keywords);

	/* Calculate an appropriate mask for the blacklist hash-set. */
	for (mask = 3; mask <= argc; mask = (mask << 1) | 1)
		;
	result = (DREF DeeBlackListKwObject *)DeeObject_Callocc(offsetof(DeeBlackListKwObject, blkw_blck),
	                                                        mask + 1, sizeof(DeeBlackListKwdsEntry));
	if unlikely(!result)
		goto done;
	Dee_atomic_rwlock_cinit(&result->blkw_lock);
	result->blkw_code = code;
	result->blkw_ckwc = argc - positional_argc;
	result->blkw_ckwv = code->co_keywords + positional_argc;
	result->blkw_kw   = kw;
	result->blkw_mask = mask;
	Dee_Incref(code);
	Dee_Incref(kw);
	DeeObject_Init(result, &DeeBlackListKw_Type);
done:
	return (DREF DeeObject *)result;
}



DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_C */
