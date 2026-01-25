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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_TRAITS_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_TRAITS_C 1

#include <deemon/api.h>

#include <deemon/bool.h>         /* DeeBool_Check, DeeBool_IsTrue */
#include <deemon/class.h>        /* DeeClass_DESC, Dee_CLASS_ATTRIBUTE_*, Dee_class_desc, Dee_class_desc_lock_endread, Dee_class_desc_lock_read */
#include <deemon/int.h>          /* DeeInt_Check, DeeInt_IsZero */
#include <deemon/method-hints.h> /* DeeMH_map_enumerate_range_t, DeeMH_map_enumerate_t, DeeMH_map_operator_getitem_t, DeeMH_seq_enumerate_index_t, DeeMH_seq_enumerate_t, DeeType_GetPrivateMethodHint, DeeType_Has*Trait, DeeType_RequireMethodHint, DeeType_TRAIT___map_getitem_always_bound__, DeeType_TRAIT___seq_getitem_always_bound__, Dee_type_trait_t */
#include <deemon/mro.h>          /* DeeType_FindAttrInfoStringLenHash, Dee_ATTRINFO_ATTR, Dee_ATTRINFO_MEMBER, Dee_attrinfo */
#include <deemon/object.h>
#include <deemon/util/atomic.h>  /* atomic_or, atomic_read */

#include <hybrid/typecore.h> /* __CHAR_BIT__ */

#include "method-hint-defaults.h"
#include "method-hints.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t, uintptr_t */

DECL_BEGIN

/*[[[deemon
import define_Dee_HashStr, _Dee_HashSelect from rt.gen.hash;
print define_Dee_HashStr("__seq_getitem_always_bound__");
print define_Dee_HashStr("__map_getitem_always_bound__");
]]]*/
#define Dee_HashStr____seq_getitem_always_bound__ _Dee_HashSelectC(0xdd77720b, 0x759218fd52f7d26)
#define Dee_HashStr____map_getitem_always_bound__ _Dee_HashSelectC(0xdf26f81c, 0x1e2baa1d10ee186a)
/*[[[end]]]*/


struct trait_name {
	Dee_hash_t                    tn_hash;  /* Trait name hash */
	size_t                        tn_size;  /* Trait name length */
	COMPILER_FLEXIBLE_ARRAY(char, tn_name); /* Trait name */
};
#define DEFINE_TRAIT_NAME(name, str, hash) \
	struct {                               \
		Dee_hash_t tn_hash;                \
		size_t tn_size;                    \
		char tn_name[COMPILER_LENOF(str)]; \
	} const name = { hash, COMPILER_LENOF(str), str }


PRIVATE ATTR_NOINLINE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasExplicitTrait_uncached(DeeTypeObject *__restrict self,
                                  Dee_type_trait_t trait) {
	DeeObject *value;
	struct Dee_attrinfo attr;
	struct trait_name const *name;
	switch (trait) {
#define CASE(t)                                                      \
	case DeeType_TRAIT_##t: {                                        \
		PRIVATE DEFINE_TRAIT_NAME(nameof_##t, #t, Dee_HashStr__##t); \
		name = (struct trait_name const *)&nameof_##t;               \
	}	break
	CASE(__seq_getitem_always_bound__);
	CASE(__map_getitem_always_bound__);
#undef CASE
	default: return false;
	}

	/* Lookup the attribute used to configure the requested trait. */
	if (!DeeType_FindAttrInfoStringLenHash(self,
	                                       name->tn_name,
	                                       name->tn_size,
	                                       name->tn_hash,
	                                       &attr))
		return false;

	/* Try so resolve the attribute, but only support:
	 * - constants embedded within type_member of C-types
	 * - constants embedded within "public static final" members of user-classes
	 */
	value = NULL;
	switch (attr.ai_type) {

	case Dee_ATTRINFO_MEMBER:
		if (Dee_TYPE_MEMBER_ISCONST(attr.ai_value.v_member)) {
			value = attr.ai_value.v_member->m_desc.md_const;
			Dee_Incref(value);
		}
		break;

	case Dee_ATTRINFO_ATTR: {
		uint16_t addr;
		struct Dee_class_desc *desc;
		/* Verify that the attribute is "public static final" */
		if ((attr.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FFINAL | Dee_CLASS_ATTRIBUTE_FREADONLY |
		                                      Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET |
		                                      Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FVISIBILITY)) !=
		    /*                            */ (Dee_CLASS_ATTRIBUTE_FFINAL | Dee_CLASS_ATTRIBUTE_FREADONLY |
		                                      Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FPUBLIC))
			break;
		addr = attr.ai_value.v_attr->ca_addr;
		desc = DeeClass_DESC((DeeTypeObject *)attr.ai_decl);
		ASSERT(addr < desc->cd_desc->cd_cmemb_size);
		Dee_class_desc_lock_read(desc);
		value = desc->cd_members[addr];
		Dee_XIncref(value);
		Dee_class_desc_lock_endread(desc);
	}	break;

	default: break;
	}
	if likely(value) {
		bool result;
		if (DeeBool_Check(value)) {
			result = DeeBool_IsTrue(value);
		} else if (DeeInt_Check(value)) {
			result = !DeeInt_IsZero(value); /* XXX: Use of integers is not documented (maybe just remove this eventually?) */
		} else {
			result = false;
		}
		Dee_Decref_unlikely(value);
		return result;
	}
	return false;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) bool
