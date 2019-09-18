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
#include <deemon/tuple.h>
#include <deemon/util/string.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeKwdsObject Kwds;
typedef DeeKwdsMappingObject KwdsMapping;


typedef struct {
	OBJECT_HEAD
	ATOMIC_DATA struct kwds_entry *ki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry             *ki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
	DREF Kwds                     *ki_map;  /* [1..1][const] The associated keywords table. */
} KwdsIterator;

#ifdef CONFIG_NO_THREADS
#define READ_ITER(x)            ((x)->ki_iter)
#else /* CONFIG_NO_THREADS */
#define READ_ITER(x) ATOMIC_READ((x)->ki_iter)
#endif /* !CONFIG_NO_THREADS */

INTDEF DeeTypeObject DeeKwdsIterator_Type;
PRIVATE DREF Kwds *DCALL kwds_ctor(void);

PRIVATE int DCALL
kwdsiter_ctor(KwdsIterator *__restrict self) {
	self->ki_map = kwds_ctor();
	if
		unlikely(!self->ki_map)
	return -1;
	self->ki_iter = self->ki_map->kw_map;
	self->ki_end  = self->ki_map->kw_map + self->ki_map->kw_mask + 1;
	return 0;
}

PRIVATE int DCALL
kwdsiter_copy(KwdsIterator *__restrict self,
              KwdsIterator *__restrict other) {
	self->ki_map  = other->ki_map;
	self->ki_iter = READ_ITER(other);
	self->ki_end  = other->ki_end;
	Dee_Incref(self->ki_map);
	return 0;
}

#define kwdsiter_deep kwdsiter_copy /* Only uses Immutable types, so deepcopy == copy */

PRIVATE int DCALL
kwdsiter_init(KwdsIterator *__restrict self, size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, "o:_KwdsIterator", &self->ki_map) ||
	    DeeObject_AssertTypeExact((DeeObject *)self->ki_map, &DeeKwds_Type))
		return -1;
	Dee_Incref(self->ki_map);
	self->ki_iter = self->ki_map->kw_map;
	self->ki_end  = self->ki_map->kw_map + self->ki_map->kw_mask + 1;
	return 0;
}

PRIVATE void DCALL
kwdsiter_fini(KwdsIterator *__restrict self) {
	Dee_Decref(self->ki_map);
}

PRIVATE void DCALL
kwdsiter_visit(KwdsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ki_map);
}

PRIVATE int DCALL
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

PRIVATE DREF DeeObject *DCALL
kwds_nsi_nextitem(KwdsIterator *__restrict self) {
	DREF DeeObject *value, *result;
#ifdef CONFIG_NO_THREADS
	struct kwds_entry *entry;
	entry = ATOMIC_READ(self->ki_iter);
	for (;;) {
		if (entry >= self->ki_end)
			return ITER_DONE;
		if (entry->ke_name)
			break;
		++entry;
	}
	self->ki_iter = entry + 1;
#else  /* CONFIG_NO_THREADS */
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		entry = old_iter = ATOMIC_READ(self->ki_iter);
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (ATOMIC_CMPXCH(self->ki_iter, old_iter, entry + 1))
			break;
	}
#endif /* !CONFIG_NO_THREADS */
	value = DeeInt_NewSize(entry->ke_index);
	if
		unlikely(!value)
	return NULL;
	result = DeeTuple_Pack(2, entry->ke_name, value);
	Dee_Decref_unlikely(value);
	return result;
}

PRIVATE DREF DeeObject *DCALL
kwds_nsi_nextkey(KwdsIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	struct kwds_entry *entry;
	entry = ATOMIC_READ(self->ki_iter);
	for (;;) {
		if (entry >= self->ki_end)
			return ITER_DONE;
		if (entry->ke_name)
			break;
		++entry;
	}
	self->ki_iter = entry + 1;
#else  /* CONFIG_NO_THREADS */
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		entry = old_iter = ATOMIC_READ(self->ki_iter);
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (ATOMIC_CMPXCH(self->ki_iter, old_iter, entry + 1))
			break;
	}
#endif /* !CONFIG_NO_THREADS */
	return_reference_((DeeObject *)entry->ke_name);
}

PRIVATE DREF DeeObject *DCALL
kwds_nsi_nextvalue(KwdsIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	struct kwds_entry *entry;
	entry = ATOMIC_READ(self->ki_iter);
	for (;;) {
		if (entry >= self->ki_end)
			return ITER_DONE;
		if (entry->ke_name)
			break;
		++entry;
	}
	self->ki_iter = entry + 1;
#else  /* CONFIG_NO_THREADS */
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		entry = old_iter = ATOMIC_READ(self->ki_iter);
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (ATOMIC_CMPXCH(self->ki_iter, old_iter, entry + 1))
			break;
	}
#endif /* !CONFIG_NO_THREADS */
	return DeeInt_NewSize(entry->ke_index);
}


