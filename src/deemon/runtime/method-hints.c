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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINTS_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINTS_C 1

#include <deemon/api.h>
#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/class.h>
#include <deemon/method-hints.h>
#include <deemon/mro.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>

/**/
#include "method-hint-defaults.h"
#include "method-hint-select.h"
#include "method-hints.h"
#include "strings.h"

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN




INTDEF Dee_funptr_t tpconst mh_unsupported_impls[Dee_TMH_COUNT];
INTERN_TPCONST Dee_funptr_t tpconst mh_unsupported_impls[Dee_TMH_COUNT] = {
	/* clang-format off */
/*[[[deemon (printMhUnsupportedArrayMembers from "..method-hints.method-hints")();]]]*/
	(Dee_funptr_t)&default__seq_operator_bool__unsupported,
	(Dee_funptr_t)&default__seq_operator_sizeob__unsupported,
	(Dee_funptr_t)&default__seq_operator_size__unsupported,
	(Dee_funptr_t)&default__seq_operator_iter__unsupported,
	(Dee_funptr_t)&default__seq_operator_foreach__unsupported,
	(Dee_funptr_t)&default__seq_operator_foreach_pair__unsupported,
	(Dee_funptr_t)&default__seq_operator_iterkeys__unsupported,
	(Dee_funptr_t)&default__seq_any__unsupported,
	(Dee_funptr_t)&default__seq_any_with_key__unsupported,
	(Dee_funptr_t)&default__seq_any_with_range__unsupported,
	(Dee_funptr_t)&default__seq_any_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_trygetfirst__unsupported,
	(Dee_funptr_t)&default__seq_getfirst__unsupported,
	(Dee_funptr_t)&default__seq_boundfirst__unsupported,
	(Dee_funptr_t)&default__seq_delfirst__unsupported,
	(Dee_funptr_t)&default__seq_setfirst__unsupported,
/*[[[end]]]*/
	/* clang-format on */
};

INTERN NONNULL((1)) void
(DCALL Dee_type_mh_cache_destroy)(struct Dee_type_mh_cache *__restrict self) {
	/* TODO: Finalize "union mhc_slot" fields. */
	Dee_type_mh_cache_free(self);
}

INTERN WUNUSED NONNULL((1)) struct Dee_type_mh_cache *
(DCALL Dee_type_mh_cache_of)(DeeTypeObject *__restrict self) {
	struct Dee_type_mh_cache *result;
	result = atomic_read(&self->tp_mhcache);
	if unlikely(!result) {
		result = Dee_type_mh_cache_alloc();
		if likely(result) {
			if unlikely(!atomic_cmpxch(&self->tp_mhcache, NULL, result)) {
				Dee_Free(result);
				result = atomic_read(&self->tp_mhcache);
				ASSERT(result);
			}
		}
	}
	return result;
}


/* Same as `DeeType_GetExplicitMethodHint', but also searches the type's
 * MRO for all matches regarding attributes named "id", and returns the
 * native version for that attribute (or `NULL' if it doesn't have one)
 *
 * This function can also be used to query the optimized, internal
 * implementation of built-in sequence (TSC) functions.
 *
 * Never returns NULL when `id' has an "%{unsupported}" implementation. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetMethodHint)(DeeTypeObject *__restrict self, enum Dee_tmh_id id) {
	Dee_funptr_t result;
	struct Dee_type_mh_cache *mhcache;
	ASSERT(id < Dee_TMH_COUNT);
	mhcache = atomic_read(&self->tp_mhcache);
	if likely(mhcache) {
read_from_mhcache:
		result = Dee_type_mh_cache_gethint(mhcache, id);
		if likely(result)
			return result;
		result = DeeType_GetUncachedMethodHint(self, id);
		Dee_type_mh_cache_sethint(mhcache, id, result);
		return result;
	}
	mhcache = Dee_type_mh_cache_alloc();
	if unlikely(!mhcache)
		return DeeType_GetUncachedMethodHint(self, id);
	if unlikely(!atomic_cmpxch(&self->tp_mhcache, NULL, result)) {
		Dee_Free(mhcache);
		mhcache = atomic_read(&self->tp_mhcache);
		ASSERT(mhcache);
		goto read_from_mhcache;
	}
	result = DeeType_GetUncachedMethodHint(self, id);
	Dee_type_mh_cache_sethint(mhcache, id, result);
	return result;
}

/* Same as `DeeType_GetMethodHint', but don't make use of the method-hint cache.
 *
 * Never returns NULL when `id' has an "%{unsupported}" implementation. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetUncachedMethodHint)(DeeTypeObject *__restrict self, enum Dee_tmh_id id) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	iter = DeeTypeMRO_Init(&mro, self);
	do {
		Dee_funptr_t result;
		result = DeeType_GetPrivateMethodHint(self, iter, id);
		if (result)
			return result;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return DeeType_GetUnsupportedMethodHint(id);
}

/* Returns a pointer to method hint's entry in `self->tp_method_hints' */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetExplicitMethodHint)(DeeTypeObject *__restrict self, enum Dee_tmh_id id) {
	struct type_method_hint const *hints = self->tp_method_hints;
	if unlikely(!hints)
		goto done;
	for (; hints->tmh_func; ++hints) {
		if (hints->tmh_id == id)
			return hints->tmh_func;
	}
done:
	return NULL;
}

/* Returns the "%{unsupported}" implementation of `id'
 * (if it has one). If not, return `NULL' instead. */
PUBLIC ATTR_CONST WUNUSED Dee_funptr_t
(DCALL DeeType_GetUnsupportedMethodHint)(enum Dee_tmh_id id) {
	ASSERT(id < Dee_TMH_COUNT);
	return mh_unsupported_impls[id];
}

/* NOTE: This one needs to be binary-compatible with functions generated by "printMhInitSelectDecls()" */
typedef ATTR_PURE_T WUNUSED_T NONNULL_T((1, 2)) Dee_funptr_t
(DCALL *mh_init_select_t)(DeeTypeObject *self, DeeTypeObject *orig_type);

struct mh_init_spec_secondary_attrib {
	DeeStringObject *missa_attrib;     /* [1..1] Name of second attrib (NULL is used as sentinel) (e.g. "any") */
	DeeTypeObject   *missa_implements; /* [0..1] Type that must be implemented for this attribute to be used. */
	unsigned int     missa_seqclass;   /* [valid_if(missa_implements)] Required sequence class for this attribute to be used. (e.g. "Dee_SEQCLASS_SEQ") */
	Dee_funptr_t     missa_withattr;   /* [1..1] Fallback for direct CallAttr(missa_attrib) (e.g. `default__seq_any_with_range__with_callattr_any') */
};

