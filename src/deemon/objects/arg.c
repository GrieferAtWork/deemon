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
#ifndef GUARD_DEEMON_OBJECTS_ARG_C
#define GUARD_DEEMON_OBJECTS_ARG_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

typedef DeeKwdsObject Kwds;
typedef DeeKwdsMappingObject KwdsMapping;


typedef struct {
	OBJECT_HEAD
	DWEAK struct kwds_entry *ki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry       *ki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
	DREF Kwds               *ki_map;  /* [1..1][const] The associated keywords table. */
} KwdsIterator;

#define READ_ITER(x) atomic_read(&(x)->ki_iter)

INTDEF DeeTypeObject DeeKwdsIterator_Type;
PRIVATE WUNUSED DREF Kwds *DCALL kwds_ctor(void);

PRIVATE WUNUSED NONNULL((1)) int DCALL
kwdsiter_ctor(KwdsIterator *__restrict self) {
	self->ki_map = kwds_ctor();
	if unlikely(!self->ki_map)
		goto err;
	self->ki_iter = self->ki_map->kw_map;
	self->ki_end  = self->ki_map->kw_map + self->ki_map->kw_mask + 1;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kwdsiter_copy(KwdsIterator *__restrict self,
              KwdsIterator *__restrict other) {
	self->ki_map  = other->ki_map;
	self->ki_iter = READ_ITER(other);
	self->ki_end  = other->ki_end;
	Dee_Incref(self->ki_map);
	return 0;
}

#define kwdsiter_deep kwdsiter_copy /* Only uses Immutable types, so deepcopy == copy */

PRIVATE WUNUSED NONNULL((1)) int DCALL
kwdsiter_init(KwdsIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_KwdsIterator", &self->ki_map))
		goto err;
	if (DeeObject_AssertTypeExact(self->ki_map, &DeeKwds_Type))
		goto err;
	Dee_Incref(self->ki_map);
	self->ki_iter = self->ki_map->kw_map;
	self->ki_end  = self->ki_map->kw_map + self->ki_map->kw_mask + 1;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
kwdsiter_fini(KwdsIterator *__restrict self) {
	Dee_Decref(self->ki_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
kwdsiter_visit(KwdsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ki_map);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
kwdsiter_bool(KwdsIterator *__restrict self) {
	struct kwds_entry *entry;
	entry = READ_ITER(self);
	for (;;) {
		if (entry >= self->ki_end)
			return 0;
		if (entry->ke_name)
			break;
		++entry;
	}
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwds_nsi_nextitem(KwdsIterator *__restrict self) {
	DREF DeeObject *value, *result;
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, entry + 1))
			break;
	}
	value = DeeInt_NewSize(entry->ke_index);
	if unlikely(!value)
		goto err;
	result = DeeTuple_Pack(2, entry->ke_name, value);
	Dee_Decref_unlikely(value);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwds_nsi_nextkey(KwdsIterator *__restrict self) {
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, entry + 1))
			break;
	}
	return_reference_((DeeObject *)entry->ke_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwds_nsi_nextvalue(KwdsIterator *__restrict self) {
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, entry + 1))
			break;
	}
	return DeeInt_NewSize(entry->ke_index);
}


#define DEFINE_KWDSITERATOR_COMPARE(name, op)                 \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL     \
	name(KwdsIterator *self, KwdsIterator *other) {           \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                         \
		return_bool(READ_ITER(self) op READ_ITER(other));     \
	err:                                                      \
		return NULL;                                          \
	}
DEFINE_KWDSITERATOR_COMPARE(kwdsiter_eq, ==)
DEFINE_KWDSITERATOR_COMPARE(kwdsiter_ne, !=)
DEFINE_KWDSITERATOR_COMPARE(kwdsiter_lo, <)
DEFINE_KWDSITERATOR_COMPARE(kwdsiter_le, <=)
DEFINE_KWDSITERATOR_COMPARE(kwdsiter_gr, >)
DEFINE_KWDSITERATOR_COMPARE(kwdsiter_ge, >=)
#undef DEFINE_KWDSITERATOR_COMPARE

PRIVATE struct type_cmp kwdsiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwdsiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwdsiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwdsiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwdsiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwdsiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwdsiter_ge,
};