(DCALL DeeType_HasExplicitTrait)(DeeTypeObject *__restrict self,
                                 Dee_type_trait_t trait) {
	bool result;
	union Dee_mhc_traits tt;
	uintptr_t extra;
	struct Dee_type_mh_cache *cache = Dee_type_mh_cache_of(self);
	if unlikely(!cache)
		return DeeType_HasExplicitTrait_uncached(self, trait);
	tt.mht_word = atomic_read(&cache->mh_explicit_traits.mht_word);
	if (tt.mht_traits.tt_load & trait)
		return (tt.mht_traits.tt_have & trait) != 0;
	result = DeeType_HasExplicitTrait_uncached(self, trait);
	extra  = trait;
	if (result)
		extra |= (uintptr_t)trait << ((sizeof(uintptr_t) / 2) * __CHAR_BIT__);
	atomic_or(&cache->mh_explicit_traits.mht_word, extra);
	return result;
}



PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasImplicitTrait_uncached___seq_getitem_always_bound__(DeeTypeObject *__restrict self) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	seq_enumerate_index = DeeType_RequireMethodHint(self, seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_enumerate) {
		DeeMH_seq_enumerate_t seq_enumerate;
		seq_enumerate = DeeType_RequireMethodHint(self, seq_enumerate);
		if (seq_enumerate == &default__seq_enumerate__empty) {
			return true;
		} else if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter) {
			return true;
		}
	} else if (seq_enumerate_index == &default__seq_enumerate_index__empty) {
		return true;
	} else if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_iter) {
		return true;
	} else if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_foreach__and__counter) {
		return true;
	} else if (seq_enumerate_index == &default__seq_enumerate_index__unsupported) {
		/* Definition is: can it thrown UnboundItem() -- if enumeration isn't supported, it
		 *                can't throw that error because it always throws NotImplemented()! */
		return true;
	}
	return false;
}

#define DeeType_RequirePrivateMethodHint(self, orig_type, hint) \
	(DeeMH_##hint##_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_##hint)

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasPrivateTrait_uncached___seq_getitem_always_bound__(DeeTypeObject *self,
                                                              DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	seq_enumerate_index = DeeType_RequirePrivateMethodHint(self, orig_type, seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_enumerate) {
		DeeMH_seq_enumerate_t seq_enumerate;
		seq_enumerate = DeeType_RequirePrivateMethodHint(self, orig_type, seq_enumerate);
		if (seq_enumerate == &default__seq_enumerate__empty) {
			return true;
		} else if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter) {
			return true;
		}
	} else if (seq_enumerate_index == &default__seq_enumerate_index__empty) {
		return true;
	} else if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_iter) {
		return true;
	} else if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_foreach__and__counter) {
		return true;
	} else if (seq_enumerate_index == &default__seq_enumerate_index__unsupported) {
		/* Definition is: can it thrown UnboundItem() -- if enumeration isn't supported, it
		 *                can't throw that error because it always throws NotImplemented()! */
		return true;
	}
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasImplicitTrait_uncached___map_getitem_always_bound__(DeeTypeObject *__restrict self) {
	DeeMH_map_operator_getitem_t map_operator_getitem;
	map_operator_getitem = DeeType_RequireMethodHint(self, map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__unsupported) {
		/* Definition is: can it thrown UnboundItem() -- if enumeration isn't supported, it
		 *                can't throw that error because it always throws NotImplemented()! */
		return true;
	} else if (map_operator_getitem == &default__map_operator_getitem__empty) {
		return true;
	} else if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate) {
		DeeMH_map_enumerate_t map_enumerate;
		map_enumerate = DeeType_RequireMethodHint(self, map_enumerate);
		if (map_enumerate == &default__map_enumerate__with__map_enumerate_range) {
			DeeMH_map_enumerate_range_t map_enumerate_range;
			map_enumerate_range = DeeType_RequireMethodHint(self, map_enumerate_range);
			if (map_enumerate_range == &default__map_enumerate_range__empty)
				return true;
		} else if (map_enumerate == &default__map_enumerate__empty) {
			return true;
		} else if (map_enumerate == &default__map_enumerate__with__seq_operator_foreach_pair) {
			return true;
		} else if (map_enumerate == &default__map_enumerate__with__map_operator_iter) {
			return true;
		} else if (map_enumerate == &default__map_enumerate__unsupported) {
			/* Definition is: can it thrown UnboundItem() -- if enumeration isn't supported, it
			 *                can't throw that error because it always throws NotImplemented()! */
			return true;
		} else if (map_enumerate == DeeType_RequireMethodHint(self, map_operator_foreach_pair))  {
			/* "map_enumerate" impl is stolen from "map_operator_foreach_pair" -- items can't be unbound */
			return true;
		}
	}
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasPrivateTrait_uncached___map_getitem_always_bound__(DeeTypeObject *self,
                                                              DeeTypeObject *orig_type) {
	DeeMH_map_operator_getitem_t map_operator_getitem;
	map_operator_getitem = DeeType_RequirePrivateMethodHint(self, orig_type, map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__unsupported) {
		/* Definition is: can it thrown UnboundItem() -- if enumeration isn't supported, it
		 *                can't throw that error because it always throws NotImplemented()! */
		return true;
	} else if (map_operator_getitem == &default__map_operator_getitem__empty) {
		return true;
	} else if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate) {
		DeeMH_map_enumerate_t map_enumerate;
		map_enumerate = DeeType_RequirePrivateMethodHint(self, orig_type, map_enumerate);
		if (map_enumerate == &default__map_enumerate__with__map_enumerate_range) {
			DeeMH_map_enumerate_range_t map_enumerate_range;
			map_enumerate_range = DeeType_RequirePrivateMethodHint(self, orig_type, map_enumerate_range);
			if (map_enumerate_range == &default__map_enumerate_range__empty)
				return true;
		} else if (map_enumerate == &default__map_enumerate__empty) {
			return true;
		} else if (map_enumerate == &default__map_enumerate__with__seq_operator_foreach_pair) {
			return true;
		} else if (map_enumerate == &default__map_enumerate__with__map_operator_iter) {
			return true;
		} else if (map_enumerate == &default__map_enumerate__unsupported) {
			/* Definition is: can it thrown UnboundItem() -- if enumeration isn't supported, it
			 *                can't throw that error because it always throws NotImplemented()! */
			return true;
		} else if (map_enumerate == DeeType_RequirePrivateMethodHint(self, orig_type, map_operator_foreach_pair))  {
			/* "map_enumerate" impl is stolen from "map_operator_foreach_pair" -- items can't be unbound */
			return true;
		}
	}
	return false;
}

