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
#ifdef __INTELLISENSE__
#include "dict.c"
//#define DEFINE_dict_init_fromcopy
#define DEFINE_dict_init_fromcopy_keysonly
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <stdbool.h> /* false, true */
#include <stddef.h>  /* size_t */

#if (defined(DEFINE_dict_init_fromcopy) + \
     defined(DEFINE_dict_init_fromcopy_keysonly)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_dict_init_fromcopy
#define LOCAL_dict_init_fromcopy dict_init_fromcopy
#elif defined(DEFINE_dict_init_fromcopy_keysonly)
#define LOCAL_dict_init_fromcopy dict_init_fromcopy_keysonly
#define LOCAL_IS_KEYSONLY
#else /* ... */
#error "Invalid configuration"
#endif /* ... */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_init_fromcopy(Dict *__restrict self, Dict *__restrict other) {
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
#ifdef LOCAL_IS_KEYSONLY
	/* Create reference to copied objects. */
	{
		size_t i, vsize = other->d_vsize;
		struct Dee_dict_item *orig_vtab;
		orig_vtab = _DeeDict_GetRealVTab(other);
		for (i = 0; i < vsize; ++i) {
			struct Dee_dict_item *dst_item = &copy_vtab[i];
			struct Dee_dict_item *src_item = &orig_vtab[i];
			dst_item->di_hash = src_item->di_hash;
			dst_item->di_key  = src_item->di_key;
			Dee_Incref(dst_item->di_key);
			/* Value will be initialized by the caller! */
			DBG_memset(&dst_item->di_value, 0xcc, sizeof(dst_item->di_value));
		}
	}
#else /* LOCAL_IS_KEYSONLY */
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
#endif /* !LOCAL_IS_KEYSONLY */

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
	dict_verify(self);
	return 0;
err:
	return -1;
}

#undef LOCAL_IS_KEYSONLY
#undef LOCAL_dict_init_fromcopy

DECL_END

#undef DEFINE_dict_init_fromcopy
#undef DEFINE_dict_init_fromcopy_keysonly