#define DEFINE_FILTERITERATOR_COMPARE(name, op)                            \
	PRIVATE DREF DeeObject *DCALL                                          \
	name(KwdsIterator *__restrict self,                                    \
	     KwdsIterator *__restrict other) {                                 \
		if (DeeObject_AssertTypeExact((DeeObject *)other, Dee_TYPE(self))) \
			return NULL;                                                   \
		return_bool(READ_ITER(self) op READ_ITER(other));                  \
	}
DEFINE_FILTERITERATOR_COMPARE(kwdsiter_eq, ==)
DEFINE_FILTERITERATOR_COMPARE(kwdsiter_ne, !=)
DEFINE_FILTERITERATOR_COMPARE(kwdsiter_lo, <)
DEFINE_FILTERITERATOR_COMPARE(kwdsiter_le, <=)
DEFINE_FILTERITERATOR_COMPARE(kwdsiter_gr, >)
DEFINE_FILTERITERATOR_COMPARE(kwdsiter_ge, >=)
#undef DEFINE_FILTERITERATOR_COMPARE

PRIVATE struct type_cmp kwdsiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwdsiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwdsiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwdsiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwdsiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwdsiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwdsiter_ge,
};

PRIVATE struct type_member kwdsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(KwdsIterator, ki_map), "->?Ert:Kwds"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeKwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwdsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&kwdsiter_ctor,
				/* .tp_copy_ctor = */ (void *)&kwdsiter_copy,
				/* .tp_deep_ctor = */ (void *)&kwdsiter_deep,
				/* .tp_any_ctor  = */ (void *)&kwdsiter_init,
				TYPE_FIXED_ALLOCATOR(KwdsIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&kwdsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&kwdsiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kwdsiter_visit,
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
	/* .tp_members       = */kwdsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTDEF DREF DeeObject *DCALL
DeeKwds_NewWithHint(size_t num_items) {
	DREF Kwds *result;
	size_t init_mask = 1;
	while (init_mask <= num_items)
		init_mask = (init_mask << 1) | 1;
	result = (DREF Kwds *)DeeObject_Calloc(COMPILER_OFFSETOF(Kwds, kw_map) +
	                                       (init_mask + 1) * sizeof(struct kwds_entry));
	if
		unlikely(!result)
	goto done;
	result->kw_mask = init_mask;
	DeeObject_Init(result, &DeeKwds_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN DREF Kwds *DCALL kwds_rehash(DREF Kwds *__restrict self) {
	DREF Kwds *result;
	size_t i, j, perturb;
	size_t new_mask = (self->kw_mask << 1) | 1;
	result          = (DREF Kwds *)DeeObject_Calloc(COMPILER_OFFSETOF(Kwds, kw_map) +
                                           (new_mask + 1) * sizeof(struct kwds_entry));
	if
		unlikely(!result)
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
INTERN int(DCALL DeeKwds_Append)(DREF DeeObject **__restrict pself,
                                 char const *__restrict name,
                                 size_t name_len, dhash_t hash) {
	dhash_t i, perturb;
	struct kwds_entry *entry;
	DREF Kwds *self = (DREF Kwds *)*pself;
	if (self->kw_size * 2 > self->kw_mask) {
		/* Must allocate a larger map. */
		self = kwds_rehash(self);
		if
			unlikely(!self)
		return -1;
		*pself = (DREF DeeObject *)self;
	}
	ASSERT(self->kw_size < self->kw_mask);
	perturb = i = hash & self->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
	}
	entry->ke_name = (DREF DeeStringObject *)DeeString_NewSized(name, name_len);
	if
		unlikely(!entry->ke_name)
	return -1;
	entry->ke_name->s_hash = hash; /* Remember the hash! */
	entry->ke_index        = self->kw_size++;
	entry->ke_hash         = hash;
	return 0;
}

LOCAL size_t DCALL
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

LOCAL size_t DCALL
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
		if (DeeString_SIZE(entry->ke_name) != namesize)
			continue;
		if (memcmp(DeeString_STR(entry->ke_name), name, namesize * sizeof(char)) != 0)
			continue;
		return entry->ke_index;
	}
	return (size_t)-1;
}




PRIVATE DREF Kwds *DCALL kwds_ctor(void) {
	DREF Kwds *result;
	result = (DREF Kwds *)DeeObject_Malloc(COMPILER_OFFSETOF(Kwds, kw_map) +
	                                       (2 * sizeof(struct kwds_entry)));
	if
		unlikely(!result)
	goto done;
	result->kw_map[0].ke_name = NULL;
	result->kw_map[1].ke_name = NULL;
	result->kw_mask           = 1;
	result->kw_size           = 0;
	DeeObject_Init(result, &DeeKwds_Type);
done:
	return result;
}

PRIVATE DREF Kwds *DCALL
kwds_init(size_t argc, DeeObject **__restrict argv) {
	/* TODO */
	(void)argc;
	(void)argv;
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE void DCALL
kwds_fini(Kwds *__restrict self) {
	size_t i;
	for (i = 0; i <= self->kw_mask; ++i)
		Dee_XDecref(self->kw_map[i].ke_name);
}

PRIVATE DREF DeeObject *DCALL
kwds_repr(Kwds *__restrict self) {
	size_t i;
	bool is_first                  = true;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (UNICODE_PRINTER_PRINT(&printer, "{ ") < 0)
		goto err;
	for (i = 0; i <= self->kw_mask; ++i) {
		if (!self->kw_map[i].ke_name)
			continue;
		if (!is_first && UNICODE_PRINTER_PRINT(&printer, ", ") < 0)
			goto err;
		if (unicode_printer_printf(&printer, "%r: %Iu",
		                           self->kw_map[i].ke_name,
		                           self->kw_map[i].ke_index) < 0)
			goto err;
		is_first = false;
	}
	if (UNICODE_PRINTER_PRINT(&printer, " }") < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE int DCALL
kwds_bool(Kwds *__restrict self) {
	return self->kw_size != 0;
}


PRIVATE DREF KwdsIterator *DCALL
kwds_iter(Kwds *__restrict self) {
	DREF KwdsIterator *result;
	result = DeeObject_MALLOC(KwdsIterator);
	if
		unlikely(!result)
	goto done;
	result->ki_iter = self->kw_map;
	result->ki_end  = self->kw_map + self->kw_mask + 1;
	result->ki_map  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeKwdsIterator_Type);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
kwds_size(Kwds *__restrict self) {
	return DeeInt_NewSize(self->kw_size);
}

PRIVATE size_t DCALL
kwds_nsi_getsize(Kwds *__restrict self) {
	return self->kw_size;
}

PRIVATE DREF DeeObject *DCALL
kwds_contains(Kwds *__restrict self,
              DeeObject *__restrict key) {
	if (!DeeString_Check(key))
		goto nope;
	return_bool(kwds_findstr(self,
	                         DeeString_STR(key),
	                         DeeString_Hash(key)) !=
	            (size_t)-1);
nope:
	return_false;
}

PRIVATE DREF DeeObject *DCALL
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

PRIVATE DREF DeeObject *DCALL
kwds_get(Kwds *__restrict self,
         DeeObject *__restrict key) {
	DREF DeeObject *result;
	result = kwds_nsi_getdefault(self, key, ITER_DONE);
	if (result != ITER_DONE)
		return result;
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE struct type_nsi kwds_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (void *)&kwds_nsi_getsize,
			/* .nsi_nextkey    = */ (void *)&kwds_nsi_nextkey,
			/* .nsi_nextvalue  = */ (void *)&kwds_nsi_nextvalue,
			/* .nsi_getdefault = */ (void *)&kwds_nsi_getdefault
		}
	}
};


PRIVATE struct type_seq kwds_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwds_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwds_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwds_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kwds_get,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &kwds_nsi
};

PRIVATE struct type_member kwds_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeKwdsIterator_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeKwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Kwds",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (void *)&kwds_ctor,
				/* .tp_copy_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (void *)&kwds_init,
				/* .tp_free      = */ (void *)NULL
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&kwds_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwds_repr,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&kwds_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
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
	/* .tp_class_members = */kwds_class_members
};
#undef READ_ITER