PRIVATE ATTR_NOINLINE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasImplicitTrait_uncached(DeeTypeObject *__restrict self,
                                  Dee_type_trait_t trait) {
	switch (trait) {
	case DeeType_TRAIT___seq_getitem_always_bound__:
		return DeeType_HasImplicitTrait_uncached___seq_getitem_always_bound__(self);
	case DeeType_TRAIT___map_getitem_always_bound__:
		return DeeType_HasImplicitTrait_uncached___map_getitem_always_bound__(self);
	default: break;
	}
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasTrait_uncached(DeeTypeObject *__restrict self, Dee_type_trait_t trait) {
	if (DeeType_HasExplicitTrait(self, trait))
		return true;
	return DeeType_HasImplicitTrait_uncached(self, trait);
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) bool
(DCALL DeeType_HasTrait)(DeeTypeObject *__restrict self, Dee_type_trait_t trait) {
	bool result;
	union Dee_mhc_traits tt;
	uintptr_t extra;
	struct Dee_type_mh_cache *cache = Dee_type_mh_cache_of(self);
	if unlikely(!cache)
		return DeeType_HasTrait_uncached(self, trait);
	tt.mht_word = atomic_read(&cache->mh_traits.mht_word);
	if (tt.mht_traits.tt_load & trait)
		return (tt.mht_traits.tt_have & trait) != 0;
	result = DeeType_HasTrait_uncached(self, trait);
	extra  = trait;
	if (result)
		extra |= (uintptr_t)trait << ((sizeof(uintptr_t) / 2) * __CHAR_BIT__);
	atomic_or(&cache->mh_traits.mht_word, extra);
	return result;
}


PRIVATE ATTR_NOINLINE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasPrivateTrait_uncached(DeeTypeObject *self,
                                 DeeTypeObject *orig_type,
                                 Dee_type_trait_t trait) {
	switch (trait) {
	case DeeType_TRAIT___seq_getitem_always_bound__:
		return DeeType_HasPrivateTrait_uncached___seq_getitem_always_bound__(self, orig_type);
	case DeeType_TRAIT___map_getitem_always_bound__:
		return DeeType_HasPrivateTrait_uncached___map_getitem_always_bound__(self, orig_type);
	default: break;
	}
	return false;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool
(DCALL DeeType_HasPrivateTrait)(DeeTypeObject *self,
                                DeeTypeObject *orig_type,
                                Dee_type_trait_t trait) {
	bool result = DeeType_HasExplicitTrait(self, trait);
	if (!result)
		result = DeeType_HasPrivateTrait_uncached(self, orig_type, trait);
	return result;
}



DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_TRAITS_C */
