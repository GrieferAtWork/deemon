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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_RT_ATTRIBUTE_C
#define GUARD_DEEMON_RUNTIME_ERROR_RT_ATTRIBUTE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/class.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/struct.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/util/atomic.h>

#include <hybrid/host.h>
#include <hybrid/minmax.h>
#include <hybrid/sched/yield.h>
/**/

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#undef byte_t
#define byte_t __BYTE_TYPE__

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

#ifndef __SIZEOF_BOOL__
#define __SIZEOF_BOOL__ __SIZEOF_CHAR__
#endif /* !__SIZEOF_BOOL__ */

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("message");
print define_Dee_HashStr("inner");
print define_Dee_HashStr("ob");
print define_Dee_HashStr("attr");
print define_Dee_HashStr("decl");
print define_Dee_HashStr("isget");
print define_Dee_HashStr("isdel");
print define_Dee_HashStr("isset");
]]]*/
#define Dee_HashStr__message _Dee_HashSelectC(0x14820755, 0xbeaa4b97155366df)
#define Dee_HashStr__inner _Dee_HashSelectC(0x20e82985, 0x4f20d07bb803c1fe)
#define Dee_HashStr__ob _Dee_HashSelectC(0xdfa5fee2, 0x80a90888850ad043)
#define Dee_HashStr__attr _Dee_HashSelectC(0x55cfee3, 0xe4311a2c8443755d)
#define Dee_HashStr__decl _Dee_HashSelectC(0x95fe81e2, 0xdc35fdc1dce5cffc)
#define Dee_HashStr__isget _Dee_HashSelectC(0x5c9806e4, 0x5d9b765d747113e0)
#define Dee_HashStr__isdel _Dee_HashSelectC(0xb56dbf78, 0x80c483cf4d2ec758)
#define Dee_HashStr__isset _Dee_HashSelectC(0xbc9a78a1, 0x5b60f653e62d5e87)
/*[[[end]]]*/

#define Error_init_params "message:?X2?Dstring?N=!N,inner:?X3?DError?O?N=!N"
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
error_print(DeeErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg);



/************************************************************************/
/* Error.AttributeError                                                 */
/************************************************************************/
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
ClassDescriptor_IsInstanceAttr(DeeClassDescriptorObject *__restrict self,
                               struct Dee_class_attribute const *__restrict attr) {
	return attr >= self->cd_iattr_list &&
	       attr <= (self->cd_iattr_list + self->cd_iattr_mask);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
ClassDescriptor_IsClassAttr(DeeClassDescriptorObject *__restrict self,
                            struct Dee_class_attribute const *__restrict attr) {
	return attr >= self->cd_cattr_list &&
	       attr <= (self->cd_cattr_list + self->cd_cattr_mask);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) bool DCALL
type_methods_contains(struct type_method const *chain,
                      struct type_method const *attr) {
	struct type_method const *iter;
	if (!chain)
		return false;
	if (attr < chain)
		return false;
	iter = chain;
	while (iter->m_name)
		++iter;
	return attr < iter;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) bool DCALL
type_getsets_contains(struct type_getset const *chain,
                      struct type_getset const *attr) {
	struct type_getset const *iter;
	if (!chain)
		return false;
	if (attr < chain)
		return false;
	iter = chain;
	while (iter->gs_name)
		++iter;
	return attr < iter;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) bool DCALL
type_members_contains(struct type_member const *chain,
                      struct type_member const *attr) {
	struct type_member const *iter;
	if (!chain)
		return false;
	if (attr < chain)
		return false;
	iter = chain;
	while (iter->m_name)
		++iter;
	return attr < iter;
}

struct type_member_buffer {
	char const                *mb_name;
	union Dee_type_member_desc mb_desc;
};
STATIC_ASSERT_MSG(sizeof(struct type_member_buffer) == (2 * sizeof(char *)),
                  "Type member buffers must be sized such they fit over the "
                  "'ad_name' and 'ad_doc' fields of 'struct Dee_attrdesc'");
STATIC_ASSERT_MSG((COMPILER_OFFSETAFTER(struct Dee_attrdesc, ad_name) == offsetof(struct Dee_attrdesc, ad_doc)) ||
                  (COMPILER_OFFSETAFTER(struct Dee_attrdesc, ad_doc) == offsetof(struct Dee_attrdesc, ad_name)),
                  "The 'ad_name' and 'ad_doc' fields must be adjacent, so that "
                  "we're able to overlay then with 'struct type_member_buffer'");
#define Dee_attrdesc__offsetof__type_member_buffer \
	__hybrid_min_c2(offsetof(struct Dee_attrdesc, ad_name), offsetof(struct Dee_attrdesc, ad_doc))

/* Returns a pointer to the "struct type_member_buffer" that may be embedded into
 * "struct Dee_attrdesc" by "AttributeError" when "AttributeError_F_LAZYDECL" is
 * used with "Dee_ATTRINFO_MEMBER" (or "Dee_ATTRINFO_INSTANCE_MEMBER") */
#define Dee_attrdesc__type_member_buffer(self) \
	((struct type_member_buffer *)((byte_t *)(self) + Dee_attrdesc__offsetof__type_member_buffer))

#define type_member_buffer_init(self, member)  \
	(void)((self)->mb_name = (member)->m_name, \
	       (self)->mb_desc = (member)->m_desc)
#define type_member_buffer_asmember(self) \
	COMPILER_CONTAINER_OF(&(self)->mb_name, struct type_member, m_name)
#define type_member_asmember_buffer(self) \
	COMPILER_CONTAINER_OF(&(self)->m_name, struct type_member_buffer, mb_name)

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) struct type_member const *DCALL
type_members_findbuffer(struct type_member const *chain,
                        struct type_member_buffer const *attr) {
	if (!chain)
		return NULL;
	for (; chain->m_name; ++chain) {
		if (chain->m_name == attr->mb_name &&
		    memcmp(&chain->m_desc, &attr->mb_desc, sizeof(chain->m_desc)) == 0)
			return chain;
	}
	return NULL;
}