typedef struct {
	OBJECT_HEAD
	ATOMIC_DATA struct kwds_entry *ki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry             *ki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
	DREF KwdsMapping              *ki_map;  /* [1..1][const] The associated keywords mapping. */
} KmapIterator;

#ifdef CONFIG_NO_THREADS
#define READ_ITER(x)            ((x)->ki_iter)
#else /* CONFIG_NO_THREADS */
#define READ_ITER(x) ATOMIC_READ((x)->ki_iter)
#endif /* !CONFIG_NO_THREADS */

STATIC_ASSERT(COMPILER_OFFSETOF(KwdsIterator, ki_iter) ==
              COMPILER_OFFSETOF(KmapIterator, ki_iter));
STATIC_ASSERT(COMPILER_OFFSETOF(KwdsIterator, ki_end) ==
              COMPILER_OFFSETOF(KmapIterator, ki_end));
STATIC_ASSERT(COMPILER_OFFSETOF(KwdsIterator, ki_map) ==
              COMPILER_OFFSETOF(KmapIterator, ki_map));


PRIVATE int DCALL
kmapiter_ctor(KmapIterator *__restrict self) {
	self->ki_map = (DREF KwdsMapping *)DeeObject_NewDefault(&DeeKwdsMapping_Type);
	if
		unlikely(!self->ki_map)
	goto err;
	self->ki_iter = self->ki_map->kmo_kwds->kw_map;
	self->ki_end  = self->ki_map->kmo_kwds->kw_map + self->ki_map->kmo_kwds->kw_mask + 1;
	return 0;
err:
	return -1;
}

