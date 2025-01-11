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
#ifndef GUARD_DEEMON_OBJECTS_DICT_C
#define GUARD_DEEMON_OBJECTS_DICT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/align.h>
#include <hybrid/bit.h>
#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/default-compare.h"
#include "seq/default-api.h"
#include "seq/hashfilter.h"

#ifndef SIZE_MAX
#include <hybrid/limitcore.h>
#ifndef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#endif /* !SIZE_MAX */
#endif /* !SIZE_MAX */

#undef shift_t
#define shift_t __SHIFT_TYPE__
#undef byte_t
#define byte_t __BYTE_TYPE__

#ifdef __OPTIMIZE_SIZE__
#define NULL_IF_Os(v) NULL
#else /* __OPTIMIZE_SIZE__ */
#define NULL_IF_Os(v) v
#endif /* !__OPTIMIZE_SIZE__ */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

typedef DeeDictObject Dict;

#ifdef CONFIG_EXPERIMENTAL_ORDERED_DICTS

/************************************************************************/
/* ITERATOR                                                             */
/************************************************************************/
typedef struct {
	PROXY_OBJECT_HEAD_EX(Dict, di_dict) /* [1..1][const] The Dict being iterated. */
	/*virt*/ Dee_dict_vidx_t   di_vidx; /* [!0][lock(ATOMIC)] Lower bound for next item to read from "d_vtab" (starts at 1 because "d_vtab" is offset by -1) */
} DictIterator;

INTDEF DeeTypeObject DictIterator_Type;

