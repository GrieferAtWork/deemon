/* Copyright (c) 2019 Griefer@Work                                            *
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
#include <deemon/tuple.h>
#include <deemon/util/rwlock.h>
#include <deemon/util/string.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

#ifdef CONFIG_NO_THREADS
#define BLVI_GETITER(self)             (self)->ki_iter
#else /* CONFIG_NO_THREADS */
#define BLVI_GETITER(self) ATOMIC_READ((self)->ki_iter)
#endif /* !CONFIG_NO_THREADS */

PRIVATE int DCALL
blvi_copy(BlackListVarkwdsIterator *__restrict self,
          BlackListVarkwdsIterator *__restrict other) {
	self->ki_iter = BLVI_GETITER(other);
	self->ki_end  = other->ki_end;
	self->ki_map  = other->ki_map;
	Dee_Incref(self->ki_map);
	return 0;
}

PRIVATE int DCALL
blvi_deep(BlackListVarkwdsIterator *__restrict self,
          BlackListVarkwdsIterator *__restrict other) {
	self->ki_iter = BLVI_GETITER(other);
	self->ki_end  = other->ki_end;
	self->ki_map  = (DREF BlackListVarkwds *)DeeObject_DeepCopy((DeeObject *)other->ki_map);
	if unlikely(!self->ki_map)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE void DCALL
blvi_fini(BlackListVarkwdsIterator *__restrict self) {
	Dee_Decref(self->ki_map);
}

PRIVATE void DCALL
blvi_visit(BlackListVarkwdsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ki_map);
}


LOCAL struct kwds_entry *DCALL
blvi_nextiter(BlackListVarkwdsIterator *__restrict self) {
	struct kwds_entry *iter;
#ifndef CONFIG_NO_THREADS
	struct kwds_entry *old_iter;
again:
#endif /* !CONFIG_NO_THREADS */
	iter = BLVI_GETITER(self);
#ifndef CONFIG_NO_THREADS
	old_iter = iter;
#endif /* !CONFIG_NO_THREADS */
	for (;;) {
		if (iter >= self->ki_end)
			return NULL;
		if (iter->ke_name) {
			/* Skip values which have been black-listed. */
			if (!BlackListVarkwds_IsBlackListed(self->ki_map, (DeeObject *)iter->ke_name))
				break;
		}
		++iter;
	}
#ifndef CONFIG_NO_THREADS
	if (!ATOMIC_CMPXCH_WEAK(self->ki_iter, old_iter, iter + 1))
		goto again;
#else  /* !CONFIG_NO_THREADS */
	self->ki_iter = iter + 1;
#endif /* CONFIG_NO_THREADS */
	return iter;
}

PRIVATE DREF DeeObject *DCALL
blvi_next(BlackListVarkwdsIterator *__restrict self) {
	DREF DeeObject *value, *result;
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return ITER_DONE;
	rwlock_read(&self->ki_map->vk_lock);
	if unlikely(!self->ki_map->vk_argv) {
		rwlock_endread(&self->ki_map->vk_lock);
		return ITER_DONE;
	}
	value = self->ki_map->vk_argv[ent->ke_index];
	Dee_Incref(value);
	rwlock_endread(&self->ki_map->vk_lock);
	result = DeeTuple_Pack(2, ent->ke_name, value);
	Dee_Decref_unlikely(value);
	return result;
}

PRIVATE DREF DeeStringObject *DCALL
blvi_nsi_nextkey(BlackListVarkwdsIterator *__restrict self) {
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return (DREF DeeStringObject *)ITER_DONE;
	return_reference_(ent->ke_name);
}

PRIVATE DREF DeeObject *DCALL
blvi_nsi_nextvalue(BlackListVarkwdsIterator *__restrict self) {
	DREF DeeObject *result;
	struct kwds_entry *ent;
	ent = blvi_nextiter(self);
	if (!ent)
		return ITER_DONE;
	rwlock_read(&self->ki_map->vk_lock);
	if unlikely(!self->ki_map->vk_argv) {
		rwlock_endread(&self->ki_map->vk_lock);
		return ITER_DONE;
	}
	result = self->ki_map->vk_argv[ent->ke_index];
	Dee_Incref(result);
	rwlock_endread(&self->ki_map->vk_lock);
	return result;
}


#define DEFINE_FILTERITERATOR_COMPARE(name, op)                               \
	PRIVATE DREF DeeObject *DCALL                                             \
	name(BlackListVarkwdsIterator *__restrict self,                           \
	     BlackListVarkwdsIterator *__restrict other) {                        \
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
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blvi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blvi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blvi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blvi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blvi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blvi_ge,
};

PRIVATE struct type_member blvi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BlackListVarkwdsIterator, ki_map), "->?Ert:BlackListVarkwds"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BlackListVarkwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListVarkwdsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)NULL,
				/* .tp_copy_ctor = */ (void *)&blvi_copy,
				/* .tp_deep_ctor = */ (void *)&blvi_deep,
				/* .tp_any_ctor  = */ (void *)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(BlackListVarkwdsIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&blvi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL /* XXX: Could easily be implemented... */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blvi_visit,
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
		if (memcmp(DeeString_STR(entry->ke_name),
		           DeeString_STR(name),
		           DeeString_SIZE(name) * sizeof(char)) != 0)
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
		if (memcmp(DeeString_STR(entry->ke_name), name, namesize * sizeof(char)) != 0)
			continue;
		return entry->ke_index;
	}
	return (size_t)-1;
}