#define kmapiter_copy kwdsiter_copy
#define kmapiter_deep kwdsiter_deep
PRIVATE int DCALL
kmapiter_init(KmapIterator *__restrict self, size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, "o:_KwdsMappingIterator", &self->ki_map))
		goto err;
	if (DeeObject_AssertTypeExact((DeeObject *)self->ki_map, &DeeKwdsMapping_Type))
		goto err;
	Dee_Incref(self->ki_map);
	self->ki_iter = self->ki_map->kmo_kwds->kw_map;
	self->ki_end  = self->ki_map->kmo_kwds->kw_map + self->ki_map->kmo_kwds->kw_mask + 1;
	return 0;
err:
	return -1;
}

#define kmapiter_fini kwdsiter_fini
PRIVATE void DCALL
kmapiter_visit(KmapIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ki_map);
}

PRIVATE int DCALL
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
#ifndef CONFIG_NO_THREADS
	return ATOMIC_READ(self->ki_map->kmo_argv) != NULL;
#else
	return self->ki_map->kmo_argv != NULL;
#endif
}

PRIVATE DREF DeeObject *DCALL
kmap_nsi_nextitem(KmapIterator *__restrict self) {
	DREF DeeObject *value, *result;
#ifdef CONFIG_NO_THREADS
	struct kwds_entry *entry;
	entry = ATOMIC_READ(self->ki_iter);
	for (;;) {
		if (entry >= self->ki_end)
			return ITER_DONE;
		if (entry->ke_name)
			break;
		++entry;
	}
	self->ki_iter = entry + 1;
#else
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		entry = old_iter = ATOMIC_READ(self->ki_iter);
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (ATOMIC_CMPXCH(self->ki_iter, old_iter, entry + 1))
			break;
	}
#endif
	rwlock_read(&self->ki_map->kmo_lock);
	if
		unlikely(!self->ki_map->kmo_argv)
	{
		rwlock_endread(&self->ki_map->kmo_lock);
		return ITER_DONE;
	}
	value = self->ki_map->kmo_argv[entry->ke_index];
	Dee_Incref(value);
	rwlock_endread(&self->ki_map->kmo_lock);
	result = DeeTuple_Pack(2, entry->ke_name, value);
	Dee_Decref_unlikely(value);
	return result;
}

PRIVATE DREF DeeObject *DCALL
kmap_nsi_nextkey(KmapIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	struct kwds_entry *entry;
	entry = ATOMIC_READ(self->ki_iter);
	for (;;) {
		if (entry >= self->ki_end)
			return ITER_DONE;
		if (entry->ke_name)
			break;
		++entry;
	}
	self->ki_iter = entry + 1;
#else
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		entry = old_iter = ATOMIC_READ(self->ki_iter);
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (ATOMIC_CMPXCH(self->ki_iter, old_iter, entry + 1))
			break;
	}
#endif
	if
		unlikely(!ATOMIC_READ(self->ki_map->kmo_argv))
	return ITER_DONE;
	return_reference_((DeeObject *)entry->ke_name);
}

PRIVATE DREF DeeObject *DCALL
kmap_nsi_nextvalue(KmapIterator *__restrict self) {
	DREF DeeObject *value;
#ifdef CONFIG_NO_THREADS
	struct kwds_entry *entry;
	entry = ATOMIC_READ(self->ki_iter);
	for (;;) {
		if (entry >= self->ki_end)
			return ITER_DONE;
		if (entry->ke_name)
			break;
		++entry;
	}
	self->ki_iter = entry + 1;
#else
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		entry = old_iter = ATOMIC_READ(self->ki_iter);
		for (;;) {
			if (entry >= self->ki_end)
				return ITER_DONE;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (ATOMIC_CMPXCH(self->ki_iter, old_iter, entry + 1))
			break;
	}
#endif
	rwlock_read(&self->ki_map->kmo_lock);
	if
		unlikely(!self->ki_map->kmo_argv)
	{
		rwlock_endread(&self->ki_map->kmo_lock);
		return ITER_DONE;
	}
	value = self->ki_map->kmo_argv[entry->ke_index];
	Dee_Incref(value);
	rwlock_endread(&self->ki_map->kmo_lock);
	return value;
}

#define kmapiter_cmp     kwdsiter_cmp
#define kmapiter_members kwdsiter_members