struct mh_init_spec {
	DeeStringObject                            *mis_attr_prim;          /* [0..1] Name of the primary attribute (unless anonymous); e.g. "__seq_bool__" */
	struct mh_init_spec_secondary_attrib const *mis_attr_seco;          /* [0..1] Chain of secondary attributes (terminated by `missa_attrib == NULL') */
	Dee_funptr_t                                mis_withattr_prim;      /* [1..1][valid_if(mis_attr_prim)] Fallback for direct CallAttr(mis_attr_prim) (e.g. `default__seq_operator_bool__with_callattr___seq_bool__') */
	__UINTPTR_HALF_TYPE__                       mis_offsetof_cache;     /* [1..1][valid_if(mis_attr_prim || mis_attr_seco)] Offset of the cache-slot in `struct Dee_type_mh_cache' (e.g. "mhc___seq_bool__") */
	__UINTPTR_HALF_TYPE__                       mis_attr_kind;          /* [valid_if(mis_attr_prim || mis_attr_seco)] Attribute kind (one of `MH_KIND_*') */
#define MH_KIND_METHOD       0 /* Attribute is a method */
#define MH_KIND_GETSET_GET   1 /* Attribute is getset (get). The get/del/set and bound-callbacks are loaded. */
#define MH_KIND_GETSET_DEL   2 /* Attribute is getset (del). The get/del/set and bound-callbacks are loaded. */
#define MH_KIND_GETSET_SET   3 /* Attribute is getset (set). The get/del/set and bound-callbacks are loaded. */
#define MH_KIND_GETSET_BOUND 4 /* Attribute is getset (bound). The get/del/set and bound-callbacks are loaded. */
	Dee_funptr_t                                mis_withcache_object;   /* [1..1][valid_if(mis_attr_prim || mis_attr_seco)] default__seq_operator_bool__with_callobjectcache___seq_bool__ */
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
	Dee_funptr_t                                mis_withcache_method;   /* [1..1][valid_if((mis_attr_prim || mis_attr_seco) && mis_attr_kind == MH_KIND_METHOD)] default__seq_operator_bool__with_callmethodcache___seq_bool__ */
	Dee_funptr_t                                mis_withcache_kwmethod; /* [1..1][valid_if((mis_attr_prim || mis_attr_seco) && mis_attr_kind == MH_KIND_METHOD)] default__seq_operator_bool__with_callkwmethodcache___seq_bool__ */
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
	mh_init_select_t                            mis_select;             /* [0..1] Custom fallback resolver function (used when there are no attribute matches) */
};

#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define MH_INIT_SPEC_INIT(mis_attr_prim, mis_attr_seco, mis_withattr_prim, mis_offsetof_cache,               \
                          mis_attr_kind, mis_withcache_object, mis_withcache_method, mis_withcache_kwmethod, \
                          mis_select)                                                                        \
	{                                                                                                        \
		/* .mis_attr_prim          = */ (DeeStringObject *)(mis_attr_prim),                                  \
		/* .mis_attr_seco          = */ mis_attr_seco,                                                       \
		/* .mis_withattr_prim      = */ (Dee_funptr_t)(mis_withattr_prim),                                   \
		/* .mis_offsetof_cache     = */ mis_offsetof_cache,                                                  \
		/* .mis_attr_kind          = */ mis_attr_kind,                                                       \
		/* .mis_withcache_object   = */ (Dee_funptr_t)(mis_withcache_object),                                \
		/* .mis_withcache_method   = */ (Dee_funptr_t)(mis_withcache_method),                                \
		/* .mis_withcache_kwmethod = */ (Dee_funptr_t)(mis_withcache_kwmethod),                              \
		/* .mis_select             = */ (mh_init_select_t)(mis_select),                                      \
	}
#else /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define MH_INIT_SPEC_INIT(mis_attr_prim, mis_attr_seco, mis_withattr_prim, mis_offsetof_cache,               \
                          mis_attr_kind, mis_withcache_object, mis_withcache_method, mis_withcache_kwmethod, \
                          mis_select)                                                                        \
	{                                                                                                        \
		/* .mis_attr_prim        = */ (DeeStringObject *)(mis_attr_prim),                                  \
		/* .mis_attr_seco        = */ mis_attr_seco,                                                       \
		/* .mis_withattr_prim    = */ (Dee_funptr_t)(mis_withattr_prim),                                   \
		/* .mis_offsetof_cache   = */ mis_offsetof_cache,                                                  \
		/* .mis_attr_kind        = */ mis_attr_kind,                                                       \
		/* .mis_withcache_object = */ (Dee_funptr_t)(mis_withcache_object),                                \
		/* .mis_select           = */ (mh_init_select_t)(mis_select),                                      \
	}
#endif /* !CONFIG_HAVE_MH_CALLMETHODCACHE */


INTDEF struct mh_init_spec tpconst mh_init_specs[Dee_TMH_COUNT];

