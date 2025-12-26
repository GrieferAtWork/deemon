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
#ifndef GUARD_DEEMON_RUNTIME_KWDS_C
#define GUARD_DEEMON_RUNTIME_KWDS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/cached-dict.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
/**/

#include <stddef.h> /* offsetof */

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
	DeeArg_Unpack1(err, argc, argv, "_KwdsIterator", &self->ki_map);
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
kwdsiter_visit(KwdsIterator *__restrict self, Dee_visit_t proc, void *arg) {
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kwdsiter_nextpair(KwdsIterator *__restrict self,
                 DREF DeeObject *key_and_value[2]) {
	DREF DeeObject *value;
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return 1;
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
	Dee_Incref(entry->ke_name);
	key_and_value[0] = Dee_AsObject(entry->ke_name);
	key_and_value[1] = value;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwdsiter_nextkey(KwdsIterator *__restrict self) {
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
kwdsiter_nextvalue(KwdsIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
kwdsiter_advance(KwdsIterator *__restrict self, size_t step) {
	size_t result;
	for (;;) {
		struct kwds_entry *old_iter, *new_iter;
		result   = 0;
		old_iter = atomic_read(&self->ki_iter);
		new_iter    = old_iter;
		for (; step; --step) {
			for (;;) {
				if (new_iter >= self->ki_end)
					goto halt;
				if ((new_iter++)->ke_name)
					break;
			}
			++result;
		}
halt:
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, new_iter))
			break;
	}
	return result;
}

PRIVATE struct type_iterator kwdsiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&kwdsiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwdsiter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwdsiter_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&kwdsiter_advance,
};


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
kwdsiter_hash(KwdsIterator *self) {
	return Dee_HashPointer(READ_ITER(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kwdsiter_compare(KwdsIterator *self, KwdsIterator *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compareT(struct kwds_entry *, READ_ITER(self),
	                    /*                */ READ_ITER(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp kwdsiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&kwdsiter_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&kwdsiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_member tpconst kwdsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(KwdsIterator, ki_map), "->?Ert:Kwds"),
	TYPE_MEMBER_END
};

#undef READ_ITER

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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ KwdsIterator,
			/* tp_ctor:        */ &kwdsiter_ctor,
			/* tp_copy_ctor:   */ &kwdsiter_copy,
			/* tp_deep_ctor:   */ &kwdsiter_deep,
			/* tp_any_ctor:    */ &kwdsiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwdsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&kwdsiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&kwdsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &kwdsiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &kwdsiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ kwdsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


INTDEF WUNUSED DREF DeeObject *DCALL
DeeKwds_NewWithHint(size_t num_items) {
	DREF Kwds *result;
	size_t init_mask = 1;
	while (init_mask <= num_items)
		init_mask = (init_mask << 1) | 1;
	result = (DREF Kwds *)DeeObject_Callocc(offsetof(Kwds, kw_map),
	                                        init_mask + 1,
	                                        sizeof(struct kwds_entry));
	if unlikely(!result)
		goto done;
	result->kw_mask = init_mask;
	DeeObject_Init(result, &DeeKwds_Type);
done:
	return Dee_AsObject(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF Kwds *DCALL
kwds_rehash(DREF Kwds *__restrict self) {
	DREF Kwds *result;
	size_t i, j, perturb;
	size_t new_mask = (self->kw_mask << 1) | 1;
	result = (DREF Kwds *)DeeObject_Callocc(offsetof(Kwds, kw_map),
	                                        new_mask + 1,
	                                        sizeof(struct kwds_entry));
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
                                    size_t name_len, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	struct kwds_entry *entry;
	DREF Kwds *self = (DREF Kwds *)*p_self;
	if (self->kw_size * 2 > self->kw_mask) {
		/* Must allocate a larger map. */
		self = kwds_rehash(self);
		if unlikely(!self)
			goto err;
		*p_self = Dee_AsObject(self);
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
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeKwds_Append)(/*DREF DeeObject **p_self */ void *arg,
                       DeeObject *__restrict name) {
	DREF DeeObject **p_self = (DREF DeeObject **)arg;
	Dee_hash_t i, perturb, hash;
	struct kwds_entry *entry;
	DREF Kwds *self = (DREF Kwds *)*p_self;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if (self->kw_size * 2 > self->kw_mask) {
		/* Must allocate a larger map. */
		self = kwds_rehash(self);
		if unlikely(!self)
			goto err;
		*p_self = Dee_AsObject(self);
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
INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) struct Dee_kwds_entry *DCALL
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



PRIVATE WUNUSED DREF Kwds *DCALL kwds_ctor(void) {
	DREF Kwds *result;
	result = (DREF Kwds *)DeeObject_Mallocc(offsetof(Kwds, kw_map),
	                                        2, sizeof(struct kwds_entry));
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
	DeeArg_Unpack1(err, argc, argv, "Kwds", &init);
	result = kwds_ctor();
	if unlikely(!result)
		goto err;
	if unlikely(DeeObject_Foreach(init, &DeeKwds_Append, &result))
		goto err_r;
	return result;
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
kwds_visit(Kwds *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->kw_mask; ++i)
		Dee_XVisit(self->kw_map[i].ke_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
kwds_serialize(Kwds *__restrict self, DeeSerial *__restrict writer) {
	Kwds *out;
	size_t i, sizeof_kwds = offsetof(Kwds, kw_map) + (self->kw_mask + 1) * sizeof(struct kwds_entry);
	Dee_seraddr_t addr = DeeSerial_ObjectMalloc(writer, sizeof_kwds, self);
	if (!Dee_SERADDR_ISOK(addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, Kwds);
	out->kw_size = self->kw_size;
	out->kw_mask = self->kw_mask;
	memcpyc(out->kw_map, self->kw_map, self->kw_mask + 1, sizeof(struct kwds_entry));
	for (i = 0; i <= self->kw_mask; ++i) {
		if (self->kw_map[i].ke_name) {
			Dee_seraddr_t addrof_out_entry_name;
			addrof_out_entry_name = addr + offsetof(Kwds, kw_map) +
			                        i * sizeof(struct kwds_entry);
			if unlikely(DeeSerial_PutObject(writer, addrof_out_entry_name,
			                                self->kw_map[i].ke_name))
				goto err;
		}
	}
	return addr;
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
kwds_printrepr(Kwds *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0
	Dee_ssize_t temp, result;
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
kwds_size(Kwds *__restrict self) {
	ASSERT(self->kw_size != (size_t)-1);
	return self->kw_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_contains(Kwds *self, DeeObject *key) {
	if (!DeeString_Check(key))
		goto nope;
	return_bool(DeeKwds_IndexOf(Dee_AsObject(self), key) != (size_t)-1);
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
kwds_foreach_pair(Kwds *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_hash_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i <= self->kw_mask; ++i) {
		DREF DeeObject *indexob;
		struct kwds_entry *entry = &self->kw_map[i];
		if (!entry->ke_name)
			continue;
		indexob = DeeInt_NewSize(entry->ke_index);
		if unlikely(!indexob)
			goto err;
		temp = (*proc)(arg, (DeeObject *)entry->ke_name, indexob);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_getitem(Kwds *self, DeeObject *key) {
	size_t index;
	if (!DeeString_Check(key))
		goto nope;
	index = DeeKwds_IndexOf(Dee_AsObject(self), key);
	if (index == (size_t)-1)
		goto nope;
	return DeeInt_NewSize(index);
nope:
	DeeRT_ErrUnknownKey(self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_getitem_string_hash(Kwds *self, char const *key, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash(Dee_AsObject(self), key, hash);
	if (index == (size_t)-1)
		goto nope;
	return DeeInt_NewSize(index);
nope:
	DeeRT_ErrUnboundKeyStr(Dee_AsObject(self), key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_getitem_string_len_hash(Kwds *self, char const *key, size_t keylen, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash(Dee_AsObject(self), key, keylen, hash);
	if (index == (size_t)-1)
		goto nope;
	return DeeInt_NewSize(index);
nope:
	DeeRT_ErrUnknownKeyStrLen(Dee_AsObject(self), key, keylen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_trygetitem(Kwds *self, DeeObject *key) {
	size_t index;
	if (!DeeString_Check(key))
		goto nope;
	index = DeeKwds_IndexOf(Dee_AsObject(self), key);
	if (index == (size_t)-1)
		goto nope;
	return DeeInt_NewSize(index);
nope:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_trygetitem_string_hash(Kwds *self, char const *key, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash(Dee_AsObject(self), key, hash);
	if (index == (size_t)-1)
		goto nope;
	return DeeInt_NewSize(index);
nope:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwds_trygetitem_string_len_hash(Kwds *self, char const *key, size_t keylen, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash(Dee_AsObject(self), key, keylen, hash);
	if (index == (size_t)-1)
		goto nope;
	return DeeInt_NewSize(index);
nope:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kwds_hasitem(Kwds *self, DeeObject *key) {
	size_t index;
	if (!DeeString_Check(key))
		return 0;
	index = DeeKwds_IndexOf(Dee_AsObject(self), key);
	return index != (size_t)-1 ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kwds_hasitem_string_hash(Kwds *self, char const *key, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash(Dee_AsObject(self), key, hash);
	return index != (size_t)-1 ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kwds_hasitem_string_len_hash(Kwds *self, char const *key, size_t keylen, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash(Dee_AsObject(self), key, keylen, hash);
	return index != (size_t)-1 ? 1 : 0;
}

PRIVATE struct type_seq kwds_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kwds_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwds_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwds_getitem,
	/* .tp_delitem                    = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&kwds_foreach_pair,
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&kwds_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&kwds_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&kwds_size,
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kwds_trygetitem,
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&kwds_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&kwds_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&kwds_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&kwds_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&kwds_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&kwds_hasitem_string_len_hash,
};

PRIVATE struct type_member tpconst kwds_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeKwdsIterator_Type),
	TYPE_MEMBER_CONST(STR_KeyType, &DeeString_Type),
	TYPE_MEMBER_CONST(STR_ValueType, &DeeInt_Type),
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};


PRIVATE struct type_operator const kwds_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeKwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Kwds",
	/* .tp_doc      = */ DOC("(names:?S?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING | TF_KW, /* Instances of this type are allowed in "kw" arguments. */
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &kwds_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ &kwds_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &kwds_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwds_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&kwds_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&kwds_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&kwds_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__2E23147A197C0EE6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E), /* TODO */
	/* .tp_seq           = */ &kwds_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ kwds_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ kwds_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(kwds_operators),
};


/* Translate an argument keyword name into its index within at given `DeeKwdsObject *self'.
 * When `self' doesn't contain a descriptor for `name', no error is thrown, and `(size_t)-1'
 * is returned instead. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
DeeKwds_IndexOf(DeeObject const *__restrict self, /*string*/ DeeObject *__restrict name) {
	Dee_hash_t i, perturb, hash;
	DeeKwdsObject *me = (DeeKwdsObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeKwds_Type);
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash    = DeeString_Hash(name);
	perturb = i = hash & me->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		struct kwds_entry *entry;
		entry = &me->kw_map[i & me->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (DeeString_EqualsSTR(entry->ke_name, name))
			return entry->ke_index;
	}
	return (size_t)-1;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
DeeKwds_IndexOfStringHash(DeeObject const *__restrict self,
                          char const *__restrict name,
                          Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeKwdsObject *me = (DeeKwdsObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeKwds_Type);
	perturb = i = hash & me->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		struct kwds_entry *entry;
		entry = &me->kw_map[i & me->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (strcmp(DeeString_STR(entry->ke_name), name) == 0)
			return entry->ke_index;
	}
	return (size_t)-1;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
DeeKwds_IndexOfStringLenHash(DeeObject const *__restrict self,
                             char const *__restrict name,
                             size_t namelen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeKwdsObject *me = (DeeKwdsObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeKwds_Type);
	perturb = i = hash & me->kw_mask;
	for (;; DeeKwds_MAPNEXT(i, perturb)) {
		struct kwds_entry *entry;
		entry = &me->kw_map[i & me->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (DeeString_EqualsBuf(entry->ke_name, name, namelen) == 0)
			return entry->ke_index;
	}
	return (size_t)-1;
}




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
	DeeArg_Unpack1(err, argc, argv, "_KwdsMappingIterator", &self->ki_map);
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
kmapiter_visit(KmapIterator *__restrict self, Dee_visit_t proc, void *arg) {
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
			return 1;
		++entry;
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kmapiter_nextpair(KmapIterator *__restrict self,
                  DREF DeeObject *key_and_value[2]) {
	DREF DeeObject *value;
	struct kwds_entry *old_iter, *entry;
	for (;;) {
		old_iter = atomic_read(&self->ki_iter);
		entry    = old_iter;
		for (;;) {
			if (entry >= self->ki_end)
				return 1;
			if (entry->ke_name)
				break;
			++entry;
		}
		if (atomic_cmpxch_weak_or_write(&self->ki_iter, old_iter, entry + 1))
			break;
	}
	DeeKwdsMapping_LockRead(self->ki_map);
	value = self->ki_map->kmo_argv[entry->ke_index];
	DeeKwdsMapping_LockEndRead(self->ki_map);
	Dee_Incref(value);
	Dee_Incref(entry->ke_name);
	key_and_value[0] = Dee_AsObject(entry->ke_name);
	key_and_value[1] = value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kmapiter_nextkey(KmapIterator *__restrict self) {
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
kmapiter_nextvalue(KmapIterator *__restrict self) {
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
	value = self->ki_map->kmo_argv[entry->ke_index];
	DeeKwdsMapping_LockEndRead(self->ki_map);
	Dee_Incref(value);
	return value;
}

STATIC_ASSERT(offsetof(KmapIterator, ki_iter) == offsetof(KwdsIterator, ki_iter));
STATIC_ASSERT(offsetof(KmapIterator, ki_end) == offsetof(KwdsIterator, ki_end));
#define kmapiter_advance kwdsiter_advance

PRIVATE struct type_iterator kmapiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&kmapiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmapiter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmapiter_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&kmapiter_advance,
};

#define kmapiter_cmp     kwdsiter_cmp
#define kmapiter_members kwdsiter_members

PRIVATE DeeTypeObject DeeKwdsMappingIterator_Type = {
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ KmapIterator,
			/* tp_ctor:        */ &kmapiter_ctor,
			/* tp_copy_ctor:   */ &kmapiter_copy,
			/* tp_deep_ctor:   */ &kmapiter_deep,
			/* tp_any_ctor:    */ &kmapiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kmapiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&kmapiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&kmapiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &kmapiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &kmapiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ kmapiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



PRIVATE WUNUSED DREF KwdsMapping *DCALL kmap_ctor(void) {
	DREF KwdsMapping *result;
	result = (DREF KwdsMapping *)DeeObject_Malloc(offsetof(KwdsMapping, kmo_args));
	if unlikely(!result)
		goto done;
	result->kmo_argv = result->kmo_args;
	result->kmo_kwds = kwds_ctor();
	if unlikely(!result->kmo_kwds)
		goto err_r;
	Dee_atomic_rwlock_init(&result->kmo_lock);
	DeeObject_Init(result, &DeeKwdsMapping_Type);
done:
	return result;
err_r:
	DeeObject_Free(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF KwdsMapping *DCALL
kmap_copy(KwdsMapping *__restrict self) {
	DREF KwdsMapping *result;
	size_t argc;
	argc   = DeeKwds_SIZE(self->kmo_kwds);
	result = (DREF KwdsMapping *)DeeObject_Mallocc(offsetof(KwdsMapping, kmo_args),
	                                               argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	DeeKwdsMapping_LockRead(self);
	Dee_Movrefv(result->kmo_args, self->kmo_argv, argc);
	DeeKwdsMapping_LockEndRead(self);
	result->kmo_argv = result->kmo_args;
	result->kmo_kwds = self->kmo_kwds;
	Dee_Incref(result->kmo_kwds);
	Dee_atomic_rwlock_init(&result->kmo_lock);
	DeeObject_Init(result, &DeeKwdsMapping_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF KwdsMapping *DCALL
kmap_deep(KwdsMapping *__restrict self) {
	DREF KwdsMapping *result;
	size_t argc;
	argc   = DeeKwds_SIZE(self->kmo_kwds);
	result = (DREF KwdsMapping *)DeeObject_Mallocc(offsetof(KwdsMapping, kmo_args),
	                                               argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	DeeKwdsMapping_LockRead(self);
	Dee_Movrefv(result->kmo_args, self->kmo_argv, argc);
	DeeKwdsMapping_LockEndRead(self);
	if (DeeObject_InplaceDeepCopyv(result->kmo_args, argc))
		goto err_r_argv;
	result->kmo_argv = result->kmo_args;
	result->kmo_kwds = self->kmo_kwds;
	Dee_Incref(result->kmo_kwds);
	Dee_atomic_rwlock_init(&result->kmo_lock);
	DeeObject_Init(result, &DeeKwdsMapping_Type);
done:
	return result;
err_r_argv:
	Dee_Decrefv(result->kmo_args, argc);
/*err_r:*/
	DeeObject_Free(result);
	return NULL;
}

PRIVATE WUNUSED DREF KwdsMapping *DCALL
kmap_init(size_t argc, DeeObject *const *argv) {
	DREF KwdsMapping *result;
	DeeKwdsObject *kwds;
	DeeTupleObject *args;
	size_t kw_argc;
	DeeArg_Unpack2(err, argc, argv, "_KwdsMapping", &kwds, &args);
	if (DeeObject_AssertTypeExact(kwds, &DeeKwds_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	kw_argc = kwds->kw_size;
	if (DeeTuple_SIZE(args) != kw_argc) {
		err_keywords_bad_for_argc((DeeKwdsObject *)kwds,
		                          DeeTuple_SIZE(args),
		                          DeeTuple_ELEM(args));
		goto err;
	}
	result = (DREF KwdsMapping *)DeeObject_Mallocc(offsetof(KwdsMapping, kmo_args),
	                                               kw_argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err;
	result->kmo_argv = Dee_Movrefv(result->kmo_args, DeeTuple_ELEM(args), kw_argc);
	result->kmo_kwds = kwds;
	Dee_Incref(kwds);
	Dee_atomic_rwlock_init(&result->kmo_lock);
	DeeObject_Init(result, &DeeKwdsMapping_Type);
	return result;
err:
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
kmap_fini(KwdsMapping *__restrict self) {
	Dee_Decrefv(self->kmo_argv, self->kmo_kwds->kw_size);
	Dee_Decref(self->kmo_kwds);
}

PRIVATE NONNULL((1, 2)) void DCALL
kmap_visit(KwdsMapping *__restrict self, Dee_visit_t proc, void *arg) {
	DeeKwdsMapping_LockRead(self);
	Dee_Visitv(self->kmo_argv, self->kmo_kwds->kw_size);
	DeeKwdsMapping_LockEndRead(self);
	/*Dee_Visit(self->kmo_kwds);*/ /* Only ever references strings, so there'd be no point. */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
kmap_bool(KwdsMapping *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
kmap_size(KwdsMapping *__restrict self) {
	return self->kmo_kwds->kw_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_contains(KwdsMapping *self, DeeObject *key) {
	size_t index;
	if (!DeeString_Check(key))
		goto nope;
	index = DeeKwds_IndexOf(Dee_AsObject(self->kmo_kwds), key);
	return_bool(index != (size_t)-1);
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
kmap_foreach_pair(KwdsMapping *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_hash_t i;
	Dee_ssize_t temp, result = 0;
	Kwds *kwds = self->kmo_kwds;
	for (i = 0; i <= kwds->kw_mask; ++i) {
		DeeObject *value;
		struct kwds_entry *entry = &kwds->kw_map[i];
		if (!entry->ke_name)
			continue;
		ASSERT(entry->ke_index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		value = self->kmo_argv[entry->ke_index];
		DeeKwdsMapping_LockEndRead(self);
		temp = (*proc)(arg, (DeeObject *)entry->ke_name, value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_getitem(KwdsMapping *self, DeeObject *key) {
	size_t index;
	if (!DeeString_Check(key))
		goto nope;
	index = DeeKwds_IndexOf(Dee_AsObject(self->kmo_kwds), key);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		Dee_Incref(result);
		return result;
	}
nope:
	DeeRT_ErrUnknownKey(self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_getitem_string_hash(KwdsMapping *self, char const *key, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash(Dee_AsObject(self->kmo_kwds), key, hash);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		Dee_Incref(result);
		return result;
	}
	DeeRT_ErrUnboundKeyStr(Dee_AsObject(self), key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_getitem_string_len_hash(KwdsMapping *self, char const *key, size_t keylen, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash(Dee_AsObject(self->kmo_kwds), key, keylen, hash);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		Dee_Incref(result);
		return result;
	}
	DeeRT_ErrUnknownKeyStrLen(Dee_AsObject(self), key, keylen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_trygetitem(KwdsMapping *self, DeeObject *key) {
	size_t index;
	if (!DeeString_Check(key))
		goto nope;
	index = DeeKwds_IndexOf(Dee_AsObject(self->kmo_kwds), key);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		Dee_Incref(result);
		return result;
	}
nope:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_trygetitem_string_hash(KwdsMapping *self, char const *key, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash(Dee_AsObject(self->kmo_kwds), key, hash);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		Dee_Incref(result);
		return result;
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kmap_trygetitem_string_len_hash(KwdsMapping *self, char const *key, size_t keylen, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash(Dee_AsObject(self->kmo_kwds), key, keylen, hash);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		Dee_Incref(result);
		return result;
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kmap_hasitem(KwdsMapping *self, DeeObject *key) {
	return kwds_hasitem(self->kmo_kwds, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kmap_hasitem_string_hash(KwdsMapping *self, char const *key, Dee_hash_t hash) {
	return kwds_hasitem_string_hash(self->kmo_kwds, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
kmap_hasitem_string_len_hash(KwdsMapping *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return kwds_hasitem_string_len_hash(self->kmo_kwds, key, keylen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
kmap_trygetitemnr(DeeKwdsMappingObject *__restrict self,
                  /*string*/ DeeObject *__restrict name) {
	size_t index = DeeKwds_IndexOf(Dee_AsObject(self->kmo_kwds), name);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		return result;
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
kmap_trygetitemnr_string_hash(DeeKwdsMappingObject *__restrict self,
                              char const *__restrict name, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringHash(Dee_AsObject(self->kmo_kwds), name, hash);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		return result;
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
kmap_trygetitemnr_string_len_hash(DeeKwdsMappingObject *__restrict self,
                                  char const *__restrict name,
                                  size_t namelen, Dee_hash_t hash) {
	size_t index = DeeKwds_IndexOfStringLenHash(Dee_AsObject(self->kmo_kwds), name, namelen, hash);
	if likely(index != (size_t)-1) {
		DeeObject *result;
		ASSERT(index < self->kmo_kwds->kw_size);
		DeeKwdsMapping_LockRead(self);
		result = self->kmo_argv[index];
		DeeKwdsMapping_LockEndRead(self);
		return result;
	}
	return ITER_DONE;
}


PRIVATE struct type_seq kmap_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&kmap_iter,
	/* .tp_sizeob                       = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kmap_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kmap_getitem,
	/* .tp_delitem                      = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                      = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                     = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                     = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                     = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                      = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&kmap_foreach_pair,
	/* .tp_bounditem                    = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&kmap_hasitem,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&kmap_size,
	/* .tp_size_fast                    = */ (size_t (DCALL *)(DeeObject *__restrict))&kmap_size,
	/* .tp_getitem_index                = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index                = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index              = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index                = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index               = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index               = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index               = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&kmap_trygetitem,
	/* .tp_trygetitem_index             = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&kmap_trygetitem_string_hash,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&kmap_getitem_string_hash,
	/* .tp_delitem_string_hash          = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash          = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash        = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&kmap_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&kmap_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&kmap_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash      = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash    = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&kmap_hasitem_string_len_hash,
	/* .tp_asvector                     = */ NULL,
	/* .tp_asvector_nothrow             = */ NULL,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&kmap_trygetitemnr,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&kmap_trygetitemnr_string_hash,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&kmap_trygetitemnr_string_len_hash,
};

PRIVATE struct type_member tpconst kmap_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___kwds__, STRUCT_OBJECT, offsetof(KwdsMapping, kmo_kwds), "->?Ert:Kwds"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst kmap_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeKwdsMappingIterator_Type),
	TYPE_MEMBER_CONST(STR_KeyType, &DeeString_Type),
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeKwdsMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwdsMapping",
	/* .tp_doc      = */ DOC("()\n"
	                         "(kwds:?Ert:Kwds,args:?DTuple)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &kmap_ctor,
			/* tp_copy_ctor:   */ &kmap_copy,
			/* tp_deep_ctor:   */ &kmap_deep,
			/* tp_any_ctor:    */ &kmap_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kmap_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&kmap_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&kmap_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__2E23147A197C0EE6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &kmap_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ kmap_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ kmap_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

/* Construct a keywords-mapping object from a given `kwds' object,
 * as well as an argument vector that will be shared with the mapping.
 * The returned object then a mapping {(string, Object)...} for the
 * actual argument values passed to the function.
 * NOTE: The caller must later invoke `DeeKwdsMapping_Decref()' in order
 *       to clean up the returned object. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwdsMapping_New(/*Kwds*/ DeeObject *kwds,
                   DeeObject *const *kw_argv) {
	DREF KwdsMapping *result;
	size_t argc;
	ASSERT_OBJECT_TYPE_EXACT(kwds, &DeeKwds_Type);
	argc   = DeeKwds_SIZE(kwds);
	result = (DREF KwdsMapping *)DeeObject_Mallocc(offsetof(KwdsMapping, kmo_args),
	                                               argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	Dee_atomic_rwlock_init(&result->kmo_lock);
	result->kmo_argv = (DREF DeeObject **)kw_argv; /* Shared (for now) */
	result->kmo_kwds = (DREF DeeKwdsObject *)kwds;
	DeeObject_Init(result, &DeeKwdsMapping_Type);
done:
	return Dee_AsObject(result);
}

/* Unshare the argument vector from a keywords-mapping object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
PUBLIC NONNULL((1)) void DCALL
DeeKwdsMapping_Decref(DREF /*KwdsMapping*/ DeeObject *__restrict self) {
	DREF KwdsMapping *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeKwdsMapping_Type);
	me = (DREF KwdsMapping *)self;
	if (DeeObject_IsShared(me)) {
		/* The mapping is being shared, so we must gift it references. */
		Dee_Movrefv(me->kmo_args, me->kmo_argv, me->kmo_kwds->kw_size);
		DeeKwdsMapping_LockWrite(me);
		me->kmo_argv = me->kmo_args;
		DeeKwdsMapping_LockEndWrite(me);
		Dee_Incref(me->kmo_kwds);
		Dee_Decref_unlikely(self);
	} else {
		/*Dee_Decref(me->kmo_kwds);*/
		Dee_DecrefNokill(&DeeKwdsMapping_Type);
		DeeObject_Free(me);
	}
}


/* Construct/access keyword arguments passed to a function as a
 * high-level {string: Object}-like mapping that is bound to the
 * actually mapped arguments. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwMapping_New(size_t *__restrict p_argc,
                 DeeObject *const *argv,
                 DeeObject *kw) {
	if (!kw)
		return_reference_(Dee_EmptyRoDict);
	if (DeeKwds_Check(kw)) {
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > *p_argc) {
			/* Argument list is too short of the given keywords */
			err_keywords_bad_for_argc((DeeKwdsObject *)kw, *p_argc, argv);
			return NULL;
		}
		*p_argc -= num_keywords;

		/* Turn keywords and arguments into a proper mapping-like object. */
		return DeeKwdsMapping_New(kw, argv + *p_argc);
	}

	/* `kw' already is a user-provided mapping. */
	return_reference_(kw);
}

PUBLIC ATTR_INS(2, 1) NONNULL((3)) void DCALL
DeeKwMapping_Decref(size_t argc, DeeObject *const *argv, DREF DeeObject *kw) {
	ASSERT_OBJECT(kw);
	if (DeeKwdsMapping_Check(kw) &&
	    DeeKwdsMapping_ARGV(kw) == argv + argc) {
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
			return err_keywords_bad_for_argc((DeeKwdsObject *)kw, *p_argc, argv);
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
 * @param: positional_argc: The value of `*p_argc' after `DeeKwArgs_Init()' returned.
 * @return: 0 : Success
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
PUBLIC WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeKwArgs_TryGetItemNR(DeeKwArgs *__restrict self,
                       /*string*/ DeeObject *__restrict name) {
	DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if (!self->kwa_kw)
		return ITER_DONE;
	if (DeeKwds_Check(self->kwa_kw)) {
		size_t kw_index = DeeKwds_IndexOf(self->kwa_kw, name);
		if unlikely(kw_index == (size_t)-1)
			return ITER_DONE;
		result = self->kwa_kwargv[kw_index];
		++self->kwa_kwused;
	} else {
		result = DeeKw_TryGetItemNR(self->kwa_kw, name);
		if likely(result != ITER_DONE)
			++self->kwa_kwused;
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeKwArgs_TryGetItemNRStringHash(DeeKwArgs *__restrict self,
                                 char const *__restrict name,
                                 Dee_hash_t hash) {
	DeeObject *result;
	if (!self->kwa_kw)
		return ITER_DONE;
	if (DeeKwds_Check(self->kwa_kw)) {
		size_t kw_index = DeeKwds_IndexOfStringHash(self->kwa_kw, name, hash);
		if unlikely(kw_index == (size_t)-1)
			return ITER_DONE;
		result = self->kwa_kwargv[kw_index];
		++self->kwa_kwused;
	} else {
		result = DeeKw_TryGetItemNRStringHash(self->kwa_kw, name, hash);
		if likely(result != ITER_DONE)
			++self->kwa_kwused;
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeKwArgs_TryGetItemNRStringLenHash(DeeKwArgs *__restrict self,
                                    char const *__restrict name,
                                    size_t namelen, Dee_hash_t hash) {
	DeeObject *result;
	if (!self->kwa_kw)
		return ITER_DONE;
	if (DeeKwds_Check(self->kwa_kw)) {
		size_t kw_index = DeeKwds_IndexOfStringLenHash(self->kwa_kw, name, namelen, hash);
		if unlikely(kw_index == (size_t)-1)
			return ITER_DONE;
		result = self->kwa_kwargv[kw_index];
		++self->kwa_kwused;
	} else {
		result = DeeKw_TryGetItemNRStringLenHash(self->kwa_kw, name, namelen, hash);
		if likely(result != ITER_DONE)
			++self->kwa_kwused;
	}
	return result;
}


/* In a keyword-enabled function, return the argument associated with a given
 * `name', or throw a TypeError exception or return `def' if not provided.
 *
 * Use these functions when you're uncertain if "kw" is non-NULL or might be
 * `DeeKwds_Check()'. If you're certain that `kw != NULL && !DeeKwds_Check(kw)',
 * you can also use the set of `DeeKw_TryGetItemNR*' functions below.
 *
 * IMPORTANT: These functions do *NOT* return references! */
PUBLIC WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL
DeeArg_TryGetKwNR(size_t argc, DeeObject *const *argv, DeeObject *kw,
                  /*string*/ DeeObject *__restrict name) {
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	if (!kw)
		return ITER_DONE;
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > argc)
			return ITER_DONE;
		kw_index = DeeKwds_IndexOf(kw, name);
		if unlikely(kw_index == (size_t)-1)
			return ITER_DONE;
		ASSERT(kw_index < num_keywords);
		return argv[(argc - num_keywords) + kw_index];
	}
	return DeeKw_TryGetItemNR(kw, name);
}

PUBLIC WUNUSED ATTR_INS(2, 1) NONNULL((4)) DREF DeeObject *DCALL
DeeArg_TryGetKwNRStringHash(size_t argc, DeeObject *const *argv,
                            DeeObject *kw, char const *__restrict name,
                            Dee_hash_t hash) {
	if (!kw)
		return ITER_DONE;
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > argc)
			return ITER_DONE;
		kw_index = DeeKwds_IndexOfStringHash(kw, name, hash);
		if unlikely(kw_index == (size_t)-1)
			return ITER_DONE;
		ASSERT(kw_index < num_keywords);
		return argv[(argc - num_keywords) + kw_index];
	}
	return DeeKw_TryGetItemNRStringHash(kw, name, hash);
}

PUBLIC WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL
DeeArg_TryGetKwNRStringLenHash(size_t argc, DeeObject *const *argv,
                               DeeObject *kw, char const *__restrict name,
                               size_t namelen, Dee_hash_t hash) {
	if (!kw)
		return ITER_DONE;
	if (DeeKwds_Check(kw)) {
		size_t kw_index;
		size_t num_keywords = DeeKwds_SIZE(kw);
		if unlikely(num_keywords > argc)
			return ITER_DONE;
		kw_index = DeeKwds_IndexOfStringLenHash(kw, name, namelen, hash);
		if unlikely(kw_index == (size_t)-1)
			return ITER_DONE;
		ASSERT(kw_index < num_keywords);
		return argv[(argc - num_keywords) + kw_index];
	}
	return DeeKw_TryGetItemNRStringLenHash(kw, name, namelen, hash);
}




/* Check if `kwds' fulfills `DeeObject_IsKw()', and if not, wrap it as a generic
 * kw-capable wrapper that calls forward to `DeeObject_GetItem(kwds)' when keywords
 * are queried, but then caches returned references such that the keyword consumer
 * doesn't need to keep track of them. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKw_Wrap(DeeObject *__restrict kwds) {
	if (DeeObject_IsKw(kwds))
		return_reference_(kwds);
	return DeeCachedDict_New(kwds);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKw_WrapInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict kwds) {
	if (DeeObject_IsKw(kwds))
		return kwds;
	return DeeCachedDict_NewInheritedOnSuccess(kwds);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKw_ForceWrap(DeeObject *__restrict kwds) {
	return DeeCachedDict_New(kwds);
}



/* Lookup keyword arguments. These functions may be used to extract keyword arguments
 * when the caller knows that `kw != NULL && DeeObject_IsKw(kw) && !DeeKwds_Check(kw)'.
 *
 * IMPORTANT: These functions do *NOT* return references! */
PUBLIC WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeKw_TryGetItemNR(DeeObject *kw, /*string*/ DeeObject *name) {
	DeeTypeObject *kw_type = Dee_TYPE(kw);
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	ASSERT(DeeType_IsKw(kw_type));
	ASSERT(kw_type->tp_seq);
	ASSERT(kw_type->tp_seq->tp_trygetitemnr);
	return (*kw_type->tp_seq->tp_trygetitemnr)(kw, name);
}

PUBLIC WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeKw_TryGetItemNRStringHash(DeeObject *kw, char const *__restrict name, Dee_hash_t hash) {
	DeeTypeObject *kw_type = Dee_TYPE(kw);
	ASSERT(DeeType_IsKw(kw_type));
	ASSERT(kw_type->tp_seq);
	ASSERT(kw_type->tp_seq->tp_trygetitemnr_string_hash);
	return (*kw_type->tp_seq->tp_trygetitemnr_string_hash)(kw, name, hash);
}

PUBLIC WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeKw_TryGetItemNRStringLenHash(DeeObject *kw, char const *__restrict name,
                                size_t namelen, Dee_hash_t hash) {
	DeeTypeObject *kw_type = Dee_TYPE(kw);
	ASSERT(DeeType_IsKw(kw_type));
	ASSERT(kw_type->tp_seq);
	ASSERT(kw_type->tp_seq->tp_trygetitemnr_string_len_hash);
	return (*kw_type->tp_seq->tp_trygetitemnr_string_len_hash)(kw, name, namelen, hash);
}




/* Construct the canonical wrapper for "**kwds" in a user-defined function,
 * such that the names of positional arguments passed via "kw" are black-listed.
 *
 * The returned object is always kw-capable, and this function is used in code
 * such as this:
 * >> function foo(a, b, **kwds) {
 * >>     return kwds;
 * >> }
 * >> print repr foo(**{ "a": 10, "b": 20, "c": 30 }); // prints '{ "c": 30 }' (because "a" and "b" are black-listed)
 *
 * This function is the high-level wrapper around:
 * - DeeBlackListKwds_New
 * - DeeBlackListKw_New
 * - DeeKwdsMapping_New
 *
 * IMPORTANT: The returned object must be decref'd using `DeeKwBlackList_Decref()'
 *            once the function that created it returns. */
PUBLIC WUNUSED NONNULL((1, 3)) ATTR_INS(4, 2) DREF DeeObject *DCALL
DeeKwBlackList_New(struct Dee_code_object *__restrict code,
                   size_t positional_argc,
                   DeeObject *const *positional_argv,
                   DeeObject *__restrict kw) {
	ASSERT_OBJECT(kw);
	ASSERT(DeeObject_IsKw(kw));
	if likely(DeeKwds_Check(kw)) {
		/* Most common case: Must create a wrapper around the kwds/argv hybrid descriptor,
		 *                   but exclude any keyword also found as part of our code's
		 *                   keyword list.
		 * >> function foo(x, y, **kw) {
		 * >> 	print repr kw;
		 * >> }
		 * >> foo(x: 10, y: 20, z: 30); // { "z": 30 }
		 * Semantically comparable to:
		 * >> return Mapping.frozen(
		 * >> 	for (local key, id: kw)
		 * >> 		if (key !in __code__.__kwds__)
		 * >> 			(key, positional_argv[positional_argc + id])
		 * >> );
		 */
		if unlikely(!DeeKwds_SIZE(kw)) {
			/* No keywords --> Return an empty mapping.
			 * -> This can happen depending on how keyword arguments
			 *    have been routed throughout the runtime.
			 * Note that we use "Dee_EmptyRoDict" because the mapping needs to be kw-capable */
			return DeeRoDict_NewEmpty();
		}
		positional_argv += positional_argc;
		if (positional_argc >= code->co_argc_max || unlikely(!code->co_keywords)) {
			/* No keyword information --> Return an unfiltered keywords mapping object.
			 * -> This happens for purely varkwds user-code functions, such a function
			 *    written as `function foo(**kw)', in which case there aren't any other
			 *    keyword which would have to be blacklisted when access is made. */
			return DeeKwdsMapping_New((DeeObject *)kw, positional_argv);
		}
		return DeeBlackListKwds_New(code, positional_argc,
		                            positional_argv, (DeeKwdsObject *)kw);
	}

	/* General case: create a proxy-mapping object for `kw' that get rids
	 *               of all keys that are equal to one of the strings found
	 *               within our keyword list.
	 * Semantically comparable to:
	 * >> return Mapping.frozen(
	 * >> 	for (local key, item: kw)
	 * >> 		if (key !in __code__.__kwds__[positional_argc:])
	 * >> 			(key, item)
	 * >> );
	 */
	if (positional_argc >= code->co_argc_max || unlikely(!code->co_keywords)) {
		/* No keyword information --> Re-return the unfiltered input mapping object. */
		return_reference_(kw);
	}
	return DeeBlackListKw_New(code, positional_argc, kw);
}

/* Unshare pointers to "argv" if "self" is still shared. */
PUBLIC NONNULL((1)) void DCALL
DeeKwBlackList_Decref(DREF DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (tp_self == &DeeKwdsMapping_Type) {
		DeeKwdsMapping_Decref(self);
	} else if (tp_self == &DeeBlackListKwds_Type) {
		DeeBlackListKwds_Decref(self);
	} else {
		Dee_Decref_likely(self);
	}
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_KWDS_C */