PRIVATE struct type_member tpconst kwdsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(KwdsIterator, ki_map), "->?Ert:Kwds"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeKwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwdsIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?Ert:Kwds)\n"
	                         "\n"
	                         "next->?T2?Dstring?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&kwdsiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&kwdsiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&kwdsiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&kwdsiter_init,
				TYPE_FIXED_ALLOCATOR(KwdsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwdsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&kwdsiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kwdsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &kwdsiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwds_nsi_nextitem,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ kwdsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTDEF WUNUSED DREF DeeObject *DCALL
DeeKwds_NewWithHint(size_t num_items) {
	DREF Kwds *result;
	size_t init_mask = 1;
	while (init_mask <= num_items)
		init_mask = (init_mask << 1) | 1;
	result = (DREF Kwds *)DeeObject_Calloc(offsetof(Kwds, kw_map) +
	                                       (init_mask + 1) * sizeof(struct kwds_entry));
	if unlikely(!result)
		goto done;
	result->kw_mask = init_mask;
	DeeObject_Init(result, &DeeKwds_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF Kwds *DCALL
kwds_rehash(DREF Kwds *__restrict self) {
	DREF Kwds *result;
	size_t i, j, perturb;
	size_t new_mask = (self->kw_mask << 1) | 1;
	result = (DREF Kwds *)DeeObject_Calloc(offsetof(Kwds, kw_map) +
	                                       (new_mask + 1) * sizeof(struct kwds_entry));
	if unlikely(!result)
		goto done;
	result->kw_mask = new_mask;
	for (i = 0; i <= self->kw_mask; ++i) {
		struct kwds_entry *src = &self->kw_map[i];
		if (!src->ke_name)
			continue;
		perturb = j = src->ke_hash & new_mask;
		for (;; DeeKwds_MAPNEXT(j, perturb)) {
			struct kwds_entry *dst = &result->kw_map[j & new_mask];
			if (dst->ke_name)
				continue;
			memcpy(dst, src, sizeof(struct kwds_entry));
			break;
		}
	}
	memcpy(result, self, COMPILER_OFFSETAFTER(Kwds, kw_size));
	DeeObject_Free(self);
done:
	return result;
}

/* Append a new entry for `name'.
 * NOTE: The keywords argument index is set to the old number of
 *       keywords that had already been defined previously. */
INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeKwds_AppendStringLenHash)(DREF DeeObject **__restrict p_self,
                                    char const *__restrict name,
                                    size_t name_len, dhash_t hash) {
	dhash_t i, perturb;
	struct kwds_entry *entry;
	DREF Kwds *self = (DREF Kwds *)*p_self;
	if (self->kw_size * 2 > self->kw_mask) {
		/* Must allocate a larger map. */
		self = kwds_rehash(self);
		if unlikely(!self)
			goto err;
		*p_self = (DREF DeeObject *)self;
	}
	ASSERT(self->kw_size < self->kw_mask);
	perturb = i = hash & self->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
	}
	entry->ke_name = (DREF DeeStringObject *)DeeString_NewSized(name, name_len);
	if unlikely(!entry->ke_name)
		goto err;
	entry->ke_name->s_hash = hash; /* Remember the hash! */
	entry->ke_index        = self->kw_size++;
	entry->ke_hash         = hash;
	return 0;
err:
	return -1;
}

/* Append a new entry for `name'.
 * NOTE: The keywords argument index is set to the old number of
 *       keywords that had already been defined previously. */
INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeKwds_Append)(DREF DeeObject **__restrict p_self,
                       DeeObject *__restrict name) {
	dhash_t i, perturb, hash;
	struct kwds_entry *entry;
	DREF Kwds *self = (DREF Kwds *)*p_self;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if (self->kw_size * 2 > self->kw_mask) {
		/* Must allocate a larger map. */
		self = kwds_rehash(self);
		if unlikely(!self)
			goto err;
		*p_self = (DREF DeeObject *)self;
	}
	ASSERT(self->kw_size < self->kw_mask);
	hash    = DeeString_Hash(name);
	perturb = i = hash & self->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
	}
	entry->ke_name = (DREF DeeStringObject *)name;
	Dee_Incref(name);
	entry->ke_name->s_hash = hash; /* Remember the hash! */
	entry->ke_index        = self->kw_size++;
	entry->ke_hash         = hash;
	return 0;
err:
	return -1;
}

/* Return the keyword-entry associated with `keyword_index'
 * The caller must ensure that `keyword_index < DeeKwds_SIZE(self)' */
INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) struct dee_kwds_entry *DCALL
DeeKwds_GetByIndex(DeeObject *__restrict self, size_t keyword_index) {
	DeeKwdsObject *me = (DeeKwdsObject *)self;
	size_t i;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeKwds_Type);
	ASSERT(keyword_index < DeeKwds_SIZE(me));
	for (i = 0;; ++i) {
		ASSERT(i <= me->kw_mask);
		if (!me->kw_map[i].ke_name)
			continue;
		if (me->kw_map[i].ke_index == keyword_index)
			return &me->kw_map[i];
	}
}



LOCAL WUNUSED NONNULL((1, 2)) size_t DCALL
kwds_findstr(Kwds *__restrict self,
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

LOCAL WUNUSED NONNULL((1)) size_t DCALL
kwds_findstr_len(Kwds *__restrict self,
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
		if (DeeString_EqualsBuf(entry->ke_name, name, namesize))
			return entry->ke_index;
	}
	return (size_t)-1;
}