/* clang-format off */
/*[[[deemon (printMhInitSpecs from "..method-hints.method-hints")();]]]*/
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_any[2] = {
	{(DeeStringObject *)&str_any, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_any__with_callattr_any },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_any_with_key[2] = {
	{(DeeStringObject *)&str_any, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_any_with_key__with_callattr_any },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_any_with_range[2] = {
	{(DeeStringObject *)&str_any, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_any_with_range__with_callattr_any },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_any_with_range_and_key[2] = {
	{(DeeStringObject *)&str_any, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_any_with_range_and_key__with_callattr_any },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_trygetfirst[2] = {
	{(DeeStringObject *)&str_first, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_trygetfirst__with_callattr_first },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_getfirst[2] = {
	{(DeeStringObject *)&str_first, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_getfirst__with_callattr_first },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_boundfirst[2] = {
	{(DeeStringObject *)&str_first, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_boundfirst__with_callattr_first },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_delfirst[2] = {
	{(DeeStringObject *)&str_first, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_delfirst__with_callattr_first },
	{ NULL, NULL, 0, NULL }
};
PRIVATE struct mh_init_spec_secondary_attrib tpconst mh_secondary_seq_setfirst[2] = {
	{(DeeStringObject *)&str_first, NULL, Dee_SEQCLASS_SEQ, (Dee_funptr_t)&default__seq_setfirst__with_callattr_first },
	{ NULL, NULL, 0, NULL }
};
INTERN_TPCONST struct mh_init_spec tpconst mh_init_specs[16] = {
	MH_INIT_SPEC_INIT(&str___seq_bool__, NULL, &default__seq_operator_bool__with_callattr___seq_bool__, offsetof(struct Dee_type_mh_cache, mhc___seq_bool__), MH_KIND_METHOD, &default__seq_operator_bool__with_callobjectcache___seq_bool__, &default__seq_operator_bool__with_callmethodcache___seq_bool__, &default__seq_operator_bool__with_callkwmethodcache___seq_bool__, &mh_select_seq_operator_bool),
	MH_INIT_SPEC_INIT(&str___seq_size__, NULL, &default__seq_operator_sizeob__with_callattr___seq_size__, offsetof(struct Dee_type_mh_cache, mhc___seq_size__), MH_KIND_METHOD, &default__seq_operator_sizeob__with_callobjectcache___seq_size__, &default__seq_operator_sizeob__with_callmethodcache___seq_size__, &default__seq_operator_sizeob__with_callkwmethodcache___seq_size__, &mh_select_seq_operator_sizeob),
	MH_INIT_SPEC_INIT(&str___seq_size__, NULL, &default__seq_operator_size__with_callattr___seq_size__, offsetof(struct Dee_type_mh_cache, mhc___seq_size__), MH_KIND_METHOD, &default__seq_operator_size__with_callobjectcache___seq_size__, &default__seq_operator_size__with_callmethodcache___seq_size__, &default__seq_operator_size__with_callkwmethodcache___seq_size__, &mh_select_seq_operator_size),
	MH_INIT_SPEC_INIT(&str___seq_iter__, NULL, &default__seq_operator_iter__with_callattr___seq_iter__, offsetof(struct Dee_type_mh_cache, mhc___seq_iter__), MH_KIND_METHOD, &default__seq_operator_iter__with_callobjectcache___seq_iter__, &default__seq_operator_iter__with_callmethodcache___seq_iter__, &default__seq_operator_iter__with_callkwmethodcache___seq_iter__, &mh_select_seq_operator_iter),
	MH_INIT_SPEC_INIT(&str___seq_iter__, NULL, &default__seq_operator_foreach__with_callattr___seq_iter__, offsetof(struct Dee_type_mh_cache, mhc___seq_iter__), MH_KIND_METHOD, &default__seq_operator_foreach__with_callobjectcache___seq_iter__, &default__seq_operator_foreach__with_callmethodcache___seq_iter__, &default__seq_operator_foreach__with_callkwmethodcache___seq_iter__, &mh_select_seq_operator_foreach),
	MH_INIT_SPEC_INIT(&str___seq_iter__, NULL, &default__seq_operator_foreach_pair__with_callattr___seq_iter__, offsetof(struct Dee_type_mh_cache, mhc___seq_iter__), MH_KIND_METHOD, &default__seq_operator_foreach_pair__with_callobjectcache___seq_iter__, &default__seq_operator_foreach_pair__with_callmethodcache___seq_iter__, &default__seq_operator_foreach_pair__with_callkwmethodcache___seq_iter__, &mh_select_seq_operator_foreach_pair),
	MH_INIT_SPEC_INIT(&str___seq_iterkeys__, NULL, &default__seq_operator_iterkeys__with_callattr___seq_iterkeys__, offsetof(struct Dee_type_mh_cache, mhc___seq_iterkeys__), MH_KIND_METHOD, &default__seq_operator_iterkeys__with_callobjectcache___seq_iterkeys__, &default__seq_operator_iterkeys__with_callmethodcache___seq_iterkeys__, &default__seq_operator_iterkeys__with_callkwmethodcache___seq_iterkeys__, &mh_select_seq_operator_iterkeys),
	MH_INIT_SPEC_INIT(&str___seq_any__, mh_secondary_seq_any, &default__seq_any__with_callattr___seq_any__, offsetof(struct Dee_type_mh_cache, mhc___seq_any__), MH_KIND_METHOD, &default__seq_any__with_callobjectcache___seq_any__, &default__seq_any__with_callmethodcache___seq_any__, &default__seq_any__with_callkwmethodcache___seq_any__, &mh_select_seq_any),
	MH_INIT_SPEC_INIT(&str___seq_any__, mh_secondary_seq_any_with_key, &default__seq_any_with_key__with_callattr___seq_any__, offsetof(struct Dee_type_mh_cache, mhc___seq_any__), MH_KIND_METHOD, &default__seq_any_with_key__with_callobjectcache___seq_any__, &default__seq_any_with_key__with_callmethodcache___seq_any__, &default__seq_any_with_key__with_callkwmethodcache___seq_any__, &mh_select_seq_any_with_key),
	MH_INIT_SPEC_INIT(&str___seq_any__, mh_secondary_seq_any_with_range, &default__seq_any_with_range__with_callattr___seq_any__, offsetof(struct Dee_type_mh_cache, mhc___seq_any__), MH_KIND_METHOD, &default__seq_any_with_range__with_callobjectcache___seq_any__, &default__seq_any_with_range__with_callmethodcache___seq_any__, &default__seq_any_with_range__with_callkwmethodcache___seq_any__, &mh_select_seq_any_with_range),
	MH_INIT_SPEC_INIT(&str___seq_any__, mh_secondary_seq_any_with_range_and_key, &default__seq_any_with_range_and_key__with_callattr___seq_any__, offsetof(struct Dee_type_mh_cache, mhc___seq_any__), MH_KIND_METHOD, &default__seq_any_with_range_and_key__with_callobjectcache___seq_any__, &default__seq_any_with_range_and_key__with_callmethodcache___seq_any__, &default__seq_any_with_range_and_key__with_callkwmethodcache___seq_any__, &mh_select_seq_any_with_range_and_key),
	MH_INIT_SPEC_INIT(&str___seq_first__, mh_secondary_seq_trygetfirst, &default__seq_trygetfirst__with_callattr___seq_first__, offsetof(struct Dee_type_mh_cache, mhc_get___seq_first__), MH_KIND_METHOD, &default__seq_trygetfirst__with_callobjectcache___seq_first__, NULL, NULL, &mh_select_seq_trygetfirst),
	MH_INIT_SPEC_INIT(&str___seq_first__, mh_secondary_seq_getfirst, &default__seq_getfirst__with_callattr___seq_first__, offsetof(struct Dee_type_mh_cache, mhc_get___seq_first__), MH_KIND_GETSET_GET, &default__seq_getfirst__with_callobjectcache___seq_first__, NULL, NULL, &mh_select_seq_getfirst),
	MH_INIT_SPEC_INIT(&str___seq_first__, mh_secondary_seq_boundfirst, &default__seq_boundfirst__with_callattr___seq_first__, offsetof(struct Dee_type_mh_cache, mhc_get___seq_first__), MH_KIND_GETSET_BOUND, &default__seq_boundfirst__with_callobjectcache___seq_first__, NULL, NULL, NULL),
	MH_INIT_SPEC_INIT(&str___seq_first__, mh_secondary_seq_delfirst, &default__seq_delfirst__with_callattr___seq_first__, offsetof(struct Dee_type_mh_cache, mhc_del___seq_first__), MH_KIND_GETSET_DEL, &default__seq_delfirst__with_callobjectcache___seq_first__, NULL, NULL, &mh_select_seq_delfirst),
	MH_INIT_SPEC_INIT(&str___seq_first__, mh_secondary_seq_setfirst, &default__seq_setfirst__with_callattr___seq_first__, offsetof(struct Dee_type_mh_cache, mhc_set___seq_first__), MH_KIND_GETSET_SET, &default__seq_setfirst__with_callobjectcache___seq_first__, NULL, NULL, &mh_select_seq_setfirst),
};
/*[[[end]]]*/
/* clang-format on */



PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 3, 4)) Dee_funptr_t DCALL
mh_init_from_attribute(DeeTypeObject *orig_type, struct Dee_attrinfo *__restrict info,
                       struct mh_init_spec const *specs, Dee_funptr_t withattr, enum Dee_tmh_id id) {
	/* Fallback: no special-case optimization available; must use generic getattr each time. */
	Dee_funptr_t result = withattr;
	switch (info->ai_type) {

	case Dee_ATTRINFO_CUSTOM:
		/* Don't accept user-defined "operator getattr" */
		return NULL;

	case Dee_ATTRINFO_METHOD:
		if (specs->mis_attr_kind == MH_KIND_METHOD) {
			/* It's a deemon method written in C -- check if the
			 * type overwrites it using an explicit method hint. */
			result = DeeType_GetExplicitMethodHint((DeeTypeObject *)info->ai_decl, id);
			if (result)
				return result;
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
			ASSERT(specs->mis_withcache_method);
			ASSERT(specs->mis_withcache_kwmethod);
			{
				struct Dee_type_mh_cache *mhcache;
				union mhc_slot *cache;
				mhcache = Dee_type_mh_cache_of(orig_type);
				if unlikely(!mhcache)
					return withattr;
				cache = (union mhc_slot *)((byte_t *)mhcache + specs->mis_offsetof_cache);
				atomic_cmpxch(&cache->c_method, NULL, info->ai_value.v_method->m_func);
			}
			return info->ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS
			       ? specs->mis_withcache_method
			       : specs->mis_withcache_kwmethod;
#else /* CONFIG_HAVE_MH_CALLMETHODCACHE */
			result = withattr;
#endif /* !CONFIG_HAVE_MH_CALLMETHODCACHE */
		}
		break;

	case Dee_ATTRINFO_GETSET:
		if (specs->mis_attr_kind != MH_KIND_METHOD) {
			/* It's a deemon getset written in C -- check if the
			 * type overwrites it using an explicit method hint. */
			result = DeeType_GetExplicitMethodHint((DeeTypeObject *)info->ai_decl, id);
			if (result)
				return result;

			/* See if we can directly bind the relevant getset callback. */
			switch (specs->mis_attr_kind) {
			case MH_KIND_GETSET_GET:
				result = (Dee_funptr_t)info->ai_value.v_getset->gs_get;
				break;
			case MH_KIND_GETSET_DEL:
				result = (Dee_funptr_t)info->ai_value.v_getset->gs_del;
				break;
			case MH_KIND_GETSET_SET:
				result = (Dee_funptr_t)info->ai_value.v_getset->gs_set;
				break;
			case MH_KIND_GETSET_BOUND:
				result = (Dee_funptr_t)info->ai_value.v_getset->gs_bound;
				break;
			default: break;
			}
			if (result)
				return result;
			result = withattr;
		}
		break;

	case Dee_ATTRINFO_ATTR:
		/* Optimization when a user-defined class defines a function/property  */
		if (specs->mis_attr_kind != MH_KIND_METHOD
		    ? ((info->ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY)) ==
		       /*                             */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FGETSET))
		    : ((info->ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY)) ==
		       /*                             */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FREADONLY))) {
			uint16_t addr;
			struct class_desc *desc;
			struct Dee_type_mh_cache *mhcache;
			DREF DeeObject *callback;
			if ((specs->mis_attr_kind == MH_KIND_GETSET_DEL ||
			     specs->mis_attr_kind == MH_KIND_GETSET_SET) &&
			    (info->ai_value.v_attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY))
				break;
			desc = DeeClass_DESC(info->ai_decl);
			addr = info->ai_value.v_attr->ca_addr;
			mhcache = Dee_type_mh_cache_of(orig_type);
			if unlikely(!mhcache)
				break;
			switch (specs->mis_attr_kind) {
			case MH_KIND_GETSET_GET:
			case MH_KIND_GETSET_BOUND:
				addr += Dee_CLASS_GETSET_GET;
				break;
			case MH_KIND_GETSET_DEL:
				addr += Dee_CLASS_GETSET_DEL;
				break;
			case MH_KIND_GETSET_SET:
				addr += Dee_CLASS_GETSET_SET;
				break;
			default: break;
			}
			Dee_class_desc_lock_read(desc);
			callback = desc->cd_members[addr];
			Dee_XIncref(callback);
			Dee_class_desc_lock_endread(desc);
			if likely(callback) {
				union mhc_slot *cache;
				cache = (union mhc_slot *)((byte_t *)mhcache + specs->mis_offsetof_cache);
				if unlikely(!atomic_cmpxch(&cache->c_object, NULL, callback))
					Dee_Decref(callback);
				return specs->mis_withcache_object;
			}
		}
		break;

	default:
		break;
	}
	return result;
}

/* Check if `self' specifically is able to supply the method hint `id'
 * in some form. If not, return `NULL' to indicate this lack of support.
 *
 * Note that this function doesn't return "%{unsupported}" implementations. */
PUBLIC ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_funptr_t DCALL
DeeType_GetPrivateMethodHint(DeeTypeObject *self, DeeTypeObject *orig_type, enum Dee_tmh_id id) {
	struct Dee_attrinfo info;
	struct mh_init_spec const *specs = &mh_init_specs[id];
	ASSERT(id < Dee_TMH_COUNT);

	/* Check if the type "self" implements attributes that can be used to implement the method hint. */
	if (specs->mis_attr_prim &&
	    DeeObject_TFindPrivateAttrInfo(self, NULL, (DeeObject *)specs->mis_attr_prim, &info)) {
		Dee_funptr_t result;
		result = mh_init_from_attribute(orig_type, &info, specs, specs->mis_withattr_prim, id);
		if (result)
			return result;
	} else if (specs->mis_attr_seco) {
		struct mh_init_spec_secondary_attrib const *iter;
		for (iter = specs->mis_attr_seco; iter->missa_attrib; ++iter) {
			if (iter->missa_implements) {
				if (!DeeType_Implements(self, iter->missa_implements))
					continue;
			} else if (iter->missa_seqclass != 0) {
				if (DeeType_GetSeqClass(self) != iter->missa_seqclass)
					continue;
			}
			if (DeeObject_TFindPrivateAttrInfo(self, NULL, (DeeObject *)iter->missa_attrib, &info)) {
				Dee_funptr_t result;
				result = mh_init_from_attribute(orig_type, &info, specs, iter->missa_withattr, id);
				if (result)
					return result;
			}
		}
	}

	/* Without attributes, invoke a custom callback selector (if defined) */
	if (specs->mis_select)
		return (*specs->mis_select)(self, orig_type);

	/* Fallback: return "NULL" to indicate that the method isn't available. */
	return NULL;
}


DECL_END
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#include <deemon/object.h>
#include <deemon/method-hints.h>

/**/
#include "../objects/seq/default-api.h"

DECL_BEGIN

#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	PUBLIC_CONST char const DeeMH_##attr_name##_name[] = method_name;                                \
	PUBLIC_CONST char const DeeMH_##attr_name##_doc[] = doc;
#define Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(new_attr_name, alias_method_name, old_attr_name) \
	PUBLIC_CONST char const DeeMH_##new_attr_name##_name[] = alias_method_name;
#include "../../../include/deemon/method-hints.def"


typedef ATTR_PURE_T WUNUSED_T NONNULL_T((1)) Dee_funptr_t
(DCALL *require_tsc_cb_t)(DeeTypeObject *__restrict self);

#define REQUIRE_TSC_seq_foreach_reverse                     DeeType_TryRequireSeqForeachReverse
#define REQUIRE_TSC_seq_enumerate_index_reverse             DeeType_TryRequireSeqEnumerateIndexReverse
#define REQUIRE_TSC_seq_makeenumeration                     DeeType_RequireSeqMakeEnumeration
#define REQUIRE_TSC_seq_makeenumeration_with_int_range      DeeType_RequireSeqMakeEnumerationWithIntRange
#define REQUIRE_TSC_seq_makeenumeration_with_range          DeeType_RequireSeqMakeEnumerationWithRange
#define REQUIRE_TSC_seq_operator_bool                       DeeType_RequireSeqOperatorBool
#define REQUIRE_TSC_seq_operator_iter                       DeeType_RequireSeqOperatorIter
#define REQUIRE_TSC_seq_operator_sizeob                     DeeType_RequireSeqOperatorSizeOb
#define REQUIRE_TSC_seq_operator_contains                   DeeType_RequireSeqOperatorContains
#define REQUIRE_TSC_seq_operator_getitem                    DeeType_RequireSeqOperatorGetItem
#define REQUIRE_TSC_seq_operator_delitem                    DeeType_RequireSeqOperatorDelItem
#define REQUIRE_TSC_seq_operator_setitem                    DeeType_RequireSeqOperatorSetItem
#define REQUIRE_TSC_seq_operator_getrange                   DeeType_RequireSeqOperatorGetRange
#define REQUIRE_TSC_seq_operator_delrange                   DeeType_RequireSeqOperatorDelRange
#define REQUIRE_TSC_seq_operator_setrange                   DeeType_RequireSeqOperatorSetRange
#define REQUIRE_TSC_seq_operator_foreach                    DeeType_RequireSeqOperatorForeach
#define REQUIRE_TSC_seq_operator_foreach_pair               DeeType_RequireSeqOperatorForeachPair
#define REQUIRE_TSC_seq_operator_enumerate                  DeeType_RequireSeqOperatorEnumerate
#define REQUIRE_TSC_seq_operator_enumerate_index            DeeType_RequireSeqOperatorEnumerateIndex
#define REQUIRE_TSC_seq_operator_iterkeys                   DeeType_RequireSeqOperatorIterKeys
#define REQUIRE_TSC_seq_operator_bounditem                  DeeType_RequireSeqOperatorBoundItem
#define REQUIRE_TSC_seq_operator_hasitem                    DeeType_RequireSeqOperatorHasItem
#define REQUIRE_TSC_seq_operator_size                       DeeType_RequireSeqOperatorSize
#define REQUIRE_TSC_seq_operator_size_fast                  DeeType_RequireSeqOperatorSizeFast
#define REQUIRE_TSC_seq_operator_getitem_index              DeeType_RequireSeqOperatorGetItemIndex
#define REQUIRE_TSC_seq_operator_delitem_index              DeeType_RequireSeqOperatorDelItemIndex
#define REQUIRE_TSC_seq_operator_setitem_index              DeeType_RequireSeqOperatorSetItemIndex
#define REQUIRE_TSC_seq_operator_bounditem_index            DeeType_RequireSeqOperatorBoundItemIndex
#define REQUIRE_TSC_seq_operator_hasitem_index              DeeType_RequireSeqOperatorHasItemIndex
#define REQUIRE_TSC_seq_operator_getrange_index             DeeType_RequireSeqOperatorGetRangeIndex
#define REQUIRE_TSC_seq_operator_delrange_index             DeeType_RequireSeqOperatorDelRangeIndex
#define REQUIRE_TSC_seq_operator_setrange_index             DeeType_RequireSeqOperatorSetRangeIndex
#define REQUIRE_TSC_seq_operator_getrange_index_n           DeeType_RequireSeqOperatorGetRangeIndexN
#define REQUIRE_TSC_seq_operator_delrange_index_n           DeeType_RequireSeqOperatorDelRangeIndexN
#define REQUIRE_TSC_seq_operator_setrange_index_n           DeeType_RequireSeqOperatorSetRangeIndexN
#define REQUIRE_TSC_seq_operator_trygetitem                 DeeType_RequireSeqOperatorTryGetItem
#define REQUIRE_TSC_seq_operator_trygetitem_index           DeeType_RequireSeqOperatorTryGetItemIndex
#define REQUIRE_TSC_seq_operator_hash                       DeeType_RequireSeqOperatorHash
#define REQUIRE_TSC_seq_operator_compare_eq                 DeeType_RequireSeqOperatorCompareEq
#define REQUIRE_TSC_seq_operator_compare                    DeeType_RequireSeqOperatorCompare
#define REQUIRE_TSC_seq_operator_trycompare_eq              DeeType_RequireSeqOperatorTryCompareEq
#define REQUIRE_TSC_seq_operator_eq                         DeeType_RequireSeqOperatorEq
#define REQUIRE_TSC_seq_operator_ne                         DeeType_RequireSeqOperatorNe
#define REQUIRE_TSC_seq_operator_lo                         DeeType_RequireSeqOperatorLo
#define REQUIRE_TSC_seq_operator_le                         DeeType_RequireSeqOperatorLe
#define REQUIRE_TSC_seq_operator_gr                         DeeType_RequireSeqOperatorGr
#define REQUIRE_TSC_seq_operator_ge                         DeeType_RequireSeqOperatorGe
#define REQUIRE_TSC_seq_operator_inplace_add                DeeType_RequireSeqOperatorInplaceAdd
#define REQUIRE_TSC_seq_operator_inplace_mul                DeeType_RequireSeqOperatorInplaceMul
#define REQUIRE_TSC_set_operator_iter                       DeeType_RequireSetOperatorIter
#define REQUIRE_TSC_set_operator_foreach                    DeeType_RequireSetOperatorForeach
#define REQUIRE_TSC_set_operator_size                       DeeType_RequireSetOperatorSize
#define REQUIRE_TSC_set_operator_sizeob                     DeeType_RequireSetOperatorSizeOb
#define REQUIRE_TSC_set_operator_hash                       DeeType_RequireSetOperatorHash
#define REQUIRE_TSC_set_operator_compare_eq                 DeeType_RequireSetOperatorCompareEq
#define REQUIRE_TSC_set_operator_trycompare_eq              DeeType_RequireSetOperatorTryCompareEq
#define REQUIRE_TSC_set_operator_eq                         DeeType_RequireSetOperatorEq
#define REQUIRE_TSC_set_operator_ne                         DeeType_RequireSetOperatorNe
#define REQUIRE_TSC_set_operator_lo                         DeeType_RequireSetOperatorLo
#define REQUIRE_TSC_set_operator_le                         DeeType_RequireSetOperatorLe
#define REQUIRE_TSC_set_operator_gr                         DeeType_RequireSetOperatorGr
#define REQUIRE_TSC_set_operator_ge                         DeeType_RequireSetOperatorGe
#define REQUIRE_TSC_set_operator_inv                        DeeType_RequireSetOperatorInv
#define REQUIRE_TSC_set_operator_add                        DeeType_RequireSetOperatorAdd
#define REQUIRE_TSC_set_operator_sub                        DeeType_RequireSetOperatorSub
#define REQUIRE_TSC_set_operator_and                        DeeType_RequireSetOperatorAnd
#define REQUIRE_TSC_set_operator_xor                        DeeType_RequireSetOperatorXor
#define REQUIRE_TSC_set_operator_inplace_add                DeeType_RequireSetOperatorInplaceAdd
#define REQUIRE_TSC_set_operator_inplace_sub                DeeType_RequireSetOperatorInplaceSub
#define REQUIRE_TSC_set_operator_inplace_and                DeeType_RequireSetOperatorInplaceAnd
#define REQUIRE_TSC_set_operator_inplace_xor                DeeType_RequireSetOperatorInplaceXor
#define REQUIRE_TSC_map_operator_contains                   DeeType_RequireMapOperatorContains
#define REQUIRE_TSC_map_operator_getitem                    DeeType_RequireMapOperatorGetItem
#define REQUIRE_TSC_map_operator_delitem                    DeeType_RequireMapOperatorDelItem
#define REQUIRE_TSC_map_operator_setitem                    DeeType_RequireMapOperatorSetItem
#define REQUIRE_TSC_map_operator_enumerate                  DeeType_RequireMapOperatorEnumerate
#define REQUIRE_TSC_map_operator_enumerate_index            DeeType_RequireMapOperatorEnumerateIndex
#define REQUIRE_TSC_map_operator_bounditem                  DeeType_RequireMapOperatorBoundItem
#define REQUIRE_TSC_map_operator_hasitem                    DeeType_RequireMapOperatorHasItem
#define REQUIRE_TSC_map_operator_getitem_index              DeeType_RequireMapOperatorGetItemIndex
#define REQUIRE_TSC_map_operator_delitem_index              DeeType_RequireMapOperatorDelItemIndex
#define REQUIRE_TSC_map_operator_setitem_index              DeeType_RequireMapOperatorSetItemIndex
#define REQUIRE_TSC_map_operator_bounditem_index            DeeType_RequireMapOperatorBoundItemIndex
#define REQUIRE_TSC_map_operator_hasitem_index              DeeType_RequireMapOperatorHasItemIndex
#define REQUIRE_TSC_map_operator_trygetitem                 DeeType_RequireMapOperatorTryGetItem
#define REQUIRE_TSC_map_operator_trygetitem_index           DeeType_RequireMapOperatorTryGetItemIndex
#define REQUIRE_TSC_map_operator_trygetitem_string_hash     DeeType_RequireMapOperatorTryGetItemStringHash
#define REQUIRE_TSC_map_operator_getitem_string_hash        DeeType_RequireMapOperatorGetItemStringHash
#define REQUIRE_TSC_map_operator_delitem_string_hash        DeeType_RequireMapOperatorDelItemStringHash
#define REQUIRE_TSC_map_operator_setitem_string_hash        DeeType_RequireMapOperatorSetItemStringHash
#define REQUIRE_TSC_map_operator_bounditem_string_hash      DeeType_RequireMapOperatorBoundItemStringHash
#define REQUIRE_TSC_map_operator_hasitem_string_hash        DeeType_RequireMapOperatorHasItemStringHash
#define REQUIRE_TSC_map_operator_trygetitem_string_len_hash DeeType_RequireMapOperatorTryGetItemStringLenHash
#define REQUIRE_TSC_map_operator_getitem_string_len_hash    DeeType_RequireMapOperatorGetItemStringLenHash
#define REQUIRE_TSC_map_operator_delitem_string_len_hash    DeeType_RequireMapOperatorDelItemStringLenHash
#define REQUIRE_TSC_map_operator_setitem_string_len_hash    DeeType_RequireMapOperatorSetItemStringLenHash
#define REQUIRE_TSC_map_operator_bounditem_string_len_hash  DeeType_RequireMapOperatorBoundItemStringLenHash
#define REQUIRE_TSC_map_operator_hasitem_string_len_hash    DeeType_RequireMapOperatorHasItemStringLenHash
#define REQUIRE_TSC_map_operator_compare_eq                 DeeType_RequireMapOperatorCompareEq
#define REQUIRE_TSC_map_operator_trycompare_eq              DeeType_RequireMapOperatorTryCompareEq
#define REQUIRE_TSC_map_operator_eq                         DeeType_RequireMapOperatorEq
#define REQUIRE_TSC_map_operator_ne                         DeeType_RequireMapOperatorNe
#define REQUIRE_TSC_map_operator_lo                         DeeType_RequireMapOperatorLo
#define REQUIRE_TSC_map_operator_le                         DeeType_RequireMapOperatorLe
#define REQUIRE_TSC_map_operator_gr                         DeeType_RequireMapOperatorGr
#define REQUIRE_TSC_map_operator_ge                         DeeType_RequireMapOperatorGe
#define REQUIRE_TSC_map_operator_add                        DeeType_RequireMapOperatorAdd
#define REQUIRE_TSC_map_operator_sub                        DeeType_RequireMapOperatorSub
#define REQUIRE_TSC_map_operator_and                        DeeType_RequireMapOperatorAnd
#define REQUIRE_TSC_map_operator_xor                        DeeType_RequireMapOperatorXor
#define REQUIRE_TSC_map_operator_inplace_add                DeeType_RequireMapOperatorInplaceAdd
#define REQUIRE_TSC_map_operator_inplace_sub                DeeType_RequireMapOperatorInplaceSub
#define REQUIRE_TSC_map_operator_inplace_and                DeeType_RequireMapOperatorInplaceAnd
#define REQUIRE_TSC_map_operator_inplace_xor                DeeType_RequireMapOperatorInplaceXor
#define REQUIRE_TSC_seq_trygetfirst                         DeeType_RequireSeqTryGetFirst
#define REQUIRE_TSC_seq_getfirst                            DeeType_RequireSeqGetFirst
#define REQUIRE_TSC_seq_boundfirst                          DeeType_RequireSeqBoundFirst
#define REQUIRE_TSC_seq_delfirst                            DeeType_RequireSeqDelFirst
#define REQUIRE_TSC_seq_setfirst                            DeeType_RequireSeqSetFirst
#define REQUIRE_TSC_seq_trygetlast                          DeeType_RequireSeqTryGetLast
#define REQUIRE_TSC_seq_getlast                             DeeType_RequireSeqGetLast
#define REQUIRE_TSC_seq_boundlast                           DeeType_RequireSeqBoundLast
#define REQUIRE_TSC_seq_dellast                             DeeType_RequireSeqDelLast
#define REQUIRE_TSC_seq_setlast                             DeeType_RequireSeqSetLast
#define REQUIRE_TSC_seq_any                                 DeeType_RequireSeqAny
#define REQUIRE_TSC_seq_any_with_key                        DeeType_RequireSeqAnyWithKey
#define REQUIRE_TSC_seq_any_with_range                      DeeType_RequireSeqAnyWithRange
#define REQUIRE_TSC_seq_any_with_range_and_key              DeeType_RequireSeqAnyWithRangeAndKey
#define REQUIRE_TSC_seq_all                                 DeeType_RequireSeqAll
#define REQUIRE_TSC_seq_all_with_key                        DeeType_RequireSeqAllWithKey
#define REQUIRE_TSC_seq_all_with_range                      DeeType_RequireSeqAllWithRange
#define REQUIRE_TSC_seq_all_with_range_and_key              DeeType_RequireSeqAllWithRangeAndKey
#define REQUIRE_TSC_seq_parity                              DeeType_RequireSeqParity
#define REQUIRE_TSC_seq_parity_with_key                     DeeType_RequireSeqParityWithKey
#define REQUIRE_TSC_seq_parity_with_range                   DeeType_RequireSeqParityWithRange
#define REQUIRE_TSC_seq_parity_with_range_and_key           DeeType_RequireSeqParityWithRangeAndKey
#define REQUIRE_TSC_seq_reduce                              DeeType_RequireSeqReduce
#define REQUIRE_TSC_seq_reduce_with_init                    DeeType_RequireSeqReduceWithInit
#define REQUIRE_TSC_seq_reduce_with_range                   DeeType_RequireSeqReduceWithRange
#define REQUIRE_TSC_seq_reduce_with_range_and_init          DeeType_RequireSeqReduceWithRangeAndInit
#define REQUIRE_TSC_seq_min                                 DeeType_RequireSeqMin
#define REQUIRE_TSC_seq_min_with_key                        DeeType_RequireSeqMinWithKey
#define REQUIRE_TSC_seq_min_with_range                      DeeType_RequireSeqMinWithRange
#define REQUIRE_TSC_seq_min_with_range_and_key              DeeType_RequireSeqMinWithRangeAndKey
#define REQUIRE_TSC_seq_max                                 DeeType_RequireSeqMax
#define REQUIRE_TSC_seq_max_with_key                        DeeType_RequireSeqMaxWithKey
#define REQUIRE_TSC_seq_max_with_range                      DeeType_RequireSeqMaxWithRange
#define REQUIRE_TSC_seq_max_with_range_and_key              DeeType_RequireSeqMaxWithRangeAndKey
#define REQUIRE_TSC_seq_sum                                 DeeType_RequireSeqSum
#define REQUIRE_TSC_seq_sum_with_range                      DeeType_RequireSeqSumWithRange
#define REQUIRE_TSC_seq_count                               DeeType_RequireSeqCount
#define REQUIRE_TSC_seq_count_with_key                      DeeType_RequireSeqCountWithKey
#define REQUIRE_TSC_seq_count_with_range                    DeeType_RequireSeqCountWithRange
#define REQUIRE_TSC_seq_count_with_range_and_key            DeeType_RequireSeqCountWithRangeAndKey
#define REQUIRE_TSC_seq_contains                            DeeType_RequireSeqContains
#define REQUIRE_TSC_seq_contains_with_key                   DeeType_RequireSeqContainsWithKey
#define REQUIRE_TSC_seq_contains_with_range                 DeeType_RequireSeqContainsWithRange
#define REQUIRE_TSC_seq_contains_with_range_and_key         DeeType_RequireSeqContainsWithRangeAndKey
#define REQUIRE_TSC_seq_locate                              DeeType_RequireSeqLocate
#define REQUIRE_TSC_seq_locate_with_key                     DeeType_RequireSeqLocateWithKey
#define REQUIRE_TSC_seq_locate_with_range                   DeeType_RequireSeqLocateWithRange
#define REQUIRE_TSC_seq_locate_with_range_and_key           DeeType_RequireSeqLocateWithRangeAndKey
#define REQUIRE_TSC_seq_rlocate_with_range                  DeeType_RequireSeqRLocateWithRange
#define REQUIRE_TSC_seq_rlocate_with_range_and_key          DeeType_RequireSeqRLocateWithRangeAndKey
#define REQUIRE_TSC_seq_startswith                          DeeType_RequireSeqStartsWith
#define REQUIRE_TSC_seq_startswith_with_key                 DeeType_RequireSeqStartsWithWithKey
#define REQUIRE_TSC_seq_startswith_with_range               DeeType_RequireSeqStartsWithWithRange
#define REQUIRE_TSC_seq_startswith_with_range_and_key       DeeType_RequireSeqStartsWithWithRangeAndKey
#define REQUIRE_TSC_seq_endswith                            DeeType_RequireSeqEndsWith
#define REQUIRE_TSC_seq_endswith_with_key                   DeeType_RequireSeqEndsWithWithKey
#define REQUIRE_TSC_seq_endswith_with_range                 DeeType_RequireSeqEndsWithWithRange
#define REQUIRE_TSC_seq_endswith_with_range_and_key         DeeType_RequireSeqEndsWithWithRangeAndKey
#define REQUIRE_TSC_seq_find                                DeeType_RequireSeqFind
#define REQUIRE_TSC_seq_find_with_key                       DeeType_RequireSeqFindWithKey
#define REQUIRE_TSC_seq_rfind                               DeeType_RequireSeqRFind
#define REQUIRE_TSC_seq_rfind_with_key                      DeeType_RequireSeqRFindWithKey
#define REQUIRE_TSC_seq_erase                               DeeType_RequireSeqErase
#define REQUIRE_TSC_seq_insert                              DeeType_RequireSeqInsert
#define REQUIRE_TSC_seq_insertall                           DeeType_RequireSeqInsertAll
#define REQUIRE_TSC_seq_pushfront                           DeeType_RequireSeqPushFront
#define REQUIRE_TSC_seq_append                              DeeType_RequireSeqAppend
#define REQUIRE_TSC_seq_extend                              DeeType_RequireSeqExtend
#define REQUIRE_TSC_seq_xchitem_index                       DeeType_RequireSeqXchItemIndex
#define REQUIRE_TSC_seq_clear                               DeeType_RequireSeqClear
#define REQUIRE_TSC_seq_pop                                 DeeType_RequireSeqPop
#define REQUIRE_TSC_seq_remove                              DeeType_RequireSeqRemove
#define REQUIRE_TSC_seq_remove_with_key                     DeeType_RequireSeqRemoveWithKey
#define REQUIRE_TSC_seq_rremove                             DeeType_RequireSeqRRemove
#define REQUIRE_TSC_seq_rremove_with_key                    DeeType_RequireSeqRRemoveWithKey
#define REQUIRE_TSC_seq_removeall                           DeeType_RequireSeqRemoveAll
#define REQUIRE_TSC_seq_removeall_with_key                  DeeType_RequireSeqRemoveAllWithKey
#define REQUIRE_TSC_seq_removeif                            DeeType_RequireSeqRemoveIf
#define REQUIRE_TSC_seq_resize                              DeeType_RequireSeqResize
#define REQUIRE_TSC_seq_fill                                DeeType_RequireSeqFill
#define REQUIRE_TSC_seq_reverse                             DeeType_RequireSeqReverse
#define REQUIRE_TSC_seq_reversed                            DeeType_RequireSeqReversed
#define REQUIRE_TSC_seq_sort                                DeeType_RequireSeqSort
#define REQUIRE_TSC_seq_sort_with_key                       DeeType_RequireSeqSortWithKey
#define REQUIRE_TSC_seq_sorted                              DeeType_RequireSeqSorted
#define REQUIRE_TSC_seq_sorted_with_key                     DeeType_RequireSeqSortedWithKey
#define REQUIRE_TSC_seq_bfind                               DeeType_RequireSeqBFind
#define REQUIRE_TSC_seq_bfind_with_key                      DeeType_RequireSeqBFindWithKey
#define REQUIRE_TSC_seq_bposition                           DeeType_RequireSeqBPosition
#define REQUIRE_TSC_seq_bposition_with_key                  DeeType_RequireSeqBPositionWithKey
#define REQUIRE_TSC_seq_brange                              DeeType_RequireSeqBRange
#define REQUIRE_TSC_seq_brange_with_key                     DeeType_RequireSeqBRangeWithKey
#define REQUIRE_TSC_seq_blocate                             DeeType_RequireSeqBLocate
#define REQUIRE_TSC_seq_blocate_with_key                    DeeType_RequireSeqBLocateWithKey
#define REQUIRE_TSC_set_insert                              DeeType_RequireSetInsert
#define REQUIRE_TSC_set_remove                              DeeType_RequireSetRemove
#define REQUIRE_TSC_set_unify                               DeeType_RequireSetUnify
#define REQUIRE_TSC_set_insertall                           DeeType_RequireSetInsertAll
#define REQUIRE_TSC_set_removeall                           DeeType_RequireSetRemoveAll
#define REQUIRE_TSC_set_pop                                 DeeType_RequireSetPop
#define REQUIRE_TSC_set_pop_with_default                    DeeType_RequireSetPopWithDefault
#define REQUIRE_TSC_map_setold                              DeeType_RequireMapSetOld
#define REQUIRE_TSC_map_setold_ex                           DeeType_RequireMapSetOldEx
#define REQUIRE_TSC_map_setnew                              DeeType_RequireMapSetNew
#define REQUIRE_TSC_map_setnew_ex                           DeeType_RequireMapSetNewEx
#define REQUIRE_TSC_map_setdefault                          DeeType_RequireMapSetDefault
#define REQUIRE_TSC_map_update                              DeeType_RequireMapUpdate
#define REQUIRE_TSC_map_remove                              DeeType_RequireMapRemove
#define REQUIRE_TSC_map_removekeys                          DeeType_RequireMapRemoveKeys
#define REQUIRE_TSC_map_pop                                 DeeType_RequireMapPop
#define REQUIRE_TSC_map_pop_with_default                    DeeType_RequireMapPopWithDefault
#define REQUIRE_TSC_map_popitem                             DeeType_RequireMapPopItem
#define REQUIRE_TSC_map_keys                                DeeType_RequireMapKeys
#define REQUIRE_TSC_map_values                              DeeType_RequireMapValues
#define REQUIRE_TSC_map_iterkeys                            DeeType_RequireMapIterKeys
#define REQUIRE_TSC_map_itervalues                          DeeType_RequireMapIterValues


PRIVATE require_tsc_cb_t tpconst require_tsc_table[] = {
#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	(require_tsc_cb_t)&REQUIRE_TSC_##func_name,
#define Dee_DEFINE_TYPE_METHOD_HINT_TSC_ONLY
#include "../../../include/deemon/method-hints.def"
#undef Dee_DEFINE_TYPE_METHOD_HINT_TSC_ONLY
};


/* Returns a pointer to method hint's entry in `self->tp_method_hints' */
PUBLIC ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetPrivateMethodHint(DeeTypeObject *__restrict self, enum Dee_tmh_id id) {
	struct type_method_hint const *hints = self->tp_method_hints;
	if unlikely(!hints)
		goto done;
	for (; hints->tmh_func; ++hints) {
		if (hints->tmh_id == id)
			return hints->tmh_func;
	}
done:
	return NULL;
}

/* Same as `DeeType_GetPrivateMethodHint', but also searches the type's
 * MRO for all matches regarding attributes named "id", and returns the
 * native version for that attribute (or `NULL' if it doesn't have one)
 *
 * This function can also be used to query the optimized, internal
 * implementation of built-in sequence (TSC) functions. */
PUBLIC ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetMethodHint(DeeTypeObject *__restrict self, enum Dee_tmh_id id) {
	Dee_funptr_t result;

	/* Special handling for TSC (TypeSequenceCache) related method hints. */
	if (id < COMPILER_LENOF(require_tsc_table))
		return (*require_tsc_table[id])(self);

	/* Search the type's MRO for this method hint. */
	/* TODO: This is wrong -- must search *only* the type that is the origin
	 *       of whatever attribute/operator the method hint is for. */
	DeeType_mro_foreach_start(self) {
		result = DeeType_GetPrivateMethodHint(self, id);
		if (result != NULL)
			goto done;
	}
	DeeType_mro_foreach_end(self);
done:
	return result;
}

DECL_END
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINTS_C */