INTERN bool DCALL
BlackListVarkwds_IsBlackListed(BlackListVarkwds *__restrict self,
                               DeeObject *__restrict name) {
	dhash_t i, perturb;
	DeeStringObject *str;
	dhash_t hash;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
again:
	i = perturb = hash & self->vk_mask;
	for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
		str = self->vk_blck[i & self->vk_mask].ve_str;
		if (!str)
			break;
		if (DeeString_HASH((DeeObject *)str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_SIZE((DeeObject *)str) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(str),
		           DeeString_STR(name),
		           DeeString_SIZE(name) *
		           sizeof(char)) != 0)
			continue;
		return true; /* It is! */
	}
	/* Check if more arguments need to be loaded. */
	if (ATOMIC_READ(self->vk_load) < self->vk_ckwc) {
		size_t index;
		rwlock_write(&self->vk_lock);
		COMPILER_READ_BARRIER();
		index = ATOMIC_READ(self->vk_load);
		while (index < self->vk_ckwc) {
			dhash_t str_hash;
			str = self->vk_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			str_hash = DeeString_Hash((DeeObject *)str);
			i = perturb = str_hash & self->vk_mask;
			for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->vk_blck[i & self->vk_mask];
				if (ent->ve_str)
					continue;
				ent->ve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (str_hash == hash &&
			    DeeString_SIZE(str) == DeeString_SIZE(name) &&
			    memcmp(DeeString_STR(str),
			           DeeString_STR(name),
			           DeeString_SIZE(name) *
			           sizeof(char)) == 0) {
				self->vk_load = index;
				rwlock_endwrite(&self->vk_lock);
				return true;
			}
		}
		self->vk_load = index;
		rwlock_endwrite(&self->vk_lock);
		goto again;
	}
	return false;
}

INTERN bool DCALL
BlackListVarkwds_IsBlackListedString(BlackListVarkwds *__restrict self,
                                     char const *__restrict name,
                                     dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->vk_mask;
	for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
		str = self->vk_blck[i & self->vk_mask].ve_str;
		if (!str)
			break;
		if (DeeString_HASH((DeeObject *)str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (strcmp(DeeString_STR(str), name) != 0)
			continue;
		return true; /* It is! */
	}
	/* Check if more arguments need to be loaded. */
	if (ATOMIC_READ(self->vk_load) < self->vk_ckwc) {
		size_t index;
		rwlock_write(&self->vk_lock);
		COMPILER_READ_BARRIER();
		index = ATOMIC_READ(self->vk_load);
		while (index < self->vk_ckwc) {
			dhash_t str_hash;
			str = self->vk_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			str_hash = DeeString_Hash((DeeObject *)str);
			i = perturb = str_hash & self->vk_mask;
			for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->vk_blck[i & self->vk_mask];
				if (ent->ve_str)
					continue;
				ent->ve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (str_hash == hash &&
			    strcmp(DeeString_STR(str), name) == 0) {
				self->vk_load = index;
				rwlock_endwrite(&self->vk_lock);
				return true;
			}
		}
		self->vk_load = index;
		rwlock_endwrite(&self->vk_lock);
		goto again;
	}
	return false;
}

INTERN bool DCALL
BlackListVarkwds_IsBlackListedStringLen(BlackListVarkwds *__restrict self,
                                        char const *__restrict name,
                                        size_t namesize, dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->vk_mask;
	for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
		str = self->vk_blck[i & self->vk_mask].ve_str;
		if (!str)
			break;
		if (DeeString_HASH((DeeObject *)str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_SIZE((DeeObject *)str) != namesize)
			continue;
		if (memcmp(DeeString_STR(str), name, namesize * sizeof(char)) != 0)
			continue;
		return true; /* It is! */
	}
	/* Check if more arguments need to be loaded. */
	if (ATOMIC_READ(self->vk_load) < self->vk_ckwc) {
		size_t index;
		rwlock_write(&self->vk_lock);
		COMPILER_READ_BARRIER();
		index = ATOMIC_READ(self->vk_load);
		while (index < self->vk_ckwc) {
			dhash_t str_hash;
			str = self->vk_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			str_hash = DeeString_Hash((DeeObject *)str);
			i = perturb = str_hash & self->vk_mask;
			for (;; BlackListVarkwds_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->vk_blck[i & self->vk_mask];
				if (ent->ve_str)
					continue;
				ent->ve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (str_hash == hash &&
			    DeeString_SIZE(str) == namesize &&
			    memcmp(DeeString_STR(str), name, namesize * sizeof(char)) == 0) {
				self->vk_load = index;
				rwlock_endwrite(&self->vk_lock);
				return true;
			}
		}
		self->vk_load = index;
		rwlock_endwrite(&self->vk_lock);
		goto again;
	}
	return false;
}



INTERN bool DCALL
BlackListVarkwds_HasItem(BlackListVarkwds *__restrict self,
                         DeeObject *__restrict name) {
	return (kwds_FindIndex(self->vk_kwds, (DeeStringObject *)name) != (size_t)-1) &&
	       !BlackListVarkwds_IsBlackListed(self, name) &&
	       ATOMIC_READ(self->vk_argv) != NULL;
}

INTERN bool DCALL
BlackListVarkwds_HasItemString(BlackListVarkwds *__restrict self,
                               char const *__restrict name,
                               dhash_t hash) {
	return (kwds_FindIndexStr(self->vk_kwds, name, hash) != (size_t)-1) &&
	       !BlackListVarkwds_IsBlackListedString(self, name, hash) &&
	       ATOMIC_READ(self->vk_argv) != NULL;
}

INTERN bool DCALL
BlackListVarkwds_HasItemStringLen(BlackListVarkwds *__restrict self,
                                  char const *__restrict name,
                                  size_t namesize, dhash_t hash) {
	return (kwds_FindIndexStrLen(self->vk_kwds, name, namesize, hash) != (size_t)-1) &&
	       !BlackListVarkwds_IsBlackListedStringLen(self, name, namesize, hash) &&
	       ATOMIC_READ(self->vk_argv) != NULL;
}

INTERN DREF DeeObject *DCALL
BlackListVarkwds_GetItem(BlackListVarkwds *__restrict self,
                         DeeObject *__restrict name) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndex(self->vk_kwds, (DeeStringObject *)name);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListed(self, name))
		goto missing;
	rwlock_read(&self->vk_lock);
	if unlikely(!self->vk_argv) {
		rwlock_endread(&self->vk_lock);
		goto missing;
	}
	result = self->vk_argv[index];
	Dee_Incref(result);
	rwlock_endread(&self->vk_lock);
	return result;
missing:
	err_unknown_key((DeeObject *)self, name);
	return NULL;
}