typedef struct {
	ERROR_OBJECT_HEAD
	DREF DeeObject     *ae_obj;   /* [0..1][const] Object whose attributes were accessed */
#define Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT    (Dee_ATTRINFO_COUNT + 0) /* Special type for "AttributeError_F_LAZYDECL": "v_any" is a "uint16_t" and an instance object address ("ai_decl" is unused; "ae_obj" is the relevant class-"DeeTypeObject") */
#define Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT (Dee_ATTRINFO_COUNT + 1) /* Special type for "AttributeError_F_LAZYDECL": "v_any" is a "uint16_t" and an instance object address ("ai_decl" is the relevant class-"DeeTypeObject"; "ae_obj" is the instance with the unbound member) */
	struct Dee_attrdesc ae_desc;  /* [valid_if(ae_obj != NULL)] Attribute descriptor. Fields in here are loaded when:
	                               * - ad_name:          [1..1][valid_if(ae_obj != NULL && !AttributeError_F_LAZYDECL)]
	                               * - ad_info.ai_decl:  [0..1][valid_if(!AttributeError_F_LAZYDECL)][DREF]
	                               *                     [if(ae_flags & AttributeError_F_INFOLOADED, [1..1])]
	                               *   When wanting to set "AttributeError_F_INFOLOADED", if
	                               *   this field is still "NULL" (i.e.: wasn't set during
	                               *   object construction), it is initialized then
	                               * - ad_info.ai_type:  [valid_if(ae_flags & AttributeError_F_INFOLOADED)]
	                               * - ad_info.ai_value: [valid_if(ae_flags & AttributeError_F_INFOLOADED)]
	                               * - ad_doc:           [valid_if(ae_flags & AttributeError_F_DESCLOADED)]
	                               * - ad_perm:          [valid_if(ae_flags & AttributeError_F_DESCLOADED)]
	                               * - ad_type:          [valid_if(ae_flags & AttributeError_F_DESCLOADED)]
	                               * NOTE: The "Dee_ATTRPERM_F_NAMEOBJ" flag in "ad_perm" is [const]
	                               *       and set during object construction.
	                               * Also: all fields are [lock(WRITE_ONCE)] (with initialization
	                               *       happening **BEFORE** the relevant `AttributeError_F_*'
	                               *       flag is set) */
#define AttributeError_F_GET DeeRT_ATTRIBUTE_ACCESS_GET /* Attempted to get attribute */
#define AttributeError_F_DEL DeeRT_ATTRIBUTE_ACCESS_DEL /* Attempted to del attribute */
#define AttributeError_F_SET DeeRT_ATTRIBUTE_ACCESS_SET /* Attempted to set attribute */
#define AttributeError_F_ACCESS (AttributeError_F_GET | AttributeError_F_DEL | AttributeError_F_SET)
#define AttributeError_F_INFOLOADED 0x0100 /* "struct Dee_attrinfo"-portion was loaded (requires "AttributeError_F_DECLLOADED" to also be set) */
#define AttributeError_F_DESCLOADED 0x0200 /* "struct Dee_attrdesc"-portion was loaded (requires "AttributeError_F_INFOLOADED" to also be set) */
#define AttributeError_F_ISDEFAULT  0x0400 /* Set if "ae_desc" is the result of "DeeObject_FindAttrInfoStringLenHash" */
#define AttributeError_F_NODEFAULT  0x0800 /* Set if "ae_desc" isn't the result of "DeeObject_FindAttrInfoStringLenHash" */
#define AttributeError_F_LAZYDECL   0x1000 /* Set if "ad_info.ai_decl" and "ad_name" (except when "Dee_ATTRINFO_CUSTOM" is used) must be
                                            * determined lazily (based on "ae_obj", "ae_desc.ad_info.ai_type" and "ae_desc.ad_info.ai_value"). */
#ifndef CONFIG_NO_THREADS
#define AttributeError_F_LOADLOCK 0x8000 /* Lock to ensure only 1 thread does the loading */
#endif /* !CONFIG_NO_THREADS */
	unsigned int        ae_flags; /* Set of `AttributeError_F_*' */

	/* When "AttributeError_F_LAZYDECL" is set, a second-stage lazy initialization
	 * takes places whenever the "name" and/or "decl" of the exception is required.
	 * This initialization requires that "AttributeError_F_INFOLOADED" be already
	 * set, and its actions depend on "ae_desc.ad_info.ai_type" and the following
	 * pre-initializations:
	 *
	 * - Dee_ATTRINFO_CUSTOM:
	 *   - DREF DeeObject         *ae_obj = <Accessed Object>;
	 *   - char const             *ae_desc.ad_name = <name_of_attribute>; // [DREF_IF(Dee_ATTRPERM_F_NAMEOBJ)]
	 *   - Dee_attrperm_t          ae_desc.ad_perm = 0 | Dee_ATTRPERM_F_NAMEOBJ;
	 *   - uintptr_t               ae_desc.ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
	 *   - struct type_attr const *ae_desc.ad_info.ai_value.v_custom = <attribute operators>;
	 *
	 * - Dee_ATTRINFO_MODSYM:
	 *   - DREF DeeModuleObject       *ae_obj = <Accessed Module>;
	 *   - uintptr_t                   ae_desc.ad_info.ai_type = Dee_ATTRINFO_MODSYM;
	 *   - struct module_symbol const *ae_desc.ad_info.ai_value.v_modsym = <accessed symbol>;
	 *
	 * - Dee_ATTRINFO_METHOD:
	 *   Dee_ATTRINFO_INSTANCE_METHOD:
	 *   - DREF DeeObject           *ae_obj = <Accessed Object or Type>;
	 *   - uintptr_t                 ae_desc.ad_info.ai_type = Dee_ATTRINFO_METHOD;
	 *   - struct type_method const *ae_desc.ad_info.ai_value.v_method = <accessed method>;
	 *   "Dee_ATTRINFO_METHOD" is changed to "Dee_ATTRINFO_INSTANCE_METHOD"
	 *   if "ae_obj" is a type, and "v_method" is one of its "tp_methods".
	 *
	 * - Dee_ATTRINFO_GETSET:
	 *   Dee_ATTRINFO_INSTANCE_GETSET:
	 *   - DREF DeeObject           *ae_obj = <Accessed Object or Type>;
	 *   - uintptr_t                 ae_desc.ad_info.ai_type = Dee_ATTRINFO_GETSET;
	 *   - struct type_getset const *ae_desc.ad_info.ai_value.v_getset = <accessed getset>;
	 *   "Dee_ATTRINFO_GETSET" is changed to "Dee_ATTRINFO_INSTANCE_GETSET"
	 *   if "ae_obj" is a type, and "v_getset" is one of its "tp_getsets".
	 *
	 * - Dee_ATTRINFO_MEMBER:
	 *   Dee_ATTRINFO_INSTANCE_MEMBER:
	 *   - DREF DeeObject           *ae_obj = <Accessed Object or Type>;
	 *   - uintptr_t                 ae_desc.ad_info.ai_type = Dee_ATTRINFO_MEMBER;
	 *   - struct type_member const *ae_desc.ad_info.ai_value.v_member = <accessed member>;
	 *   "Dee_ATTRINFO_MEMBER" is changed to "Dee_ATTRINFO_INSTANCE_MEMBER"
	 *   if "ae_obj" is a type, and "v_member" is one of its "tp_members".
	 *   Alternatively, you can also set:
	 *   - struct type_member const  *ae_desc.ad_info.ai_value.v_member = (struct type_member *)Dee_attrdesc__type_member_buffer(&ae_desc);
	 *   - char const                *Dee_attrdesc__type_member_buffer(&ae_desc)->mb_name = <`m_name' of accessed type_member>;
	 *   - union Dee_type_member_desc Dee_attrdesc__type_member_buffer(&ae_desc)->mb_desc = <`m_desc' of accessed type_member>;
	 *   This behaves the same as the above, but also works then you don't have access
	 *   to the pointer to the original "struct type_member" from the relevant type's
	 *   "tp_members" or "tp_class_members", as is the case when called from a type's
	 *   "struct Dee_membercache_table".
	 *
	 * - Dee_ATTRINFO_ATTR:
	 *   Dee_ATTRINFO_INSTANCE_ATTR:
	 *   - DREF DeeObject               *ae_obj = <Accessed Object or Type>;
	 *   - uintptr_t                     ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR;
	 *   - struct class_attribute const *ae_desc.ad_info.ai_value.v_attr = <accessed attribute>;
	 *   "Dee_ATTRINFO_ATTR" is changed to "Dee_ATTRINFO_INSTANCE_ATTR" if "ae_obj"
	 *   is a type, and "v_attr" is one of its "tp_class->cd_desc->cd_iattr_list".
	 *
	 * - Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT:
	 *   - DREF DeeTypeObject *ae_obj = <Accessed Type (which must be DeeType_IsClass)>;
	 *   - uintptr_t           ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT;
	 *   - uint16_t            ae_desc.ad_info.ai_value.v_any = <Index into `struct class_desc::cd_members'>;
	 *   "Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT" is changed to "Dee_ATTRINFO_ATTR"
	 *   or "Dee_ATTRINFO_INSTANCE_ATTR" as appropriate.
	 *
	 * - Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT:
	 *   - DREF DeeObject *ae_obj = <Accessed instance object (or a class type)>;
	 *   - DeeTypeObject  *ae_desc.ad_info.ai_decl = <Accessed Type (which must be DeeType_IsClass)>; // -- !!NOT!! initialized as a reference!
	 *   - uintptr_t       ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT;
	 *   - uint16_t        ae_desc.ad_info.ai_value.v_any = <Index into `struct instance_desc::id_vtab'>;
	 *   "Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT" is changed to "Dee_ATTRINFO_ATTR".
	 */
} AttributeError;

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) struct class_attribute const *DCALL
ClassDescriptor_InstanceAttrAt(DeeClassDescriptorObject const *__restrict self, uint16_t addr) {
	size_t i;
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		struct class_attribute const *attr;
		attr = &self->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
			continue;
		return attr;
	}
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) struct class_attribute const *DCALL
ClassDescriptor_ClassAttrAt(DeeClassDescriptorObject const *__restrict self, uint16_t addr) {
	size_t i;
	struct class_attribute const *attr;
	for (i = 0; i <= self->cd_cattr_mask; ++i) {
		attr = &self->cd_cattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr >= attr->ca_addr &&
		    addr < (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			return attr;
	}
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		attr = &self->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
			continue;
		if (addr >= attr->ca_addr &&
		    addr < (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			return attr;
	}
	return NULL;
}

PRIVATE NONNULL((1)) DeeObject *DCALL
AttributeError_GetDecl_impl(AttributeError *__restrict self) {
	DeeTypeObject *decl_type;
	DeeObject *ob = self->ae_obj;
	ASSERTF(ob, "No object, but 'AttributeError_F_LAZYDECL' flag is set?");
	switch (self->ae_desc.ad_info.ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		DeeTypeMRO mro;
		struct type_attr const *custom = self->ae_desc.ad_info.ai_value.v_custom;
		ASSERTF(self->ae_desc.ad_name, "Dee_ATTRINFO_CUSTOM requires the caller to fill in the attribute name");
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (decl_type->tp_attr == custom)
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (decl_type->tp_attr == custom)
					goto return_decl_type;
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_MODSYM: {
		struct module_symbol const *sym = self->ae_desc.ad_info.ai_value.v_modsym;
		self->ae_desc.ad_name = sym->ss_name;
		self->ae_desc.ad_perm = 0;
		if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ) {
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
		}
		if likely(DeeModule_Check(ob)) {
			DeeModuleObject *mod = (DeeModuleObject *)ob;
			if likely(sym >= (mod->mo_bucketv) &&
			          sym <= (mod->mo_bucketv + mod->mo_bucketm)) {
				return (DeeObject *)mod;
			}
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_ATTR:
	case Dee_ATTRINFO_INSTANCE_ATTR: {
		DeeTypeMRO mro;
		struct class_attribute const *attr;
		attr = self->ae_desc.ad_info.ai_value.v_attr;
		self->ae_desc.ad_name = DeeString_STR(attr->ca_name);
		Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
		self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (DeeType_IsClass(decl_type) &&
			    ClassDescriptor_IsInstanceAttr(DeeClass_DESC(decl_type)->cd_desc, attr))
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (DeeType_IsClass(decl_type) &&
				    ClassDescriptor_IsClassAttr(DeeClass_DESC(decl_type)->cd_desc, attr))
					goto return_decl_type;
				if (DeeType_IsClass(decl_type) &&
				    ClassDescriptor_IsInstanceAttr(DeeClass_DESC(decl_type)->cd_desc, attr)) {
					self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_ATTR;
					goto return_decl_type;
				}
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
return_decl_type:
		return (DeeObject *)decl_type;
	}	break;

	case Dee_ATTRINFO_METHOD:
	case Dee_ATTRINFO_INSTANCE_METHOD: {
		DeeTypeMRO mro;
		struct type_method const *method;
		method = self->ae_desc.ad_info.ai_value.v_method;
		self->ae_desc.ad_name = method->m_name;
		self->ae_desc.ad_perm = 0;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (type_methods_contains(decl_type->tp_methods, method))
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (type_methods_contains(decl_type->tp_class_methods, method))
					goto return_decl_type;
				if (type_methods_contains(decl_type->tp_methods, method)) {
					self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_METHOD;
					goto return_decl_type;
				}
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_GETSET:
	case Dee_ATTRINFO_INSTANCE_GETSET: {
		DeeTypeMRO mro;
		struct type_getset const *getset;
		getset = self->ae_desc.ad_info.ai_value.v_getset;
		self->ae_desc.ad_name = getset->gs_name;
		self->ae_desc.ad_perm = 0;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (type_getsets_contains(decl_type->tp_getsets, getset))
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (type_getsets_contains(decl_type->tp_class_getsets, getset))
					goto return_decl_type;
				if (type_getsets_contains(decl_type->tp_getsets, getset)) {
					self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_GETSET;
					goto return_decl_type;
				}
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_MEMBER:
	case Dee_ATTRINFO_INSTANCE_MEMBER: {
		DeeTypeMRO mro;
		struct type_member const *member;
		member = self->ae_desc.ad_info.ai_value.v_member;
		self->ae_desc.ad_name = member->m_name;
		self->ae_desc.ad_perm = 0;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		if (member == type_member_buffer_asmember(Dee_attrdesc__type_member_buffer(&self->ae_desc))) {
			struct type_member_buffer *buffer;
			buffer = type_member_asmember_buffer(member);
			ASSERT(buffer == Dee_attrdesc__type_member_buffer(&self->ae_desc));
			do {
				member = type_members_findbuffer(decl_type->tp_members, buffer);
				if (member)
					goto got_member_from_buffer;
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			if (DeeType_Check(ob)) {
				decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
				do {
					member = type_members_findbuffer(decl_type->tp_class_members, buffer);
					if (member)
						goto got_member_from_buffer;
					member = type_members_findbuffer(decl_type->tp_members, buffer);
					if (member) {
						self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_MEMBER;
						goto got_member_from_buffer;
					}
				} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			}
		} else {
			do {
				if (type_members_contains(decl_type->tp_members, member))
					goto return_decl_type;
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			if (DeeType_Check(ob)) {
				decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
				do {
					if (type_members_contains(decl_type->tp_class_members, member))
						goto return_decl_type;
					if (type_members_contains(decl_type->tp_members, member)) {
						self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_MEMBER;
						goto return_decl_type;
					}
				} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			}
		}
		goto fail_no_clear_name;
got_member_from_buffer:
		self->ae_desc.ad_info.ai_value.v_member = member;
		goto return_decl_type;
	}	break;

	case Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT: {
		uint16_t addr = (uint16_t)(uintptr_t)self->ae_desc.ad_info.ai_value.v_any;
		DeeTypeObject *class_type = (DeeTypeObject *)self->ae_obj;
		struct class_attribute const *attr;
		DeeClassDescriptorObject *desc;
		ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(class_type));
		desc = DeeClass_DESC(class_type)->cd_desc;
		attr = ClassDescriptor_ClassAttrAt(desc, addr);
		if likely(attr) {
			self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR;
			if (ClassDescriptor_IsInstanceAttr(desc, attr))
				self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_ATTR;
			self->ae_desc.ad_info.ai_value.v_attr = attr;
			self->ae_desc.ad_name = DeeString_STR(attr->ca_name);
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
			return (DeeObject *)class_type;
		}
		goto fail_clear_name;
	}	break;

	case Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT: {
		uint16_t addr = (uint16_t)(uintptr_t)self->ae_desc.ad_info.ai_value.v_any;
		DeeTypeObject *class_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		struct class_attribute const *attr;
		ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(class_type));
		attr = ClassDescriptor_InstanceAttrAt(DeeClass_DESC(class_type)->cd_desc, addr);
		if likely(attr) {
			self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR;
			self->ae_desc.ad_info.ai_value.v_attr = attr;
			self->ae_desc.ad_name = DeeString_STR(attr->ca_name);
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
			return (DeeObject *)class_type;
		}
		goto fail_clear_name;
	}	break;

	default: break;
	}

	/* Shouldn't happen (unable to locate declaration) */
fail_clear_name:
	self->ae_desc.ad_name = NULL;
	self->ae_desc.ad_perm = 0;
fail_no_clear_name:
	return NULL;
}

PRIVATE NONNULL((1)) DeeObject *DCALL
AttributeError_GetDecl(AttributeError *__restrict self) {
	DeeObject *result;
	for (;;) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (!(flags & AttributeError_F_LAZYDECL))
			return atomic_read(&self->ae_desc.ad_info.ai_decl);
#ifndef CONFIG_NO_THREADS
		flags = atomic_fetchor(&self->ae_flags, AttributeError_F_LOADLOCK);
		if (flags & AttributeError_F_LOADLOCK) {
			SCHED_YIELD();
			continue;
		}
#endif /* !CONFIG_NO_THREADS */
		break;
	}
	ASSERTF(self->ae_obj, "No object, but 'AttributeError_F_LAZYDECL' flag is set?");
	result = AttributeError_GetDecl_impl(self);
	if unlikely(!result) {
		atomic_write(&self->ae_desc.ad_info.ai_decl, NULL);
		atomic_write(&self->ae_flags, 0);
		return NULL;
	}
	Dee_Incref(result);
	atomic_write(&self->ae_desc.ad_info.ai_decl, result);
#ifdef AttributeError_F_LOADLOCK
	atomic_and(&self->ae_flags, ~(AttributeError_F_LAZYDECL | AttributeError_F_LOADLOCK));
#else /* AttributeError_F_LOADLOCK */
	atomic_and(&self->ae_flags, ~(AttributeError_F_LAZYDECL));
#endif /* !AttributeError_F_LOADLOCK */
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
AttributeError_LoadInfo_impl(AttributeError *__restrict self,
                             struct Dee_attrinfo *__restrict info) {
	bool ok;
	char const *name;
	size_t namelen;
	Dee_hash_t namehash;
	DeeObject *decl;

	/* Load attribute name */
	name = self->ae_desc.ad_name;
	if unlikely(!name)
		return false;
	if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *name_ob;
		name_ob  = COMPILER_CONTAINER_OF(name, DeeStringObject, s_str);
		namelen  = DeeString_SIZE(name_ob);
		namehash = DeeString_Hash((DeeObject *)name_ob);
	} else {
		namelen  = strlen(name);
		namehash = Dee_HashPtr(name, namelen);
	}

	/* Load attribute info */
	ASSERTF(!(self->ae_flags & AttributeError_F_LAZYDECL),
	        "Must checked for- and cleared by the caller");
	decl = atomic_read(&self->ae_desc.ad_info.ai_decl);
	if (decl != NULL) {
		DeeObject *obj = self->ae_obj;
		DeeTypeObject *tp_obj = Dee_TYPE(obj);
		if (tp_obj == &DeeSuper_Type) {
			tp_obj = DeeSuper_TYPE(obj);
			obj    = DeeSuper_SELF(obj);
		}
		if (decl != self->ae_obj && DeeType_Check(decl) &&
		    DeeType_Implements(tp_obj, (DeeTypeObject *)decl)) {
			ok = DeeObject_TFindPrivateAttrInfoStringLenHash((DeeTypeObject *)decl,
			                                                 obj, name, namelen,
			                                                 namehash, info);
		} else {
			ok = DeeObject_FindAttrInfoStringLenHash(obj, name, namelen, namehash, info);
		}
		if (ok && info->ai_decl != decl)
			ok = false; /* Ensure that the correct declaring object is referenced */
	} else {
		ok = DeeObject_FindAttrInfoStringLenHash(self->ae_obj, name, namelen, namehash, info);
	}
	return ok;
}

/* Ensure that "AttributeError_F_INFOLOADED" is set
 * @return: true:  Success
 * @return: false: Attribute info cannot be loaded */
PRIVATE NONNULL((1)) bool DCALL
AttributeError_LoadInfo(AttributeError *__restrict self) {
	bool ok;
	struct Dee_attrinfo info;
	for (;;) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (flags & AttributeError_F_INFOLOADED)
			return true;
		if (flags & AttributeError_F_LAZYDECL) {
			AttributeError_GetDecl(self);
			continue;
		}
		if unlikely(!self->ae_obj)
			return false;
#ifndef CONFIG_NO_THREADS
		flags = atomic_fetchor(&self->ae_flags, AttributeError_F_LOADLOCK);
		if (flags & AttributeError_F_LOADLOCK) {
			SCHED_YIELD();
			continue;
		}
#endif /* !CONFIG_NO_THREADS */
		break;
	}

	ok = AttributeError_LoadInfo_impl(self, &info);

	/* If the specified attribute doesn't exist, then we can't access it */
	if likely(ok) {
		if (atomic_cmpxch(&self->ae_desc.ad_info.ai_decl, NULL, info.ai_decl))
			Dee_Incref(info.ai_decl);
		self->ae_desc.ad_info.ai_type  = info.ai_type;
		self->ae_desc.ad_info.ai_value = info.ai_value;
		COMPILER_WRITE_BARRIER();
		atomic_or(&self->ae_flags, (AttributeError_F_INFOLOADED |
		                            AttributeError_F_ISDEFAULT));
	}
#ifndef CONFIG_NO_THREADS
	atomic_and(&self->ae_flags, ~AttributeError_F_LOADLOCK);
#endif /* !CONFIG_NO_THREADS */
	return ok;
}

/* Ensure that "AttributeError_F_DESCLOADED" is set
 * @return: true:  Success
 * @return: false: Attribute info cannot be loaded */
PRIVATE NONNULL((1)) bool DCALL
AttributeError_LoadDesc(AttributeError *__restrict self) {
	for (;;) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (flags & AttributeError_F_DESCLOADED)
			return true;
		if (flags & AttributeError_F_LAZYDECL) {
			AttributeError_GetDecl(self);
			continue;
		}
		if (!(flags & AttributeError_F_INFOLOADED)) {
			if (!AttributeError_LoadInfo(self))
				return false;
			continue;
		}
#ifndef CONFIG_NO_THREADS
		flags = atomic_fetchor(&self->ae_flags, AttributeError_F_LOADLOCK);
		if (flags & AttributeError_F_LOADLOCK) {
			SCHED_YIELD();
			continue;
		}
#endif /* !CONFIG_NO_THREADS */
		break;
	}

	/* Initialize to defaults */
	self->ae_desc.ad_doc  = NULL;
	self->ae_desc.ad_type = NULL;
	self->ae_desc.ad_perm &= Dee_ATTRPERM_F_NAMEOBJ;

	/* Load attribute properties based on implementation */
	switch (self->ae_desc.ad_info.ai_type) {
	case Dee_ATTRINFO_MODSYM: {
		struct module_symbol const *sym;
		DeeModuleObject *mod;
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET;
		sym = self->ae_desc.ad_info.ai_value.v_modsym;
		if (!(sym->ss_flags & MODSYM_FREADONLY))
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET;
		if (sym->ss_flags & MODSYM_FPROPERTY) {
			self->ae_desc.ad_perm &= ~(Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
			if (!(sym->ss_flags & MODSYM_FCONSTEXPR))
				self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PROPERTY;
		}
		if (sym->ss_doc) {
			self->ae_desc.ad_doc = sym->ss_doc;
			if (sym->ss_flags & Dee_MODSYM_FDOCOBJ) {
				Dee_Incref(Dee_attrdesc_docobj(&self->ae_desc));
				self->ae_desc.ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
			}
		}
		mod = (DeeModuleObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(mod, &DeeModule_Type);
		if (mod->mo_flags & MODULE_FDIDINIT) {
			DeeModule_LockRead(mod);
			if (sym->ss_flags & MODSYM_FPROPERTY) {
				/* Check which property operations have been bound. */
				if (mod->mo_globalv[sym->ss_index + MODULE_PROPERTY_GET])
					self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET;
				if (!(sym->ss_flags & MODSYM_FREADONLY)) {
					/* These callbacks are only allocated if the READONLY flag isn't set. */
					if (mod->mo_globalv[sym->ss_index + MODULE_PROPERTY_DEL])
						self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANDEL;
					if (mod->mo_globalv[sym->ss_index + MODULE_PROPERTY_SET])
						self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANSET;
				}
			}
			DeeModule_LockEndRead(mod);
		}
	}	break;

	case Dee_ATTRINFO_METHOD: {
		DeeTypeObject *decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		if (type_methods_contains(decl_type->tp_class_methods, self->ae_desc.ad_info.ai_value.v_method)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (type_methods_contains(decl_type->tp_methods, self->ae_desc.ad_info.ai_value.v_method)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
	}	/* ... */
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_METHOD:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER |
			                         Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_CANGET |
			                         Dee_ATTRPERM_F_CANCALL;
		}
		self->ae_desc.ad_doc = self->ae_desc.ad_info.ai_value.v_method->m_doc;
		break;

	case Dee_ATTRINFO_GETSET: {
		DeeTypeObject *decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		if (type_getsets_contains(decl_type->tp_class_getsets, self->ae_desc.ad_info.ai_value.v_getset)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (type_getsets_contains(decl_type->tp_getsets, self->ae_desc.ad_info.ai_value.v_getset)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
	}	/* ... */
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PROPERTY;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_GETSET:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER |
			                         Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_PROPERTY;
		}
		if (self->ae_desc.ad_info.ai_value.v_getset->gs_get)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET;
		if (self->ae_desc.ad_info.ai_value.v_getset->gs_del)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANDEL;
		if (self->ae_desc.ad_info.ai_value.v_getset->gs_set)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANSET;
		self->ae_desc.ad_doc = self->ae_desc.ad_info.ai_value.v_getset->gs_doc;
		break;

	case Dee_ATTRINFO_MEMBER: {
		DeeTypeObject *decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		if (type_members_contains(decl_type->tp_class_members, self->ae_desc.ad_info.ai_value.v_member)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (type_members_contains(decl_type->tp_members, self->ae_desc.ad_info.ai_value.v_member)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
	}	/* ... */
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_MEMBER:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_WRAPPER;
		}
		self->ae_desc.ad_doc = self->ae_desc.ad_info.ai_value.v_member->m_doc;
		if (TYPE_MEMBER_ISCONST(self->ae_desc.ad_info.ai_value.v_member)) {
			/* Constant -- cannot del or set */
		} else if (self->ae_desc.ad_info.ai_value.v_member->m_desc.md_field.mdf_type & STRUCT_CONST) {
			/* Read-only field -- cannot del or set */
		} else {
			self->ae_desc.ad_perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		}
		break;

	case Dee_ATTRINFO_ATTR: {
		struct class_attribute const *attr;
		struct instance_desc *inst;
		DeeTypeObject *decl_type;
		decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(decl_type));
		if (ClassDescriptor_IsClassAttr(DeeClass_DESC(decl_type)->cd_desc, self->ae_desc.ad_info.ai_value.v_attr)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (ClassDescriptor_IsInstanceAttr(DeeClass_DESC(decl_type)->cd_desc, self->ae_desc.ad_info.ai_value.v_attr)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_ATTR:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER |
			                         Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_CANGET |
			                         Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET;
		}
		attr = self->ae_desc.ad_info.ai_value.v_attr;
		if (attr->ca_doc) {
			self->ae_desc.ad_doc = DeeString_STR(attr->ca_doc);
			Dee_Incref(Dee_attrdesc_docobj(&self->ae_desc));
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PRIVATE;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PROPERTY;
		} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANCALL;
		}
		inst = NULL;
		decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(decl_type));
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			inst = class_desc_as_instance(DeeClass_DESC(decl_type));
		} else if (DeeObject_InstanceOf(self->ae_obj, decl_type)) {
			inst = DeeInstance_DESC(DeeClass_DESC(decl_type), self->ae_obj);
		}
		if (inst != NULL) {
			Dee_instance_desc_lock_read(inst);
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
					self->ae_desc.ad_perm &= ~Dee_ATTRPERM_F_CANGET;
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
					if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
						self->ae_desc.ad_perm &= ~Dee_ATTRPERM_F_CANDEL;
					if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
						self->ae_desc.ad_perm &= ~Dee_ATTRPERM_F_CANSET;
				}
			}
			Dee_instance_desc_lock_endread(inst);
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			self->ae_desc.ad_perm &= ~(Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
	}	break;

	default: break;
	}

	atomic_or(&self->ae_flags, AttributeError_F_DESCLOADED);
#ifndef CONFIG_NO_THREADS
	atomic_and(&self->ae_flags, ~AttributeError_F_LOADLOCK);
#endif /* !CONFIG_NO_THREADS */
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
AttributeError_LoadIsDefault(AttributeError *__restrict self) {
	bool ok;
	struct Dee_attrinfo info;
	unsigned int flags = atomic_read(&self->ae_flags);
	if (flags & AttributeError_F_ISDEFAULT)
		return true;
	if (flags & AttributeError_F_NODEFAULT)
		return false;
	if (!(flags & AttributeError_F_INFOLOADED))
		return AttributeError_LoadDesc(self);
	if (!AttributeError_LoadDesc(self))
		return false;
	ok = AttributeError_LoadInfo_impl(self, &info);
	if (ok) {
		ok = (info.ai_decl == AttributeError_GetDecl(self) &&
		      info.ai_type == self->ae_desc.ad_info.ai_type &&
		      info.ai_value.v_any == self->ae_desc.ad_info.ai_value.v_any);
	}
	atomic_or(&self->ae_flags, ok ? AttributeError_F_ISDEFAULT
	                              : AttributeError_F_NODEFAULT);
	return ok;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_init_access_flag(AttributeError *__restrict self,
                                DeeObject *state, unsigned int mask) {
	int temp;
	if (!state)
		return 0;
	temp = DeeObject_Bool(state);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		self->ae_flags |= mask;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_init_kw(AttributeError *__restrict self, size_t argc,
                       DeeObject *const *argv, DeeObject *kw) {
	DeeKwArgs kwds;
	DeeObject *attr;
	DeeObject *isget, *isdel, *isset;
	if unlikely(DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
#define LOADARG(T, dst, i, name)            \
	do {                                    \
		if ((i) < argc)                     \
			*(dst) = (T *)argv[i];          \
		else {                              \
			*(dst) = (T *)DeeKwArgs_TryGetItemNRStringHash(&kwds, #name, Dee_HashStr__##name); \
			if (!ITER_ISOK(*(dst))) {       \
				if unlikely(*(dst) == NULL) \
					goto err;               \
				*(dst) = NULL;              \
			}                               \
		}                                   \
	}	__WHILE0
#define AttributeError_init_params   Error_init_params ",ob?,attr?:?X2?Dstring?DAttribute,decl?:?X3?DType?DModule?O,isget=!f,isdel=!f,isset=!f"
#define UnboundAttribute_init_params Error_init_params ",ob?,attr?:?X2?Dstring?DAttribute,decl?:?X3?DType?DModule?O,isget=!t,isdel=!f,isset=!f"
	LOADARG(DeeStringObject, &self->e_message, 0, message);
	LOADARG(DeeObject, &self->e_inner, 1, inner);
	LOADARG(DeeObject, &self->ae_obj, 2, ob);
	LOADARG(DeeObject, &attr, 3, attr);
	LOADARG(DeeObject, &self->ae_desc.ad_info.ai_decl, 4, decl);
	LOADARG(DeeObject, &isget, 5, isget);
	LOADARG(DeeObject, &isdel, 6, isdel);
	LOADARG(DeeObject, &isset, 7, isset);
#undef LOADARG
	if (argc > 8)
		return DeeArg_BadArgcEx(Dee_TYPE(self)->tp_name, argc, 0, 8);
	self->ae_flags = 0;
	if unlikely(AttributeError_init_access_flag(self, isget, AttributeError_F_GET))
		goto err;
	if unlikely(AttributeError_init_access_flag(self, isdel, AttributeError_F_DEL))
		goto err;
	if unlikely(AttributeError_init_access_flag(self, isset, AttributeError_F_SET))
		goto err;
	/* Hacky special case: in "UnboundAttribute", "isget" defaults to "true" */
	if (!isget && DeeObject_InstanceOf(self, &DeeError_UnboundAttribute))
		self->ae_flags |= AttributeError_F_GET;
	if (attr) {
		if unlikely(!self->ae_obj)
			return DeeError_Throwf(&DeeError_TypeError, "%s: 'attr' given, but no 'ob'", Dee_TYPE(self)->tp_name);
		if (DeeObject_InstanceOf(attr, &DeeAttribute_Type)) {
			DeeAttributeObject *attrib = (DeeAttributeObject *)attr;
			if unlikely(self->ae_desc.ad_info.ai_decl)
				return DeeError_Throwf(&DeeError_TypeError, "%s: 'decl' given, but 'attr' is an Attribute and not a string", Dee_TYPE(self)->tp_name);
			Dee_attrdesc_init_copy(&self->ae_desc, &attrib->a_desc);
			ASSERT(self->ae_desc.ad_info.ai_decl == attrib->a_desc.ad_info.ai_decl);
			ASSERT(self->ae_desc.ad_info.ai_decl);
			Dee_Incref(self->ae_desc.ad_info.ai_decl);
			self->ae_flags |= (AttributeError_F_INFOLOADED | AttributeError_F_DESCLOADED);
		} else if (DeeString_Check(attr)) {
			Dee_Incref(attr);
			self->ae_desc.ad_name = DeeString_STR(attr);
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
		} else {
			DeeObject_TypeAssertFailed2(attr, &DeeAttribute_Type, &DeeString_Type);
			goto err;
		}
		Dee_XIncref(self->ae_desc.ad_info.ai_decl);
		Dee_Incref(self->ae_obj);
	} else {
		if unlikely(self->ae_desc.ad_info.ai_decl)
			return DeeError_Throwf(&DeeError_TypeError, "%s: 'decl' given, but no 'attr'", Dee_TYPE(self)->tp_name);
		if unlikely(self->ae_obj)
			return DeeError_Throwf(&DeeError_TypeError, "%s: 'ob' given, but no 'attr'", Dee_TYPE(self)->tp_name);
	}
	Dee_XIncref(self->e_message);
	Dee_XIncref(self->e_inner);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_copy(AttributeError *__restrict self,
                    AttributeError *__restrict other) {
	self->e_message = other->e_message;
	Dee_XIncref(self->e_message);
	self->e_inner = other->e_inner;
	Dee_XIncref(self->e_inner);
	(void)AttributeError_LoadDesc(other); /* Make sure that "other" is stable. */
	(void)AttributeError_LoadDesc(other); /* ... */
	self->ae_obj = other->ae_obj;
	self->ae_flags = other->ae_flags;
	if (self->ae_obj) {
		self->ae_desc = other->ae_desc;
		Dee_XIncref(self->ae_desc.ad_info.ai_decl);
		if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
		if (self->ae_flags & AttributeError_F_DESCLOADED) {
			if ((self->ae_desc.ad_perm & Dee_ATTRPERM_F_DOCOBJ) && self->ae_desc.ad_doc)
				Dee_Incref(Dee_attrdesc_docobj(&self->ae_desc));
			Dee_XIncref(self->ae_desc.ad_type);
		}
		Dee_Incref(self->ae_obj);
	}
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
AttributeError_fini(AttributeError *__restrict self) {
	if (self->ae_obj) {
		if (!(self->ae_flags & AttributeError_F_LAZYDECL)) {
			Dee_XDecref(self->ae_desc.ad_info.ai_decl);
			if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
				Dee_Decref(Dee_attrdesc_nameobj(&self->ae_desc));
			if (self->ae_flags & AttributeError_F_DESCLOADED) {
				if ((self->ae_desc.ad_perm & Dee_ATTRPERM_F_DOCOBJ) && self->ae_desc.ad_doc)
					Dee_Decref(Dee_attrdesc_docobj(&self->ae_desc));
				Dee_XDecref(self->ae_desc.ad_type);
			}
		}
		Dee_Decref(self->ae_obj);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_deep(AttributeError *__restrict self,
                    AttributeError *__restrict other) {
	if unlikely(AttributeError_copy(self, other))
		goto err;
	if unlikely(DeeObject_XInplaceDeepCopy((DeeObject **)&self->e_inner))
		goto err_self;
	if unlikely(DeeObject_XInplaceDeepCopy((DeeObject **)&self->ae_obj))
		goto err_self;
	return 0;
err_self:
	AttributeError_fini(self);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
AttributeError_visit(AttributeError *__restrict self, Dee_visit_t proc, void *arg) {
	if (self->ae_obj) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (!(flags & AttributeError_F_LAZYDECL)) {
			DeeObject *decl = atomic_read(&self->ae_desc.ad_info.ai_decl);
			Dee_Visit(decl);
		}
		if (self->ae_flags & AttributeError_F_DESCLOADED)
			Dee_XVisit(self->ae_desc.ad_type);
		Dee_Visit(self->ae_obj);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
AttributeError_print_access_mode(AttributeError *__restrict self,
                                 Dee_formatprinter_t printer, void *arg) {
	char const *word;
	char buffer[sizeof("{get,del,set}")];
	unsigned int flags = atomic_read(&self->ae_flags);
	switch (flags) {
	case 0:
		word = "access";
		break;
	case AttributeError_F_GET:
		word = "read";
		break;
	case AttributeError_F_DEL:
		word = "delete";
		break;
	case AttributeError_F_SET:
		word = "write";
		break;
	default: {
		char *iter = buffer;
		*iter++ = '{';
		if (flags & AttributeError_F_GET)
			iter = stpcpy(iter, "get");
		if (flags & AttributeError_F_DEL) {
			if (flags & AttributeError_F_GET)
				*iter++ = ',';
			iter = stpcpy(iter, "del");
		}
		if (flags & AttributeError_F_SET) {
			if (flags & (AttributeError_F_GET | AttributeError_F_DEL))
				*iter++ = ',';
			iter = stpcpy(iter, "set");
		}
		*iter++ = '}';
		*iter++ = '\0';

		word = buffer;
	}	break;
	}
	return DeeFormat_PrintStr(printer, arg, word);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
AttributeError_print_attr_str(AttributeError *__restrict self,
                              Dee_formatprinter_t printer, void *arg) {
	DeeObject *decl = AttributeError_GetDecl(self);
	if (decl == NULL)
		decl = self->ae_obj;
	if (decl == NULL)
		return DeeFormat_PRINT(printer, arg, "?.?");
	ASSERT(self->ae_desc.ad_name);
	if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		return DeeFormat_Printf(printer, arg, "%k.%k", decl,
		                        Dee_attrdesc_nameobj(&self->ae_desc));
	}
	return DeeFormat_Printf(printer, arg, "%k.%s",
	                        decl, self->ae_desc.ad_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
AttributeError_print(AttributeError *__restrict self,
                     Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (self->e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	result = DeeFormat_Printf(printer, arg, "<%s", Dee_TYPE(self)->tp_name);
	if unlikely(result < 0)
		goto done;
	DO(err_temp, DeeFormat_PRINT(printer, arg, " during "));
	DO(err_temp, AttributeError_print_access_mode(self, printer, arg));
	DO(err_temp, DeeFormat_PRINT(printer, arg, " of "));
	DO(err_temp, AttributeError_print_attr_str(self, printer, arg));
	DO(err_temp, DeeFormat_PRINT(printer, arg, ">"));
done:
	return result;
err_temp:
	return temp;
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
attr_printrepr_impl(struct Dee_attrdesc const *__restrict self,
                    Dee_formatprinter_t printer, void *arg);

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
AttributeError_printrepr(AttributeError *__restrict self,
                         Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	char const *prefix = "";
	result = DeeFormat_Printf(printer, arg, "%s(", Dee_TYPE(self)->tp_name);
	if unlikely(result < 0)
		goto done;
	if (self->e_message) {
		DO(err_temp, DeeFormat_Printf(printer, arg, "message: %r", self->e_message));
		prefix = ", ";
	}
	if (self->e_inner) {
		DO(err_temp, DeeFormat_Printf(printer, arg, "%sinner: %r", prefix, self->e_inner));
		prefix = ", ";
	}
	if (self->ae_obj) {
		DeeObject *decl = AttributeError_GetDecl(self);
		if (self->ae_desc.ad_name) {
			DO(err_temp, DeeFormat_Printf(printer, arg, "%sob: %r, attr: ", prefix, self->ae_obj));
			if (AttributeError_LoadDesc(self) && !AttributeError_LoadIsDefault(self)) {
				DO(err_temp, attr_printrepr_impl(&self->ae_desc, printer, arg));
			} else {
				DO(err_temp, (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
				   ? DeeString_PrintRepr((DeeObject *)Dee_attrdesc_nameobj(&self->ae_desc), printer, arg)
				   : DeeFormat_Printf(printer, arg, "%q", self->ae_desc.ad_name));
				if (decl != NULL)
					DO(err_temp, DeeFormat_Printf(printer, arg, ", decl: %r", decl));
			}
		} else {
			DO(err_temp, DeeFormat_Printf(printer, arg, "%sob: %r", prefix, self->ae_obj));
		}
	}
	if (self->ae_flags & AttributeError_F_GET)
		DO(err_temp, DeeFormat_PRINT(printer, arg, ", isget: true"));
	if (self->ae_flags & AttributeError_F_DEL)
		DO(err_temp, DeeFormat_PRINT(printer, arg, ", isdel: true"));
	if (self->ae_flags & AttributeError_F_SET)
		DO(err_temp, DeeFormat_PRINT(printer, arg, ", isset: true"));
	DO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return -1;
}

PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
dee_memcmp2(void const *lhs, size_t lhs_size,
            void const *rhs, size_t rhs_size) {
	size_t common = MIN(lhs_size, rhs_size);
	int result = memcmp(lhs, rhs, common * sizeof(char));
	if (result < -1) {
		result = -1;
	} else if (result > 1) {
		result = 1;
	} else if (result == 0) {
		if (lhs_size < rhs_size) {
			result = -1;
		} else if (lhs_size > rhs_size) {
			result = 1;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
AttributeError_compare_impl(AttributeError *lhs, AttributeError *rhs,
                            int (DCALL *cmp)(DeeObject *, DeeObject *)) {
	DeeObject *lhs_decl, *rhs_decl;
	int result;
	if (lhs->e_message != rhs->e_message) {
		if (!lhs->e_message)
			return -1;
		if (!rhs->e_message)
			return 1;
		result = (*cmp)((DeeObject *)lhs->e_message,
		                (DeeObject *)rhs->e_message);
		if (result != 0)
			return result;
	}
	if (lhs->e_inner != rhs->e_inner) {
		if (!lhs->e_inner)
			return -1;
		if (!rhs->e_inner)
			return 1;
		result = (*cmp)(lhs->e_inner, rhs->e_inner);
		if (result != 0)
			return result;
	}
	if (lhs->ae_obj != rhs->ae_obj) {
		if (!lhs->ae_obj)
			return -1;
		if (!rhs->ae_obj)
			return 1;
#if 1 /* If instances differ, then then the errors aren't the same */
		return Dee_CompareNe(DeeObject_Id(lhs->ae_obj),
		                     DeeObject_Id(rhs->ae_obj));
#else
		result = (*cmp)(lhs->ae_obj, rhs->ae_obj);
		if (result != 0)
			return result;
#endif
	}

	AttributeError_LoadInfo(lhs);
	AttributeError_LoadInfo(rhs);
	lhs_decl = AttributeError_GetDecl(lhs);
	rhs_decl = AttributeError_GetDecl(rhs);
	/* Only touch names after decls were loaded. */
	if (lhs->ae_desc.ad_name != rhs->ae_desc.ad_name) {
		if (!lhs->ae_desc.ad_name)
			return -1;
		if (!rhs->ae_desc.ad_name)
			return 1;
		if ((lhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) &&
		    (rhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)) {
			DeeStringObject *lhs_name = Dee_attrdesc_nameobj(&lhs->ae_desc);
			DeeStringObject *rhs_name = Dee_attrdesc_nameobj(&rhs->ae_desc);
			result = (*cmp)((DeeObject *)lhs_name, (DeeObject *)rhs_name);
		} else {
			char const *lhs_str = lhs->ae_desc.ad_name;
			char const *rhs_str = rhs->ae_desc.ad_name;
			size_t lhs_len = (lhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) ? WSTR_LENGTH(lhs_str) : strlen(lhs_str);
			size_t rhs_len = (rhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) ? WSTR_LENGTH(rhs_str) : strlen(rhs_str);
			result = dee_memcmp2(lhs_str, lhs_len, rhs_str, rhs_len);
		}
		if (result != 0)
			return result;
	}
	if (lhs_decl != rhs_decl) {
		if (!lhs_decl)
			return -1;
		if (!rhs_decl)
			return 1;
#if 1 /* If instances differ, then then the errors aren't the same */
		return Dee_CompareNe(DeeObject_Id(lhs_decl),
		                     DeeObject_Id(rhs_decl));
#else
		if ((lhs_decl == lhs->ae_obj) &&
		    (rhs_decl == rhs->ae_obj)) {
			/* Already compared (and was equal) */
		} else {
			result = (*cmp)(lhs_decl,
			                rhs_decl);
			if (result != 0)
				return result;
		}
#endif
	}

	/* Compare access mode. */
	{
		unsigned int lhs_flags = atomic_read(&lhs->ae_flags) & AttributeError_F_ACCESS;
		unsigned int rhs_flags = atomic_read(&rhs->ae_flags) & AttributeError_F_ACCESS;
		if (lhs_flags != rhs_flags)
			return Dee_CompareNe(lhs_flags, rhs_flags);
	}

	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
AttributeError_hash(AttributeError *__restrict self) {
	Dee_hash_t result = DeeObject_HashGeneric(Dee_TYPE(self));
	if (self->e_message)
		result = Dee_HashCombine(result, DeeObject_Hash((DeeObject *)self->e_message));
	if (self->e_inner)
		result = Dee_HashCombine(result, DeeObject_Hash(self->e_inner));
	if (self->ae_obj) {
		DeeObject *decl;
		result = Dee_HashCombine(result, DeeObject_Hash(self->ae_obj));
		AttributeError_LoadInfo(self);
		decl = AttributeError_GetDecl(self);
		/* Only touch names after decls were loaded. */
		if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
			result = Dee_HashCombine(result, DeeString_Hash((DeeObject *)Dee_attrdesc_nameobj(&self->ae_desc)));
		} else {
			result = Dee_HashCombine(result, Dee_HashStr(self->ae_desc.ad_name));
		}
		if (decl && decl != self->ae_obj)
			result = Dee_HashCombine(result, DeeObject_Hash(decl));
	}
	result = Dee_HashCombine(result, atomic_read(&self->ae_flags) & AttributeError_F_ACCESS);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_compare_eq(AttributeError *self, AttributeError *some_object) {
	if (DeeObject_AssertTypeExact(some_object, Dee_TYPE(self)))
		goto err;
	return AttributeError_compare_impl(self, some_object, &DeeObject_TryCompareEq);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_compare(AttributeError *self, AttributeError *some_object) {
	if (DeeObject_AssertTypeExact(some_object, Dee_TYPE(self)))
		goto err;
	return AttributeError_compare_impl(self, some_object, &DeeObject_Compare);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_trycompare_eq(AttributeError *self, AttributeError *some_object) {
	if (!DeeObject_InstanceOfExact(some_object, Dee_TYPE(self)))
		return 1;
	return AttributeError_compare_impl(self, some_object, &DeeObject_TryCompareEq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeAttributeObject *DCALL
AttributeError_get_attr(AttributeError *__restrict self) {
	DREF DeeAttributeObject *result;
	if unlikely(!AttributeError_LoadDesc(self))
		goto err_unbound;
	result = DeeObject_MALLOC(DeeAttributeObject);
	if unlikely(!result)
		goto err;
	Dee_attrdesc_init_copy(&result->a_desc, &self->ae_desc);
	Dee_Incref(result->a_desc.ad_info.ai_decl);
	DeeObject_Init(result, &DeeAttribute_Type);
	return result;
err_unbound:
	return (DREF DeeAttributeObject *)DeeRT_ErrTUnboundAttrCStr(&DeeError_AttributeError, self, "attr");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_bound_attr(AttributeError *__restrict self) {
	bool has_desc = AttributeError_LoadDesc(self);
	return Dee_BOUND_FROMBOOL(has_desc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
AttributeError_get_name(AttributeError *__restrict self) {
	AttributeError_GetDecl(self);
	if unlikely(!self->ae_desc.ad_name)
		goto err_unbound;
	if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *result = Dee_attrdesc_nameobj(&self->ae_desc);
		Dee_Incref(result);
		return (DREF DeeObject *)result;
	}
	return DeeString_New(self->ae_desc.ad_name);
err_unbound:
	return DeeRT_ErrTUnboundAttrCStr(&DeeError_AttributeError, self, "name");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_bound_name(AttributeError *__restrict self) {
	AttributeError_GetDecl(self);
	return Dee_BOUND_FROMBOOL(self->ae_desc.ad_name != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
AttributeError_get_decl(AttributeError *__restrict self) {
	DeeObject *result = AttributeError_GetDecl(self);
	if unlikely(!result)
		goto err_unbound;
	Dee_Incref(result);
	return result;
err_unbound:
	return DeeRT_ErrTUnboundAttrCStr(&DeeError_AttributeError, self, "decl");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_bound_decl(AttributeError *__restrict self) {
	DeeObject *decl = AttributeError_GetDecl(self);
	return Dee_BOUND_FROMBOOL(decl != NULL);
}

PRIVATE struct Dee_type_cmp AttributeError_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&AttributeError_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&AttributeError_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *self, DeeObject *))&AttributeError_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&AttributeError_trycompare_eq,
};

PRIVATE struct type_getset tpconst AttributeError_getsets[] = {
	TYPE_GETTER_BOUND_F("attr", &AttributeError_get_attr, &AttributeError_bound_attr,
	                    METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                    "->?DAttribute\n"
	                    "The attribute that caused the error (or unbound if "
	                    /**/ "the attribute is unknown, or the point of the error is "
	                    /**/ "to complain about an unknown attribute)"),
	TYPE_GETTER_BOUND_F("name", &AttributeError_get_name, &AttributeError_bound_name,
	                    METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                    "->?Dstring\n"
	                    "Name of the attribute that was being accessed"),
	TYPE_GETTER_BOUND_F("decl", &AttributeError_get_decl, &AttributeError_bound_decl,
	                    METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                    "->?X3?DType?DModule?O\n"
	                    "The object (or ?DType or ?DModule) that is declaring the attribute"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst AttributeError_members[] = {
	TYPE_MEMBER_FIELD_DOC("ob", STRUCT_OBJECT, offsetof(AttributeError, ae_obj),
	                      "The object whose attributes were accessed"),
	TYPE_MEMBER_BITFIELD_DOC("isget", STRUCT_CONST, AttributeError, ae_flags, AttributeError_F_GET,
	                         "True if the error happened during an attribute read"),
	TYPE_MEMBER_BITFIELD_DOC("isdel", STRUCT_CONST, AttributeError, ae_flags, AttributeError_F_DEL,
	                         "True if the error happened during an attribute delete"),
	TYPE_MEMBER_BITFIELD_DOC("isset", STRUCT_CONST, AttributeError, ae_flags, AttributeError_F_SET,
	                         "True if the error happened during an attribute write"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst AttributeError_class_members[] = {
	TYPE_MEMBER_CONST("UnboundAttribute", &DeeError_UnboundAttribute),
	TYPE_MEMBER_CONST("UnknownAttribute", &DeeError_UnknownAttribute),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_AttributeError = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "AttributeError",
	/* .tp_doc      = */ "(" AttributeError_init_params ")",
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeError_Error,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&AttributeError_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&AttributeError_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(AttributeError),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&AttributeError_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&AttributeError_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&AttributeError_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&AttributeError_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&AttributeError_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &AttributeError_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ AttributeError_getsets,
	/* .tp_members       = */ AttributeError_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ AttributeError_class_members
};


#define INIT_LIKE_ATTRIBUTE_ERROR(tp_name, tp_doc, tp_flags,                \
                                  tp_base, tp_str, tp_print,                \
                                  tp_methods, tp_getsets, tp_class_members) \
	{                                                                       \
		OBJECT_HEAD_INIT(&DeeType_Type),                                    \
		/* .tp_name     = */ tp_name,                                       \
		/* .tp_doc      = */ DOC(tp_doc),                                   \
		/* .tp_flags    = */ TP_FNORMAL | (tp_flags),                       \
		/* .tp_weakrefs = */ 0,                                             \
		/* .tp_features = */ TF_NONE,                                       \
		/* .tp_base     = */ tp_base,                                       \
		/* .tp_init = */ {                                                  \
			{                                                               \
				/* .tp_alloc = */ {                                         \
					/* .tp_ctor      = */ (Dee_funptr_t)NULL,               \
					/* .tp_copy_ctor = */ (Dee_funptr_t)&AttributeError_copy, \
					/* .tp_deep_ctor = */ (Dee_funptr_t)&AttributeError_deep, \
					/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,               \
					TYPE_FIXED_ALLOCATOR(AttributeError),                   \
					/* .tp_any_ctor_kw = */ (Dee_funptr_t)&AttributeError_init_kw, \
				}                                                           \
			},                                                              \
			/* .tp_dtor        = */ NULL,                                   \
			/* .tp_assign      = */ NULL,                                   \
			/* .tp_move_assign = */ NULL                                    \
		},                                                                  \
		/* .tp_cast = */ {                                                  \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
			/* .tp_repr      = */ NULL,                                     \
			/* .tp_bool      = */ NULL,                                     \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&AttributeError_printrepr, \
		},                                                                  \
		/* .tp_visit         = */ NULL,                                     \
		/* .tp_gc            = */ NULL,                                     \
		/* .tp_math          = */ NULL,                                     \
		/* .tp_cmp           = */ &AttributeError_cmp,                      \
		/* .tp_seq           = */ NULL,                                     \
		/* .tp_iter_next     = */ NULL,                                     \
		/* .tp_iterator      = */ NULL,                                     \
		/* .tp_attr          = */ NULL,                                     \
		/* .tp_with          = */ NULL,                                     \
		/* .tp_buffer        = */ NULL,                                     \
		/* .tp_methods       = */ tp_methods,                               \
		/* .tp_getsets       = */ tp_getsets,                               \
		/* .tp_members       = */ NULL,                                     \
		/* .tp_class_methods = */ NULL,                                     \
		/* .tp_class_getsets = */ NULL,                                     \
		/* .tp_class_members = */ tp_class_members                          \
	}

PRIVATE NONNULL((1, 2)) void DCALL
unwrap_decl_and_ob(DeeObject **p_decl, DeeObject **p_ob) {
	if (*p_decl == (DeeObject *)&DeeSuper_Type &&
	    DeeObject_InstanceOf(*p_ob, &DeeSuper_Type)) {
		*p_decl = (DeeObject *)DeeSuper_TYPE(*p_ob);
		*p_ob   = DeeSuper_SELF(*p_ob);
	}
}

PRIVATE ATTR_COLD NONNULL((1, 3, 4)) int DCALL
DeeRT_ErrAttributeError_impl(DeeTypeObject *error_type, DeeObject *decl,
                             DeeObject *ob, DeeObject *attr,
                             unsigned int flags) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	unwrap_decl_and_ob(&decl, &ob);
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERTF((flags & ~AttributeError_F_ACCESS) == 0,
	        "Only these flags may be specified");
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	Dee_Incref(attr);
	result->ae_desc.ad_name = DeeString_STR(attr);
	result->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
	Dee_XIncref(decl);
	result->ae_desc.ad_info.ai_decl = decl;
	result->ae_flags = flags;
	DeeObject_Init(result, error_type);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PRIVATE ATTR_COLD NONNULL((1, 3, 4)) int DCALL
DeeRT_ErrAttributeErrorCStr_impl(DeeTypeObject *error_type, DeeObject *decl,
                                 DeeObject *ob, /*static*/ char const *attr,
                                 unsigned int flags) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	unwrap_decl_and_ob(&decl, &ob);
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERTF((flags & ~(AttributeError_F_GET |
	                   AttributeError_F_DEL |
	                   AttributeError_F_SET)) == 0,
	        "Only these flags may be specified");
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	result->ae_desc.ad_name = attr;
	result->ae_desc.ad_perm = 0;
	Dee_XIncref(decl);
	result->ae_desc.ad_info.ai_decl = decl;
	result->ae_flags = flags;
	DeeObject_Init(result, error_type);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}



/************************************************************************/
/* Error.AttributeError.UnboundAttribute                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
UnboundAttribute_print(AttributeError *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (self->e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	result = DeeFormat_PRINT(printer, arg, "Unbound attribute ");
	if likely(result >= 0) {
		temp = AttributeError_print_attr_str(self, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PUBLIC DeeTypeObject DeeError_UnboundAttribute =
INIT_LIKE_ATTRIBUTE_ERROR("UnboundAttribute", "(" UnboundAttribute_init_params ")",
                          TP_FNORMAL, &DeeError_AttributeError, NULL, &UnboundAttribute_print,
                          NULL, NULL, NULL);

/* Throws an `DeeError_UnboundAttribute' indicating that some attribute isn't bound
 * @return: NULL: Always returns "NULL" (for easy chaining when called form getters) */
PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundAttr)(DeeObject *ob, /*string*/ DeeObject *attr) {
	DeeRT_ErrAttributeError_impl(&DeeError_UnboundAttribute, NULL,
	                             ob, attr, AttributeError_F_GET);
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundAttrCStr)(DeeObject *ob, /*static*/ char const *attr) {
	DeeRT_ErrAttributeErrorCStr_impl(&DeeError_UnboundAttribute, NULL,
	                                 ob, attr, AttributeError_F_GET);
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundMember)(DeeObject *ob, struct Dee_type_member const *attr) {
	struct type_member_buffer *buffer;
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_MEMBER; /* Changed to "Dee_ATTRINFO_INSTANCE_MEMBER" if appropriate */
	buffer = Dee_attrdesc__type_member_buffer(&result->ae_desc);
	result->ae_desc.ad_info.ai_value.v_member = type_member_buffer_asmember(buffer);
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	/* Special case: We might get here from a context where "attr" is a stack-allocated copy
	 *               of an incomplete (as in: missing the doc string) member descriptor that
	 *               was previously copied out of `struct Dee_membercache_slot'.
	 * To deal with this case, we have to create yet another copy that will then be resolved
	 * lazily when the attribute error's declaration location is loaded. */
	type_member_buffer_init(buffer, attr);
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundAttrEx)(DeeObject *ob, struct Dee_attrdesc const *attr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	Dee_attrdesc_init_copy(&result->ae_desc, attr);
	Dee_Incref(result->ae_desc.ad_info.ai_decl);
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_DESCLOADED;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) DeeObject *
(DCALL DeeRT_ErrTUnboundAttr)(DeeObject *decl, DeeObject *ob, /*string*/ DeeObject *attr) {
	DeeRT_ErrAttributeError_impl(&DeeError_UnboundAttribute, decl, ob,
	                             attr, AttributeError_F_GET);
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) DeeObject *
(DCALL DeeRT_ErrTUnboundAttrCStr)(DeeObject *decl, DeeObject *ob, /*static*/ char const *attr) {
	DeeRT_ErrAttributeErrorCStr_impl(&DeeError_UnboundAttribute, decl, ob,
	                                 attr, AttributeError_F_GET);
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrCUnboundAttrCA)(DeeObject *ob, struct Dee_class_attribute const *attr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR; /* Changed to "Dee_ATTRINFO_INSTANCE_ATTR" if appropriate */
	result->ae_desc.ad_info.ai_value.v_attr = attr;
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrCUnboundInstanceMember)(DeeTypeObject *class_type,
                                        DeeObject *instance, uint16_t addr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERT_OBJECT_TYPE_A(instance, class_type);
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(instance);
	result->ae_obj = instance;
	result->ae_desc.ad_info.ai_decl = (DeeObject *)class_type;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT;
	result->ae_desc.ad_info.ai_value.v_any = (void *)(uintptr_t)addr;
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1)) DeeObject *
(DCALL DeeRT_ErrCUnboundClassMember)(DeeTypeObject *class_type, uint16_t addr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(class_type);
	result->ae_obj = (DeeObject *)class_type;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT;
	result->ae_desc.ad_info.ai_value.v_any = (void *)(uintptr_t)addr;
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}



/************************************************************************/
/* Error.AttributeError.UnknownAttribute                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
UnknownAttribute_print(AttributeError *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (self->e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	result = DeeFormat_PRINT(printer, arg, "Cannot ");
	if unlikely(result < 0)
		goto done;
	DO(err_temp, AttributeError_print_access_mode(self, printer, arg));
	DO(err_temp, DeeFormat_PRINT(printer, arg, " unknown attribute "));
	DO(err_temp, AttributeError_print_attr_str(self, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PUBLIC DeeTypeObject DeeError_UnknownAttribute =
INIT_LIKE_ATTRIBUTE_ERROR("UnknownAttribute", "(" AttributeError_init_params ")",
                          TP_FNORMAL, &DeeError_AttributeError, NULL, &UnknownAttribute_print,
                          NULL, NULL, NULL);

/* Throws an `DeeError_UnknownAttribute' indicating that some attribute doesn't exist */
PUBLIC ATTR_COLD NONNULL((2, 3)) int
(DCALL DeeRT_ErrTUnknownAttr)(DeeObject *decl, DeeObject *ob,
                              DeeObject *attr, unsigned int access) {
	return DeeRT_ErrAttributeError_impl(&DeeError_UnknownAttribute, decl, ob, attr, access);
}

PUBLIC ATTR_COLD NONNULL((2, 3)) int
(DCALL DeeRT_ErrTUnknownAttrStr)(DeeObject *decl, DeeObject *ob,
                                 char const *attr, unsigned int access) {
	int result;
	DREF DeeObject *attr_ob;
	if (DeeSystem_IsStaticPointer(attr))
		return DeeRT_ErrAttributeErrorCStr_impl(&DeeError_UnknownAttribute, decl, ob, attr, access);
	attr_ob = DeeString_New(attr);
	if unlikely(!attr_ob)
		goto err;
	result = DeeRT_ErrAttributeError_impl(&DeeError_UnknownAttribute, decl, ob, attr_ob, access);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((2, 3)) int
(DCALL DeeRT_ErrTUnknownAttrStrLen)(DeeObject *decl, DeeObject *ob,
                                    char const *attr, size_t attrlen,
                                    unsigned int access) {
	int result;
	DREF DeeObject *attr_ob;
#ifdef __ARCH_PAGESIZE
	if (DeeSystem_IsStaticPointer(attr)) {
		char const *attr_end = attr + attrlen;
		uintptr_t attr_startpage = (uintptr_t)attr & (__ARCH_PAGESIZE - 1);
		uintptr_t attr_endpage = (uintptr_t)attr_end & (__ARCH_PAGESIZE - 1);
		if (attr_startpage == attr_endpage && *attr_end == '\0') {
			return DeeRT_ErrAttributeErrorCStr_impl(&DeeError_UnknownAttribute,
			                                        decl, ob, attr, access);
		}
	}
#endif /* __ARCH_PAGESIZE */
	attr_ob = DeeString_NewSized(attr, attrlen);
	if unlikely(!attr_ob)
		goto err;
	result = DeeRT_ErrAttributeError_impl(&DeeError_UnknownAttribute, decl, ob, attr_ob, access);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ERROR_RT_ATTRIBUTE_C */
