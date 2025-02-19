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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_INVOKE_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_INVOKE_C 1

#include <deemon/api.h>
#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/class.h>
#include <deemon/method-hints.h>
#include <deemon/operator-hints.h>
#include <deemon/super.h>

/**/
#include "method-hint-defaults.h"

DECL_BEGIN

/************************************************************************/
/* Database for mapping impls for super calls                           */
/************************************************************************/
struct mh_super_map_replace {
	Dee_funptr_t msmr_old; /* [1..1] Old callback */
	Dee_funptr_t msmr_new; /* [1..1] New callback */
};

#define MH_SUPER_MAP_REPLACE_END { NULL, NULL }
#define MH_SUPER_MAP_REPLACE_INIT(msmr_old, msmr_new) \
	{                                                 \
		/* .msmr_old = */ (Dee_funptr_t)(msmr_old),   \
		/* .msmr_new = */ (Dee_funptr_t)(msmr_new)    \
	}

struct mh_super_map_typed {
	Dee_funptr_t msmr_regular; /* [1..1] Regular callback */
	Dee_funptr_t msmr_typed;   /* [1..1] Typed callback */
};

#define MH_SUPER_MAP_TYPED_END { NULL, NULL }
#define MH_SUPER_MAP_TYPED_INIT(msmr_regular, msmr_typed)   \
	{                                                       \
		/* .msmr_regular = */ (Dee_funptr_t)(msmr_regular), \
		/* .msmr_typed   = */ (Dee_funptr_t)(msmr_typed)    \
	}

struct mh_super_map {
	struct mh_super_map_replace const *msm_replace;    /* [0..n] Array of impls that should be replaced with other impls (terminated by `msmr_old == NULL') */
	Dee_funptr_t                const *msm_with_super; /* [0..n] Array of impls to call as `Dee_SUPER_METHOD_HINT_CC_WITH_SUPER' (terminated by `NULL') */
	struct mh_super_map_typed   const *msm_with_type;  /* [0..n] Array of impls that should be replaced with typed versions (terminated by `msmr_regular == NULL') */
};

#define MH_SUPER_MAP_INIT(msm_replace, msm_with_super, msm_with_type) \
	{                                                                 \
		/* .msm_replace    = */ msm_replace,                          \
		/* .msm_with_super = */ msm_with_super,                       \
		/* .msm_with_type  = */ msm_with_type                         \
	}

