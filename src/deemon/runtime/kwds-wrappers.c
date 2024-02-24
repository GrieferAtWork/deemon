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
#ifndef GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_C
#define GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
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

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

/* ======================== DeeBlackListKwdsObject ======================== */

typedef struct {
	OBJECT_HEAD
	DWEAK struct kwds_entry     *blki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry           *blki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
	DREF DeeBlackListKwdsObject *blki_map;  /* [1..1][const] The associated keywords mapping. */
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

PRIVATE NONNULL((1)) void DCALL
blvi_fini(DeeBlackListKwdsIterator *__restrict self) {
	Dee_Decref(self->blki_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
blvi_visit(DeeBlackListKwdsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->blki_map);
}


LOCAL struct kwds_entry *DCALL
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blvi_next(DeeBlackListKwdsIterator *__restrict self) {
	DREF DeeObject *value, *result;
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return ITER_DONE;
	DeeBlackListKwds_LockRead(self->blki_map);
	if unlikely(!self->blki_map->blkd_argv) {
		DeeBlackListKwds_LockEndRead(self->blki_map);
		return ITER_DONE;
	}
	value = self->blki_map->blkd_argv[ent->ke_index];
	Dee_Incref(value);
	DeeBlackListKwds_LockEndRead(self->blki_map);
	result = DeeTuple_Pack(2, ent->ke_name, value);
	Dee_Decref_unlikely(value);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
blvi_nsi_nextkey(DeeBlackListKwdsIterator *__restrict self) {
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return (DREF DeeStringObject *)ITER_DONE;
	return_reference_(ent->ke_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blvi_nsi_nextvalue(DeeBlackListKwdsIterator *__restrict self) {
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


#define DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE(name, op)                     \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                     \
	name(DeeBlackListKwdsIterator *self, DeeBlackListKwdsIterator *other) {   \
		if (DeeObject_AssertTypeExact(other, &DeeBlackListKwdsIterator_Type)) \
			goto err;                                                         \
		return_bool(BLVI_GETITER(self) op BLVI_GETITER(other));               \
	err:                                                                      \
		return NULL;                                                          \
	}
DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE(blvi_eq, ==)
DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE(blvi_ne, !=)
DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE(blvi_lo, <)
DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE(blvi_le, <=)
DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE(blvi_gr, >)
DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE(blvi_ge, >=)
#undef DEFINE_BLACKLISTVARKWDSITERATOR_COMPARE

PRIVATE struct type_cmp blvi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blvi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blvi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blvi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blvi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blvi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blvi_ge,
};

PRIVATE struct type_member tpconst blvi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DeeBlackListKwdsIterator, blki_map), "->?Ert:DeeBlackListKwdsObject"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeBlackListKwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DeeBlackListKwdsIterator",
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL /* XXX: Could easily be implemented... */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blvi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &blvi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blvi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ blvi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKwds_IsBlackListed(DeeBlackListKwdsObject *__restrict self,
                               DeeObject *__restrict name) {
	dhash_t i, perturb;
	DeeStringObject *str;
	dhash_t hash;
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
			dhash_t hashof_str;
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
                                         dhash_t hash) {
	dhash_t i, perturb;
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
			dhash_t hashof_str;
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

INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL
DeeBlackListKwds_IsBlackListedStringLenHash(DeeBlackListKwdsObject *__restrict self,
                                            char const *__restrict name,
                                            size_t namelen, dhash_t hash) {
	dhash_t i, perturb;
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
			dhash_t hashof_str;
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



PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKwds_HasItem(DeeBlackListKwdsObject *__restrict self,
                         DeeObject *__restrict name) {
	size_t index = DeeKwds_IndexOf((DeeObject *)self->blkd_kwds, name);
	return (index != (size_t)-1) && !DeeBlackListKwds_IsBlackListed(self, name);
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKwds_HasItemStringHash(DeeBlackListKwdsObject *__restrict self,
                                   char const *__restrict name,
                                   dhash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash((DeeObject *)self->blkd_kwds, name, hash);
	return (index != (size_t)-1) && !DeeBlackListKwds_IsBlackListedStringHash(self, name, hash);
}

INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL
DeeBlackListKwds_HasItemStringLenHash(DeeBlackListKwdsObject *__restrict self,
                                      char const *__restrict name,
                                      size_t namelen, dhash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash((DeeObject *)self->blkd_kwds, name, namelen, hash);
	return (index != (size_t)-1) && !DeeBlackListKwds_IsBlackListedStringLenHash(self, name, namelen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeBlackListKwds_GetItemNR(DeeBlackListKwdsObject *__restrict self,
                           DeeObject *__restrict name) {
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

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeBlackListKwds_GetItemNRStringHash(DeeBlackListKwdsObject *__restrict self,
                                     char const *__restrict name, dhash_t hash) {
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

INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) DeeObject *DCALL
DeeBlackListKwds_GetItemNRStringLenHash(DeeBlackListKwdsObject *__restrict self,
                                        char const *__restrict name,
                                        size_t namelen, dhash_t hash) {
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

INTERN WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
DeeBlackListKwds_GetItemNRDef(DeeBlackListKwdsObject *__restrict self,
                              DeeObject *__restrict name,
                              DeeObject *__restrict def) {
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
	return def;
}

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeBlackListKwds_GetItemNRStringHashDef(DeeBlackListKwdsObject *__restrict self,
                                        char const *__restrict name, dhash_t hash,
                                        DeeObject *__restrict def) {
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
	return def;
}

INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) DeeObject *DCALL
DeeBlackListKwds_GetItemNRStringLenHashDef(DeeBlackListKwdsObject *__restrict self,
                                           char const *__restrict name,
                                           size_t namelen, dhash_t hash,
                                           DeeObject *__restrict def) {
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
	return def;
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
blv_nsi_size(DeeBlackListKwdsObject *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blv_size(DeeBlackListKwdsObject *__restrict self) {
	return DeeInt_NewSize(blv_nsi_size(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_contains(DeeBlackListKwdsObject *self,
             DeeObject *key) {
	return_bool(likely(DeeString_Check(key)) &&
	            DeeBlackListKwds_HasItem(self, key));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_getitem(DeeBlackListKwdsObject *self,
            DeeObject *key) {
	DREF DeeObject *result;
	if unlikely(!DeeString_Check(key)) {
		err_unknown_key((DeeObject *)self, key);
		return NULL;
	}
	result = DeeBlackListKwds_GetItemNR(self, key);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
blv_getdefault(DeeBlackListKwdsObject *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict def) {
	DREF DeeObject *result;
	if unlikely(!DeeString_Check(key)) {
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	result = DeeBlackListKwds_GetItemNRDef(self, key, def);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE struct type_nsi tpconst blv_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&blv_nsi_size,
			/* .nsi_nextkey    = */ (dfunptr_t)&blvi_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&blvi_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&blv_getdefault
		}
	}
};

PRIVATE struct type_seq blv_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blv_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blv_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blv_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blv_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &blv_nsi
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwdsObject *DCALL
blv_copy(DeeBlackListKwdsObject *__restrict self) {
	size_t count = self->blkd_kwds->kw_size;
	size_t sizeof_blkd_blck = ((self->blkd_mask + 1) * sizeof(DeeBlackListKwdsEntry));
	DREF DeeBlackListKwdsObject *result;
	result = (DREF DeeBlackListKwdsObject *)DeeObject_Malloc(offsetof(DeeBlackListKwdsObject, blkd_blck) +
	                                                         sizeof_blkd_blck + (count * sizeof(DREF DeeObject *)));
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
	result = (DREF DeeBlackListKwdsObject *)DeeObject_Malloc(offsetof(DeeBlackListKwdsObject, blkd_blck) +
	                                                         sizeof_blkd_blck + (count * sizeof(DREF DeeObject *)));
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
	/* .tp_name     = */ "_DeeBlackListKwds",
	/* .tp_doc      = */ DOC("A ${{string: Object}}-like mapping that is used to exclude positional "
	                         /**/ "keyword arguments for a variable-keywords user-code function, when that "
	                         /**/ "function is invoked with regular keywords being passed:\n"
	                         "${"
	                         /**/ "function foo(a, **kwds) {\n"
	                         /**/ "	print type kwds; /* _DeeBlackListKwds */\n"
	                         /**/ "}\n"
	                         /**/ "foo(10, b: 20);\n"
	                         /**/ "foo(a: 10, b: 20);"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | TF_KW,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_copy_ctor = */ (dfunptr_t)&blv_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&blv_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&blv_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &blv_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ blv_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ blv_class_members
};


/* Construct a new mapping for keywords that follows the black-listing scheme.
 * NOTE: If `kwds' is empty, return `Dee_EmptyMapping' instead.
 * NOTE: If `code' doesn't specify any keywords, return `DeeKwdsMapping_New()' instead.
 * Otherwise, the caller must decref the returned object using `DeeBlackListKwds_Decref()' */
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
	result = (DREF DeeBlackListKwdsObject *)DeeObject_Calloc(offsetof(DeeBlackListKwdsObject, blkd_blck) +
	                                                         ((mask + 1) * sizeof(DeeBlackListKwdsEntry)) +
	                                                         (kwds->kw_size * sizeof(DREF DeeObject *)));
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
	OBJECT_HEAD
	DREF DeeObject            *mi_iter; /* [1..1][const] An iterator for the underlying `mi_map->blkw_kw'. */
	DREF DeeBlackListKwObject *mi_map;  /* [1..1][const] The general-purpose blacklist mapping being iterated. */
} DeeBlackListKwIterator;

INTDEF DeeTypeObject DeeBlackListKwIterator_Type;

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blmi_copy(DeeBlackListKwIterator *__restrict self,
          DeeBlackListKwIterator *__restrict other) {
	self->mi_iter = DeeObject_Copy(other->mi_iter);
	if unlikely(!self->mi_iter)
		goto err;
	self->mi_map = other->mi_map;
	Dee_Incref(self->mi_map);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
blmi_fini(DeeBlackListKwIterator *__restrict self) {
	Dee_Decref(self->mi_iter);
	Dee_Decref(self->mi_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
blmi_visit(DeeBlackListKwIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mi_iter);
	Dee_Visit(self->mi_map);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blmi_next(DeeBlackListKwIterator *__restrict self) {
	DREF DeeObject *result, *pair[2];
again:
	result = DeeObject_IterNext(self->mi_iter);
	if (!ITER_ISOK(result))
		goto done;
	if unlikely(DeeObject_Unpack(result, 2, pair))
		goto err_r;
	Dee_Decref(pair[1]);
	if (DeeString_Check(pair[0]) &&
	    DeeBlackListKw_IsBlackListed(self->mi_map, pair[0])) {
		Dee_Decref(pair[0]);
		Dee_Decref(result);
		goto again;
	}
	Dee_Decref(pair[0]);
done:
	return result;
err_r:
	Dee_Decref(result);
/*err:*/
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blmi_nsi_nextkey(DeeBlackListKwIterator *__restrict self) {
	DREF DeeObject *result, *pair[2];
again:
	result = DeeObject_IterNext(self->mi_iter);
	if (!ITER_ISOK(result))
		goto done_r;
	if unlikely(DeeObject_Unpack(result, 2, pair))
		goto err_r;
	Dee_Decref(pair[1]);
	if (DeeString_Check(pair[0]) &&
	    DeeBlackListKw_IsBlackListed(self->mi_map, pair[0])) {
		Dee_Decref(pair[0]);
		Dee_Decref(result);
		goto again;
	}
	Dee_Decref(result);
	return pair[0];
done_r:
	return result;
err_r:
	Dee_Decref(result);
/*err:*/
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blmi_nsi_nextvalue(DeeBlackListKwIterator *__restrict self) {
	DREF DeeObject *result, *pair[2];
again:
	result = DeeObject_IterNext(self->mi_iter);
	if (!ITER_ISOK(result))
		goto done_r;
	if unlikely(DeeObject_Unpack(result, 2, pair))
		goto err_r;
	if (DeeString_Check(pair[0]) &&
	    DeeBlackListKw_IsBlackListed(self->mi_map, pair[0])) {
		Dee_Decref(pair[1]);
		Dee_Decref(pair[0]);
		Dee_Decref(result);
		goto again;
	}
	Dee_Decref(pair[0]);
	Dee_Decref(result);
	return pair[1];
done_r:
	return result;
err_r:
	Dee_Decref(result);
/*err:*/
	return NULL;
}


#define DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE(name, func)                 \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                   \
	name(DeeBlackListKwIterator *self, DeeBlackListKwIterator *other) {     \
		if (DeeObject_AssertTypeExact(other, &DeeBlackListKwIterator_Type)) \
			goto err;                                                       \
		return func(self->mi_iter, other->mi_iter);                         \
	err:                                                                    \
		return NULL;                                                        \
	}
DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE(blmi_eq, DeeObject_CompareEqObject)
DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE(blmi_ne, DeeObject_CompareNeObject)
DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE(blmi_lo, DeeObject_CompareLoObject)
DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE(blmi_le, DeeObject_CompareLeObject)
DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE(blmi_gr, DeeObject_CompareGrObject)
DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE(blmi_ge, DeeObject_CompareGeObject)
#undef DEFINE_BLACKLISTMAPPINGITERATOR_COMPARE

PRIVATE struct type_cmp blmi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blmi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blmi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blmi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blmi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blmi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blmi_ge,
};

PRIVATE struct type_member tpconst blmi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DeeBlackListKwIterator, mi_map), "->?Ert:DeeBlackListKwObject"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(DeeBlackListKwIterator, mi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeBlackListKwIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DeeBlackListKwIterator",
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blmi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &blmi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blmi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ blmi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeBlackListKw_IsBlackListed(DeeBlackListKwObject *__restrict self,
                             /*String*/ DeeObject *name) {
	dhash_t i, perturb;
	DeeStringObject *str;
	dhash_t hash;
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
			dhash_t hashof_str;
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
                                       dhash_t hash) {
	dhash_t i, perturb;
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
			dhash_t hashof_str;
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
                                          size_t namelen, dhash_t hash) {
	dhash_t i, perturb;
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
			dhash_t hashof_str;
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



INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeBlackListKw_BoundItemStringHash(DeeBlackListKwObject *__restrict self,
                                   char const *__restrict name,
                                   dhash_t hash, bool allow_missing) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeObject_BoundItemStringHash(DeeBlackListKw_KW(self), name, hash, allow_missing);
	if (!allow_missing)
		return err_unknown_key_str((DeeObject *)self, name);
	return -2;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeBlackListKw_BoundItemStringLenHash(DeeBlackListKwObject *__restrict self,
                                      char const *__restrict name,
                                      size_t namelen, dhash_t hash,
                                      bool allow_missing) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeObject_BoundItemStringLenHash(DeeBlackListKw_KW(self), name, namelen, hash, allow_missing);
	if (!allow_missing)
		return err_unknown_key_str_len((DeeObject *)self, name, namelen);
	return -2;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeBlackListKw_HasItemStringHash(DeeBlackListKwObject *__restrict self,
                                 char const *__restrict name,
                                 dhash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeObject_HasItemStringHash(DeeBlackListKw_KW(self), name, hash);
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeBlackListKw_HasItemStringLenHash(DeeBlackListKwObject *__restrict self,
                                    char const *__restrict name,
                                    size_t namelen, dhash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeObject_HasItemStringLenHash(DeeBlackListKw_KW(self), name, namelen, hash);
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeBlackListKw_GetItemNR(DeeBlackListKwObject *__restrict self,
                         /*string*/ DeeObject *__restrict name) {
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if likely(!DeeBlackListKw_IsBlackListed(self, name))
		return DeeKw_GetItemNR(DeeBlackListKw_KW(self), name);
	err_unknown_key((DeeObject *)self, name);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeBlackListKw_GetItemNRStringHash(DeeBlackListKwObject *__restrict self,
                                   char const *__restrict name, dhash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeKw_GetItemNRStringHash(DeeBlackListKw_KW(self), name, hash);
	err_unknown_key_str((DeeObject *)self, name);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeBlackListKw_GetItemNRStringLenHash(DeeBlackListKwObject *__restrict self,
                                      char const *__restrict name,
                                      size_t namelen, dhash_t hash) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeKw_GetItemNRStringLenHash(DeeBlackListKw_KW(self), name, namelen, hash);
	err_unknown_key_str_len((DeeObject *)self, name, namelen);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
DeeBlackListKw_GetItemNRDef(DeeBlackListKwObject *__restrict self,
                            /*string*/ DeeObject *__restrict name,
                            DeeObject *__restrict def) {
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if likely(!DeeBlackListKw_IsBlackListed(self, name))
		return DeeKw_GetItemNRDef(DeeBlackListKw_KW(self), name, def);
	return def;
}

INTERN WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL
DeeBlackListKw_GetItemNRStringHashDef(DeeBlackListKwObject *__restrict self,
                                      char const *__restrict name, dhash_t hash,
                                      DeeObject *__restrict def) {
	if likely(!DeeBlackListKw_IsBlackListedStringHash(self, name, hash))
		return DeeKw_GetItemNRStringHashDef(DeeBlackListKw_KW(self), name, hash, def);
	return def;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DeeObject *DCALL
DeeBlackListKw_GetItemNRStringLenHashDef(DeeBlackListKwObject *__restrict self,
                                         char const *__restrict name,
                                         size_t namelen, dhash_t hash,
                                         DeeObject *__restrict def) {
	if likely(!DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, hash))
		return DeeKw_GetItemNRStringLenHashDef(DeeBlackListKw_KW(self), name, namelen, hash, def);
	return def;
}

PRIVATE NONNULL((1)) void DCALL
blkw_fini(DeeBlackListKwObject *__restrict self) {
	Dee_Decref(self->blkw_code);
	Dee_Decref(self->blkw_kw);
}

PRIVATE NONNULL((1, 2)) void DCALL
blkw_visit(DeeBlackListKwObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->blkw_code);
	Dee_Visit(self->blkw_kw);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
blkw_bool_foreach(void *arg, DeeObject *key, DeeObject *value) {
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
	int status = (int)DeeObject_ForeachPair(self->blkw_kw, &blkw_bool_foreach, self);
	ASSERT(status == 0 || status == -1 || status == -2);
	if (status == -2)
		status = 1;
	return status;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
blkw_size_foreach(void *arg, DeeObject *key, DeeObject *value) {
	DeeBlackListKwObject *self = (DeeBlackListKwObject *)arg;
	(void)value;
	if (!DeeString_Check(key) ||
	    !DeeBlackListKw_IsBlackListed(self, key))
		return 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
blkw_nsi_size(DeeBlackListKwObject *__restrict self) {
	return (size_t)DeeObject_ForeachPair(self->blkw_kw, &blkw_size_foreach, self);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeBlackListKwIterator *DCALL
blkw_iter(DeeBlackListKwObject *__restrict self) {
	DREF DeeBlackListKwIterator *result;
	result = DeeObject_MALLOC(DeeBlackListKwIterator);
	if unlikely(!result)
		goto done;
	result->mi_iter = DeeObject_IterSelf(self->blkw_kw);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blkw_size(DeeBlackListKwObject *__restrict self) {
	return DeeInt_NewSize(blkw_nsi_size(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blkw_contains(DeeBlackListKwObject *self,
             DeeObject *key) {
	if (DeeString_Check(key) &&
	    DeeBlackListKw_IsBlackListed(self, key))
		return_false;
	return DeeObject_ContainsObject(self->blkw_kw, key);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
blkw_getdefault(DeeBlackListKwObject *__restrict self,
                DeeObject *__restrict key,
                DeeObject *__restrict def) {
	DeeObject *result;
	if unlikely(!DeeString_Check(key)) {
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	result = DeeBlackListKw_GetItemNRDef(self, key, def);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE struct type_nsi tpconst blkw_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&blkw_nsi_size,
			/* .nsi_nextkey    = */ (dfunptr_t)&blmi_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&blmi_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&blkw_getdefault
		}
	}
};

PRIVATE struct type_seq blkw_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blkw_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blkw_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blkw_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&DeeBlackListKw_GetItemNR,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &blkw_nsi
};

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

	result = (DREF DeeBlackListKwObject *)DeeObject_Malloc(offsetof(DeeBlackListKwObject, blkw_blck) +
	                                                       (self->blkw_mask + 1) *
	                                                       sizeof(DeeBlackListKwdsEntry));
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
	result = (DREF DeeBlackListKwObject *)DeeObject_Malloc(offsetof(DeeBlackListKwObject, blkw_blck) +
	                                                       (self->blkw_mask + 1) *
	                                                       sizeof(DeeBlackListKwdsEntry));
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

	result = (DREF DeeBlackListKwObject *)DeeObject_Malloc(offsetof(DeeBlackListKwObject, blkw_blck) +
	                                                       (self->blkw_mask + 1) *
	                                                       sizeof(DeeBlackListKwdsEntry));
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
	/* .tp_name     = */ "_DeeBlackListKw",
	/* .tp_doc      = */ DOC("A ${{string: Object}}-like mapping that is similar to ?GDeeBlackListKwds, "
	                         /**/ "however gets used when the function is invoked using a custom keyword "
	                         /**/ "protocol, rather than conventional keyword arguments that store their "
	                         /**/ "values as part of the argument vector:\n"
	                         "${"
	                         /**/ "function foo(a, **kwds) {\n"
	                         /**/ "	print type kwds; /* _DeeBlackListKw */\n"
	                         /**/ "}\n"
	                         /**/ "foo(**{ \"a\": 10, \"b\": 20});"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | TF_KW,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&blkw_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&blkw_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blkw_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&blkw_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blkw_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &blkw_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ blkw_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ blkw_class_members
};


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
	result = (DREF DeeBlackListKwObject *)DeeObject_Calloc(offsetof(DeeBlackListKwObject, blkw_blck) +
	                                                       (mask + 1) * sizeof(DeeBlackListKwdsEntry));
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
