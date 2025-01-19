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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINTS_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINTS_H 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>

#include <hybrid/typecore.h>


#undef CONFIG_HAVE_MH_CALLMETHODCACHE
#if 1
#define CONFIG_HAVE_MH_CALLMETHODCACHE
#endif

DECL_BEGIN

union mhc_slot {
	DREF DeeObject   *c_object;
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
	Dee_objmethod_t   c_method;
	Dee_kwobjmethod_t c_kwmethod;
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
};

struct Dee_type_mh_cache_array {
	Dee_funptr_t mh_funcs[Dee_TMH_COUNT];
};

struct Dee_type_mh_cache {
	/* Method hint function pointer caches.
	 * All of these are [0..1][lock(WRITE_ONCE)]
	 *
	 * Also note that the offset of any hint function pointer is always the "Dee_TMH_*" constant:
	 * >> void *mh_seq_operator_foreach = ((void **)tp_mhcache)[Dee_TMH_seq_operator_foreach]; */
#define Dee_type_mh_cache_gethint(self, id)    ((struct Dee_type_mh_cache_array const *)(self))->mh_funcs[id]
#define Dee_type_mh_cache_sethint(self, id, v) (void)(((struct Dee_type_mh_cache_array *)(self))->mh_funcs[id] = (v))

	/* clang-format off */
/*[[[deemon (printMhCacheNativeMembers from "..method-hints.method-hints")();]]]*/
	DeeMH_seq_operator_bool_t mh_seq_operator_bool;
	DeeMH_seq_operator_sizeob_t mh_seq_operator_sizeob;
	DeeMH_seq_operator_size_t mh_seq_operator_size;
	DeeMH_seq_operator_iter_t mh_seq_operator_iter;
	DeeMH_seq_operator_foreach_t mh_seq_operator_foreach;
	DeeMH_seq_operator_foreach_pair_t mh_seq_operator_foreach_pair;
	DeeMH_seq_operator_iterkeys_t mh_seq_operator_iterkeys;
	DeeMH_seq_any_t mh_seq_any;
	DeeMH_seq_any_with_key_t mh_seq_any_with_key;
	DeeMH_seq_any_with_range_t mh_seq_any_with_range;
	DeeMH_seq_any_with_range_and_key_t mh_seq_any_with_range_and_key;
	DeeMH_seq_trygetfirst_t mh_seq_trygetfirst;
	DeeMH_seq_getfirst_t mh_seq_getfirst;
	DeeMH_seq_boundfirst_t mh_seq_boundfirst;
	DeeMH_seq_delfirst_t mh_seq_delfirst;
	DeeMH_seq_setfirst_t mh_seq_setfirst;
/*[[[end]]]*/
	/* clang-format on */

	/* Method hint attribute data caches.
	 * All of these are [0..1][lock(WRITE_ONCE)] */

	/* clang-format off */
/*[[[deemon (printMhCacheAttributeMembers from "..method-hints.method-hints")();]]]*/
#define MHC_FIRST mhc___seq_bool__
	union mhc_slot mhc___seq_bool__;
	union mhc_slot mhc___seq_size__;
	union mhc_slot mhc___seq_iter__;
	union mhc_slot mhc___seq_iterkeys__;
	union mhc_slot mhc___seq_any__;
	union mhc_slot mhc_get___seq_first__;
	union mhc_slot mhc_del___seq_first__;
	union mhc_slot mhc_set___seq_first__;
#define MHC_LAST mhc_set___seq_first__
/*[[[end]]]*/
	/* clang-format on */
};

#define Dee_type_mh_cache_alloc() \
	((struct Dee_type_mh_cache *)Dee_TryCalloc(sizeof(struct Dee_type_mh_cache)))
#define Dee_type_mh_cache_free(self) Dee_Free(self)

INTDEF NONNULL((1)) void DCALL
Dee_type_mh_cache_destroy(struct Dee_type_mh_cache *__restrict self);

INTDEF WUNUSED NONNULL((1)) struct Dee_type_mh_cache *DCALL
Dee_type_mh_cache_of(DeeTypeObject *__restrict self);

INTDEF Dee_funptr_t tpconst mh_unsupported_impls[Dee_TMH_COUNT];
#define DeeType_GetUnsupportedMethodHint(id) mh_unsupported_impls[id]

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINTS_H */