PRIVATE WUNUSED DREF Kwds *DCALL kwds_ctor(void) {
	DREF Kwds *result;
	result = (DREF Kwds *)DeeObject_Malloc(offsetof(Kwds, kw_map) +
	                                       (2 * sizeof(struct kwds_entry)));
	if unlikely(!result)
		goto done;
	result->kw_map[0].ke_name = NULL;
	result->kw_map[1].ke_name = NULL;
	result->kw_mask           = 1;
	result->kw_size           = 0;
	DeeObject_Init(result, &DeeKwds_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF Kwds *DCALL
kwds_init(size_t argc, DeeObject *const *argv) {
	DREF Kwds *result;
	DeeObject *init;
	DREF DeeObject *iter, *elem;
	if (DeeArg_Unpack(argc, argv, "o:Kwds", &init))
		goto err;
	result = kwds_ctor();
	if unlikely(!result)
		goto err;
	iter = DeeObject_IterSelf(init);
	if unlikely(!iter)
		goto err_r;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
			goto err_r_iter_elem;
		if (DeeKwds_Append((DeeObject **)&result, elem))
			goto err_r_iter_elem;
		Dee_Decref_likely(elem);
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref_likely(iter);
	return result;
err_r_iter_elem:
	Dee_Decref_likely(elem);
err_r_iter:
	Dee_Decref_likely(iter);
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
kwds_fini(Kwds *__restrict self) {
	size_t i;
	for (i = 0; i <= self->kw_mask; ++i)
		Dee_XDecref(self->kw_map[i].ke_name);
}

PRIVATE NONNULL((1, 2)) void DCALL
kwds_visit(Kwds *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->kw_mask; ++i)
		Dee_XVisit(self->kw_map[i].ke_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
kwds_printrepr(Kwds *__restrict self,
               dformatprinter printer, void *arg) {
#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0
	dssize_t temp, result;
	size_t i;
	bool is_first = true;
	result = DeeFormat_PRINT(printer, arg, "{ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i <= self->kw_mask; ++i) {
		if (!self->kw_map[i].ke_name)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg,
		                         "%r: %" PRFuSIZ,
		                         self->kw_map[i].ke_name,
		                         self->kw_map[i].ke_index));
		is_first = false;
	}
	DO(err, DeeFormat_PRINT(printer, arg, " }"));
done:
	return result;
err:
	return temp;
#undef DO
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
kwds_bool(Kwds *__restrict self) {
	return self->kw_size != 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF KwdsIterator *DCALL
kwds_iter(Kwds *__restrict self) {
	DREF KwdsIterator *result;
	result = DeeObject_MALLOC(KwdsIterator);
	if unlikely(!result)
		goto done;
	result->ki_iter = self->kw_map;
	result->ki_end  = self->kw_map + self->kw_mask + 1;
	result->ki_map  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeKwdsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwds_size(Kwds *__restrict self) {
	return DeeInt_NewSize(self->kw_size);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
kwds_nsi_getsize(Kwds *__restrict self) {
	return self->kw_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_contains(Kwds *self,
              DeeObject *key) {
	if (!DeeString_Check(key))
		goto nope;
	return_bool(kwds_findstr(self,
	                         DeeString_STR(key),
	                         DeeString_Hash(key)) !=
	            (size_t)-1);
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
kwds_nsi_getdefault(Kwds *__restrict self,
                    DeeObject *__restrict key,
                    DeeObject *__restrict def) {
	size_t index;
	if (!DeeString_Check(key))
		goto nope;
	index = kwds_findstr(self,
	                     DeeString_STR(key),
	                     DeeString_Hash(key));
	if (index == (size_t)-1)
		goto nope;
	return DeeInt_NewSize(index);
nope:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_get(Kwds *self,
         DeeObject *key) {
	DREF DeeObject *result;
	result = kwds_nsi_getdefault(self, key, ITER_DONE);
	if (result != ITER_DONE)
		return result;
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE struct type_nsi tpconst kwds_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&kwds_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&kwds_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&kwds_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&kwds_nsi_getdefault
		}
	}
};


PRIVATE struct type_seq kwds_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwds_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwds_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwds_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwds_get,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &kwds_nsi
};

PRIVATE struct type_member tpconst kwds_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeKwdsIterator_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeKwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Kwds",
	/* .tp_doc      = */ DOC("(names:?S?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&kwds_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)&kwds_init,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwds_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&kwds_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&kwds_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kwds_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ &kwds_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ kwds_class_members
};
#undef READ_ITER



typedef struct {
	OBJECT_HEAD
	DWEAK struct kwds_entry *ki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry       *ki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
	DREF KwdsMapping        *ki_map;  /* [1..1][const] The associated keywords mapping. */
} KmapIterator;

#define READ_ITER(x) atomic_read(&(x)->ki_iter)

STATIC_ASSERT(offsetof(KwdsIterator, ki_iter) == offsetof(KmapIterator, ki_iter));
STATIC_ASSERT(offsetof(KwdsIterator, ki_end) == offsetof(KmapIterator, ki_end));
STATIC_ASSERT(offsetof(KwdsIterator, ki_map) == offsetof(KmapIterator, ki_map));


PRIVATE WUNUSED NONNULL((1)) int DCALL
kmapiter_ctor(KmapIterator *__restrict self) {
	self->ki_map = (DREF KwdsMapping *)DeeObject_NewDefault(&DeeKwdsMapping_Type);
	if unlikely(!self->ki_map)
		goto err;
	self->ki_iter = self->ki_map->kmo_kwds->kw_map;
	self->ki_end  = self->ki_map->kmo_kwds->kw_map + self->ki_map->kmo_kwds->kw_mask + 1;
	return 0;
err:
	return -1;
}

#define kmapiter_copy kwdsiter_copy
#define kmapiter_deep kwdsiter_deep
PRIVATE WUNUSED NONNULL((1)) int DCALL
kmapiter_init(KmapIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_KwdsMappingIterator", &self->ki_map))
		goto err;
	if (DeeObject_AssertTypeExact(self->ki_map, &DeeKwdsMapping_Type))
		goto err;
	Dee_Incref(self->ki_map);
	self->ki_iter = self->ki_map->kmo_kwds->kw_map;
	self->ki_end  = self->ki_map->kmo_kwds->kw_map + self->ki_map->kmo_kwds->kw_mask + 1;
	return 0;
err:
	return -1;
}

#define kmapiter_fini kwdsiter_fini
PRIVATE NONNULL((1, 2)) void DCALL
kmapiter_visit(KmapIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ki_map);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
kmapiter_bool(KmapIterator *__restrict self) {
	struct kwds_entry *entry;
	entry = READ_ITER(self);
	for (;;) {
		if (entry >= self->ki_end)
			return 0;
		if (entry->ke_name)
			break;
		++entry;
	}
	return atomic_read(&self->ki_map->kmo_argv) != NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kmap_nsi_nextitem(KmapIterator *__restrict self) {
	DREF DeeObject *value, *result;
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, entry + 1))
			break;
	}
	DeeKwdsMapping_LockRead(self->ki_map);
	if unlikely(!self->ki_map->kmo_argv) {
		DeeKwdsMapping_LockEndRead(self->ki_map);
		return ITER_DONE;
	}
	value = self->ki_map->kmo_argv[entry->ke_index];
	Dee_Incref(value);
	DeeKwdsMapping_LockEndRead(self->ki_map);
	result = DeeTuple_Pack(2, entry->ke_name, value);
	Dee_Decref_unlikely(value);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kmap_nsi_nextkey(KmapIterator *__restrict self) {
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, entry + 1))
			break;
	}
	if unlikely(!atomic_read(&self->ki_map->kmo_argv))
		return ITER_DONE;
	return_reference_((DeeObject *)entry->ke_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kmap_nsi_nextvalue(KmapIterator *__restrict self) {
	DREF DeeObject *value;
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, entry + 1))
			break;
	}
	DeeKwdsMapping_LockRead(self->ki_map);
	if unlikely(!self->ki_map->kmo_argv) {
		DeeKwdsMapping_LockEndRead(self->ki_map);
		return ITER_DONE;
	}
	value = self->ki_map->kmo_argv[entry->ke_index];
	Dee_Incref(value);
	DeeKwdsMapping_LockEndRead(self->ki_map);
	return value;
}

#define kmapiter_cmp     kwdsiter_cmp
#define kmapiter_members kwdsiter_members

INTERN DeeTypeObject DeeKwdsMappingIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwdsMappingIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?Ert:KwdsMapping)\n"
	                         "\n"
	                         "next->?T2?Dstring?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&kmapiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&kmapiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&kmapiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&kmapiter_init,
				TYPE_FIXED_ALLOCATOR(KmapIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kmapiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&kmapiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kmapiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &kmapiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmap_nsi_nextitem,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ kmapiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




PRIVATE WUNUSED NONNULL((1)) int DCALL
kmap_ctor(KwdsMapping *__restrict self) {
	self->kmo_kwds = kwds_ctor();
	if unlikely(!self->kmo_kwds)
		goto err;
	self->kmo_argv = NULL;
	Dee_atomic_rwlock_init(&self->kmo_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kmap_copy(KwdsMapping *__restrict self,
          KwdsMapping *__restrict other) {
	size_t i, count;
	count          = other->kmo_kwds->kw_size;
	self->kmo_argv = (DREF DeeObject **)Dee_Mallocc(count, sizeof(DREF DeeObject *));
	if unlikely(!self->kmo_argv)
		goto err;
	DeeKwdsMapping_LockRead(other);
	for (i = 0; i < count; ++i) {
		self->kmo_argv[i] = other->kmo_argv[i];
		Dee_Incref(self->kmo_argv[i]);
	}
	DeeKwdsMapping_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->kmo_lock);
	self->kmo_kwds = other->kmo_kwds;
	Dee_Incref(self->kmo_kwds);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kmap_deep(KwdsMapping *__restrict self,
          KwdsMapping *__restrict other) {
	size_t i, count;
	count          = other->kmo_kwds->kw_size;
	self->kmo_argv = (DREF DeeObject **)Dee_Mallocc(count, sizeof(DREF DeeObject *));
	if unlikely(!self->kmo_argv)
		goto err;
	DeeKwdsMapping_LockRead(other);
	for (i = 0; i < count; ++i) {
		self->kmo_argv[i] = other->kmo_argv[i];
		Dee_Incref(self->kmo_argv[i]);
	}
	DeeKwdsMapping_LockEndRead(other);

	/* Construct deep copies of all of the arguments. */
	for (i = 0; i < count; ++i) {
		if (DeeObject_InplaceDeepCopy(&self->kmo_argv[i]))
			goto err_r_argv;
	}
	Dee_atomic_rwlock_init(&self->kmo_lock);
	self->kmo_kwds = other->kmo_kwds;
	Dee_Incref(self->kmo_kwds);
	return 0;
err_r_argv:
	Dee_Decrefv(self->kmo_argv, count);
	Dee_Free(self->kmo_argv);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
kmap_init(KwdsMapping *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *args;
	size_t i;
	if (DeeArg_Unpack(argc, argv, "oo:_KwdsMapping", &self->kmo_kwds, &args))
		goto err;
	if (DeeObject_AssertTypeExact(self->kmo_kwds, &DeeKwds_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if (DeeTuple_SIZE(args) != self->kmo_kwds->kw_size) {
		return err_keywords_bad_for_argc(DeeTuple_SIZE(args),
		                                 self->kmo_kwds->kw_size);
	}
	self->kmo_argv = (DREF DeeObject **)Dee_Mallocc(DeeTuple_SIZE(args),
	                                                sizeof(DREF DeeObject *));
	if unlikely(!self->kmo_argv)
		goto err;
	for (i = 0; i < DeeTuple_SIZE(args); ++i) {
		self->kmo_argv[i] = DeeTuple_GET(args, i);
		Dee_Incref(self->kmo_argv[i]);
	}
	Dee_Incref(self->kmo_kwds);
	Dee_atomic_rwlock_init(&self->kmo_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
kmap_fini(KwdsMapping *__restrict self) {
	if (self->kmo_argv) {
		size_t count = self->kmo_kwds->kw_size;
		while (count--)
			Dee_Decref(self->kmo_argv[count]);
		Dee_Free(self->kmo_argv);
	}

	Dee_Decref(self->kmo_kwds);
}

PRIVATE NONNULL((1, 2)) void DCALL
kmap_visit(KwdsMapping *__restrict self, dvisit_t proc, void *arg) {
	DeeKwdsMapping_LockRead(self);
	if (self->kmo_argv)
		Dee_Visitv(self->kmo_argv, self->kmo_kwds->kw_size);
	DeeKwdsMapping_LockEndRead(self);
	/*Dee_Visit(self->kmo_kwds);*/ /* Only ever references strings, so there'd be no point. */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
kmap_bool(KwdsMapping *__restrict self) {
	if (!atomic_read(&self->kmo_argv))
		return 0;
	return self->kmo_kwds->kw_size != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF KmapIterator *DCALL
kmap_iter(KwdsMapping *__restrict self) {
	DREF KmapIterator *result;
	result = DeeObject_MALLOC(KmapIterator);
	if unlikely(!result)
		goto done;
	result->ki_iter = self->kmo_kwds->kw_map;
	result->ki_end  = self->kmo_kwds->kw_map + self->kmo_kwds->kw_mask + 1;
	result->ki_map  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeKwdsMappingIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kmap_size(KwdsMapping *__restrict self) {
	if (!atomic_read(&self->kmo_argv))
		return_reference_(DeeInt_Zero);
	return DeeInt_NewSize(self->kmo_kwds->kw_size);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
kmap_nsi_getsize(KwdsMapping *__restrict self) {
	if (!atomic_read(&self->kmo_argv))
		return 0;
	return self->kmo_kwds->kw_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_contains(KwdsMapping *self, DeeObject *key) {
	if (!atomic_read(&self->kmo_argv))
		goto nope;
	if (!DeeString_Check(key))
		goto nope;
	return_bool(kwds_findstr(self->kmo_kwds,
	                         DeeString_STR(key),
	                         DeeString_Hash(key)) !=
	            (size_t)-1);
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
kmap_nsi_getdefault(KwdsMapping *self, DeeObject *key, DeeObject *def) {
	size_t index;
	DREF DeeObject *result;
	if (!DeeString_Check(key))
		goto nope;
	index = kwds_findstr(self->kmo_kwds,
	                     DeeString_STR(key),
	                     DeeString_Hash(key));
	if (index == (size_t)-1)
		goto nope;
	DeeKwdsMapping_LockRead(self);
	if unlikely(!self->kmo_argv) {
		DeeKwdsMapping_LockEndRead(self);
		goto nope;
	}
	ASSERT(index < self->kmo_kwds->kw_size);
	result = self->kmo_argv[index];
	Dee_Incref(result);
	DeeKwdsMapping_LockEndRead(self);
	return result;
nope:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_get(KwdsMapping *self,
         DeeObject *key) {
	DREF DeeObject *result;
	result = kmap_nsi_getdefault(self, key, ITER_DONE);
	if (result != ITER_DONE)
		return result;
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE struct type_nsi tpconst kmap_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&kmap_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&kmap_nsi_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&kmap_nsi_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&kmap_nsi_getdefault
		}
	}
};

PRIVATE struct type_seq kmap_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmap_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmap_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kmap_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kmap_get,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &kmap_nsi
};


PRIVATE struct type_member tpconst kmap_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___kwds__, STRUCT_OBJECT, offsetof(KwdsMapping, kmo_kwds), "->?Ert:Kwds"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst kmap_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeKwdsMappingIterator_Type),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeKwdsMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwdsMapping",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&kmap_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&kmap_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&kmap_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&kmap_init,
				TYPE_FIXED_ALLOCATOR(KwdsMapping)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kmap_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&kmap_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kmap_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &kmap_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ kmap_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ kmap_class_members
};

/* Construct a keywords-mapping object from a given `kwds' object,
 * as well as an argument vector that will be shared with the mapping.
 * The returned object then a mapping {(string, Object)...} for the
 * actual argument values passed to the function.
 * NOTE: The caller must later invoke `DeeKwdsMapping_Decref()' in order
 *       to clean up the returned object. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwdsMapping_New(/*Kwds*/ DeeObject *kwds,
                   DeeObject *const *argv) {
	DREF KwdsMapping *result;
	ASSERT_OBJECT_TYPE_EXACT(kwds, &DeeKwds_Type);
	result = DeeObject_MALLOC(KwdsMapping);
	if unlikely(!result)
		goto done;
	result->kmo_argv = (DREF DeeObject **)(DeeObject **)argv;
	result->kmo_kwds = (DREF DeeKwdsObject *)kwds;
	Dee_atomic_rwlock_init(&result->kmo_lock);
	DeeObject_Init(result, &DeeKwdsMapping_Type);
done:
	return (DREF DeeObject *)result;
}

/* Unshare the argument vector from a keywords-mapping object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
PUBLIC NONNULL((1)) void DCALL
DeeKwdsMapping_Decref(DREF DeeObject *__restrict self) {
	DREF KwdsMapping *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me = (DREF KwdsMapping *)self;
	if (DeeObject_IsShared(me)) {
		/* The mapping is being shared, so we must construct a copy of its argument list. */
		size_t argc = me->kmo_kwds->kw_size;
		DREF DeeObject **argv;
		if (!argc) {
clear_argv:
			DeeKwdsMapping_LockWrite(me);
			me->kmo_argv = NULL;
			DeeKwdsMapping_LockEndWrite(me);
		} else {
			do {
				argv = (DREF DeeObject **)Dee_TryMallocc(argc, sizeof(DREF DeeObject *));
			} while (unlikely(!argv) && Dee_TryCollectMemory(argc * sizeof(DREF DeeObject *)));
			if unlikely(!argv)
				goto clear_argv;
			DeeKwdsMapping_LockWrite(me);
			if unlikely(!me->kmo_argv) {
				/* Shouldn't really happen, but is allowed by the specs... */
				Dee_Free(argv);
				argv = NULL;
			} else {
				Dee_Movrefv(argv, me->kmo_argv, argc);
			}
			me->kmo_argv = argv; /* Remember the old arguments. */
			DeeKwdsMapping_LockEndWrite(me);
		}

		/* Create a reference for `kmo_kwds', which didn't contain one until now. */
		Dee_Incref(me->kmo_kwds);

		/* Drop our own reference (which should still be shared) */
		Dee_Decref_unlikely(self);
	} else {
		/*Dee_Decref(me->kmo_kwds);*/
		Dee_DecrefNokill(&DeeKwdsMapping_Type);
		DeeObject_Free(me);
	}
}



INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeKwdsMapping_HasItemStringHash(DeeObject *__restrict self,
                                 char const *__restrict name,
                                 dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr(me->kmo_kwds, name, hash);
	if unlikely(index == (size_t)-1)
		return false;
	if unlikely(!atomic_read(&me->kmo_argv))
		return false;
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeKwdsMapping_HasItemStringLenHash(DeeObject *__restrict self,
                                    char const *__restrict name,
                                    size_t namesize,
                                    dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr_len(me->kmo_kwds, name, namesize, hash);
	if unlikely(index == (size_t)-1)
		return false;
	if unlikely(!atomic_read(&me->kmo_argv))
		return false;
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwdsMapping_GetItemStringHash(DeeObject *__restrict self,
                                 char const *__restrict name,
                                 dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr(me->kmo_kwds, name, hash);
	if unlikely(index == (size_t)-1) {
no_such_key:
		err_unknown_key_str((DeeObject *)self, name);
		return NULL;
	}
	DeeKwdsMapping_LockRead(me);
	if unlikely(!me->kmo_argv) {
		DeeKwdsMapping_LockEndRead(me);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	DeeKwdsMapping_LockEndRead(me);
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeKwdsMapping_GetItemStringHashDef(DeeObject *self,
                                    char const *__restrict name,
                                    dhash_t hash,
                                    DeeObject *def) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr(me->kmo_kwds, name, hash);
	if unlikely(index == (size_t)-1) {
no_such_key:
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	DeeKwdsMapping_LockRead(me);
	if unlikely(!me->kmo_argv) {
		DeeKwdsMapping_LockEndRead(me);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	DeeKwdsMapping_LockEndRead(me);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwdsMapping_GetItemStringLenHash(DeeObject *__restrict self,
                                    char const *__restrict name,
                                    size_t namesize,
                                    dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr_len(me->kmo_kwds, name, namesize, hash);
	if unlikely(index == (size_t)-1) {
no_such_key:
		err_unknown_key_str_len((DeeObject *)self, name, namesize);
		return NULL;
	}
	DeeKwdsMapping_LockRead(me);
	if unlikely(!me->kmo_argv) {
		DeeKwdsMapping_LockEndRead(me);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	DeeKwdsMapping_LockEndRead(me);
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeKwdsMapping_GetItemStringLenHashDef(DeeObject *self,
                                       char const *__restrict name,
                                       size_t namesize,
                                       dhash_t hash,
                                       DeeObject *def) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr_len(me->kmo_kwds, name, namesize, hash);
	if unlikely(index == (size_t)-1) {
no_such_key:
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	DeeKwdsMapping_LockRead(me);
	if unlikely(!me->kmo_argv) {
		DeeKwdsMapping_LockEndRead(me);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	DeeKwdsMapping_LockEndRead(me);
	return result;
}

/* Construct/access keyword arguments passed to a function as a
 * high-level {string: Object}-like mapping that is bound to the
 * actually mapped arguments. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeArg_GetKw(size_t *__restrict p_argc,
             DeeObject *const *argv,
             DeeObject *kw) {
	if (!kw)
		return_reference_(Dee_EmptyMapping);
	if (DeeKwds_Check(kw)) {
		size_t num_keywords;
		num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > *p_argc) {
			/* Argument list is too short of the given keywords */
			err_keywords_bad_for_argc(*p_argc, num_keywords);
			return NULL;
		}
		*p_argc -= num_keywords;

		/* Turn keywords and arguments into a proper mapping-like object. */
		return DeeKwdsMapping_New(kw, argv + *p_argc);
	}

	/* `kw' already is a user-provided mapping. */
	return_reference_(kw);
}

PUBLIC NONNULL((3)) void DCALL
DeeArg_PutKw(size_t argc, DeeObject *const *argv, DREF DeeObject *kw) {
	ASSERT_OBJECT(kw);
	if (DeeKwdsMapping_Check(kw) &&
	    DeeKwdsMapping_GetArgv(kw) == argv + argc) {
		/* If we're the ones owning the keywords-mapping, we must also decref() it. */
		DeeKwdsMapping_Decref(kw);
	} else {
		Dee_Decref(kw);
	}
}


/* Initialize `self' to load keyword arguments.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeKwArgs_Init)(DeeKwArgs *__restrict self, size_t *__restrict p_argc,
                       DeeObject *const *argv, DeeObject *kw) {
	self->kwa_kwused = 0;
	if (!kw) {
		self->kwa_kwargv = NULL;
		self->kwa_kw     = NULL;
		return 0;
	}
	if (DeeKwds_Check(kw)) {
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > *p_argc) {
			/* Argument list is too short of the given keywords */
			return err_keywords_bad_for_argc(*p_argc, num_keywords);
		}
		*p_argc -= num_keywords;
		self->kwa_kwargv = argv + *p_argc;
	} else {
		self->kwa_kwargv = NULL;
	}
	self->kwa_kw = kw;
	return 0;
}

/* Indicate that you're doing loading arguments from `self'.
 * This function asserts that `kwa_kwused == #kwa_kw' so-as
 * to ensure that `kwa_kw' doesn't contain any unused keyword
 * arguments.
 * @return: 0 : Success
 * @param: positional_argc: The value of `*p_argc' after `DeeKwArgs_Init()' returned.
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeKwArgs_Done)(DeeKwArgs *__restrict self,
                       size_t positional_argc,
                       char const *function_name) {
	size_t avail_argc;
	if (!self->kwa_kw)
		return 0;
	if (DeeKwds_Check(self->kwa_kw)) {
		avail_argc = DeeKwds_SIZE(self->kwa_kw);
		Dee_ASSERT(avail_argc >= self->kwa_kwused);
	} else {
		avail_argc = DeeObject_Size(self->kwa_kw);
		if unlikely(avail_argc == (size_t)-1)
			goto err;
	}
	if (avail_argc > self->kwa_kwused) {
		/* TODO: Include the names of the unused arguments here! */
		err_invalid_argc(function_name,
		                 positional_argc + avail_argc,
		                 positional_argc + self->kwa_kwused,
		                 positional_argc + self->kwa_kwused);
		goto err;
	}
	return 0;
err:
	return -1;
}

/* Lookup a named keyword argument from `self'
 * @return: * :   Reference to named keyword argument.
 * @return: NULL: An error was thrown.*/
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwArgs_GetStringHash(DeeKwArgs *__restrict self,
                        char const *__restrict name,
                        Dee_hash_t hash) {
	DREF DeeObject *result;
	if (!self->kwa_kw) {
		err_unknown_key_str(Dee_EmptyMapping, name);
		return NULL;
	}
	if (DeeKwds_Check(self->kwa_kw)) {
		size_t kw_index;
		kw_index = kwds_findstr((Kwds *)self->kwa_kw, name, hash);
		if unlikely(kw_index == (size_t)-1) {
			err_keywords_not_found(name);
			return NULL;
		}
		++self->kwa_kwused;
		return_reference(self->kwa_kwargv[kw_index]);
	}
	result = DeeObject_GetItemStringHash(self->kwa_kw, name, hash);
	if likely(result)
		++self->kwa_kwused;
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwArgs_GetStringLenHash(DeeKwArgs *__restrict self,
                           char const *__restrict name,
                           size_t namelen, Dee_hash_t hash) {
	DREF DeeObject *result;
	if (!self->kwa_kw) {
		err_unknown_key_str(Dee_EmptyMapping, name);
		return NULL;
	}
	++self->kwa_kwused;
	if (DeeKwds_Check(self->kwa_kw)) {
		size_t kw_index;
		kw_index = kwds_findstr_len((Kwds *)self->kwa_kw, name, namelen, hash);
		if unlikely(kw_index == (size_t)-1) {
			err_keywords_not_found(name);
			return NULL;
		}
		++self->kwa_kwused;
		return_reference(self->kwa_kwargv[kw_index]);
	}
	result = DeeObject_GetItemStringLenHash(self->kwa_kw, name, namelen, hash);
	if likely(result)
		++self->kwa_kwused;
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeKwArgs_GetStringHashDef(DeeKwArgs *__restrict self,
                           char const *__restrict name,
                           Dee_hash_t hash, DeeObject *def) {
	if (self->kwa_kw) {
		if (DeeKwds_Check(self->kwa_kw)) {
			size_t kw_index;
			kw_index = kwds_findstr((Kwds *)self->kwa_kw, name, hash);
			if unlikely(kw_index != (size_t)-1) {
				def = self->kwa_kwargv[kw_index];
				++self->kwa_kwused;
			}
		} else {
			DREF DeeObject *result;
			result = DeeObject_GetItemStringHashDef(self->kwa_kw, name, hash, ITER_DONE);
			if likely(result != ITER_DONE) {
				++self->kwa_kwused;
			} else {
				result = def;
				if (result != ITER_DONE)
					Dee_Incref(result);
			}
			return result;
		}
	}
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeKwArgs_GetStringLenHashDef(DeeKwArgs *__restrict self, char const *__restrict name,
                              size_t namelen, Dee_hash_t hash, DeeObject *def) {
	if (self->kwa_kw) {
		if (DeeKwds_Check(self->kwa_kw)) {
			size_t kw_index;
			kw_index = kwds_findstr_len((Kwds *)self->kwa_kw, name, namelen, hash);
			if unlikely(kw_index != (size_t)-1) {
				def = self->kwa_kwargv[kw_index];
				++self->kwa_kwused;
			}
		} else {
			DREF DeeObject *result;
			result = DeeObject_GetItemStringLenHashDef(self->kwa_kw, name, namelen, hash, ITER_DONE);
			if likely(result != ITER_DONE) {
				++self->kwa_kwused;
			} else {
				result = def;
				if (result != ITER_DONE)
					Dee_Incref(result);
			}
			return result;
		}
	}
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}



PUBLIC WUNUSED NONNULL((4)) DREF DeeObject *DCALL
DeeArg_GetKwStringHash(size_t argc, DeeObject *const *argv, DeeObject *kw,
                       char const *__restrict name, Dee_hash_t hash) {
	if (!kw) {
		err_unknown_key_str(Dee_EmptyMapping, name);
		return NULL;
	}
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > argc) {
			/* Argument list is too short of the given keywords */
			err_keywords_bad_for_argc(argc, num_keywords);
			return NULL;
		}
		kw_index = kwds_findstr((Kwds *)kw, name, hash);
		if unlikely(kw_index == (size_t)-1) {
			err_keywords_not_found(name);
			return NULL;
		}
		ASSERT(kw_index < num_keywords);
		return_reference(argv[(argc - num_keywords) + kw_index]);
	}
	return DeeObject_GetItemStringHash(kw, name, hash);
}

PUBLIC WUNUSED NONNULL((4)) DREF DeeObject *DCALL
DeeArg_GetKwStringLenHash(size_t argc, DeeObject *const *argv, DeeObject *kw,
                          char const *__restrict name, size_t namelen, dhash_t hash) {
	if (!kw) {
		err_unknown_key_str_len(Dee_EmptyMapping, name, namelen);
		return NULL;
	}
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > argc) {
			/* Argument list is too short of the given keywords */
			err_keywords_bad_for_argc(argc, num_keywords);
			return NULL;
		}
		kw_index = kwds_findstr_len((Kwds *)kw, name, namelen, hash);
		if unlikely(kw_index == (size_t)-1) {
			err_keywords_not_found(name);
			return NULL;
		}
		ASSERT(kw_index < num_keywords);
		return_reference(argv[(argc - num_keywords) + kw_index]);
	}
	return DeeObject_GetItemStringLenHash(kw, name, namelen, hash);
}

PUBLIC WUNUSED NONNULL((4, 6)) DREF DeeObject *DCALL
DeeArg_GetKwStringHashDef(size_t argc, DeeObject *const *argv,
                          DeeObject *kw, char const *__restrict name,
                          Dee_hash_t hash, DeeObject *def) {
	if (!kw) {
return_def:
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > argc)
			goto return_def;
		kw_index = kwds_findstr((Kwds *)kw, name, hash);
		if (kw_index == (size_t)-1)
			goto return_def;
		ASSERT(kw_index < num_keywords);
		return_reference(argv[(argc - num_keywords) + kw_index]);
	}
	return DeeObject_GetItemStringHashDef(kw, name, hash, def);
}

PUBLIC WUNUSED NONNULL((4, 7)) DREF DeeObject *DCALL
DeeArg_GetKwStringLenHashDef(size_t argc, DeeObject *const *argv,
                             DeeObject *kw, char const *__restrict name,
                             size_t namelen, dhash_t hash,
                             DeeObject *def) {
	if (!kw) {
return_def:
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if (num_keywords > argc)
			goto return_def;
		kw_index = kwds_findstr_len((Kwds *)kw, name, namelen, hash);
		if (kw_index == (size_t)-1)
			goto return_def;
		ASSERT(kw_index < num_keywords);
		return_reference(argv[(argc - num_keywords) + kw_index]);
	}
	return DeeObject_GetItemStringLenHashDef(kw, name, namelen, hash, def);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ARG_C */