INTERN DeeTypeObject DeeKwdsMappingIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwdsMappingIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&kmapiter_ctor,
				/* .tp_copy_ctor = */ (void *)&kmapiter_copy,
				/* .tp_deep_ctor = */ (void *)&kmapiter_deep,
				/* .tp_any_ctor  = */ (void *)&kmapiter_init,
				TYPE_FIXED_ALLOCATOR(KmapIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&kmapiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&kmapiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kmapiter_visit,
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
	/* .tp_members       = */kmapiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




PRIVATE int DCALL
kmap_ctor(KwdsMapping *__restrict self) {
	self->kmo_kwds = kwds_ctor();
	if
		unlikely(!self->kmo_kwds)
	goto err;
	self->kmo_argv = NULL;
	rwlock_init(&self->kmo_lock);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
kmap_copy(KwdsMapping *__restrict self,
          KwdsMapping *__restrict other) {
	size_t i, count;
	count          = other->kmo_kwds->kw_size;
	self->kmo_argv = (DREF DeeObject **)Dee_Malloc(count *
	                                               sizeof(DREF DeeObject *));
	if
		unlikely(!self->kmo_argv)
	goto err;
	rwlock_read(&other->kmo_lock);
	for (i = 0; i < count; ++i) {
		self->kmo_argv[i] = other->kmo_argv[i];
		Dee_Incref(self->kmo_argv[i]);
	}
	rwlock_endread(&other->kmo_lock);
	rwlock_init(&self->kmo_lock);
	self->kmo_kwds = other->kmo_kwds;
	Dee_Incref(self->kmo_kwds);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
kmap_deep(KwdsMapping *__restrict self,
          KwdsMapping *__restrict other) {
	size_t i, count;
	count          = other->kmo_kwds->kw_size;
	self->kmo_argv = (DREF DeeObject **)Dee_Malloc(count *
	                                               sizeof(DREF DeeObject *));
	if
		unlikely(!self->kmo_argv)
	goto err;
	rwlock_read(&other->kmo_lock);
	for (i = 0; i < count; ++i) {
		self->kmo_argv[i] = other->kmo_argv[i];
		Dee_Incref(self->kmo_argv[i]);
	}
	rwlock_endread(&other->kmo_lock);
	/* Construct deep copies of all of the arguments. */
	for (i = 0; i < count; ++i) {
		if (DeeObject_InplaceDeepCopy(&self->kmo_argv[i]))
			goto err_r_argv;
	}
	rwlock_init(&self->kmo_lock);
	self->kmo_kwds = other->kmo_kwds;
	Dee_Incref(self->kmo_kwds);
	return 0;
err_r_argv:
	i = count;
	while (i--)
		Dee_Decref(self->kmo_argv[i]);
	Dee_Free(self->kmo_argv);
err:
	return -1;
}

PRIVATE int DCALL
kmap_init(KwdsMapping *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *args;
	size_t i;
	if (DeeArg_Unpack(argc, argv, "oo:_KwdsMapping", &self->kmo_kwds, &args))
		goto err;
	if (DeeObject_AssertTypeExact((DeeObject *)self->kmo_kwds, &DeeKwds_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if (DeeTuple_SIZE(args) != self->kmo_kwds->kw_size) {
		return err_keywords_bad_for_argc(DeeTuple_SIZE(args),
		                                 self->kmo_kwds->kw_size);
	}
	self->kmo_argv = (DREF DeeObject **)Dee_Malloc(DeeTuple_SIZE(args) *
	                                               sizeof(DREF DeeObject *));
	if
		unlikely(!self->kmo_argv)
	goto err;
	for (i = 0; i < DeeTuple_SIZE(args); ++i) {
		self->kmo_argv[i] = DeeTuple_GET(args, i);
		Dee_Incref(self->kmo_argv[i]);
	}
	Dee_Incref(self->kmo_kwds);
	rwlock_init(&self->kmo_lock);
	return 0;
err:
	return -1;
}

PRIVATE void DCALL
kmap_fini(KwdsMapping *__restrict self) {
	if (self->kmo_argv) {
		size_t count = self->kmo_kwds->kw_size;
		while (count--)
			Dee_Decref(self->kmo_argv[count]);
		Dee_Free(self->kmo_argv);
	}

	Dee_Decref(self->kmo_kwds);
}

PRIVATE void DCALL
kmap_visit(KwdsMapping *__restrict self, dvisit_t proc, void *arg) {
	rwlock_read(&self->kmo_lock);
	if (self->kmo_argv) {
		size_t count = self->kmo_kwds->kw_size;
		while (count--)
			Dee_Visit(self->kmo_argv[count]);
	}
	rwlock_endread(&self->kmo_lock);
	/*Dee_Visit(self->kmo_kwds);*/ /* Only ever references strings, so there'd be no point. */
}

PRIVATE int DCALL
kmap_bool(KwdsMapping *__restrict self) {
#ifndef CONFIG_NO_THREADS
	if (!ATOMIC_READ(self->kmo_argv))
		return 0;
#else
	if (!self->kmo_argv)
		return 0;
#endif
	return self->kmo_kwds->kw_size != 0;
}

PRIVATE DREF KmapIterator *DCALL
kmap_iter(KwdsMapping *__restrict self) {
	DREF KmapIterator *result;
	result = DeeObject_MALLOC(KmapIterator);
	if
		unlikely(!result)
	goto done;
	result->ki_iter = self->kmo_kwds->kw_map;
	result->ki_end  = self->kmo_kwds->kw_map + self->kmo_kwds->kw_mask + 1;
	result->ki_map  = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeKwdsMappingIterator_Type);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
kmap_size(KwdsMapping *__restrict self) {
#ifndef CONFIG_NO_THREADS
	if (!ATOMIC_READ(self->kmo_argv))
		return_reference_(&DeeInt_Zero);
#else
	if (!self->kmo_argv)
		return_reference_(&DeeInt_Zero);
#endif
	return DeeInt_NewSize(self->kmo_kwds->kw_size);
}

PRIVATE size_t DCALL
kmap_nsi_getsize(KwdsMapping *__restrict self) {
#ifndef CONFIG_NO_THREADS
	if (!ATOMIC_READ(self->kmo_argv))
		return 0;
#else
	if (!self->kmo_argv)
		return 0;
#endif
	return self->kmo_kwds->kw_size;
}

PRIVATE DREF DeeObject *DCALL
kmap_contains(KwdsMapping *__restrict self,
              DeeObject *__restrict key) {
#ifndef CONFIG_NO_THREADS
	if (!ATOMIC_READ(self->kmo_argv))
		goto nope;
#else
	if (!self->kmo_argv)
		goto nope;
#endif
	if (!DeeString_Check(key))
		goto nope;
	return_bool(kwds_findstr(self->kmo_kwds,
	                         DeeString_STR(key),
	                         DeeString_Hash(key)) !=
	            (size_t)-1);
nope:
	return_false;
}

PRIVATE DREF DeeObject *DCALL
kmap_nsi_getdefault(KwdsMapping *__restrict self,
                    DeeObject *__restrict key,
                    DeeObject *__restrict def) {
	size_t index;
	DREF DeeObject *result;
	if (!DeeString_Check(key))
		goto nope;
	index = kwds_findstr(self->kmo_kwds,
	                     DeeString_STR(key),
	                     DeeString_Hash(key));
	if (index == (size_t)-1)
		goto nope;
	rwlock_read(&self->kmo_lock);
	if
		unlikely(!self->kmo_argv)
	{
		rwlock_endread(&self->kmo_lock);
		goto nope;
	}
	ASSERT(index < self->kmo_kwds->kw_size);
	result = self->kmo_argv[index];
	Dee_Incref(result);
	rwlock_endread(&self->kmo_lock);
	return result;
nope:
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

PRIVATE DREF DeeObject *DCALL
kmap_get(KwdsMapping *__restrict self,
         DeeObject *__restrict key) {
	DREF DeeObject *result;
	result = kmap_nsi_getdefault(self, key, ITER_DONE);
	if (result != ITER_DONE)
		return result;
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE struct type_nsi kmap_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (void *)&kmap_nsi_getsize,
			/* .nsi_nextkey    = */ (void *)&kmap_nsi_nextkey,
			/* .nsi_nextvalue  = */ (void *)&kmap_nsi_nextvalue,
			/* .nsi_getdefault = */ (void *)&kmap_nsi_getdefault
		}
	}
};

PRIVATE struct type_seq kmap_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmap_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmap_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kmap_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&kmap_get,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &kmap_nsi
};


PRIVATE struct type_member kmap_members[] = {
	TYPE_MEMBER_FIELD_DOC("__kwds__", STRUCT_OBJECT, offsetof(KwdsMapping, kmo_kwds), "->?Ert:Kwds"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member kmap_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeKwdsMappingIterator_Type),
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
				/* .tp_ctor      = */ (void *)&kmap_ctor,
				/* .tp_copy_ctor = */ (void *)&kmap_copy,
				/* .tp_deep_ctor = */ (void *)&kmap_deep,
				/* .tp_any_ctor  = */ (void *)&kmap_init,
				TYPE_FIXED_ALLOCATOR(KwdsMapping)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&kmap_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&kmap_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kmap_visit,
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
 * The returned object then a mapping {(string,object)...} for the
 * actual argument values passed to the function.
 * NOTE: The caller must later invoke `DeeKwdsMapping_Decref()' in order
 *       to clean up the returned object. */
PUBLIC DREF DeeObject *DCALL
DeeKwdsMapping_New(/*Kwds*/ DeeObject *__restrict kwds,
                   DeeObject **__restrict argv) {
	DREF KwdsMapping *result;
	ASSERT_OBJECT_TYPE_EXACT(kwds, &DeeKwds_Type);
	result = DeeObject_MALLOC(KwdsMapping);
	if
		unlikely(!result)
	goto done;
	result->kmo_argv = argv;
	result->kmo_kwds = (DREF DeeKwdsObject *)kwds;
	rwlock_init(&result->kmo_lock);
	DeeObject_Init(result, &DeeKwdsMapping_Type);
done:
	return (DREF DeeObject *)result;
}

/* Unshare the argument vector from a keywords-mapping object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
PUBLIC void DCALL
DeeKwdsMapping_Decref(DREF DeeObject *__restrict self) {
	DREF KwdsMapping *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me = (DREF KwdsMapping *)self;
	if (DeeObject_IsShared(me)) {
		/* The mapping is being shared, so we must construct a copy of its argument list. */
		size_t i, argc = me->kmo_kwds->kw_size;
		DREF DeeObject **argv;
		if (!argc) {
		clear_argv:
			rwlock_write(&me->kmo_lock);
			me->kmo_argv = NULL;
			rwlock_endwrite(&me->kmo_lock);
		} else {
			do
				argv = (DREF DeeObject **)Dee_TryMalloc(argc * sizeof(DREF DeeObject *));
			while (unlikely(!argv) && Dee_TryCollectMemory(argc * sizeof(DREF DeeObject *)));
			if
				unlikely(!argv)
			goto clear_argv;
			rwlock_write(&me->kmo_lock);
			if
				unlikely(!me->kmo_argv)
			{
				/* Shouldn't really happen, but is allowed by the specs... */
				Dee_Free(argv);
				argv = NULL;
			}
			else {
				MEMCPY_PTR(argv, me->kmo_argv, argc);
				for (i = 0; i < argc; ++i)
					Dee_Incref(argv[i]);
			}
			me->kmo_argv = argv; /* Remember the old arguments. */
			rwlock_endwrite(&me->kmo_lock);
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



INTERN bool DCALL
DeeKwdsMapping_HasItemString(DeeObject *__restrict self,
                             char const *__restrict name,
                             dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr(me->kmo_kwds, name, hash);
	if
		unlikely(index == (size_t)-1)
	return false;
#ifdef CONFIG_NO_THREADS
	if
		unlikely(!me->kmo_argv)
	return false;
#else
	if
		unlikely(!ATOMIC_READ(me->kmo_argv))
	return false;
#endif
	return true;
}

INTERN bool DCALL
DeeKwdsMapping_HasItemStringLen(DeeObject *__restrict self,
                                char const *__restrict name,
                                size_t namesize,
                                dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr_len(me->kmo_kwds, name, namesize, hash);
	if
		unlikely(index == (size_t)-1)
	return false;
#ifdef CONFIG_NO_THREADS
	if
		unlikely(!me->kmo_argv)
	return false;
#else
	if
		unlikely(!ATOMIC_READ(me->kmo_argv))
	return false;
#endif
	return true;
}

INTERN DREF DeeObject *DCALL
DeeKwdsMapping_GetItemString(DeeObject *__restrict self,
                             char const *__restrict name,
                             dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr(me->kmo_kwds, name, hash);
	if
		unlikely(index == (size_t)-1)
	{
	no_such_key:
		err_unknown_key_str((DeeObject *)self, name);
		return NULL;
	}
	rwlock_read(&me->kmo_lock);
	if
		unlikely(!me->kmo_argv)
	{
		rwlock_endread(&me->kmo_lock);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	rwlock_endread(&me->kmo_lock);
	return result;
}

INTERN DREF DeeObject *DCALL
DeeKwdsMapping_GetItemStringDef(DeeObject *__restrict self,
                                char const *__restrict name,
                                dhash_t hash,
                                DeeObject *__restrict def) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr(me->kmo_kwds, name, hash);
	if
		unlikely(index == (size_t)-1)
	{
	no_such_key:
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	rwlock_read(&me->kmo_lock);
	if
		unlikely(!me->kmo_argv)
	{
		rwlock_endread(&me->kmo_lock);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	rwlock_endread(&me->kmo_lock);
	return result;
}

INTERN DREF DeeObject *DCALL
DeeKwdsMapping_GetItemStringLen(DeeObject *__restrict self,
                                char const *__restrict name,
                                size_t namesize,
                                dhash_t hash) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr_len(me->kmo_kwds, name, namesize, hash);
	if
		unlikely(index == (size_t)-1)
	{
	no_such_key:
		err_unknown_key_str_len((DeeObject *)self, name, namesize);
		return NULL;
	}
	rwlock_read(&me->kmo_lock);
	if
		unlikely(!me->kmo_argv)
	{
		rwlock_endread(&me->kmo_lock);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	rwlock_endread(&me->kmo_lock);
	return result;
}

INTERN DREF DeeObject *DCALL
DeeKwdsMapping_GetItemStringLenDef(DeeObject *__restrict self,
                                   char const *__restrict name,
                                   size_t namesize,
                                   dhash_t hash,
                                   DeeObject *__restrict def) {
	size_t index;
	KwdsMapping *me;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me    = (KwdsMapping *)self;
	index = kwds_findstr_len(me->kmo_kwds, name, namesize, hash);
	if
		unlikely(index == (size_t)-1)
	{
	no_such_key:
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	rwlock_read(&me->kmo_lock);
	if
		unlikely(!me->kmo_argv)
	{
		rwlock_endread(&me->kmo_lock);
		goto no_such_key;
	}
	ASSERT(index < me->kmo_kwds->kw_size);
	result = me->kmo_argv[index];
	Dee_Incref(result);
	rwlock_endread(&me->kmo_lock);
	return result;
}


/* Construct/access keyword arguments passed to a function as a
 * high-level {(string,object)...}-like mapping that is bound to
 * the actually mapped arguments. */
PUBLIC DREF DeeObject *DCALL
DeeArg_GetKw(size_t *__restrict pargc,
             DeeObject **__restrict argv,
             DeeObject *kw) {
	if (!kw)
		return_reference_(Dee_EmptyMapping);
	if (DeeKwds_Check(kw)) {
		size_t num_keywords;
		num_keywords = DeeKwds_SIZE(kw);
		if
			unlikely(num_keywords > *pargc)
		{
			/* Argument list is too short of the given keywords */
			err_keywords_bad_for_argc(*pargc, num_keywords);
			return NULL;
		}
		*pargc -= num_keywords;
		/* Turn keywords and arguments into a proper mapping-like object. */
		return DeeKwdsMapping_New(kw, argv + *pargc);
	}
	/* `kw' already is a user-provided mapping. */
	return_reference_(kw);
}

PUBLIC void DCALL
DeeArg_PutKw(size_t argc, DeeObject **__restrict argv, DeeObject *__restrict kw) {
	if (DeeKwdsMapping_Check(kw) &&
	    ((DeeKwdsMappingObject *)kw)->kmo_argv == argv + argc) {
		/* If we're the ones owning the keywords-mapping, we must also decref() it. */
		DeeKwdsMapping_Decref(kw);
	} else {
		Dee_Decref(kw);
	}
}


PUBLIC DREF DeeObject *DCALL
DeeArg_GetKwString(size_t argc, DeeObject **__restrict argv,
                   DeeObject *kw, char const *__restrict name) {
	dhash_t hash;
	if (!kw) {
		err_unknown_key_str(Dee_EmptyMapping, name);
		return NULL;
	}
	hash = Dee_HashStr(name);
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if
			unlikely(num_keywords > argc)
		{
			/* Argument list is too short of the given keywords */
			err_keywords_bad_for_argc(argc, num_keywords);
			return NULL;
		}
		kw_index = kwds_findstr((Kwds *)kw, name, hash);
		if
			unlikely(kw_index == (size_t)-1)
		{
			err_keywords_not_found(name);
			return NULL;
		}
		ASSERT(kw_index < num_keywords);
		return_reference(argv[(argc - num_keywords) + kw_index]);
	}
	return DeeObject_GetItemString(kw, name, hash);
}

PUBLIC DREF DeeObject *DCALL
DeeArg_GetKwStringLen(size_t argc, DeeObject **__restrict argv, DeeObject *kw,
                      char const *__restrict name, size_t namelen, dhash_t hash) {
	if (!kw) {
		err_unknown_key_str_len(Dee_EmptyMapping, name, namelen);
		return NULL;
	}
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if
			unlikely(num_keywords > argc)
		{
			/* Argument list is too short of the given keywords */
			err_keywords_bad_for_argc(argc, num_keywords);
			return NULL;
		}
		kw_index = kwds_findstr_len((Kwds *)kw, name, namelen, hash);
		if
			unlikely(kw_index == (size_t)-1)
		{
			err_keywords_not_found(name);
			return NULL;
		}
		ASSERT(kw_index < num_keywords);
		return_reference(argv[(argc - num_keywords) + kw_index]);
	}
	return DeeObject_GetItemStringLen(kw, name, namelen, hash);
}

PUBLIC DREF DeeObject *DCALL
DeeArg_GetKwStringDef(size_t argc, DeeObject **__restrict argv,
                      DeeObject *kw, char const *__restrict name,
                      DeeObject *__restrict def) {
	dhash_t hash;
	if (!kw) {
	return_def:
		if (def != ITER_DONE)
			Dee_Incref(def);
		return def;
	}
	hash = Dee_HashStr(name);
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if
			unlikely(num_keywords > argc)
		goto return_def;
		kw_index = kwds_findstr((Kwds *)kw, name, hash);
		if (kw_index == (size_t)-1)
			goto return_def;
		ASSERT(kw_index < num_keywords);
		return_reference(argv[(argc - num_keywords) + kw_index]);
	}
	return DeeObject_GetItemStringDef(kw, name, hash, def);
}

PUBLIC DREF DeeObject *DCALL
DeeArg_GetKwStringLenDef(size_t argc, DeeObject **__restrict argv,
                         DeeObject *kw, char const *__restrict name,
                         size_t namelen, dhash_t hash,
                         DeeObject *__restrict def) {
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
	return DeeObject_GetItemStringLenDef(kw, name, namelen, hash, def);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ARG_C */
