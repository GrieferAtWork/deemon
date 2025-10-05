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
#ifdef __INTELLISENSE__
#include "mro.c"
//#define DEFINE_MRO_ATTRLEN_FUNCTIONS
#endif /* __INTELLISENSE__ */

#include <deemon/error.h>
#include <deemon/none.h>

DECL_BEGIN

#ifdef DEFINE_MRO_ATTRLEN_FUNCTIONS
#define S(without_len, with_len) with_len
#define NLen(x)                  x##Len
#define NLenHash(x)              x##LenHash
#define N_len(x)                 x##_len
#define N_len_hash(x)            x##_len_hash
#define IFELSE(if_nlen, if_len)  if_len
#define ATTR_ARG                 char const *__restrict attr, size_t attrlen
#define NAMEEQ(name)             streq_len(name, attr, attrlen)
#else /* DEFINE_MRO_ATTRLEN_FUNCTIONS */
#define S(without_len, with_len) without_len
#define NLen(x)                  x
#define N_len(x)                 x
#define NLenHash(x)              x##Hash
#define N_len_hash(x)            x##_hash
#define IFELSE(if_nlen, if_len)  if_nlen
#define ATTR_ARG                 char const *__restrict attr
#define NAMEEQ(name)             streq(name, attr)
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */


#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL DeeType_QueryAttributeHash)(DeeTypeObject *tp_invoker,
                                   DeeTypeObject *tp_self,
                                   /*String*/ DeeObject *attr,
                                   dhash_t hash) {
	struct class_attribute *result;
	result = DeeClass_QueryInstanceAttributeHash(tp_self, attr, hash);
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_cache, tp_self, hash, result);
	return result;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL S(DeeType_QueryAttributeStringHash,
         DeeType_QueryAttributeStringLenHash))(DeeTypeObject *tp_invoker,
                                               DeeTypeObject *tp_self,
                                               ATTR_ARG, dhash_t hash) {
	struct class_attribute *result;
	result = S(DeeClass_QueryInstanceAttributeStringHash(tp_self, attr, hash),
	           DeeClass_QueryInstanceAttributeStringLenHash(tp_self, attr, attrlen, hash));
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_cache, tp_self, hash, result);
	return result;
}

#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL DeeType_QueryClassAttributeHash)(DeeTypeObject *tp_invoker,
                                        DeeTypeObject *tp_self,
                                        /*String*/ DeeObject *attr,
                                        dhash_t hash) {
	struct class_attribute *result;
	result = DeeClass_QueryClassAttributeHash(tp_self, attr, hash);
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL S(DeeType_QueryClassAttributeStringHash,
         DeeType_QueryClassAttributeStringLenHash))(DeeTypeObject *tp_invoker,
                                                    DeeTypeObject *tp_self,
                                                    ATTR_ARG, dhash_t hash) {
	struct class_attribute *result;
	result = S(DeeClass_QueryClassAttributeStringHash(tp_self, attr, hash),
	           DeeClass_QueryClassAttributeStringLenHash(tp_self, attr, attrlen, hash));
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}

#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL DeeType_QueryInstanceAttributeHash)(DeeTypeObject *tp_invoker,
                                           DeeTypeObject *tp_self,
                                           /*String*/ DeeObject *attr,
                                           dhash_t hash) {
	struct class_attribute *result;
	result = DeeClass_QueryInstanceAttributeHash(tp_self, attr, hash);
	if (result)
		Dee_membercache_addinstanceattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL S(DeeType_QueryInstanceAttributeStringHash,
         DeeType_QueryInstanceAttributeStringLenHash))(DeeTypeObject *tp_invoker,
                                                       DeeTypeObject *tp_self,
                                                       ATTR_ARG, dhash_t hash) {
	struct class_attribute *result;
	result = S(DeeClass_QueryInstanceAttributeStringHash(tp_self, attr, hash),
	           DeeClass_QueryInstanceAttributeStringLenHash(tp_self, attr, attrlen, hash));
	if (result)
		Dee_membercache_addinstanceattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}



INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
N_len_hash(type_method_getattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                       struct type_method const *chain, DeeObject *self,
                                       ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		return type_method_get(chain, self);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL NLenHash(DeeType_GetInstanceMethodAttrString))(DeeTypeObject *tp_invoker,
                                                      DeeTypeObject *tp_self,
                                                      ATTR_ARG, dhash_t hash) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmeth_get(tp_self, chain);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL NLenHash(DeeType_GetIInstanceMethodAttrString))(DeeTypeObject *tp_invoker,
                                                       DeeTypeObject *tp_self,
                                                       ATTR_ARG, dhash_t hash) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obmeth_get(tp_self, chain);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
N_len_hash(type_method_callattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                        struct type_method const *chain, DeeObject *self,
                                        ATTR_ARG, dhash_t hash,
                                        size_t argc, DeeObject *const *argv) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		return type_method_call(chain, self, argc, argv);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL NLenHash(DeeType_CallInstanceMethodAttrString))(DeeTypeObject *tp_invoker,
                                                       DeeTypeObject *tp_self,
                                                       ATTR_ARG, dhash_t hash,
                                                       size_t argc, DeeObject *const *argv) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmeth_call(tp_self, chain, argc, argv);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
S(type_method_callattr_string_hash_kw,
  type_method_callattr_string_len_hash_kw)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                           struct type_method const *chain, DeeObject *self,
                                           ATTR_ARG, dhash_t hash,
                                           size_t argc, DeeObject *const *argv,
                                           DeeObject *kw) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		return type_method_call_kw(chain, self, argc, argv, kw);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallInstanceMethodAttrStringHashKw,
  DeeType_CallInstanceMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker,
                                                 DeeTypeObject *tp_self,
                                                 ATTR_ARG, dhash_t hash,
                                                 size_t argc, DeeObject *const *argv,
                                                 DeeObject *kw) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmeth_call_kw(tp_self, chain, argc, argv, kw);
	}
	return ITER_DONE;
}

#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) DREF DeeObject *
(DCALL S(DeeType_VCallInstanceMethodAttrStringHashf,
         DeeType_VCallInstanceMethodAttrStringLenHashf))(DeeTypeObject *tp_invoker,
                                                         DeeTypeObject *tp_self,
                                                         ATTR_ARG, dhash_t hash,
                                                         char const *__restrict format,
                                                         va_list args) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmeth_vcallf(tp_self, chain, format, args);
	}
	return ITER_DONE;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */


INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallIInstanceMethodAttrStringHashKw,
  DeeType_CallIInstanceMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker,
                                                  DeeTypeObject *tp_self,
                                                  ATTR_ARG, dhash_t hash,
                                                  size_t argc, DeeObject *const *argv,
                                                  DeeObject *kw) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obmeth_call_kw(tp_self, chain, argc, argv, kw);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
S(type_instance_method_callattr_string_hash_kw,
  type_instance_method_callattr_string_len_hash_kw)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                                    struct type_method const *chain, ATTR_ARG, dhash_t hash,
                                                    size_t argc, DeeObject *const *argv,
                                                    DeeObject *kw) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(cache, decl, hash, chain);
		return type_obmeth_call_kw(decl, chain, argc, argv, kw);
	}
	return ITER_DONE;
}

#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5, IFELSE(7, 8))) DREF DeeObject *DCALL
type_method_vcallattr_string_hashf(struct Dee_membercache *cache, DeeTypeObject *decl,
                                   struct type_method const *chain, DeeObject *self,
                                   ATTR_ARG, dhash_t hash,
                                   char const *__restrict format, va_list args) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		return type_method_vcallf(chain, self, format, args);
	}
	return ITER_DONE;
}

#if 0
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) DREF DeeObject *
(DCALL DeeType_VCallInstanceMethodAttrStringHashf)(DeeTypeObject *tp_invoker,
                                                   DeeTypeObject *tp_self,
                                                   ATTR_ARG, dhash_t hash,
                                                   char const *__restrict format, va_list args) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmeth_vcallf(tp_self, chain, format, args);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) DREF DeeObject *
(DCALL DeeType_VCallIInstanceMethodAttrStringHashf)(DeeTypeObject *tp_invoker,
                                                    DeeTypeObject *tp_self,
                                                    ATTR_ARG, dhash_t hash,
                                                    char const *__restrict format, va_list args) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obmeth_vcallf(tp_self, chain, format, args);
	}
	return ITER_DONE;
}
#endif
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */


INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_GETSET */
N_len_hash(type_getset_getattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                       struct type_getset const *chain, DeeObject *self,
                                       ATTR_ARG, dhash_t hash) {
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(cache, decl, hash, chain);
		return type_getset_get(chain, self);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_GetInstanceGetSetAttrString)(DeeTypeObject *tp_invoker,
                                              DeeTypeObject *tp_self,
                                              ATTR_ARG, dhash_t hash) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obprop_get(tp_self, chain);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_GetIInstanceGetSetAttrString)(DeeTypeObject *tp_invoker,
                                               DeeTypeObject *tp_self,
                                               ATTR_ARG, dhash_t hash) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obprop_get(tp_self, chain);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_CallInstanceGetSetAttrString)(DeeTypeObject *tp_invoker,
                                               DeeTypeObject *tp_self,
                                               ATTR_ARG, dhash_t hash,
                                               size_t argc, DeeObject *const *argv) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obprop_call(tp_self, chain, argc, argv);
	}
	return ITER_DONE;
}

#if 0
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_CallIInstanceGetSetAttrString)(DeeTypeObject *tp_invoker,
                                                DeeTypeObject *tp_self,
                                                ATTR_ARG, dhash_t hash,
                                                size_t argc, DeeObject *const *argv) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obprop_call(tp_self, chain, argc, argv);
	}
	return ITER_DONE;
}
#endif

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallInstanceGetSetAttrStringHashKw,
  DeeType_CallInstanceGetSetAttrStringLenHashKw)(DeeTypeObject *tp_invoker,
                                                 DeeTypeObject *tp_self,
                                                 ATTR_ARG, dhash_t hash,
                                                 size_t argc, DeeObject *const *argv,
                                                 DeeObject *kw) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obprop_call_kw(tp_self, chain, argc, argv, kw);
	}
	return ITER_DONE;
}


#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) DREF DeeObject *
(DCALL S(DeeType_VCallInstanceGetSetAttrStringHashf,
         DeeType_VCallInstanceGetSetAttrStringLenHashf))(DeeTypeObject *tp_invoker,
                                                         DeeTypeObject *tp_self,
                                                         ATTR_ARG, dhash_t hash,
                                                         char const *__restrict format,
                                                         va_list args) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obprop_vcallf(tp_self, chain, format, args);
	}
	return ITER_DONE;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallIInstanceGetSetAttrStringHashKw,
  DeeType_CallIInstanceGetSetAttrStringLenHashKw)(DeeTypeObject *tp_invoker,
                                                  DeeTypeObject *tp_self,
                                                  ATTR_ARG, dhash_t hash,
                                                  size_t argc, DeeObject *const *argv,
                                                  DeeObject *kw) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obprop_call_kw(tp_self, chain, argc, argv, kw);
	}
	return ITER_DONE;
}



INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_GETSET */
N_len_hash(type_getset_boundattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                         struct type_getset const *chain, DeeObject *self,
                                         ATTR_ARG, dhash_t hash) {
	DREF DeeObject *temp;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(cache, decl, hash, chain);
		if (chain->gs_bound)
			return (*chain->gs_bound)(self);
		if unlikely(!chain->gs_get)
			return Dee_BOUND_NO;
		temp = (*chain->gs_get)(self);
		if likely(temp) {
			Dee_Decref(temp);
			return Dee_BOUND_YES;
		}
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return Dee_BOUND_NO;
		return Dee_BOUND_ERR;
	}
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_GETSET */
N_len_hash(type_getset_delattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                       struct type_getset const *chain, DeeObject *self,
                                       ATTR_ARG, dhash_t hash) {
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(cache, decl, hash, chain);
		return type_getset_del(chain, self);
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5, IFELSE(7, 8))) int DCALL /* SET_GETSET */
N_len_hash(type_getset_setattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                       struct type_getset const *chain, DeeObject *self,
                                       ATTR_ARG, dhash_t hash, DeeObject *value) {
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(cache, decl, hash, chain);
		return type_getset_set(chain, self, value);
	}
	return 1;
}


INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_MEMBER */
N_len_hash(type_member_getattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                       struct type_member const *chain, DeeObject *self,
                                       ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		return type_member_get(chain, self);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_GetInstanceMemberAttrString)(DeeTypeObject *tp_invoker,
                                              DeeTypeObject *tp_self,
                                              ATTR_ARG, dhash_t hash) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmemb_get(tp_self, chain);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_GetIInstanceMemberAttrString)(DeeTypeObject *tp_invoker,
                                               DeeTypeObject *tp_self,
                                               ATTR_ARG, dhash_t hash) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obmemb_get(tp_self, chain);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_CallInstanceMemberAttrString)(DeeTypeObject *tp_invoker,
                                               DeeTypeObject *tp_self,
                                               ATTR_ARG, dhash_t hash,
                                               size_t argc, DeeObject *const *argv) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmemb_call(tp_self, chain, argc, argv);
	}
	return ITER_DONE;
}

#if 0
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
NLenHash(DeeType_CallIInstanceMemberAttrString)(DeeTypeObject *tp_invoker,
                                                DeeTypeObject *tp_self,
                                                ATTR_ARG, dhash_t hash,
                                                size_t argc, DeeObject *const *argv) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obmemb_call(tp_self, chain, argc, argv);
	}
	return ITER_DONE;
}
#endif

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallInstanceMemberAttrStringHashKw,
  DeeType_CallInstanceMemberAttrStringLenHashKw)(DeeTypeObject *tp_invoker,
                                                 DeeTypeObject *tp_self,
                                                 ATTR_ARG, dhash_t hash,
                                                 size_t argc, DeeObject *const *argv,
                                                 DeeObject *kw) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return type_obmemb_call_kw(tp_self, chain, argc, argv, kw);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallIInstanceMemberAttrStringHashKw,
  DeeType_CallIInstanceMemberAttrStringLenHashKw)(DeeTypeObject *tp_invoker,
                                                  DeeTypeObject *tp_self,
                                                  ATTR_ARG, dhash_t hash,
                                                  size_t argc, DeeObject *const *argv,
                                                  DeeObject *kw) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obmemb_call_kw(tp_self, chain, argc, argv, kw);
	}
	return ITER_DONE;
}

#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) DREF DeeObject *
(DCALL S(DeeType_VCallInstanceMemberAttrStringHashf,
         DeeType_VCallInstanceMemberAttrStringLenHashf))(DeeTypeObject *tp_invoker,
                                                         DeeTypeObject *tp_self,
                                                         ATTR_ARG, dhash_t hash,
                                                         char const *__restrict format,
                                                         va_list args) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(&tp_invoker->tp_cache, tp_self, hash, chain);
		return type_obmemb_vcallf(tp_self, chain, format, args);
	}
	return ITER_DONE;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */



INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_MEMBER */
N_len_hash(type_member_boundattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                         struct type_member const *chain, DeeObject *self,
                                         ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		bool result;
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		result = type_member_bound(chain, self);
		return Dee_BOUND_FROMBOOL(result);
	}
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_MEMBER */
N_len_hash(type_member_delattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                       struct type_member const *chain, DeeObject *self,
                                       ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		return type_member_del(chain, self);
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* SET_MEMBER */
N_len_hash(type_member_setattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                       struct type_member const *chain, DeeObject *self,
                                       ATTR_ARG, dhash_t hash, DeeObject *value) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		return type_member_set(chain, self, value);
	}
	return 1;
}


INTERN WUNUSED NONNULL((1, 2, 3, 4)) struct type_method const *DCALL /* METHOD */
N_len_hash(type_method_locateattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                          struct type_method const *chain, ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		return chain;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) struct type_method const *DCALL
NLenHash(DeeType_LocateInstanceMethodAttrString)(DeeTypeObject *tp_invoker,
                                                 DeeTypeObject *tp_self,
                                                 ATTR_ARG, dhash_t hash) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return chain;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) struct type_getset const *DCALL /* GETSET */
N_len_hash(type_getset_locateattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                          struct type_getset const *chain, ATTR_ARG, dhash_t hash) {
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(cache, decl, hash, chain);
		return chain;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) struct type_getset const *DCALL