/* clang-format off */
/*[[[deemon (printMhSuperMap from "..method-hints.method-hints")();]]]*/
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_bool[9] = {
	(Dee_funptr_t)&default__seq_operator_bool__with_callattr___seq_bool__,
	(Dee_funptr_t)&default__seq_operator_bool__unsupported,
	(Dee_funptr_t)&default__seq_operator_bool__empty,
	(Dee_funptr_t)&default__seq_operator_bool__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_bool__with__seq_operator_size,
	(Dee_funptr_t)&default__seq_operator_bool__with__seq_operator_sizeob,
	(Dee_funptr_t)&default__seq_operator_bool__with__seq_operator_compare_eq,
	(Dee_funptr_t)&default__seq_operator_bool__with__set_operator_compare_eq,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_bool[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_bool__with_callobjectcache___seq_bool__, &tdefault__seq_operator_bool__with_callobjectcache___seq_bool__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__bool__with__BOOL, &tusrtype__bool__with__BOOL),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_sizeob[5] = {
	(Dee_funptr_t)&default__seq_operator_sizeob__with_callattr___seq_size__,
	(Dee_funptr_t)&default__seq_operator_sizeob__unsupported,
	(Dee_funptr_t)&default__seq_operator_sizeob__with__seq_operator_size,
	(Dee_funptr_t)&default__seq_operator_sizeob__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_sizeob[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_sizeob__with_callobjectcache___seq_size__, &tdefault__seq_operator_sizeob__with_callobjectcache___seq_size__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__sizeob__with__SIZE, &tusrtype__sizeob__with__SIZE),
	MH_SUPER_MAP_TYPED_INIT(&default__sizeob__with__size, &tdefault__sizeob__with__size),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_size[9] = {
	(Dee_funptr_t)&default__seq_operator_size__with_callattr___seq_size__,
	(Dee_funptr_t)&default__seq_operator_size__unsupported,
	(Dee_funptr_t)&default__seq_operator_size__with__seq_operator_sizeob,
	(Dee_funptr_t)&default__seq_operator_size__empty,
	(Dee_funptr_t)&default__seq_operator_size__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_size__with__seq_operator_foreach_pair,
	(Dee_funptr_t)&default__seq_operator_size__with__set_operator_sizeob,
	(Dee_funptr_t)&default__seq_operator_size__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_size[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__size__with__sizeob, &tdefault__size__with__sizeob),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_iter[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_iter[12] = {
	(Dee_funptr_t)&default__seq_operator_iter__with_callattr___seq_iter__,
	(Dee_funptr_t)&default__seq_operator_iter__unsupported,
	(Dee_funptr_t)&default__seq_operator_iter__empty,
	(Dee_funptr_t)&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_iter__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_iter__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_iter__with__map_enumerate,
	(Dee_funptr_t)&default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem,
	(Dee_funptr_t)&default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_iter[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_iter__with_callobjectcache___seq_iter__, &tdefault__seq_operator_iter__with_callobjectcache___seq_iter__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__iter__with__ITER, &tusrtype__iter__with__ITER),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_foreach[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_foreach[12] = {
	(Dee_funptr_t)&default__seq_operator_foreach__with_callattr___seq_iter__,
	(Dee_funptr_t)&default__seq_operator_foreach__unsupported,
	(Dee_funptr_t)&default__seq_operator_foreach__with__seq_operator_foreach_pair,
	(Dee_funptr_t)&default__seq_operator_foreach__with__seq_operator_iter,
	(Dee_funptr_t)&default__seq_operator_foreach__empty,
	(Dee_funptr_t)&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_foreach__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_foreach__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_foreach__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_foreach[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__foreach__with__iter, &tdefault__foreach__with__iter),
	MH_SUPER_MAP_TYPED_INIT(&default__foreach__with__foreach_pair, &tdefault__foreach__with__foreach_pair),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_foreach_pair[7] = {
	(Dee_funptr_t)&default__seq_operator_foreach_pair__with_callattr___seq_iter__,
	(Dee_funptr_t)&default__seq_operator_foreach_pair__unsupported,
	(Dee_funptr_t)&default__seq_operator_foreach_pair__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_foreach_pair__with__seq_operator_iter,
	(Dee_funptr_t)&default__seq_operator_foreach_pair__empty,
	(Dee_funptr_t)&default__seq_operator_foreach_pair__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_foreach_pair[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__foreach_pair__with__foreach, &tdefault__foreach_pair__with__foreach),
	MH_SUPER_MAP_TYPED_INIT(&default__foreach_pair__with__iter, &tdefault__foreach_pair__with__iter),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_getitem[5] = {
	(Dee_funptr_t)&default__seq_operator_getitem__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_getitem__unsupported,
	(Dee_funptr_t)&default__seq_operator_getitem__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_getitem__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_getitem[9] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_getitem__with_callobjectcache___seq_getitem__, &tdefault__seq_operator_getitem__with_callobjectcache___seq_getitem__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__getitem__with__GETITEM, &tusrtype__getitem__with__GETITEM),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__trygetitem, &tdefault__getitem__with__trygetitem),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_index__and__getitem_string_len_hash, &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_index__and__getitem_string_hash, &tdefault__getitem__with__getitem_index__and__getitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_index, &tdefault__getitem__with__getitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_string_len_hash, &tdefault__getitem__with__getitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_string_hash, &tdefault__getitem__with__getitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_getitem_index[7] = {
	(Dee_funptr_t)&default__seq_operator_getitem_index__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_getitem_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_getitem_index__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_getitem_index__empty,
	(Dee_funptr_t)&default__seq_operator_getitem_index__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_getitem_index__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_getitem_index[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_index__with__size__and__getitem_index_fast, &tdefault__getitem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_index__with__trygetitem_index, &tdefault__getitem_index__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_index__with__getitem, &tdefault__getitem_index__with__getitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_trygetitem[6] = {
	(Dee_funptr_t)&default__seq_operator_trygetitem__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_trygetitem__unsupported,
	(Dee_funptr_t)&default__seq_operator_trygetitem__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_trygetitem__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_trygetitem[7] = {
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__getitem, &tdefault__trygetitem__with__getitem),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash, &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash, &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_index, &tdefault__trygetitem__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_string_len_hash, &tdefault__trygetitem__with__trygetitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_string_hash, &tdefault__trygetitem__with__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_trygetitem_index[7] = {
	(Dee_funptr_t)&default__seq_operator_trygetitem_index__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_trygetitem_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_trygetitem_index__empty,
	(Dee_funptr_t)&default__seq_operator_trygetitem_index__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_trygetitem_index__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_trygetitem_index[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_index__with__size__and__getitem_index_fast, &tdefault__trygetitem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_index__with__getitem_index, &tdefault__trygetitem_index__with__getitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_index__with__trygetitem, &tdefault__trygetitem_index__with__trygetitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_hasitem[7] = {
	(Dee_funptr_t)&default__seq_operator_hasitem__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_hasitem__unsupported,
	(Dee_funptr_t)&default__seq_operator_hasitem__with__seq_operator_hasitem_index,
	(Dee_funptr_t)&default__seq_operator_hasitem__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_hasitem__empty,
	(Dee_funptr_t)&default__seq_operator_hasitem__with__seq_operator_sizeob,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_hasitem[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__trygetitem, &tdefault__hasitem__with__trygetitem),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__bounditem, &tdefault__hasitem__with__bounditem),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_len_hash, &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_hash, &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_index, &tdefault__hasitem__with__hasitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_string_len_hash, &tdefault__hasitem__with__hasitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_string_hash, &tdefault__hasitem__with__hasitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_hasitem_index[6] = {
	(Dee_funptr_t)&default__seq_operator_hasitem_index__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_hasitem_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_hasitem_index__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_hasitem_index__empty,
	(Dee_funptr_t)&default__seq_operator_hasitem_index__with__seq_operator_size,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_hasitem_index[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__size__and__getitem_index_fast, &tdefault__hasitem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__trygetitem_index, &tdefault__hasitem_index__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__bounditem_index, &tdefault__hasitem_index__with__bounditem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__hasitem, &tdefault__hasitem_index__with__hasitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_bounditem[6] = {
	(Dee_funptr_t)&default__seq_operator_bounditem__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_bounditem__unsupported,
	(Dee_funptr_t)&default__seq_operator_bounditem__with__seq_operator_bounditem_index,
	(Dee_funptr_t)&default__seq_operator_bounditem__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_bounditem__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_bounditem[9] = {
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__getitem, &tdefault__bounditem__with__getitem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_len_hash, &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_hash, &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__trygetitem__and__hasitem, &tdefault__bounditem__with__trygetitem__and__hasitem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_index, &tdefault__bounditem__with__bounditem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_string_len_hash, &tdefault__bounditem__with__bounditem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_string_hash, &tdefault__bounditem__with__bounditem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__trygetitem, &tdefault__bounditem__with__trygetitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_bounditem_index[6] = {
	(Dee_funptr_t)&default__seq_operator_bounditem_index__with_callattr___seq_getitem__,
	(Dee_funptr_t)&default__seq_operator_bounditem_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_bounditem_index__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_bounditem_index__empty,
	(Dee_funptr_t)&default__seq_operator_bounditem_index__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_bounditem_index[6] = {
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__size__and__getitem_index_fast, &tdefault__bounditem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__getitem_index, &tdefault__bounditem_index__with__getitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__bounditem, &tdefault__bounditem_index__with__bounditem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__trygetitem_index__and__hasitem_index, &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__trygetitem_index, &tdefault__bounditem_index__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_delitem[5] = {
	(Dee_funptr_t)&default__seq_operator_delitem__with_callattr___seq_delitem__,
	(Dee_funptr_t)&default__seq_operator_delitem__unsupported,
	(Dee_funptr_t)&default__seq_operator_delitem__with__seq_operator_delitem_index,
	(Dee_funptr_t)&default__seq_operator_delitem__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_delitem[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_delitem__with_callobjectcache___seq_delitem__, &tdefault__seq_operator_delitem__with_callobjectcache___seq_delitem__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__delitem__with__DELITEM, &tusrtype__delitem__with__DELITEM),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_index__and__delitem_string_len_hash, &tdefault__delitem__with__delitem_index__and__delitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_index__and__delitem_string_hash, &tdefault__delitem__with__delitem_index__and__delitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_index, &tdefault__delitem__with__delitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_string_len_hash, &tdefault__delitem__with__delitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_string_hash, &tdefault__delitem__with__delitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_delitem_index[6] = {
	(Dee_funptr_t)&default__seq_operator_delitem_index__with_callattr___seq_delitem__,
	(Dee_funptr_t)&default__seq_operator_delitem_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_delitem_index__empty,
	(Dee_funptr_t)&default__seq_operator_delitem_index__with__seq_operator_delrange_index,
	(Dee_funptr_t)&default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_delitem_index[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_delitem_index__with_callobjectcache___seq_delitem__, &tdefault__seq_operator_delitem_index__with_callobjectcache___seq_delitem__),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem_index__with__delitem, &tdefault__delitem_index__with__delitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_setitem[5] = {
	(Dee_funptr_t)&default__seq_operator_setitem__with_callattr___seq_setitem__,
	(Dee_funptr_t)&default__seq_operator_setitem__unsupported,
	(Dee_funptr_t)&default__seq_operator_setitem__with__seq_operator_setitem_index,
	(Dee_funptr_t)&default__seq_operator_setitem__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_setitem[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_setitem__with_callobjectcache___seq_setitem__, &tdefault__seq_operator_setitem__with_callobjectcache___seq_setitem__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__setitem__with__SETITEM, &tusrtype__setitem__with__SETITEM),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_index__and__setitem_string_len_hash, &tdefault__setitem__with__setitem_index__and__setitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_index__and__setitem_string_hash, &tdefault__setitem__with__setitem_index__and__setitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_index, &tdefault__setitem__with__setitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_string_len_hash, &tdefault__setitem__with__setitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_string_hash, &tdefault__setitem__with__setitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_setitem_index[5] = {
	(Dee_funptr_t)&default__seq_operator_setitem_index__with_callattr___seq_setitem__,
	(Dee_funptr_t)&default__seq_operator_setitem_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_setitem_index__empty,
	(Dee_funptr_t)&default__seq_operator_setitem_index__with__seq_operator_setrange_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_setitem_index[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_setitem_index__with_callobjectcache___seq_setitem__, &tdefault__seq_operator_setitem_index__with_callobjectcache___seq_setitem__),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem_index__with__setitem, &tdefault__setitem_index__with__setitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_getrange[6] = {
	(Dee_funptr_t)&default__seq_operator_getrange__with_callattr___seq_getrange__,
	(Dee_funptr_t)&default__seq_operator_getrange__unsupported,
	(Dee_funptr_t)&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n,
	(Dee_funptr_t)&default__seq_operator_getrange__empty,
	(Dee_funptr_t)&default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_getrange[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_getrange__with_callobjectcache___seq_getrange__, &tdefault__seq_operator_getrange__with_callobjectcache___seq_getrange__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__getrange__with__GETRANGE, &tusrtype__getrange__with__GETRANGE),
	MH_SUPER_MAP_TYPED_INIT(&default__getrange__with__getrange_index__and__getrange_index_n, &tdefault__getrange__with__getrange_index__and__getrange_index_n),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_getrange_index[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_getrange_index[9] = {
	(Dee_funptr_t)&default__seq_operator_getrange_index__with_callattr___seq_getrange__,
	(Dee_funptr_t)&default__seq_operator_getrange_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_getrange_index__with__seq_operator_getrange,
	(Dee_funptr_t)&default__seq_operator_getrange_index__empty,
	(Dee_funptr_t)&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_getrange_index[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__getrange_index__with__getrange, &tdefault__getrange_index__with__getrange),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_getrange_index_n[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_getrange_index_n[10] = {
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__with_callattr___seq_getrange__,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__unsupported,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__with__seq_operator_getrange,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getrange_index,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__empty,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_getrange_index_n[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__getrange_index_n__with__getrange, &tdefault__getrange_index_n__with__getrange),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_delrange[6] = {
	(Dee_funptr_t)&default__seq_operator_delrange__with_callattr___seq_delrange__,
	(Dee_funptr_t)&default__seq_operator_delrange__unsupported,
	(Dee_funptr_t)&default__seq_operator_delrange__with__seq_operator_delrange_index__and__seq_operator_delrange_index_n,
	(Dee_funptr_t)&default__seq_operator_delrange__empty,
	(Dee_funptr_t)&default__seq_operator_delrange__with__seq_operator_setrange,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_delrange[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_delrange__with_callobjectcache___seq_delrange__, &tdefault__seq_operator_delrange__with_callobjectcache___seq_delrange__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__delrange__with__DELRANGE, &tusrtype__delrange__with__DELRANGE),
	MH_SUPER_MAP_TYPED_INIT(&default__delrange__with__delrange_index__and__delrange_index_n, &tdefault__delrange__with__delrange_index__and__delrange_index_n),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_delrange_index[7] = {
	(Dee_funptr_t)&default__seq_operator_delrange_index__with_callattr___seq_delrange__,
	(Dee_funptr_t)&default__seq_operator_delrange_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_delrange_index__with__seq_operator_delrange,
	(Dee_funptr_t)&default__seq_operator_delrange_index__empty,
	(Dee_funptr_t)&default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase,
	(Dee_funptr_t)&default__seq_operator_delrange_index__with__seq_operator_setrange_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_delrange_index[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__delrange_index__with__delrange, &tdefault__delrange_index__with__delrange),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_delrange_index_n[7] = {
	(Dee_funptr_t)&default__seq_operator_delrange_index_n__with_callattr___seq_delrange__,
	(Dee_funptr_t)&default__seq_operator_delrange_index_n__unsupported,
	(Dee_funptr_t)&default__seq_operator_delrange_index_n__with__seq_operator_size__and__seq_operator_delrange_index,
	(Dee_funptr_t)&default__seq_operator_delrange_index_n__empty,
	(Dee_funptr_t)&default__seq_operator_delrange_index_n__with__seq_operator_delrange,
	(Dee_funptr_t)&default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_delrange_index_n[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__delrange_index_n__with__delrange, &tdefault__delrange_index_n__with__delrange),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_setrange[5] = {
	(Dee_funptr_t)&default__seq_operator_setrange__with_callattr___seq_setrange__,
	(Dee_funptr_t)&default__seq_operator_setrange__unsupported,
	(Dee_funptr_t)&default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n,
	(Dee_funptr_t)&default__seq_operator_setrange__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_setrange[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_setrange__with_callobjectcache___seq_setrange__, &tdefault__seq_operator_setrange__with_callobjectcache___seq_setrange__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__setrange__with__SETRANGE, &tusrtype__setrange__with__SETRANGE),
	MH_SUPER_MAP_TYPED_INIT(&default__setrange__with__setrange_index__and__setrange_index_n, &tdefault__setrange__with__setrange_index__and__setrange_index_n),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_setrange_index[6] = {
	(Dee_funptr_t)&default__seq_operator_setrange_index__with_callattr___seq_setrange__,
	(Dee_funptr_t)&default__seq_operator_setrange_index__unsupported,
	(Dee_funptr_t)&default__seq_operator_setrange_index__with__seq_operator_setrange,
	(Dee_funptr_t)&default__seq_operator_setrange_index__empty,
	(Dee_funptr_t)&default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_setrange_index[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__setrange_index__with__setrange, &tdefault__setrange_index__with__setrange),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_setrange_index_n[6] = {
	(Dee_funptr_t)&default__seq_operator_setrange_index_n__with_callattr___seq_setrange__,
	(Dee_funptr_t)&default__seq_operator_setrange_index_n__unsupported,
	(Dee_funptr_t)&default__seq_operator_setrange_index_n__with__seq_operator_size__and__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_operator_setrange_index_n__with__seq_operator_setrange,
	(Dee_funptr_t)&default__seq_operator_setrange_index_n__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_setrange_index_n[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__setrange_index_n__with__setrange, &tdefault__setrange_index_n__with__setrange),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_assign[5] = {
	(Dee_funptr_t)&default__seq_operator_assign__with_callattr___seq_assign__,
	(Dee_funptr_t)&default__seq_operator_assign__unsupported,
	(Dee_funptr_t)&default__seq_operator_assign__empty,
	(Dee_funptr_t)&default__seq_operator_assign__with__seq_operator_setrange,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_assign[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_assign__with_callobjectcache___seq_assign__, &tdefault__seq_operator_assign__with_callobjectcache___seq_assign__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__assign__with__ASSIGN, &tusrtype__assign__with__ASSIGN),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_hash[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_hash__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_hash[8] = {
	(Dee_funptr_t)&default__seq_operator_hash__with_callattr___seq_hash__,
	(Dee_funptr_t)&default__seq_operator_hash__unsupported,
	(Dee_funptr_t)&default__seq_operator_hash__empty,
	(Dee_funptr_t)&default__seq_operator_hash__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_hash__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_hash__with__seq_operator_sizeob__and__seq_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_hash[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_hash__with_callobjectcache___seq_hash__, &tdefault__seq_operator_hash__with_callobjectcache___seq_hash__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__hash__with__HASH, &tusrtype__hash__with__HASH),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__hash__with__, &tusrtype__hash__with__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_compare[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_compare[18] = {
	(Dee_funptr_t)&default__seq_operator_compare__with_callattr___seq_compare__,
	(Dee_funptr_t)&default__seq_operator_compare__unsupported,
	(Dee_funptr_t)&default__seq_operator_compare__empty,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__lo,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__le,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__gr,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__ge,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__lo,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__le,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__gr,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__ge,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_lo__and__seq_operator__gr,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_le__and__seq_operator__ge,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_compare[13] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_compare__with_callobjectcache___seq_compare__, &tdefault__seq_operator_compare__with_callobjectcache___seq_compare__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__compare__with__, &tusrtype__compare__with__),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__eq__and__lo, &tdefault__compare__with__eq__and__lo),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__eq__and__le, &tdefault__compare__with__eq__and__le),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__eq__and__gr, &tdefault__compare__with__eq__and__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__eq__and__ge, &tdefault__compare__with__eq__and__ge),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__ne__and__lo, &tdefault__compare__with__ne__and__lo),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__ne__and__le, &tdefault__compare__with__ne__and__le),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__ne__and__gr, &tdefault__compare__with__ne__and__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__ne__and__ge, &tdefault__compare__with__ne__and__ge),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__lo__and__gr, &tdefault__compare__with__lo__and__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__compare__with__le__and__ge, &tdefault__compare__with__le__and__ge),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_compare_eq[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_compare_eq[12] = {
	(Dee_funptr_t)&default__seq_operator_compare_eq__with_callattr___seq_compare_eq__,
	(Dee_funptr_t)&default__seq_operator_compare_eq__unsupported,
	(Dee_funptr_t)&default__seq_operator_compare_eq__empty,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_eq,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_ne,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_lo__and__seq_operator__gr,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_le__and__seq_operator__ge,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_compare_eq[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__, &tdefault__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__compare_eq__with__, &tusrtype__compare_eq__with__),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__compare, &tdefault__compare_eq__with__compare),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__eq, &tdefault__compare_eq__with__eq),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__ne, &tdefault__compare_eq__with__ne),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__lo__and__gr, &tdefault__compare_eq__with__lo__and__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__le__and__ge, &tdefault__compare_eq__with__le__and__ge),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_operator_trycompare_eq[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_operator_trycompare_eq__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_trycompare_eq[9] = {
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__with_callattr___seq_compare_eq__,
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__unsupported,
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__with__seq_operator_compare_eq,
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__empty,
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_operator_trycompare_eq__with__seq_operator_sizeob__and__seq_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_trycompare_eq[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_trycompare_eq__with_callobjectcache___seq_compare_eq__, &tdefault__seq_operator_trycompare_eq__with_callobjectcache___seq_compare_eq__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__trycompare_eq__with__, &tusrtype__trycompare_eq__with__),
	MH_SUPER_MAP_TYPED_INIT(&default__trycompare_eq__with__compare_eq, &tdefault__trycompare_eq__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_eq[6] = {
	(Dee_funptr_t)&default__seq_operator_eq__with_callattr___seq_eq__,
	(Dee_funptr_t)&default__seq_operator_eq__unsupported,
	(Dee_funptr_t)&default__seq_operator_eq__empty,
	(Dee_funptr_t)&default__seq_operator_eq__with__seq_operator_ne,
	(Dee_funptr_t)&default__seq_operator_eq__with__seq_operator_compare_eq,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_eq[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_eq__with_callobjectcache___seq_eq__, &tdefault__seq_operator_eq__with_callobjectcache___seq_eq__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__eq__with__EQ, &tusrtype__eq__with__EQ),
	MH_SUPER_MAP_TYPED_INIT(&default__eq__with__ne, &tdefault__eq__with__ne),
	MH_SUPER_MAP_TYPED_INIT(&default__eq__with__compare_eq, &tdefault__eq__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_ne[6] = {
	(Dee_funptr_t)&default__seq_operator_ne__with_callattr___seq_ne__,
	(Dee_funptr_t)&default__seq_operator_ne__unsupported,
	(Dee_funptr_t)&default__seq_operator_ne__empty,
	(Dee_funptr_t)&default__seq_operator_ne__with__seq_operator_eq,
	(Dee_funptr_t)&default__seq_operator_ne__with__seq_operator_compare_eq,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_ne[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_ne__with_callobjectcache___seq_ne__, &tdefault__seq_operator_ne__with_callobjectcache___seq_ne__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__ne__with__NE, &tusrtype__ne__with__NE),
	MH_SUPER_MAP_TYPED_INIT(&default__ne__with__eq, &tdefault__ne__with__eq),
	MH_SUPER_MAP_TYPED_INIT(&default__ne__with__compare_eq, &tdefault__ne__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_lo[6] = {
	(Dee_funptr_t)&default__seq_operator_lo__with_callattr___seq_lo__,
	(Dee_funptr_t)&default__seq_operator_lo__unsupported,
	(Dee_funptr_t)&default__seq_operator_lo__empty,
	(Dee_funptr_t)&default__seq_operator_lo__with__seq_operator_ge,
	(Dee_funptr_t)&default__seq_operator_lo__with__seq_operator_compare,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_lo[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_lo__with_callobjectcache___seq_lo__, &tdefault__seq_operator_lo__with_callobjectcache___seq_lo__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__lo__with__LO, &tusrtype__lo__with__LO),
	MH_SUPER_MAP_TYPED_INIT(&default__lo__with__ge, &tdefault__lo__with__ge),
	MH_SUPER_MAP_TYPED_INIT(&default__lo__with__compare, &tdefault__lo__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_le[6] = {
	(Dee_funptr_t)&default__seq_operator_le__with_callattr___seq_le__,
	(Dee_funptr_t)&default__seq_operator_le__unsupported,
	(Dee_funptr_t)&default__seq_operator_le__empty,
	(Dee_funptr_t)&default__seq_operator_le__with__seq_operator_gr,
	(Dee_funptr_t)&default__seq_operator_le__with__seq_operator_compare,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_le[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_le__with_callobjectcache___seq_le__, &tdefault__seq_operator_le__with_callobjectcache___seq_le__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__le__with__LE, &tusrtype__le__with__LE),
	MH_SUPER_MAP_TYPED_INIT(&default__le__with__gr, &tdefault__le__with__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__le__with__compare, &tdefault__le__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_gr[6] = {
	(Dee_funptr_t)&default__seq_operator_gr__with_callattr___seq_gr__,
	(Dee_funptr_t)&default__seq_operator_gr__unsupported,
	(Dee_funptr_t)&default__seq_operator_gr__empty,
	(Dee_funptr_t)&default__seq_operator_gr__with__seq_operator_le,
	(Dee_funptr_t)&default__seq_operator_gr__with__seq_operator_compare,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_gr[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_gr__with_callobjectcache___seq_gr__, &tdefault__seq_operator_gr__with_callobjectcache___seq_gr__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__gr__with__GR, &tusrtype__gr__with__GR),
	MH_SUPER_MAP_TYPED_INIT(&default__gr__with__le, &tdefault__gr__with__le),
	MH_SUPER_MAP_TYPED_INIT(&default__gr__with__compare, &tdefault__gr__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_ge[6] = {
	(Dee_funptr_t)&default__seq_operator_ge__with_callattr___seq_ge__,
	(Dee_funptr_t)&default__seq_operator_ge__unsupported,
	(Dee_funptr_t)&default__seq_operator_ge__empty,
	(Dee_funptr_t)&default__seq_operator_ge__with__seq_operator_lo,
	(Dee_funptr_t)&default__seq_operator_ge__with__seq_operator_compare,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_ge[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_ge__with_callobjectcache___seq_ge__, &tdefault__seq_operator_ge__with_callobjectcache___seq_ge__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__ge__with__GE, &tusrtype__ge__with__GE),
	MH_SUPER_MAP_TYPED_INIT(&default__ge__with__lo, &tdefault__ge__with__lo),
	MH_SUPER_MAP_TYPED_INIT(&default__ge__with__compare, &tdefault__ge__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_inplace_add[6] = {
	(Dee_funptr_t)&default__seq_operator_inplace_add__with_callattr___seq_inplace_add__,
	(Dee_funptr_t)&default__seq_operator_inplace_add__unsupported,
	(Dee_funptr_t)&default__seq_operator_inplace_add__with__seq_extend,
	(Dee_funptr_t)&default__seq_operator_inplace_add__empty,
	(Dee_funptr_t)&default__seq_operator_inplace_add__with__DeeSeq_Concat,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_inplace_add[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__, &tdefault__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_add__with__INPLACE_ADD, &tusrtype__inplace_add__with__INPLACE_ADD),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_add__with__add, &tdefault__inplace_add__with__add),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_inplace_mul[6] = {
	(Dee_funptr_t)&default__seq_operator_inplace_mul__with_callattr___seq_inplace_mul__,
	(Dee_funptr_t)&default__seq_operator_inplace_mul__unsupported,
	(Dee_funptr_t)&default__seq_operator_inplace_mul__empty,
	(Dee_funptr_t)&default__seq_operator_inplace_mul__with__seq_clear__and__seq_extend,
	(Dee_funptr_t)&default__seq_operator_inplace_mul__with__DeeSeq_Repeat,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_inplace_mul[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__, &tdefault__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_mul__with__INPLACE_MUL, &tusrtype__inplace_mul__with__INPLACE_MUL),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_mul__with__mul, &tdefault__inplace_mul__with__mul),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_enumerate[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_enumerate[11] = {
	(Dee_funptr_t)&default__seq_enumerate__with_callattr___seq_enumerate__,
	(Dee_funptr_t)&default__seq_enumerate__unsupported,
	(Dee_funptr_t)&default__seq_enumerate__with__seq_enumerate_index,
	(Dee_funptr_t)&default__seq_enumerate__empty,
	(Dee_funptr_t)&default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_enumerate__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_enumerate__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_enumerate__with__seq_operator_foreach__and__counter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_enumerate[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_enumerate__with_callobjectcache___seq_enumerate__, &tdefault__seq_enumerate__with_callobjectcache___seq_enumerate__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_enumerate_index[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_enumerate_index[9] = {
	(Dee_funptr_t)&default__seq_enumerate_index__with_callattr___seq_enumerate__,
	(Dee_funptr_t)&default__seq_enumerate_index__unsupported,
	(Dee_funptr_t)&default__seq_enumerate_index__with__seq_enumerate,
	(Dee_funptr_t)&default__seq_enumerate_index__empty,
	(Dee_funptr_t)&default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_enumerate_index__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_enumerate_index__with__seq_operator_foreach__and__counter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_enumerate_index[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_enumerate_index__with_callobjectcache___seq_enumerate__, &tdefault__seq_enumerate_index__with_callobjectcache___seq_enumerate__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_makeenumeration[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_makeenumeration[11] = {
	(Dee_funptr_t)&default__seq_makeenumeration__with_callattr___seq_enumerate_items__,
	(Dee_funptr_t)&default__seq_makeenumeration__unsupported,
	(Dee_funptr_t)&default__seq_makeenumeration__empty,
	(Dee_funptr_t)&default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_makeenumeration__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_makeenumeration__with__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_makeenumeration__with__seq_operator_iter__and__counter,
	(Dee_funptr_t)&default__seq_makeenumeration__with__seq_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_makeenumeration[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__, &tdefault__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_makeenumeration_with_range[7] = {
	(Dee_funptr_t)&default__seq_makeenumeration_with_range__with_callattr___seq_enumerate_items__,
	(Dee_funptr_t)&default__seq_makeenumeration_with_range__unsupported,
	(Dee_funptr_t)&default__seq_makeenumeration_with_range__with__seq_makeenumeration_with_intrange,
	(Dee_funptr_t)&default__seq_makeenumeration_with_range__empty,
	(Dee_funptr_t)&default__seq_makeenumeration_with_range__with__seq_operator_sizeob__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_makeenumeration_with_range__with__seq_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_makeenumeration_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__, &tdefault__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_makeenumeration_with_intrange[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_makeenumeration_with_intrange[10] = {
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__with_callattr___seq_enumerate_items__,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__unsupported,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__with__seq_makeenumeration_with_range,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__empty,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__with__seq_operator_iter__and__counter,
	(Dee_funptr_t)&default__seq_makeenumeration_with_intrange__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_makeenumeration_with_intrange[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_makeenumeration_with_intrange__with_callobjectcache___seq_enumerate_items__, &tdefault__seq_makeenumeration_with_intrange__with_callobjectcache___seq_enumerate_items__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_unpack[3] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_unpack__with__tp_asvector, &default__seq_unpack__with__seq_operator_foreach),
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_unpack__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_unpack__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_unpack[10] = {
	(Dee_funptr_t)&default__seq_unpack__with_callattr_unpack,
	(Dee_funptr_t)&default__seq_unpack__with_callattr___seq_unpack__,
	(Dee_funptr_t)&default__seq_unpack__unsupported,
	(Dee_funptr_t)&default__seq_unpack__with__seq_unpack_ex,
	(Dee_funptr_t)&default__seq_unpack__empty,
	(Dee_funptr_t)&default__seq_unpack__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_unpack__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_unpack__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_unpack__with__seq_operator_iter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_unpack[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_unpack__with_callobjectcache___seq_unpack__, &tdefault__seq_unpack__with_callobjectcache___seq_unpack__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_unpack_ex[3] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_unpack_ex__with__tp_asvector, &default__seq_unpack_ex__with__seq_operator_foreach),
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_unpack_ex__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_unpack_ex[9] = {
	(Dee_funptr_t)&default__seq_unpack_ex__with_callattr_unpack,
	(Dee_funptr_t)&default__seq_unpack_ex__with_callattr___seq_unpack__,
	(Dee_funptr_t)&default__seq_unpack_ex__unsupported,
	(Dee_funptr_t)&default__seq_unpack_ex__empty,
	(Dee_funptr_t)&default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_unpack_ex__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_unpack_ex__with__seq_operator_iter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_unpack_ex[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_unpack_ex__with_callobjectcache___seq_unpack__, &tdefault__seq_unpack_ex__with_callobjectcache___seq_unpack__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_unpack_ub[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_unpack_ub__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_unpack_ub[7] = {
	(Dee_funptr_t)&default__seq_unpack_ub__with_callattr_unpackub,
	(Dee_funptr_t)&default__seq_unpack_ub__with_callattr___seq_unpackub__,
	(Dee_funptr_t)&default__seq_unpack_ub__unsupported,
	(Dee_funptr_t)&default__seq_unpack_ub__empty,
	(Dee_funptr_t)&default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_getitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_unpack_ub[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_unpack_ub__with_callobjectcache___seq_unpackub__, &tdefault__seq_unpack_ub__with_callobjectcache___seq_unpackub__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_trygetfirst[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_trygetfirst__with__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_trygetfirst[7] = {
	(Dee_funptr_t)&default__seq_trygetfirst__with_callattr_first,
	(Dee_funptr_t)&default__seq_trygetfirst__with_callattr___seq_first__,
	(Dee_funptr_t)&default__seq_trygetfirst__unsupported,
	(Dee_funptr_t)&default__seq_trygetfirst__empty,
	(Dee_funptr_t)&default__seq_trygetfirst__with__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_trygetfirst__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_trygetfirst[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_trygetfirst__with_callobjectcache___seq_first__, &tdefault__seq_trygetfirst__with_callobjectcache___seq_first__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_getfirst[7] = {
	(Dee_funptr_t)&default__seq_getfirst__with_callattr_first,
	(Dee_funptr_t)&default__seq_getfirst__with_callattr___seq_first__,
	(Dee_funptr_t)&default__seq_getfirst__unsupported,
	(Dee_funptr_t)&default__seq_getfirst__empty,
	(Dee_funptr_t)&default__seq_getfirst__with__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_getfirst__with__seq_trygetfirst,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_getfirst[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_getfirst__with_callobjectcache___seq_first__, &tdefault__seq_getfirst__with_callobjectcache___seq_first__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_boundfirst[7] = {
	(Dee_funptr_t)&default__seq_boundfirst__with_callattr_first,
	(Dee_funptr_t)&default__seq_boundfirst__with_callattr___seq_first__,
	(Dee_funptr_t)&default__seq_boundfirst__unsupported,
	(Dee_funptr_t)&default__seq_boundfirst__empty,
	(Dee_funptr_t)&default__seq_boundfirst__with__seq_operator_bounditem_index,
	(Dee_funptr_t)&default__seq_boundfirst__with__seq_trygetfirst,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_boundfirst[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_boundfirst__with_callobjectcache___seq_first__, &tdefault__seq_boundfirst__with_callobjectcache___seq_first__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_delfirst[6] = {
	(Dee_funptr_t)&default__seq_delfirst__with_callattr_first,
	(Dee_funptr_t)&default__seq_delfirst__with_callattr___seq_first__,
	(Dee_funptr_t)&default__seq_delfirst__unsupported,
	(Dee_funptr_t)&default__seq_delfirst__empty,
	(Dee_funptr_t)&default__seq_delfirst__with__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_delfirst[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_delfirst__with_callobjectcache___seq_first__, &tdefault__seq_delfirst__with_callobjectcache___seq_first__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_setfirst[6] = {
	(Dee_funptr_t)&default__seq_setfirst__with_callattr_first,
	(Dee_funptr_t)&default__seq_setfirst__with_callattr___seq_first__,
	(Dee_funptr_t)&default__seq_setfirst__unsupported,
	(Dee_funptr_t)&default__seq_setfirst__empty,
	(Dee_funptr_t)&default__seq_setfirst__with__seq_operator_setitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_setfirst[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_setfirst__with_callobjectcache___seq_first__, &tdefault__seq_setfirst__with_callobjectcache___seq_first__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_trygetlast[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_trygetlast[7] = {
	(Dee_funptr_t)&default__seq_trygetlast__with_callattr_last,
	(Dee_funptr_t)&default__seq_trygetlast__with_callattr___seq_last__,
	(Dee_funptr_t)&default__seq_trygetlast__unsupported,
	(Dee_funptr_t)&default__seq_trygetlast__empty,
	(Dee_funptr_t)&default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_trygetlast__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_trygetlast[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_trygetlast__with_callobjectcache___seq_last__, &tdefault__seq_trygetlast__with_callobjectcache___seq_last__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_getlast[7] = {
	(Dee_funptr_t)&default__seq_getlast__with_callattr_last,
	(Dee_funptr_t)&default__seq_getlast__with_callattr___seq_last__,
	(Dee_funptr_t)&default__seq_getlast__unsupported,
	(Dee_funptr_t)&default__seq_getlast__empty,
	(Dee_funptr_t)&default__seq_getlast__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_getlast__with__seq_trygetlast,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_getlast[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_getlast__with_callobjectcache___seq_last__, &tdefault__seq_getlast__with_callobjectcache___seq_last__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_boundlast[7] = {
	(Dee_funptr_t)&default__seq_boundlast__with_callattr_last,
	(Dee_funptr_t)&default__seq_boundlast__with_callattr___seq_last__,
	(Dee_funptr_t)&default__seq_boundlast__unsupported,
	(Dee_funptr_t)&default__seq_boundlast__empty,
	(Dee_funptr_t)&default__seq_boundlast__with__seq_operator_size__and__seq_operator_bounditem_index,
	(Dee_funptr_t)&default__seq_boundlast__with__seq_trygetlast,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_boundlast[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_boundlast__with_callobjectcache___seq_last__, &tdefault__seq_boundlast__with_callobjectcache___seq_last__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_dellast[6] = {
	(Dee_funptr_t)&default__seq_dellast__with_callattr_last,
	(Dee_funptr_t)&default__seq_dellast__with_callattr___seq_last__,
	(Dee_funptr_t)&default__seq_dellast__unsupported,
	(Dee_funptr_t)&default__seq_dellast__empty,
	(Dee_funptr_t)&default__seq_dellast__with__seq_operator_size__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_dellast[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_dellast__with_callobjectcache___seq_last__, &tdefault__seq_dellast__with_callobjectcache___seq_last__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_setlast[6] = {
	(Dee_funptr_t)&default__seq_setlast__with_callattr_last,
	(Dee_funptr_t)&default__seq_setlast__with_callattr___seq_last__,
	(Dee_funptr_t)&default__seq_setlast__unsupported,
	(Dee_funptr_t)&default__seq_setlast__empty,
	(Dee_funptr_t)&default__seq_setlast__with__seq_operator_size__and__seq_operator_setitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_setlast[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_setlast__with_callobjectcache___seq_last__, &tdefault__seq_setlast__with_callobjectcache___seq_last__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_cached[9] = {
	(Dee_funptr_t)&default__seq_cached__with_callattr_cached,
	(Dee_funptr_t)&default__seq_cached__with_callattr___seq_cached__,
	(Dee_funptr_t)&default__seq_cached__unsupported,
	(Dee_funptr_t)&default__seq_cached__empty,
	(Dee_funptr_t)&default__seq_cached__with__seq_operator_iter,
	(Dee_funptr_t)&default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_cached__with__seq_operator_sizeob__and__seq_operator_getitem,
	(Dee_funptr_t)&default__seq_cached__with__seq_operator_getitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_cached[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_cached__with_callobjectcache___seq_cached__, &tdefault__seq_cached__with_callobjectcache___seq_cached__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_frozen[6] = {
	(Dee_funptr_t)&default__seq_frozen__with_callattr_frozen,
	(Dee_funptr_t)&default__seq_frozen__with_callattr___seq_frozen__,
	(Dee_funptr_t)&default__seq_frozen__unsupported,
	(Dee_funptr_t)&default__seq_frozen__empty,
	(Dee_funptr_t)&default__seq_frozen__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_frozen[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_frozen__with_callobjectcache___seq_frozen__, &tdefault__seq_frozen__with_callobjectcache___seq_frozen__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_any[6] = {
	(Dee_funptr_t)&default__seq_any__with_callattr_any,
	(Dee_funptr_t)&default__seq_any__with_callattr___seq_any__,
	(Dee_funptr_t)&default__seq_any__unsupported,
	(Dee_funptr_t)&default__seq_any__empty,
	(Dee_funptr_t)&default__seq_any__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_any[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_any__with_callobjectcache___seq_any__, &tdefault__seq_any__with_callobjectcache___seq_any__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_any_with_key[6] = {
	(Dee_funptr_t)&default__seq_any_with_key__with_callattr_any,
	(Dee_funptr_t)&default__seq_any_with_key__with_callattr___seq_any__,
	(Dee_funptr_t)&default__seq_any_with_key__unsupported,
	(Dee_funptr_t)&default__seq_any_with_key__empty,
	(Dee_funptr_t)&default__seq_any_with_key__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_any_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_any_with_key__with_callobjectcache___seq_any__, &tdefault__seq_any_with_key__with_callobjectcache___seq_any__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_any_with_range[6] = {
	(Dee_funptr_t)&default__seq_any_with_range__with_callattr_any,
	(Dee_funptr_t)&default__seq_any_with_range__with_callattr___seq_any__,
	(Dee_funptr_t)&default__seq_any_with_range__unsupported,
	(Dee_funptr_t)&default__seq_any_with_range__empty,
	(Dee_funptr_t)&default__seq_any_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_any_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_any_with_range__with_callobjectcache___seq_any__, &tdefault__seq_any_with_range__with_callobjectcache___seq_any__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_any_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_any_with_range_and_key__with_callattr_any,
	(Dee_funptr_t)&default__seq_any_with_range_and_key__with_callattr___seq_any__,
	(Dee_funptr_t)&default__seq_any_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_any_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_any_with_range_and_key__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_any_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_any_with_range_and_key__with_callobjectcache___seq_any__, &tdefault__seq_any_with_range_and_key__with_callobjectcache___seq_any__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_all[6] = {
	(Dee_funptr_t)&default__seq_all__with_callattr_all,
	(Dee_funptr_t)&default__seq_all__with_callattr___seq_all__,
	(Dee_funptr_t)&default__seq_all__unsupported,
	(Dee_funptr_t)&default__seq_all__empty,
	(Dee_funptr_t)&default__seq_all__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_all[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_all__with_callobjectcache___seq_all__, &tdefault__seq_all__with_callobjectcache___seq_all__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_all_with_key[6] = {
	(Dee_funptr_t)&default__seq_all_with_key__with_callattr_all,
	(Dee_funptr_t)&default__seq_all_with_key__with_callattr___seq_all__,
	(Dee_funptr_t)&default__seq_all_with_key__unsupported,
	(Dee_funptr_t)&default__seq_all_with_key__empty,
	(Dee_funptr_t)&default__seq_all_with_key__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_all_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_all_with_key__with_callobjectcache___seq_all__, &tdefault__seq_all_with_key__with_callobjectcache___seq_all__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_all_with_range[6] = {
	(Dee_funptr_t)&default__seq_all_with_range__with_callattr_all,
	(Dee_funptr_t)&default__seq_all_with_range__with_callattr___seq_all__,
	(Dee_funptr_t)&default__seq_all_with_range__unsupported,
	(Dee_funptr_t)&default__seq_all_with_range__empty,
	(Dee_funptr_t)&default__seq_all_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_all_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_all_with_range__with_callobjectcache___seq_all__, &tdefault__seq_all_with_range__with_callobjectcache___seq_all__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_all_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_all_with_range_and_key__with_callattr_all,
	(Dee_funptr_t)&default__seq_all_with_range_and_key__with_callattr___seq_all__,
	(Dee_funptr_t)&default__seq_all_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_all_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_all_with_range_and_key__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_all_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_all_with_range_and_key__with_callobjectcache___seq_all__, &tdefault__seq_all_with_range_and_key__with_callobjectcache___seq_all__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_parity[7] = {
	(Dee_funptr_t)&default__seq_parity__with_callattr_parity,
	(Dee_funptr_t)&default__seq_parity__with_callattr___seq_parity__,
	(Dee_funptr_t)&default__seq_parity__unsupported,
	(Dee_funptr_t)&default__seq_parity__empty,
	(Dee_funptr_t)&default__seq_parity__with__seq_count,
	(Dee_funptr_t)&default__seq_parity__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_parity[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_parity__with_callobjectcache___seq_parity__, &tdefault__seq_parity__with_callobjectcache___seq_parity__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_parity_with_key[6] = {
	(Dee_funptr_t)&default__seq_parity_with_key__with_callattr_parity,
	(Dee_funptr_t)&default__seq_parity_with_key__with_callattr___seq_parity__,
	(Dee_funptr_t)&default__seq_parity_with_key__unsupported,
	(Dee_funptr_t)&default__seq_parity_with_key__empty,
	(Dee_funptr_t)&default__seq_parity_with_key__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_parity_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_parity_with_key__with_callobjectcache___seq_parity__, &tdefault__seq_parity_with_key__with_callobjectcache___seq_parity__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_parity_with_range[7] = {
	(Dee_funptr_t)&default__seq_parity_with_range__with_callattr_parity,
	(Dee_funptr_t)&default__seq_parity_with_range__with_callattr___seq_parity__,
	(Dee_funptr_t)&default__seq_parity_with_range__unsupported,
	(Dee_funptr_t)&default__seq_parity_with_range__empty,
	(Dee_funptr_t)&default__seq_parity_with_range__with__seq_count_with_range,
	(Dee_funptr_t)&default__seq_parity_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_parity_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_parity_with_range__with_callobjectcache___seq_parity__, &tdefault__seq_parity_with_range__with_callobjectcache___seq_parity__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_parity_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_parity_with_range_and_key__with_callattr_parity,
	(Dee_funptr_t)&default__seq_parity_with_range_and_key__with_callattr___seq_parity__,
	(Dee_funptr_t)&default__seq_parity_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_parity_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_parity_with_range_and_key__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_parity_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__, &tdefault__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_reduce[6] = {
	(Dee_funptr_t)&default__seq_reduce__with_callattr_reduce,
	(Dee_funptr_t)&default__seq_reduce__with_callattr___seq_reduce__,
	(Dee_funptr_t)&default__seq_reduce__unsupported,
	(Dee_funptr_t)&default__seq_reduce__empty,
	(Dee_funptr_t)&default__seq_reduce__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_reduce[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_reduce__with_callobjectcache___seq_reduce__, &tdefault__seq_reduce__with_callobjectcache___seq_reduce__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_reduce_with_init[6] = {
	(Dee_funptr_t)&default__seq_reduce_with_init__with_callattr_reduce,
	(Dee_funptr_t)&default__seq_reduce_with_init__with_callattr___seq_reduce__,
	(Dee_funptr_t)&default__seq_reduce_with_init__unsupported,
	(Dee_funptr_t)&default__seq_reduce_with_init__empty,
	(Dee_funptr_t)&default__seq_reduce_with_init__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_reduce_with_init[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_reduce_with_init__with_callobjectcache___seq_reduce__, &tdefault__seq_reduce_with_init__with_callobjectcache___seq_reduce__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_reduce_with_range[6] = {
	(Dee_funptr_t)&default__seq_reduce_with_range__with_callattr_reduce,
	(Dee_funptr_t)&default__seq_reduce_with_range__with_callattr___seq_reduce__,
	(Dee_funptr_t)&default__seq_reduce_with_range__unsupported,
	(Dee_funptr_t)&default__seq_reduce_with_range__empty,
	(Dee_funptr_t)&default__seq_reduce_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_reduce_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_reduce_with_range__with_callobjectcache___seq_reduce__, &tdefault__seq_reduce_with_range__with_callobjectcache___seq_reduce__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_reduce_with_range_and_init[6] = {
	(Dee_funptr_t)&default__seq_reduce_with_range_and_init__with_callattr_reduce,
	(Dee_funptr_t)&default__seq_reduce_with_range_and_init__with_callattr___seq_reduce__,
	(Dee_funptr_t)&default__seq_reduce_with_range_and_init__unsupported,
	(Dee_funptr_t)&default__seq_reduce_with_range_and_init__empty,
	(Dee_funptr_t)&default__seq_reduce_with_range_and_init__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_reduce_with_range_and_init[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_reduce_with_range_and_init__with_callobjectcache___seq_reduce__, &tdefault__seq_reduce_with_range_and_init__with_callobjectcache___seq_reduce__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_min[6] = {
	(Dee_funptr_t)&default__seq_min__with_callattr_min,
	(Dee_funptr_t)&default__seq_min__with_callattr___seq_min__,
	(Dee_funptr_t)&default__seq_min__unsupported,
	(Dee_funptr_t)&default__seq_min__empty,
	(Dee_funptr_t)&default__seq_min__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_min[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_min__with_callobjectcache___seq_min__, &tdefault__seq_min__with_callobjectcache___seq_min__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_min_with_key[6] = {
	(Dee_funptr_t)&default__seq_min_with_key__with_callattr_min,
	(Dee_funptr_t)&default__seq_min_with_key__with_callattr___seq_min__,
	(Dee_funptr_t)&default__seq_min_with_key__unsupported,
	(Dee_funptr_t)&default__seq_min_with_key__empty,
	(Dee_funptr_t)&default__seq_min_with_key__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_min_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_min_with_key__with_callobjectcache___seq_min__, &tdefault__seq_min_with_key__with_callobjectcache___seq_min__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_min_with_range[6] = {
	(Dee_funptr_t)&default__seq_min_with_range__with_callattr_min,
	(Dee_funptr_t)&default__seq_min_with_range__with_callattr___seq_min__,
	(Dee_funptr_t)&default__seq_min_with_range__unsupported,
	(Dee_funptr_t)&default__seq_min_with_range__empty,
	(Dee_funptr_t)&default__seq_min_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_min_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_min_with_range__with_callobjectcache___seq_min__, &tdefault__seq_min_with_range__with_callobjectcache___seq_min__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_min_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_min_with_range_and_key__with_callattr_min,
	(Dee_funptr_t)&default__seq_min_with_range_and_key__with_callattr___seq_min__,
	(Dee_funptr_t)&default__seq_min_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_min_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_min_with_range_and_key__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_min_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_min_with_range_and_key__with_callobjectcache___seq_min__, &tdefault__seq_min_with_range_and_key__with_callobjectcache___seq_min__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_max[6] = {
	(Dee_funptr_t)&default__seq_max__with_callattr_max,
	(Dee_funptr_t)&default__seq_max__with_callattr___seq_max__,
	(Dee_funptr_t)&default__seq_max__unsupported,
	(Dee_funptr_t)&default__seq_max__empty,
	(Dee_funptr_t)&default__seq_max__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_max[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_max__with_callobjectcache___seq_max__, &tdefault__seq_max__with_callobjectcache___seq_max__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_max_with_key[6] = {
	(Dee_funptr_t)&default__seq_max_with_key__with_callattr_max,
	(Dee_funptr_t)&default__seq_max_with_key__with_callattr___seq_max__,
	(Dee_funptr_t)&default__seq_max_with_key__unsupported,
	(Dee_funptr_t)&default__seq_max_with_key__empty,
	(Dee_funptr_t)&default__seq_max_with_key__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_max_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_max_with_key__with_callobjectcache___seq_max__, &tdefault__seq_max_with_key__with_callobjectcache___seq_max__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_max_with_range[6] = {
	(Dee_funptr_t)&default__seq_max_with_range__with_callattr_max,
	(Dee_funptr_t)&default__seq_max_with_range__with_callattr___seq_max__,
	(Dee_funptr_t)&default__seq_max_with_range__unsupported,
	(Dee_funptr_t)&default__seq_max_with_range__empty,
	(Dee_funptr_t)&default__seq_max_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_max_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_max_with_range__with_callobjectcache___seq_max__, &tdefault__seq_max_with_range__with_callobjectcache___seq_max__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_max_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_max_with_range_and_key__with_callattr_max,
	(Dee_funptr_t)&default__seq_max_with_range_and_key__with_callattr___seq_max__,
	(Dee_funptr_t)&default__seq_max_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_max_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_max_with_range_and_key__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_max_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_max_with_range_and_key__with_callobjectcache___seq_max__, &tdefault__seq_max_with_range_and_key__with_callobjectcache___seq_max__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_sum[6] = {
	(Dee_funptr_t)&default__seq_sum__with_callattr_sum,
	(Dee_funptr_t)&default__seq_sum__with_callattr___seq_sum__,
	(Dee_funptr_t)&default__seq_sum__unsupported,
	(Dee_funptr_t)&default__seq_sum__empty,
	(Dee_funptr_t)&default__seq_sum__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_sum[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_sum__with_callobjectcache___seq_sum__, &tdefault__seq_sum__with_callobjectcache___seq_sum__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_sum_with_range[6] = {
	(Dee_funptr_t)&default__seq_sum_with_range__with_callattr_sum,
	(Dee_funptr_t)&default__seq_sum_with_range__with_callattr___seq_sum__,
	(Dee_funptr_t)&default__seq_sum_with_range__unsupported,
	(Dee_funptr_t)&default__seq_sum_with_range__empty,
	(Dee_funptr_t)&default__seq_sum_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_sum_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_sum_with_range__with_callobjectcache___seq_sum__, &tdefault__seq_sum_with_range__with_callobjectcache___seq_sum__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_count[8] = {
	(Dee_funptr_t)&default__seq_count__with_callattr_count,
	(Dee_funptr_t)&default__seq_count__with_callattr___seq_count__,
	(Dee_funptr_t)&default__seq_count__unsupported,
	(Dee_funptr_t)&default__seq_count__empty,
	(Dee_funptr_t)&default__seq_count__with__set_operator_contains,
	(Dee_funptr_t)&default__seq_count__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_count__with__seq_find,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_count[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_count__with_callobjectcache___seq_count__, &tdefault__seq_count__with_callobjectcache___seq_count__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_count_with_key[7] = {
	(Dee_funptr_t)&default__seq_count_with_key__with_callattr_count,
	(Dee_funptr_t)&default__seq_count_with_key__with_callattr___seq_count__,
	(Dee_funptr_t)&default__seq_count_with_key__unsupported,
	(Dee_funptr_t)&default__seq_count_with_key__empty,
	(Dee_funptr_t)&default__seq_count_with_key__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_count_with_key__with__seq_find_with_key,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_count_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_count_with_key__with_callobjectcache___seq_count__, &tdefault__seq_count_with_key__with_callobjectcache___seq_count__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_count_with_range[7] = {
	(Dee_funptr_t)&default__seq_count_with_range__with_callattr_count,
	(Dee_funptr_t)&default__seq_count_with_range__with_callattr___seq_count__,
	(Dee_funptr_t)&default__seq_count_with_range__unsupported,
	(Dee_funptr_t)&default__seq_count_with_range__empty,
	(Dee_funptr_t)&default__seq_count_with_range__with__seq_enumerate_index,
	(Dee_funptr_t)&default__seq_count_with_range__with__seq_find,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_count_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_count_with_range__with_callobjectcache___seq_count__, &tdefault__seq_count_with_range__with_callobjectcache___seq_count__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_count_with_range_and_key[7] = {
	(Dee_funptr_t)&default__seq_count_with_range_and_key__with_callattr_count,
	(Dee_funptr_t)&default__seq_count_with_range_and_key__with_callattr___seq_count__,
	(Dee_funptr_t)&default__seq_count_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_count_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_count_with_range_and_key__with__seq_enumerate_index,
	(Dee_funptr_t)&default__seq_count_with_range_and_key__with__seq_find_with_key,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_count_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_count_with_range_and_key__with_callobjectcache___seq_count__, &tdefault__seq_count_with_range_and_key__with_callobjectcache___seq_count__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_contains[8] = {
	(Dee_funptr_t)&default__seq_contains__with_callattr___seq_contains__,
	(Dee_funptr_t)&default__seq_contains__unsupported,
	(Dee_funptr_t)&default__seq_contains__with__seq_operator_contains,
	(Dee_funptr_t)&default__seq_contains__empty,
	(Dee_funptr_t)&default__seq_contains__with__map_operator_trygetitem,
	(Dee_funptr_t)&default__seq_contains__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_contains__with__seq_find,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_contains[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_contains__with_callobjectcache___seq_contains__, &tdefault__seq_contains__with_callobjectcache___seq_contains__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_contains_with_key[6] = {
	(Dee_funptr_t)&default__seq_contains_with_key__with_callattr___seq_contains__,
	(Dee_funptr_t)&default__seq_contains_with_key__unsupported,
	(Dee_funptr_t)&default__seq_contains_with_key__empty,
	(Dee_funptr_t)&default__seq_contains_with_key__with__seq_operator_foreach,
	(Dee_funptr_t)&default__seq_contains_with_key__with__seq_find_with_key,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_contains_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_contains_with_key__with_callobjectcache___seq_contains__, &tdefault__seq_contains_with_key__with_callobjectcache___seq_contains__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_contains_with_range[6] = {
	(Dee_funptr_t)&default__seq_contains_with_range__with_callattr___seq_contains__,
	(Dee_funptr_t)&default__seq_contains_with_range__unsupported,
	(Dee_funptr_t)&default__seq_contains_with_range__empty,
	(Dee_funptr_t)&default__seq_contains_with_range__with__seq_enumerate_index,
	(Dee_funptr_t)&default__seq_contains_with_range__with__seq_find,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_contains_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_contains_with_range__with_callobjectcache___seq_contains__, &tdefault__seq_contains_with_range__with_callobjectcache___seq_contains__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_contains_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_contains_with_range_and_key__with_callattr___seq_contains__,
	(Dee_funptr_t)&default__seq_contains_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_contains_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_contains_with_range_and_key__with__seq_enumerate_index,
	(Dee_funptr_t)&default__seq_contains_with_range_and_key__with__seq_find_with_key,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_contains_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__, &tdefault__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_operator_contains[5] = {
	(Dee_funptr_t)&default__seq_operator_contains__with_callattr___seq_contains__,
	(Dee_funptr_t)&default__seq_operator_contains__unsupported,
	(Dee_funptr_t)&default__seq_operator_contains__with__seq_contains,
	(Dee_funptr_t)&default__seq_operator_contains__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_operator_contains[2] = {
	MH_SUPER_MAP_TYPED_INIT(&usrtype__contains__with__CONTAINS, &tusrtype__contains__with__CONTAINS),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_locate[6] = {
	(Dee_funptr_t)&default__seq_locate__with_callattr_locate,
	(Dee_funptr_t)&default__seq_locate__with_callattr___seq_locate__,
	(Dee_funptr_t)&default__seq_locate__unsupported,
	(Dee_funptr_t)&default__seq_locate__empty,
	(Dee_funptr_t)&default__seq_locate__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_locate[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_locate__with_callobjectcache___seq_locate__, &tdefault__seq_locate__with_callobjectcache___seq_locate__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_locate_with_range[6] = {
	(Dee_funptr_t)&default__seq_locate_with_range__with_callattr_locate,
	(Dee_funptr_t)&default__seq_locate_with_range__with_callattr___seq_locate__,
	(Dee_funptr_t)&default__seq_locate_with_range__unsupported,
	(Dee_funptr_t)&default__seq_locate_with_range__empty,
	(Dee_funptr_t)&default__seq_locate_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_locate_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_locate_with_range__with_callobjectcache___seq_locate__, &tdefault__seq_locate_with_range__with_callobjectcache___seq_locate__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_rlocate[7] = {
	(Dee_funptr_t)&default__seq_rlocate__with_callattr_rlocate,
	(Dee_funptr_t)&default__seq_rlocate__with_callattr___seq_rlocate__,
	(Dee_funptr_t)&default__seq_rlocate__unsupported,
	(Dee_funptr_t)&default__seq_rlocate__empty,
	(Dee_funptr_t)&default__seq_rlocate__with__seq_foreach_reverse,
	(Dee_funptr_t)&default__seq_rlocate__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_rlocate[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_rlocate__with_callobjectcache___seq_rlocate__, &tdefault__seq_rlocate__with_callobjectcache___seq_rlocate__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_rlocate_with_range[7] = {
	(Dee_funptr_t)&default__seq_rlocate_with_range__with_callattr_rlocate,
	(Dee_funptr_t)&default__seq_rlocate_with_range__with_callattr___seq_rlocate__,
	(Dee_funptr_t)&default__seq_rlocate_with_range__unsupported,
	(Dee_funptr_t)&default__seq_rlocate_with_range__empty,
	(Dee_funptr_t)&default__seq_rlocate_with_range__with__seq_enumerate_index_reverse,
	(Dee_funptr_t)&default__seq_rlocate_with_range__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_rlocate_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__, &tdefault__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_startswith[6] = {
	(Dee_funptr_t)&default__seq_startswith__with_callattr_startswith,
	(Dee_funptr_t)&default__seq_startswith__with_callattr___seq_startswith__,
	(Dee_funptr_t)&default__seq_startswith__unsupported,
	(Dee_funptr_t)&default__seq_startswith__empty,
	(Dee_funptr_t)&default__seq_startswith__with__seq_trygetfirst,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_startswith[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_startswith__with_callobjectcache___seq_startswith__, &tdefault__seq_startswith__with_callobjectcache___seq_startswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_startswith_with_key[6] = {
	(Dee_funptr_t)&default__seq_startswith_with_key__with_callattr_startswith,
	(Dee_funptr_t)&default__seq_startswith_with_key__with_callattr___seq_startswith__,
	(Dee_funptr_t)&default__seq_startswith_with_key__unsupported,
	(Dee_funptr_t)&default__seq_startswith_with_key__empty,
	(Dee_funptr_t)&default__seq_startswith_with_key__with__seq_trygetfirst,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_startswith_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_startswith_with_key__with_callobjectcache___seq_startswith__, &tdefault__seq_startswith_with_key__with_callobjectcache___seq_startswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_startswith_with_range[6] = {
	(Dee_funptr_t)&default__seq_startswith_with_range__with_callattr_startswith,
	(Dee_funptr_t)&default__seq_startswith_with_range__with_callattr___seq_startswith__,
	(Dee_funptr_t)&default__seq_startswith_with_range__unsupported,
	(Dee_funptr_t)&default__seq_startswith_with_range__empty,
	(Dee_funptr_t)&default__seq_startswith_with_range__with__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_startswith_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_startswith_with_range__with_callobjectcache___seq_startswith__, &tdefault__seq_startswith_with_range__with_callobjectcache___seq_startswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_startswith_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_startswith_with_range_and_key__with_callattr_startswith,
	(Dee_funptr_t)&default__seq_startswith_with_range_and_key__with_callattr___seq_startswith__,
	(Dee_funptr_t)&default__seq_startswith_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_startswith_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_startswith_with_range_and_key__with__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_startswith_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__, &tdefault__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_endswith[6] = {
	(Dee_funptr_t)&default__seq_endswith__with_callattr_endswith,
	(Dee_funptr_t)&default__seq_endswith__with_callattr___seq_endswith__,
	(Dee_funptr_t)&default__seq_endswith__unsupported,
	(Dee_funptr_t)&default__seq_endswith__empty,
	(Dee_funptr_t)&default__seq_endswith__with__seq_trygetlast,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_endswith[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_endswith__with_callobjectcache___seq_endswith__, &tdefault__seq_endswith__with_callobjectcache___seq_endswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_endswith_with_key[6] = {
	(Dee_funptr_t)&default__seq_endswith_with_key__with_callattr_endswith,
	(Dee_funptr_t)&default__seq_endswith_with_key__with_callattr___seq_endswith__,
	(Dee_funptr_t)&default__seq_endswith_with_key__unsupported,
	(Dee_funptr_t)&default__seq_endswith_with_key__empty,
	(Dee_funptr_t)&default__seq_endswith_with_key__with__seq_trygetlast,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_endswith_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_endswith_with_key__with_callobjectcache___seq_endswith__, &tdefault__seq_endswith_with_key__with_callobjectcache___seq_endswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_endswith_with_range[6] = {
	(Dee_funptr_t)&default__seq_endswith_with_range__with_callattr_endswith,
	(Dee_funptr_t)&default__seq_endswith_with_range__with_callattr___seq_endswith__,
	(Dee_funptr_t)&default__seq_endswith_with_range__unsupported,
	(Dee_funptr_t)&default__seq_endswith_with_range__empty,
	(Dee_funptr_t)&default__seq_endswith_with_range__with__seq_operator_size__and__operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_endswith_with_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_endswith_with_range__with_callobjectcache___seq_endswith__, &tdefault__seq_endswith_with_range__with_callobjectcache___seq_endswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_endswith_with_range_and_key[6] = {
	(Dee_funptr_t)&default__seq_endswith_with_range_and_key__with_callattr_endswith,
	(Dee_funptr_t)&default__seq_endswith_with_range_and_key__with_callattr___seq_endswith__,
	(Dee_funptr_t)&default__seq_endswith_with_range_and_key__unsupported,
	(Dee_funptr_t)&default__seq_endswith_with_range_and_key__empty,
	(Dee_funptr_t)&default__seq_endswith_with_range_and_key__with__seq_operator_size__and__operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_endswith_with_range_and_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__, &tdefault__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_find[6] = {
	(Dee_funptr_t)&default__seq_find__with_callattr_find,
	(Dee_funptr_t)&default__seq_find__with_callattr___seq_find__,
	(Dee_funptr_t)&default__seq_find__unsupported,
	(Dee_funptr_t)&default__seq_find__empty,
	(Dee_funptr_t)&default__seq_find__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_find[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_find__with_callobjectcache___seq_find__, &tdefault__seq_find__with_callobjectcache___seq_find__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_find_with_key[6] = {
	(Dee_funptr_t)&default__seq_find_with_key__with_callattr_find,
	(Dee_funptr_t)&default__seq_find_with_key__with_callattr___seq_find__,
	(Dee_funptr_t)&default__seq_find_with_key__unsupported,
	(Dee_funptr_t)&default__seq_find_with_key__empty,
	(Dee_funptr_t)&default__seq_find_with_key__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_find_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_find_with_key__with_callobjectcache___seq_find__, &tdefault__seq_find_with_key__with_callobjectcache___seq_find__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_rfind[7] = {
	(Dee_funptr_t)&default__seq_rfind__with_callattr_rfind,
	(Dee_funptr_t)&default__seq_rfind__with_callattr___seq_rfind__,
	(Dee_funptr_t)&default__seq_rfind__unsupported,
	(Dee_funptr_t)&default__seq_rfind__empty,
	(Dee_funptr_t)&default__seq_rfind__with__seq_enumerate_index_reverse,
	(Dee_funptr_t)&default__seq_rfind__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_rfind[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_rfind__with_callobjectcache___seq_rfind__, &tdefault__seq_rfind__with_callobjectcache___seq_rfind__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_rfind_with_key[7] = {
	(Dee_funptr_t)&default__seq_rfind_with_key__with_callattr_rfind,
	(Dee_funptr_t)&default__seq_rfind_with_key__with_callattr___seq_rfind__,
	(Dee_funptr_t)&default__seq_rfind_with_key__unsupported,
	(Dee_funptr_t)&default__seq_rfind_with_key__empty,
	(Dee_funptr_t)&default__seq_rfind_with_key__with__seq_enumerate_index_reverse,
	(Dee_funptr_t)&default__seq_rfind_with_key__with__seq_enumerate_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_rfind_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_rfind_with_key__with_callobjectcache___seq_rfind__, &tdefault__seq_rfind_with_key__with_callobjectcache___seq_rfind__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_erase[7] = {
	(Dee_funptr_t)&default__seq_erase__with_callattr_erase,
	(Dee_funptr_t)&default__seq_erase__with_callattr___seq_erase__,
	(Dee_funptr_t)&default__seq_erase__unsupported,
	(Dee_funptr_t)&default__seq_erase__empty,
	(Dee_funptr_t)&default__seq_erase__with__seq_operator_delrange_index,
	(Dee_funptr_t)&default__seq_erase__with__seq_pop,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_erase[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_erase__with_callobjectcache___seq_erase__, &tdefault__seq_erase__with_callobjectcache___seq_erase__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_insert[6] = {
	(Dee_funptr_t)&default__seq_insert__with_callattr_insert,
	(Dee_funptr_t)&default__seq_insert__with_callattr___seq_insert__,
	(Dee_funptr_t)&default__seq_insert__unsupported,
	(Dee_funptr_t)&default__seq_insert__empty,
	(Dee_funptr_t)&default__seq_insert__with__seq_insertall,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_insert[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_insert__with_callobjectcache___seq_insert__, &tdefault__seq_insert__with_callobjectcache___seq_insert__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_insertall[7] = {
	(Dee_funptr_t)&default__seq_insertall__with_callattr_insertall,
	(Dee_funptr_t)&default__seq_insertall__with_callattr___seq_insertall__,
	(Dee_funptr_t)&default__seq_insertall__unsupported,
	(Dee_funptr_t)&default__seq_insertall__empty,
	(Dee_funptr_t)&default__seq_insertall__with__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_insertall__with__seq_insert,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_insertall[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_insertall__with_callobjectcache___seq_insertall__, &tdefault__seq_insertall__with_callobjectcache___seq_insertall__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_pushfront[6] = {
	(Dee_funptr_t)&default__seq_pushfront__with_callattr_pushfront,
	(Dee_funptr_t)&default__seq_pushfront__with_callattr___seq_pushfront__,
	(Dee_funptr_t)&default__seq_pushfront__unsupported,
	(Dee_funptr_t)&default__seq_pushfront__empty,
	(Dee_funptr_t)&default__seq_pushfront__with__seq_insert,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_pushfront[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_pushfront__with_callobjectcache___seq_pushfront__, &tdefault__seq_pushfront__with_callobjectcache___seq_pushfront__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_append[7] = {
	(Dee_funptr_t)&default__seq_append__with_callattr_append,
	(Dee_funptr_t)&default__seq_append__with_callattr_pushback,
	(Dee_funptr_t)&default__seq_append__with_callattr___seq_append__,
	(Dee_funptr_t)&default__seq_append__unsupported,
	(Dee_funptr_t)&default__seq_append__empty,
	(Dee_funptr_t)&default__seq_append__with__seq_extend,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_append[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_append__with_callobjectcache___seq_append__, &tdefault__seq_append__with_callobjectcache___seq_append__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_extend[8] = {
	(Dee_funptr_t)&default__seq_extend__with_callattr_extend,
	(Dee_funptr_t)&default__seq_extend__with_callattr___seq_extend__,
	(Dee_funptr_t)&default__seq_extend__unsupported,
	(Dee_funptr_t)&default__seq_extend__empty,
	(Dee_funptr_t)&default__seq_extend__with__seq_operator_size__and__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_extend__with__seq_operator_size__and__seq_insertall,
	(Dee_funptr_t)&default__seq_extend__with__seq_append,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_extend[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_extend__with_callobjectcache___seq_extend__, &tdefault__seq_extend__with_callobjectcache___seq_extend__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_xchitem_index[6] = {
	(Dee_funptr_t)&default__seq_xchitem_index__with_callattr_xchitem,
	(Dee_funptr_t)&default__seq_xchitem_index__with_callattr___seq_xchitem__,
	(Dee_funptr_t)&default__seq_xchitem_index__unsupported,
	(Dee_funptr_t)&default__seq_xchitem_index__empty,
	(Dee_funptr_t)&default__seq_xchitem_index__with__seq_operator_getitem_index__and__seq_operator_setitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_xchitem_index[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_xchitem_index__with_callobjectcache___seq_xchitem__, &tdefault__seq_xchitem_index__with_callobjectcache___seq_xchitem__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_clear[8] = {
	(Dee_funptr_t)&default__seq_clear__with_callattr_clear,
	(Dee_funptr_t)&default__seq_clear__with_callattr___seq_clear__,
	(Dee_funptr_t)&default__seq_clear__unsupported,
	(Dee_funptr_t)&default__seq_clear__empty,
	(Dee_funptr_t)&default__seq_clear__with__seq_operator_delrange,
	(Dee_funptr_t)&default__seq_clear__with__seq_operator_delrange_index_n,
	(Dee_funptr_t)&default__seq_clear__with__seq_erase,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_clear[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_clear__with_callobjectcache___seq_clear__, &tdefault__seq_clear__with_callobjectcache___seq_clear__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_pop[7] = {
	(Dee_funptr_t)&default__seq_pop__with_callattr_pop,
	(Dee_funptr_t)&default__seq_pop__with_callattr___seq_pop__,
	(Dee_funptr_t)&default__seq_pop__unsupported,
	(Dee_funptr_t)&default__seq_pop__empty,
	(Dee_funptr_t)&default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase,
	(Dee_funptr_t)&default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_pop[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_pop__with_callobjectcache___seq_pop__, &tdefault__seq_pop__with_callobjectcache___seq_pop__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_remove[8] = {
	(Dee_funptr_t)&default__seq_remove__with_callattr_remove,
	(Dee_funptr_t)&default__seq_remove__with_callattr___seq_remove__,
	(Dee_funptr_t)&default__seq_remove__unsupported,
	(Dee_funptr_t)&default__seq_remove__empty,
	(Dee_funptr_t)&default__seq_remove__with__seq_removeall,
	(Dee_funptr_t)&default__seq_remove__with__seq_enumerate_index__and__seq_operator_delitem_index,
	(Dee_funptr_t)&default__seq_remove__with__seq_find__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_remove[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_remove__with_callobjectcache___seq_remove__, &tdefault__seq_remove__with_callobjectcache___seq_remove__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_remove_with_key[8] = {
	(Dee_funptr_t)&default__seq_remove_with_key__with_callattr_remove,
	(Dee_funptr_t)&default__seq_remove_with_key__with_callattr___seq_remove__,
	(Dee_funptr_t)&default__seq_remove_with_key__unsupported,
	(Dee_funptr_t)&default__seq_remove_with_key__empty,
	(Dee_funptr_t)&default__seq_remove_with_key__with__seq_removeall,
	(Dee_funptr_t)&default__seq_remove_with_key__with__seq_enumerate_index__and__seq_operator_delitem_index,
	(Dee_funptr_t)&default__seq_remove_with_key__with__seq_find_with_key__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_remove_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_remove_with_key__with_callobjectcache___seq_remove__, &tdefault__seq_remove_with_key__with_callobjectcache___seq_remove__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_rremove[7] = {
	(Dee_funptr_t)&default__seq_rremove__with_callattr_rremove,
	(Dee_funptr_t)&default__seq_rremove__with_callattr___seq_rremove__,
	(Dee_funptr_t)&default__seq_rremove__unsupported,
	(Dee_funptr_t)&default__seq_rremove__empty,
	(Dee_funptr_t)&default__seq_rremove__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index,
	(Dee_funptr_t)&default__seq_rremove__with__seq_rfind__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_rremove[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_rremove__with_callobjectcache___seq_rremove__, &tdefault__seq_rremove__with_callobjectcache___seq_rremove__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_rremove_with_key[7] = {
	(Dee_funptr_t)&default__seq_rremove_with_key__with_callattr_rremove,
	(Dee_funptr_t)&default__seq_rremove_with_key__with_callattr___seq_rremove__,
	(Dee_funptr_t)&default__seq_rremove_with_key__unsupported,
	(Dee_funptr_t)&default__seq_rremove_with_key__empty,
	(Dee_funptr_t)&default__seq_rremove_with_key__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index,
	(Dee_funptr_t)&default__seq_rremove_with_key__with__seq_rfind_with_key__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_rremove_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_rremove_with_key__with_callobjectcache___seq_rremove__, &tdefault__seq_rremove_with_key__with_callobjectcache___seq_rremove__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_removeall[9] = {
	(Dee_funptr_t)&default__seq_removeall__with_callattr_removeall,
	(Dee_funptr_t)&default__seq_removeall__with_callattr___seq_removeall__,
	(Dee_funptr_t)&default__seq_removeall__unsupported,
	(Dee_funptr_t)&default__seq_removeall__empty,
	(Dee_funptr_t)&default__seq_removeall__with__seq_removeif,
	(Dee_funptr_t)&default__seq_removeall__with__seq_operator_size__and__seq_remove,
	(Dee_funptr_t)&default__seq_removeall__with__seq_remove__once,
	(Dee_funptr_t)&default__seq_removeall__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_removeall[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_removeall__with_callobjectcache___seq_removeall__, &tdefault__seq_removeall__with_callobjectcache___seq_removeall__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_removeall_with_key[9] = {
	(Dee_funptr_t)&default__seq_removeall_with_key__with_callattr_removeall,
	(Dee_funptr_t)&default__seq_removeall_with_key__with_callattr___seq_removeall__,
	(Dee_funptr_t)&default__seq_removeall_with_key__unsupported,
	(Dee_funptr_t)&default__seq_removeall_with_key__empty,
	(Dee_funptr_t)&default__seq_removeall_with_key__with__seq_removeif,
	(Dee_funptr_t)&default__seq_removeall_with_key__with__seq_operator_size__and__seq_remove_with_key,
	(Dee_funptr_t)&default__seq_removeall_with_key__with__seq_remove_with_key__once,
	(Dee_funptr_t)&default__seq_removeall_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_removeall_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_removeall_with_key__with_callobjectcache___seq_removeall__, &tdefault__seq_removeall_with_key__with_callobjectcache___seq_removeall__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_removeif[7] = {
	(Dee_funptr_t)&default__seq_removeif__with_callattr_removeif,
	(Dee_funptr_t)&default__seq_removeif__with_callattr___seq_removeif__,
	(Dee_funptr_t)&default__seq_removeif__unsupported,
	(Dee_funptr_t)&default__seq_removeif__empty,
	(Dee_funptr_t)&default__seq_removeif__with__seq_removeall_with_key,
	(Dee_funptr_t)&default__seq_removeif__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_removeif[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_removeif__with_callobjectcache___seq_removeif__, &tdefault__seq_removeif__with_callobjectcache___seq_removeif__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_resize[8] = {
	(Dee_funptr_t)&default__seq_resize__with_callattr_resize,
	(Dee_funptr_t)&default__seq_resize__with_callattr___seq_resize__,
	(Dee_funptr_t)&default__seq_resize__unsupported,
	(Dee_funptr_t)&default__seq_resize__empty,
	(Dee_funptr_t)&default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index__and__seq_operator_delrange_index,
	(Dee_funptr_t)&default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_resize__with__seq_operator_size__and__seq_erase__and__seq_extend,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_resize[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_resize__with_callobjectcache___seq_resize__, &tdefault__seq_resize__with_callobjectcache___seq_resize__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_fill[7] = {
	(Dee_funptr_t)&default__seq_fill__with_callattr_fill,
	(Dee_funptr_t)&default__seq_fill__with_callattr___seq_fill__,
	(Dee_funptr_t)&default__seq_fill__unsupported,
	(Dee_funptr_t)&default__seq_fill__empty,
	(Dee_funptr_t)&default__seq_fill__with__seq_operator_size__and__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_fill__with__seq_enumerate_index__and__seq_operator_setitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_fill[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_fill__with_callobjectcache___seq_fill__, &tdefault__seq_fill__with_callobjectcache___seq_fill__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_reverse[8] = {
	(Dee_funptr_t)&default__seq_reverse__with_callattr_reverse,
	(Dee_funptr_t)&default__seq_reverse__with_callattr___seq_reverse__,
	(Dee_funptr_t)&default__seq_reverse__unsupported,
	(Dee_funptr_t)&default__seq_reverse__empty,
	(Dee_funptr_t)&default__seq_reverse__with__seq_reversed__and__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index__and__seq_operator_delitem_index,
	(Dee_funptr_t)&default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_reverse[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_reverse__with_callobjectcache___seq_reverse__, &tdefault__seq_reverse__with_callobjectcache___seq_reverse__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_reversed[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_reversed__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_reversed__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_reversed[8] = {
	(Dee_funptr_t)&default__seq_reversed__with_callattr_reversed,
	(Dee_funptr_t)&default__seq_reversed__with_callattr___seq_reversed__,
	(Dee_funptr_t)&default__seq_reversed__unsupported,
	(Dee_funptr_t)&default__seq_reversed__empty,
	(Dee_funptr_t)&default__seq_reversed__with__seq_operator_size__and__seq_operator_getitem_index,
	(Dee_funptr_t)&default__seq_reversed__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_reversed__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_reversed[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_reversed__with_callobjectcache___seq_reversed__, &tdefault__seq_reversed__with_callobjectcache___seq_reversed__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_sort[7] = {
	(Dee_funptr_t)&default__seq_sort__with_callattr_sort,
	(Dee_funptr_t)&default__seq_sort__with_callattr___seq_sort__,
	(Dee_funptr_t)&default__seq_sort__unsupported,
	(Dee_funptr_t)&default__seq_sort__empty,
	(Dee_funptr_t)&default__seq_sort__with__seq_sorted__and__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_sort__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_sort[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_sort__with_callobjectcache___seq_sort__, &tdefault__seq_sort__with_callobjectcache___seq_sort__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_sort_with_key[7] = {
	(Dee_funptr_t)&default__seq_sort_with_key__with_callattr_sort,
	(Dee_funptr_t)&default__seq_sort_with_key__with_callattr___seq_sort__,
	(Dee_funptr_t)&default__seq_sort_with_key__unsupported,
	(Dee_funptr_t)&default__seq_sort_with_key__empty,
	(Dee_funptr_t)&default__seq_sort_with_key__with__seq_sorted_with_key__and__seq_operator_setrange_index,
	(Dee_funptr_t)&default__seq_sort_with_key__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_sort_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_sort_with_key__with_callobjectcache___seq_sort__, &tdefault__seq_sort_with_key__with_callobjectcache___seq_sort__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_sorted[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_sorted__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_sorted__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_sorted[7] = {
	(Dee_funptr_t)&default__seq_sorted__with_callattr_sorted,
	(Dee_funptr_t)&default__seq_sorted__with_callattr___seq_sorted__,
	(Dee_funptr_t)&default__seq_sorted__unsupported,
	(Dee_funptr_t)&default__seq_sorted__empty,
	(Dee_funptr_t)&default__seq_sorted__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_sorted__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_sorted[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_sorted__with_callobjectcache___seq_sorted__, &tdefault__seq_sorted__with_callobjectcache___seq_sorted__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map_replace tpconst msm_replace__seq_sorted_with_key[2] = {
	MH_SUPER_MAP_REPLACE_INIT(&default__seq_sorted_with_key__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_sorted_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index),
	MH_SUPER_MAP_REPLACE_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_sorted_with_key[7] = {
	(Dee_funptr_t)&default__seq_sorted_with_key__with_callattr_sorted,
	(Dee_funptr_t)&default__seq_sorted_with_key__with_callattr___seq_sorted__,
	(Dee_funptr_t)&default__seq_sorted_with_key__unsupported,
	(Dee_funptr_t)&default__seq_sorted_with_key__empty,
	(Dee_funptr_t)&default__seq_sorted_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index,
	(Dee_funptr_t)&default__seq_sorted_with_key__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_sorted_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_sorted_with_key__with_callobjectcache___seq_sorted__, &tdefault__seq_sorted_with_key__with_callobjectcache___seq_sorted__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_bfind[6] = {
	(Dee_funptr_t)&default__seq_bfind__with_callattr_bfind,
	(Dee_funptr_t)&default__seq_bfind__with_callattr___seq_bfind__,
	(Dee_funptr_t)&default__seq_bfind__unsupported,
	(Dee_funptr_t)&default__seq_bfind__empty,
	(Dee_funptr_t)&default__seq_bfind__with__seq_operator_size__and__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_bfind[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_bfind__with_callobjectcache___seq_bfind__, &tdefault__seq_bfind__with_callobjectcache___seq_bfind__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_bfind_with_key[6] = {
	(Dee_funptr_t)&default__seq_bfind_with_key__with_callattr_bfind,
	(Dee_funptr_t)&default__seq_bfind_with_key__with_callattr___seq_bfind__,
	(Dee_funptr_t)&default__seq_bfind_with_key__unsupported,
	(Dee_funptr_t)&default__seq_bfind_with_key__empty,
	(Dee_funptr_t)&default__seq_bfind_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_bfind_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_bfind_with_key__with_callobjectcache___seq_bfind__, &tdefault__seq_bfind_with_key__with_callobjectcache___seq_bfind__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_bposition[6] = {
	(Dee_funptr_t)&default__seq_bposition__with_callattr_bposition,
	(Dee_funptr_t)&default__seq_bposition__with_callattr___seq_bposition__,
	(Dee_funptr_t)&default__seq_bposition__unsupported,
	(Dee_funptr_t)&default__seq_bposition__empty,
	(Dee_funptr_t)&default__seq_bposition__with__seq_operator_size__and__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_bposition[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_bposition__with_callobjectcache___seq_bposition__, &tdefault__seq_bposition__with_callobjectcache___seq_bposition__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_bposition_with_key[6] = {
	(Dee_funptr_t)&default__seq_bposition_with_key__with_callattr_bposition,
	(Dee_funptr_t)&default__seq_bposition_with_key__with_callattr___seq_bposition__,
	(Dee_funptr_t)&default__seq_bposition_with_key__unsupported,
	(Dee_funptr_t)&default__seq_bposition_with_key__empty,
	(Dee_funptr_t)&default__seq_bposition_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_bposition_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_bposition_with_key__with_callobjectcache___seq_bposition__, &tdefault__seq_bposition_with_key__with_callobjectcache___seq_bposition__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_brange[6] = {
	(Dee_funptr_t)&default__seq_brange__with_callattr_brange,
	(Dee_funptr_t)&default__seq_brange__with_callattr___seq_brange__,
	(Dee_funptr_t)&default__seq_brange__unsupported,
	(Dee_funptr_t)&default__seq_brange__empty,
	(Dee_funptr_t)&default__seq_brange__with__seq_operator_size__and__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_brange[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_brange__with_callobjectcache___seq_brange__, &tdefault__seq_brange__with_callobjectcache___seq_brange__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__seq_brange_with_key[6] = {
	(Dee_funptr_t)&default__seq_brange_with_key__with_callattr_brange,
	(Dee_funptr_t)&default__seq_brange_with_key__with_callattr___seq_brange__,
	(Dee_funptr_t)&default__seq_brange_with_key__unsupported,
	(Dee_funptr_t)&default__seq_brange_with_key__empty,
	(Dee_funptr_t)&default__seq_brange_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__seq_brange_with_key[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__seq_brange_with_key__with_callobjectcache___seq_brange__, &tdefault__seq_brange_with_key__with_callobjectcache___seq_brange__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_iter[5] = {
	(Dee_funptr_t)&default__set_operator_iter__with_callattr___set_iter__,
	(Dee_funptr_t)&default__set_operator_iter__unsupported,
	(Dee_funptr_t)&default__set_operator_iter__empty,
	(Dee_funptr_t)&default__set_operator_iter__with__seq_operator_iter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_iter[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_iter__with_callobjectcache___set_iter__, &tdefault__set_operator_iter__with_callobjectcache___set_iter__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__iter__with__ITER, &tusrtype__iter__with__ITER),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_foreach[6] = {
	(Dee_funptr_t)&default__set_operator_foreach__with_callattr___set_iter__,
	(Dee_funptr_t)&default__set_operator_foreach__unsupported,
	(Dee_funptr_t)&default__set_operator_foreach__with__set_operator_iter,
	(Dee_funptr_t)&default__set_operator_foreach__empty,
	(Dee_funptr_t)&default__set_operator_foreach__with__seq_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_foreach[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__foreach__with__iter, &tdefault__foreach__with__iter),
	MH_SUPER_MAP_TYPED_INIT(&default__foreach__with__foreach_pair, &tdefault__foreach__with__foreach_pair),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_foreach_pair[5] = {
	(Dee_funptr_t)&default__set_operator_foreach_pair__with_callattr___set_iter__,
	(Dee_funptr_t)&default__set_operator_foreach_pair__unsupported,
	(Dee_funptr_t)&default__set_operator_foreach_pair__with__set_operator_foreach,
	(Dee_funptr_t)&default__set_operator_foreach_pair__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_foreach_pair[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__foreach_pair__with__foreach, &tdefault__foreach_pair__with__foreach),
	MH_SUPER_MAP_TYPED_INIT(&default__foreach_pair__with__iter, &tdefault__foreach_pair__with__iter),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_sizeob[5] = {
	(Dee_funptr_t)&default__set_operator_sizeob__with_callattr___set_size__,
	(Dee_funptr_t)&default__set_operator_sizeob__unsupported,
	(Dee_funptr_t)&default__set_operator_sizeob__with__set_operator_size,
	(Dee_funptr_t)&default__set_operator_sizeob__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_sizeob[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_sizeob__with_callobjectcache___set_size__, &tdefault__set_operator_sizeob__with_callobjectcache___set_size__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__sizeob__with__SIZE, &tusrtype__sizeob__with__SIZE),
	MH_SUPER_MAP_TYPED_INIT(&default__sizeob__with__size, &tdefault__sizeob__with__size),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_size[6] = {
	(Dee_funptr_t)&default__set_operator_size__with_callattr___set_size__,
	(Dee_funptr_t)&default__set_operator_size__unsupported,
	(Dee_funptr_t)&default__set_operator_size__with__set_operator_sizeob,
	(Dee_funptr_t)&default__set_operator_size__empty,
	(Dee_funptr_t)&default__set_operator_size__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_size[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__size__with__sizeob, &tdefault__size__with__sizeob),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_hash[5] = {
	(Dee_funptr_t)&default__set_operator_hash__with_callattr___set_hash__,
	(Dee_funptr_t)&default__set_operator_hash__unsupported,
	(Dee_funptr_t)&default__set_operator_hash__empty,
	(Dee_funptr_t)&default__set_operator_hash__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_hash[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_hash__with_callobjectcache___set_hash__, &tdefault__set_operator_hash__with_callobjectcache___set_hash__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__hash__with__HASH, &tusrtype__hash__with__HASH),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__hash__with__, &tusrtype__hash__with__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_compare_eq[7] = {
	(Dee_funptr_t)&default__set_operator_compare_eq__with_callattr___set_compare_eq__,
	(Dee_funptr_t)&default__set_operator_compare_eq__unsupported,
	(Dee_funptr_t)&default__set_operator_compare_eq__empty,
	(Dee_funptr_t)&default__set_operator_compare_eq__with__set_operator_eq,
	(Dee_funptr_t)&default__set_operator_compare_eq__with__set_operator_ne,
	(Dee_funptr_t)&default__set_operator_compare_eq__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_compare_eq[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_compare_eq__with_callobjectcache___set_compare_eq__, &tdefault__set_operator_compare_eq__with_callobjectcache___set_compare_eq__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__compare_eq__with__, &tusrtype__compare_eq__with__),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__compare, &tdefault__compare_eq__with__compare),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__eq, &tdefault__compare_eq__with__eq),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__ne, &tdefault__compare_eq__with__ne),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__lo__and__gr, &tdefault__compare_eq__with__lo__and__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__le__and__ge, &tdefault__compare_eq__with__le__and__ge),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_trycompare_eq[5] = {
	(Dee_funptr_t)&default__set_operator_trycompare_eq__with_callattr___set_compare_eq__,
	(Dee_funptr_t)&default__set_operator_trycompare_eq__unsupported,
	(Dee_funptr_t)&default__set_operator_trycompare_eq__with__set_operator_compare_eq,
	(Dee_funptr_t)&default__set_operator_trycompare_eq__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_trycompare_eq[3] = {
	MH_SUPER_MAP_TYPED_INIT(&usrtype__trycompare_eq__with__, &tusrtype__trycompare_eq__with__),
	MH_SUPER_MAP_TYPED_INIT(&default__trycompare_eq__with__compare_eq, &tdefault__trycompare_eq__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_eq[5] = {
	(Dee_funptr_t)&default__set_operator_eq__with_callattr___set_eq__,
	(Dee_funptr_t)&default__set_operator_eq__unsupported,
	(Dee_funptr_t)&default__set_operator_eq__empty,
	(Dee_funptr_t)&default__set_operator_eq__with__set_operator_compare_eq,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_eq[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_eq__with_callobjectcache___set_eq__, &tdefault__set_operator_eq__with_callobjectcache___set_eq__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__eq__with__EQ, &tusrtype__eq__with__EQ),
	MH_SUPER_MAP_TYPED_INIT(&default__eq__with__ne, &tdefault__eq__with__ne),
	MH_SUPER_MAP_TYPED_INIT(&default__eq__with__compare_eq, &tdefault__eq__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_ne[5] = {
	(Dee_funptr_t)&default__set_operator_ne__with_callattr___set_ne__,
	(Dee_funptr_t)&default__set_operator_ne__unsupported,
	(Dee_funptr_t)&default__set_operator_ne__empty,
	(Dee_funptr_t)&default__set_operator_ne__with__set_operator_compare_eq,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_ne[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_ne__with_callobjectcache___set_ne__, &tdefault__set_operator_ne__with_callobjectcache___set_ne__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__ne__with__NE, &tusrtype__ne__with__NE),
	MH_SUPER_MAP_TYPED_INIT(&default__ne__with__eq, &tdefault__ne__with__eq),
	MH_SUPER_MAP_TYPED_INIT(&default__ne__with__compare_eq, &tdefault__ne__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_lo[6] = {
	(Dee_funptr_t)&default__set_operator_lo__with_callattr___set_lo__,
	(Dee_funptr_t)&default__set_operator_lo__unsupported,
	(Dee_funptr_t)&default__set_operator_lo__empty,
	(Dee_funptr_t)&default__set_operator_lo__with__set_operator_ge,
	(Dee_funptr_t)&default__set_operator_lo__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_lo[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_lo__with_callobjectcache___set_lo__, &tdefault__set_operator_lo__with_callobjectcache___set_lo__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__lo__with__LO, &tusrtype__lo__with__LO),
	MH_SUPER_MAP_TYPED_INIT(&default__lo__with__ge, &tdefault__lo__with__ge),
	MH_SUPER_MAP_TYPED_INIT(&default__lo__with__compare, &tdefault__lo__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_le[7] = {
	(Dee_funptr_t)&default__set_operator_le__with_callattr_issubset,
	(Dee_funptr_t)&default__set_operator_le__with_callattr___set_le__,
	(Dee_funptr_t)&default__set_operator_le__unsupported,
	(Dee_funptr_t)&default__set_operator_le__empty,
	(Dee_funptr_t)&default__set_operator_le__with__set_operator_gr,
	(Dee_funptr_t)&default__set_operator_le__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_le[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_le__with_callobjectcache___set_le__, &tdefault__set_operator_le__with_callobjectcache___set_le__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__le__with__LE, &tusrtype__le__with__LE),
	MH_SUPER_MAP_TYPED_INIT(&default__le__with__gr, &tdefault__le__with__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__le__with__compare, &tdefault__le__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_gr[6] = {
	(Dee_funptr_t)&default__set_operator_gr__with_callattr___set_gr__,
	(Dee_funptr_t)&default__set_operator_gr__unsupported,
	(Dee_funptr_t)&default__set_operator_gr__empty,
	(Dee_funptr_t)&default__set_operator_gr__with__set_operator_le,
	(Dee_funptr_t)&default__set_operator_gr__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_gr[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_gr__with_callobjectcache___set_gr__, &tdefault__set_operator_gr__with_callobjectcache___set_gr__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__gr__with__GR, &tusrtype__gr__with__GR),
	MH_SUPER_MAP_TYPED_INIT(&default__gr__with__le, &tdefault__gr__with__le),
	MH_SUPER_MAP_TYPED_INIT(&default__gr__with__compare, &tdefault__gr__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_ge[7] = {
	(Dee_funptr_t)&default__set_operator_ge__with_callattr_issuperset,
	(Dee_funptr_t)&default__set_operator_ge__with_callattr___set_ge__,
	(Dee_funptr_t)&default__set_operator_ge__unsupported,
	(Dee_funptr_t)&default__set_operator_ge__empty,
	(Dee_funptr_t)&default__set_operator_ge__with__set_operator_lo,
	(Dee_funptr_t)&default__set_operator_ge__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_ge[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_ge__with_callobjectcache___set_ge__, &tdefault__set_operator_ge__with_callobjectcache___set_ge__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__ge__with__GE, &tusrtype__ge__with__GE),
	MH_SUPER_MAP_TYPED_INIT(&default__ge__with__lo, &tdefault__ge__with__lo),
	MH_SUPER_MAP_TYPED_INIT(&default__ge__with__compare, &tdefault__ge__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_inv[4] = {
	(Dee_funptr_t)&default__set_operator_inv__with_callattr___set_size__,
	(Dee_funptr_t)&default__set_operator_inv__unsupported,
	(Dee_funptr_t)&default__set_operator_inv__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_inv[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_inv__with_callobjectcache___set_size__, &tdefault__set_operator_inv__with_callobjectcache___set_size__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inv__with__INV, &tusrtype__inv__with__INV),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_add[5] = {
	(Dee_funptr_t)&default__set_operator_add__with_callattr_union,
	(Dee_funptr_t)&default__set_operator_add__with_callattr___set_add__,
	(Dee_funptr_t)&default__set_operator_add__unsupported,
	(Dee_funptr_t)&default__set_operator_add__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_add[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_add__with_callobjectcache___set_add__, &tdefault__set_operator_add__with_callobjectcache___set_add__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__add__with__ADD, &tusrtype__add__with__ADD),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_sub[5] = {
	(Dee_funptr_t)&default__set_operator_sub__with_callattr_difference,
	(Dee_funptr_t)&default__set_operator_sub__with_callattr___set_sub__,
	(Dee_funptr_t)&default__set_operator_sub__unsupported,
	(Dee_funptr_t)&default__set_operator_sub__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_sub[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_sub__with_callobjectcache___set_sub__, &tdefault__set_operator_sub__with_callobjectcache___set_sub__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__sub__with__SUB, &tusrtype__sub__with__SUB),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_and[5] = {
	(Dee_funptr_t)&default__set_operator_and__with_callattr_intersection,
	(Dee_funptr_t)&default__set_operator_and__with_callattr___set_and__,
	(Dee_funptr_t)&default__set_operator_and__unsupported,
	(Dee_funptr_t)&default__set_operator_and__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_and[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_and__with_callobjectcache___set_and__, &tdefault__set_operator_and__with_callobjectcache___set_and__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__and__with__AND, &tusrtype__and__with__AND),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_xor[5] = {
	(Dee_funptr_t)&default__set_operator_xor__with_callattr_symmetric_difference,
	(Dee_funptr_t)&default__set_operator_xor__with_callattr___set_xor__,
	(Dee_funptr_t)&default__set_operator_xor__unsupported,
	(Dee_funptr_t)&default__set_operator_xor__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_xor[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_xor__with_callobjectcache___set_xor__, &tdefault__set_operator_xor__with_callobjectcache___set_xor__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__xor__with__XOR, &tusrtype__xor__with__XOR),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_inplace_add[5] = {
	(Dee_funptr_t)&default__set_operator_inplace_add__with_callattr___set_inplace_add__,
	(Dee_funptr_t)&default__set_operator_inplace_add__unsupported,
	(Dee_funptr_t)&default__set_operator_inplace_add__empty,
	(Dee_funptr_t)&default__set_operator_inplace_add__with__set_insertall,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_inplace_add[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_inplace_add__with_callobjectcache___set_inplace_add__, &tdefault__set_operator_inplace_add__with_callobjectcache___set_inplace_add__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_add__with__INPLACE_ADD, &tusrtype__inplace_add__with__INPLACE_ADD),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_add__with__add, &tdefault__inplace_add__with__add),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_inplace_sub[5] = {
	(Dee_funptr_t)&default__set_operator_inplace_sub__with_callattr___set_inplace_sub__,
	(Dee_funptr_t)&default__set_operator_inplace_sub__unsupported,
	(Dee_funptr_t)&default__set_operator_inplace_sub__empty,
	(Dee_funptr_t)&default__set_operator_inplace_sub__with__set_operator_foreach__and__set_removeall,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_inplace_sub[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_inplace_sub__with_callobjectcache___set_inplace_sub__, &tdefault__set_operator_inplace_sub__with_callobjectcache___set_inplace_sub__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_sub__with__INPLACE_SUB, &tusrtype__inplace_sub__with__INPLACE_SUB),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_sub__with__sub, &tdefault__inplace_sub__with__sub),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_inplace_and[5] = {
	(Dee_funptr_t)&default__set_operator_inplace_and__with_callattr___set_inplace_and__,
	(Dee_funptr_t)&default__set_operator_inplace_and__unsupported,
	(Dee_funptr_t)&default__set_operator_inplace_and__empty,
	(Dee_funptr_t)&default__set_operator_inplace_and__with__set_operator_foreach__and__set_removeall,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_inplace_and[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_inplace_and__with_callobjectcache___set_inplace_and__, &tdefault__set_operator_inplace_and__with_callobjectcache___set_inplace_and__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_and__with__INPLACE_AND, &tusrtype__inplace_and__with__INPLACE_AND),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_and__with__and, &tdefault__inplace_and__with__and),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_operator_inplace_xor[5] = {
	(Dee_funptr_t)&default__set_operator_inplace_xor__with_callattr___set_inplace_xor__,
	(Dee_funptr_t)&default__set_operator_inplace_xor__unsupported,
	(Dee_funptr_t)&default__set_operator_inplace_xor__empty,
	(Dee_funptr_t)&default__set_operator_inplace_xor__with__set_operator_foreach__and__set_insertall__and__set_removeall,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_operator_inplace_xor[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_operator_inplace_xor__with_callobjectcache___set_inplace_xor__, &tdefault__set_operator_inplace_xor__with_callobjectcache___set_inplace_xor__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_xor__with__INPLACE_XOR, &tusrtype__inplace_xor__with__INPLACE_XOR),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_xor__with__xor, &tdefault__inplace_xor__with__xor),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_frozen[6] = {
	(Dee_funptr_t)&default__set_frozen__with_callattr_frozen,
	(Dee_funptr_t)&default__set_frozen__with_callattr___set_frozen__,
	(Dee_funptr_t)&default__set_frozen__unsupported,
	(Dee_funptr_t)&default__set_frozen__empty,
	(Dee_funptr_t)&default__set_frozen__with__set_operator_foreach,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_frozen[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_frozen__with_callobjectcache___set_frozen__, &tdefault__set_frozen__with_callobjectcache___set_frozen__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_unify[7] = {
	(Dee_funptr_t)&default__set_unify__with_callattr_unify,
	(Dee_funptr_t)&default__set_unify__with_callattr___set_unify__,
	(Dee_funptr_t)&default__set_unify__unsupported,
	(Dee_funptr_t)&default__set_unify__empty,
	(Dee_funptr_t)&default__set_unify__with__seq_operator_foreach__and__set_insert,
	(Dee_funptr_t)&default__set_unify__with__seq_operator_foreach__and__seq_append,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_unify[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_unify__with_callobjectcache___set_unify__, &tdefault__set_unify__with_callobjectcache___set_unify__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_insert[8] = {
	(Dee_funptr_t)&default__set_insert__with_callattr_insert,
	(Dee_funptr_t)&default__set_insert__with_callattr___set_insert__,
	(Dee_funptr_t)&default__set_insert__unsupported,
	(Dee_funptr_t)&default__set_insert__empty,
	(Dee_funptr_t)&default__set_insert__with__map_setnew,
	(Dee_funptr_t)&default__set_insert__with__seq_operator_size__and__set_insertall,
	(Dee_funptr_t)&default__set_insert__with__seq_contains__and__seq_append,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_insert[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_insert__with_callobjectcache___set_insert__, &tdefault__set_insert__with_callobjectcache___set_insert__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_insertall[7] = {
	(Dee_funptr_t)&default__set_insertall__with_callattr_insertall,
	(Dee_funptr_t)&default__set_insertall__with_callattr___set_insertall__,
	(Dee_funptr_t)&default__set_insertall__unsupported,
	(Dee_funptr_t)&default__set_insertall__empty,
	(Dee_funptr_t)&default__set_insertall__with__set_operator_inplace_add,
	(Dee_funptr_t)&default__set_insertall__with__set_insert,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_insertall[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_insertall__with_callobjectcache___set_insertall__, &tdefault__set_insertall__with_callobjectcache___set_insertall__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_remove[8] = {
	(Dee_funptr_t)&default__set_remove__with_callattr_remove,
	(Dee_funptr_t)&default__set_remove__with_callattr___set_remove__,
	(Dee_funptr_t)&default__set_remove__unsupported,
	(Dee_funptr_t)&default__set_remove__empty,
	(Dee_funptr_t)&default__set_remove__with__map_operator_trygetitem__and__map_operator_delitem,
	(Dee_funptr_t)&default__set_remove__with__seq_operator_size__and__set_removeall,
	(Dee_funptr_t)&default__set_remove__with__seq_removeall,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_remove[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_remove__with_callobjectcache___set_remove__, &tdefault__set_remove__with_callobjectcache___set_remove__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_removeall[7] = {
	(Dee_funptr_t)&default__set_removeall__with_callattr_removeall,
	(Dee_funptr_t)&default__set_removeall__with_callattr___set_removeall__,
	(Dee_funptr_t)&default__set_removeall__unsupported,
	(Dee_funptr_t)&default__set_removeall__empty,
	(Dee_funptr_t)&default__set_removeall__with__set_operator_inplace_sub,
	(Dee_funptr_t)&default__set_removeall__with__set_remove,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_removeall[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_removeall__with_callobjectcache___set_removeall__, &tdefault__set_removeall__with_callobjectcache___set_removeall__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_pop[8] = {
	(Dee_funptr_t)&default__set_pop__with_callattr_pop,
	(Dee_funptr_t)&default__set_pop__with_callattr___set_pop__,
	(Dee_funptr_t)&default__set_pop__unsupported,
	(Dee_funptr_t)&default__set_pop__empty,
	(Dee_funptr_t)&default__set_pop__with__seq_trygetfirst__and__set_remove,
	(Dee_funptr_t)&default__set_pop__with__map_popitem,
	(Dee_funptr_t)&default__set_pop__with__seq_pop,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_pop[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_pop__with_callobjectcache___set_pop__, &tdefault__set_pop__with_callobjectcache___set_pop__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__set_pop_with_default[8] = {
	(Dee_funptr_t)&default__set_pop_with_default__with_callattr_pop,
	(Dee_funptr_t)&default__set_pop_with_default__with_callattr___set_pop__,
	(Dee_funptr_t)&default__set_pop_with_default__unsupported,
	(Dee_funptr_t)&default__set_pop_with_default__empty,
	(Dee_funptr_t)&default__set_pop_with_default__with__seq_trygetfirst__and__set_remove,
	(Dee_funptr_t)&default__set_pop_with_default__with__map_popitem,
	(Dee_funptr_t)&default__set_pop_with_default__with__seq_pop,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__set_pop_with_default[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__set_pop_with_default__with_callobjectcache___set_pop__, &tdefault__set_pop_with_default__with_callobjectcache___set_pop__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_getitem[10] = {
	(Dee_funptr_t)&default__map_operator_getitem__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_getitem__unsupported,
	(Dee_funptr_t)&default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_hash,
	(Dee_funptr_t)&default__map_operator_getitem__with__map_operator_getitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_getitem__with__map_operator_getitem_string_hash,
	(Dee_funptr_t)&default__map_operator_getitem__with__map_operator_getitem_index,
	(Dee_funptr_t)&default__map_operator_getitem__empty,
	(Dee_funptr_t)&default__map_operator_getitem__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_getitem[9] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_getitem__with_callobjectcache___map_getitem__, &tdefault__map_operator_getitem__with_callobjectcache___map_getitem__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__getitem__with__GETITEM, &tusrtype__getitem__with__GETITEM),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__trygetitem, &tdefault__getitem__with__trygetitem),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_index__and__getitem_string_len_hash, &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_index__and__getitem_string_hash, &tdefault__getitem__with__getitem_index__and__getitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_index, &tdefault__getitem__with__getitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_string_len_hash, &tdefault__getitem__with__getitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem__with__getitem_string_hash, &tdefault__getitem__with__getitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_trygetitem[11] = {
	(Dee_funptr_t)&default__map_operator_trygetitem__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_trygetitem__unsupported,
	(Dee_funptr_t)&default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash,
	(Dee_funptr_t)&default__map_operator_trygetitem__with__map_operator_trygetitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_trygetitem__with__map_operator_trygetitem_string_hash,
	(Dee_funptr_t)&default__map_operator_trygetitem__with__map_operator_trygetitem_index,
	(Dee_funptr_t)&default__map_operator_trygetitem__with__map_operator_getitem,
	(Dee_funptr_t)&default__map_operator_trygetitem__empty,
	(Dee_funptr_t)&default__map_operator_trygetitem__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_trygetitem[7] = {
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__getitem, &tdefault__trygetitem__with__getitem),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash, &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash, &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_index, &tdefault__trygetitem__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_string_len_hash, &tdefault__trygetitem__with__trygetitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem__with__trygetitem_string_hash, &tdefault__trygetitem__with__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_getitem_index[5] = {
	(Dee_funptr_t)&default__map_operator_getitem_index__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_getitem_index__unsupported,
	(Dee_funptr_t)&default__map_operator_getitem_index__with__map_operator_getitem,
	(Dee_funptr_t)&default__map_operator_getitem_index__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_getitem_index[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_index__with__size__and__getitem_index_fast, &tdefault__getitem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_index__with__trygetitem_index, &tdefault__getitem_index__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_index__with__getitem, &tdefault__getitem_index__with__getitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_trygetitem_index[5] = {
	(Dee_funptr_t)&default__map_operator_trygetitem_index__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_trygetitem_index__unsupported,
	(Dee_funptr_t)&default__map_operator_trygetitem_index__with__map_operator_trygetitem,
	(Dee_funptr_t)&default__map_operator_trygetitem_index__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_trygetitem_index[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_index__with__size__and__getitem_index_fast, &tdefault__trygetitem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_index__with__getitem_index, &tdefault__trygetitem_index__with__getitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_index__with__trygetitem, &tdefault__trygetitem_index__with__trygetitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_getitem_string_hash[6] = {
	(Dee_funptr_t)&default__map_operator_getitem_string_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_getitem_string_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_getitem_string_hash__with__map_operator_getitem,
	(Dee_funptr_t)&default__map_operator_getitem_string_hash__empty,
	(Dee_funptr_t)&default__map_operator_getitem_string_hash__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_getitem_string_hash[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_string_hash__with__trygetitem_string_hash, &tdefault__getitem_string_hash__with__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_string_hash__with__getitem, &tdefault__getitem_string_hash__with__getitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_trygetitem_string_hash[6] = {
	(Dee_funptr_t)&default__map_operator_trygetitem_string_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_hash__empty,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_hash__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_trygetitem_string_hash[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_string_hash__with__getitem_string_hash, &tdefault__trygetitem_string_hash__with__getitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_string_hash__with__trygetitem, &tdefault__trygetitem_string_hash__with__trygetitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_getitem_string_len_hash[6] = {
	(Dee_funptr_t)&default__map_operator_getitem_string_len_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_getitem_string_len_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_getitem_string_len_hash__with__map_operator_getitem,
	(Dee_funptr_t)&default__map_operator_getitem_string_len_hash__empty,
	(Dee_funptr_t)&default__map_operator_getitem_string_len_hash__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_getitem_string_len_hash[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_string_len_hash__with__trygetitem_string_len_hash, &tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_string_len_hash__with__getitem, &tdefault__getitem_string_len_hash__with__getitem),
	MH_SUPER_MAP_TYPED_INIT(&default__getitem_string_len_hash__with__getitem_string_hash, &tdefault__getitem_string_len_hash__with__getitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_trygetitem_string_len_hash[6] = {
	(Dee_funptr_t)&default__map_operator_trygetitem_string_len_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_len_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_len_hash__empty,
	(Dee_funptr_t)&default__map_operator_trygetitem_string_len_hash__with__map_enumerate,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_trygetitem_string_len_hash[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_string_len_hash__with__getitem_string_len_hash, &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_string_len_hash__with__trygetitem, &tdefault__trygetitem_string_len_hash__with__trygetitem),
	MH_SUPER_MAP_TYPED_INIT(&default__trygetitem_string_len_hash__with__trygetitem_string_hash, &tdefault__trygetitem_string_len_hash__with__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_bounditem[7] = {
	(Dee_funptr_t)&default__map_operator_bounditem__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_bounditem__unsupported,
	(Dee_funptr_t)&default__map_operator_bounditem__with__map_operator_getitem,
	(Dee_funptr_t)&default__map_operator_bounditem__empty,
	(Dee_funptr_t)&default__map_operator_bounditem__with__map_enumerate,
	(Dee_funptr_t)&default__map_operator_bounditem__with__map_operator_contains,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_bounditem[9] = {
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__getitem, &tdefault__bounditem__with__getitem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_len_hash, &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_hash, &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__trygetitem__and__hasitem, &tdefault__bounditem__with__trygetitem__and__hasitem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_index, &tdefault__bounditem__with__bounditem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_string_len_hash, &tdefault__bounditem__with__bounditem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__bounditem_string_hash, &tdefault__bounditem__with__bounditem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem__with__trygetitem, &tdefault__bounditem__with__trygetitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_bounditem_index[6] = {
	(Dee_funptr_t)&default__map_operator_bounditem_index__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_bounditem_index__unsupported,
	(Dee_funptr_t)&default__map_operator_bounditem_index__with__map_operator_bounditem,
	(Dee_funptr_t)&default__map_operator_bounditem_index__with__map_operator_getitem_index,
	(Dee_funptr_t)&default__map_operator_bounditem_index__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_bounditem_index[6] = {
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__size__and__getitem_index_fast, &tdefault__bounditem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__getitem_index, &tdefault__bounditem_index__with__getitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__bounditem, &tdefault__bounditem_index__with__bounditem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__trygetitem_index__and__hasitem_index, &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_index__with__trygetitem_index, &tdefault__bounditem_index__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_bounditem_string_hash[6] = {
	(Dee_funptr_t)&default__map_operator_bounditem_string_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_bounditem_string_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_bounditem_string_hash__with__map_operator_bounditem,
	(Dee_funptr_t)&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash,
	(Dee_funptr_t)&default__map_operator_bounditem_string_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_bounditem_string_hash[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_hash__with__getitem_string_hash, &tdefault__bounditem_string_hash__with__getitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_hash__with__bounditem, &tdefault__bounditem_string_hash__with__bounditem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash, &tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_hash__with__trygetitem_string_hash, &tdefault__bounditem_string_hash__with__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_bounditem_string_len_hash[6] = {
	(Dee_funptr_t)&default__map_operator_bounditem_string_len_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_bounditem_string_len_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_bounditem_string_len_hash__with__map_operator_bounditem,
	(Dee_funptr_t)&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_bounditem_string_len_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_bounditem_string_len_hash[6] = {
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_len_hash__with__getitem_string_len_hash, &tdefault__bounditem_string_len_hash__with__getitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_len_hash__with__bounditem, &tdefault__bounditem_string_len_hash__with__bounditem),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash, &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash, &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__bounditem_string_len_hash__with__bounditem_string_hash, &tdefault__bounditem_string_len_hash__with__bounditem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_hasitem[5] = {
	(Dee_funptr_t)&default__map_operator_hasitem__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_hasitem__unsupported,
	(Dee_funptr_t)&default__map_operator_hasitem__with__map_operator_bounditem,
	(Dee_funptr_t)&default__map_operator_hasitem__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_hasitem[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__trygetitem, &tdefault__hasitem__with__trygetitem),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__bounditem, &tdefault__hasitem__with__bounditem),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_len_hash, &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_hash, &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_index, &tdefault__hasitem__with__hasitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_string_len_hash, &tdefault__hasitem__with__hasitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem__with__hasitem_string_hash, &tdefault__hasitem__with__hasitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_hasitem_index[5] = {
	(Dee_funptr_t)&default__map_operator_hasitem_index__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_hasitem_index__unsupported,
	(Dee_funptr_t)&default__map_operator_hasitem_index__with__map_operator_bounditem_index,
	(Dee_funptr_t)&default__map_operator_hasitem_index__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_hasitem_index[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__size__and__getitem_index_fast, &tdefault__hasitem_index__with__size__and__getitem_index_fast),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__trygetitem_index, &tdefault__hasitem_index__with__trygetitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__bounditem_index, &tdefault__hasitem_index__with__bounditem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_index__with__hasitem, &tdefault__hasitem_index__with__hasitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_hasitem_string_hash[5] = {
	(Dee_funptr_t)&default__map_operator_hasitem_string_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_hasitem_string_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash,
	(Dee_funptr_t)&default__map_operator_hasitem_string_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_hasitem_string_hash[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_string_hash__with__trygetitem_string_hash, &tdefault__hasitem_string_hash__with__trygetitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_string_hash__with__bounditem_string_hash, &tdefault__hasitem_string_hash__with__bounditem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_string_hash__with__hasitem, &tdefault__hasitem_string_hash__with__hasitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_hasitem_string_len_hash[5] = {
	(Dee_funptr_t)&default__map_operator_hasitem_string_len_hash__with_callattr___map_getitem__,
	(Dee_funptr_t)&default__map_operator_hasitem_string_len_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_hasitem_string_len_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_hasitem_string_len_hash[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_string_len_hash__with__trygetitem_string_len_hash, &tdefault__hasitem_string_len_hash__with__trygetitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_string_len_hash__with__bounditem_string_len_hash, &tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_string_len_hash__with__hasitem, &tdefault__hasitem_string_len_hash__with__hasitem),
	MH_SUPER_MAP_TYPED_INIT(&default__hasitem_string_len_hash__with__hasitem_string_hash, &tdefault__hasitem_string_len_hash__with__hasitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_delitem[11] = {
	(Dee_funptr_t)&default__map_operator_delitem__with_callattr___map_delitem__,
	(Dee_funptr_t)&default__map_operator_delitem__unsupported,
	(Dee_funptr_t)&default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_hash,
	(Dee_funptr_t)&default__map_operator_delitem__with__map_operator_delitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_delitem__with__map_operator_delitem_string_hash,
	(Dee_funptr_t)&default__map_operator_delitem__with__map_operator_delitem_index,
	(Dee_funptr_t)&default__map_operator_delitem__empty,
	(Dee_funptr_t)&default__map_operator_delitem__with__map_remove,
	(Dee_funptr_t)&default__map_operator_delitem__with__map_removekeys,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_delitem[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_delitem__with_callobjectcache___map_delitem__, &tdefault__map_operator_delitem__with_callobjectcache___map_delitem__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__delitem__with__DELITEM, &tusrtype__delitem__with__DELITEM),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_index__and__delitem_string_len_hash, &tdefault__delitem__with__delitem_index__and__delitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_index__and__delitem_string_hash, &tdefault__delitem__with__delitem_index__and__delitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_index, &tdefault__delitem__with__delitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_string_len_hash, &tdefault__delitem__with__delitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem__with__delitem_string_hash, &tdefault__delitem__with__delitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_delitem_index[5] = {
	(Dee_funptr_t)&default__map_operator_delitem_index__with_callattr___map_delitem__,
	(Dee_funptr_t)&default__map_operator_delitem_index__unsupported,
	(Dee_funptr_t)&default__map_operator_delitem_index__with__map_operator_delitem,
	(Dee_funptr_t)&default__map_operator_delitem_index__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_delitem_index[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__delitem_index__with__delitem, &tdefault__delitem_index__with__delitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_delitem_string_hash[5] = {
	(Dee_funptr_t)&default__map_operator_delitem_string_hash__with_callattr___map_delitem__,
	(Dee_funptr_t)&default__map_operator_delitem_string_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_delitem_string_hash__with__map_operator_delitem,
	(Dee_funptr_t)&default__map_operator_delitem_string_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_delitem_string_hash[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__delitem_string_hash__with__delitem, &tdefault__delitem_string_hash__with__delitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_delitem_string_len_hash[5] = {
	(Dee_funptr_t)&default__map_operator_delitem_string_len_hash__with_callattr___map_delitem__,
	(Dee_funptr_t)&default__map_operator_delitem_string_len_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_delitem_string_len_hash__with__map_operator_delitem,
	(Dee_funptr_t)&default__map_operator_delitem_string_len_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_delitem_string_len_hash[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__delitem_string_len_hash__with__delitem, &tdefault__delitem_string_len_hash__with__delitem),
	MH_SUPER_MAP_TYPED_INIT(&default__delitem_string_len_hash__with__delitem_string_hash, &tdefault__delitem_string_len_hash__with__delitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_setitem[9] = {
	(Dee_funptr_t)&default__map_operator_setitem__with_callattr___map_setitem__,
	(Dee_funptr_t)&default__map_operator_setitem__unsupported,
	(Dee_funptr_t)&default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_hash,
	(Dee_funptr_t)&default__map_operator_setitem__with__map_operator_setitem_string_len_hash,
	(Dee_funptr_t)&default__map_operator_setitem__with__map_operator_setitem_string_hash,
	(Dee_funptr_t)&default__map_operator_setitem__with__map_operator_setitem_index,
	(Dee_funptr_t)&default__map_operator_setitem__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_setitem[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_setitem__with_callobjectcache___map_setitem__, &tdefault__map_operator_setitem__with_callobjectcache___map_setitem__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__setitem__with__SETITEM, &tusrtype__setitem__with__SETITEM),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_index__and__setitem_string_len_hash, &tdefault__setitem__with__setitem_index__and__setitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_index__and__setitem_string_hash, &tdefault__setitem__with__setitem_index__and__setitem_string_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_index, &tdefault__setitem__with__setitem_index),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_string_len_hash, &tdefault__setitem__with__setitem_string_len_hash),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem__with__setitem_string_hash, &tdefault__setitem__with__setitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_setitem_index[5] = {
	(Dee_funptr_t)&default__map_operator_setitem_index__with_callattr___map_setitem__,
	(Dee_funptr_t)&default__map_operator_setitem_index__unsupported,
	(Dee_funptr_t)&default__map_operator_setitem_index__with__map_operator_setitem,
	(Dee_funptr_t)&default__map_operator_setitem_index__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_setitem_index[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__setitem_index__with__setitem, &tdefault__setitem_index__with__setitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_setitem_string_hash[5] = {
	(Dee_funptr_t)&default__map_operator_setitem_string_hash__with_callattr___map_setitem__,
	(Dee_funptr_t)&default__map_operator_setitem_string_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_setitem_string_hash__with__map_operator_setitem,
	(Dee_funptr_t)&default__map_operator_setitem_string_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_setitem_string_hash[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__setitem_string_hash__with__setitem, &tdefault__setitem_string_hash__with__setitem),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_setitem_string_len_hash[5] = {
	(Dee_funptr_t)&default__map_operator_setitem_string_len_hash__with_callattr___map_setitem__,
	(Dee_funptr_t)&default__map_operator_setitem_string_len_hash__unsupported,
	(Dee_funptr_t)&default__map_operator_setitem_string_len_hash__with__map_operator_setitem,
	(Dee_funptr_t)&default__map_operator_setitem_string_len_hash__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_setitem_string_len_hash[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__setitem_string_len_hash__with__setitem, &tdefault__setitem_string_len_hash__with__setitem),
	MH_SUPER_MAP_TYPED_INIT(&default__setitem_string_len_hash__with__setitem_string_hash, &tdefault__setitem_string_len_hash__with__setitem_string_hash),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_contains[6] = {
	(Dee_funptr_t)&default__map_operator_contains__with_callattr___map_contains__,
	(Dee_funptr_t)&default__map_operator_contains__unsupported,
	(Dee_funptr_t)&default__map_operator_contains__empty,
	(Dee_funptr_t)&default__map_operator_contains__with__map_operator_trygetitem,
	(Dee_funptr_t)&default__map_operator_contains__with__map_operator_bounditem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_contains[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_contains__with_callobjectcache___map_contains__, &tdefault__map_operator_contains__with_callobjectcache___map_contains__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__contains__with__CONTAINS, &tusrtype__contains__with__CONTAINS),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_keys[6] = {
	(Dee_funptr_t)&default__map_keys__with_callattr_keys,
	(Dee_funptr_t)&default__map_keys__with_callattr___map_keys__,
	(Dee_funptr_t)&default__map_keys__unsupported,
	(Dee_funptr_t)&default__map_keys__empty,
	(Dee_funptr_t)&default__map_keys__with__map_iterkeys,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_keys[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_keys__with_callobjectcache___map_keys__, &tdefault__map_keys__with_callobjectcache___map_keys__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_iterkeys[8] = {
	(Dee_funptr_t)&default__map_iterkeys__with_callattr_iterkeys,
	(Dee_funptr_t)&default__map_iterkeys__with_callattr___map_iterkeys__,
	(Dee_funptr_t)&default__map_iterkeys__unsupported,
	(Dee_funptr_t)&default__map_iterkeys__empty,
	(Dee_funptr_t)&default__map_iterkeys__with__map_keys,
	(Dee_funptr_t)&default__map_iterkeys__with__map_enumerate,
	(Dee_funptr_t)&default__map_iterkeys__with__set_operator_iter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_iterkeys[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_iterkeys__with_callobjectcache___map_iterkeys__, &tdefault__map_iterkeys__with_callobjectcache___map_iterkeys__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_values[6] = {
	(Dee_funptr_t)&default__map_values__with_callattr_values,
	(Dee_funptr_t)&default__map_values__with_callattr___map_values__,
	(Dee_funptr_t)&default__map_values__unsupported,
	(Dee_funptr_t)&default__map_values__empty,
	(Dee_funptr_t)&default__map_values__with__map_itervalues,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_values[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_values__with_callobjectcache___map_values__, &tdefault__map_values__with_callobjectcache___map_values__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_itervalues[7] = {
	(Dee_funptr_t)&default__map_itervalues__with_callattr_itervalues,
	(Dee_funptr_t)&default__map_itervalues__with_callattr___map_itervalues__,
	(Dee_funptr_t)&default__map_itervalues__unsupported,
	(Dee_funptr_t)&default__map_itervalues__empty,
	(Dee_funptr_t)&default__map_itervalues__with__map_values,
	(Dee_funptr_t)&default__map_itervalues__with__set_operator_iter,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_itervalues[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_itervalues__with_callobjectcache___map_itervalues__, &tdefault__map_itervalues__with_callobjectcache___map_itervalues__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_enumerate[6] = {
	(Dee_funptr_t)&default__map_enumerate__with_callattr___map_enumerate__,
	(Dee_funptr_t)&default__map_enumerate__unsupported,
	(Dee_funptr_t)&default__map_enumerate__with__map_enumerate_range,
	(Dee_funptr_t)&default__map_enumerate__empty,
	(Dee_funptr_t)&default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_enumerate[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_enumerate__with_callobjectcache___map_enumerate__, &tdefault__map_enumerate__with_callobjectcache___map_enumerate__),
	MH_SUPER_MAP_TYPED_INIT(&default__foreach_pair__with__foreach, &tdefault__foreach_pair__with__foreach),
	MH_SUPER_MAP_TYPED_INIT(&default__foreach_pair__with__iter, &tdefault__foreach_pair__with__iter),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_enumerate_range[6] = {
	(Dee_funptr_t)&default__map_enumerate_range__with_callattr___map_enumerate__,
	(Dee_funptr_t)&default__map_enumerate_range__unsupported,
	(Dee_funptr_t)&default__map_enumerate_range__with__map_enumerate,
	(Dee_funptr_t)&default__map_enumerate_range__empty,
	(Dee_funptr_t)&default__map_enumerate_range__with__map_iterkeys__and__map_operator_trygetitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_enumerate_range[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_enumerate_range__with_callobjectcache___map_enumerate__, &tdefault__map_enumerate_range__with_callobjectcache___map_enumerate__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_compare_eq[7] = {
	(Dee_funptr_t)&default__map_operator_compare_eq__with_callattr___map_compare_eq__,
	(Dee_funptr_t)&default__map_operator_compare_eq__unsupported,
	(Dee_funptr_t)&default__map_operator_compare_eq__empty,
	(Dee_funptr_t)&default__map_operator_compare_eq__with__map_operator_eq,
	(Dee_funptr_t)&default__map_operator_compare_eq__with__map_operator_ne,
	(Dee_funptr_t)&default__map_operator_compare_eq__with__set_operator_foreach_pair,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_compare_eq[8] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_compare_eq__with_callobjectcache___map_compare_eq__, &tdefault__map_operator_compare_eq__with_callobjectcache___map_compare_eq__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__compare_eq__with__, &tusrtype__compare_eq__with__),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__compare, &tdefault__compare_eq__with__compare),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__eq, &tdefault__compare_eq__with__eq),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__ne, &tdefault__compare_eq__with__ne),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__lo__and__gr, &tdefault__compare_eq__with__lo__and__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__compare_eq__with__le__and__ge, &tdefault__compare_eq__with__le__and__ge),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_trycompare_eq[5] = {
	(Dee_funptr_t)&default__map_operator_trycompare_eq__with_callattr___map_compare_eq__,
	(Dee_funptr_t)&default__map_operator_trycompare_eq__unsupported,
	(Dee_funptr_t)&default__map_operator_trycompare_eq__with__map_operator_compare_eq,
	(Dee_funptr_t)&default__map_operator_trycompare_eq__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_trycompare_eq[3] = {
	MH_SUPER_MAP_TYPED_INIT(&usrtype__trycompare_eq__with__, &tusrtype__trycompare_eq__with__),
	MH_SUPER_MAP_TYPED_INIT(&default__trycompare_eq__with__compare_eq, &tdefault__trycompare_eq__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_eq[5] = {
	(Dee_funptr_t)&default__map_operator_eq__with_callattr___map_eq__,
	(Dee_funptr_t)&default__map_operator_eq__unsupported,
	(Dee_funptr_t)&default__map_operator_eq__empty,
	(Dee_funptr_t)&default__map_operator_eq__with__map_operator_compare_eq,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_eq[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_eq__with_callobjectcache___map_eq__, &tdefault__map_operator_eq__with_callobjectcache___map_eq__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__eq__with__EQ, &tusrtype__eq__with__EQ),
	MH_SUPER_MAP_TYPED_INIT(&default__eq__with__ne, &tdefault__eq__with__ne),
	MH_SUPER_MAP_TYPED_INIT(&default__eq__with__compare_eq, &tdefault__eq__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_ne[5] = {
	(Dee_funptr_t)&default__map_operator_ne__with_callattr___map_ne__,
	(Dee_funptr_t)&default__map_operator_ne__unsupported,
	(Dee_funptr_t)&default__map_operator_ne__empty,
	(Dee_funptr_t)&default__map_operator_ne__with__map_operator_compare_eq,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_ne[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_ne__with_callobjectcache___map_ne__, &tdefault__map_operator_ne__with_callobjectcache___map_ne__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__ne__with__NE, &tusrtype__ne__with__NE),
	MH_SUPER_MAP_TYPED_INIT(&default__ne__with__eq, &tdefault__ne__with__eq),
	MH_SUPER_MAP_TYPED_INIT(&default__ne__with__compare_eq, &tdefault__ne__with__compare_eq),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_lo[6] = {
	(Dee_funptr_t)&default__map_operator_lo__with_callattr___map_lo__,
	(Dee_funptr_t)&default__map_operator_lo__unsupported,
	(Dee_funptr_t)&default__map_operator_lo__empty,
	(Dee_funptr_t)&default__map_operator_lo__with__map_operator_ge,
	(Dee_funptr_t)&default__map_operator_lo__with__set_operator_foreach_pair,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_lo[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_lo__with_callobjectcache___map_lo__, &tdefault__map_operator_lo__with_callobjectcache___map_lo__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__lo__with__LO, &tusrtype__lo__with__LO),
	MH_SUPER_MAP_TYPED_INIT(&default__lo__with__ge, &tdefault__lo__with__ge),
	MH_SUPER_MAP_TYPED_INIT(&default__lo__with__compare, &tdefault__lo__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_le[6] = {
	(Dee_funptr_t)&default__map_operator_le__with_callattr___map_le__,
	(Dee_funptr_t)&default__map_operator_le__unsupported,
	(Dee_funptr_t)&default__map_operator_le__empty,
	(Dee_funptr_t)&default__map_operator_le__with__map_operator_gr,
	(Dee_funptr_t)&default__map_operator_le__with__set_operator_foreach_pair,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_le[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_le__with_callobjectcache___map_le__, &tdefault__map_operator_le__with_callobjectcache___map_le__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__le__with__LE, &tusrtype__le__with__LE),
	MH_SUPER_MAP_TYPED_INIT(&default__le__with__gr, &tdefault__le__with__gr),
	MH_SUPER_MAP_TYPED_INIT(&default__le__with__compare, &tdefault__le__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_gr[6] = {
	(Dee_funptr_t)&default__map_operator_gr__with_callattr___map_gr__,
	(Dee_funptr_t)&default__map_operator_gr__unsupported,
	(Dee_funptr_t)&default__map_operator_gr__empty,
	(Dee_funptr_t)&default__map_operator_gr__with__map_operator_le,
	(Dee_funptr_t)&default__map_operator_gr__with__set_operator_foreach_pair,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_gr[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_gr__with_callobjectcache___map_gr__, &tdefault__map_operator_gr__with_callobjectcache___map_gr__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__gr__with__GR, &tusrtype__gr__with__GR),
	MH_SUPER_MAP_TYPED_INIT(&default__gr__with__le, &tdefault__gr__with__le),
	MH_SUPER_MAP_TYPED_INIT(&default__gr__with__compare, &tdefault__gr__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_ge[6] = {
	(Dee_funptr_t)&default__map_operator_ge__with_callattr___map_ge__,
	(Dee_funptr_t)&default__map_operator_ge__unsupported,
	(Dee_funptr_t)&default__map_operator_ge__empty,
	(Dee_funptr_t)&default__map_operator_ge__with__map_operator_lo,
	(Dee_funptr_t)&default__map_operator_ge__with__set_operator_foreach_pair,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_ge[5] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_ge__with_callobjectcache___map_ge__, &tdefault__map_operator_ge__with_callobjectcache___map_ge__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__ge__with__GE, &tusrtype__ge__with__GE),
	MH_SUPER_MAP_TYPED_INIT(&default__ge__with__lo, &tdefault__ge__with__lo),
	MH_SUPER_MAP_TYPED_INIT(&default__ge__with__compare, &tdefault__ge__with__compare),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_add[5] = {
	(Dee_funptr_t)&default__map_operator_add__with_callattr_union,
	(Dee_funptr_t)&default__map_operator_add__with_callattr___map_add__,
	(Dee_funptr_t)&default__map_operator_add__unsupported,
	(Dee_funptr_t)&default__map_operator_add__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_add[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_add__with_callobjectcache___map_add__, &tdefault__map_operator_add__with_callobjectcache___map_add__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__add__with__ADD, &tusrtype__add__with__ADD),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_sub[5] = {
	(Dee_funptr_t)&default__map_operator_sub__with_callattr_difference,
	(Dee_funptr_t)&default__map_operator_sub__with_callattr___map_sub__,
	(Dee_funptr_t)&default__map_operator_sub__unsupported,
	(Dee_funptr_t)&default__map_operator_sub__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_sub[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_sub__with_callobjectcache___map_sub__, &tdefault__map_operator_sub__with_callobjectcache___map_sub__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__sub__with__SUB, &tusrtype__sub__with__SUB),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_and[5] = {
	(Dee_funptr_t)&default__map_operator_and__with_callattr_intersection,
	(Dee_funptr_t)&default__map_operator_and__with_callattr___map_and__,
	(Dee_funptr_t)&default__map_operator_and__unsupported,
	(Dee_funptr_t)&default__map_operator_and__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_and[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_and__with_callobjectcache___map_and__, &tdefault__map_operator_and__with_callobjectcache___map_and__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__and__with__AND, &tusrtype__and__with__AND),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_xor[5] = {
	(Dee_funptr_t)&default__map_operator_xor__with_callattr_symmetric_difference,
	(Dee_funptr_t)&default__map_operator_xor__with_callattr___map_xor__,
	(Dee_funptr_t)&default__map_operator_xor__unsupported,
	(Dee_funptr_t)&default__map_operator_xor__empty,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_xor[3] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_xor__with_callobjectcache___map_xor__, &tdefault__map_operator_xor__with_callobjectcache___map_xor__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__xor__with__XOR, &tusrtype__xor__with__XOR),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_inplace_add[5] = {
	(Dee_funptr_t)&default__map_operator_inplace_add__with_callattr___map_inplace_add__,
	(Dee_funptr_t)&default__map_operator_inplace_add__unsupported,
	(Dee_funptr_t)&default__map_operator_inplace_add__empty,
	(Dee_funptr_t)&default__map_operator_inplace_add__with__map_update,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_inplace_add[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_inplace_add__with_callobjectcache___map_inplace_add__, &tdefault__map_operator_inplace_add__with_callobjectcache___map_inplace_add__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_add__with__INPLACE_ADD, &tusrtype__inplace_add__with__INPLACE_ADD),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_add__with__add, &tdefault__inplace_add__with__add),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_inplace_sub[5] = {
	(Dee_funptr_t)&default__map_operator_inplace_sub__with_callattr___map_inplace_sub__,
	(Dee_funptr_t)&default__map_operator_inplace_sub__unsupported,
	(Dee_funptr_t)&default__map_operator_inplace_sub__empty,
	(Dee_funptr_t)&default__map_operator_inplace_sub__with__map_removekeys,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_inplace_sub[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_inplace_sub__with_callobjectcache___map_inplace_sub__, &tdefault__map_operator_inplace_sub__with_callobjectcache___map_inplace_sub__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_sub__with__INPLACE_SUB, &tusrtype__inplace_sub__with__INPLACE_SUB),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_sub__with__sub, &tdefault__inplace_sub__with__sub),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_inplace_and[5] = {
	(Dee_funptr_t)&default__map_operator_inplace_and__with_callattr___map_inplace_and__,
	(Dee_funptr_t)&default__map_operator_inplace_and__unsupported,
	(Dee_funptr_t)&default__map_operator_inplace_and__empty,
	(Dee_funptr_t)&default__map_operator_inplace_and__with__set_operator_foreach_pair__and__map_removekeys,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_inplace_and[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_inplace_and__with_callobjectcache___map_inplace_and__, &tdefault__map_operator_inplace_and__with_callobjectcache___map_inplace_and__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_and__with__INPLACE_AND, &tusrtype__inplace_and__with__INPLACE_AND),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_and__with__and, &tdefault__inplace_and__with__and),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_operator_inplace_xor[5] = {
	(Dee_funptr_t)&default__map_operator_inplace_xor__with_callattr___map_inplace_xor__,
	(Dee_funptr_t)&default__map_operator_inplace_xor__unsupported,
	(Dee_funptr_t)&default__map_operator_inplace_xor__empty,
	(Dee_funptr_t)&default__map_operator_inplace_xor__with__set_operator_foreach_pair__and__map_update__and__map_removekeys,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_operator_inplace_xor[4] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_operator_inplace_xor__with_callobjectcache___map_inplace_xor__, &tdefault__map_operator_inplace_xor__with_callobjectcache___map_inplace_xor__),
	MH_SUPER_MAP_TYPED_INIT(&usrtype__inplace_xor__with__INPLACE_XOR, &tusrtype__inplace_xor__with__INPLACE_XOR),
	MH_SUPER_MAP_TYPED_INIT(&default__inplace_xor__with__xor, &tdefault__inplace_xor__with__xor),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_frozen[6] = {
	(Dee_funptr_t)&default__map_frozen__with_callattr_frozen,
	(Dee_funptr_t)&default__map_frozen__with_callattr___map_frozen__,
	(Dee_funptr_t)&default__map_frozen__unsupported,
	(Dee_funptr_t)&default__map_frozen__empty,
	(Dee_funptr_t)&default__map_frozen__with__set_operator_foreach_pair,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_frozen[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_frozen__with_callobjectcache___map_frozen__, &tdefault__map_frozen__with_callobjectcache___map_frozen__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_setold[7] = {
	(Dee_funptr_t)&default__map_setold__with_callattr_setold,
	(Dee_funptr_t)&default__map_setold__with_callattr___map_setold__,
	(Dee_funptr_t)&default__map_setold__unsupported,
	(Dee_funptr_t)&default__map_setold__empty,
	(Dee_funptr_t)&default__map_setold__with__map_setold_ex,
	(Dee_funptr_t)&default__map_setold__with__map_operator_trygetitem__and__map_operator_setitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_setold[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_setold__with_callobjectcache___map_setold__, &tdefault__map_setold__with_callobjectcache___map_setold__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_setold_ex[7] = {
	(Dee_funptr_t)&default__map_setold_ex__with_callattr_setold_ex,
	(Dee_funptr_t)&default__map_setold_ex__with_callattr___map_setold_ex__,
	(Dee_funptr_t)&default__map_setold_ex__unsupported,
	(Dee_funptr_t)&default__map_setold_ex__empty,
	(Dee_funptr_t)&default__map_setold_ex__with__map_operator_trygetitem__and__map_setold,
	(Dee_funptr_t)&default__map_setold_ex__with__map_operator_trygetitem__and__map_operator_setitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_setold_ex[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_setold_ex__with_callobjectcache___map_setold_ex__, &tdefault__map_setold_ex__with_callobjectcache___map_setold_ex__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_setnew[8] = {
	(Dee_funptr_t)&default__map_setnew__with_callattr_setnew,
	(Dee_funptr_t)&default__map_setnew__with_callattr___map_setnew__,
	(Dee_funptr_t)&default__map_setnew__unsupported,
	(Dee_funptr_t)&default__map_setnew__empty,
	(Dee_funptr_t)&default__map_setnew__with__map_setnew_ex,
	(Dee_funptr_t)&default__map_setnew__with__map_operator_trygetitem__and__map_setdefault,
	(Dee_funptr_t)&default__map_setnew__with__map_operator_trygetitem__and__map_operator_setitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_setnew[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_setnew__with_callobjectcache___map_setnew__, &tdefault__map_setnew__with_callobjectcache___map_setnew__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_setnew_ex[8] = {
	(Dee_funptr_t)&default__map_setnew_ex__with_callattr_setnew_ex,
	(Dee_funptr_t)&default__map_setnew_ex__with_callattr___map_setnew_ex__,
	(Dee_funptr_t)&default__map_setnew_ex__unsupported,
	(Dee_funptr_t)&default__map_setnew_ex__empty,
	(Dee_funptr_t)&default__map_setnew_ex__with__map_operator_trygetitem__and__map_setnew,
	(Dee_funptr_t)&default__map_setnew_ex__with__map_operator_trygetitem__and__map_setdefault,
	(Dee_funptr_t)&default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_setnew_ex[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_setnew_ex__with_callobjectcache___map_setnew_ex__, &tdefault__map_setnew_ex__with_callobjectcache___map_setnew_ex__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_setdefault[8] = {
	(Dee_funptr_t)&default__map_setdefault__with_callattr_setdefault,
	(Dee_funptr_t)&default__map_setdefault__with_callattr___map_setdefault__,
	(Dee_funptr_t)&default__map_setdefault__unsupported,
	(Dee_funptr_t)&default__map_setdefault__empty,
	(Dee_funptr_t)&default__map_setdefault__with__map_setnew_ex,
	(Dee_funptr_t)&default__map_setdefault__with__map_setnew__and__map_operator_getitem,
	(Dee_funptr_t)&default__map_setdefault__with__map_operator_trygetitem__and__map_operator_setitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_setdefault[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_setdefault__with_callobjectcache___map_setdefault__, &tdefault__map_setdefault__with_callobjectcache___map_setdefault__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_update[7] = {
	(Dee_funptr_t)&default__map_update__with_callattr_update,
	(Dee_funptr_t)&default__map_update__with_callattr___map_update__,
	(Dee_funptr_t)&default__map_update__unsupported,
	(Dee_funptr_t)&default__map_update__empty,
	(Dee_funptr_t)&default__map_update__with__map_operator_inplace_add,
	(Dee_funptr_t)&default__map_update__with__map_operator_setitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_update[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_update__with_callobjectcache___map_update__, &tdefault__map_update__with_callobjectcache___map_update__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_remove[7] = {
	(Dee_funptr_t)&default__map_remove__with_callattr_remove,
	(Dee_funptr_t)&default__map_remove__with_callattr___map_remove__,
	(Dee_funptr_t)&default__map_remove__unsupported,
	(Dee_funptr_t)&default__map_remove__empty,
	(Dee_funptr_t)&default__map_remove__with__map_operator_bounditem__and__map_operator_delitem,
	(Dee_funptr_t)&default__map_remove__with__seq_operator_size__and__map_operator_delitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_remove[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_remove__with_callobjectcache___map_remove__, &tdefault__map_remove__with_callobjectcache___map_remove__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_removekeys[7] = {
	(Dee_funptr_t)&default__map_removekeys__with_callattr_removekeys,
	(Dee_funptr_t)&default__map_removekeys__with_callattr___map_removekeys__,
	(Dee_funptr_t)&default__map_removekeys__unsupported,
	(Dee_funptr_t)&default__map_removekeys__empty,
	(Dee_funptr_t)&default__map_removekeys__with__map_operator_inplace_sub,
	(Dee_funptr_t)&default__map_removekeys__with__map_remove,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_removekeys[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_removekeys__with_callobjectcache___map_removekeys__, &tdefault__map_removekeys__with_callobjectcache___map_removekeys__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_pop[6] = {
	(Dee_funptr_t)&default__map_pop__with_callattr_pop,
	(Dee_funptr_t)&default__map_pop__with_callattr___map_pop__,
	(Dee_funptr_t)&default__map_pop__unsupported,
	(Dee_funptr_t)&default__map_pop__empty,
	(Dee_funptr_t)&default__map_pop__with__map_operator_getitem__and__map_operator_delitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_pop[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_pop__with_callobjectcache___map_pop__, &tdefault__map_pop__with_callobjectcache___map_pop__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_pop_with_default[6] = {
	(Dee_funptr_t)&default__map_pop_with_default__with_callattr_pop,
	(Dee_funptr_t)&default__map_pop_with_default__with_callattr___map_pop__,
	(Dee_funptr_t)&default__map_pop_with_default__unsupported,
	(Dee_funptr_t)&default__map_pop_with_default__empty,
	(Dee_funptr_t)&default__map_pop_with_default__with__map_operator_trygetitem__and__map_operator_delitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_pop_with_default[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_pop_with_default__with_callobjectcache___map_pop__, &tdefault__map_pop_with_default__with_callobjectcache___map_pop__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE Dee_funptr_t tpconst msm_with_super__map_popitem[7] = {
	(Dee_funptr_t)&default__map_popitem__with_callattr_popitem,
	(Dee_funptr_t)&default__map_popitem__with_callattr___map_popitem__,
	(Dee_funptr_t)&default__map_popitem__unsupported,
	(Dee_funptr_t)&default__map_popitem__empty,
	(Dee_funptr_t)&default__map_popitem__with__seq_trygetlast__and__map_operator_delitem,
	(Dee_funptr_t)&default__map_popitem__with__seq_trygetfirst__and__map_operator_delitem,
	NULL
};
PRIVATE struct mh_super_map_typed tpconst msm_with_type__map_popitem[2] = {
	MH_SUPER_MAP_TYPED_INIT(&default__map_popitem__with_callobjectcache___map_popitem__, &tdefault__map_popitem__with_callobjectcache___map_popitem__),
	MH_SUPER_MAP_TYPED_END
};
PRIVATE struct mh_super_map tpconst mh_super_maps[233] = {
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_bool, msm_with_type__seq_operator_bool),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_sizeob, msm_with_type__seq_operator_sizeob),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_size, msm_with_type__seq_operator_size),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_iter, msm_with_super__seq_operator_iter, msm_with_type__seq_operator_iter),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_foreach, msm_with_super__seq_operator_foreach, msm_with_type__seq_operator_foreach),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_foreach_pair, msm_with_type__seq_operator_foreach_pair),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_getitem, msm_with_type__seq_operator_getitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_getitem_index, msm_with_type__seq_operator_getitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_trygetitem, msm_with_type__seq_operator_trygetitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_trygetitem_index, msm_with_type__seq_operator_trygetitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_hasitem, msm_with_type__seq_operator_hasitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_hasitem_index, msm_with_type__seq_operator_hasitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_bounditem, msm_with_type__seq_operator_bounditem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_bounditem_index, msm_with_type__seq_operator_bounditem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_delitem, msm_with_type__seq_operator_delitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_delitem_index, msm_with_type__seq_operator_delitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_setitem, msm_with_type__seq_operator_setitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_setitem_index, msm_with_type__seq_operator_setitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_getrange, msm_with_type__seq_operator_getrange),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_getrange_index, msm_with_super__seq_operator_getrange_index, msm_with_type__seq_operator_getrange_index),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_getrange_index_n, msm_with_super__seq_operator_getrange_index_n, msm_with_type__seq_operator_getrange_index_n),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_delrange, msm_with_type__seq_operator_delrange),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_delrange_index, msm_with_type__seq_operator_delrange_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_delrange_index_n, msm_with_type__seq_operator_delrange_index_n),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_setrange, msm_with_type__seq_operator_setrange),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_setrange_index, msm_with_type__seq_operator_setrange_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_setrange_index_n, msm_with_type__seq_operator_setrange_index_n),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_assign, msm_with_type__seq_operator_assign),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_hash, msm_with_super__seq_operator_hash, msm_with_type__seq_operator_hash),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_compare, msm_with_super__seq_operator_compare, msm_with_type__seq_operator_compare),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_compare_eq, msm_with_super__seq_operator_compare_eq, msm_with_type__seq_operator_compare_eq),
	MH_SUPER_MAP_INIT(msm_replace__seq_operator_trycompare_eq, msm_with_super__seq_operator_trycompare_eq, msm_with_type__seq_operator_trycompare_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_eq, msm_with_type__seq_operator_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_ne, msm_with_type__seq_operator_ne),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_lo, msm_with_type__seq_operator_lo),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_le, msm_with_type__seq_operator_le),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_gr, msm_with_type__seq_operator_gr),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_ge, msm_with_type__seq_operator_ge),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_inplace_add, msm_with_type__seq_operator_inplace_add),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_inplace_mul, msm_with_type__seq_operator_inplace_mul),
	MH_SUPER_MAP_INIT(msm_replace__seq_enumerate, msm_with_super__seq_enumerate, msm_with_type__seq_enumerate),
	MH_SUPER_MAP_INIT(msm_replace__seq_enumerate_index, msm_with_super__seq_enumerate_index, msm_with_type__seq_enumerate_index),
	MH_SUPER_MAP_INIT(msm_replace__seq_makeenumeration, msm_with_super__seq_makeenumeration, msm_with_type__seq_makeenumeration),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_makeenumeration_with_range, msm_with_type__seq_makeenumeration_with_range),
	MH_SUPER_MAP_INIT(msm_replace__seq_makeenumeration_with_intrange, msm_with_super__seq_makeenumeration_with_intrange, msm_with_type__seq_makeenumeration_with_intrange),
	MH_SUPER_MAP_INIT(NULL, NULL, NULL),
	MH_SUPER_MAP_INIT(NULL, NULL, NULL),
	MH_SUPER_MAP_INIT(msm_replace__seq_unpack, msm_with_super__seq_unpack, msm_with_type__seq_unpack),
	MH_SUPER_MAP_INIT(msm_replace__seq_unpack_ex, msm_with_super__seq_unpack_ex, msm_with_type__seq_unpack_ex),
	MH_SUPER_MAP_INIT(msm_replace__seq_unpack_ub, msm_with_super__seq_unpack_ub, msm_with_type__seq_unpack_ub),
	MH_SUPER_MAP_INIT(msm_replace__seq_trygetfirst, msm_with_super__seq_trygetfirst, msm_with_type__seq_trygetfirst),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_getfirst, msm_with_type__seq_getfirst),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_boundfirst, msm_with_type__seq_boundfirst),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_delfirst, msm_with_type__seq_delfirst),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_setfirst, msm_with_type__seq_setfirst),
	MH_SUPER_MAP_INIT(msm_replace__seq_trygetlast, msm_with_super__seq_trygetlast, msm_with_type__seq_trygetlast),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_getlast, msm_with_type__seq_getlast),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_boundlast, msm_with_type__seq_boundlast),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_dellast, msm_with_type__seq_dellast),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_setlast, msm_with_type__seq_setlast),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_cached, msm_with_type__seq_cached),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_frozen, msm_with_type__seq_frozen),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_any, msm_with_type__seq_any),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_any_with_key, msm_with_type__seq_any_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_any_with_range, msm_with_type__seq_any_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_any_with_range_and_key, msm_with_type__seq_any_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_all, msm_with_type__seq_all),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_all_with_key, msm_with_type__seq_all_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_all_with_range, msm_with_type__seq_all_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_all_with_range_and_key, msm_with_type__seq_all_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_parity, msm_with_type__seq_parity),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_parity_with_key, msm_with_type__seq_parity_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_parity_with_range, msm_with_type__seq_parity_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_parity_with_range_and_key, msm_with_type__seq_parity_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_reduce, msm_with_type__seq_reduce),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_reduce_with_init, msm_with_type__seq_reduce_with_init),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_reduce_with_range, msm_with_type__seq_reduce_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_reduce_with_range_and_init, msm_with_type__seq_reduce_with_range_and_init),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_min, msm_with_type__seq_min),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_min_with_key, msm_with_type__seq_min_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_min_with_range, msm_with_type__seq_min_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_min_with_range_and_key, msm_with_type__seq_min_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_max, msm_with_type__seq_max),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_max_with_key, msm_with_type__seq_max_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_max_with_range, msm_with_type__seq_max_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_max_with_range_and_key, msm_with_type__seq_max_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_sum, msm_with_type__seq_sum),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_sum_with_range, msm_with_type__seq_sum_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_count, msm_with_type__seq_count),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_count_with_key, msm_with_type__seq_count_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_count_with_range, msm_with_type__seq_count_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_count_with_range_and_key, msm_with_type__seq_count_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_contains, msm_with_type__seq_contains),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_contains_with_key, msm_with_type__seq_contains_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_contains_with_range, msm_with_type__seq_contains_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_contains_with_range_and_key, msm_with_type__seq_contains_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_operator_contains, msm_with_type__seq_operator_contains),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_locate, msm_with_type__seq_locate),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_locate_with_range, msm_with_type__seq_locate_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_rlocate, msm_with_type__seq_rlocate),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_rlocate_with_range, msm_with_type__seq_rlocate_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_startswith, msm_with_type__seq_startswith),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_startswith_with_key, msm_with_type__seq_startswith_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_startswith_with_range, msm_with_type__seq_startswith_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_startswith_with_range_and_key, msm_with_type__seq_startswith_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_endswith, msm_with_type__seq_endswith),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_endswith_with_key, msm_with_type__seq_endswith_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_endswith_with_range, msm_with_type__seq_endswith_with_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_endswith_with_range_and_key, msm_with_type__seq_endswith_with_range_and_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_find, msm_with_type__seq_find),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_find_with_key, msm_with_type__seq_find_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_rfind, msm_with_type__seq_rfind),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_rfind_with_key, msm_with_type__seq_rfind_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_erase, msm_with_type__seq_erase),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_insert, msm_with_type__seq_insert),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_insertall, msm_with_type__seq_insertall),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_pushfront, msm_with_type__seq_pushfront),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_append, msm_with_type__seq_append),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_extend, msm_with_type__seq_extend),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_xchitem_index, msm_with_type__seq_xchitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_clear, msm_with_type__seq_clear),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_pop, msm_with_type__seq_pop),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_remove, msm_with_type__seq_remove),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_remove_with_key, msm_with_type__seq_remove_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_rremove, msm_with_type__seq_rremove),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_rremove_with_key, msm_with_type__seq_rremove_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_removeall, msm_with_type__seq_removeall),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_removeall_with_key, msm_with_type__seq_removeall_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_removeif, msm_with_type__seq_removeif),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_resize, msm_with_type__seq_resize),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_fill, msm_with_type__seq_fill),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_reverse, msm_with_type__seq_reverse),
	MH_SUPER_MAP_INIT(msm_replace__seq_reversed, msm_with_super__seq_reversed, msm_with_type__seq_reversed),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_sort, msm_with_type__seq_sort),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_sort_with_key, msm_with_type__seq_sort_with_key),
	MH_SUPER_MAP_INIT(msm_replace__seq_sorted, msm_with_super__seq_sorted, msm_with_type__seq_sorted),
	MH_SUPER_MAP_INIT(msm_replace__seq_sorted_with_key, msm_with_super__seq_sorted_with_key, msm_with_type__seq_sorted_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_bfind, msm_with_type__seq_bfind),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_bfind_with_key, msm_with_type__seq_bfind_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_bposition, msm_with_type__seq_bposition),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_bposition_with_key, msm_with_type__seq_bposition_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_brange, msm_with_type__seq_brange),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__seq_brange_with_key, msm_with_type__seq_brange_with_key),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_iter, msm_with_type__set_operator_iter),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_foreach, msm_with_type__set_operator_foreach),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_foreach_pair, msm_with_type__set_operator_foreach_pair),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_sizeob, msm_with_type__set_operator_sizeob),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_size, msm_with_type__set_operator_size),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_hash, msm_with_type__set_operator_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_compare_eq, msm_with_type__set_operator_compare_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_trycompare_eq, msm_with_type__set_operator_trycompare_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_eq, msm_with_type__set_operator_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_ne, msm_with_type__set_operator_ne),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_lo, msm_with_type__set_operator_lo),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_le, msm_with_type__set_operator_le),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_gr, msm_with_type__set_operator_gr),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_ge, msm_with_type__set_operator_ge),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_inv, msm_with_type__set_operator_inv),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_add, msm_with_type__set_operator_add),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_sub, msm_with_type__set_operator_sub),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_and, msm_with_type__set_operator_and),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_xor, msm_with_type__set_operator_xor),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_inplace_add, msm_with_type__set_operator_inplace_add),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_inplace_sub, msm_with_type__set_operator_inplace_sub),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_inplace_and, msm_with_type__set_operator_inplace_and),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_operator_inplace_xor, msm_with_type__set_operator_inplace_xor),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_frozen, msm_with_type__set_frozen),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_unify, msm_with_type__set_unify),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_insert, msm_with_type__set_insert),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_insertall, msm_with_type__set_insertall),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_remove, msm_with_type__set_remove),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_removeall, msm_with_type__set_removeall),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_pop, msm_with_type__set_pop),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__set_pop_with_default, msm_with_type__set_pop_with_default),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_getitem, msm_with_type__map_operator_getitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_trygetitem, msm_with_type__map_operator_trygetitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_getitem_index, msm_with_type__map_operator_getitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_trygetitem_index, msm_with_type__map_operator_trygetitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_getitem_string_hash, msm_with_type__map_operator_getitem_string_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_trygetitem_string_hash, msm_with_type__map_operator_trygetitem_string_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_getitem_string_len_hash, msm_with_type__map_operator_getitem_string_len_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_trygetitem_string_len_hash, msm_with_type__map_operator_trygetitem_string_len_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_bounditem, msm_with_type__map_operator_bounditem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_bounditem_index, msm_with_type__map_operator_bounditem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_bounditem_string_hash, msm_with_type__map_operator_bounditem_string_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_bounditem_string_len_hash, msm_with_type__map_operator_bounditem_string_len_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_hasitem, msm_with_type__map_operator_hasitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_hasitem_index, msm_with_type__map_operator_hasitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_hasitem_string_hash, msm_with_type__map_operator_hasitem_string_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_hasitem_string_len_hash, msm_with_type__map_operator_hasitem_string_len_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_delitem, msm_with_type__map_operator_delitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_delitem_index, msm_with_type__map_operator_delitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_delitem_string_hash, msm_with_type__map_operator_delitem_string_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_delitem_string_len_hash, msm_with_type__map_operator_delitem_string_len_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_setitem, msm_with_type__map_operator_setitem),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_setitem_index, msm_with_type__map_operator_setitem_index),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_setitem_string_hash, msm_with_type__map_operator_setitem_string_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_setitem_string_len_hash, msm_with_type__map_operator_setitem_string_len_hash),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_contains, msm_with_type__map_operator_contains),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_keys, msm_with_type__map_keys),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_iterkeys, msm_with_type__map_iterkeys),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_values, msm_with_type__map_values),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_itervalues, msm_with_type__map_itervalues),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_enumerate, msm_with_type__map_enumerate),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_enumerate_range, msm_with_type__map_enumerate_range),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_compare_eq, msm_with_type__map_operator_compare_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_trycompare_eq, msm_with_type__map_operator_trycompare_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_eq, msm_with_type__map_operator_eq),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_ne, msm_with_type__map_operator_ne),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_lo, msm_with_type__map_operator_lo),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_le, msm_with_type__map_operator_le),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_gr, msm_with_type__map_operator_gr),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_ge, msm_with_type__map_operator_ge),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_add, msm_with_type__map_operator_add),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_sub, msm_with_type__map_operator_sub),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_and, msm_with_type__map_operator_and),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_xor, msm_with_type__map_operator_xor),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_inplace_add, msm_with_type__map_operator_inplace_add),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_inplace_sub, msm_with_type__map_operator_inplace_sub),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_inplace_and, msm_with_type__map_operator_inplace_and),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_operator_inplace_xor, msm_with_type__map_operator_inplace_xor),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_frozen, msm_with_type__map_frozen),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_setold, msm_with_type__map_setold),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_setold_ex, msm_with_type__map_setold_ex),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_setnew, msm_with_type__map_setnew),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_setnew_ex, msm_with_type__map_setnew_ex),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_setdefault, msm_with_type__map_setdefault),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_update, msm_with_type__map_update),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_remove, msm_with_type__map_remove),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_removekeys, msm_with_type__map_removekeys),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_pop, msm_with_type__map_pop),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_pop_with_default, msm_with_type__map_pop_with_default),
	MH_SUPER_MAP_INIT(NULL, msm_with_super__map_popitem, msm_with_type__map_popitem),
};
/*[[[end]]]*/
/* clang-format on */



/* Same as `DeeType_GetMethodHint(DeeSuper_TYPE(super), id)', but must be used in
 * order to lookup information on how to invoke a method hint on a Super-object.
 * @return: true:  Success (always returned for method hints with "%{unsupported}")
 * @return: false: Failure (method it is not supported by `DeeSuper_TYPE(super)',
 *                          and also has no "%{unsupported}" version) */
PUBLIC NONNULL((1, 3)) bool
(DCALL DeeType_GetMethodHintForSuper)(struct Dee_super_object *__restrict super, enum Dee_tmh_id id,
                                      struct Dee_super_method_hint *__restrict result) {
	struct mh_super_map const *specs;
	DeeTypeObject *view_type = DeeSuper_TYPE(super);
	Dee_funptr_t view_impl;
	if (view_type->tp_mhcache == &mh_cache_empty) {
		/* Hack needed so that invoking method hints on something like "foo as Set"
		 * doesn't *actually* like an instance of "Set()", but rather invokes the
		 * true and proper method hints of "foo". */
		goto fallback;
	}
	view_impl = DeeType_GetMethodHint(view_type, id);
	if unlikely(!view_impl)
		return false;

	/* Load invocation specs for `specs' */
	specs = &mh_super_maps[id];

	/* Apply callback replacements */
	if (specs->msm_replace) {
		struct mh_super_map_replace const *iter = specs->msm_replace;
		for (; iter->msmr_old; ++iter) {
			if (view_impl == iter->msmr_old) {
				view_impl = iter->msmr_new;
				break;
			}
		}
	}

	/* Check for callbacks that should be invoke by
	 * fast-forwarding "Super" to the underlying impl. */
	if (specs->msm_with_super) {
		Dee_funptr_t const *iter = specs->msm_with_super;
		for (; *iter; ++iter) {
			if (view_impl == *iter) {
				result->smh_cb = view_impl;
				result->smh_cc = Dee_SUPER_METHOD_HINT_CC_WITH_SUPER;
				return true;
			}
		}
	}

	/* Check for callbacks that should be invoke
	 * by injecting an extra "tp_self" argument. */
	if (specs->msm_with_type) {
		struct mh_super_map_typed const *iter = specs->msm_with_type;
		for (; iter->msmr_regular; ++iter) {
			if (view_impl == iter->msmr_regular) {
				result->smh_cb = iter->msmr_typed;
				result->smh_cc = Dee_SUPER_METHOD_HINT_CC_WITH_TYPE;
				return true;
			}
		}
	}

	/* Abstract types must "by definition" accept any type of object as "self"
	 * As such, we can *always* invoke the method hint implemented here by just
	 * fast-forwarding "super" */
	if (DeeType_IsAbstract(view_type)) {
		result->smh_cb = view_impl;
		result->smh_cc = Dee_SUPER_METHOD_HINT_CC_WITH_SUPER;
		return true;
	}

fallback:
	/* Fallback: anything we don't recognize can't be made to have
	 *           super-object support. As such, better be safe and
	 *           invoke the *real* impl of the target type (even
	 *           if that means that we loose type information) */
	{
		DeeObject *super_self = DeeSuper_SELF(super);
		DeeTypeObject *real_type = Dee_TYPE(super_self);
		result->smh_cb = DeeType_GetMethodHint(real_type, id);
		result->smh_cc = Dee_SUPER_METHOD_HINT_CC_WITH_SELF;
	}
	return true;
}

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_INVOKE_C */