STATIC_ASSERT(offsetof(DictIterator, di_dict) == offsetof(ProxyObject, po_obj));
#define diter_fini  generic_proxy_fini
#define diter_visit generic_proxy_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
diter_ctor(DictIterator *__restrict self) {
	self->di_dict = (DREF Dict *)DeeDict_New();
	if unlikely(!self->di_dict)
		goto err;
	self->di_vidx = Dee_dict_vidx_tovirt(0);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
diter_copy(DictIterator *__restrict self,
           DictIterator *__restrict other) {
	self->di_dict = other->di_dict;
	Dee_Incref(self->di_dict);
	self->di_vidx = atomic_read(&other->di_vidx);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
diter_deep(DictIterator *__restrict self,
           DictIterator *__restrict other) {
	self->di_dict = (DREF Dict *)DeeObject_DeepCopy((DeeObject *)other->di_dict);
	if unlikely(!self->di_dict)
		goto err;
	self->di_vidx = atomic_read(&other->di_vidx);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
diter_init(DictIterator *__restrict self,
           size_t argc, DeeObject *const *argv) {
	Dict *dict;
	if (DeeArg_Unpack(argc, argv, "o:_DictIterator", &dict))
		goto err;
	if (DeeObject_AssertType(dict, &DeeDict_Type))
		goto err;
	self->di_dict = dict;
	Dee_Incref(dict);
	self->di_vidx = Dee_dict_vidx_tovirt(0);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diter_nextkey(DictIterator *__restrict self) {
	DREF DeeObject *result;
	Dict *dict = self->di_dict;
	for (;;) {
		/*virt*/ Dee_dict_vidx_t old_vidx = atomic_read(&self->di_vidx);
		/*virt*/ Dee_dict_vidx_t new_vidx = old_vidx;
		DeeDict_LockRead(dict);
		do {
			if unlikely(new_vidx > dict->d_vsize) {
				DeeDict_LockEndRead(dict);
				return ITER_DONE;
			}
			result = _DeeDict_GetVirtVTab(dict)[new_vidx].di_key;
			++new_vidx;
		} while (!result);
		Dee_Incref(result);
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
		Dee_Decref_unlikely(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diter_nextvalue(DictIterator *__restrict self) {
	DREF DeeObject *result;
	Dict *dict = self->di_dict;
	for (;;) {
		DeeObject *key;
		/*virt*/ Dee_dict_vidx_t old_vidx = atomic_read(&self->di_vidx);
		/*virt*/ Dee_dict_vidx_t new_vidx = old_vidx;
		struct Dee_dict_item *virt_vtab;
		DeeDict_LockRead(dict);
		virt_vtab = _DeeDict_GetVirtVTab(dict);
		do {
			if unlikely(new_vidx > dict->d_vsize) {
				DeeDict_LockEndRead(dict);
				return ITER_DONE;
			}
			key    = virt_vtab[new_vidx].di_key;
			result = virt_vtab[new_vidx].di_value;
			++new_vidx;
		} while (!key);
		Dee_Incref(result);
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
		Dee_Decref_unlikely(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
diter_nextpair(DictIterator *__restrict self,
               DREF DeeObject *key_and_value[2]) {
	Dict *dict = self->di_dict;
	for (;;) {
		/*virt*/ Dee_dict_vidx_t old_vidx = atomic_read(&self->di_vidx);
		/*virt*/ Dee_dict_vidx_t new_vidx = old_vidx;
		struct Dee_dict_item *virt_vtab;
		DeeDict_LockRead(dict);
		virt_vtab = _DeeDict_GetVirtVTab(dict);
		do {
			if unlikely(new_vidx > dict->d_vsize) {
				DeeDict_LockEndRead(dict);
				return 1;
			}
			key_and_value[0] = virt_vtab[new_vidx].di_key;
			key_and_value[1] = virt_vtab[new_vidx].di_value;
			++new_vidx;
		} while (!key_and_value[0]);
		Dee_Incref(key_and_value[0]);
		Dee_Incref(key_and_value[1]);
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
		Dee_Decref_unlikely(key_and_value[1]);
		Dee_Decref_unlikely(key_and_value[0]);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
diter_advance(DictIterator *__restrict self, size_t skip) {
	/*virt*/ Dee_dict_vidx_t old_vidx, new_vidx;
	Dict *dict = self->di_dict;
	for (;;) {
		old_vidx = atomic_read(&self->di_vidx);
		new_vidx = old_vidx;
		DeeDict_LockRead(dict);
		ASSERT(dict->d_vused <= dict->d_vsize);
		if (dict->d_vused == dict->d_vsize) {
			/* Dict is optimized -> can just increment "new_vidx" */
			if (OVERFLOW_UADD(new_vidx, skip, &new_vidx))
				new_vidx = (size_t)-1;
			if (new_vidx > Dee_dict_vidx_tovirt(dict->d_vsize))
				new_vidx = Dee_dict_vidx_tovirt(dict->d_vsize);
		} else {
			/* Only keys that haven't been deleted count. */
			size_t n_skip = skip;
			struct Dee_dict_item *virt_vtab = _DeeDict_GetVirtVTab(dict);
			for (; Dee_dict_vidx_virt_lt_real(new_vidx, dict->d_vsize) && n_skip; ++new_vidx) {
				if (virt_vtab[new_vidx].di_key)
					--n_skip;
			}
		}
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
	}
	return new_vidx - old_vidx;
}

PRIVATE struct type_iterator diter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&diter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diter_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&diter_advance,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
diter_bool(DictIterator *__restrict self) {
	Dict *dict = self->di_dict;
	/*virt*/Dee_dict_vidx_t vidx = atomic_read(&self->di_vidx);
	struct Dee_dict_item *virt_vtab;
	ASSERT(vidx >= Dee_dict_vidx_tovirt(0));
	DeeDict_LockRead(dict);
	virt_vtab = _DeeDict_GetVirtVTab(dict);
	for (; Dee_dict_vidx_virt_lt_real(vidx, dict->d_vsize); ++vidx) {
		if (virt_vtab[vidx].di_key) {
			DeeDict_LockEndRead(dict);
			return 1;
		}
	}
	DeeDict_LockEndRead(dict);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
diter_hash(DictIterator *self) {
	return atomic_read(&self->di_vidx);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
diter_compare(DictIterator *lhs, DictIterator *rhs) {
	if (DeeObject_AssertType(rhs, &DictIterator_Type))
		goto err;
	Dee_return_compareT(size_t, atomic_read(&lhs->di_vidx),
	                    /*   */ atomic_read(&rhs->di_vidx));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp diter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&diter_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&diter_compare,
};


PRIVATE struct type_member tpconst dict_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DictIterator, di_dict), "->?DDict"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(dict:?DDict)\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&diter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&diter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&diter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&diter_init,
				TYPE_FIXED_ALLOCATOR(DictIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&diter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&diter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&diter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &diter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &diter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dict_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};















/************************************************************************/
/* DICT                                                                 */
/************************************************************************/

PUBLIC DeeObject DeeDict_Dummy = { /* DEPRECATED */
	OBJECT_HEAD_INIT(&DeeObject_Type)
};

PUBLIC_CONST byte_t const _DeeDict_EmptyTab[] = { 0 };

/* Heap functions for allocating/freeing dict tables (d_vtab + d_htab) */
#define _DeeDict_TabsMalloc(n_bytes)          Dee_Malloc(n_bytes)
#define _DeeDict_TabsCalloc(n_bytes)          Dee_Calloc(n_bytes)
#define _DeeDict_TabsRealloc(ptr, n_bytes)    Dee_Realloc(ptr, n_bytes)
#define _DeeDict_TabsTryMalloc(n_bytes)       Dee_TryMalloc(n_bytes)
#define _DeeDict_TabsTryCalloc(n_bytes)       Dee_TryCalloc(n_bytes)
#define _DeeDict_TabsTryRealloc(ptr, n_bytes) Dee_TryRealloc(ptr, n_bytes)
#define _DeeDict_TabsFree(ptr)                Dee_Free(ptr)


DFUNDEF WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx8(void *__restrict htab, size_t index);
DFUNDEF NONNULL((1)) void DCALL Dee_dict_sethidx8(void *__restrict htab, size_t index, size_t value);
PRIVATE NONNULL((1)) void DCALL Dee_dict_movhidx8(void *__restrict dst, void const *__restrict src, size_t n_words);

#if DEE_DICT_HIDXIO_COUNT >= 2
PRIVATE WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx16(void *__restrict htab, size_t index);
PRIVATE NONNULL((1)) void DCALL Dee_dict_sethidx16(void *__restrict htab, size_t index, size_t value);
PRIVATE NONNULL((1)) void DCALL Dee_dict_movhidx16(void *__restrict dst, void const *__restrict src, size_t n_words);
#define Dee_dict_uprhidx8_PTR &Dee_dict_uprhidx8
PRIVATE NONNULL((1)) void DCALL Dee_dict_uprhidx8(void *__restrict dst, void const *__restrict src, size_t n_words);
PRIVATE NONNULL((1)) void DCALL Dee_dict_dwnhidx16(void *__restrict dst, void const *__restrict src, size_t n_words);
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */

#if DEE_DICT_HIDXIO_COUNT >= 3
PRIVATE WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx32(void *__restrict htab, size_t index);
PRIVATE NONNULL((1)) void DCALL Dee_dict_sethidx32(void *__restrict htab, size_t index, size_t value);
PRIVATE NONNULL((1)) void DCALL Dee_dict_movhidx32(void *__restrict dst, void const *__restrict src, size_t n_words);
#define Dee_dict_uprhidx16_PTR &Dee_dict_uprhidx16
PRIVATE NONNULL((1)) void DCALL Dee_dict_uprhidx16(void *__restrict dst, void const *__restrict src, size_t n_words);
PRIVATE NONNULL((1)) void DCALL Dee_dict_dwnhidx32(void *__restrict dst, void const *__restrict src, size_t n_words);
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */

#if DEE_DICT_HIDXIO_COUNT >= 4
PRIVATE WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx64(void *__restrict htab, size_t index);
PRIVATE NONNULL((1)) void DCALL Dee_dict_sethidx64(void *__restrict htab, size_t index, size_t value);
PRIVATE NONNULL((1)) void DCALL Dee_dict_movhidx64(void *__restrict dst, void const *__restrict src, size_t n_words);
#define Dee_dict_uprhidx32_PTR &Dee_dict_uprhidx32
PRIVATE NONNULL((1)) void DCALL Dee_dict_uprhidx32(void *__restrict dst, void const *__restrict src, size_t n_words);
PRIVATE NONNULL((1)) void DCALL Dee_dict_dwnhidx64(void *__restrict dst, void const *__restrict src, size_t n_words);
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */

#ifndef Dee_dict_uprhidx8_PTR
#define Dee_dict_uprhidx8_PTR NULL
#endif /* !Dee_dict_uprhidx8_PTR */
#ifndef Dee_dict_uprhidx16_PTR
#define Dee_dict_uprhidx16_PTR NULL
#endif /* !Dee_dict_uprhidx16_PTR */
#ifndef Dee_dict_uprhidx32_PTR
#define Dee_dict_uprhidx32_PTR NULL
#endif /* !Dee_dict_uprhidx32_PTR */


#ifndef __INTELLISENSE__
DECL_END
#define LOCAL_HIDXIO_NBITS 8
#include "dict-hidxio.c.inl"

#if DEE_DICT_HIDXIO_COUNT >= 2
#define LOCAL_HIDXIO_NBITS 16
#include "dict-hidxio.c.inl"
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */

#if DEE_DICT_HIDXIO_COUNT >= 3
#define LOCAL_HIDXIO_NBITS 32
#include "dict-hidxio.c.inl"
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */

#if DEE_DICT_HIDXIO_COUNT >= 4
#define LOCAL_HIDXIO_NBITS 64
#include "dict-hidxio.c.inl"
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

PUBLIC_TPCONST struct Dee_dict_hidxio_struct tpconst Dee_dict_hidxio[DEE_DICT_HIDXIO_COUNT] = {
	/* [0] = */ { &Dee_dict_gethidx8, &Dee_dict_sethidx8, &Dee_dict_movhidx8, Dee_dict_uprhidx8_PTR, NULL },
#if DEE_DICT_HIDXIO_COUNT >= 2
	/* [1] = */ { &Dee_dict_gethidx16, &Dee_dict_sethidx16, &Dee_dict_movhidx16, Dee_dict_uprhidx16_PTR, &Dee_dict_dwnhidx16 },
#if DEE_DICT_HIDXIO_COUNT >= 3
	/* [2] = */ { &Dee_dict_gethidx32, &Dee_dict_sethidx32, &Dee_dict_movhidx32, Dee_dict_uprhidx32_PTR, &Dee_dict_dwnhidx32 },
#if DEE_DICT_HIDXIO_COUNT >= 4
	/* [3] = */ { &Dee_dict_gethidx64, &Dee_dict_sethidx64, &Dee_dict_movhidx64, NULL, &Dee_dict_dwnhidx64 },
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */
};



/* This right here encodes our target ratio of used vs. unused hash indices.
 * Currently, we aim for a ratio of:
 * >> d_valloc * 2   ~=   d_hmask * 3 */
#define DICT_VTAB_HTAB_RATIO_V 2
#define DICT_VTAB_HTAB_RATIO_H 3

STATIC_ASSERT(DICT_VTAB_HTAB_RATIO_H > DICT_VTAB_HTAB_RATIO_V);

/* Returns true if "d_htab" *should* be grown to its next larger multiple */
#define _DeeDict_ShouldGrowHTab(self)                                                       \
	((dict_suggesed_max_valloc_from_count((self)->d_vused + 1) * DICT_VTAB_HTAB_RATIO_V) >= \
	 ((self)->d_hmask * DICT_VTAB_HTAB_RATIO_H))

/* Returns true if "d_htab" *MUST* be grown to its next larger
 * multiple (before a new item can be added to the dict) */
#define _DeeDict_MustGrowHTab(self) \
	((self)->d_valloc >= (self)->d_hmask)


/* Evaluates to true if the dict's "d_vtab" should be optimized.
 * s.a. `dict_optimize_vtab()' */
#define _DeeDict_ShouldOptimizeVTab(self) \
	(((self)->d_vsize >> 1) > (self)->d_vused)
#define _DeeDict_CanOptimizeVTab(self) \
	((self)->d_vsize > (self)->d_vused)

#define _DeeDict_MustGrowVTab(self) \
	((self)->d_vsize >= (self)->d_valloc)
#define _DeeDict_CanGrowVTab(self) \
	((self)->d_valloc < (self)->d_hmask)


/* Returns true if the vtab should shrink following an item being deleted. */
#define _DeeDict_ShouldShrinkVTab(self) \
	(_DeeDict_ShouldShrinkVTab_NEWALLOC(self) < (self)->d_valloc)
#define _DeeDict_ShouldShrinkVTab_NEWALLOC(self) \
	((self)->d_vused << 1) /* Shrink the vtab when more than half of it is unused */

/* Returns true if the vtab can be shrunk. */
#define _DeeDict_CanShrinkVTab(self) \
	((self)->d_vused < (self)->d_valloc)

/* Returns true if the htab should shrink following the vtab having been shrunk */
#define _DeeDict_ShouldShrinkHTab(self) \
	_DeeDict_ShouldShrinkHTab2((self)->d_valloc, (self)->d_hmask)
#define _DeeDict_ShouldShrinkHTab2(valloc, hmask) \
	(((valloc)*DICT_VTAB_HTAB_RATIO_H) <=         \
	 (((hmask) >> 1) * DICT_VTAB_HTAB_RATIO_V) && (hmask))

/* Returns true if the htab can be shrunk. */
#define _DeeDict_CanShrinkHTab(self) \
	_DeeDict_CanShrinkHTab2((self)->d_valloc, (self)->d_hmask)
#define _DeeDict_CanShrinkHTab2(valloc, hmask) \
	((valloc) <= ((hmask) >> 1) && (hmask))


#define NULL_IF__DeeDict_EmptyTab(/*real*/ p) \
	((p) == (struct Dee_dict_item *)_DeeDict_EmptyTab ? NULL : (p))

/* Default hint in `DeeDict_FromSequence' when the sequence doesn't provide one itself. */
#ifndef DICT_FROMSEQ_DEFAULT_HINT
#define DICT_FROMSEQ_DEFAULT_HINT 64
#endif /* !DICT_FROMSEQ_DEFAULT_HINT */


#undef DICT_NDEBUG
#if defined(NDEBUG) || 0
#define DICT_NDEBUG
#endif /* NDEBUG */


#define HAVE_dict_setitem_unlocked
#ifdef __OPTIMIZE_SIZE__
#undef HAVE_dict_setitem_unlocked
#endif /* __OPTIMIZE_SIZE__ */


LOCAL ATTR_CONST size_t DCALL
dict_suggesed_max_valloc_from_count(size_t num_item) {
	shift_t items_nbits;
	size_t result;
	ASSERT(num_item > 0);
	items_nbits = CLZ(num_item);
	if unlikely(!items_nbits)
		return (size_t)-1;
	items_nbits = (sizeof(size_t) * __CHAR_BIT__) - items_nbits;
	result = ((size_t)1 << (items_nbits)) |
	         ((size_t)1 << (items_nbits + 1));
	ASSERT(result >= num_item);
	return result;
}

/* Calculate what would be a good "d_valloc" for a given "d_hmask" and "d_vsize" */
LOCAL ATTR_CONST size_t DCALL
dict_valloc_from_hmask_and_count(size_t hmask, size_t num_item, bool allow_overalloc) {
	size_t result, max_valloc;
	ASSERT(num_item <= hmask);
	if (!allow_overalloc)
		return num_item;
	result = (hmask / DICT_VTAB_HTAB_RATIO_H) * DICT_VTAB_HTAB_RATIO_V;
	ASSERT(result <= hmask);
	if unlikely(result < num_item)
		return num_item;
	if unlikely(!num_item)
		return 0;
	max_valloc = dict_suggesed_max_valloc_from_count(num_item);
	if (result > max_valloc)
		result = max_valloc;
	ASSERT(result >= num_item);
	ASSERT(result <= hmask);
	return result;
}


LOCAL ATTR_CONST size_t DCALL
dict_hmask_from_count(size_t num_item) {
	size_t result;
	shift_t mask_nbits;
	if unlikely(!num_item)
		return 0;
	if (OVERFLOW_UADD(num_item, DICT_VTAB_HTAB_RATIO_H - 1, &result))
		goto fallback;
	if (OVERFLOW_UMUL(result, DICT_VTAB_HTAB_RATIO_H, &result))
		goto fallback;
#if DICT_VTAB_HTAB_RATIO_V == 2
	result >>= 1;
#else /* DICT_VTAB_HTAB_RATIO_V == 2 */
	result /= DICT_VTAB_HTAB_RATIO_V;
#endif /* DICT_VTAB_HTAB_RATIO_V != 2 */
	ASSERT(result);
	mask_nbits = CLZ(result);
	mask_nbits = (sizeof(size_t) * __CHAR_BIT__) - mask_nbits;
	result = 1;
	result <<= mask_nbits;
	--result;
	ASSERT(result >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, true) >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, false) >= num_item);
	return result;
fallback:
	return SIZE_MAX;
}

LOCAL ATTR_CONST size_t DCALL
dict_tiny_hmask_from_count(size_t num_item) {
	size_t result;
	shift_t mask_nbits;
	if unlikely(!num_item)
		return 0;
	mask_nbits = CLZ(num_item);
	mask_nbits = (sizeof(size_t) * __CHAR_BIT__) - mask_nbits;
	result = 1;
	result <<= mask_nbits;
	--result;
	ASSERT(result >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, true) >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, false) >= num_item);
	return result;
}

#define dict_sizeoftabs_from_hmask_and_valloc(hmask, valloc) \
	dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(hmask, valloc, DEE_DICT_HIDXIO_FROMALLOC(valloc))
LOCAL ATTR_CONST size_t DCALL
dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(size_t hmask, size_t valloc, shift_t hidxio) {
	size_t result, hmask_size;
	if (OVERFLOW_UMUL(valloc, sizeof(struct Dee_dict_item), &result))
		goto toobig;
	if (OVERFLOW_UADD(hmask, 1, &hmask_size))
		goto toobig;
	if unlikely(((hmask_size << hidxio) >> hidxio) != hmask_size)
		goto toobig;
	hmask_size <<= hidxio;
	if (OVERFLOW_UADD(result, hmask_size, &result))
		goto toobig;
	return result;
toobig:
	return (size_t)-1; /* Force down-stream allocation failure */
}


#ifdef DICT_NDEBUG
#define dict_verify(self) (void)0
#else /* DICT_NDEBUG */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_verify(Dict *__restrict self) {
	size_t i, real_vused;
	shift_t hidxio;
	ASSERT(self->d_vused <= self->d_vsize);
	ASSERT(self->d_vsize <= self->d_valloc);
	ASSERT(self->d_valloc <= self->d_hmask);
	ASSERT(IS_POWER_OF_TWO(self->d_hmask + 1));
	ASSERT(self->d_htab == _DeeDict_GetRealVTab(self) + self->d_valloc);
	hidxio = DEE_DICT_HIDXIO_FROMALLOC(self->d_valloc);
	ASSERT(/*hidxio >= 0 &&*/ hidxio < DEE_DICT_HIDXIO_COUNT);
	ASSERT(self->d_hidxget == Dee_dict_hidxio[hidxio].dhxio_get);
	ASSERT(self->d_hidxset == Dee_dict_hidxio[hidxio].dhxio_set);
	for (i = Dee_dict_vidx_tovirt(0), real_vused = 0;
	     Dee_dict_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		struct Dee_dict_item *item = &_DeeDict_GetVirtVTab(self)[i];
		if (item->di_key) {
			ASSERT_OBJECT(item->di_key);
			ASSERT_OBJECT(item->di_value);
			++real_vused;
		}
	}
	ASSERTF(real_vused == self->d_vused,
	        "vtab key count=%" PRFuSIZ " differs from d_vused=%" PRFuSIZ,
	        real_vused, self->d_vused);
	for (i = 0; i <= self->d_hmask; ++i) {
		Dee_dict_vidx_t vidx;
		vidx = (*self->d_hidxget)(self->d_htab, i);
		if (vidx == Dee_DICT_HTAB_EOF)
			continue;
		Dee_dict_vidx_virt2real(&vidx);
		ASSERTF(vidx < self->d_vsize,
		        "htab[%" PRFuSIZ "] points out-of-bounds: %" PRFuSIZ " >= %" PRFuSIZ,
		        i, vidx, self->d_vsize);
	}
	for (i = 0;; ++i) {
		Dee_dict_vidx_t vidx;
		ASSERTF(i <= self->d_hmask, "htab contains no EOF pointers (infinite loop would occur on non-present item lookup)");
		vidx = (*self->d_hidxget)(self->d_htab, i);
		if (vidx == Dee_DICT_HTAB_EOF)
			break;
	}
	for (i = Dee_dict_vidx_tovirt(0), real_vused = 0;
	     Dee_dict_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		Dee_hash_t hs, perturb;
		struct Dee_dict_item *item = &_DeeDict_GetVirtVTab(self)[i];
		if (!item->di_key)
			continue;
		_DeeDict_HashIdxInit(self, &hs, &perturb, item->di_hash);
		for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
			Dee_dict_vidx_t vidx;
			struct Dee_dict_item *hitem;
			vidx = _DeeDict_HTabGet(self, hs);
			ASSERTF(vidx != Dee_DICT_HTAB_EOF,
			        "End-of-hash-chain[hash:%#" PRFxSIZ "] before item idx=%" PRFuSIZ ",count=%" PRFuSIZ " <%r:%r> was found",
			        item->di_hash, Dee_dict_vidx_toreal(i), self->d_vsize,
			        item->di_key, item->di_value);
			hitem = &_DeeDict_GetVirtVTab(self)[vidx];
			if (hitem == item)
				break;
		}
	}
}

#if 1
#undef DeeDict_LockEndWrite
#undef DeeDict_LockEndRead
#undef DeeDict_LockEnd
#define DeeDict_LockEndWrite(self)   (dict_verify(self), Dee_atomic_rwlock_endwrite(&(self)->d_lock))
#define DeeDict_LockEndRead(self)    (dict_verify(self), Dee_atomic_rwlock_endread(&(self)->d_lock))
#define DeeDict_LockEnd(self)        (dict_verify(self), Dee_atomic_rwlock_end(&(self)->d_lock))
#endif
#endif /* !DICT_NDEBUG */






LOCAL WUNUSED DREF DeeObject *DCALL
dict_new_with_hint(size_t num_items, bool tryalloc, bool allow_overalloc) {
	DREF Dict *result;
	size_t hmask, valloc;
	shift_t hidxio;
	void *tabs;
	if unlikely(!num_items) {
		hmask  = 0;
		valloc = 0;
		hidxio = 0;
		tabs   = (void *)_DeeDict_EmptyTab;
	} else {
		size_t tabssz;
		hmask  = dict_hmask_from_count(num_items);
		valloc = dict_valloc_from_hmask_and_count(hmask, num_items, allow_overalloc);
		hidxio = DEE_DICT_HIDXIO_FROMALLOC(valloc);
		tabssz = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(hmask, valloc, hidxio);
		tabs   = tryalloc ? _DeeDict_TabsTryCalloc(tabssz)
		                  : _DeeDict_TabsCalloc(tabssz);
		if unlikely(!tabs)
			goto err;
	}
	result = tryalloc ? DeeGCObject_TRYMALLOC(Dict)
	                  : DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err_tabs;
	result->d_valloc  = valloc;
	result->d_vsize   = 0;
	result->d_vused   = 0;
	_DeeDict_SetRealVTab(result, (struct Dee_dict_item *)tabs);
	result->d_hmask   = hmask;
	result->d_hidxget = Dee_dict_hidxio[hidxio].dhxio_get;
	result->d_hidxset = Dee_dict_hidxio[hidxio].dhxio_set;
	result->d_htab    = (struct Dee_dict_item *)tabs + valloc;
	Dee_atomic_rwlock_init(&result->d_lock);
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	result = DeeGC_TRACK(Dict, result);
	return (DREF DeeObject *)result;
err_tabs:
	if likely(num_items)
		_DeeDict_TabsFree(tabs);
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_TryNewWithHint(size_t num_items) {
	return dict_new_with_hint(num_items, true, false);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_TryNewWithWeakHint(size_t num_items) {
	return dict_new_with_hint(num_items, true, true);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewWithHint(size_t num_items) {
	return dict_new_with_hint(num_items, false, false);
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewWithWeakHint(size_t num_items) {
	return dict_new_with_hint(num_items, false, true);
}

LOCAL NONNULL((1)) void DCALL
dict_htab_insert8(Dict *__restrict self, Dee_hash_t hash, /*virt*/Dee_dict_vidx_t vidx) {
	Dee_hash_t hs, perturb;
	uint8_t *htab = (uint8_t *)self->d_htab;
	_DeeDict_HashIdxInit(self, &hs, &perturb, hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
		size_t htab_idx = hs & self->d_hmask;
		if likely(htab[htab_idx] == Dee_DICT_HTAB_EOF) {
			htab[htab_idx] = (uint8_t)vidx;
			break;
		}
	}
}

#if DEE_DICT_HIDXIO_COUNT >= 2
LOCAL NONNULL((1)) void DCALL
dict_htab_insert16(Dict *__restrict self, Dee_hash_t hash, /*virt*/Dee_dict_vidx_t vidx) {
	Dee_hash_t hs, perturb;
	uint16_t *htab = (uint16_t *)self->d_htab;
	_DeeDict_HashIdxInit(self, &hs, &perturb, hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
		size_t htab_idx = hs & self->d_hmask;
		if likely(htab[htab_idx] == Dee_DICT_HTAB_EOF) {
			htab[htab_idx] = (uint16_t)vidx;
			break;
		}
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */

#if DEE_DICT_HIDXIO_COUNT >= 3
LOCAL NONNULL((1)) void DCALL
dict_htab_insert32(Dict *__restrict self, Dee_hash_t hash, /*virt*/Dee_dict_vidx_t vidx) {
	Dee_hash_t hs, perturb;
	uint32_t *htab = (uint32_t *)self->d_htab;
	_DeeDict_HashIdxInit(self, &hs, &perturb, hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
		size_t htab_idx = hs & self->d_hmask;
		if likely(htab[htab_idx] == Dee_DICT_HTAB_EOF) {
			htab[htab_idx] = (uint32_t)vidx;
			break;
		}
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */

#if DEE_DICT_HIDXIO_COUNT >= 4
LOCAL NONNULL((1)) void DCALL
dict_htab_insert64(Dict *__restrict self, Dee_hash_t hash, /*virt*/Dee_dict_vidx_t vidx) {
	Dee_hash_t hs, perturb;
	uint64_t *htab = (uint64_t *)self->d_htab;
	_DeeDict_HashIdxInit(self, &hs, &perturb, hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
		size_t htab_idx = hs & self->d_hmask;
		if likely(htab[htab_idx] == Dee_DICT_HTAB_EOF) {
			htab[htab_idx] = (uint64_t)vidx;
			break;
		}
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */


/* Re-build the dict's "d_htab" (allowed to assume that "d_vtab" does not contain deleted keys) */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_htab_rebuild_after_optimize(Dict *__restrict self) {
	size_t i, vsize = self->d_vsize;
	struct Dee_dict_item *vtab = _DeeDict_GetVirtVTab(self);
	ASSERT(vsize == self->d_vused);
	if (DEE_DICT_HIDXIO_IS8(self->d_valloc)) {
		uint8_t *htab = (uint8_t *)self->d_htab;
		bzerob(htab, self->d_hmask + 1);
		for (i = Dee_dict_vidx_tovirt(0);
		     Dee_dict_vidx_virt_lt_real(i, vsize); ++i)
			dict_htab_insert8(self, vtab[i].di_hash, i);
	} else
#if DEE_DICT_HIDXIO_COUNT >= 2
	if (DEE_DICT_HIDXIO_IS16(self->d_valloc)) {
		uint16_t *htab = (uint16_t *)self->d_htab;
		bzerow(htab, self->d_hmask + 1);
		for (i = Dee_dict_vidx_tovirt(0);
		     Dee_dict_vidx_virt_lt_real(i, vsize); ++i)
			dict_htab_insert16(self, vtab[i].di_hash, i);
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */
#if DEE_DICT_HIDXIO_COUNT >= 3
	if (DEE_DICT_HIDXIO_IS32(self->d_valloc)) {
		uint32_t *htab = (uint32_t *)self->d_htab;
		bzerol(htab, self->d_hmask + 1);
		for (i = Dee_dict_vidx_tovirt(0);
		     Dee_dict_vidx_virt_lt_real(i, vsize); ++i)
			dict_htab_insert32(self, vtab[i].di_hash, i);
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */
#if DEE_DICT_HIDXIO_COUNT >= 4
	if (DEE_DICT_HIDXIO_IS64(self->d_valloc)) {
		uint64_t *htab = (uint64_t *)self->d_htab;
		bzeroq(htab, self->d_hmask + 1);
		for (i = Dee_dict_vidx_tovirt(0);
		     Dee_dict_vidx_virt_lt_real(i, vsize); ++i)
			dict_htab_insert64(self, vtab[i].di_hash, i);
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */
	{
		__builtin_unreachable();
	}
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_do_optimize_vtab_without_rebuild(Dict *__restrict self) {
	/*real*/ Dee_dict_vidx_t i;
	struct Dee_dict_item *vtab;
	vtab = _DeeDict_GetRealVTab(self);
	for (i = 0; i < self->d_vsize;) {
		/*real*/ Dee_dict_vidx_t j;
		size_t delta;
		if (vtab[i].di_key) {
			++i;
			continue;
		}
		delta = 1;
		for (j = i;;) {
			++j;
			if (j >= self->d_vsize)
				break;
			if (vtab[j].di_key)
				break;
			++delta;
		}
		memmovedownc(&vtab[i], &vtab[j], self->d_vsize - j,
		             sizeof(struct Dee_dict_item));
		self->d_vsize -= delta;
#ifndef __OPTIMIZE_SIZE__
		if (self->d_vsize <= self->d_vused)
			break; /* Fully optimized -> can stop early */
#endif /* !__OPTIMIZE_SIZE__ */
	}
	ASSERT(self->d_vsize == self->d_vused);
	ASSERT(self->d_vsize < self->d_valloc);
}

PRIVATE NONNULL((1)) void DCALL
dict_optimize_vtab(Dict *__restrict self) {
	ASSERT(_DeeDict_CanOptimizeVTab(self));
	dict_do_optimize_vtab_without_rebuild(self);
	dict_htab_rebuild_after_optimize(self);
}

PRIVATE NONNULL((1)) void DCALL
dict_htab_rebuild(Dict *__restrict self) {
	if (_DeeDict_CanOptimizeVTab(self))
		dict_do_optimize_vtab_without_rebuild(self);
	dict_htab_rebuild_after_optimize(self);
}



/* Try to make it so "d_vsize < d_valloc" by enlarging the vector.
 * Do this while the caller is holding a write-lock to "self", and
 * do so without ever releasing that lock.
 * NOTES:
 * - This function will NEVER rehash the dict or change the contents of d_htab!
 * - The caller must ensure that `_DeeDict_CanGrowVTab(self)' is true
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
dict_trygrow_vtab(Dict *__restrict self) {
	size_t new_valloc;
	size_t new_tabsize;
	shift_t old_hidxio;
	shift_t new_hidxio;
	struct Dee_dict_item *old_vtab;
	struct Dee_dict_item *new_vtab;
	void *old_htab;
	void *new_htab;

	/* Must truly allocate a new, larger v-table */
	ASSERT(_DeeDict_CanGrowVTab(self));
	new_valloc = dict_valloc_from_hmask_and_count(self->d_hmask, self->d_vsize + 1, true);
	old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(self->d_valloc);
	new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, new_hidxio);
	old_vtab = _DeeDict_GetRealVTab(self);
	old_vtab = NULL_IF__DeeDict_EmptyTab(old_vtab);
	new_vtab = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab) {
		new_valloc = dict_valloc_from_hmask_and_count(self->d_hmask, self->d_vsize + 1, false);
		old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(self->d_valloc);
		new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, new_hidxio);
		new_vtab = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
		if unlikely(!new_vtab)
			return false;
	}
	old_htab = new_vtab + self->d_valloc;
	new_htab = new_vtab + new_valloc;
	if likely(old_hidxio == new_hidxio) {
		(*Dee_dict_hidxio[old_hidxio].dhxio_movup)(new_htab, old_htab, self->d_hmask + 1);
	} else {
		(*Dee_dict_hidxio[old_hidxio].dhxio_upr)(new_htab, old_htab, self->d_hmask + 1);
		self->d_hidxget = Dee_dict_hidxio[old_hidxio].dhxio_get;
		self->d_hidxset = Dee_dict_hidxio[old_hidxio].dhxio_set;
	}
	bzeroc(new_vtab + self->d_valloc, new_valloc - self->d_valloc, sizeof(struct Dee_dict_item));
	_DeeDict_SetRealVTab(self, new_vtab);
	self->d_valloc = new_valloc;
	self->d_htab   = new_htab;
	return true;
}

/* Same as `dict_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!_DeeDict_CanGrowVTab(self)"
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
dict_trygrow_vtab_and_htab(Dict *__restrict self) {
	size_t old_hmask;
	size_t new_hmask;
#ifndef NDEBUG
	size_t old_valloc;
#endif /* !NDEBUG */
	size_t new_valloc;
	size_t new_tabsize;
	shift_t old_hidxio;
	shift_t new_hidxio;
	struct Dee_dict_item *old_vtab;
	struct Dee_dict_item *new_vtab;
	void *old_htab;
	void *new_htab;

	/* Must truly allocate a new, larger v-table */
	old_hmask = self->d_hmask;
	new_hmask = dict_hmask_from_count(self->d_vsize + 1);
	if unlikely(new_hmask < old_hmask)
		new_hmask = old_hmask;
	if (new_hmask <= old_hmask && _DeeDict_ShouldGrowHTab(self))
		new_hmask = (new_hmask << 1) | 1;
	new_valloc = dict_valloc_from_hmask_and_count(new_hmask, self->d_vsize + 1, true);
	old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(self->d_valloc);
	new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);
	old_vtab = _DeeDict_GetRealVTab(self);
	old_vtab = NULL_IF__DeeDict_EmptyTab(old_vtab);
	new_vtab = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab) {
		new_hmask  = dict_tiny_hmask_from_count(self->d_vsize + 1);
		new_valloc = dict_valloc_from_hmask_and_count(new_hmask, self->d_vsize + 1, false);
		old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(self->d_valloc);
		new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);
		new_vtab = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
		if unlikely(!new_vtab)
			return false;
	}
	old_htab = new_vtab + self->d_valloc;
	new_htab = new_vtab + new_valloc;
	_DeeDict_SetRealVTab(self, new_vtab);
#ifndef NDEBUG
	old_valloc = self->d_valloc;
#endif /* !NDEBUG */
	self->d_valloc = new_valloc;
	self->d_htab   = new_htab;
	if (old_hmask == new_hmask) {
		if likely(old_hidxio == new_hidxio) {
			(*Dee_dict_hidxio[old_hidxio].dhxio_movup)(new_htab, old_htab, old_hmask + 1);
		} else {
			(*Dee_dict_hidxio[old_hidxio].dhxio_upr)(new_htab, old_htab, old_hmask + 1);
			self->d_hidxget = Dee_dict_hidxio[new_hidxio].dhxio_get;
			self->d_hidxset = Dee_dict_hidxio[new_hidxio].dhxio_set;
		}
	} else {
		/* Must rebuild d_htab */
		self->d_hmask   = new_hmask;
		self->d_hidxget = Dee_dict_hidxio[new_hidxio].dhxio_get;
		self->d_hidxset = Dee_dict_hidxio[new_hidxio].dhxio_set;
		dict_htab_rebuild(self);
	}
#ifndef NDEBUG
	DBG_memset(new_vtab + old_valloc, 0xcc,
	           (new_valloc - old_valloc) *
	           sizeof(struct Dee_dict_item));
#endif /* !NDEBUG */
	return true;
}

#if 0
/* Try to change "d_hmask = (d_hmask << 1) | 1",
 * and (if we want to), also increase "d_valloc"
 * @return: true:  Success
 * @return: false: Failure (allocation failed) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
dict_trygrow_htab_and_maybe_vtab(Dict *__restrict self) {
	struct Dee_dict_item *old_vtab;
	struct Dee_dict_item *new_vtab;
	size_t new_hmask, new_tabsize;
	size_t old_valloc = self->d_valloc;
	size_t new_valloc;
	shift_t hidxio = DEE_DICT_HIDXIO_FROMALLOC(old_valloc);
	ASSERTF(self->d_hmask != (size_t)-1, "How? This should have been an OOM");
	new_hmask  = (self->d_hmask << 1) | 1;
	new_valloc = (new_hmask / DICT_VTAB_HTAB_RATIO_H) * DICT_VTAB_HTAB_RATIO_V;
	if (new_valloc < old_valloc)
		new_valloc = old_valloc;
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, hidxio);

	old_vtab = _DeeDict_GetRealVTab(self);
	old_vtab = NULL_IF__DeeDict_EmptyTab(old_vtab);
	new_vtab = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab) {
		/* Fine... We won't grow the vtab if you wanna be that way... */
		new_valloc  = old_valloc;
		new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, hidxio);
		new_vtab    = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
		if unlikely(!new_vtab)
			goto err;
	}
	self->d_valloc = new_valloc;
	self->d_hmask  = new_hmask;
	_DeeDict_SetRealVTab(self, new_vtab);
	self->d_htab = new_vtab + new_valloc;
	dict_htab_rebuild(self);
	return true;
err:
	return false;
}
#endif

/* Make it so "!_DeeDict_MustGrowVTab(self)"
 * (aka: " d_vsize < d_valloc && d_valloc <= d_hmask")
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
#ifdef HAVE_dict_setitem_unlocked
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_grow_vtab_and_htab_and_relock(Dict *__restrict self, bool without_locks)
#else /* HAVE_dict_setitem_unlocked */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_grow_vtab_and_htab_and_relock_impl(Dict *__restrict self)
#define dict_grow_vtab_and_htab_and_relock(self, without_locks) \
	dict_grow_vtab_and_htab_and_relock_impl(self)
#endif /* !HAVE_dict_setitem_unlocked */
{
#ifdef HAVE_dict_setitem_unlocked
#define IF_with_locks(x) if (!without_locks) x
#else /* HAVE_dict_setitem_unlocked */
#define IF_with_locks(x) x
#endif /* !HAVE_dict_setitem_unlocked */
	size_t new_valloc;
	size_t old_hmask;
	size_t new_hmask;
	size_t new_tabsize;
	shift_t new_hidxio;
	struct Dee_dict_item *old_vtab;
	struct Dee_dict_item *new_vtab;
again:
#ifdef HAVE_dict_setitem_unlocked
	ASSERT(without_locks || DeeDict_LockWriting(self));
#else /* HAVE_dict_setitem_unlocked */
	ASSERT(DeeDict_LockWriting(self));
#endif /* !HAVE_dict_setitem_unlocked */
	ASSERT(_DeeDict_MustGrowVTab(self));

	/* Figure out allocation sizes (never overallocate here; we only get here when memory is low!) */
	old_hmask = self->d_hmask;
	new_hmask = old_hmask;
	if (_DeeDict_MustGrowHTab(self)) {
		new_hmask = dict_tiny_hmask_from_count(self->d_vsize + 1);
		ASSERT(new_hmask > old_hmask);
	}
	new_valloc = dict_valloc_from_hmask_and_count(new_hmask, self->d_vsize + 1, false);
	new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);

	/* Release dict lock */
	IF_with_locks(DeeDict_LockEndWrite(self));

	/* Allocate new dict tables. */
	new_vtab = (struct Dee_dict_item *)_DeeDict_TabsMalloc(new_tabsize);
	if unlikely(!new_vtab)
		goto err;

	/* Re-acquire lock to the dict. */
	IF_with_locks(DeeDict_LockWrite(self));

	/* Check if the dict still needs the new buffer */
	if unlikely(!_DeeDict_MustGrowVTab(self)) {
free_buffer_and_try_again:
		IF_with_locks(DeeDict_LockEndWrite(self));
		_DeeDict_TabsFree(new_vtab);
		IF_with_locks(DeeDict_LockWrite(self));
		if likely(!_DeeDict_MustGrowVTab(self))
			return 0;
		goto again;
	}

	/* Check that the buffer we just allocated is actually large enough */
	if unlikely(new_valloc <= self->d_valloc)
		goto free_buffer_and_try_again;
	if unlikely(old_hmask != self->d_hmask)
		goto free_buffer_and_try_again;
	ASSERT(new_hidxio >= DEE_DICT_HIDXIO_FROMALLOC(self->d_valloc));

	/* Everything checks out: the buffer can be installed like this! */

	/* Copy over the contents of the old vtab */
	old_vtab = _DeeDict_GetRealVTab(self);
	if likely(old_hmask == new_hmask) {
		shift_t old_hidxio;
		void *new_htab = new_vtab + new_valloc;
		new_vtab = (struct Dee_dict_item *)memcpyc(new_vtab, old_vtab, self->d_vsize,
		                                           sizeof(struct Dee_dict_item));
		old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(self->d_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		if likely(old_hidxio == new_hidxio) {
			(*Dee_dict_hidxio[old_hidxio].dhxio_movup)(new_htab, self->d_htab, new_hmask + 1);
		} else {
			(*Dee_dict_hidxio[old_hidxio].dhxio_upr)(new_htab, self->d_htab, new_hmask + 1);
			self->d_hidxget = Dee_dict_hidxio[new_hidxio].dhxio_get;
			self->d_hidxset = Dee_dict_hidxio[new_hidxio].dhxio_set;
		}

		/* Update dict control elements to use the new tables. */
		self->d_valloc = new_valloc;
		_DeeDict_SetRealVTab(self, new_vtab);
		self->d_htab   = new_htab;
	} else {
		/* Must rebuild htab */
		if (_DeeDict_CanOptimizeVTab(self))
			dict_do_optimize_vtab_without_rebuild(self);
		new_vtab = (struct Dee_dict_item *)memcpyc(new_vtab, old_vtab, self->d_vsize,
		                                           sizeof(struct Dee_dict_item));
		self->d_hidxget = Dee_dict_hidxio[new_hidxio].dhxio_get;
		self->d_hidxset = Dee_dict_hidxio[new_hidxio].dhxio_set;
		self->d_valloc = new_valloc;
		_DeeDict_SetRealVTab(self, new_vtab);
		self->d_htab = new_vtab + new_valloc;
		dict_htab_rebuild_after_optimize(self);
	}

	/* Free the old tables. */
	IF_with_locks(DeeDict_LockEndWrite(self));
	_DeeDict_TabsFree(old_vtab);
	IF_with_locks(DeeDict_LockWrite(self));
	if unlikely(_DeeDict_MustGrowVTab(self))
		goto again;
	return 0;
err:
	return -1;
#undef IF_with_locks
}

#if 0
/* Make it so "!_DeeDict_MustGrowHTab(self)".
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_grow_htab_and_relock(Dict *__restrict self) {
	struct Dee_dict_item *old_vtab;
	struct Dee_dict_item *new_vtab;
	size_t new_hmask, new_tabsize;
	size_t valloc;
	shift_t hidxio;
again:
	ASSERT(DeeDict_LockWriting(self));
	ASSERT(_DeeDict_MustGrowHTab(self));
	ASSERTF(self->d_hmask != (size_t)-1, "How? This should have been an OOM");
	valloc = self->d_valloc;
	hidxio = DEE_DICT_HIDXIO_FROMALLOC(valloc);

	/* Figure out the size for the new table. */
	new_hmask   = (self->d_hmask << 1) | 1;
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, valloc, hidxio);
	DeeDict_LockEndWrite(self);

	/* Allocate the new table. */
	new_vtab = (struct Dee_dict_item *)_DeeDict_TabsMalloc(new_tabsize);
	if unlikely(!new_vtab)
		goto err;

	/* Re-acquire lock to the dict. */
	DeeDict_LockWrite(self);

	/* Verify that the dict still needs to have it's htab grow. */
	if unlikely(!_DeeDict_MustGrowHTab(self)) {
free_buffer_and_try_again:
		DeeDict_LockEndWrite(self);
		_DeeDict_TabsFree(new_vtab);
		DeeDict_LockWrite(self);
		if likely(!_DeeDict_MustGrowHTab(self))
			return 0;
		goto again;
	}
	if unlikely(new_hmask <= self->d_hmask)
		goto free_buffer_and_try_again;
	if unlikely(valloc <= self->d_valloc)
		goto free_buffer_and_try_again;

	/* Everything checks out: the buffer can be installed like this! */
	if (_DeeDict_CanOptimizeVTab(self))
		dict_do_optimize_vtab_without_rebuild(self); /* Optimize first so the memcpy needs to copy less stuff! */
	old_vtab = _DeeDict_GetRealVTab(self);
	new_vtab = (struct Dee_dict_item *)memcpyc(new_vtab, old_vtab, self->d_vsize,
	                                           sizeof(struct Dee_dict_item));
	self->d_hmask = new_hmask;
	_DeeDict_SetRealVTab(self, new_vtab);
	self->d_htab  = new_vtab + valloc;
	dict_htab_rebuild_after_optimize(self);

	/* Free the old tables. */
	DeeDict_LockEndWrite(self);
	_DeeDict_TabsFree(old_vtab);
	DeeDict_LockWrite(self);
	if unlikely(_DeeDict_MustGrowHTab(self))
		goto again;
	return 0;
err:
	return -1;
}
#endif

/* Shrink the vtab and release a lock to "self". Must be called when:
 * - holding a write-lock
 * - _DeeDict_CanShrinkHTab(self) is true
 * - _DeeDict_ShouldShrinkHTab(self) is true (or `fully_shrink=true')
 * NOTE: After a call to this function, the caller must always rebuild the htab! */
PRIVATE NONNULL((1)) void DCALL
dict_shrink_htab(Dict *__restrict self, bool fully_shrink) {
	size_t new_hmask;
	size_t new_tabsize;
	struct Dee_dict_item *old_vtab;
	struct Dee_dict_item *new_vtab;
	ASSERT(DeeDict_LockWriting(self));
	ASSERT(_DeeDict_CanShrinkHTab(self));
	ASSERT(_DeeDict_ShouldShrinkHTab(self) || fully_shrink);
	/* Figure out the new hmask */
	new_hmask = self->d_hmask;
	if (fully_shrink) {
		do {
			new_hmask >>= 1;
		} while (_DeeDict_CanShrinkHTab2(self->d_valloc, new_hmask));
	} else {
		do {
			new_hmask >>= 1;
		} while (_DeeDict_ShouldShrinkHTab2(self->d_valloc, new_hmask));
	}
	ASSERT(new_hmask >= self->d_valloc);
	ASSERT(new_hmask < self->d_hmask);
	ASSERT(self->d_valloc > 0);
	ASSERT(new_hmask > 0);
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc(new_hmask, self->d_valloc);
	old_vtab = _DeeDict_GetRealVTab(self);
	ASSERT(old_vtab != (struct Dee_dict_item *)_DeeDict_EmptyTab);
	new_vtab = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab)
		new_vtab = old_vtab;
	_DeeDict_SetRealVTab(self, new_vtab);
	self->d_hmask = new_hmask;

	dict_htab_rebuild(self);
}


/* Shrink the vtab+htab. Must be called while:
 * - holding a write-lock
 * - _DeeDict_CanShrinkVTab(self) is true
 * - _DeeDict_ShouldShrinkVTab(self) is true (or `fully_shrink=true') */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_shrink_vtab_and_htab(Dict *__restrict self, bool fully_shrink) {
	bool must_rebuild_htab = false;
	struct Dee_dict_item *old_vtab;
	struct Dee_dict_item *new_vtab;
	size_t old_valloc;
	size_t new_valloc;
	size_t new_tabsize;
	size_t old_hmask;
	size_t new_hmask;
	shift_t old_hidxio;
	shift_t new_hidxio;
	ASSERT(DeeDict_LockWriting(self));
	ASSERT(_DeeDict_CanShrinkVTab(self));
	ASSERT(_DeeDict_ShouldShrinkVTab(self) || fully_shrink);
	old_vtab = _DeeDict_GetRealVTab(self);
	if unlikely(!self->d_vused) {
		/* Special case: dict is now empty -> clear all data. */
		self->d_valloc  = 0;
		self->d_vsize   = 0;
		_DeeDict_SetVirtVTab(self, DeeDict_EmptyVTab);
		self->d_hmask   = 0;
		self->d_hidxget = &Dee_dict_gethidx8;
		self->d_hidxset = &Dee_dict_sethidx8;
		self->d_htab    = DeeDict_EmptyHTab;
		if (old_vtab != (struct Dee_dict_item *)_DeeDict_EmptyTab)
			_DeeDict_TabsFree(old_vtab);
		return;
	}

	/* Optimize the dict so we don't have to transfer deleted
	 * items, and can shrink even more when `fully_shrink=true' */
	if (_DeeDict_CanOptimizeVTab(self)) {
		dict_do_optimize_vtab_without_rebuild(self);
		must_rebuild_htab = true;
	}

	/* Calculate changes in buffer size */
	ASSERT(self->d_vsize < self->d_valloc);
	old_valloc = self->d_valloc;
	new_valloc = fully_shrink ? self->d_vsize : _DeeDict_ShouldShrinkVTab_NEWALLOC(self);
	ASSERT(new_valloc >= self->d_vsize);
	old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(old_valloc);
	new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
	ASSERT(new_hidxio <= old_hidxio);
	old_hmask = self->d_hmask;
	new_hmask = old_hmask;
	if (fully_shrink) {
		while (_DeeDict_CanShrinkHTab2(new_valloc, new_hmask))
			new_hmask >>= 1;
	} else {
		while (_DeeDict_ShouldShrinkHTab2(new_valloc, new_hmask))
			new_hmask >>= 1;
	}
	ASSERT(new_hmask <= old_hmask);
	ASSERT(new_hmask >= new_valloc);

	/* Shrink dict hash-table in-place (so we can then inplace-realloc-trunc the table heap area) */
	if (old_hmask == new_hmask) {
		if (!must_rebuild_htab) {
			void *old_htab = old_vtab + old_valloc;
			void *new_htab = old_vtab + new_valloc;
			if (new_hidxio == old_hidxio) {
				(*Dee_dict_hidxio[new_hidxio].dhxio_movdown)(new_htab, old_htab, self->d_hmask + 1);
			} else {
				shift_t current_hidxio;
				(*Dee_dict_hidxio[old_hidxio].dhxio_dwn)(new_htab, old_htab, self->d_hmask + 1);
				current_hidxio = old_hidxio - 1;
				while (current_hidxio > new_hidxio) {
					(*Dee_dict_hidxio[current_hidxio].dhxio_dwn)(new_htab, new_htab, self->d_hmask + 1);
					--current_hidxio;
				}
			}
		}
	} else {
		must_rebuild_htab = true;
	}

	/* Actually realloc the dict tables. */
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);
	ASSERT(new_valloc < old_valloc);
	ASSERT(new_valloc >= self->d_vsize);
	ASSERTF(old_vtab != (struct Dee_dict_item *)_DeeDict_EmptyTab,
	        "The empty table should have been handled by '!self->d_vused'");
	new_vtab = (struct Dee_dict_item *)_DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab)
		new_vtab = old_vtab; /* Shouldn't get here because the new table is always smaller! */

	/* Assign new tables / control values. */
	self->d_valloc  = new_valloc;
	_DeeDict_SetRealVTab(self, new_vtab);
	self->d_htab    = new_vtab + new_valloc;
	self->d_hmask   = new_hmask;
	self->d_hidxget = Dee_dict_hidxio[new_hidxio].dhxio_get;
	self->d_hidxset = Dee_dict_hidxio[new_hidxio].dhxio_set;

	/* Re-build the hash-table if necessary. */
	if (must_rebuild_htab)
		dict_htab_rebuild_after_optimize(self);
}


/* Automatically shrink allocated tables of "self" if appropriate.
 * Call this function are removing elements from "self"
 * Same as the API function "dict.shrink(fully: false)" */
LOCAL NONNULL((1)) void DCALL
dict_autoshrink(Dict *__restrict self) {
	if (_DeeDict_ShouldShrinkVTab(self))
		dict_shrink_vtab_and_htab(self, false);
}



/* Decrement all vtab indices appearing the htab that are ">= vtab_threshold" */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL dict_htab_decafter(Dict *__restrict self, /*virt*/Dee_dict_vidx_t vtab_threshold);

/* Increment all vtab indices appearing the htab that are ">= vtab_threshold" */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL dict_htab_incafter(Dict *__restrict self, /*virt*/Dee_dict_vidx_t vtab_threshold);

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL dict_htab_decrange(Dict *__restrict self, /*virt*/Dee_dict_vidx_t vtab_min, /*virt*/Dee_dict_vidx_t vtab_max);
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL dict_htab_incrange(Dict *__restrict self, /*virt*/Dee_dict_vidx_t vtab_min, /*virt*/Dee_dict_vidx_t vtab_max);

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_dict_htab_decafter
#include "dict-htabmod.c.inl"
#define DEFINE_dict_htab_incafter
#include "dict-htabmod.c.inl"
#define DEFINE_dict_htab_decrange
#include "dict-htabmod.c.inl"
#define DEFINE_dict_htab_incrange
#include "dict-htabmod.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */



PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_makespace_at_impl(Dict *__restrict self, /*real*/ Dee_dict_vidx_t vtab_idx) {
	struct Dee_dict_item *vtab = _DeeDict_GetRealVTab(self);
	ASSERT(vtab_idx < self->d_vsize);
	memmoveupc(&vtab[vtab_idx + 1], &vtab[vtab_idx],
	           self->d_vsize - vtab_idx, sizeof(struct Dee_dict_item));
	dict_htab_incafter(self, Dee_dict_vidx_tovirt(vtab_idx));
}

LOCAL NONNULL((1)) void DCALL
dict_makespace_at(Dict *__restrict self, /*real*/ Dee_dict_vidx_t vtab_idx) {
	ASSERT(vtab_idx <= self->d_vsize);
	if (vtab_idx < self->d_vsize)
		dict_makespace_at_impl(self, vtab_idx);
}



/* Fast-compare functions that never throw or invoke user-code:
 * - Can be called while holding locks.
 * @return:  1: It is guarantied that "index != rhs"
 * @return:  0: It is guarantied that "index == rhs"
 * @return: -1: Comparison cannot be done "fast" - load "rhs" properly, release locks, and try again. */
PRIVATE WUNUSED NONNULL((2)) int DCALL fastcmp_index(size_t lhs, DeeObject *rhs);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL fastcmp_string(char const *lhs, DeeObject *rhs);
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL fastcmp_string_len(char const *lhs, size_t lhslen, DeeObject *rhs);

/* Slow equivalents to the above -- only use these when the above returned "-1"
 * Return value is like `DeeObject_TryCompareEq()' (iow: Dee_COMPARE_ERR on error) */
PRIVATE WUNUSED NONNULL((2)) int DCALL slowcmp_index(size_t lhs, DeeObject *rhs);
//PRIVATE WUNUSED NONNULL((1, 2)) int DCALL slowcmp_string(char const *lhs, DeeObject *rhs);
//PRIVATE WUNUSED NONNULL((1, 3)) int DCALL slowcmp_string_len(char const *lhs, size_t lhslen, DeeObject *rhs);

PRIVATE WUNUSED NONNULL((2)) DREF DeeObject *DCALL matched_keyob_tryfrom_index(size_t key, DeeObject *matched_key);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL matched_keyob_tryfrom_string(char const *key, DeeObject *matched_key);
PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL matched_keyob_tryfrom_string_len(char const *key, size_t keylen, DeeObject *matched_key);



PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_getitem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getitem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_getitem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_getitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_trygetitem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_trygetitem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_trygetitem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_trygetitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_bounditem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_bounditem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_bounditem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_bounditem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);

#ifdef Dee_BOUND_MAYALIAS_HAS
#define dict_hasitem                 dict_bounditem
#define dict_hasitem_index           dict_bounditem_index
#define dict_hasitem_string_hash     dict_bounditem_string_hash
#define dict_hasitem_string_len_hash dict_bounditem_string_len_hash
#else /* Dee_BOUND_MAYALIAS_HAS */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_hasitem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_hasitem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_hasitem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_hasitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);
#ifdef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem(Dict *self, DeeObject *key) {
	int bound = dict_bounditem(self, key);
	return Dee_BOUND_ASHAS(bound);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_hasitem_index(Dict *self, size_t index) {
	int bound = dict_bounditem_index(self, index);
	return Dee_BOUND_ASHAS(bound);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem_string_hash(Dict *self, char const *key, Dee_hash_t hash) {
	int bound = dict_bounditem_string_hash(self, key, hash);
	return Dee_BOUND_ASHAS(bound);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int bound = dict_bounditem_string_len_hash(self, key, keylen, hash);
	return Dee_BOUND_ASHAS(bound);
}
#endif /* __OPTIMIZE_SIZE__ */
#endif /* !Dee_BOUND_MAYALIAS_HAS */

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL dict_setitem(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL dict_setitem_index(Dict *self, size_t index, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL dict_setitem_string_hash(Dict *self, char const *key, Dee_hash_t hash, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL dict_setitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_setold_ex(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_setnew_ex(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_setdefault(Dict *self, DeeObject *key, DeeObject *value);
/* @param: getindex: returns the unoptimized (iow: including deleted keys) index
 *                   in "d_vtab" where the new key-value pair should be inserted.
 *                   All of the other functions above simply append at the end of
 *                   "d_vtab", which is the same as this callback returning "d_vsize"
 *                   @param: overwrite_index: When "key" already exists, the index of
 *                                            the item that will be deleted. Else, set
 *                                            to `Dee_DICT_HTAB_EOF' when "key" is new.
 *                   @param: p_value: Pointer to the value that will be written to the key
 *                                    May be changed here to some other value already within
 *                                    the dict, in order to allow atomically assigning some
 *                                    value already found within the dict.
 *                   - To throw an error, the callback should:
 *                     >> DeeDict_LockEndWrite(self)
 *                     >> DeeError_Throw(...);
 *                     >> return Dee_DICT_HTAB_EOF; */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL dict_setitem_at(Dict *self, DeeObject *key, DeeObject *value, /*virt*/ Dee_dict_vidx_t (DCALL *getindex)(void *cookie, Dict *self, /*virt*/ Dee_dict_vidx_t overwrite_index, DeeObject **p_value), void *getindex_cookie);
#ifndef HAVE_dict_setitem_unlocked
#define dict_setitem_unlocked dict_setitem
#else /* !HAVE_dict_setitem_unlocked */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL dict_setitem_unlocked(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL dict_setitem_unlocked_fast_inherited(Dict *self, /*inherit(on_success)*/ DREF DeeObject *key, /*inherit(on_success)*/ DREF DeeObject *value);
#define HAVE_dict_setitem_unlocked_fast_inherited
#endif /* HAVE_dict_setitem_unlocked */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_delitem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_delitem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_delitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_delitem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_mh_remove(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_mh_pop(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_pop_with_default(Dict *self, DeeObject *key, DeeObject *def);
#ifdef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_popvalue(Dict *self, DeeObject *key);
#endif /* __OPTIMIZE_SIZE__ */





/* Fast-compare functions that never throw or invoke user-code:
 * - Can be called while holding locks.
 * @return:  1: It is guarantied that "index != rhs"
 * @return:  0: It is guarantied that "index == rhs"
 * @return: -1: Comparison cannot be done "fast" - load "rhs" properly, release locks, and try again. */
PRIVATE WUNUSED NONNULL((2)) int DCALL
fastcmp_index(size_t lhs, DeeObject *rhs) {
	if (DeeInt_Check(rhs)) {
		size_t rhs_value;
		if (!DeeInt_TryAsSize(rhs, &rhs_value))
			return 1;
		return lhs == rhs_value ? 0 : 1;
	}
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fastcmp_string(char const *lhs, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return !!strcmp(lhs, DeeString_STR(rhs));
	if (DeeBytes_Check(rhs)) {
		size_t lhslen = strlen(lhs);
		if (lhslen != DeeBytes_SIZE(rhs))
			return 1;
		return !!memcmp(lhs, DeeBytes_DATA(rhs), lhslen);
	}
	/* TODO: No need for "-1" here -- re-write consumers of this function to take advantage of that fact! */
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
fastcmp_string_len(char const *lhs, size_t lhslen, DeeObject *rhs) {
	if (DeeString_Check(rhs)) {
		if (lhslen != DeeString_SIZE(rhs))
			return 1;
		return !!memcmp(lhs, DeeString_STR(rhs), lhslen);
	}
	if (DeeBytes_Check(rhs)) {
		if (lhslen != DeeBytes_SIZE(rhs))
			return 1;
		return !!memcmp(lhs, DeeBytes_DATA(rhs), lhslen);
	}
	/* TODO: No need for "-1" here -- re-write consumers of this function to take advantage of that fact! */
	return -1;
}

/* Slow equivalents to the above -- only use these when the above returned "-1"
 * Return value is like `DeeObject_TryCompareEq()' (iow: Dee_COMPARE_ERR on error) */
PRIVATE WUNUSED NONNULL((2)) int DCALL
slowcmp_index(size_t lhs, DeeObject *rhs) {
	bool ok;
	size_t rhs_value;
	DREF DeeObject *rhs_as_int;
	rhs_as_int = DeeObject_Int(rhs);
	if unlikely(!rhs_as_int)
		goto err;
	ok = DeeInt_TryAsSize(rhs_as_int, &rhs_value);
	Dee_Decref(rhs_as_int);
	if (!ok)
		return 1;
	return lhs == rhs_value ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((2)) DREF DeeObject *DCALL
matched_keyob_tryfrom_index(size_t key, DeeObject *matched_key) {
	if (DeeInt_Check(matched_key))
		return_reference_(matched_key);
#ifdef DeeInt_TryNewSize
	return DeeInt_TryNewSize(key);
#else /* DeeInt_TryNewSize */
	(void)key;
	return NULL;
#endif /* !DeeInt_TryNewSize */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
matched_keyob_tryfrom_string(char const *key, DeeObject *matched_key) {
	if (DeeString_Check(matched_key))
		return_reference_(matched_key);
#ifdef DeeString_TryNew
	return DeeString_TryNew(key);
#else /* DeeString_TryNew */
	(void)key;
	return NULL;
#endif /* !DeeString_TryNew */
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
matched_keyob_tryfrom_string_len(char const *key, size_t keylen, DeeObject *matched_key) {
	if (DeeString_Check(matched_key))
		return_reference_(matched_key);
#ifdef DeeString_TryNewSized
	return DeeString_TryNewSized(key, keylen);
#else /* DeeString_TryNewSized */
	(void)key;
	(void)keylen;
	return NULL;
#endif /* !DeeString_TryNewSized */
}



#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_dict_trygetitem
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_trygetitem_index
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_trygetitem_string_hash
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_trygetitem_string_len_hash
#include "dict-getitem-impl.c.inl"

#ifndef __OPTIMIZE_SIZE__
#define DEFINE_dict_getitem
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_getitem_index
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_getitem_string_hash
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_getitem_string_len_hash
#include "dict-getitem-impl.c.inl"

#define DEFINE_dict_bounditem
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_bounditem_index
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_bounditem_string_hash
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_bounditem_string_len_hash
#include "dict-getitem-impl.c.inl"

#ifndef Dee_BOUND_MAYALIAS_HAS
#define DEFINE_dict_hasitem
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_hasitem_index
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_hasitem_string_hash
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_hasitem_string_len_hash
#include "dict-getitem-impl.c.inl"
#endif /* !Dee_BOUND_MAYALIAS_HAS */
#endif /* !__OPTIMIZE_SIZE__ */

#define DEFINE_dict_setitem
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_at
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_string_hash
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_string_len_hash
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_index
#include "dict-setitem-impl.c.inl"
#ifdef HAVE_dict_setitem_unlocked
#define DEFINE_dict_setitem_unlocked
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_unlocked_fast_inherited
#endif /* HAVE_dict_setitem_unlocked */
#ifndef __OPTIMIZE_SIZE__
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_mh_setdefault
#include "dict-setitem-impl.c.inl"
#endif /* !__OPTIMIZE_SIZE__ */
#define DEFINE_dict_mh_setold_ex
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_mh_setnew_ex
#include "dict-setitem-impl.c.inl"

#define DEFINE_dict_delitem_string_hash
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_delitem_string_len_hash
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_delitem_index
#include "dict-delitem-impl.c.inl"
#ifdef __OPTIMIZE_SIZE__
#define DEFINE_dict_popvalue
#include "dict-delitem-impl.c.inl"
#else /* __OPTIMIZE_SIZE__ */
#define DEFINE_dict_delitem
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_mh_remove
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_mh_pop
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_mh_pop_with_default
#include "dict-delitem-impl.c.inl"
#endif /* !__OPTIMIZE_SIZE__ */
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#ifdef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_remove(Dict *self, DeeObject *key) {
	DREF DeeObject *value = dict_popvalue(self, key);
	if unlikely(!value)
		return -1;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_delitem(Dict *self, DeeObject *key) {
	DREF DeeObject *value = dict_popvalue(self, key);
	if unlikely(!value)
		return -1;
	if (value != ITER_DONE)
		Dee_Decref(value);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_mh_pop(Dict *self, DeeObject *key) {
	DREF DeeObject *result = dict_popvalue(self, key);
	if unlikely(result == ITER_DONE) {
		err_unknown_key((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_pop_with_default(Dict *self, DeeObject *key, DeeObject *def) {
	DREF DeeObject *result = dict_popvalue(self, key);
	if unlikely(result == ITER_DONE) {
		Dee_Incref(def);
		result = def;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem(Dict *self, DeeObject *key) {
	DREF DeeObject *result = dict_trygetitem(self, DeeObject key);
	if unlikely(result == ITER_DONE) {
		err_unknown_key((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_getitem_index(Dict *self, size_t index) {
	DREF DeeObject *result = dict_trygetitem_index(self, index);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_int((DeeObject *)self, index);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem_string_hash(Dict *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = dict_trygetitem_string_hash(self, key, hash);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = dict_trygetitem_string_len_hash(self, key, size_t keylen, hash);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str_len((DeeObject *)self, key, keylen);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED int DCALL
bound_from_trygetitem(DREF DeeObject *value) {
	if unlikely(!value)
		return Dee_BOUND_ERR;
	if (value == ITER_DONE)
		return Dee_BOUND_MISSING;
	Dee_Decref(value);
	return Dee_BOUND_YES;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem(Dict *self, DeeObject *key) {
	return bound_from_trygetitem(dict_trygetitem(self, key));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bounditem_index(Dict *self, size_t index) {
	return bound_from_trygetitem(dict_trygetitem_index(self, index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_hash(Dict *self, char const *key, Dee_hash_t hash) {
	return bound_from_trygetitem(dict_trygetitem_string_hash(self, key, hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return bound_from_trygetitem(dict_trygetitem_string_len_hash(self, key, keylen, hash));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_setdefault(Dict *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result = dict_mh_setnew_ex(self, key, value);
	if (result == ITER_DONE) {
		result = value;
		Dee_Incref(result);
	}
	return result;
}
#endif /* __OPTIMIZE_SIZE__ */


#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define dict_fromsequence_foreach_cb (*(Dee_ssize_t (DCALL *)(void *, DeeObject *, DeeObject *))&dict_setitem_unlocked)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
dict_fromsequence_foreach_cb(void *self, DeeObject *key, DeeObject *value) {
	return dict_setitem_unlocked((Dict *)self, key, value);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

PRIVATE NONNULL((1)) void DCALL
DeeDict_LockReadAndOptimize(Dict *__restrict self) {
	DeeDict_LockRead(self);
	if (_DeeDict_CanOptimizeVTab(self)) {
		if (DeeDict_LockUpgrade(self) || _DeeDict_CanOptimizeVTab(self))
			dict_optimize_vtab(self);
		DeeDict_LockDowngrade(self);
	}
}

#undef DICT_INITFROM_NEEDSLOCK
#ifndef HAVE_dict_setitem_unlocked /* Because of "#define dict_setitem_unlocked dict_setitem" */
#define DICT_INITFROM_NEEDSLOCK
#endif /* !HAVE_dict_setitem_unlocked */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_fromcopy(Dict *__restrict self, Dict *__restrict other) {
	size_t copy_hmask;
	size_t copy_valloc;
	size_t copy_vsize;
	shift_t copy_hidxio;
	void *copy_tabs;
	size_t copy_tabssz;
	struct Dee_dict_item *copy_vtab;
	void *copy_htab;
again:
	DeeDict_LockReadAndOptimize(other);

	/* Figure out size of the copied tables. */
	copy_vsize  = other->d_vsize;
	copy_hmask  = dict_hmask_from_count(copy_vsize);
	copy_valloc = dict_valloc_from_hmask_and_count(copy_hmask, copy_vsize, true);
	if (copy_valloc >= other->d_valloc) {
		copy_valloc = other->d_valloc;
	} else {
		copy_valloc = dict_valloc_from_hmask_and_count(copy_hmask, copy_vsize, false);
		if unlikely(copy_valloc > other->d_valloc)
			copy_valloc = other->d_valloc;
	}
	copy_hidxio = DEE_DICT_HIDXIO_FROMALLOC(copy_valloc);
	copy_tabssz = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(copy_hmask, copy_valloc, copy_hidxio);
	copy_tabs   = _DeeDict_TabsTryMalloc(copy_tabssz);

	/* Try to allocate smaller tables if the first allocation failed. */
	if unlikely(!copy_tabs) {
		copy_valloc = dict_valloc_from_hmask_and_count(copy_hmask, copy_vsize, false);
		if unlikely(copy_valloc > other->d_valloc)
			copy_valloc = other->d_valloc;
		copy_hidxio = DEE_DICT_HIDXIO_FROMALLOC(copy_valloc);
		copy_tabssz = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(copy_hmask, copy_valloc, copy_hidxio);
		copy_tabs   = _DeeDict_TabsTryMalloc(copy_tabssz);
		if unlikely(!copy_tabs) {
			DeeDict_LockEndRead(other);
			copy_tabs = _DeeDict_TabsMalloc(copy_tabssz);
			if unlikely(!copy_tabs)
				goto err;
			DeeDict_LockReadAndOptimize(other);
			if unlikely(copy_vsize < other->d_vsize) {
				DeeDict_LockEndRead(other);
				_DeeDict_TabsFree(copy_tabs);
				goto again;
			}
		}
	}

	/* Copy data into copy's tables. */
	copy_vtab = (struct Dee_dict_item *)copy_tabs;
	copy_htab = (struct Dee_dict_item *)copy_tabs + copy_valloc;
	copy_vtab = (struct Dee_dict_item *)memcpyc(copy_vtab, _DeeDict_GetRealVTab(other), other->d_vsize,
	                                            sizeof(struct Dee_dict_item));

	/* Create reference to copied objects. */
	{
		size_t i;
		for (i = 0; i < copy_vsize; ++i) {
			struct Dee_dict_item *item;
			item = &copy_vtab[i];
			ASSERTF(item->di_key, "Table was optimized above, so there should be no deleted keys!");
			Dee_Incref(item->di_key);
			Dee_Incref(item->di_value);
		}
	}

	/* Fill in control structures. */
	self->d_valloc  = copy_valloc;
	self->d_vsize   = copy_vsize;
	self->d_vused   = other->d_vused;
	_DeeDict_SetRealVTab(self, copy_vtab);
	self->d_hmask   = copy_hmask;
	self->d_hidxget = Dee_dict_hidxio[copy_hidxio].dhxio_get;
	self->d_hidxset = Dee_dict_hidxio[copy_hidxio].dhxio_set;
	self->d_htab    = copy_htab;

	if (copy_hmask == other->d_hmask) {
		/* No need to re-build the hash table. Can instead copy is verbatim. */
		shift_t orig_hidxio = DEE_DICT_HIDXIO_FROMALLOC(other->d_valloc);
		size_t htab_words = copy_hmask + 1;
		ASSERT(copy_hidxio <= orig_hidxio || (copy_hidxio + 1) == orig_hidxio);
		if (orig_hidxio == copy_hidxio) {
			size_t sizeof_htab = htab_words << copy_hidxio;
			(void)memcpy(copy_htab, other->d_htab, sizeof_htab);
		} else if (copy_hidxio > orig_hidxio) {
			(*Dee_dict_hidxio[copy_hidxio].dhxio_upr)(copy_htab, other->d_htab, htab_words);
		} else if (copy_hidxio == orig_hidxio - 1) {
			(*Dee_dict_hidxio[copy_hidxio].dhxio_dwn)(copy_htab, other->d_htab, htab_words);
		} else {
			size_t i;
			ASSERT(copy_hidxio < orig_hidxio);
			for (i = 0; i < htab_words; ++i)
				(*self->d_hidxset)(copy_htab, i, (*other->d_hidxget)(other->d_htab, i));
		}
	} else {
		/* Copy has a different hash-mask -> hash table cannot be copied and needs to be re-build! */
		dict_htab_rebuild_after_optimize(self);
	}
	DeeDict_LockEndRead(other);
#ifdef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* DICT_INITFROM_NEEDSLOCK */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF Dict *DCALL
dict_new_copy(Dict *__restrict self) {
	DREF Dict *result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if unlikely(dict_init_fromcopy(result, self))
		goto err_r;
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&result->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_TRACK(Dict, result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromSequence(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	DREF Dict *result;
	size_t hint;
	if (DeeDict_CheckExact(self)) {
		/* Special optimization when "self" is another dict:
		 * Optimize "self" and then duplicate its control structures */
		return (DREF DeeObject *)dict_new_copy((Dict *)self);
	}
	if (DeeRoDict_Check(self)) {
		/* Special optimization when "self" is an RoDict:
		 * Duplicate its control structures */
		/* TODO */
	}
	hint = DeeObject_SizeFast(self);
	if likely(hint == (size_t)-1) {
		result = (DREF Dict *)DeeDict_TryNewWithHint(hint);
	} else {
		result = (DREF Dict *)DeeDict_TryNewWithWeakHint(DICT_FROMSEQ_DEFAULT_HINT);
	}
	if unlikely(!result) {
		result = (DREF Dict *)DeeDict_New();
		if unlikely(!result)
			goto err;
	}
	foreach_status = DeeObject_ForeachPair(self, &dict_fromsequence_foreach_cb, result);
	if unlikely(foreach_status < 0)
		goto err_r;
	return (DREF DeeObject *)result;
err_r:
	Dee_DecrefDokill(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromSequenceInherited(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	if (DeeDict_CheckExact(self) && !DeeObject_IsShared(self))
		return self; /* Can re-use existing Dict object. */
	result = DeeDict_FromSequence(self);
	if likely(result)
		Dee_Decref_unlikely(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromRoDict(DeeObject *__restrict self) {
	/* TODO */
	return DeeDict_FromSequence(self);
}

/* Create a new Dict by inheriting a set of passed key-item pairs.
 * @param: key_values: A vector containing `num_items*2' elements,
 *                     even ones being keys and odd ones being items.
 * @param: num_items:  The number of key-value pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewKeyValuesInherited(size_t num_items,
                              /*inhert(on_success)*/ DREF DeeObject **key_values) {
	DREF Dict *result;
#ifdef HAVE_dict_setitem_unlocked_fast_inherited
	result = (DREF Dict *)DeeDict_TryNewWithHint(num_items);
	if likely(result) {
		size_t i;
		for (i = 0; i < num_items; ++i) {
			DREF DeeObject *key   = key_values[(i * 2) + 0];
			DREF DeeObject *value = key_values[(i * 2) + 1];
			/* FIXME: This decrefs duplicate keys immediately, which won't be restored on error. */
			if unlikely(dict_setitem_unlocked_fast_inherited(result, key, value))
				goto err_r;
		}
	} else
#endif /* HAVE_dict_setitem_unlocked_fast_inherited */
	{
		size_t i;
		result = (DREF Dict *)DeeDict_New();
		if unlikely(!result)
			goto err;
		for (i = 0; i < num_items; ++i) {
			DREF DeeObject *key   = key_values[(i * 2) + 0];
			DREF DeeObject *value = key_values[(i * 2) + 1];
			if unlikely(dict_setitem_unlocked(result, key, value))
				goto err_r;
			ASSERT(DeeObject_IsShared(key));
			ASSERT(DeeObject_IsShared(value));
			Dee_DecrefNokill(value);
			Dee_DecrefNokill(key);
		}
	}
	return (DREF DeeObject *)result;
err_r:
	/* Free "result" without inheriting references to already inserted key/value pairs */
	if likely(_DeeDict_GetVirtVTab(result) != DeeDict_EmptyVTab)
		_DeeDict_TabsFree(_DeeDict_GetRealVTab(result));
	Dee_DecrefNokill(&DeeDict_Type);
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_fini(Dict *__restrict self) {
	if (_DeeDict_GetVirtVTab(self) != DeeDict_EmptyVTab) {
		Dee_dict_vidx_t i;
		struct Dee_dict_item *vtab = _DeeDict_GetRealVTab(self);
		for (i = 0; i < self->d_vsize; ++i) {
			if (vtab[i].di_key) {
				Dee_Decref(vtab[i].di_key);
				Dee_Decref(vtab[i].di_value);
			}
		}
		_DeeDict_TabsFree(vtab);
	}
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_visit(Dict *__restrict self, dvisit_t proc, void *arg) {
	Dee_dict_vidx_t i;
	struct Dee_dict_item *vtab;
	DeeDict_LockRead(self);
	vtab = _DeeDict_GetVirtVTab(self);
	for (i = Dee_dict_vidx_tovirt(0);
	     Dee_dict_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		if (vtab[i].di_key) {
			Dee_Visit(vtab[i].di_key);
			Dee_Visit(vtab[i].di_value);
		}
	}
	DeeDict_LockEndRead(self);
}


LOCAL NONNULL((1)) void DCALL
dict_initfrom_empty(Dict *__restrict self) {
	self->d_valloc  = 0;
	self->d_vsize   = 0;
	self->d_vused   = 0;
	_DeeDict_SetVirtVTab(self, DeeDict_EmptyVTab);
	self->d_hmask   = 0;
	self->d_hidxget = &Dee_dict_gethidx8;
	self->d_hidxset = &Dee_dict_sethidx8;
	self->d_htab    = DeeDict_EmptyHTab;
#ifdef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* DICT_INITFROM_NEEDSLOCK */
}

LOCAL NONNULL((1)) void DCALL
dict_initfrom_hint(Dict *__restrict self, size_t num_items, bool allow_overalloc) {
	size_t hmask, valloc;
	shift_t hidxio;
	void *tabs;
	if unlikely(!num_items) {
init_empty:
		hmask  = 0;
		valloc = 0;
		hidxio = 0;
		tabs   = (void *)_DeeDict_EmptyTab;
	} else {
		size_t tabssz;
		hmask  = dict_hmask_from_count(num_items);
		valloc = dict_valloc_from_hmask_and_count(hmask, num_items, allow_overalloc);
		hidxio = DEE_DICT_HIDXIO_FROMALLOC(valloc);
		tabssz = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(hmask, valloc, hidxio);
		tabs   = _DeeDict_TabsTryCalloc(tabssz);
		if unlikely(!tabs)
			goto init_empty;
	}
	self->d_valloc  = valloc;
	self->d_vsize   = 0;
	self->d_vused   = 0;
	_DeeDict_SetRealVTab(self, (struct Dee_dict_item *)tabs);
	self->d_hmask   = hmask;
	self->d_hidxget = Dee_dict_hidxio[hidxio].dhxio_get;
	self->d_hidxset = Dee_dict_hidxio[hidxio].dhxio_set;
	self->d_htab    = (struct Dee_dict_item *)tabs + valloc;
#ifdef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* DICT_INITFROM_NEEDSLOCK */
}

LOCAL WUNUSED NONNULL((1)) int DCALL
dict_initfrom_seq(Dict *__restrict self, DeeObject *seq) {
	Dee_ssize_t foreach_status;
	size_t hint;

	/* Special optimization when "seq" is a Dict: Duplicate its control structures */
	if (DeeDict_CheckExact(seq))
		return dict_init_fromcopy(self, (Dict *)seq);

	/* Special optimization when "seq" is an RoDict: Duplicate its control structures */
	if (DeeRoDict_Check(seq)) {
		/* TODO */
	}
	hint = DeeObject_SizeFast(seq);
	if likely(hint != (size_t)-1) {
		dict_initfrom_hint(self, hint, false);
	} else {
		dict_initfrom_hint(self, DICT_FROMSEQ_DEFAULT_HINT, true);
	}
	foreach_status = DeeObject_ForeachPair(seq, &dict_fromsequence_foreach_cb, self);
	if unlikely(foreach_status < 0)
		goto err_self;
	return 0;
err_self:
	dict_fini(self);
	return -1;
}



/* Minimal (mostly) Dict-compatible buffer for constructing temporary dict objects. */
#define _DictBuffer_startoff offsetof(Dict, d_valloc)
#ifndef CONFIG_NO_THREADS
#define _DictBuffer_endoff_WITHOUT_LOCK offsetof(Dict, d_lock)
#else /* !CONFIG_NO_THREADS */
#define _DictBuffer_endoff_WITHOUT_LOCK offsetof(Dict, ob_weakrefs)
#endif /* CONFIG_NO_THREADS */
#ifdef DICT_INITFROM_NEEDSLOCK
#define _DictBuffer_endoff_WITH_LOCK offsetof(Dict, ob_weakrefs)
#else /* DICT_INITFROM_NEEDSLOCK */
#define _DictBuffer_endoff_WITH_LOCK _DictBuffer_endoff_WITHOUT_LOCK
#endif /* !DICT_INITFROM_NEEDSLOCK */
#define _DictBuffer_NWORDS_WITH_LOCK    CEILDIV(_DictBuffer_endoff_WITH_LOCK - _DictBuffer_startoff, sizeof(uintptr_t))
#define _DictBuffer_NWORDS_WITHOUT_LOCK CEILDIV(_DictBuffer_endoff_WITHOUT_LOCK - _DictBuffer_startoff, sizeof(uintptr_t))

typedef struct {
	uintptr_t db_data[_DictBuffer_NWORDS_WITH_LOCK];
} DictBuffer;
#define DictBuffer_AsDict(self) ((Dict *)((byte_t *)((self)->db_data) - _DictBuffer_startoff))
#define Dict_AsDictBuffer(self) ((DictBuffer *)((byte_t *)(self) + _DictBuffer_startoff))
#define DictBuffer_Fini(self) \
	dict_fini(DictBuffer_AsDict(self))
#define DictBuffer_InitEmpty(self) dict_initfrom_empty(DictBuffer_AsDict(self))
#define DictBuffer_InitWithHint(self, num_items, allow_overalloc) \
	dict_initfrom_hint(DictBuffer_AsDict(self), num_items, allow_overalloc)
#define DictBuffer_InitFromSequence(self, seq) \
	dict_initfrom_seq(DictBuffer_AsDict(self), seq)

/* Init "self" by stealing data from "dict"
 * CAUTION: WILL NOT INITIALIZE THE BUFFER'S LOCK WORD!!! */
#define DictBuffer_FromDict(self, dict) \
	memcpyp((self)->db_data, Dict_AsDictBuffer(dict), _DictBuffer_NWORDS_WITHOUT_LOCK)

/* Transfer the contents of the buffer into "dict"
 * CAUTION: WILL OVERRIDE WHATEVER WAS IN "dict" BEFORE!!! */
#define DictBuffer_IntoDict(self, dict) \
	memcpyp(Dict_AsDictBuffer(dict), (self)->db_data, _DictBuffer_NWORDS_WITHOUT_LOCK)
	

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_assign(Dict *self, DeeObject *seq) {
	DictBuffer new_buffer;
	DictBuffer old_buffer;
	if unlikely(self == (Dict *)seq)
		return 0;
	if unlikely(DictBuffer_InitFromSequence(&new_buffer, seq))
		goto err;
	DeeDict_LockWrite(self);
	DictBuffer_FromDict(&old_buffer, self);
	DictBuffer_IntoDict(&new_buffer, self);
	DeeDict_LockEndWrite(self);
	DictBuffer_Fini(&old_buffer);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_moveassign(Dict *self, Dict *other) {
	if likely(self != other) {
		DictBuffer old_buffer;
		DeeLock_Acquire2(DeeDict_LockWrite(self), DeeDict_LockTryWrite(self), DeeDict_LockEndWrite(self),
		                 DeeDict_LockWrite(other), DeeDict_LockTryWrite(other), DeeDict_LockEndWrite(other));
		DictBuffer_FromDict(&old_buffer, self);
		DictBuffer_IntoDict(Dict_AsDictBuffer(other), self);
		DictBuffer_InitEmpty(Dict_AsDictBuffer(other));
		DeeDict_LockEndWrite(other);
		DeeDict_LockEndWrite(self);
		DictBuffer_Fini(&old_buffer);
	}
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_ctor(Dict *__restrict self) {
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	weakref_support_init(self);
	dict_initfrom_empty(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_copy(Dict *__restrict self, Dict *__restrict other) {
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	weakref_support_init(self);
	return dict_init_fromcopy(self, other);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_init(Dict *__restrict self, size_t argc, DeeObject *const *argv) {
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	weakref_support_init(self);
	switch (argc) {

	case 1: {
		DeeObject *arg = argv[0];
		if (DeeInt_Check(arg)) {
			size_t hint;
			if unlikely(DeeInt_AsSize(arg, &hint))
				goto err;
			dict_initfrom_hint(self, hint, false);
			return 0;
		}
		return dict_initfrom_seq(self, arg);
	}	break;

	case 2: {
		int weak;
		size_t hint;
		if unlikely(DeeObject_AsSize(argv[0], &hint))
			goto err;
		weak = DeeObject_Bool(argv[1]);
		if unlikely(weak < 0)
			goto err;
		dict_initfrom_hint(self, hint, !!weak);
		return 0;
	}	break;

	default: break;
	}
	return err_invalid_argc(STR_Dict, argc, 1, 2);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_deepload(Dict *__restrict self) {
	int result = 0;
	bool must_rebuild_htab = false;
	/*virt*/ Dee_dict_vidx_t i;
	DeeDict_LockRead(self);
	for (i = Dee_dict_vidx_tovirt(0);
	     Dee_dict_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		struct Dee_dict_item *item;
		DREF DeeObject *old_item_key;
		DREF DeeObject *old_item_value;
		DREF DeeObject *new_item_key;
		DREF DeeObject *new_item_value;
		Dee_hash_t old_key_hash;
		Dee_hash_t new_key_hash;
again_deepload_at_i:
		item = &_DeeDict_GetVirtVTab(self)[i];
		old_item_key = item->di_key;
		if (!old_item_key)
			continue; /* Deleted key */
		old_item_value = item->di_value;
		Dee_Incref(old_item_key);
		Dee_Incref(old_item_value);
		old_key_hash = item->di_hash;
		DeeDict_LockEndRead(self);
		new_item_key = DeeObject_DeepCopy(old_item_key);
		Dee_Decref_unlikely(old_item_key);
		if unlikely(!new_item_key) {
			Dee_Decref_unlikely(old_item_value);
			goto err;
		}
		new_item_value = DeeObject_DeepCopy(old_item_value);
		Dee_Decref_unlikely(old_item_value);
		if unlikely(!new_item_value) {
			Dee_Decref_unlikely(new_item_key);
			goto err;
		}
		new_key_hash = DeeObject_Hash(new_item_key);
		if (old_key_hash != new_key_hash)
			must_rebuild_htab = true; /* Hash changed (must rebuild htab later on...) */
		DeeDict_LockWrite(self);
		if unlikely(item != &_DeeDict_GetVirtVTab(self)[i] ||
		            item->di_hash != old_key_hash ||
		            item->di_key != old_item_key ||
		            item->di_value != old_item_value) {
			DeeDict_LockEndWrite(self);
			Dee_Decref(new_item_value);
			Dee_Decref(new_item_key);
			DeeDict_LockRead(self);
			goto again_deepload_at_i;
		}
		item->di_hash  = new_key_hash;
		item->di_key   = new_item_key;   /* Inherit reference (x2) */
		item->di_value = new_item_value; /* Inherit reference (x2) */
		/* NOTE: It's OK if the htab is invalid (item lookup will just be
		 *       broken until "d_htab" was re-build below, but that's OK) */
		DeeDict_LockEndWrite(self);
		Dee_Decref(old_item_key);
		Dee_Decref(old_item_value);
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);

done:
	/* If necessary, rebuild "d_htab" */
	if (must_rebuild_htab) {
		DeeDict_LockWrite(self);
		dict_htab_rebuild(self);
		DeeDict_LockEndWrite(self);
	}
	return result;
err:
	result = -1;
	goto done;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_printrepr(Dict *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	/*virt*/ struct Dee_dict_item *vtab;
	/*virt*/ Dee_dict_vidx_t i;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "Dict({ ");
	if unlikely(result < 0)
		return result;
	is_first = true;
	DeeDict_LockRead(self);
	vtab = _DeeDict_GetVirtVTab(self);
	for (i = Dee_dict_vidx_tovirt(0);
	     Dee_dict_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		DREF DeeObject *key, *value;
		key = vtab[i].di_key;
		if (key == NULL)
			continue;
		value = vtab[i].di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		/* Print this key/value pair. */
		temp = DeeFormat_Printf(printer, arg, "%s%r: %r", is_first ? "" : ", ", key, value);
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		is_first = false;
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);
	temp = is_first ? DeeFormat_PRINT(printer, arg, "})")
	                : DeeFormat_PRINT(printer, arg, " })");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	return result;
err_temp:
	return temp;
}

PRIVATE NONNULL((1)) int DCALL
dict_mh_clear(Dict *__restrict self) {
	size_t i, vsize;
	struct Dee_dict_item *vtab;
	DeeDict_LockWrite(self);
	if (!self->d_vsize) {
		DeeDict_LockEndWrite(self);
		return 0;
	}
	vtab  = _DeeDict_GetRealVTab(self);
	vsize = self->d_vsize;
	self->d_valloc  = 0;
	self->d_vsize   = 0;
	self->d_vused   = 0;
	_DeeDict_SetVirtVTab(self, DeeDict_EmptyVTab);
	self->d_hmask   = 0;
	self->d_hidxget = &Dee_dict_gethidx8;
	self->d_hidxset = &Dee_dict_sethidx8;
	self->d_htab    = DeeDict_EmptyHTab;
	DeeDict_LockEndWrite(self);
	ASSERTF(vtab != (struct Dee_dict_item *)_DeeDict_EmptyTab,
	        "Should have been handled by '!self->d_vsize' above");
	for (i = 0; i < vsize; ++i) {
		if (vtab[i].di_key) {
			Dee_Decref(vtab[i].di_key);
			Dee_Decref(vtab[i].di_value);
		}
	}
	_DeeDict_TabsFree(vtab);
	return 0;
}

#ifdef DCALL_RETURN_COMMON
#define dict_clear (*(void (DCALL *)(Dict *__restrict))&dict_mh_clear)
#else /* DCALL_RETURN_COMMON */
PRIVATE NONNULL((1)) void DCALL
dict_clear(Dict *__restrict self) {
	dict_mh_clear(self);
}
#endif /* !DCALL_RETURN_COMMON */

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bool(Dict *__restrict self) {
	return DeeDict_SIZE_ATOMIC(self) != 0;
}

#define dict_size_fast dict_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
dict_size(Dict *__restrict self) {
	return DeeDict_SIZE_ATOMIC(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DictIterator *DCALL
dict_iter(Dict *__restrict self) {
	DREF DictIterator *result = DeeObject_MALLOC(DictIterator);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->di_dict = self;
	result->di_vidx = Dee_dict_vidx_tovirt(0);
	DeeObject_Init(result, &DictIterator_Type);
	return result;
err:
	return NULL;
}



#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_sizeob(Dict *__restrict self) {
	size_t result = DeeDict_SIZE_ATOMIC(self);
	return DeeInt_NewSize(result);
}
#endif /* !__OPTIMIZE_SIZE__ */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_foreach_pair(Dict *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_dict_vidx_t i;
	Dee_ssize_t temp, result = 0;
	DeeDict_LockRead(self);
	for (i = Dee_dict_vidx_tovirt(0);
	     Dee_dict_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		DREF DeeObject *key, *value;
		struct Dee_dict_item *item;
		item = &_DeeDict_GetVirtVTab(self)[i];
		key  = item->di_key;
		if unlikely(!key)
			continue; /* Deleted, and not-yet-optimized key */
		Dee_Incref(key);
		value = item->di_value;
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		temp = (*cb)(arg, key, value);
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_enumerate_index(Dict *__restrict self, Dee_enumerate_index_t cb,
                     void *arg, size_t start, size_t end) {
	Dee_dict_vidx_t i;
	Dee_ssize_t temp, result = 0;
	DeeDict_LockRead(self);
	for (i = Dee_dict_vidx_tovirt(0);
	     Dee_dict_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		DREF DeeObject *key, *value;
		struct Dee_dict_item *item;
		size_t index, key_index;
		item  = &_DeeDict_GetVirtVTab(self)[i];
		index = item->di_hash;

		/* Integer hashes are equal to the integer itself, so
		 * if the hash doesn't fall into the start...end range,
		 * then we know it shouldn't be enumerated! */
		if (index < start)
			continue;
		if (index >= end)
			continue;
		key = item->di_key;
		if unlikely(!key)
			continue; /* Deleted, and not-yet-optimized key */
		if likely(DeeInt_Check(key)) {
			if (!DeeInt_TryAsSize(key, &key_index))
				continue; /* Hash only matched due to overflow... */
			ASSERTF(key_index == index, "Huh? Is the hash incorrect?");
			value = item->di_value;
			Dee_Incref(value);
			DeeDict_LockEndRead(self);
		} else {
			bool ok;
			Dee_Incref(key);
			value = item->di_value;
			Dee_Incref(value);
			DeeDict_LockEndRead(self);
			key = DeeObject_IntInherited(key);
			if unlikely(!key)
				goto err;
			ok = DeeInt_TryAsSize(key, &key_index) &&
			     key_index == index;
			Dee_Decref(key);
			if (!ok)
				continue; /* Hash only matched due to overflow... */
		}
		temp = (*cb)(arg, key_index, value);
		Dee_Decref_unlikely(value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);
	return result;
err_temp:
	return temp;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_mh_popitem(Dict *__restrict self) {
	DREF DeeTupleObject *result;
	struct Dee_dict_item *item;
	/* Allocate a tuple which we're going to fill with some key-value pair. */
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err;
	DeeDict_LockWrite(self);
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		DeeTuple_FreeUninitialized(result);
		return_none;
	}
	item = _DeeDict_GetRealVTab(self) + self->d_vsize - 1;
	while (!item->di_key) {
		ASSERT(_DeeDict_CanOptimizeVTab(self));
		ASSERT(item > _DeeDict_GetRealVTab(self));
		--item;
	}
	result->t_elem[0] = item->di_key;   /* Inherit reference. */
	result->t_elem[1] = item->di_value; /* Inherit reference. */
	item->di_key = NULL;
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	--self->d_vused;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
dict_mh_seq_getitem_index_impl(Dict *__restrict self, size_t index, bool tryget) {
	struct Dee_dict_item *item;
	DREF DeeTupleObject *result;
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err;
	DeeDict_LockReadAndOptimize(self);
	if unlikely(index >= self->d_vused) {
		size_t real_size = self->d_vused;
		DeeDict_LockEndRead(self);
		DeeTuple_FreeUninitialized(result);
		if (tryget)
			return (DREF DeeTupleObject *)ITER_DONE;
		err_index_out_of_bounds((DeeObject *)self, index, real_size);
		goto err;
	}
	item = &_DeeDict_GetRealVTab(self)[index];
	result->t_elem[0] = item->di_key;
	result->t_elem[1] = item->di_value;
	Dee_Incref(result->t_elem[0]);
	Dee_Incref(result->t_elem[1]);
	DeeDict_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_mh_seq_delitem_index(Dict *__restrict self, size_t index) {
	struct Dee_dict_item *item;
	DREF DeeObject *old_key, *old_value;
	DeeDict_LockWrite(self);
	if unlikely(index >= self->d_vused) {
		size_t real_size = self->d_vused;
		DeeDict_LockEndWrite(self);
		err_index_out_of_bounds((DeeObject *)self, index, real_size);
		goto err;
	}
	if (_DeeDict_CanOptimizeVTab(self))
		dict_optimize_vtab(self);
	item = &_DeeDict_GetRealVTab(self)[index];
	old_key   = item->di_key;
	old_value = item->di_value;
	item->di_key = NULL; /* Delete item */
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	--self->d_vused;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	Dee_Decref(old_key);
	Dee_Decref(old_value);
	return 0;
err:
	return -1;
}

/* Given an "index" in range `[0,d_used)', return a value
 * in range `[0,d_size)' that points to the index'th non-
 * deleted key in "d_vtab" */
PRIVATE WUNUSED NONNULL((1)) /*real*/ Dee_dict_vidx_t DCALL
dict_unoptimize_vtab_index(Dict *self, size_t index) {
	size_t result;
	ASSERT(self->d_vused <= self->d_vsize);
	ASSERT(index <= self->d_vused);
	if (self->d_vused == self->d_vsize)
		return index; /* Dict is fully optimized -> no translation needed */
	if (index <= 0)
		return 0;
	if (index >= self->d_vused)
		return self->d_vsize;
	for (result = 0; index; ++result) {
		ASSERT(result <= self->d_vsize);
		if (_DeeDict_GetRealVTab(self)[result].di_key)
			--index;
	}
	return result;
}

struct dict_mh_seq_setitem_index_impl_data {
	size_t          dsqsii_index;         /* in */
	DREF DeeObject *dsqsii_deleted_key;   /* out[1..1] */
	DREF DeeObject *dsqsii_deleted_value; /* out[1..1] */
};

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_dict_vidx_t DCALL
dict_mh_seq_setitem_index_impl_cb(void *arg, Dict *self,
                                  /*virt*/ Dee_dict_vidx_t overwrite_index,
                                  DeeObject **p_value) {
	/*real*/ Dee_dict_vidx_t result;
	struct dict_mh_seq_setitem_index_impl_data *data;
	struct Dee_dict_item *real_vtab;
	(void)p_value;
	data = (struct dict_mh_seq_setitem_index_impl_data *)arg;
	if unlikely(data->dsqsii_index >= self->d_vused) {
		size_t used = self->d_vused;
		DeeDict_LockEndWrite(self);
		err_index_out_of_bounds((DeeObject *)self, data->dsqsii_index, used);
		return Dee_DICT_HTAB_EOF;
	}
	result = dict_unoptimize_vtab_index(self, data->dsqsii_index);
	real_vtab = _DeeDict_GetRealVTab(self);
	for (;;) {
		ASSERT(result < self->d_vsize);
		if (real_vtab[result].di_key)
			break;
		++result;
	}
	ASSERT(real_vtab[result].di_key);
	ASSERT(real_vtab[result].di_value);
	if (result != Dee_dict_vidx_toreal(overwrite_index)) {
		/* Delete item that used to exist at this index (sequence setitem acts as an overwrite operation) */
		data->dsqsii_deleted_key   = real_vtab[result].di_key;
		data->dsqsii_deleted_value = real_vtab[result].di_value;
		real_vtab[result].di_key   = NULL;
		DBG_memset(&real_vtab[result].di_value, 0xcc, sizeof(real_vtab[result].di_value));
		--self->d_vused;
	} else {
		/* for xchitem_index... */
		data->dsqsii_deleted_key   = real_vtab[result].di_key;
		data->dsqsii_deleted_value = real_vtab[result].di_value;
		Dee_Incref(data->dsqsii_deleted_key);
		Dee_Incref(data->dsqsii_deleted_value);
	}
	return Dee_dict_vidx_tovirt(result);
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
dict_mh_seq_setitem_index_impl(Dict *self, size_t index,
                               DeeObject *key, DeeObject *value) {
	int result;
	struct dict_mh_seq_setitem_index_impl_data data;
	data.dsqsii_index = index;
	data.dsqsii_deleted_key = NULL;
	result = dict_setitem_at(self, key, value, &dict_mh_seq_setitem_index_impl_cb, &data);
	if (data.dsqsii_deleted_key) {
		Dee_Decref(data.dsqsii_deleted_key);
		Dee_Decref(data.dsqsii_deleted_value);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) DREF DeeTupleObject *DCALL
dict_mh_seq_xchitem_index_impl(Dict *self, size_t index,
                               DeeObject *key, DeeObject *value) {
	DREF DeeTupleObject *result;
	struct dict_mh_seq_setitem_index_impl_data data;
	data.dsqsii_index = index;
	data.dsqsii_deleted_key = NULL;
	if unlikely(dict_setitem_at(self, key, value, &dict_mh_seq_setitem_index_impl_cb, &data)) {
		if (data.dsqsii_deleted_key)
			goto err_kv;
		goto err;
	}
	ASSERT(data.dsqsii_deleted_key);
	ASSERT(data.dsqsii_deleted_value);
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_kv;
	result->t_elem[0] = data.dsqsii_deleted_key;   /* Inherit reference */
	result->t_elem[1] = data.dsqsii_deleted_value; /* Inherit reference */
	return result;
err_kv:
	Dee_Decref(data.dsqsii_deleted_key);
	Dee_Decref(data.dsqsii_deleted_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_dict_vidx_t DCALL
dict_mh_seq_insert_impl_cb(void *arg, Dict *self,
                           /*virt*/ Dee_dict_vidx_t overwrite_index,
                           DeeObject **p_value) {
	/*real*/ Dee_dict_vidx_t result;
	(void)overwrite_index;
	(void)p_value;
	result = (/*real*/ Dee_dict_vidx_t)(size_t)(uintptr_t)arg;
	if unlikely(result >= self->d_vused) {
		size_t used = self->d_vused;
		DeeDict_LockEndWrite(self);
		err_index_out_of_bounds((DeeObject *)self, result, used);
		return Dee_DICT_HTAB_EOF;
	}
	result = dict_unoptimize_vtab_index(self, result);
	return Dee_dict_vidx_tovirt(result);
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
dict_mh_seq_insert_impl(Dict *self, size_t index, DeeObject *key, DeeObject *value) {
	return dict_setitem_at(self, key, value, &dict_mh_seq_insert_impl_cb, (void *)(uintptr_t)index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
dict_mh_seq_getitem_index(Dict *__restrict self, size_t index) {
	return dict_mh_seq_getitem_index_impl(self, index, false);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
dict_mh_seq_trygetitem_index(Dict *__restrict self, size_t index) {
	return dict_mh_seq_getitem_index_impl(self, index, true);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
dict_mh_seq_setitem_index(Dict *self, size_t index, DeeObject *key_and_value_tuple) {
	int result;
	DREF DeeObject *key_and_value[2];
	if (DeeObject_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_mh_seq_setitem_index_impl(self, index, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeTupleObject *DCALL
dict_mh_seq_xchitem_index(Dict *self, size_t index, DeeObject *key_and_value_tuple) {
	DREF DeeTupleObject *result;
	DREF DeeObject *key_and_value[2];
	if (DeeObject_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_mh_seq_xchitem_index_impl(self, index, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
dict_mh_seq_insert(Dict *self, size_t index, DeeObject *key_and_value_tuple) {
	int result;
	DREF DeeObject *key_and_value[2];
	if (DeeObject_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_mh_seq_insert_impl(self, index, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_seq_pushfront(Dict *self, DeeObject *key_and_value_tuple) {
	return dict_mh_seq_insert(self, 0, key_and_value_tuple);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_seq_append(Dict *self, DeeObject *key_and_value_tuple) {
	int result;
	DREF DeeObject *key_and_value[2];
	if (DeeObject_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_setitem(self, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return -1;
}

struct dict_seq_erase_data {
	size_t dse_start;
	size_t dse_end;
	size_t dse_count;
};

PRIVATE NONNULL((1, 2)) bool DCALL
dict_seq_erase_data_init(struct dict_seq_erase_data *__restrict self,
                         Dict *__restrict dict, size_t start, size_t count) {
	self->dse_start = start;
	if (self->dse_start > dict->d_vused)
		self->dse_start = dict->d_vused;
	if (OVERFLOW_UADD(self->dse_start, count, &self->dse_end))
		self->dse_end = (size_t)-1;
	if (self->dse_end > dict->d_vused)
		self->dse_end = dict->d_vused;
	return OVERFLOW_USUB(self->dse_end, self->dse_start, &self->dse_count);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_mh_seq_erase(Dict *__restrict self, size_t start, size_t count) {
	size_t i;
	DREF DeeObject **old_objv;
	struct dict_seq_erase_data used;
	struct Dee_dict_item *vtab;
again:
	DeeDict_LockWrite(self);
	if unlikely(dict_seq_erase_data_init(&used, self, start, count)) {
		DeeDict_LockEndWrite(self);
		return 0;
	}
	old_objv = (DREF DeeObject **)Dee_TryMallocac(used.dse_count, sizeof(DREF DeeObject *));
	if unlikely(!old_objv) {
		struct dict_seq_erase_data new_used;
		DeeDict_LockEndWrite(self);
		old_objv = (DREF DeeObject **)Dee_Mallocac(used.dse_count, sizeof(DREF DeeObject *));
		if unlikely(!old_objv)
			goto err;
		DeeDict_LockWrite(self);
		if unlikely(dict_seq_erase_data_init(&new_used, self, start, count)) {
			DeeDict_LockEndWrite(self);
			Dee_Freea(old_objv);
			return 0;
		}
		if unlikely(new_used.dse_count != used.dse_count) {
			DeeDict_LockEndWrite(self);
			Dee_Freea(old_objv);
			goto again;
		}
		used.dse_start = new_used.dse_start;
		used.dse_end   = new_used.dse_end;
	}

	/* Optimize dict to ensure linear ordering. */
	if (_DeeDict_CanOptimizeVTab(self))
		dict_optimize_vtab(self);

	/* Copy references and delete items. */
	vtab = _DeeDict_GetRealVTab(self);
	ASSERT(self->d_vused >= used.dse_count);
	for (i = used.dse_start; i < used.dse_end; ++i) {
		struct Dee_dict_item *item = &vtab[i];
		ASSERTF(item->di_key, "Nothing should be deleted because we're optimized!");
		old_objv[(i * 2) + 0] = item->di_key;   /* Inherit reference */
		old_objv[(i * 2) + 1] = item->di_value; /* Inherit reference */
		item->di_key = NULL;
		DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	}
	self->d_vused -= used.dse_count;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	Dee_Decrefv(old_objv, used.dse_count * 2);
	Dee_Freea(old_objv);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
dict_mh_seq_pop(Dict *self, Dee_ssize_t index) {
	DREF DeeTupleObject *result;
	struct Dee_dict_item *item;
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err;
	DeeDict_LockWrite(self);
	if (index < 0)
		index += self->d_vused;
	if unlikely((size_t)index >= self->d_vused) {
		size_t real_size = self->d_vused;
		DeeDict_LockEndWrite(self);
		err_index_out_of_bounds((DeeObject *)self, (size_t)index, real_size);
		goto err_r;
	}
	if (_DeeDict_CanOptimizeVTab(self))
		dict_optimize_vtab(self);
	item = &_DeeDict_GetRealVTab(self)[index];
	result->t_elem[0] = item->di_key;   /* Inherit reference */
	result->t_elem[1] = item->di_value; /* Inherit reference */
	item->di_key = NULL; /* Delete item */
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	--self->d_vused;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
dict_mh_seq_removeif(Dict *self, DeeObject *should, size_t start, size_t end, size_t max) {
	size_t i, result = 0;
	DeeDict_LockWrite(self);
	if (end > self->d_vused)
		end = self->d_vused;
	for (i = start; i < end && result < max; ++i) {
		struct Dee_dict_item *item;
		DREF DeeTupleObject *item_key_and_value;
		DREF DeeObject *item_key, *item_value;
		DREF DeeObject *should_result_ob;
		int should_result;
again_index_i:
		if (_DeeDict_CanOptimizeVTab(self))
			dict_optimize_vtab(self);
		item = &_DeeDict_GetRealVTab(self)[i];
		item_key = item->di_key;
		Dee_Incref(item_key);
		item_value = item->di_value;
		Dee_Incref(item_value);
		DeeDict_LockEndWrite(self);
		if (DeeThread_CheckInterrupt())
			goto err_item_key_and_value;
		item_key_and_value = DeeTuple_NewUninitialized(2);
		if unlikely(!item_key_and_value) {
err_item_key_and_value:
			Dee_Decref_unlikely(item_key);
			Dee_Decref_unlikely(item_value);
			goto err;
		}
		item_key_and_value->t_elem[0] = item_key;   /* Inherit reference */
		item_key_and_value->t_elem[1] = item_value; /* Inherit reference */
		should_result_ob = DeeObject_Call(should, 1, (DeeObject *const *)&item_key_and_value);
		Dee_Decref_likely(item_key_and_value);
		if unlikely(!should_result_ob)
			goto err;
		should_result = DeeObject_BoolInherited(should_result_ob);
		if unlikely(should_result < 0)
			goto err;
		DeeDict_LockWrite(self);
		if unlikely(item != &_DeeDict_GetRealVTab(self)[i])
			goto again_index_i;
		if unlikely(item->di_key != item_key)
			goto again_index_i;
		if unlikely(item->di_value != item_value)
			goto again_index_i;
		if (should_result) {
			item->di_key = NULL; /* Delete item */
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
			--self->d_vused;
			dict_autoshrink(self);
			++result;
		}
	}
	DeeDict_LockEndWrite(self);
	return result;
err:
	return (size_t)-1;
}

PRIVATE NONNULL((1)) void DCALL
dict_items_reverse(struct Dee_dict_item *items, size_t count) {
	struct Dee_dict_item *lo = items;
	struct Dee_dict_item *hi = items + count;
	while (lo < hi) {
		struct Dee_dict_item temp;
		--hi;
		memcpy(&temp, lo, sizeof(struct Dee_dict_item));
		memcpy(lo, hi, sizeof(struct Dee_dict_item));
		memcpy(hi, &temp, sizeof(struct Dee_dict_item));
		++lo;
	}
}

LOCAL NONNULL((1)) void DCALL
dict_htab_reverse8(Dict *__restrict self, uint8_t vmin, uint8_t vmax) {
	size_t i;
	uint8_t *htab = (uint8_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		uint8_t idx = htab[i];
		if (idx >= vmin && idx <= vmax)
			htab[i] = vmin + vmax - idx;
	}
}

#if DEE_DICT_HIDXIO_COUNT >= 2
LOCAL NONNULL((1)) void DCALL
dict_htab_reverse16(Dict *__restrict self, uint16_t vmin, uint16_t vmax) {
	size_t i;
	uint16_t *htab = (uint16_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		uint16_t idx = htab[i];
		if (idx >= vmin && idx <= vmax)
			htab[i] = vmin + vmax - idx;
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */

#if DEE_DICT_HIDXIO_COUNT >= 3
LOCAL NONNULL((1)) void DCALL
dict_htab_reverse32(Dict *__restrict self, uint32_t vmin, uint32_t vmax) {
	size_t i;
	uint32_t *htab = (uint32_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		uint32_t idx = htab[i];
		if (idx >= vmin && idx <= vmax)
			htab[i] = vmin + vmax - idx;
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */

#if DEE_DICT_HIDXIO_COUNT >= 4
LOCAL NONNULL((1)) void DCALL
dict_htab_reverse64(Dict *__restrict self, uint64_t vmin, uint64_t vmax) {
	size_t i;
	uint64_t *htab = (uint64_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		uint64_t idx = htab[i];
		if (idx >= vmin && idx <= vmax)
			htab[i] = vmin + vmax - idx;
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */

PRIVATE NONNULL((1)) void DCALL
dict_htab_reverse(Dict *__restrict self, /*virt*/Dee_dict_vidx_t vmin, /*virt*/Dee_dict_vidx_t vmax) {
	if (DEE_DICT_HIDXIO_IS8(self->d_valloc)) {
		dict_htab_reverse8(self, (uint8_t)vmin, (uint8_t)vmax);
	} else
#if DEE_DICT_HIDXIO_COUNT >= 2
	if (DEE_DICT_HIDXIO_IS16(self->d_valloc)) {
		dict_htab_reverse16(self, (uint16_t)vmin, (uint16_t)vmax);
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */
#if DEE_DICT_HIDXIO_COUNT >= 3
	if (DEE_DICT_HIDXIO_IS32(self->d_valloc)) {
		dict_htab_reverse32(self, (uint32_t)vmin, (uint32_t)vmax);
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */
#if DEE_DICT_HIDXIO_COUNT >= 4
	if (DEE_DICT_HIDXIO_IS64(self->d_valloc)) {
		dict_htab_reverse64(self, (uint64_t)vmin, (uint64_t)vmax);
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */
	{
		__builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_mh_seq_reverse(Dict *self, size_t start, size_t end) {
	DeeDict_LockWrite(self);
	if (end > self->d_vused)
		end = self->d_vused;
	if (start < end) {
		/*virt*/Dee_dict_vidx_t vstart;
		/*virt*/Dee_dict_vidx_t vend;
		if (_DeeDict_CanOptimizeVTab(self))
			dict_optimize_vtab(self);
		vstart = Dee_dict_vidx_tovirt(start);
		vend   = Dee_dict_vidx_tovirt(end);
		dict_items_reverse(_DeeDict_GetVirtVTab(self) + vstart, vend - vstart);
		/* Also reverse all index references in "htab" */
		ASSERT(vstart != Dee_DICT_HTAB_EOF);
		dict_htab_reverse(self, vstart, vend - 1);
	}
	DeeDict_LockEndWrite(self);
	return 0;
}

PRIVATE NONNULL((1)) bool DCALL
dict_shrink_impl(Dict *__restrict self, bool fully_shrink) {
	bool result = false;
	DeeDict_LockWrite(self);
	if (fully_shrink ? _DeeDict_CanShrinkVTab(self)
	                 : _DeeDict_ShouldShrinkVTab(self)) {
		dict_shrink_vtab_and_htab(self, fully_shrink);
		result = true;
	} else if (fully_shrink ? _DeeDict_CanShrinkHTab(self)
	                        : _DeeDict_ShouldShrinkHTab(self)) {
		dict_shrink_htab(self, fully_shrink);
		result = true;
	}
	DeeDict_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_shrink(Dict *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	bool result, fully = true;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__fully, "|b:shrink", &fully))
		goto err;
	result = dict_shrink_impl(self, fully);
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
dict_cc(Dict *__restrict self) {
	return dict_shrink_impl(self, true);
}


PRIVATE struct type_gc dict_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&dict_clear,
	/* .tp_pclear = */ NULL,
	/* .tp_gcprio = */ 0,
	/* .tp_cc     = */ (bool (DCALL *)(DeeObject *__restrict))&dict_cc,
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_contains(Dict *self, DeeObject *key) {
	int result = dict_hasitem(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE struct type_seq dict_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_iter,
	/* .tp_sizeob                     = */ NULL_IF_Os((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_sizeob),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&dict_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&dict_foreach_pair,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&dict_foreach_pair,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&dict_enumerate_index, /* TODO: Optimization: only need to enumerate integer keys, so we can take advantage of the fact that "hash(int) == int". */
	/* .tp_iterkeys                   = */ &DeeMap_DefaultIterKeysWithIter,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&dict_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&dict_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&dict_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&dict_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_hasitem_string_len_hash,
};


#ifndef CONFIG_NO_DEEMON_100_COMPAT
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deprecated_d100_get_maxloadfactor(DeeObject *__restrict self);
#define deprecated_d100_del_maxloadfactor (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)
#define deprecated_d100_set_maxloadfactor (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_sizeof(Dict *__restrict self) {
	size_t hmask, valloc, result;
	DeeDict_LockRead(self);
	valloc = self->d_valloc;
	hmask  = self->d_hmask;
	DeeDict_LockEndRead(self);
	result = dict_sizeoftabs_from_hmask_and_valloc(hmask, valloc);
	return DeeInt_NewSize(sizeof(Dict) + result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_nonempty_as_bound(Dict *__restrict self) {
	size_t size = DeeDict_SIZE_ATOMIC(self);
	return Dee_BOUND_FROMBOOL(size);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL dict_trygetfirst(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL dict_getfirst(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getfirstkey(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getfirstvalue(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_delfirst(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setfirst(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setfirstkey(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setfirstvalue(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL dict_trygetlast(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL dict_getlast(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getlastkey(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getlastvalue(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_dellast(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setlast(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setlastkey(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setlastvalue(Dict *self, DeeObject *value);

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_dict_first
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_firstkey
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_firstvalue
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_last
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_lastkey
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_lastvalue
#include "dict-firstlast-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict___hdxio__(Dict *__restrict self) {
	size_t valloc = atomic_read(&self->d_valloc);
	shift_t hdxio = DEE_DICT_HIDXIO_FROMALLOC(valloc);
	return DeeInt_NEWU(hdxio);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_mh_seq_foreach_reverse(Dict *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	Dee_dict_vidx_t i;
	DeeDict_LockRead(self);
	i = Dee_dict_vidx_tovirt(self->d_vsize);
	while (i > Dee_dict_vidx_tovirt(0)) {
		struct Dee_dict_item *item;
		--i;
		item = &_DeeDict_GetVirtVTab(self)[i];
		if likely(item->di_key) {
			DREF DeeTupleObject *item_pair;
			DREF DeeObject *item_key;
			DREF DeeObject *item_value;
			item_key   = item->di_key;
			item_value = item->di_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			DeeDict_LockEndRead(self);
			item_pair = DeeTuple_NewUninitialized(2);
			if unlikely(!item_pair) {
				Dee_Decref_unlikely(item_key);
				Dee_Decref_unlikely(item_value);
				goto err;
			}
			item_pair->t_elem[0] = item_key;  /* Inherit reference */
			item_pair->t_elem[1] = item_value; /* Inherit reference */
			temp = (*proc)(arg, (DeeObject *)item_pair);
			Dee_Decref_likely(item_pair);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
			DeeDict_LockRead(self);
			if unlikely(i > Dee_dict_vidx_tovirt(self->d_vsize))
				i = Dee_dict_vidx_tovirt(self->d_vsize);
		}
	}
	DeeDict_LockEndRead(self);
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_mh_seq_enumerate_index(Dict *__restrict self, Dee_enumerate_index_t proc,
                            void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	for (;;) {
		DREF DeeTupleObject *item_pair;
		DREF DeeObject *item_key;
		DREF DeeObject *item_value;
		struct Dee_dict_item *item;
		DeeDict_LockReadAndOptimize(self);
		if (end > self->d_vused)
			end = self->d_vused;
		if (start >= end)
			break;
		item = &_DeeDict_GetRealVTab(self)[start];
		ASSERTF(item->di_key, "The dict should be have been optimized above!");
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		item_pair = DeeTuple_NewUninitialized(2);
		if unlikely(!item_pair) {
			Dee_Decref_unlikely(item_key);
			Dee_Decref_unlikely(item_value);
			goto err;
		}
		item_pair->t_elem[0] = item_key;  /* Inherit reference */
		item_pair->t_elem[1] = item_value; /* Inherit reference */
		temp = (*proc)(arg, start, (DeeObject *)item_pair);
		Dee_Decref_likely(item_pair);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		++start;
	}
	DeeDict_LockEndRead(self);
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_mh_seq_enumerate_index_reverse(Dict *__restrict self, Dee_enumerate_index_t proc,
                                    void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	for (;;) {
		DREF DeeTupleObject *item_pair;
		DREF DeeObject *item_key;
		DREF DeeObject *item_value;
		struct Dee_dict_item *item;
		DeeDict_LockReadAndOptimize(self);
		if (end > self->d_vused)
			end = self->d_vused;
		if (end <= start)
			break;
		--end;
		item = &_DeeDict_GetRealVTab(self)[end];
		ASSERTF(item->di_key, "The dict should be have been optimized above!");
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		item_pair = DeeTuple_NewUninitialized(2);
		if unlikely(!item_pair) {
			Dee_Decref_unlikely(item_key);
			Dee_Decref_unlikely(item_value);
			goto err;
		}
		item_pair->t_elem[0] = item_key;  /* Inherit reference */
		item_pair->t_elem[1] = item_value; /* Inherit reference */
		temp = (*proc)(arg, end, (DeeObject *)item_pair);
		Dee_Decref_likely(item_pair);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	DeeDict_LockEndRead(self);
	return result;
err_temp:
	return temp;
err:
	return -1;
}

struct dict_fromkeys_data {
	Dict      *dfkd_dict;  /* [1..1] Dict to insert stuff into. */
	DeeObject *dfkd_value; /* [1..1] Value to insert, or callback to generate value. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
dict_fromkeys_with_value(void *arg, DeeObject *key) {
	int result;
	struct dict_fromkeys_data *data;
	data = (struct dict_fromkeys_data *)arg;
#ifdef HAVE_dict_setitem_unlocked_fast_inherited
	Dee_Incref(key);
	Dee_Incref(data->dfkd_value);
	result = dict_setitem_unlocked_fast_inherited(data->dfkd_dict, key, data->dfkd_value);
	if unlikely(result) {
		Dee_DecrefNokill(key);
		Dee_DecrefNokill(data->dfkd_value);
	}
#else /* HAVE_dict_setitem_unlocked_fast_inherited */
	result = dict_setitem_unlocked(data->dfkd_dict, key, data->dfkd_value);
#endif /* !HAVE_dict_setitem_unlocked_fast_inherited */
	ASSERT(result <= 0);
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
dict_fromkeys_with_valuefor(void *arg, DeeObject *key) {
	int result;
	DREF DeeObject *used_value;
	struct dict_fromkeys_data *data;
	data = (struct dict_fromkeys_data *)arg;
	used_value = DeeObject_Call(data->dfkd_value, 1, &key);
	if unlikely(!used_value)
		goto err;
#ifdef HAVE_dict_setitem_unlocked_fast_inherited
	Dee_Incref(key);
	result = dict_setitem_unlocked_fast_inherited(data->dfkd_dict, key, used_value);
	if unlikely(result) {
		Dee_DecrefNokill(key);
		Dee_Decref(used_value);
	}
#else /* HAVE_dict_setitem_unlocked_fast_inherited */
	result = dict_setitem_unlocked(data->dfkd_dict, key, used_value);
	Dee_Decref_unlikely(used_value);
#endif /* !HAVE_dict_setitem_unlocked_fast_inherited */
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Dict *DCALL
dict_fromkeys(DeeObject *keys, DeeObject *value, DeeObject *valuefor) {
	struct dict_fromkeys_data data;
	Dee_ssize_t foreach_status;
	size_t hint;

	/* Special optimization when "seq" is a HashSet: Duplicate its control structures */
	if (DeeHashSet_CheckExact(keys)) {
		/* TODO: do this once "HashSet" uses the same data structure as Dict. */
	}

	/* Special optimization when "seq" is a RoSet: Duplicate its control structures */
	if (DeeRoSet_Check(keys)) {
		/* TODO: do this once "RoSet" uses the same data structure as Dict. */
	}

	hint = DeeObject_SizeFast(keys);
	if likely(hint == (size_t)-1) {
		data.dfkd_dict = (DREF Dict *)DeeDict_TryNewWithHint(hint);
	} else {
		data.dfkd_dict = (DREF Dict *)DeeDict_TryNewWithWeakHint(DICT_FROMSEQ_DEFAULT_HINT);
	}
	if unlikely(!data.dfkd_dict) {
		data.dfkd_dict = (DREF Dict *)DeeDict_New();
		if unlikely(!data.dfkd_dict)
			goto err;
	}
	if unlikely(valuefor) {
		data.dfkd_value = valuefor;
		foreach_status  = DeeObject_Foreach(keys, &dict_fromkeys_with_valuefor, &data);
	} else {
		data.dfkd_value = value;
		foreach_status  = DeeObject_Foreach(keys, &dict_fromkeys_with_value, &data);
	}
	if unlikely(foreach_status < 0)
		goto err_r;
	return data.dfkd_dict;
err_r:
	Dee_DecrefDokill(data.dfkd_dict);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Dict *DCALL
dict_fromkeys_f(DeeTypeObject *__restrict UNUSED(dict),
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	DeeObject *keys, *value = Dee_None;
	DeeObject *valuefor = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__keys_value_valuefor,
	                    "o|oo:fromkeys", &keys, &value, &valuefor))
		goto err;
	return dict_fromkeys(keys, value, valuefor);
err:
	return NULL;
}


struct dict_compare_seq_foreach_data {
	Dict                   *dcsfd_lhs;   /* [1..1] lhs-dict. */
	/*real*/Dee_dict_vidx_t dcsfd_index; /* Next index into "dcsfd_lhs" to compare against. */
};
#define DICT_COMPARE_SEQ_FOREACH_ERROR    (-1)
#define DICT_COMPARE_SEQ_FOREACH_EQUAL    (0)
#define DICT_COMPARE_SEQ_FOREACH_NOTEQUAL (-2)
#define DICT_COMPARE_SEQ_FOREACH_LESS     (-2)
#define DICT_COMPARE_SEQ_FOREACH_GREATER  (-3)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
dict_compare_seq_foreach(void *arg, DeeObject *rhs_item) {
	Dict *dict;
	int cmp_result;
	struct Dee_dict_item *lhs_item;
	DREF DeeObject *lhs_key_and_value[2];
	struct dict_compare_seq_foreach_data *data;
	data = (struct dict_compare_seq_foreach_data *)arg;
	dict = data->dcsfd_lhs;
	DeeDict_LockReadAndOptimize(dict);
	if unlikely(data->dcsfd_index >= dict->d_vused) {
		DeeDict_LockEndRead(dict);
		return DICT_COMPARE_SEQ_FOREACH_LESS;
	}
	lhs_item = &_DeeDict_GetRealVTab(dict)[data->dcsfd_index];
	ASSERT(lhs_item->di_key);
	lhs_key_and_value[0] = lhs_item->di_key;
	lhs_key_and_value[1] = lhs_item->di_value;
	Dee_Incref(lhs_key_and_value[0]);
	Dee_Incref(lhs_key_and_value[1]);
	DeeDict_LockEndRead(dict);
	cmp_result = seq_docompare__lhs_vector(lhs_key_and_value, 2, rhs_item);
	Dee_Decref_unlikely(lhs_key_and_value[1]);
	Dee_Decref_unlikely(lhs_key_and_value[0]);
	if unlikely(cmp_result == Dee_COMPARE_ERR)
		goto err;
	++data->dcsfd_index;
	if (cmp_result < 0)
		return DICT_COMPARE_SEQ_FOREACH_LESS;
	if (cmp_result > 0)
		return DICT_COMPARE_SEQ_FOREACH_GREATER;
	return DICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return DICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
dict_compare_eq_seq_foreach(void *arg, DeeObject *rhs_item) {
	Dict *dict;
	int cmp_result;
	struct Dee_dict_item *lhs_item;
	DREF DeeObject *lhs_key_and_value[2];
	struct dict_compare_seq_foreach_data *data;
	DeeTypeObject *tp_rhs_item = Dee_TYPE(rhs_item);
	if ((!tp_rhs_item->tp_seq || !tp_rhs_item->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs_item))
		return DICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	data = (struct dict_compare_seq_foreach_data *)arg;
	dict = data->dcsfd_lhs;
	DeeDict_LockReadAndOptimize(dict);
	if unlikely(data->dcsfd_index >= dict->d_vused) {
		DeeDict_LockEndRead(dict);
		return DICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	}
	lhs_item = &_DeeDict_GetRealVTab(dict)[data->dcsfd_index];
	ASSERT(lhs_item->di_key);
	lhs_key_and_value[0] = lhs_item->di_key;
	lhs_key_and_value[1] = lhs_item->di_value;
	Dee_Incref(lhs_key_and_value[0]);
	Dee_Incref(lhs_key_and_value[1]);
	DeeDict_LockEndRead(dict);
	cmp_result = seq_docompareeq__lhs_vector(lhs_key_and_value, 2, rhs_item);
	Dee_Decref_unlikely(lhs_key_and_value[1]);
	Dee_Decref_unlikely(lhs_key_and_value[0]);
	if unlikely(cmp_result == Dee_COMPARE_ERR)
		goto err;
	++data->dcsfd_index;
	if (cmp_result != 0)
		return DICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	return DICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return DICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_seq_compare(Dict *lhs, DeeObject *rhs) {
	size_t lhs_size;
	Dee_ssize_t foreach_status;
	struct dict_compare_seq_foreach_data data;
	data.dcsfd_index = 0;
	data.dcsfd_lhs   = lhs;
	foreach_status   = DeeObject_Foreach(rhs, &dict_compare_seq_foreach, &data);
	if unlikely(foreach_status == DICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == DICT_COMPARE_SEQ_FOREACH_LESS)
		return -1;
	if (foreach_status == DICT_COMPARE_SEQ_FOREACH_GREATER)
		return 1;
	lhs_size = DeeDict_SIZE_ATOMIC(lhs);
	if (data.dcsfd_index < lhs_size)
		return 1;
	return 0;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_seq_compare_eq(Dict *lhs, DeeObject *rhs) {
	size_t lhs_size;
	Dee_ssize_t foreach_status;
	struct dict_compare_seq_foreach_data data;
	data.dcsfd_index = 0;
	data.dcsfd_lhs   = lhs;
	foreach_status   = DeeObject_Foreach(rhs, &dict_compare_eq_seq_foreach, &data);
	if unlikely(foreach_status == DICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == DICT_COMPARE_SEQ_FOREACH_NOTEQUAL)
		return 1;
	lhs_size = DeeDict_SIZE_ATOMIC(lhs);
	if (data.dcsfd_index < lhs_size)
		return 1;
	return 0;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_seq_trycompare_eq(Dict *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return dict_mh_seq_compare_eq(lhs, rhs);
}


PRIVATE struct type_getset tpconst dict_getsets[] = {
	TYPE_GETSET_BOUND(STR_first, &dict_getfirst, &dict_delfirst, &dict_setfirst, &dict_nonempty_as_bound, "->?T2?O?O"),
	TYPE_GETSET_BOUND(STR_last, &dict_getlast, &dict_dellast, &dict_setlast, &dict_nonempty_as_bound, "->?T2?O?O"),
	TYPE_GETSET_BOUND("firstkey", &dict_getfirstkey, &dict_delfirst, &dict_setfirstkey, &dict_nonempty_as_bound, "->?O"),
	TYPE_GETSET_BOUND("lastkey", &dict_getlastkey, &dict_dellast, &dict_setlastkey, &dict_nonempty_as_bound, "->?O"),
	TYPE_GETSET_BOUND("firstvalue", &dict_getfirstvalue, &dict_delfirst, &dict_setfirstvalue, &dict_nonempty_as_bound, "->?O"),
	TYPE_GETSET_BOUND("lastvalue", &dict_getlastvalue, &dict_dellast, &dict_setlastvalue, &dict_nonempty_as_bound, "->?O"),

	TYPE_GETTER(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER(STR_frozen, &DeeRoDict_FromSequence, "->?#Frozen"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETSET_F("max_load_factor",
	              &deprecated_d100_get_maxloadfactor,
	              &deprecated_d100_del_maxloadfactor,
	              &deprecated_d100_set_maxloadfactor,
	              METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "->?Dfloat\n"
	              "Deprecated. Always returns ${1.0}, with del/set being ignored"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETTER_F("__sizeof__", &dict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_F("__hdxio__", &dict___hdxio__, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Size shift-multipler for htab words (word size is ${1 << __hdxio__})"),
	TYPE_GETSET_END
};



PRIVATE struct type_member tpconst dict_members[] = {
	TYPE_MEMBER_FIELD_DOC("__valloc__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_valloc), "## of allocated vtab slots"),
	TYPE_MEMBER_FIELD_DOC("__vsize__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_vsize), "## of used vtab (including deleted slots)"),
	TYPE_MEMBER_FIELD_DOC("__vused__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_vused), "## of used vtab (excluding deleted slots; same as ?#op:size)"),
	TYPE_MEMBER_FIELD_DOC("__hmask__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_hmask), "Currently active hash-mask"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst dict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst dict_methods[] = {
//	TYPE_KWMETHOD("byhash", &dict_byhash, DOC_GET(map_byhash_doc)), /* TODO */
	TYPE_METHOD_HINTREF(seq_clear),
	TYPE_METHOD_HINTREF(map_pop),
	TYPE_METHOD_HINTREF(map_setold_ex),
	TYPE_METHOD_HINTREF(map_setnew_ex),
	TYPE_METHOD_HINTREF(map_setdefault),
	TYPE_METHOD_HINTREF(map_popitem),
	TYPE_METHOD_HINTREF(map_remove),

	TYPE_KWMETHOD("shrink", &dict_shrink,
	              "(fully=!t)->?Dbool\n"
	              "#pfully{when !f, the same sort of shrinking as done automatically as items are removed. "
	              /*   */ "When !t, force deallocation of #Iall unused items, even if that ruins hash "
	              /*   */ "characteristics / memory efficiency}\n"
	              "#r{Returns !t if memory was released}"
	              "Release unused memory"),

	TYPE_METHOD_HINTREF_DOC(explicit_seq_xchitem, "(index:?Dint,item:?T2?O?O)->?T2?O?O"),
	TYPE_METHOD_HINTREF(explicit_seq_erase),
	TYPE_METHOD_HINTREF_DOC(explicit_seq_insert, "(index:?Dint,item:?T2?O?O)"),
	TYPE_METHOD_HINTREF_DOC(explicit_seq_append, "(item:?T2?O?O)"),
	TYPE_METHOD_HINTREF_DOC(explicit_seq_pushfront, "(item:?T2?O?O)"),
	TYPE_METHOD_HINTREF_DOC(explicit_seq_pop, "(index=!-1)->?T2?O?O"),
	TYPE_METHOD_HINTREF(explicit_seq_removeif),
	TYPE_METHOD_HINTREF(explicit_seq_reverse),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst dict_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_clear, &dict_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setold_ex, &dict_mh_setold_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setnew_ex, &dict_mh_setnew_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setdefault, &dict_mh_setdefault, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop, &dict_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop_with_default, &dict_mh_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_popitem, &dict_mh_popitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_remove, &dict_mh_remove, METHOD_FNOREFESCAPE),

	/* Operators for "Dict as Sequence" (specifically defined because dicts are ordered) */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &dict_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &dict_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &dict_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &dict_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_foreach_pair, &dict_foreach_pair, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_enumerate_index, &dict_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_size, &dict_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_size_fast, &dict_size_fast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_getitem_index, &dict_mh_seq_getitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trygetitem_index, &dict_mh_seq_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_delitem_index, &dict_mh_seq_delitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_setitem_index, &dict_mh_seq_setitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare_eq, &dict_mh_seq_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare, &dict_mh_seq_compare, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trycompare_eq, &dict_mh_seq_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_xchitem_index, &dict_mh_seq_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_erase, &dict_mh_seq_erase, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insert, &dict_mh_seq_insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pushfront, &dict_mh_seq_pushfront, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_append, &dict_mh_seq_append, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pop, &dict_mh_seq_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeif, &dict_mh_seq_removeif, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reverse, &dict_mh_seq_reverse, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sort, &dict_mh_sort, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sort_with_key, &dict_mh_sort_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method tpconst dict_class_methods[] = {
	TYPE_KWMETHOD_F("fromkeys", &dict_fromkeys_f, METHOD_FNOREFESCAPE,
	                "(keys:?S?O,value=!n,valuefor?:DCallable)->?.\n"
	                "Construct a new ?. from @keys, and @value (or ${valuefor(key)}) as value."),
	TYPE_METHOD_END
};

PRIVATE struct type_operator const dict_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0004_ASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0005_MOVEASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0033_DELITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0034_SETITEM, METHOD_FNOREFESCAPE),
};

/* The main `Dict' container class */
PUBLIC DeeTypeObject DeeDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Dict),
	/* .tp_doc      = */ DOC("The builtin ?DMapping object for translating keys to items: ${{Key: Value}}\n"
	                         "Dicts also retain the order in which items are inserted, such that during "
	                         /**/ "enumeration, key-value pairs (aka. items) are enumerated from least-recently, "
	                         /**/ "to most-recently inserted.\n"
	                         "In order to easier control the order of items, certain ?DSequence functions are "
	                         /**/ "also implemented, such as ?#__seq_insert__ or ?#__seq_erase__.\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(hint:?Dint,weak=!f)\n"
	                         "#pweak{When true, @hint represents a lower-bound guess for how many items to allocate. "
	                         /*  */ "In this case, then runtime may allocate much more space than needed for @items if "
	                         /*  */ "doing so would be appropriate for the hash-mask appropriate for @hint}"
	                         "Create a new dict, while trying to pre-alloc enough space for @hint items. When @hint is "
	                         /**/ "too large to pre-allocate a buffer of sufficient size, a smaller buffer, or no buffer "
	                         /**/ "at all may be pre-allocated.\n"
	                         "\n"

	                         "(items:?S?T2?O?O)\n"
	                         "Create a new ?., using key-value pairs extracted from @items.\n"
	                         "Iterate @items and unpack each element into 2 others, using them "
	                         /**/ "as key and value to insert into @this ?.\n"
	                         "\n"

	                         "iter->\n"
	                         "Enumerate key-value pairs stored in the ?.\n"
	                         "\n"

	                         "size->\n"
	                         "Return the number of key-value pairs\n"
	                         "\n"

	                         "contains(key)->\n"
	                         "Check if the dict contains a @key\n"
	                         "\n"

	                         "[](key)->\n"
	                         "#tKeyError{Given @key doesn't exist}\n"
	                         "Return the value associated with @key\n"
	                         "\n"

	                         "del[](key)->\n"
	                         "Remove @key from @this. No-op if @key doesn't exist (s.a. ?#remove)\n"
	                         "\n"

	                         "[]=(key,value)->\n"
	                         "Insert/override @key by assigning @value"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(Dict),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&dict_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&dict_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&dict_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&dict_init,
				TYPE_FIXED_ALLOCATOR_GC(Dict)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dict_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_moveassign,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&dict_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&dict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&dict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dict_visit,
	/* .tp_gc            = */ &dict_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ &dict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ dict_methods,
	/* .tp_getsets       = */ dict_getsets,
	/* .tp_members       = */ dict_members,
	/* .tp_class_methods = */ dict_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_class_members,
	/* .tp_method_hints  = */ dict_method_hints,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ dict_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(dict_operators)
};

#else /* CONFIG_EXPERIMENTAL_ORDERED_DICTS */

/* A dummy object used by Dict and HashSet to refer to deleted
 * keys that are still apart of the hash chain.
 * DO NOT EXPOSE THIS OBJECT TO USER-CODE! */
PUBLIC DeeObject DeeDict_Dummy = {
	OBJECT_HEAD_INIT(&DeeObject_Type)
};
#define dummy (&DeeDict_Dummy)

#define empty_dict_items ((struct Dee_dict_item *)DeeDict_EmptyItems)
PUBLIC_CONST struct Dee_dict_item const DeeDict_EmptyItems[1] = {
	{ NULL, NULL, 0 }
};



/************************************************************************/
/* ITERATOR                                                             */
/************************************************************************/
typedef struct {
	OBJECT_HEAD
	DREF DeeDictObject *di_dict; /* [1..1][const] The Dict being iterated. */
	struct dict_item   *di_next; /* [?..1][MAYBE(in(di_dict->d_elem))][lock(ATOMIC)]
	                              * The first candidate for the next item.
	                              * NOTE: Before being dereferenced, this pointer is checked
	                              *       for being located inside the Dict's element vector.
	                              *       In the event that it is located at its end, `ITER_DONE'
	                              *       is returned, though in the event that it is located
	                              *       outside, an error is thrown (`err_changed_sequence()'). */
} DictIterator;
#define READ_ITEM(x) atomic_read(&(x)->di_next)
INTDEF DeeTypeObject DictIterator_Type;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dictiterator_nextkey(DictIterator *__restrict self) {
	DREF DeeObject *result;
	struct dict_item *item, *end;
	DeeDictObject *dict = self->di_dict;
	DeeDict_LockRead(dict);
	end = dict->d_elem + (dict->d_mask + 1);
	for (;;) {
		struct dict_item *old_item;
		item     = atomic_read(&self->di_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < dict->d_elem)
			goto dict_has_changed;

		/* Search for the next non-empty item. */
		while (item < end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->di_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->di_next, old_item, item + 1))
			break;
	}
	result = item->di_key;
	Dee_Incref(result);
	DeeDict_LockEndRead(dict);
	return result;
dict_has_changed:
	DeeDict_LockEndRead(dict);
	err_changed_sequence((DeeObject *)dict);
	return NULL;
iter_exhausted:
	DeeDict_LockEndRead(dict);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dictiterator_nextvalue(DictIterator *__restrict self) {
	DREF DeeObject *result;
	struct dict_item *item, *end;
	DeeDictObject *dict = self->di_dict;
	DeeDict_LockRead(dict);
	end = dict->d_elem + (dict->d_mask + 1);
	for (;;) {
		struct dict_item *old_item;
		item     = atomic_read(&self->di_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < dict->d_elem)
			goto dict_has_changed;

		/* Search for the next non-empty item. */
		while (item < end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->di_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->di_next, old_item, item + 1))
			break;
	}
	result = item->di_value;
	Dee_Incref(result);
	DeeDict_LockEndRead(dict);
	return result;
dict_has_changed:
	DeeDict_LockEndRead(dict);
	err_changed_sequence((DeeObject *)dict);
	return NULL;
iter_exhausted:
	DeeDict_LockEndRead(dict);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dictiterator_nextpair(DictIterator *__restrict self,
                      DREF DeeObject *key_and_value[2]) {
	struct dict_item *item, *end;
	DeeDictObject *dict = self->di_dict;
	DeeDict_LockRead(dict);
	end = dict->d_elem + (dict->d_mask + 1);
	for (;;) {
		struct dict_item *old_item;
		item     = atomic_read(&self->di_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < dict->d_elem)
			goto dict_has_changed;

		/* Search for the next non-empty item. */
		while (item < end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->di_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->di_next, old_item, item + 1))
			break;
	}
	key_and_value[0] = item->di_key;
	key_and_value[1] = item->di_value;
	Dee_Incref(key_and_value[0]);
	Dee_Incref(key_and_value[1]);
	DeeDict_LockEndRead(dict);
	return 0;
dict_has_changed:
	DeeDict_LockEndRead(dict);
	return err_changed_sequence((DeeObject *)dict);
iter_exhausted:
	DeeDict_LockEndRead(dict);
	return 1;
}

PRIVATE struct type_iterator dictiterator_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&dictiterator_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dictiterator_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dictiterator_nextvalue,
	/* .tp_advance   = */ NULL, // TODO: (size_t (DCALL *)(DeeObject *__restrict, size_t))&dictiterator_advance,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
dictiterator_bool(DictIterator *__restrict self) {
	struct dict_item *item, *end;
	DeeDictObject *dict = self->di_dict;
	DeeDict_LockRead(dict);
	end  = dict->d_elem + (dict->d_mask + 1);
	item = atomic_read(&self->di_next);
	/* Validate that the pointer is still located in-bounds. */
	if (item >= end) {
		if unlikely(item > end)
			goto dict_has_changed;
		goto iter_exhausted;
	}
	if unlikely(item < dict->d_elem)
		goto dict_has_changed;
	/* Search for the next non-empty item. */
	while (item < end && (!item->di_key || item->di_key == dummy))
		++item;
	if (item == end)
		goto iter_exhausted;
	DeeDict_LockEndRead(dict);
	return 1;
dict_has_changed:
	DeeDict_LockEndRead(dict);
	return err_changed_sequence((DeeObject *)dict);
iter_exhausted:
	DeeDict_LockEndRead(dict);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dictiterator_ctor(DictIterator *__restrict self) {
	self->di_dict = (DeeDictObject *)DeeDict_New();
	if unlikely(!self->di_dict)
		goto err;
	self->di_next = self->di_dict->d_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dictiterator_copy(DictIterator *__restrict self,
                  DictIterator *__restrict other) {
	self->di_dict = other->di_dict;
	Dee_Incref(self->di_dict);
	self->di_next = READ_ITEM(other);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dictiterator_init(DictIterator *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	DeeDictObject *dict;
	if (DeeArg_Unpack(argc, argv, "o:_DictIterator", &dict))
		goto err;
	if (DeeObject_AssertType(dict, &DeeDict_Type))
		goto err;
	self->di_dict = dict;
	Dee_Incref(dict);
	self->di_next = atomic_read(&dict->d_elem);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(DictIterator, di_dict) == offsetof(ProxyObject, po_obj));
#define dictiterator_fini  generic_proxy_fini
#define dictiterator_visit generic_proxy_visit

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
dictiterator_hash(DictIterator *self) {
	return Dee_HashPointer(READ_ITEM(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dictiterator_compare(DictIterator *self, DictIterator *other) {
	if (DeeObject_AssertType(other, &DictIterator_Type))
		goto err;
	Dee_return_compareT(struct dict_item *, READ_ITEM(self),
	                    /*               */ READ_ITEM(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp dictiterator_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&dictiterator_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dictiterator_compare,
};


PRIVATE struct type_member tpconst dict_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DictIterator, di_dict), "->?DDict"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(dict:?DDict)\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&dictiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&dictiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&dictiterator_init,
				TYPE_FIXED_ALLOCATOR(DictIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&dictiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: bi-directional iterator support */
	/* .tp_cmp           = */ &dictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &dictiterator_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dict_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};















/************************************************************************/
/* DICT                                                                 */
/************************************************************************/
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_insert_remainder_with_duplicates(Dict *self, size_t num_items,
                                      /*inhert(on_success)*/ DREF DeeObject **key_values) {
	size_t keyitem_i = 1;
	size_t extra_duplicates_c = 0;
	size_t extra_duplicates_a = 0;
	size_t *extra_duplicates_v = NULL;
next_keyitem:
	while (keyitem_i < num_items) {
		DREF DeeObject *key   = key_values[keyitem_i * 2 + 0];
		DREF DeeObject *value = key_values[keyitem_i * 2 + 1];
		Dee_hash_t i, perturb, hash;
		++keyitem_i;
		hash = DeeObject_Hash(key);
		perturb = i = hash & self->d_mask;
		for (;; DeeDict_HashNx(i, perturb)) {
			struct dict_item *item = &self->d_elem[i & self->d_mask];
			if (item->di_key) { /* Already in use */
				int temp;
				if likely(item->di_hash != hash)
					continue;
				temp = DeeObject_TryCompareEq(item->di_key, key);
				if unlikely(temp == Dee_COMPARE_ERR)
					goto err;
				if likely(temp != 0)
					continue;

				/* Another duplicate key. */
				ASSERT(extra_duplicates_c <= extra_duplicates_a);
				if (extra_duplicates_c >= extra_duplicates_a) {
					size_t min_alloc = extra_duplicates_c + 1;
					size_t new_alloc = extra_duplicates_a * 2;
					size_t *newvec;
					if (new_alloc < 4)
						new_alloc = 4;
					if (new_alloc < min_alloc)
						new_alloc = min_alloc;
					newvec = (size_t *)Dee_TryReallocc(extra_duplicates_v, new_alloc, sizeof(size_t));
					if unlikely(!newvec) {
						new_alloc = min_alloc;
						newvec = (size_t *)Dee_Reallocc(extra_duplicates_v, new_alloc, sizeof(size_t));
						if unlikely(!newvec)
							goto err;
					}
					extra_duplicates_v = newvec;
					extra_duplicates_a = new_alloc;
				}
				extra_duplicates_v[extra_duplicates_c] = keyitem_i;
				++extra_duplicates_c;
				--self->d_used;
				--self->d_size;
				goto next_keyitem;
			}
			item->di_hash  = hash;
			item->di_key   = key;   /* Inherit reference. */
			item->di_value = value; /* Inherit reference. */
			break;
		}
	}
	Dee_Decref_unlikely(key_values[0]);
	Dee_Decref_unlikely(key_values[1]);
#ifndef __OPTIMIZE_SIZE__
	if (extra_duplicates_c)
#endif /* !__OPTIMIZE_SIZE__ */
	{
		for (keyitem_i = 0; keyitem_i < extra_duplicates_c; ++keyitem_i) {
			size_t index = extra_duplicates_v[keyitem_i];
			Dee_Decref_unlikely(key_values[index * 2 + 0]);
			Dee_Decref_unlikely(key_values[index * 2 + 1]);
		}
		Dee_Free(extra_duplicates_v);
	}
	return 0;
err:
	Dee_Free(extra_duplicates_v);
	return -1;
}

/* Create a new Dict by inheriting a set of passed key-item pairs.
 * @param: key_values: A vector containing `num_items*2' elements,
 *                     even ones being keys and odd ones being items.
 * @param: num_items:  The number of key-value pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewKeyValuesInherited(size_t num_items,
                              /*inhert(on_success)*/ DREF DeeObject **key_values) {
	DREF Dict *result;
	/* Allocate the Dict object. */
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if (!num_items) {
		/* Special case: allocate an empty Dict. */
		result->d_mask = 0;
		result->d_size = 0;
		result->d_used = 0;
		result->d_elem = empty_dict_items;
	} else {
		size_t min_mask = 16 - 1, mask;

		/* Figure out how large the mask of the Dict is going to be. */
		while ((num_items & min_mask) != num_items)
			min_mask = (min_mask << 1) | 1;

		/* Prefer using a mask of one greater level to improve performance. */
		mask = (min_mask << 1) | 1;
		result->d_elem = (struct dict_item *)Dee_TryCallocc(mask + 1, sizeof(struct dict_item));
		if unlikely(!result->d_elem) {
			/* Try one level less if that failed. */
			mask = min_mask;
			result->d_elem = (struct dict_item *)Dee_Callocc(mask + 1, sizeof(struct dict_item));
			if unlikely(!result->d_elem)
				goto err_r;
		}

		/* Without any dummy items, these are identical. */
		result->d_size = num_items;
		result->d_used = num_items;
		result->d_mask = mask;
		while (num_items--) {
			DREF DeeObject *key   = *key_values++;
			DREF DeeObject *value = *key_values++;
			Dee_hash_t i, perturb, hash;
			hash = DeeObject_Hash(key);
			perturb = i = hash & mask;
			for (;; DeeDict_HashNx(i, perturb)) {
				struct dict_item *item = &result->d_elem[i & mask];
				if (item->di_key) { /* Already in use */
					int temp;
					if likely(item->di_hash != hash)
						continue;
					temp = DeeObject_TryCompareEq(item->di_key, key);
					if unlikely(temp == Dee_COMPARE_ERR)
						goto err_r_elem;
					if likely(temp != 0)
						continue;

					/* Duplicate key. */
					key_values -= 2;
					++num_items;
					if unlikely(dict_insert_remainder_with_duplicates(result, num_items, key_values))
						goto err_r_elem;
					goto done_populate_result;
				}
				item->di_hash  = hash;
				item->di_key   = key;   /* Inherit reference. */
				item->di_value = value; /* Inherit reference. */
				break;
			}
		}
done_populate_result:;
	}
	Dee_atomic_rwlock_init(&result->d_lock);

	/* Initialize and start tracking the new Dict. */
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err_r_elem:
	Dee_Free(result->d_elem);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE NONNULL((1)) void DCALL dict_fini(Dict *__restrict self);

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define dict_insert_sequence_foreach_PTR ((Dee_foreach_pair_t)&dict_setitem)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
#define dict_insert_sequence_foreach_PTR &dict_insert_sequence_foreach
PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
dict_insert_sequence_foreach(void *arg, DeeObject *key, DeeObject *value) {
	return dict_setitem((Dict *)arg, key, value);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_copy(Dict *__restrict self, Dict *__restrict other);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_sequence(Dict *__restrict self,
                   DeeObject *__restrict sequence) {
	DeeTypeObject *tp = Dee_TYPE(sequence);
	if (tp == &DeeDict_Type)
		return dict_copy(self, (Dict *)sequence);

	/* Optimizations for `_RoDict' */
	if (tp == &DeeRoDict_Type) {
		STATIC_ASSERT(sizeof(struct dict_item) == sizeof(struct rodict_item));
		STATIC_ASSERT(offsetof(struct dict_item, di_key) == offsetof(struct rodict_item, rdi_key));
		STATIC_ASSERT(offsetof(struct dict_item, di_value) == offsetof(struct rodict_item, rdi_value));
		STATIC_ASSERT(offsetof(struct dict_item, di_hash) == offsetof(struct rodict_item, rdi_hash));
		struct dict_item *iter, *end;
		DeeRoDictObject *src = (DeeRoDictObject *)sequence;
		Dee_atomic_rwlock_init(&self->d_lock);
		self->d_used = self->d_size = src->rd_size;
		if unlikely(!self->d_size) {
			self->d_mask = 0;
			self->d_elem = empty_dict_items;
		} else {
			self->d_mask = src->rd_mask;
			self->d_elem = (struct dict_item *)Dee_Mallocc(src->rd_mask + 1,
			                                               sizeof(struct dict_item));
			if unlikely(!self->d_elem)
				goto err;
			memcpyc(self->d_elem, src->rd_elem,
			        self->d_mask + 1,
			        sizeof(struct dict_item));
			end = (iter = self->d_elem) + (self->d_mask + 1);
			for (; iter < end; ++iter) {
				if (!iter->di_key)
					continue;
				Dee_Incref(iter->di_key);
				Dee_Incref(iter->di_value);
			}
		}
		weakref_support_init(self);
		return 0;
	}
	/* TODO: Optimizations for `_SharedMap' */
	/* TODO: Fast-sequence support */

	/* Fallback: enumerate the sequence pair-wise and insert into "self" */
	self->d_mask = 0;
	self->d_size = 0;
	self->d_used = 0;
	self->d_elem = empty_dict_items;
	Dee_atomic_rwlock_init(&self->d_lock);
	weakref_support_init(self);
	if unlikely(DeeObject_ForeachPair(sequence, dict_insert_sequence_foreach_PTR, self))
		goto err_self;
	return 0;
err_self:
	dict_fini(self);
err:
	return -1;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_TryNewWithHint(size_t num_items) {
	/* Not implemented (and never will; only supported by new dict system) */
	(void)num_items;
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewWithHint(size_t num_items) {
	DREF Dict *result;
	/* Allocate the Dict object. */
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if (!num_items) {
		/* Special case: allocate an empty Dict. */
return_empty_dict:
		result->d_mask = 0;
		result->d_elem = empty_dict_items;
	} else {
		size_t min_mask = 16 - 1, mask;

		/* Figure out how large the mask of the Dict is going to be. */
		while ((num_items & min_mask) != num_items)
			min_mask = (min_mask << 1) | 1;

		/* Prefer using a mask of one greater level to improve performance. */
		mask = (min_mask << 1) | 1;
		result->d_elem = (struct dict_item *)Dee_TryCallocc(mask + 1, sizeof(struct dict_item));
		if unlikely(!result->d_elem) {
			/* Try one level less if that failed. */
			mask = min_mask;
			result->d_elem = (struct dict_item *)Dee_TryCallocc(mask + 1, sizeof(struct dict_item));
			if unlikely(!result->d_elem)
				goto return_empty_dict;
		}

		/* Without any dummy items, these are identical. */
		result->d_mask = mask;
	}
	result->d_size = 0;
	result->d_used = 0;
	Dee_atomic_rwlock_init(&result->d_lock);
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromSequence(DeeObject *__restrict self) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if unlikely(dict_init_sequence(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromSequenceInherited(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	if (DeeDict_CheckExact(self)) {
		if (!DeeObject_IsShared(self))
			return self; /* Can re-use existing Dict object. */
	}
	result = DeeDict_FromSequence(self);
	if likely(result)
		Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromRoDict(DeeObject *__restrict self) {
	DREF Dict *result;
	struct dict_item *iter, *end;
	DeeRoDictObject *src = (DeeRoDictObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(src, &DeeRoDict_Type);
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	Dee_atomic_rwlock_init(&result->d_lock);
	result->d_mask = src->rd_mask;
	result->d_used = result->d_size = src->rd_size;
	if unlikely(!result->d_size) {
		result->d_elem = empty_dict_items;
	} else {
		result->d_elem = (struct dict_item *)Dee_Mallocc(src->rd_mask + 1,
		                                                 sizeof(struct dict_item));
		if unlikely(!result->d_elem)
			goto err_r;
		iter = (struct dict_item *)memcpyc(result->d_elem, src->rd_elem,
		                                   result->d_mask + 1,
		                                   sizeof(struct dict_item));
		end = iter + (result->d_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Incref(iter->di_key);
			Dee_Incref(iter->di_value);
		}
	}
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_ctor(Dict *__restrict self) {
	self->d_mask = 0;
	self->d_size = 0;
	self->d_used = 0;
	self->d_elem = empty_dict_items;
	Dee_atomic_rwlock_init(&self->d_lock);
	weakref_support_init(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_copy(Dict *__restrict self,
          Dict *__restrict other) {
	struct dict_item *iter, *end;
	Dee_atomic_rwlock_init(&self->d_lock);
again:
	DeeDict_LockRead(other);
	self->d_mask = other->d_mask;
	self->d_used = other->d_used;
	self->d_size = other->d_size;
	if ((self->d_elem = other->d_elem) != empty_dict_items) {
		self->d_elem = (struct dict_item *)Dee_TryMallocc(other->d_mask + 1,
		                                                  sizeof(struct dict_item));
		if unlikely(!self->d_elem) {
			DeeDict_LockEndRead(other);
			if (Dee_CollectMemory((other->d_mask + 1) *
			                      sizeof(struct dict_item)))
				goto again;
			goto err;
		}
		memcpyc(self->d_elem, other->d_elem,
		        self->d_mask + 1,
		        sizeof(struct dict_item));
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Incref(iter->di_key);
			Dee_XIncref(iter->di_value);
		}
	}
	DeeDict_LockEndRead(other);
	weakref_support_init(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_deepload(Dict *__restrict self) {
	typedef struct {
		DREF DeeObject *e_key;   /* [0..1][lock(:d_lock)] Dictionary item key. */
		DREF DeeObject *e_value; /* [1..1|if(di_key == dummy, 0..0)][valid_if(di_key)][lock(:d_lock)] Dictionary item value. */
	} Entry;

	/* #1 Allocate 2 new element-vector of the same size as `self'
	 *    One of them has a length `d_mask+1', the other `d_used'
	 * #2 Copy all key/value pairs from `self' into the d_used-one (create references)
	 *    NOTE: Skip NULL/dummy entries in the Dict vector.
	 * #3 Go through the vector and create deep copies of all keys and items.
	 *    For every key, hash it and insert it into the 2nd vector from before.
	 * #4 Clear and free the 1st vector.
	 * #5 Assign the 2nd vector to the Dict, extracting the old one at the same time.
	 * #6 Clear and free the old vector. */
	Entry *new_items, *items = NULL;
	size_t i, hash_i, item_count, old_item_count = 0;
	struct dict_item *new_map, *old_map;
	size_t new_mask;
	for (;;) {
		DeeDict_LockRead(self);
		/* Optimization: if the Dict is empty, then there's nothing to copy! */
		if (self->d_elem == empty_dict_items) {
			DeeDict_LockEndRead(self);
			return 0;
		}
		item_count = self->d_used;
		if (item_count <= old_item_count)
			break;
		DeeDict_LockEndRead(self);
		new_items = (Entry *)Dee_Reallocc(items, item_count, sizeof(Entry));
		if unlikely(!new_items)
			goto err_items;
		old_item_count = item_count;
		items          = new_items;
	}

	/* Copy all used items. */
	for (i = 0, hash_i = 0; i < item_count; ++hash_i) {
		ASSERT(hash_i <= self->d_mask);
		if (self->d_elem[hash_i].di_key == NULL)
			continue;
		if (self->d_elem[hash_i].di_key == dummy)
			continue;
		items[i].e_key   = self->d_elem[hash_i].di_key;
		items[i].e_value = self->d_elem[hash_i].di_value;
		Dee_Incref(items[i].e_key);
		Dee_Incref(items[i].e_value);
		++i;
	}
	DeeDict_LockEndRead(self);

	/* With our own local copy of all items being
	 * used, replace all of them with deep copies. */
	for (i = 0; i < item_count; ++i) {
		if (DeeObject_InplaceDeepCopy(&items[i].e_key))
			goto err_items_v;
		if (DeeObject_InplaceDeepCopy(&items[i].e_value))
			goto err_items_v;
	}
	new_mask = 1;
	while ((item_count & new_mask) != item_count)
		new_mask = (new_mask << 1) | 1;
	new_map = (struct dict_item *)Dee_Callocc(new_mask + 1, sizeof(struct dict_item));
	if unlikely(!new_map)
		goto err_items_v;

	/* Insert all the copied items into the new map. */
	for (i = 0; i < item_count; ++i) {
		Dee_hash_t j, perturb, hash;
		hash    = DeeObject_Hash(items[i].e_key);
		perturb = j = hash & new_mask;
		for (;; DeeDict_HashNx(j, perturb)) {
			struct dict_item *item = &new_map[j & new_mask];
			if (item->di_key) {
				/* Check if deepcopy caused one of the elements to get duplicated. */
				if unlikely(item->di_key == items[i].e_key) {
remove_duplicate_key:
					Dee_Decref(items[i].e_key);
					Dee_Decref(items[i].e_value);
					--item_count;
					memmovedownc(&items[i],
					             &items[i + 1],
					             item_count - i,
					             sizeof(struct dict_item));
					break;
				}
				if (Dee_TYPE(item->di_key) == Dee_TYPE(items[i].e_key)) {
					int error = DeeObject_TryCompareEq(item->di_key, items[i].e_key);
					if unlikely(error == Dee_COMPARE_ERR)
						goto err_items_v_new_map;
					if (error == 0)
						goto remove_duplicate_key;
				}
				/* Slot already in use */
				continue;
			}
			item->di_hash  = hash;
			item->di_key   = items[i].e_key;   /* Inherit reference. */
			item->di_value = items[i].e_value; /* Inherit reference. */
			break;
		}
	}
	DeeDict_LockWrite(self);
	i            = self->d_mask + 1;
	self->d_mask = new_mask;
	self->d_used = item_count;
	self->d_size = item_count;
	old_map      = self->d_elem;
	self->d_elem = new_map;
	DeeDict_LockEndWrite(self);
	if (old_map != empty_dict_items) {
		while (i--) {
			if (!old_map[i].di_key)
				continue;
			Dee_Decref(old_map[i].di_value);
			Dee_Decref(old_map[i].di_key);
		}
		Dee_Free(old_map);
	}
	Dee_Free(items);
	return 0;
err_items_v_new_map:
	Dee_Free(new_map);
err_items_v:
	i = item_count;
	while (i--) {
		Dee_Decref(items[i].e_value);
		Dee_Decref(items[i].e_key);
	}
err_items:
	Dee_Free(items);
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
dict_fini(Dict *__restrict self) {
	weakref_support_fini(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);
	if (self->d_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(self->d_elem);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_assign(Dict *self, DeeObject *other) {
	Dict temp;
	size_t old_mask;
	struct Dee_dict_item *old_elem;
	if unlikely(dict_init_sequence(&temp, other))
		goto err;
	DeeDict_LockWrite(self);
	old_mask = self->d_mask;
	old_elem = self->d_elem;
	self->d_mask = temp.d_mask;
	self->d_size = temp.d_size;
	self->d_used = temp.d_used;
	self->d_elem = temp.d_elem;
	DeeDict_LockEndWrite(self);
	if (old_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = old_elem) + (old_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(old_elem);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_moveassign(Dict *self, Dict *other) {
	size_t old_mask;
	struct Dee_dict_item *old_elem;
	if unlikely(self == other)
		return 0;
	/* Steal everything from "other" and put it into "self" */
	DeeLock_Acquire2(DeeDict_LockWrite(self), DeeDict_LockTryWrite(self), DeeDict_LockEndWrite(self),
	                 DeeDict_LockWrite(other), DeeDict_LockTryWrite(other), DeeDict_LockEndWrite(other));
	old_mask = self->d_mask;
	old_elem = self->d_elem;
	self->d_mask = other->d_mask;
	self->d_size = other->d_size;
	self->d_used = other->d_used;
	self->d_elem = other->d_elem;
	other->d_mask = 0;
	other->d_size = 0;
	other->d_used = 0;
	other->d_elem = empty_dict_items;
	DeeDict_LockEndWrite(self);
	DeeDict_LockEndWrite(other);
	if (old_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = old_elem) + (old_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(old_elem);
	}
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
dict_clear(Dict *__restrict self) {
	struct dict_item *elem;
	size_t mask;
	DeeDict_LockWrite(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);

	/* Extract the vector and mask. */
	elem         = self->d_elem;
	mask         = self->d_mask;
	self->d_elem = empty_dict_items;
	self->d_mask = 0;
	self->d_used = 0;
	self->d_size = 0;
	DeeDict_LockEndWrite(self);

	/* Destroy the vector. */
	if (elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = elem) + (mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(elem);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
dict_visit(Dict *__restrict self, dvisit_t proc, void *arg) {
	DeeDict_LockRead(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);
	if (self->d_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			/* Visit all keys and associated values. */
			Dee_Visit(iter->di_key);
			Dee_XVisit(iter->di_value);
		}
	}
	DeeDict_LockEndRead(self);
}


/* Resize the hash size by a factor of 2 and re-insert all elements.
 * When `sizedir > 0', increase the hash; When `sizedir < 0', decrease it.
 * During this process all dummy items are discarded.
 * @return: true:  Successfully rehashed the Dict.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE NONNULL((1)) bool DCALL
dict_rehash(Dict *__restrict self, int sizedir) {
	struct dict_item *new_vector, *iter, *end;
	size_t new_mask = self->d_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		if unlikely(!self->d_used) {
			ASSERT(!self->d_used);

			/* Special case: delete the vector. */
			if (self->d_size) {
				ASSERT(self->d_elem != empty_dict_items);

				/* Must discard dummy items. */
				end = (iter = self->d_elem) + (self->d_mask + 1);
				for (; iter < end; ++iter) {
					ASSERT(iter->di_key == NULL ||
					       iter->di_key == dummy);
					if (iter->di_key == dummy)
						Dee_DecrefNokill(dummy);
				}
			}
			if (self->d_elem != empty_dict_items)
				Dee_Free(self->d_elem);
			self->d_elem = empty_dict_items;
			self->d_mask = 0;
			self->d_size = 0;
			return true;
		}
		new_mask = (new_mask >> 1);
		if (self->d_used >= new_mask)
			return true;
	}
	ASSERT(self->d_used < new_mask);
	ASSERT(self->d_used <= self->d_size);
	new_vector = (struct dict_item *)Dee_TryCallocc(new_mask + 1, sizeof(struct dict_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	if (self->d_elem != empty_dict_items) {
		/* Re-insert all existing items into the new Dict vector. */
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter < end; ++iter) {
			struct dict_item *item;
			Dee_hash_t i, perturb;

			/* Skip dummy keys. */
			if (!iter->di_key || iter->di_key == dummy)
				continue;
			perturb = i = iter->di_hash & new_mask;
			for (;; DeeDict_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->di_key)
					break; /* Empty slot found. */
			}

			/* Transfer this object. */
			item->di_key   = iter->di_key;
			item->di_hash  = iter->di_hash;
			item->di_value = iter->di_value;
		}
		Dee_Free(self->d_elem);

		/* With all dummy items gone, the size now equals what is actually used. */
		self->d_size = self->d_used;
	}
	ASSERT(self->d_size == self->d_used);
	self->d_mask = new_mask;
	self->d_elem = new_vector;
	return true;
}



INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeDict_GetItemStringHash(DeeObject *__restrict self,
                          char const *__restrict key,
                          Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (!strcmp(DeeString_STR(item->di_key), key)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	err_unknown_key_str((DeeObject *)me, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeDict_GetItemStringLenHash(DeeObject *__restrict self,
                             char const *__restrict key,
                             size_t keylen,
                             Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_EqualsBuf(item->di_key, key, keylen)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	err_unknown_key_str_len((DeeObject *)me, key, keylen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (strcmp(DeeString_STR(item->di_key), key) == 0) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_trygetitem_string_len_hash(DeeObject *self, char const *key,
                                size_t keylen, Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_EqualsBuf(item->di_key, key, keylen)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem_string_hash(DeeObject *__restrict self,
                         char const *__restrict key,
                         Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (strcmp(DeeString_STR(item->di_key), key) == 0) {
			DeeDict_LockEndRead(me);
			return 1;
		}
	}
	DeeDict_LockEndRead(me);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem_string_len_hash(DeeObject *__restrict self,
                             char const *__restrict key,
                             size_t keylen, Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_EqualsBuf(item->di_key, key, keylen)) {
			DeeDict_LockEndRead(me);
			return 1;
		}
	}
	DeeDict_LockEndRead(me);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_hash(DeeObject *__restrict self,
                           char const *__restrict key,
                           Dee_hash_t hash) {
	int has = dict_hasitem_string_hash(self, key, hash);
	ASSERT(has >= 0);
	return Dee_BOUND_FROMPRESENT_BOUND(has);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_len_hash(DeeObject *__restrict self,
                               char const *__restrict key,
                               size_t keylen, Dee_hash_t hash) {
	int has = dict_hasitem_string_len_hash(self, key, keylen, hash);
	ASSERT(has >= 0);
	return Dee_BOUND_FROMPRESENT_BOUND(has);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeDict_DelItemStringHash(DeeObject *__restrict self,
                          char const *__restrict key,
                          Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_key, *old_value;
	Dee_hash_t i, perturb;
#ifndef CONFIG_NO_THREADS
again_lock:
#endif /* !CONFIG_NO_THREADS */
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (strcmp(DeeString_STR(item->di_key), key) != 0)
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
		old_key   = item->di_key;
		old_value = item->di_value;
		Dee_Incref(dummy);
		item->di_key   = dummy;
		item->di_value = NULL;
		ASSERT(me->d_used);

		/* Try to rehash the Dict and get rid of dummy
		 * items if there are a lot of them now. */
		if (--me->d_used <= me->d_size / 3)
			dict_rehash(me, -1);
		DeeDict_LockEndWrite(me);
		Dee_Decref(old_value);
		Dee_Decref(old_key);
		return 0;
	}
	DeeDict_LockEndRead(me);
	return err_unknown_key_str((DeeObject *)me, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeDict_DelItemStringLenHash(DeeObject *__restrict self,
                             char const *__restrict key,
                             size_t keylen,
                             Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_key, *old_value;
	Dee_hash_t i, perturb;
#ifndef CONFIG_NO_THREADS
again_lock:
#endif /* !CONFIG_NO_THREADS */
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (!DeeString_EqualsBuf(item->di_key, key, keylen))
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
		old_key   = item->di_key;
		old_value = item->di_value;
		Dee_Incref(dummy);
		item->di_key   = dummy;
		item->di_value = NULL;
		ASSERT(me->d_used);

		/* Try to rehash the Dict and get rid of dummy
		 * items if there are a lot of them now. */
		if (--me->d_used <= me->d_size / 3)
			dict_rehash(me, -1);
		DeeDict_LockEndWrite(me);
		Dee_Decref(old_value);
		Dee_Decref(old_key);
		return 0;
	}
	DeeDict_LockEndRead(me);
	return err_unknown_key_str_len((DeeObject *)me, key, keylen);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeDict_SetItemStringHash(DeeObject *self,
                          char const *__restrict key,
                          Dee_hash_t hash,
                          DeeObject *value) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_value;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb;
again_lock:
	DeeDict_LockRead(me);
again:
	first_dummy = NULL;
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(DeeString_STR(item->di_key), key) != 0)
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */

		/* Override an existing entry. */
		old_value = item->di_value;
		Dee_Incref(value);
		item->di_value = value;
		DeeDict_LockEndWrite(me);
		Dee_Decref(old_value);
		return 0;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(me)) {
		DeeDict_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->d_size + 1 < me->d_mask) {
		DREF DeeStringObject *key_ob;
		size_t key_len = strlen(key);
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);

		/* Write to the first dummy item. */
		key_ob = (DREF DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
		                                                      key_len + 1, sizeof(char));
		if unlikely(!key_ob)
			goto collect_memory;
		DeeObject_Init(key_ob, &DeeString_Type);
		key_ob->s_data = NULL;
		key_ob->s_hash = hash;
		key_ob->s_len  = key_len;
		memcpyc(key_ob->s_str, key, key_len + 1, sizeof(char));
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);

		/* Fill in the target slot. */
		first_dummy->di_key   = (DREF DeeObject *)key_ob; /* Inherit reference. */
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(value);
		++me->d_used;
		++me->d_size;

		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (me->d_size * 2 > me->d_mask)
			dict_rehash(me, 1);
		DeeDict_LockEndWrite(me);
		return 0;
	}

	/* Rehash the Dict and try again. */
	if (dict_rehash(me, 1)) {
		DeeDict_LockDowngrade(me);
		goto again;
	}
collect_memory:
	DeeDict_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeDict_SetItemStringLenHash(DeeObject *self,
                             char const *__restrict key,
                             size_t keylen,
                             Dee_hash_t hash,
                             DeeObject *value) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_value;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb;
again_lock:
	DeeDict_LockRead(me);
again:
	first_dummy = NULL;
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue;
		if (!DeeString_EqualsBuf(item->di_key, key, keylen))
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */

		/* Override an existing entry. */
		old_value = item->di_value;
		Dee_Incref(value);
		item->di_value = value;
		DeeDict_LockEndWrite(me);
		Dee_Decref(old_value);
		return 0;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(me)) {
		DeeDict_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->d_size + 1 < me->d_mask) {
		DREF DeeStringObject *key_ob;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);

		/* Write to the first dummy item. */
		key_ob = (DREF DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
		                                                      keylen + 1, sizeof(char));
		if unlikely(!key_ob)
			goto collect_memory;
		DeeObject_Init(key_ob, &DeeString_Type);
		key_ob->s_data = NULL;
		key_ob->s_hash = hash;
		key_ob->s_len  = keylen;
		*(char *)mempcpyc(key_ob->s_str, key, keylen, sizeof(char)) = '\0';
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);

		/* Fill in the target slot. */
		first_dummy->di_key   = (DREF DeeObject *)key_ob; /* Inherit reference. */
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(value);
		++me->d_used;
		++me->d_size;

		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (me->d_size * 2 > me->d_mask)
			dict_rehash(me, 1);
		DeeDict_LockEndWrite(me);
		return 0;
	}

	/* Rehash the Dict and try again. */
	if (dict_rehash(me, 1)) {
		DeeDict_LockDowngrade(me);
		goto again;
	}
collect_memory:
	DeeDict_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}

/* This one's basically your hasitem operator. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_contains(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0)
			return_true; /* Found the item. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return_false;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_getitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	err_unknown_key_int((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_trygetitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_trygetitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return 1; /* Found the item. */
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_hasitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return 1; /* Found the item. */
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem(Dict *self, DeeObject *key) {
	int has = dict_hasitem(self, key);
	return Dee_BOUND_FROMHAS_BOUND(has);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bounditem_index(Dict *self, size_t key) {
	int has = dict_hasitem_index(self, key);
	return Dee_BOUND_FROMHAS_BOUND(has);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_byhash_impl(DeeObject *__restrict self, Dee_hash_t hash, bool key_only) {
	DREF DeeObject *result;
	DREF DeeObject *match;
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	Dict *me = (Dict *)self;
again:
	match = NULL;
	DeeDict_LockRead(me);
	vector  = me->d_elem;
	mask    = me->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item;
		item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		if unlikely(match) {
			/* There are multiple matches for `hash'. */
			DeeDict_LockEndRead(me);
			Dee_Decref(match);
			/* TODO: Dict-specific optimizations? */
			return DeeMap_HashFilter(self, hash);
		}
		if (key_only) {
			match = item->di_key;
			Dee_Incref(match);
		} else {
			DREF DeeObject *key, *value;
			key   = item->di_key;
			value = item->di_value;
			Dee_Incref(key);
			Dee_Incref(value);
			DeeDict_LockEndRead(me);
			match = (DREF DeeObject *)DeeTuple_NewUninitialized(2);
			if unlikely(!match) {
				Dee_Decref(key);
				Dee_Decref(value);
				goto err;
			}
			DeeTuple_SET(match, 0, key);   /* Inherit reference */
			DeeTuple_SET(match, 1, value); /* Inherit reference */
			DeeDict_LockRead(me);
			/* Check if the Dict was modified. */
			if (me->d_elem != vector || me->d_mask != mask ||
			    item->di_key != key || item->di_value != value) {
				DeeDict_LockEndRead(me);
				DeeTuple_FreeUninitialized((DREF DeeTupleObject *)match);
				goto again;
			}
		}
	}
	DeeDict_LockEndRead(me);
	if (!match)
		return_empty_tuple;
	result = (DREF DeeObject *)DeeTuple_NewUninitialized(1);
	if unlikely(!result)
		goto err_match;
	DeeTuple_SET(result, 0, match); /* Inherit reference */
	return result;
err_match:
	Dee_Decref(match);
err:
	return NULL;
}


/* Returns "ITER_DONE" if "key" wasn't found. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_popitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
			DREF DeeObject *item_value;
			/* Found it! */
			DeeDict_LockWrite(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key) {
				DeeDict_LockDowngrade(self);
				goto restart;
			}
			item_value = item->di_value;
			Dee_Incref(dummy);
			item->di_key   = dummy;
			item->di_value = NULL;
			ASSERT(self->d_used);
			if (--self->d_used <= self->d_size / 3)
				dict_rehash(self, -1);
			DeeDict_LockEndWrite(self);
			Dee_Decref(item_key);
			return item_value;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_delitem(Dict *self, DeeObject *key) {
	DREF DeeObject *pop_item;
	pop_item = dict_popitem(self, key);
	if (pop_item == ITER_DONE)
		return 0;
	if unlikely(!pop_item)
		goto err;
	Dee_Decref(pop_item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_delitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
			DREF DeeObject *item_value;
			/* Found it! */
			DeeDict_LockWrite(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key) {
				DeeDict_LockDowngrade(self);
				goto restart;
			}
			item_value = item->di_value;
			Dee_Incref(dummy);
			item->di_key   = dummy;
			item->di_value = NULL;
			ASSERT(self->d_used);
			if (--self->d_used <= self->d_size / 3)
				dict_rehash(self, -1);
			DeeDict_LockEndWrite(self);
			Dee_Decref(item_key);
			Dee_Decref(item_value);
			return 0;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem(Dict *self, DeeObject *key, DeeObject *value) {
	size_t mask;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
			DREF DeeObject *item_value;
			/* Found an existing item. */
			DeeDict_LockWrite(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key) {
				DeeDict_LockDowngrade(self);
				goto again;
			}
			item_value = item->di_value;
			Dee_Incref(key);
			Dee_Incref(value);
			item->di_key   = key;
			item->di_value = value;
			DeeDict_LockEndWrite(self);
			Dee_Decref(item_key);
			Dee_Decref(item_value);
			return 0;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(self)) {
		DeeDict_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if ((first_dummy != NULL) &&
	    (self->d_size + 1 < self->d_mask ||
	     first_dummy->di_key != NULL)) {
		bool wasdummy;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		wasdummy = first_dummy->di_key != NULL;
		if (wasdummy)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->d_used;
		if (!wasdummy) {
			++self->d_size;
			/* Try to keep the Dict vector big at least twice as big as the element count. */
			if (self->d_size * 2 > self->d_mask)
				dict_rehash(self, 1);
		}
		DeeDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(self, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
dict_setitem_index(Dict *self, size_t key, DeeObject *value) {
	size_t mask;
	DREF DeeObject *keyob = NULL;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeInt_Size_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_keyob; /* Error in compare operator. */
		if (error == 0) {
			DREF DeeObject *item_value;
			/* Found an existing item. */
			DeeDict_LockWrite(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key) {
				DeeDict_LockDowngrade(self);
				goto again;
			}
			item_value = item->di_value;
			Dee_Incref(value);
			item->di_value = value;
			DeeDict_LockEndWrite(self);
			Dee_Decref(item_value);
			if unlikely(keyob)
				Dee_Decref(keyob);
			return 0;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
	if (!keyob) {
		DeeDict_LockEndRead(self);
		keyob = DeeInt_NewSize(key);
		if unlikely(!keyob)
			goto err;
		DeeDict_LockWrite(self);
		goto again_lock;
	} else {
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(self)) {
			DeeDict_LockEndWrite(self);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
	}
	if ((first_dummy != NULL) &&
	    (self->d_size + 1 < self->d_mask ||
	     first_dummy->di_key != NULL)) {
		bool wasdummy;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		wasdummy = first_dummy->di_key != NULL;
		if (wasdummy)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = keyob; /* Inherit reference */
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(value);
		++self->d_used;
		if (!wasdummy) {
			++self->d_size;
			/* Try to keep the Dict vector big at least twice as big as the element count. */
			if (self->d_size * 2 > self->d_mask)
				dict_rehash(self, 1);
		}
		DeeDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(self, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err_keyob:
	Dee_XDecref(keyob);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_setold_ex(Dict *self, DeeObject *key, DeeObject *value) {
	size_t mask;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
/*again_lock:*/
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
			DREF DeeObject *item_value;
			/* Found an existing key. */
			DeeDict_LockWrite(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key) {
				DeeDict_LockDowngrade(self);
				goto again;
			}
			item_value = item->di_value;
			Dee_Incref(key);
			Dee_Incref(value);
			item->di_key   = key;
			item->di_value = value;
			DeeDict_LockEndWrite(self);
			return item_value; /* Inherit reference */
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
	DeeDict_LockEndRead(self);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_setnew_ex(Dict *self, DeeObject *key, DeeObject *value) {
	size_t mask;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
			DREF DeeObject *item_value;
			/* Found an existing key. */
			DeeDict_LockRead(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key)
				goto again;
			item_value = item->di_value;
			Dee_Incref(item_value);
			DeeDict_LockEndRead(self);
			return item_value;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(self)) {
		DeeDict_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if ((first_dummy != NULL) &&
	    (self->d_size + 1 < self->d_mask ||
	     first_dummy->di_key != NULL)) {
		bool wasdummy;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		wasdummy = first_dummy->di_key != NULL;
		if (wasdummy)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->d_used;
		if (!wasdummy) {
			++self->d_size;
			/* Try to keep the Dict vector big at least twice as big as the element count. */
			if (self->d_size * 2 > self->d_mask)
				dict_rehash(self, 1);
		}
		DeeDict_LockEndWrite(self);
		return ITER_DONE;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(self, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_setdefault(Dict *self, DeeObject *key, DeeObject *value) {
	size_t mask;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
			DREF DeeObject *item_value;
			/* Found an existing key. */
			DeeDict_LockRead(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key)
				goto again;
			item_value = item->di_value;
			Dee_Incref(item_value);
			DeeDict_LockEndRead(self);
			return item_value;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(self)) {
		DeeDict_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if ((first_dummy != NULL) &&
	    (self->d_size + 1 < self->d_mask ||
	     first_dummy->di_key != NULL)) {
		bool wasdummy;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		wasdummy = first_dummy->di_key != NULL;
		if (wasdummy)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref_n(value, 2); /* `first_dummy->di_key', `return' */
		++self->d_used;
		if (!wasdummy) {
			++self->d_size;
			/* Try to keep the Dict vector big at least twice as big as the element count. */
			if (self->d_size * 2 > self->d_mask)
				dict_rehash(self, 1);
		}
		DeeDict_LockEndWrite(self);
		return value;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(self, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) size_t DCALL
dict_size(Dict *__restrict self) {
	return atomic_read(&self->d_used);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_sizeob(Dict *__restrict self) {
	size_t result = atomic_read(&self->d_used);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_foreach_pair(Dict *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	DeeDict_LockRead(self);
	for (i = 0; i <= self->d_mask; ++i) {
		DREF DeeObject *key, *value;
		key = self->d_elem[i].di_key;
		if (!key || key == dummy)
			continue;
		value = self->d_elem[i].di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		temp = (*proc)(arg, key, value);
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_iter(DeeDictObject *__restrict self) {
	DREF DictIterator *result;
	result = DeeObject_MALLOC(DictIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DictIterator_Type);
	result->di_dict = self;
	Dee_Incref(self);
	result->di_next = atomic_read(&self->d_elem);
done:
	return (DREF DeeObject *)result;
}

PRIVATE struct type_seq dict_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&dict_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&dict_foreach_pair,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&dict_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&dict_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeDict_GetItemStringHash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeDict_DelItemStringHash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&DeeDict_SetItemStringHash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeDict_GetItemStringLenHash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeDict_DelItemStringLenHash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&DeeDict_SetItemStringLenHash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_hasitem_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_repr(Dict *__restrict self) {
	Dee_ssize_t error;
	struct unicode_printer p;
	struct dict_item *iter, *end;
	struct dict_item *vector;
	size_t mask;
	bool is_first;
again:
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "Dict({ ") < 0)
		goto err;
	is_first = true;
	DeeDict_LockRead(self);
	vector = self->d_elem;
	mask   = self->d_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key, *value;
		key = iter->di_key;
		if (key == NULL || key == dummy)
			continue;
		value = iter->di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		/* Print this key/value pair. */
		error = unicode_printer_printf(&p, "%s%r: %r", is_first ? "" : ", ", key, value);
		Dee_Decref(value);
		Dee_Decref(key);
		if unlikely(error < 0)
			goto err;
		is_first = false;
		DeeDict_LockRead(self);
		if unlikely(self->d_elem != vector ||
		            self->d_mask != mask)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	if unlikely((is_first ? UNICODE_PRINTER_PRINT(&p, "})")
	                      : UNICODE_PRINTER_PRINT(&p, " })")) < 0)
		goto err;
	return unicode_printer_pack(&p);
restart:
	DeeDict_LockEndRead(self);
	unicode_printer_fini(&p);
	goto again;
err:
	unicode_printer_fini(&p);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_printrepr(Dict *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	struct dict_item *iter, *end;
	struct dict_item *vector;
	size_t mask;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "Dict({ ");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	DeeDict_LockRead(self);
	vector = self->d_elem;
	mask   = self->d_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key, *value;
		key = iter->di_key;
		if (key == NULL || key == dummy)
			continue;
		value = iter->di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		/* Print this key/value pair. */
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0) {
				Dee_Decref(value);
				Dee_Decref(key);
				goto err;
			}
			result += temp;
		}
		temp = DeeFormat_Printf(printer, arg, "%r: %r", key, value);
		Dee_Decref(value);
		Dee_Decref(key);
		if unlikely(temp < 0)
			goto err;
		is_first = false;
		DeeDict_LockRead(self);
		if unlikely(self->d_elem != vector ||
		            self->d_mask != mask) {
			DeeDict_LockEndRead(self);
			temp = DeeFormat_PRINT(printer, arg, ", <Dict changed while being iterated>");
			if (temp < 0)
				goto err;
			result += temp;
			goto stop_after_changed;
		}
	}
	DeeDict_LockEndRead(self);
stop_after_changed:
	temp = is_first ? DeeFormat_PRINT(printer, arg, "})")
	                : DeeFormat_PRINT(printer, arg, " })");
	if (temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bool(Dict *__restrict self) {
	return atomic_read(&self->d_used) != 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_init(Dict *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if unlikely(DeeArg_Unpack(argc, argv, "o:Dict", &seq))
		goto err;
	return dict_init_sequence(self, seq);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_mh_clear(Dict *self) {
	dict_clear(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_mh_pop(Dict *self, DeeObject *key) {
	DREF DeeObject *result;
	result = dict_popitem(self, key);
	if unlikely(result == ITER_DONE) {
		err_unknown_key((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_pop_with_default(Dict *self, DeeObject *key, DeeObject *default_) {
	DREF DeeObject *result;
	result = dict_popitem(self, key);
	if (result == ITER_DONE) {
		Dee_Incref(default_);
		result = default_;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_remove(Dict *self, DeeObject *key) {
	DREF DeeObject *result;
	result = dict_popitem(self, key);
	if (result == ITER_DONE)
		return 0;
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 1; /* Removed! */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_mh_popitem(Dict *__restrict self) {
	DREF DeeTupleObject *result;
	struct dict_item *iter;
	/* Allocate a tuple which we're going to fill with some key-value pair. */
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err;
	DeeDict_LockWrite(self);
	if unlikely(!self->d_used) {
		DeeDict_LockEndWrite(self);
		DeeTuple_FreeUninitialized(result);
		return_none;
	}
	iter = self->d_elem;
	while (!iter->di_key || iter->di_key == dummy) {
		ASSERT(iter != self->d_elem + self->d_mask);
		++iter;
	}
	DeeTuple_SET(result, 0, iter->di_key);   /* Inherit reference. */
	DeeTuple_SET(result, 1, iter->di_value); /* Inherit reference. */
	Dee_Incref(dummy);
	iter->di_key   = dummy;
	iter->di_value = NULL;
	ASSERT(self->d_used);
	if (--self->d_used <= self->d_size / 3)
		dict_rehash(self, -1);
	DeeDict_LockEndWrite(self);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_mh_update(Dict *self, DeeObject *items) {
	return (int)DeeObject_ForeachPair(items, dict_insert_sequence_foreach_PTR, self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_sizeof(Dict *self) {
	return DeeInt_NewSize(sizeof(Dict) +
	                      ((self->d_mask + 1) *
	                       sizeof(struct dict_item)));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_byhash(Dict *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__template, "o:byhash", &template_))
		goto err;
	/* TODO: Some way to expose this function with "key_only=true" (so it can be used by "Dict.keys.byhash") */
	return dict_byhash_impl((DeeObject *)self,
	                        DeeObject_Hash(template_),
	                        false);
err:
	return NULL;
}


/* TODO: Introduce a function `__missing__(key)->object' that is called
 *       when a key can't be found (won't be called by GetItemDef()).
 *       The default implementation of this function should then throw
 *       a `KeyError', rather than `operator []' itself.
 *    -> User-classes can then override that function to implement
 *       some custom behavior for dealing with missing keys.
 * XXX: This would need to be implemented in "DeeMapping_Type"; not here
 */

DOC_REF(map_byhash_doc);

PRIVATE struct type_method tpconst dict_methods[] = {
	TYPE_KWMETHOD("byhash", &dict_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_HINTREF(seq_clear),
	TYPE_METHOD_HINTREF(map_pop),
	TYPE_METHOD_HINTREF(map_setold_ex),
	TYPE_METHOD_HINTREF(map_setnew_ex),
	TYPE_METHOD_HINTREF(map_setdefault),
	TYPE_METHOD_HINTREF(map_popitem),
	TYPE_METHOD_HINTREF(map_update),
	TYPE_METHOD_HINTREF(map_remove),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst dict_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_clear, &dict_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setold_ex, &dict_mh_setold_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setnew_ex, &dict_mh_setnew_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setdefault, &dict_mh_setdefault, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop, &dict_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop_with_default, &dict_mh_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_popitem, &dict_mh_popitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_update, &dict_mh_update, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_remove, &dict_mh_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

#ifndef CONFIG_NO_DEEMON_100_COMPAT
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deprecated_d100_get_maxloadfactor(DeeObject *__restrict self);
#define deprecated_d100_del_maxloadfactor (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)
#define deprecated_d100_set_maxloadfactor (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_getset tpconst dict_getsets[] = {
	TYPE_GETTER(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER(STR_frozen, &DeeRoDict_FromSequence, "->?#Frozen"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETSET_F("max_load_factor",
	              &deprecated_d100_get_maxloadfactor,
	              &deprecated_d100_del_maxloadfactor,
	              &deprecated_d100_set_maxloadfactor,
	              METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "->?Dfloat\n"
	              "Deprecated. Always returns ${1.0}, with del/set being ignored"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETTER_F("__sizeof__", &dict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};



PRIVATE struct type_member tpconst dict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_gc tpconst dict_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&dict_clear
};

PRIVATE struct type_operator const dict_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0004_ASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0005_MOVEASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0033_DELITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0034_SETITEM, METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Dict),
	/* .tp_doc      = */ DOC("The builtin mapping object for translating keys to items\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(items:?S?T2?O?O)\n"
	                         "Create a new ?., using key-items pairs extracted from @items.\n"
	                         "Iterate @items and unpack each element into 2 others, using them "
	                         /**/ "as key and value to insert into @this ?."),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(Dict),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&dict_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&dict_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&dict_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&dict_init,
				TYPE_FIXED_ALLOCATOR_GC(Dict)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dict_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_moveassign,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&dict_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&dict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&dict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dict_visit,
	/* .tp_gc            = */ &dict_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ dict_methods,
	/* .tp_getsets       = */ dict_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_class_members,
	/* .tp_method_hints  = */ dict_method_hints,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ dict_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(dict_operators)
};
#endif /* !CONFIG_EXPERIMENTAL_ORDERED_DICTS */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_DICT_C */