NLenHash(DeeType_LocateInstanceGetSetAttrString)(DeeTypeObject *tp_invoker,
                                                 DeeTypeObject *tp_self,
                                                 ATTR_ARG, dhash_t hash) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return chain;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) struct type_member const *DCALL /* MEMBER */
N_len_hash(type_member_locateattr_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                          struct type_member const *chain, ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		return chain;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) struct type_member const *DCALL
NLenHash(DeeType_LocateInstanceMemberAttrString)(DeeTypeObject *tp_invoker,
                                                 DeeTypeObject *tp_self,
                                                 ATTR_ARG, dhash_t hash) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return chain;
	}
	return NULL;
}



#ifdef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3, 5, IFELSE(7, 8))) bool DCALL /* METHOD */
N_len_hash(type_method_findattrinfo_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                            struct type_method const *chain, uintptr_t type,
                                            ATTR_ARG, dhash_t hash, struct Dee_attrinfo *__restrict retinfo) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		retinfo->ai_type = type;
		retinfo->ai_decl = (DeeObject *)decl;
		retinfo->ai_value.v_method = chain;
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5, IFELSE(7, 8))) bool DCALL /* GETSET */
N_len_hash(type_getset_findattrinfo_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                            struct type_getset const *chain, uintptr_t type,
                                            ATTR_ARG, dhash_t hash, struct Dee_attrinfo *__restrict retinfo) {
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(cache, decl, hash, chain);
		retinfo->ai_type = type;
		retinfo->ai_decl = (DeeObject *)decl;
		retinfo->ai_value.v_getset = chain;
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5, IFELSE(7, 8))) bool DCALL /* MEMBER */
N_len_hash(type_member_findattrinfo_string)(struct Dee_membercache *cache, DeeTypeObject *decl,
                                            struct type_member const *chain, uintptr_t type,
                                            ATTR_ARG, dhash_t hash, struct Dee_attrinfo *__restrict retinfo) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		retinfo->ai_type = type;
		retinfo->ai_decl = (DeeObject *)decl;
		retinfo->ai_value.v_member = chain;
		return true;
	}
	return false;
}

#undef DeeType_FindInstanceMethodAttrInfoStringHash
#undef DeeType_FindInstanceMethodAttrInfoStringLenHash
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) bool DCALL
NLenHash(DeeType_FindInstanceMethodAttrInfoString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self,
                                                   ATTR_ARG, dhash_t hash, struct Dee_attrinfo *__restrict retinfo) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		retinfo->ai_type = Dee_ATTRINFO_INSTANCE_METHOD;
		retinfo->ai_decl = (DeeObject *)tp_self;
		retinfo->ai_value.v_instance_method = chain;
		return true;
	}
	return false;
}

#undef DeeType_FindInstanceGetSetAttrInfoStringHash
#undef DeeType_FindInstanceGetSetAttrInfoStringLenHash
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) bool DCALL
NLenHash(DeeType_FindInstanceGetSetAttrInfoString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self,
                                                   ATTR_ARG, dhash_t hash, struct Dee_attrinfo *__restrict retinfo) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		retinfo->ai_type = Dee_ATTRINFO_INSTANCE_GETSET;
		retinfo->ai_decl = (DeeObject *)tp_self;
		retinfo->ai_value.v_instance_getset = chain;
		return true;
	}
	return false;
}

#undef DeeType_FindInstanceMemberAttrInfoStringHash
#undef DeeType_FindInstanceMemberAttrInfoStringLenHash
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) bool DCALL
NLenHash(DeeType_FindInstanceMemberAttrInfoString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self,
                                                   ATTR_ARG, dhash_t hash, struct Dee_attrinfo *__restrict retinfo) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		retinfo->ai_type = Dee_ATTRINFO_INSTANCE_MEMBER;
		retinfo->ai_decl = (DeeObject *)tp_self;
		retinfo->ai_value.v_instance_member = chain;
		return true;
	}
	return false;
}
#endif /* DEFINE_MRO_ATTRLEN_FUNCTIONS */


DECL_END

#undef S
#undef N_len
#undef N_len_hash
#undef NLen
#undef NLenHash
#undef IFELSE
#undef ATTR_ARG
#undef NAMEEQ
#undef DEFINE_MRO_ATTRLEN_FUNCTIONS
