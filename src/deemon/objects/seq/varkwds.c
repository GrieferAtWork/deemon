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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_C
#define GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_C 1

#include "varkwds.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#define BLVI_GETITER(self) atomic_read(&(self)->blki_iter)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blvi_copy(BlackListVarkwdsIterator *__restrict self,
          BlackListVarkwdsIterator *__restrict other) {
	self->blki_iter = BLVI_GETITER(other);
	self->blki_end  = other->blki_end;
	self->blki_map  = other->blki_map;
	Dee_Incref(self->blki_map);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blvi_deep(BlackListVarkwdsIterator *__restrict self,
          BlackListVarkwdsIterator *__restrict other) {
	self->blki_iter = BLVI_GETITER(other);
	self->blki_end  = other->blki_end;
	self->blki_map  = (DREF BlackListVarkwds *)DeeObject_DeepCopy((DeeObject *)other->blki_map);
	if unlikely(!self->blki_map)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
blvi_fini(BlackListVarkwdsIterator *__restrict self) {
	Dee_Decref(self->blki_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
blvi_visit(BlackListVarkwdsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->blki_map);
}


LOCAL struct kwds_entry *DCALL
blvi_nextiter(BlackListVarkwdsIterator *__restrict self) {
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
			if (!BlackListVarkwds_IsBlackListed(self->blki_map, (DeeObject *)iter->ke_name))
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
blvi_next(BlackListVarkwdsIterator *__restrict self) {
	DREF DeeObject *value, *result;
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return ITER_DONE;
	BlackListVarkwds_LockRead(self->blki_map);
	if unlikely(!self->blki_map->blvk_argv) {
		BlackListVarkwds_LockEndRead(self->blki_map);
		return ITER_DONE;
	}
	value = self->blki_map->blvk_argv[ent->ke_index];
	Dee_Incref(value);
	BlackListVarkwds_LockEndRead(self->blki_map);
	result = DeeTuple_Pack(2, ent->ke_name, value);
	Dee_Decref_unlikely(value);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
blvi_nsi_nextkey(BlackListVarkwdsIterator *__restrict self) {
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return (DREF DeeStringObject *)ITER_DONE;
	return_reference_(ent->ke_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blvi_nsi_nextvalue(BlackListVarkwdsIterator *__restrict self) {
	DREF DeeObject *result;
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return ITER_DONE;
	BlackListVarkwds_LockRead(self->blki_map);
	if unlikely(!self->blki_map->blvk_argv) {
		BlackListVarkwds_LockEndRead(self->blki_map);
		return ITER_DONE;
	}
	result = self->blki_map->blvk_argv[ent->ke_index];
	Dee_Incref(result);
	BlackListVarkwds_LockEndRead(self->blki_map);
	return result;
}


#define DEFINE_FILTERITERATOR_COMPARE(name, op)                               \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                     \
	name(BlackListVarkwdsIterator *self, BlackListVarkwdsIterator *other) {   \
		if (DeeObject_AssertTypeExact(other, &BlackListVarkwdsIterator_Type)) \
			goto err;                                                         \
		return_bool(BLVI_GETITER(self) op BLVI_GETITER(other));               \
	err:                                                                      \
		return NULL;                                                          \
	}
DEFINE_FILTERITERATOR_COMPARE(blvi_eq, ==)
DEFINE_FILTERITERATOR_COMPARE(blvi_ne, !=)
DEFINE_FILTERITERATOR_COMPARE(blvi_lo, <)
DEFINE_FILTERITERATOR_COMPARE(blvi_le, <=)
DEFINE_FILTERITERATOR_COMPARE(blvi_gr, >)
DEFINE_FILTERITERATOR_COMPARE(blvi_ge, >=)
#undef DEFINE_FILTERITERATOR_COMPARE

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
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(BlackListVarkwdsIterator, blki_map), "->?Ert:BlackListVarkwds"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BlackListVarkwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListVarkwdsIterator",
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
				TYPE_FIXED_ALLOCATOR(BlackListVarkwdsIterator)
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





LOCAL size_t DCALL
kwds_FindIndex(DeeKwdsObject *__restrict self,
               DeeStringObject *__restrict name) {
	dhash_t i, perturb, hash;
	hash    = DeeString_Hash((DeeObject *)name);
	perturb = i = hash & self->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		struct kwds_entry *entry;
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (DeeString_SIZE(entry->ke_name) != DeeString_SIZE(name))
			continue;
		if (bcmpc(DeeString_STR(entry->ke_name),
		          DeeString_STR(name),
		          DeeString_SIZE(name),
		          sizeof(char)) != 0)
			continue;
		return entry->ke_index;
	}
	return (size_t)-1;
}

LOCAL size_t DCALL
kwds_FindIndexStr(DeeKwdsObject *__restrict self,
                  char const *__restrict name,
                  dhash_t hash) {
	dhash_t i, perturb;
	perturb = i = hash & self->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		struct kwds_entry *entry;
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (strcmp(DeeString_STR(entry->ke_name), name) != 0)
			continue;
		return entry->ke_index;
	}
	return (size_t)-1;
}

LOCAL size_t DCALL
kwds_FindIndexStrLen(DeeKwdsObject *__restrict self,
                     char const *__restrict name,
                     size_t namesize,
                     dhash_t hash) {
	dhash_t i, perturb;
	perturb = i = hash & self->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		struct kwds_entry *entry;
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (DeeString_SIZE(entry->ke_name) != namesize)
			continue;
		if (bcmpc(DeeString_STR(entry->ke_name), name,
		          namesize, sizeof(char)) != 0)
			continue;
		return entry->ke_index;
	}
	return (size_t)-1;
}




INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListVarkwds_IsBlackListed(BlackListVarkwds *__restrict self,
                               DeeObject *__restrict name) {
	dhash_t i, perturb;
	DeeStringObject *str;
	dhash_t hash;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
again:
	i = perturb = hash & self->blvk_mask;
	for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
		str = self->blvk_blck[i & self->blvk_mask].blve_str;
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
	if (atomic_read(&self->blvk_load) < self->blvk_ckwc) {
		size_t index;
		BlackListVarkwds_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blvk_load);
		while (index < self->blvk_ckwc) {
			dhash_t hashof_str;
			str = self->blvk_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blvk_mask;
			for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->blvk_blck[i & self->blvk_mask];
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
				self->blvk_load = index;
				BlackListVarkwds_LockEndWrite(self);
				return true;
			}
		}
		self->blvk_load = index;
		BlackListVarkwds_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListVarkwds_IsBlackListedString(BlackListVarkwds *__restrict self,
                                     char const *__restrict name,
                                     dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blvk_mask;
	for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
		str = self->blvk_blck[i & self->blvk_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (strcmp(DeeString_STR(str), name) != 0)
			continue;
		return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blvk_load) < self->blvk_ckwc) {
		size_t index;
		BlackListVarkwds_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blvk_load);
		while (index < self->blvk_ckwc) {
			dhash_t hashof_str;
			str = self->blvk_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blvk_mask;
			for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->blvk_blck[i & self->blvk_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash &&
			    strcmp(DeeString_STR(str), name) == 0) {
				self->blvk_load = index;
				BlackListVarkwds_LockEndWrite(self);
				return true;
			}
		}
		self->blvk_load = index;
		BlackListVarkwds_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListVarkwds_IsBlackListedStringLen(BlackListVarkwds *__restrict self,
                                        char const *__restrict name,
                                        size_t namesize, dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blvk_mask;
	for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
		str = self->blvk_blck[i & self->blvk_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_EqualsBuf(str, name, namesize))
			return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blvk_load) < self->blvk_ckwc) {
		size_t index;
		BlackListVarkwds_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blvk_load);
		while (index < self->blvk_ckwc) {
			dhash_t hashof_str;
			str = self->blvk_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blvk_mask;
			for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->blvk_blck[i & self->blvk_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash && DeeString_SIZE(str) == namesize &&
			    bcmpc(DeeString_STR(str), name, namesize, sizeof(char)) == 0) {
				self->blvk_load = index;
				BlackListVarkwds_LockEndWrite(self);
				return true;
			}
		}
		self->blvk_load = index;
		BlackListVarkwds_LockEndWrite(self);
		goto again;
	}
	return false;
}



INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListVarkwds_HasItem(BlackListVarkwds *__restrict self,
                         DeeObject *__restrict name) {
	return (kwds_FindIndex(self->blvk_kwds, (DeeStringObject *)name) != (size_t)-1) &&
	       !BlackListVarkwds_IsBlackListed(self, name) &&
	       atomic_read(&self->blvk_argv) != NULL;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListVarkwds_HasItemString(BlackListVarkwds *__restrict self,
                               char const *__restrict name,
                               dhash_t hash) {
	return (kwds_FindIndexStr(self->blvk_kwds, name, hash) != (size_t)-1) &&
	       !BlackListVarkwds_IsBlackListedString(self, name, hash) &&
	       atomic_read(&self->blvk_argv) != NULL;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListVarkwds_HasItemStringLen(BlackListVarkwds *__restrict self,
                                  char const *__restrict name,
                                  size_t namesize, dhash_t hash) {
	return (kwds_FindIndexStrLen(self->blvk_kwds, name, namesize, hash) != (size_t)-1) &&
	       !BlackListVarkwds_IsBlackListedStringLen(self, name, namesize, hash) &&
	       atomic_read(&self->blvk_argv) != NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
BlackListVarkwds_GetItem(BlackListVarkwds *self,
                         DeeObject *name) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndex(self->blvk_kwds, (DeeStringObject *)name);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListed(self, name))
		goto missing;
	BlackListVarkwds_LockRead(self);
	if unlikely(!self->blvk_argv) {
		BlackListVarkwds_LockEndRead(self);
		goto missing;
	}
	result = self->blvk_argv[index];
	Dee_Incref(result);
	BlackListVarkwds_LockEndRead(self);
	return result;
missing:
	err_unknown_key((DeeObject *)self, name);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
BlackListVarkwds_GetItemString(BlackListVarkwds *__restrict self,
                               char const *__restrict name, dhash_t hash) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStr(self->blvk_kwds, name, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedString(self, name, hash))
		goto missing;
	BlackListVarkwds_LockRead(self);
	if unlikely(!self->blvk_argv) {
		BlackListVarkwds_LockEndRead(self);
		goto missing;
	}
	result = self->blvk_argv[index];
	Dee_Incref(result);
	BlackListVarkwds_LockEndRead(self);
	return result;
missing:
	err_unknown_key_str((DeeObject *)self, name);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
BlackListVarkwds_GetItemStringLen(BlackListVarkwds *__restrict self,
                                  char const *__restrict name,
                                  size_t namesize, dhash_t hash) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStrLen(self->blvk_kwds, name, namesize, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	BlackListVarkwds_LockRead(self);
	if unlikely(!self->blvk_argv) {
		BlackListVarkwds_LockEndRead(self);
		goto missing;
	}
	result = self->blvk_argv[index];
	Dee_Incref(result);
	BlackListVarkwds_LockEndRead(self);
	return result;
missing:
	err_unknown_key_str_len((DeeObject *)self, name, namesize);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
BlackListVarkwds_GetItemDef(BlackListVarkwds *__restrict self,
                            DeeObject *__restrict name,
                            DeeObject *__restrict def) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndex(self->blvk_kwds, (DeeStringObject *)name);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListed(self, name))
		goto missing;
	BlackListVarkwds_LockRead(self);
	if unlikely(!self->blvk_argv) {
		BlackListVarkwds_LockEndRead(self);
		goto missing;
	}
	result = self->blvk_argv[index];
	Dee_Incref(result);
	BlackListVarkwds_LockEndRead(self);
	return result;
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN WUNUSED DREF DeeObject *DCALL
BlackListVarkwds_GetItemStringDef(BlackListVarkwds *__restrict self,
                                  char const *__restrict name, dhash_t hash,
                                  DeeObject *__restrict def) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStr(self->blvk_kwds, name, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedString(self, name, hash))
		goto missing;
	BlackListVarkwds_LockRead(self);
	if unlikely(!self->blvk_argv) {
		BlackListVarkwds_LockEndRead(self);
		goto missing;
	}
	result = self->blvk_argv[index];
	Dee_Incref(result);
	BlackListVarkwds_LockEndRead(self);
	return result;
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN WUNUSED DREF DeeObject *DCALL
BlackListVarkwds_GetItemStringLenDef(BlackListVarkwds *__restrict self,
                                     char const *__restrict name,
                                     size_t namesize, dhash_t hash,
                                     DeeObject *__restrict def) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStrLen(self->blvk_kwds, name, namesize, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	BlackListVarkwds_LockRead(self);
	if unlikely(!self->blvk_argv) {
		BlackListVarkwds_LockEndRead(self);
		goto missing;
	}
	result = self->blvk_argv[index];
	Dee_Incref(result);
	BlackListVarkwds_LockEndRead(self);
	return result;
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}


PRIVATE NONNULL((1)) void DCALL
blv_fini(BlackListVarkwds *__restrict self) {
	if (self->blvk_argv) {
		size_t argc;
		argc = DeeKwds_SIZE(self->blvk_kwds);
		Dee_Decrefv(self->blvk_argv, argc);
		Dee_Free(self->blvk_argv);
	}
	Dee_Decref(self->blvk_code);
	Dee_Decref(self->blvk_kwds);
}

PRIVATE NONNULL((1, 2)) void DCALL
blv_visit(BlackListVarkwds *__restrict self, dvisit_t proc, void *arg) {
	BlackListVarkwds_LockRead(self);
	if (self->blvk_argv) {
		size_t argc;
		argc = DeeKwds_SIZE(self->blvk_kwds);
		Dee_Visitv(self->blvk_argv, argc);
	}
	BlackListVarkwds_LockEndRead(self);
	Dee_Visit(self->blvk_code);
	Dee_Visit(self->blvk_kwds);
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
blv_bool(BlackListVarkwds *__restrict self) {
	size_t i;
	DeeKwdsObject *kw = self->blvk_kwds;
	for (i = 0; i <= kw->kw_mask; ++i) {
		DeeStringObject *name;
		name = kw->kw_map[i].ke_name;
		if (!name)
			continue;

		/* Check if this keyword has been black-listed. */
		if (!BlackListVarkwds_IsBlackListed(self, (DeeObject *)name))
			return 1; /* non-empty. */
	}
	return 0; /* empty */
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
blv_nsi_size(BlackListVarkwds *__restrict self) {
	size_t i, result = 0;
	DeeKwdsObject *kw = self->blvk_kwds;
	for (i = 0; i <= kw->kw_mask; ++i) {
		DeeStringObject *name;
		name = kw->kw_map[i].ke_name;
		if (!name)
			continue;

		/* Check if this keyword has been black-listed. */
		if (!BlackListVarkwds_IsBlackListed(self, (DeeObject *)name))
			++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF BlackListVarkwdsIterator *DCALL
blv_iter(BlackListVarkwds *__restrict self) {
	DREF BlackListVarkwdsIterator *result;
	result = DeeObject_MALLOC(BlackListVarkwdsIterator);
	if unlikely(!result)
		goto done;
	result->blki_iter = self->blvk_kwds->kw_map;
	result->blki_end  = result->blki_iter + self->blvk_kwds->kw_mask + 1;
	result->blki_map  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &BlackListVarkwdsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blv_size(BlackListVarkwds *__restrict self) {
	return DeeInt_NewSize(blv_nsi_size(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_contains(BlackListVarkwds *self,
             DeeObject *key) {
	return_bool(likely(DeeString_Check(key)) &&
	            BlackListVarkwds_HasItem(self, key));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blv_getitem(BlackListVarkwds *self,
            DeeObject *key) {
	if unlikely(!DeeString_Check(key)) {
		err_unknown_key((DeeObject *)self, key);
		return NULL;
	}
	return BlackListVarkwds_GetItem(self, key);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
blv_getdefault(BlackListVarkwds *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict def) {
	if unlikely(!DeeString_Check(key)) {
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	return BlackListVarkwds_GetItemDef(self, key, def);
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
	TYPE_MEMBER_CONST(STR_Iterator, &BlackListVarkwdsIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &BlackListVarkwds_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF BlackListVarkwds *DCALL
blv_copy(BlackListVarkwds *__restrict self) {
	DREF BlackListVarkwds *result;
	size_t count;
	result = (DREF BlackListVarkwds *)DeeObject_Malloc(offsetof(BlackListVarkwds, blvk_blck) +
	                                                   (self->blvk_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	count = self->blvk_kwds->kw_size;
	result->blvk_argv = (DREF DeeObject **)Dee_Mallocc(count,
	                                                 sizeof(DREF DeeObject *));
	if unlikely(!result->blvk_argv)
		goto err_r;
	BlackListVarkwds_LockRead(self);
	Dee_Movrefv(result->blvk_argv, self->blvk_argv, count);
	result->blvk_load = self->blvk_load;
	memcpyc(result->blvk_blck,
	        self->blvk_blck,
	        self->blvk_mask + 1,
	        sizeof(BlackListVarkwdsEntry));
	BlackListVarkwds_LockEndRead(self);
	Dee_atomic_rwlock_init(&result->blvk_lock);
	result->blvk_code = self->blvk_code;
	Dee_Incref(result->blvk_code);
	result->blvk_ckwc = self->blvk_ckwc;
	result->blvk_ckwv = self->blvk_ckwv;
	result->blvk_kwds = self->blvk_kwds;
	Dee_Incref(result->blvk_kwds);
	result->blvk_mask = self->blvk_mask;
	DeeObject_Init(result, &BlackListVarkwds_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BlackListVarkwds *DCALL
blv_deep(BlackListVarkwds *__restrict self) {
	DREF BlackListVarkwds *result;
	size_t i, count;
	result = (DREF BlackListVarkwds *)DeeObject_Malloc(offsetof(BlackListVarkwds, blvk_blck) +
	                                                   (self->blvk_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	count = self->blvk_kwds->kw_size;
	result->blvk_argv = (DREF DeeObject **)Dee_Mallocc(count,
	                                                 sizeof(DREF DeeObject *));
	if unlikely(!result->blvk_argv)
		goto err_r;
	BlackListVarkwds_LockRead(self);
	Dee_Movrefv(result->blvk_argv, self->blvk_argv, count);
	result->blvk_load = self->blvk_load;
	memcpyc(result->blvk_blck,
	        self->blvk_blck,
	        self->blvk_mask + 1,
	        sizeof(BlackListVarkwdsEntry));
	BlackListVarkwds_LockEndRead(self);

	/* Construct deep copies of all of the arguments. */
	for (i = 0; i < count; ++i) {
		if (DeeObject_InplaceDeepCopy(&result->blvk_argv[i]))
			goto err_r_argv;
	}
	Dee_atomic_rwlock_init(&result->blvk_lock);
	result->blvk_code = self->blvk_code;
	Dee_Incref(result->blvk_code);
	result->blvk_ckwc = self->blvk_ckwc;
	result->blvk_ckwv = self->blvk_ckwv;
	result->blvk_kwds = self->blvk_kwds;
	Dee_Incref(result->blvk_kwds);
	result->blvk_mask = self->blvk_mask;
	DeeObject_Init(result, &BlackListVarkwds_Type);
done:
	return result;
err_r_argv:
	Dee_Decrefv(result->blvk_argv, count);
	Dee_Free(result->blvk_argv);
err_r:
	DeeObject_Free(result);
	return NULL;
}


INTERN DeeTypeObject BlackListVarkwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListVarkwds",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&blv_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&blv_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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
 * Otherwise, the caller must decref the returned object using `BlackListVarkwds_Decref()' */
INTERN WUNUSED DREF DeeObject *DCALL
BlackListVarkwds_New(struct code_object *__restrict code,
                     size_t positional_argc,
                     DeeKwdsObject *__restrict kwds,
                     DeeObject *const *argv) {
	DREF BlackListVarkwds *result;
	size_t argc, mask;
	if unlikely(!DeeKwds_SIZE(kwds)) {
		/* No keywords --> Return an empty mapping.
		 * -> This can happen depending on how keyword arguments
		 *    have been routed throughout the runtime. */
		Dee_Incref(Dee_EmptyMapping);
		return Dee_EmptyMapping;
	}
	argc = code->co_argc_max;
	if (positional_argc >= argc || !code->co_keywords) {
		/* No keyword information --> Return an unfiltered keywords mapping object.
		 * -> This happens for purely varkwds user-code functions, such a function
		 *    written as `function foo(**kwds)', in which case there aren't any other
		 *    keyword which would have to be blacklisted when access is made. */
		return DeeKwdsMapping_New((DeeObject *)kwds, argv);
	}

	/* Calculate an appropriate mask for the blacklist hash-set. */
	for (mask = 3; mask <= argc; mask = (mask << 1) | 1)
		;
	result = (DREF BlackListVarkwds *)DeeObject_Calloc(offsetof(BlackListVarkwds, blvk_blck) +
	                                                   (mask + 1) * sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	Dee_atomic_rwlock_cinit(&result->blvk_lock);
	result->blvk_code = code; /* Weakly referenced. */
	result->blvk_ckwc = argc - positional_argc;
	result->blvk_ckwv = code->co_keywords + positional_argc;
	result->blvk_kwds = kwds; /* Weakly referenced. */
	result->blvk_argv = (DREF DeeObject **)(DeeObject **)argv; /* Weakly referenced. */
	result->blvk_mask = mask;
	DeeObject_Init(result, &BlackListVarkwds_Type);
done:
	return (DREF DeeObject *)result;
}


/* Unshare the argument vector from a blacklist-varkwds object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
INTERN void DCALL
BlackListVarkwds_Decref(DREF DeeObject *__restrict self) {
	DREF BlackListVarkwds *me;
	size_t kwdc;
	DREF DeeObject **argv;
	me = (DREF BlackListVarkwds *)self;
	if (!DeeObject_IsShared(me)) {
		/* Simple case: user-code didn't share the keyword mapping with its caller,
		 *              so our cleanup process has been greatly simplified. */
		/*Dee_Decref(me->blvk_code);*/ /* Not actually referenced */
		/*Dee_Decref(me->blvk_kwds);*/ /* Not actually referenced */
		Dee_DecrefNokill(&BlackListVarkwds_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
		return;
	}

	/* Must transform the object such that it can continue to exist without causing problems. */
	kwdc = DeeKwds_SIZE(me->blvk_kwds);
	argv = (DREF DeeObject **)Dee_TryMallocc(kwdc, sizeof(DREF DeeObject *));
	if likely(argv) {
		/* Initialize the argument vector copy. */
		Dee_Movrefv(argv, me->blvk_argv, kwdc);
	}

	/* Override the old argv such that the object holds its own copy. */
	BlackListVarkwds_LockWrite(me);
	me->blvk_argv = argv; /* Inherit */
	BlackListVarkwds_LockEndWrite(me);

	/* Construct references to pointed-to objects (done now, so we could
	 * skip that step within the `!DeeObject_IsShared(me)' path above) */
	Dee_Incref(me->blvk_code);
	Dee_Incref(me->blvk_kwds);

	/* Drop our own reference (which should still be shared right now) */
	Dee_Decref_unlikely(self);
}














/* ======================== BlackListMapping ======================== */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blmi_copy(BlackListMappingIterator *__restrict self,
          BlackListMappingIterator *__restrict other) {
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
blmi_fini(BlackListMappingIterator *__restrict self) {
	Dee_Decref(self->mi_iter);
	Dee_Decref(self->mi_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
blmi_visit(BlackListMappingIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mi_iter);
	Dee_Visit(self->mi_map);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blmi_next(BlackListMappingIterator *__restrict self) {
	DREF DeeObject *result, *pair[2];
again:
	result = DeeObject_IterNext(self->mi_iter);
	if (!ITER_ISOK(result))
		goto done;
	if unlikely(DeeObject_Unpack(result, 2, pair))
		goto err_r;
	Dee_Decref(pair[1]);
	if (DeeString_Check(pair[0]) &&
	    BlackListMapping_IsBlackListed(self->mi_map, pair[0])) {
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
blmi_nsi_nextkey(BlackListMappingIterator *__restrict self) {
	DREF DeeObject *result, *pair[2];
again:
	result = DeeObject_IterNext(self->mi_iter);
	if (!ITER_ISOK(result))
		goto done_r;
	if unlikely(DeeObject_Unpack(result, 2, pair))
		goto err_r;
	Dee_Decref(pair[1]);
	if (DeeString_Check(pair[0]) &&
	    BlackListMapping_IsBlackListed(self->mi_map, pair[0])) {
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
blmi_nsi_nextvalue(BlackListMappingIterator *__restrict self) {
	DREF DeeObject *result, *pair[2];
again:
	result = DeeObject_IterNext(self->mi_iter);
	if (!ITER_ISOK(result))
		goto done_r;
	if unlikely(DeeObject_Unpack(result, 2, pair))
		goto err_r;
	if (DeeString_Check(pair[0]) &&
	    BlackListMapping_IsBlackListed(self->mi_map, pair[0])) {
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


#define DEFINE_FILTERITERATOR_COMPARE(name, func)                             \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                     \
	name(BlackListMappingIterator *self, BlackListMappingIterator *other) {   \
		if (DeeObject_AssertTypeExact(other, &BlackListMappingIterator_Type)) \
			goto err;                                                         \
		return func(self->mi_iter, other->mi_iter);                           \
	err:                                                                      \
		return NULL;                                                          \
	}
DEFINE_FILTERITERATOR_COMPARE(blmi_eq, DeeObject_CompareEqObject)
DEFINE_FILTERITERATOR_COMPARE(blmi_ne, DeeObject_CompareNeObject)
DEFINE_FILTERITERATOR_COMPARE(blmi_lo, DeeObject_CompareLoObject)
DEFINE_FILTERITERATOR_COMPARE(blmi_le, DeeObject_CompareLeObject)
DEFINE_FILTERITERATOR_COMPARE(blmi_gr, DeeObject_CompareGrObject)
DEFINE_FILTERITERATOR_COMPARE(blmi_ge, DeeObject_CompareGeObject)
#undef DEFINE_FILTERITERATOR_COMPARE

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
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(BlackListMappingIterator, mi_map), "->?Ert:BlackListMapping"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(BlackListMappingIterator, mi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BlackListMappingIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListMappingIterator",
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
				TYPE_FIXED_ALLOCATOR(BlackListMappingIterator)
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
BlackListMapping_IsBlackListed(BlackListMapping *__restrict self,
                               DeeObject *__restrict name) {
	dhash_t i, perturb;
	DeeStringObject *str;
	dhash_t hash;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
again:
	i = perturb = hash & self->blm_mask;
	for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
		str = self->blm_blck[i & self->blm_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_EqualsSTR(str, name))
			return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blm_load) < self->blm_ckwc) {
		size_t index;
		BlackListMapping_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blm_load);
		while (index < self->blm_ckwc) {
			dhash_t hashof_str;
			str = self->blm_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blm_mask;
			for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->blm_blck[i & self->blm_mask];
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
				self->blm_load = index;
				BlackListMapping_LockEndWrite(self);
				return true;
			}
		}
		self->blm_load = index;
		BlackListMapping_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListMapping_IsBlackListedString(BlackListMapping *__restrict self,
                                     char const *__restrict name,
                                     dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blm_mask;
	for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
		str = self->blm_blck[i & self->blm_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (strcmp(DeeString_STR(str), name) != 0)
			continue;
		return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blm_load) < self->blm_ckwc) {
		size_t index;
		BlackListMapping_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blm_load);
		while (index < self->blm_ckwc) {
			dhash_t hashof_str;
			str = self->blm_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blm_mask;
			for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->blm_blck[i & self->blm_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash &&
			    strcmp(DeeString_STR(str), name) == 0) {
				self->blm_load = index;
				BlackListMapping_LockEndWrite(self);
				return true;
			}
		}
		self->blm_load = index;
		BlackListMapping_LockEndWrite(self);
		goto again;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
BlackListMapping_IsBlackListedStringLen(BlackListMapping *__restrict self,
                                        char const *__restrict name,
                                        size_t namesize, dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->blm_mask;
	for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
		str = self->blm_blck[i & self->blm_mask].blve_str;
		if (!str)
			break;
		if (DeeString_HASH(str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_EqualsBuf(str, name, namesize))
			return true; /* It is! */
	}

	/* Check if more arguments need to be loaded. */
	if (atomic_read(&self->blm_load) < self->blm_ckwc) {
		size_t index;
		BlackListMapping_LockWrite(self);
		COMPILER_READ_BARRIER();
		index = atomic_read(&self->blm_load);
		while (index < self->blm_ckwc) {
			dhash_t hashof_str;
			str = self->blm_ckwv[index++];

			/* Remember the string by caching it within out hash-vector. */
			hashof_str = DeeString_Hash((DeeObject *)str);
			i = perturb = hashof_str & self->blm_mask;
			for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->blm_blck[i & self->blm_mask];
				if (ent->blve_str)
					continue;
				ent->blve_str = str;
				break;
			}

			/* Check if this one's our's, and stop loading if it is. */
			if (hashof_str == hash && DeeString_SIZE(str) == namesize &&
			    bcmpc(DeeString_STR(str), name, namesize, sizeof(char)) == 0) {
				self->blm_load = index;
				BlackListMapping_LockEndWrite(self);
				return true;
			}
		}
		self->blm_load = index;
		BlackListMapping_LockEndWrite(self);
		goto again;
	}
	return false;
}



INTERN WUNUSED NONNULL((1, 2)) int DCALL
BlackListMapping_BoundItem(BlackListMapping *__restrict self,
                           DeeObject *__restrict name,
                           bool allow_missing) {
	if (DeeString_Check(name) &&
	    BlackListMapping_IsBlackListed(self, name)) {
		if (!allow_missing)
			return err_unknown_key((DeeObject *)self, name);
		return -2;
	}
	return DeeObject_BoundItem(self->blm_kw, name, allow_missing);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
BlackListMapping_BoundItemString(BlackListMapping *__restrict self,
                                 char const *__restrict name,
                                 dhash_t hash, bool allow_missing) {
	if (BlackListMapping_IsBlackListedString(self, name, hash)) {
		if (!allow_missing)
			return err_unknown_key_str((DeeObject *)self, name);
		return -2;
	}
	return DeeObject_BoundItemString(self->blm_kw, name, hash, allow_missing);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
BlackListMapping_BoundItemStringLen(BlackListMapping *__restrict self,
                                    char const *__restrict name,
                                    size_t namesize, dhash_t hash,
                                    bool allow_missing) {
	if (BlackListMapping_IsBlackListedStringLen(self, name, namesize, hash)) {
		if (!allow_missing)
			return err_unknown_key_str_len((DeeObject *)self, name, namesize);
		return -2;
	}
	return DeeObject_BoundItemStringLen(self->blm_kw, name, namesize, hash, allow_missing);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
BlackListMapping_GetItem(BlackListMapping *self,
                         DeeObject *name) {
	if (DeeString_Check(name) &&
	    BlackListMapping_IsBlackListed(self, name))
		goto missing;
	return DeeObject_GetItem(self->blm_kw, name);
missing:
	err_unknown_key((DeeObject *)self, name);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
BlackListMapping_GetItemString(BlackListMapping *__restrict self,
                               char const *__restrict name, dhash_t hash) {
	if (BlackListMapping_IsBlackListedString(self, name, hash))
		goto missing;
	return DeeObject_GetItemString(self->blm_kw, name, hash);
missing:
	err_unknown_key_str((DeeObject *)self, name);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
BlackListMapping_GetItemStringLen(BlackListMapping *__restrict self,
                                  char const *__restrict name,
                                  size_t namesize, dhash_t hash) {
	if (BlackListMapping_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	return DeeObject_GetItemStringLen(self->blm_kw, name, namesize, hash);
missing:
	err_unknown_key_str_len((DeeObject *)self, name, namesize);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
BlackListMapping_GetItemDef(BlackListMapping *__restrict self,
                            DeeObject *__restrict name,
                            DeeObject *__restrict def) {
	if (DeeString_Check(name) &&
	    BlackListMapping_IsBlackListed(self, name))
		goto missing;
	return DeeObject_GetItemDef(self->blm_kw, name, def);
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN WUNUSED DREF DeeObject *DCALL
BlackListMapping_GetItemStringDef(BlackListMapping *__restrict self,
                                  char const *__restrict name, dhash_t hash,
                                  DeeObject *__restrict def) {
	if (BlackListMapping_IsBlackListedString(self, name, hash))
		goto missing;
	return DeeObject_GetItemStringDef(self->blm_kw, name, hash, def);
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN WUNUSED DREF DeeObject *DCALL
BlackListMapping_GetItemStringLenDef(BlackListMapping *__restrict self,
                                     char const *__restrict name,
                                     size_t namesize, dhash_t hash,
                                     DeeObject *__restrict def) {
	if (BlackListMapping_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	return DeeObject_GetItemStringLenDef(self->blm_kw, name, namesize, hash, def);
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

PRIVATE NONNULL((1)) void DCALL
blm_fini(BlackListMapping *__restrict self) {
	Dee_Decref(self->blm_code);
	Dee_Decref(self->blm_kw);
}

PRIVATE NONNULL((1, 2)) void DCALL
blm_visit(BlackListMapping *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->blm_code);
	Dee_Visit(self->blm_kw);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
blm_bool(BlackListMapping *__restrict self) {
	DREF DeeObject *vals[2], *elem, *iter;
	iter = DeeObject_IterSelf(self->blm_kw);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		int error;
		error = DeeObject_Unpack(elem, 2, vals);
		Dee_Decref(elem);
		if unlikely(error)
			goto err_iter;
		Dee_Decref(vals[1]);
		if (!DeeString_Check(vals[0]) ||
		    !BlackListMapping_IsBlackListed(self, vals[0])) {
			Dee_Decref(vals[0]);
			return 1;
		}
		Dee_Decref(vals[0]);
	}
	if unlikely(!elem)
		goto err_iter;
	Dee_Decref(iter);
	return 0;
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
blm_nsi_size(BlackListMapping *__restrict self) {
	size_t result = 0;
	DREF DeeObject *vals[2], *elem, *iter;
	iter = DeeObject_IterSelf(self->blm_kw);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		int error;
		error = DeeObject_Unpack(elem, 2, vals);
		Dee_Decref(elem);
		if unlikely(error)
			goto err_iter;
		Dee_Decref(vals[1]);
		if (!DeeString_Check(vals[0]) ||
		    !BlackListMapping_IsBlackListed(self, vals[0]))
			++result;
		Dee_Decref(vals[0]);
	}
	if unlikely(!elem)
		goto err_iter;
	Dee_Decref(iter);
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return (size_t)-1;
}


PRIVATE WUNUSED NONNULL((1)) DREF BlackListMappingIterator *DCALL
blm_iter(BlackListMapping *__restrict self) {
	DREF BlackListMappingIterator *result;
	result = DeeObject_MALLOC(BlackListMappingIterator);
	if unlikely(!result)
		goto done;
	result->mi_iter = DeeObject_IterSelf(self->blm_kw);
	if unlikely(!result->mi_iter)
		goto err_r;
	result->mi_map = self;
	Dee_Incref(self);
	DeeObject_Init(result, &BlackListMappingIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blm_size(BlackListMapping *__restrict self) {
	return DeeInt_NewSize(blm_nsi_size(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
blm_contains(BlackListMapping *self,
             DeeObject *key) {
	if (DeeString_Check(key) &&
	    BlackListMapping_IsBlackListed(self, key))
		return_false;
	return DeeObject_ContainsObject(self->blm_kw, key);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
blm_getdefault(BlackListMapping *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict def) {
	if unlikely(!DeeString_Check(key)) {
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	return BlackListMapping_GetItemDef(self, key, def);
}

PRIVATE struct type_nsi tpconst blm_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&blm_nsi_size,
			/* .nsi_nextkey    = */ (dfunptr_t)&blmi_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&blmi_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&blm_getdefault
		}
	}
};

PRIVATE struct type_seq blm_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blm_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blm_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&blm_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&BlackListMapping_GetItem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &blm_nsi
};

PRIVATE WUNUSED NONNULL((1)) DREF BlackListMapping *DCALL
blm_copy(BlackListMapping *__restrict self) {
	DREF DeeObject *result_kw;
	DREF BlackListMapping *result;
	/* Create a copy of the original keywords, since we're acting as a proxy. */
	result_kw = DeeObject_Copy(self->blm_kw);
	if unlikely(!result_kw)
		goto err;
	if (result_kw == self->blm_kw) {
		Dee_DecrefNokill(result_kw);
		return_reference_(self);
	}

	result = (DREF BlackListMapping *)DeeObject_Malloc(offsetof(BlackListMapping, blm_blck) +
	                                                   (self->blm_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto err_result_kw;
	result->blm_kw = result_kw; /* Inherit reference */
	Dee_atomic_rwlock_init(&result->blm_lock);
	result->blm_code = self->blm_code;
	Dee_Incref(result->blm_code);
	result->blm_ckwc = self->blm_ckwc;
	result->blm_ckwv = self->blm_ckwv;
	result->blm_mask = self->blm_mask;
	BlackListMapping_LockRead(self);
	result->blm_load = self->blm_load;
	memcpyc(result->blm_blck,
	        self->blm_blck,
	        result->blm_mask + 1,
	        sizeof(BlackListVarkwdsEntry));
	BlackListMapping_LockEndRead(self);
	DeeObject_Init(result, &BlackListMapping_Type);
	return result;
err_result_kw:
	Dee_Decref(result_kw);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BlackListMapping *DCALL
blm_deep(BlackListMapping *__restrict self) {
	DREF BlackListMapping *result;
	result = (DREF BlackListMapping *)DeeObject_Malloc(offsetof(BlackListMapping, blm_blck) +
	                                                   (self->blm_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	result->blm_kw = DeeObject_DeepCopy(self->blm_kw);
	if unlikely(!result->blm_kw)
		goto err_r;
	Dee_atomic_rwlock_init(&result->blm_lock);
	result->blm_code = self->blm_code; /* Immutable, so no copy required */
	Dee_Incref(result->blm_code);
	result->blm_ckwc = self->blm_ckwc;
	result->blm_ckwv = self->blm_ckwv;
	result->blm_mask = self->blm_mask;
	BlackListMapping_LockRead(self);
	result->blm_load = self->blm_load;
	memcpyc(result->blm_blck,
	        self->blm_blck,
	        result->blm_mask + 1,
	        sizeof(BlackListVarkwdsEntry));
	BlackListMapping_LockEndRead(self);
	DeeObject_Init(result, &BlackListMapping_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF BlackListMapping *DCALL
blm_get_frozen(BlackListMapping *__restrict self) {
	DREF DeeObject *frozen_kw;
	DREF BlackListMapping *result;
	frozen_kw = DeeObject_GetAttr(self->blm_kw, (DeeObject *)&str_frozen);
	if unlikely(!frozen_kw)
		goto err;
	if (frozen_kw == self->blm_kw) {
		Dee_DecrefNokill(frozen_kw);
		return_reference_(self);
	}

	result = (DREF BlackListMapping *)DeeObject_Malloc(offsetof(BlackListMapping, blm_blck) +
	                                                   (self->blm_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto err_frozen_kw;

	result->blm_kw = frozen_kw; /* Inherit reference */
	Dee_atomic_rwlock_init(&result->blm_lock);
	result->blm_code = self->blm_code;
	Dee_Incref(result->blm_code);
	result->blm_ckwc = self->blm_ckwc;
	result->blm_ckwv = self->blm_ckwv;
	result->blm_mask = self->blm_mask;
	BlackListMapping_LockRead(self);
	result->blm_load = self->blm_load;
	memcpyc(result->blm_blck,
	        self->blm_blck,
	        result->blm_mask + 1,
	        sizeof(BlackListVarkwdsEntry));
	BlackListMapping_LockEndRead(self);
	DeeObject_Init(result, &BlackListMapping_Type);
	return result;
err_frozen_kw:
	Dee_Decref(frozen_kw);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst blm_getsets[] = {
	TYPE_GETTER(STR_frozen, &blm_get_frozen, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst blm_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BlackListMappingIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BlackListMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListMapping",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&blm_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&blm_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blm_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&blm_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blm_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &blm_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ blm_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ blm_class_members
};


INTERN WUNUSED DREF DeeObject *DCALL
BlackListMapping_New(struct code_object *__restrict code,
                     size_t positional_argc,
                     DeeObject *__restrict kw) {
	DREF BlackListMapping *result;
	size_t argc, mask;
	argc = code->co_argc_max;
	if (positional_argc >= argc || !code->co_keywords) {
		/* No keyword information --> Re-return the unfiltered input mapping object. */
		return_reference_(kw);
	}

	/* Calculate an appropriate mask for the blacklist hash-set. */
	for (mask = 3; mask <= argc; mask = (mask << 1) | 1)
		;
	result = (DREF BlackListMapping *)DeeObject_Calloc(offsetof(BlackListMapping, blm_blck) +
	                                                   (mask + 1) * sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	Dee_atomic_rwlock_cinit(&result->blm_lock);
	result->blm_code = code;
	result->blm_ckwc = argc - positional_argc;
	result->blm_ckwv = code->co_keywords + positional_argc;
	result->blm_kw   = kw;
	result->blm_mask = mask;
	Dee_Incref(code);
	Dee_Incref(kw);
	DeeObject_Init(result, &BlackListMapping_Type);
done:
	return (DREF DeeObject *)result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_C */