INTERN DREF DeeObject *DCALL
BlackListVarkwds_GetItemString(BlackListVarkwds *__restrict self,
                               char const *__restrict name, dhash_t hash) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStr(self->vk_kwds, name, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedString(self, name, hash))
		goto missing;
	rwlock_read(&self->vk_lock);
	if unlikely(!self->vk_argv) {
		rwlock_endread(&self->vk_lock);
		goto missing;
	}
	result = self->vk_argv[index];
	Dee_Incref(result);
	rwlock_endread(&self->vk_lock);
	return result;
missing:
	err_unknown_key_str((DeeObject *)self, name);
	return NULL;
}

INTERN DREF DeeObject *DCALL
BlackListVarkwds_GetItemStringLen(BlackListVarkwds *__restrict self,
                                  char const *__restrict name,
                                  size_t namesize, dhash_t hash) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStrLen(self->vk_kwds, name, namesize, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	rwlock_read(&self->vk_lock);
	if unlikely(!self->vk_argv) {
		rwlock_endread(&self->vk_lock);
		goto missing;
	}
	result = self->vk_argv[index];
	Dee_Incref(result);
	rwlock_endread(&self->vk_lock);
	return result;
missing:
	err_unknown_key_str_len((DeeObject *)self, name, namesize);
	return NULL;
}

INTERN DREF DeeObject *DCALL
BlackListVarkwds_GetItemDef(BlackListVarkwds *__restrict self,
                            DeeObject *__restrict name,
                            DeeObject *__restrict def) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndex(self->vk_kwds, (DeeStringObject *)name);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListed(self, name))
		goto missing;
	rwlock_read(&self->vk_lock);
	if unlikely(!self->vk_argv) {
		rwlock_endread(&self->vk_lock);
		goto missing;
	}
	result = self->vk_argv[index];
	Dee_Incref(result);
	rwlock_endread(&self->vk_lock);
	return result;
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN DREF DeeObject *DCALL
BlackListVarkwds_GetItemStringDef(BlackListVarkwds *__restrict self,
                                  char const *__restrict name, dhash_t hash,
                                  DeeObject *__restrict def) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStr(self->vk_kwds, name, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedString(self, name, hash))
		goto missing;
	rwlock_read(&self->vk_lock);
	if unlikely(!self->vk_argv) {
		rwlock_endread(&self->vk_lock);
		goto missing;
	}
	result = self->vk_argv[index];
	Dee_Incref(result);
	rwlock_endread(&self->vk_lock);
	return result;
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN DREF DeeObject *DCALL
BlackListVarkwds_GetItemStringLenDef(BlackListVarkwds *__restrict self,
                                     char const *__restrict name,
                                     size_t namesize, dhash_t hash,
                                     DeeObject *__restrict def) {
	size_t index;
	DREF DeeObject *result;
	index = kwds_FindIndexStrLen(self->vk_kwds, name, namesize, hash);
	if unlikely(index == (size_t)-1)
		goto missing;
	if unlikely(BlackListVarkwds_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	rwlock_read(&self->vk_lock);
	if unlikely(!self->vk_argv) {
		rwlock_endread(&self->vk_lock);
		goto missing;
	}
	result = self->vk_argv[index];
	Dee_Incref(result);
	rwlock_endread(&self->vk_lock);
	return result;
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}


PRIVATE void DCALL
blv_fini(BlackListVarkwds *__restrict self) {
	if (self->vk_argv) {
		size_t i, argc;
		argc = DeeKwds_SIZE(self->vk_kwds);
		for (i = 0; i < argc; ++i)
			Dee_Decref(self->vk_argv[i]);
		Dee_Free(self->vk_argv);
	}
	Dee_Decref(self->vk_code);
	Dee_Decref(self->vk_kwds);
}

PRIVATE void DCALL
blv_visit(BlackListVarkwds *__restrict self, dvisit_t proc, void *arg) {
	rwlock_read(&self->vk_lock);
	if (self->vk_argv) {
		size_t i, argc;
		argc = DeeKwds_SIZE(self->vk_kwds);
		for (i = 0; i < argc; ++i)
			Dee_Visit(self->vk_argv[i]);
	}
	rwlock_endread(&self->vk_lock);
	Dee_Visit(self->vk_code);
	Dee_Visit(self->vk_kwds);
}



PRIVATE int DCALL
blv_bool(BlackListVarkwds *__restrict self) {
	size_t i;
	DeeKwdsObject *kw = self->vk_kwds;
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

PRIVATE size_t DCALL
blv_nsi_size(BlackListVarkwds *__restrict self) {
	size_t i, result = 0;
	DeeKwdsObject *kw = self->vk_kwds;
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

PRIVATE DREF BlackListVarkwdsIterator *DCALL
blv_iter(BlackListVarkwds *__restrict self) {
	DREF BlackListVarkwdsIterator *result;
	result = DeeObject_MALLOC(BlackListVarkwdsIterator);
	if unlikely(!result)
		goto done;
	result->ki_iter = self->vk_kwds->kw_map;
	result->ki_end  = result->ki_iter + self->vk_kwds->kw_mask + 1;
	result->ki_map  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &BlackListVarkwdsIterator_Type);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
blv_size(BlackListVarkwds *__restrict self) {
	return DeeInt_NewSize(blv_nsi_size(self));
}

PRIVATE DREF DeeObject *DCALL
blv_contains(BlackListVarkwds *__restrict self,
             DeeObject *__restrict key) {
	return_bool(likely(DeeString_Check(key)) &&
	            BlackListVarkwds_HasItem(self, key));
}

PRIVATE DREF DeeObject *DCALL
blv_getitem(BlackListVarkwds *__restrict self,
            DeeObject *__restrict key) {
	if unlikely(!DeeString_Check(key)) {
		err_unknown_key((DeeObject *)self, key);
		return NULL;
	}
	return BlackListVarkwds_GetItem(self, key);
}

PRIVATE DREF DeeObject *DCALL
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

PRIVATE struct type_nsi blv_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (void *)&blv_nsi_size,
			/* .nsi_nextkey    = */ (void *)&blvi_nsi_nextkey,
			/* .nsi_nextvalue  = */ (void *)&blvi_nsi_nextvalue,
			/* .nsi_getdefault = */ (void *)&blv_getdefault
		}
	}
};

PRIVATE struct type_seq blv_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blv_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blv_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blv_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blv_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &blv_nsi
};

PRIVATE struct type_member blv_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &BlackListVarkwdsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE DREF BlackListVarkwds *DCALL
blv_copy(BlackListVarkwds *__restrict self) {
	DREF BlackListVarkwds *result;
	size_t i, count;
	result = (DREF BlackListVarkwds *)DeeObject_Malloc(offsetof(BlackListVarkwds, vk_blck) +
	                                                   (self->vk_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	count           = self->vk_kwds->kw_size;
	result->vk_argv = (DREF DeeObject **)Dee_Malloc(count *
	                                                sizeof(DREF DeeObject *));
	if unlikely(!result->vk_argv)
		goto err_r;
	rwlock_read(&self->vk_lock);
	for (i = 0; i < count; ++i) {
		result->vk_argv[i] = self->vk_argv[i];
		Dee_Incref(result->vk_argv[i]);
	}
	result->vk_load = self->vk_load;
	memcpy(result->vk_blck, self->vk_blck,
	       (self->vk_mask + 1) *
	       sizeof(BlackListVarkwdsEntry));
	rwlock_endread(&self->vk_lock);
	rwlock_init(&result->vk_lock);
	result->vk_code = self->vk_code;
	Dee_Incref(result->vk_code);
	result->vk_ckwc = self->vk_ckwc;
	result->vk_ckwv = self->vk_ckwv;
	result->vk_kwds = self->vk_kwds;
	Dee_Incref(result->vk_kwds);
	result->vk_mask = self->vk_mask;
	DeeObject_Init(result, &BlackListVarkwds_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}

PRIVATE DREF BlackListVarkwds *DCALL
blv_deep(BlackListVarkwds *__restrict self) {
	DREF BlackListVarkwds *result;
	size_t i, count;
	result = (DREF BlackListVarkwds *)DeeObject_Malloc(offsetof(BlackListVarkwds, vk_blck) +
	                                                   (self->vk_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	count           = self->vk_kwds->kw_size;
	result->vk_argv = (DREF DeeObject **)Dee_Malloc(count *
	                                                sizeof(DREF DeeObject *));
	if unlikely(!result->vk_argv)
		goto err_r;
	rwlock_read(&self->vk_lock);
	for (i = 0; i < count; ++i) {
		result->vk_argv[i] = self->vk_argv[i];
		Dee_Incref(result->vk_argv[i]);
	}
	result->vk_load = self->vk_load;
	memcpy(result->vk_blck, self->vk_blck,
	       (self->vk_mask + 1) *
	       sizeof(BlackListVarkwdsEntry));
	rwlock_endread(&self->vk_lock);
	/* Construct deep copies of all of the arguments. */
	for (i = 0; i < count; ++i) {
		if (DeeObject_InplaceDeepCopy(&result->vk_argv[i]))
			goto err_r_argv;
	}
	rwlock_init(&result->vk_lock);
	result->vk_code = self->vk_code;
	Dee_Incref(result->vk_code);
	result->vk_ckwc = self->vk_ckwc;
	result->vk_ckwv = self->vk_ckwv;
	result->vk_kwds = self->vk_kwds;
	Dee_Incref(result->vk_kwds);
	result->vk_mask = self->vk_mask;
	DeeObject_Init(result, &BlackListVarkwds_Type);
done:
	return result;
err_r_argv:
	i = count;
	while (i--)
		Dee_Decref(result->vk_argv[i]);
	Dee_Free(result->vk_argv);
err_r:
	DeeObject_Free(result);
	return NULL;
}


INTERN DeeTypeObject BlackListVarkwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListVarkwds",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL|TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (void *)NULL,
				/* .tp_copy_ctor = */ (void *)&blv_copy,
				/* .tp_deep_ctor = */ (void *)&blv_deep,
				/* .tp_any_ctor  = */ (void *)NULL,
				/* .tp_free      = */ (void *)NULL
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&blv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&blv_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &blv_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */blv_class_members
};


/* Construct a new mapping for keywords that follows the black-listing scheme.
 * NOTE: If `kwds' is empty, return `Dee_EmptyMapping' instead.
 * NOTE: If `code' doesn't specify any keywords, return `DeeKwdsMapping_New()' instead.
 * Otherwise, the caller must decref the returned object using `BlackListVarkwds_Decref()' */
INTERN DREF DeeObject *DCALL
BlackListVarkwds_New(struct code_object *__restrict code,
                     size_t positional_argc,
                     DeeKwdsObject *__restrict kwds,
                     DeeObject **__restrict argv) {
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
	result = (DREF BlackListVarkwds *)DeeObject_Calloc(offsetof(BlackListVarkwds, vk_blck) +
	                                                   (mask + 1) * sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	rwlock_cinit(&result->vk_lock);
	result->vk_code = code; /* Weakly referenced. */
	result->vk_ckwc = argc - positional_argc;
	result->vk_ckwv = code->co_keywords + positional_argc;
	result->vk_kwds = kwds; /* Weakly referenced. */
	result->vk_argv = argv; /* Weakly referenced. */
	result->vk_mask = mask;
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
		/*Dee_Decref(me->vk_code);*/ /* Not actually referenced */
		/*Dee_Decref(me->vk_kwds);*/ /* Not actually referenced */
		Dee_DecrefNokill(&BlackListVarkwds_Type);
		DeeObject_FreeTracker(me);
		DeeObject_Free(me);
		return;
	}
	/* Must transform the object such that it can continue to exist without causing problems. */
	kwdc = DeeKwds_SIZE(me->vk_kwds);
	argv = (DREF DeeObject **)Dee_TryMalloc(kwdc * sizeof(DREF DeeObject *));
	if likely(argv) {
		size_t i;
		/* Initialize the argument vector copy. */
		MEMCPY_PTR(argv, me->vk_argv, kwdc);
		for (i = 0; i < kwdc; ++i)
			Dee_Incref(argv[i]);
	}
	/* Override the old argv such that the object holds its own copy. */
	rwlock_write(&me->vk_lock);
	me->vk_argv = argv; /* Inherit */
	rwlock_endwrite(&me->vk_lock);
	/* Construct references to pointed-to objects (done now, so we could
	 * skip that step within the `!DeeObject_IsShared(me)' path above) */
	Dee_Incref(me->vk_code);
	Dee_Incref(me->vk_kwds);
	/* Drop our own reference (which should still be shared right now) */
	Dee_Decref_unlikely(self);
}































/* ======================== BlackListMapping ======================== */
PRIVATE int DCALL
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

PRIVATE void DCALL
blmi_fini(BlackListMappingIterator *__restrict self) {
	Dee_Decref(self->mi_iter);
	Dee_Decref(self->mi_map);
}

PRIVATE void DCALL
blmi_visit(BlackListMappingIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mi_iter);
	Dee_Visit(self->mi_map);
}


PRIVATE DREF DeeObject *DCALL
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

PRIVATE DREF DeeObject *DCALL
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

PRIVATE DREF DeeObject *DCALL
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
	PRIVATE DREF DeeObject *DCALL                                             \
	name(BlackListMappingIterator *__restrict self,                           \
	     BlackListMappingIterator *__restrict other) {                        \
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
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blmi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blmi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blmi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blmi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blmi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blmi_ge,
};

PRIVATE struct type_member blmi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BlackListMappingIterator, mi_map), "->?Ert:BlackListMapping"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(BlackListMappingIterator, mi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BlackListMappingIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListMappingIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ &blmi_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(BlackListMappingIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&blmi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blmi_visit,
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

INTERN bool DCALL
BlackListMapping_IsBlackListed(BlackListMapping *__restrict self,
                               DeeObject *__restrict name) {
	dhash_t i, perturb;
	DeeStringObject *str;
	dhash_t hash;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
again:
	i = perturb = hash & self->bm_mask;
	for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
		str = self->bm_blck[i & self->bm_mask].ve_str;
		if (!str)
			break;
		if (DeeString_HASH((DeeObject *)str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_SIZE((DeeObject *)str) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(str),
		           DeeString_STR(name),
		           DeeString_SIZE(name) *
		           sizeof(char)) != 0)
			continue;
		return true; /* It is! */
	}
	/* Check if more arguments need to be loaded. */
	if (ATOMIC_READ(self->bm_load) < self->bm_ckwc) {
		size_t index;
		rwlock_write(&self->bm_lock);
		COMPILER_READ_BARRIER();
		index = ATOMIC_READ(self->bm_load);
		while (index < self->bm_ckwc) {
			dhash_t str_hash;
			str = self->bm_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			str_hash = DeeString_Hash((DeeObject *)str);
			i = perturb = str_hash & self->bm_mask;
			for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->bm_blck[i & self->bm_mask];
				if (ent->ve_str)
					continue;
				ent->ve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (str_hash == hash &&
			    DeeString_SIZE(str) == DeeString_SIZE(name) &&
			    memcmp(DeeString_STR(str),
			           DeeString_STR(name),
			           DeeString_SIZE(name) *
			           sizeof(char)) == 0) {
				self->bm_load = index;
				rwlock_endwrite(&self->bm_lock);
				return true;
			}
		}
		self->bm_load = index;
		rwlock_endwrite(&self->bm_lock);
		goto again;
	}
	return false;
}

INTERN bool DCALL
BlackListMapping_IsBlackListedString(BlackListMapping *__restrict self,
                                     char const *__restrict name,
                                     dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->bm_mask;
	for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
		str = self->bm_blck[i & self->bm_mask].ve_str;
		if (!str)
			break;
		if (DeeString_HASH((DeeObject *)str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (strcmp(DeeString_STR(str), name) != 0)
			continue;
		return true; /* It is! */
	}
	/* Check if more arguments need to be loaded. */
	if (ATOMIC_READ(self->bm_load) < self->bm_ckwc) {
		size_t index;
		rwlock_write(&self->bm_lock);
		COMPILER_READ_BARRIER();
		index = ATOMIC_READ(self->bm_load);
		while (index < self->bm_ckwc) {
			dhash_t str_hash;
			str = self->bm_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			str_hash = DeeString_Hash((DeeObject *)str);
			i = perturb = str_hash & self->bm_mask;
			for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->bm_blck[i & self->bm_mask];
				if (ent->ve_str)
					continue;
				ent->ve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (str_hash == hash &&
			    strcmp(DeeString_STR(str), name) == 0) {
				self->bm_load = index;
				rwlock_endwrite(&self->bm_lock);
				return true;
			}
		}
		self->bm_load = index;
		rwlock_endwrite(&self->bm_lock);
		goto again;
	}
	return false;
}

INTERN bool DCALL
BlackListMapping_IsBlackListedStringLen(BlackListMapping *__restrict self,
                                        char const *__restrict name,
                                        size_t namesize, dhash_t hash) {
	dhash_t i, perturb;
	DeeStringObject *str;
again:
	i = perturb = hash & self->bm_mask;
	for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
		str = self->bm_blck[i & self->bm_mask].ve_str;
		if (!str)
			break;
		if (DeeString_HASH((DeeObject *)str) != hash)
			continue; /* Strings loaded within the hash-set have always been pre-hashed! */
		if (DeeString_SIZE((DeeObject *)str) != namesize)
			continue;
		if (memcmp(DeeString_STR(str), name, namesize * sizeof(char)) != 0)
			continue;
		return true; /* It is! */
	}
	/* Check if more arguments need to be loaded. */
	if (ATOMIC_READ(self->bm_load) < self->bm_ckwc) {
		size_t index;
		rwlock_write(&self->bm_lock);
		COMPILER_READ_BARRIER();
		index = ATOMIC_READ(self->bm_load);
		while (index < self->bm_ckwc) {
			dhash_t str_hash;
			str = self->bm_ckwv[index++];
			/* Remember the string by caching it within out hash-vector. */
			str_hash = DeeString_Hash((DeeObject *)str);
			i = perturb = str_hash & self->bm_mask;
			for (;; BlackListMapping_BLCKNEXT(i, perturb)) {
				BlackListVarkwdsEntry *ent;
				ent = &self->bm_blck[i & self->bm_mask];
				if (ent->ve_str)
					continue;
				ent->ve_str = str;
				break;
			}
			/* Check if this one's our's, and stop loading if it is. */
			if (str_hash == hash &&
			    DeeString_SIZE(str) == namesize &&
			    memcmp(DeeString_STR(str), name, namesize * sizeof(char)) == 0) {
				self->bm_load = index;
				rwlock_endwrite(&self->bm_lock);
				return true;
			}
		}
		self->bm_load = index;
		rwlock_endwrite(&self->bm_lock);
		goto again;
	}
	return false;
}



INTERN int DCALL
BlackListMapping_BoundItem(BlackListMapping *__restrict self,
                           DeeObject *__restrict name,
                           bool allow_missing) {
	if (DeeString_Check(name) &&
	    BlackListMapping_IsBlackListed(self, name)) {
		if (!allow_missing)
			return err_unknown_key((DeeObject *)self, name);
		return -2;
	}
	return DeeObject_BoundItem(self->bm_kw, name, allow_missing);
}

INTERN int DCALL
BlackListMapping_BoundItemString(BlackListMapping *__restrict self,
                                 char const *__restrict name,
                                 dhash_t hash, bool allow_missing) {
	if (BlackListMapping_IsBlackListedString(self, name, hash)) {
		if (!allow_missing)
			return err_unknown_key_str((DeeObject *)self, name);
		return -2;
	}
	return DeeObject_BoundItemString(self->bm_kw, name, hash, allow_missing);
}

INTERN int DCALL
BlackListMapping_BoundItemStringLen(BlackListMapping *__restrict self,
                                    char const *__restrict name,
                                    size_t namesize, dhash_t hash,
                                    bool allow_missing) {
	if (BlackListMapping_IsBlackListedStringLen(self, name, namesize, hash)) {
		if (!allow_missing)
			return err_unknown_key_str_len((DeeObject *)self, name, namesize);
		return -2;
	}
	return DeeObject_BoundItemStringLen(self->bm_kw, name, namesize, hash, allow_missing);
}

INTERN DREF DeeObject *DCALL
BlackListMapping_GetItem(BlackListMapping *__restrict self,
                         DeeObject *__restrict name) {
	if (DeeString_Check(name) &&
	    BlackListMapping_IsBlackListed(self, name))
		goto missing;
	return DeeObject_GetItem(self->bm_kw, name);
missing:
	err_unknown_key((DeeObject *)self, name);
	return NULL;
}

INTERN DREF DeeObject *DCALL
BlackListMapping_GetItemString(BlackListMapping *__restrict self,
                               char const *__restrict name, dhash_t hash) {
	if (BlackListMapping_IsBlackListedString(self, name, hash))
		goto missing;
	return DeeObject_GetItemString(self->bm_kw, name, hash);
missing:
	err_unknown_key_str((DeeObject *)self, name);
	return NULL;
}

INTERN DREF DeeObject *DCALL
BlackListMapping_GetItemStringLen(BlackListMapping *__restrict self,
                                  char const *__restrict name,
                                  size_t namesize, dhash_t hash) {
	if (BlackListMapping_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	return DeeObject_GetItemStringLen(self->bm_kw, name, namesize, hash);
missing:
	err_unknown_key_str_len((DeeObject *)self, name, namesize);
	return NULL;
}

INTERN DREF DeeObject *DCALL
BlackListMapping_GetItemDef(BlackListMapping *__restrict self,
                            DeeObject *__restrict name,
                            DeeObject *__restrict def) {
	if (DeeString_Check(name) &&
	    BlackListMapping_IsBlackListed(self, name))
		goto missing;
	return DeeObject_GetItemDef(self->bm_kw, name, def);
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN DREF DeeObject *DCALL
BlackListMapping_GetItemStringDef(BlackListMapping *__restrict self,
                                  char const *__restrict name, dhash_t hash,
                                  DeeObject *__restrict def) {
	if (BlackListMapping_IsBlackListedString(self, name, hash))
		goto missing;
	return DeeObject_GetItemStringDef(self->bm_kw, name, hash, def);
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN DREF DeeObject *DCALL
BlackListMapping_GetItemStringLenDef(BlackListMapping *__restrict self,
                                     char const *__restrict name,
                                     size_t namesize, dhash_t hash,
                                     DeeObject *__restrict def) {
	if (BlackListMapping_IsBlackListedStringLen(self, name, namesize, hash))
		goto missing;
	return DeeObject_GetItemStringLenDef(self->bm_kw, name, namesize, hash, def);
missing:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

PRIVATE void DCALL
blm_fini(BlackListMapping *__restrict self) {
	Dee_Decref(self->bm_code);
	Dee_Decref(self->bm_kw);
}

PRIVATE void DCALL
blm_visit(BlackListMapping *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bm_code);
	Dee_Visit(self->bm_kw);
}

PRIVATE int DCALL
blm_bool(BlackListMapping *__restrict self) {
	DREF DeeObject *vals[2], *elem, *iter;
	iter = DeeObject_IterSelf(self->bm_kw);
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

PRIVATE size_t DCALL
blm_nsi_size(BlackListMapping *__restrict self) {
	size_t result = 0;
	DREF DeeObject *vals[2], *elem, *iter;
	iter = DeeObject_IterSelf(self->bm_kw);
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


PRIVATE DREF BlackListMappingIterator *DCALL
blm_iter(BlackListMapping *__restrict self) {
	DREF BlackListMappingIterator *result;
	result = DeeObject_MALLOC(BlackListMappingIterator);
	if unlikely(!result)
		goto done;
	result->mi_iter = DeeObject_IterSelf(self->bm_kw);
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

PRIVATE DREF DeeObject *DCALL
blm_size(BlackListMapping *__restrict self) {
	return DeeInt_NewSize(blm_nsi_size(self));
}

PRIVATE DREF DeeObject *DCALL
blm_contains(BlackListMapping *__restrict self,
             DeeObject *__restrict key) {
	if (DeeString_Check(key) &&
	    BlackListMapping_IsBlackListed(self, key))
		return_false;
	return DeeObject_ContainsObject(self->bm_kw, key);
}

PRIVATE DREF DeeObject *DCALL
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

PRIVATE struct type_nsi blm_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (void *)&blm_nsi_size,
			/* .nsi_nextkey    = */ (void *)&blmi_nsi_nextkey,
			/* .nsi_nextvalue  = */ (void *)&blmi_nsi_nextvalue,
			/* .nsi_getdefault = */ (void *)&blm_getdefault
		}
	}
};

PRIVATE struct type_seq blm_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blm_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blm_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&blm_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&BlackListMapping_GetItem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &blm_nsi
};

PRIVATE struct type_member blm_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &BlackListMappingIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE DREF BlackListMapping *DCALL
blm_copy(BlackListMapping *__restrict self) {
	DREF BlackListMapping *result;
	result = (DREF BlackListMapping *)DeeObject_Malloc(offsetof(BlackListMapping, bm_blck) +
	                                                   (self->bm_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	/* Create a copy of the original keywords, since we're acting as a proxy. */
	result->bm_kw = DeeObject_Copy(self->bm_kw);
	if unlikely(!result->bm_kw)
		goto err_r;
	rwlock_init(&result->bm_lock);
	result->bm_code = self->bm_code;
	Dee_Incref(result->bm_code);
	result->bm_ckwc = self->bm_ckwc;
	result->bm_ckwv = self->bm_ckwv;
	result->bm_mask = self->bm_mask;
	rwlock_read(&self->bm_lock);
	result->bm_load = self->bm_load;
	memcpy(result->bm_blck, self->bm_blck,
	       (result->bm_mask + 1) *
	       sizeof(BlackListVarkwdsEntry));
	rwlock_endread(&self->bm_lock);
	DeeObject_Init(result, &BlackListMapping_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}

PRIVATE DREF BlackListMapping *DCALL
blm_deep(BlackListMapping *__restrict self) {
	DREF BlackListMapping *result;
	result = (DREF BlackListMapping *)DeeObject_Malloc(offsetof(BlackListMapping, bm_blck) +
	                                                   (self->bm_mask + 1) *
	                                                   sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	result->bm_kw = DeeObject_DeepCopy(self->bm_kw);
	if unlikely(!result->bm_kw)
		goto err_r;
	rwlock_init(&result->bm_lock);
	result->bm_code = self->bm_code; /* Immutable, so no copy required */
	Dee_Incref(result->bm_code);
	result->bm_ckwc = self->bm_ckwc;
	result->bm_ckwv = self->bm_ckwv;
	result->bm_mask = self->bm_mask;
	rwlock_read(&self->bm_lock);
	result->bm_load = self->bm_load;
	memcpy(result->bm_blck, self->bm_blck,
	       (result->bm_mask + 1) *
	       sizeof(BlackListVarkwdsEntry));
	rwlock_endread(&self->bm_lock);
	DeeObject_Init(result, &BlackListMapping_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}


INTERN DeeTypeObject BlackListMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BlackListMapping",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL|TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (void *)NULL,
				/* .tp_copy_ctor = */ (void *)&blm_copy,
				/* .tp_deep_ctor = */ (void *)&blm_deep,
				/* .tp_any_ctor  = */ (void *)NULL,
				/* .tp_free      = */ (void *)NULL
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&blm_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&blm_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blm_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &blm_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ blm_class_members
};


INTERN DREF DeeObject *DCALL
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
	result = (DREF BlackListMapping *)DeeObject_Calloc(offsetof(BlackListMapping, bm_blck) +
	                                                   (mask + 1) * sizeof(BlackListVarkwdsEntry));
	if unlikely(!result)
		goto done;
	rwlock_cinit(&result->bm_lock);
	result->bm_code = code;
	result->bm_ckwc = argc - positional_argc;
	result->bm_ckwv = code->co_keywords + positional_argc;
	result->bm_kw   = kw;
	result->bm_mask = mask;
	Dee_Incref(code);
	Dee_Incref(kw);
	DeeObject_Init(result, &BlackListMapping_Type);
done:
	return (DREF DeeObject *)result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_C */
