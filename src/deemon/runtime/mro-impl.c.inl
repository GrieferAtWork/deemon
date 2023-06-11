/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "mro.c"
#define DEFINE_MRO_ATTRLEN_FUNCTIONS
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#ifdef DEFINE_MRO_ATTRLEN_FUNCTIONS
#define S(without_len, with_len) with_len
#define NLen(x)                  x##Len
#define N_len(x)                 x##_len
#define IFELSE(if_nlen, if_len)  if_len
#define ATTR_ARG                 char const *__restrict attr, size_t attrlen
#define NAMEEQ(name)             streq_len(name, attr, attrlen)
#else /* DEFINE_MRO_ATTRLEN_FUNCTIONS */
#define S(without_len, with_len) without_len
#define NLen(x)                  x
#define N_len(x)                 x
#define IFELSE(if_nlen, if_len)  if_nlen
#define ATTR_ARG                 char const *__restrict attr
#define NAMEEQ(name)             streq(name, attr)
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */


#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL DeeType_QueryAttributeWithHash)(DeeTypeObject *tp_invoker,
                                       DeeTypeObject *tp_self,
                                       /*String*/ DeeObject *attr,
                                       dhash_t hash) {
	struct class_attribute *result;
	result = DeeClass_QueryInstanceAttributeWithHash(tp_self, attr, hash);
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_cache, tp_self, hash, result);
	return result;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL S(DeeType_QueryAttributeStringWithHash,
         DeeType_QueryAttributeStringLenWithHash))(DeeTypeObject *tp_invoker,
                                                   DeeTypeObject *tp_self,
                                                   ATTR_ARG, dhash_t hash) {
	struct class_attribute *result;
	result = S(DeeClass_QueryInstanceAttributeStringWithHash(tp_self, attr, hash),
	           DeeClass_QueryInstanceAttributeStringLenWithHash(tp_self, attr, attrlen, hash));
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_cache, tp_self, hash, result);
	return result;
}

#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL DeeType_QueryClassAttributeWithHash)(DeeTypeObject *tp_invoker,
                                            DeeTypeObject *tp_self,
                                            /*String*/ DeeObject *attr,
                                            dhash_t hash) {
	struct class_attribute *result;
	result = DeeClass_QueryClassAttributeWithHash(tp_self, attr, hash);
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL S(DeeType_QueryClassAttributeStringWithHash,
         DeeType_QueryClassAttributeStringLenWithHash))(DeeTypeObject *tp_invoker,
                                                        DeeTypeObject *tp_self,
                                                        ATTR_ARG, dhash_t hash) {
	struct class_attribute *result;
	result = S(DeeClass_QueryClassAttributeStringWithHash(tp_self, attr, hash),
	           DeeClass_QueryClassAttributeStringLenWithHash(tp_self, attr, attrlen, hash));
	if (result)
		Dee_membercache_addattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}

#ifndef DEFINE_MRO_ATTRLEN_FUNCTIONS
INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL DeeType_QueryInstanceAttributeWithHash)(DeeTypeObject *tp_invoker,
                                               DeeTypeObject *tp_self,
                                               /*String*/ DeeObject *attr,
                                               dhash_t hash) {
	struct class_attribute *result;
	result = DeeClass_QueryInstanceAttributeWithHash(tp_self, attr, hash);
	if (result)
		Dee_membercache_addinstanceattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) struct class_attribute *
(DCALL S(DeeType_QueryInstanceAttributeStringWithHash,
         DeeType_QueryInstanceAttributeStringLenWithHash))(DeeTypeObject *tp_invoker,
                                                           DeeTypeObject *tp_self,
                                                           ATTR_ARG, dhash_t hash) {
	struct class_attribute *result;
	result = S(DeeClass_QueryInstanceAttributeStringWithHash(tp_self, attr, hash),
	           DeeClass_QueryInstanceAttributeStringLenWithHash(tp_self, attr, attrlen, hash));
	if (result)
		Dee_membercache_addinstanceattrib(&tp_invoker->tp_class_cache, tp_self, hash, result);
	return result;
}



INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
N_len(type_method_getattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
(DCALL NLen(DeeType_GetInstanceMethodAttr))(DeeTypeObject *tp_invoker,
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
(DCALL NLen(DeeType_GetIInstanceMethodAttr))(DeeTypeObject *tp_invoker,
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
N_len(type_method_callattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL NLen(DeeType_CallInstanceMethodAttr))(DeeTypeObject *tp_invoker,
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
S(type_method_callattr_kw,
  type_method_callattr_len_kw)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
S(DeeType_CallInstanceMethodAttrKw,
  DeeType_CallInstanceMethodAttrLenKw)(DeeTypeObject *tp_invoker,
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

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallIInstanceMethodAttrKw,
  DeeType_CallIInstanceMethodAttrLenKw)(DeeTypeObject *tp_invoker,
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
S(type_instance_method_callattr_kw,
  type_instance_method_callattr_len_kw)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
type_method_vcallattrf(struct Dee_membercache *cache, DeeTypeObject *decl,
                       struct type_method const *chain, DeeObject *self,
                       ATTR_ARG, dhash_t hash,
                       char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		args_tuple = DeeTuple_VNewf(format, args);
		if unlikely(!args_tuple)
			goto err;
		result = type_method_call(chain, self,
		                          DeeTuple_SIZE(args_tuple),
		                          DeeTuple_ELEM(args_tuple));
		Dee_Decref(args_tuple);
		return result;
	}
	return ITER_DONE;
err:
	return NULL;
}

#if 0
INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) DREF DeeObject *
(DCALL DeeType_VCallInstanceMethodAttrf)(DeeTypeObject *tp_invoker,
                                         DeeTypeObject *tp_self,
                                         ATTR_ARG, dhash_t hash,
                                         char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		args_tuple = DeeTuple_VNewf(format, args);
		if unlikely(!args_tuple)
			goto err;
		result = type_obmeth_call(tp_self, chain,
		                          DeeTuple_SIZE(args_tuple),
		                          DeeTuple_ELEM(args_tuple));
		Dee_Decref(args_tuple);
		return result;
	}
	return ITER_DONE;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, IFELSE(5, 6))) DREF DeeObject *
(DCALL DeeType_VCallIInstanceMethodAttrf)(DeeTypeObject *tp_invoker,
                                          DeeTypeObject *tp_self,
                                          ATTR_ARG, dhash_t hash,
                                          char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(&tp_invoker->tp_cache, tp_self, hash, chain);
		args_tuple = DeeTuple_VNewf(format, args);
		if unlikely(!args_tuple)
			goto err;
		result = type_obmeth_call(tp_self, chain,
		                          DeeTuple_SIZE(args_tuple),
		                          DeeTuple_ELEM(args_tuple));
		Dee_Decref(args_tuple);
		return result;
	}
	return ITER_DONE;
err:
	return NULL;
}
#endif
#endif /* !DEFINE_MRO_ATTRLEN_FUNCTIONS */


INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_GETSET */
N_len(type_getset_getattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
NLen(DeeType_GetInstanceGetSetAttr)(DeeTypeObject *tp_invoker,
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
NLen(DeeType_GetIInstanceGetSetAttr)(DeeTypeObject *tp_invoker,
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
NLen(DeeType_CallInstanceGetSetAttr)(DeeTypeObject *tp_invoker,
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
NLen(DeeType_CallIInstanceGetSetAttr)(DeeTypeObject *tp_invoker,
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
S(DeeType_CallInstanceGetSetAttrKw,
  DeeType_CallInstanceGetSetAttrLenKw)(DeeTypeObject *tp_invoker,
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

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
S(DeeType_CallIInstanceGetSetAttrKw,
  DeeType_CallIInstanceGetSetAttrLenKw)(DeeTypeObject *tp_invoker,
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
N_len(type_getset_boundattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
			return 0;
		temp = (*chain->gs_get)(self);
		if likely(temp) {
			Dee_Decref(temp);
			return 1;
		}
		if (CATCH_ATTRIBUTE_ERROR())
			return -3;
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return 0;
		return -1;
	}
	return -2;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_GETSET */
N_len(type_getset_delattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
N_len(type_getset_setattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
N_len(type_member_getattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
NLen(DeeType_GetInstanceMemberAttr)(DeeTypeObject *tp_invoker,
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
NLen(DeeType_GetIInstanceMemberAttr)(DeeTypeObject *tp_invoker,
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
NLen(DeeType_CallInstanceMemberAttr)(DeeTypeObject *tp_invoker,
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
NLen(DeeType_CallIInstanceMemberAttr)(DeeTypeObject *tp_invoker,
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
S(DeeType_CallInstanceMemberAttrKw,
  DeeType_CallInstanceMemberAttrLenKw)(DeeTypeObject *tp_invoker,
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
S(DeeType_CallIInstanceMemberAttrKw,
  DeeType_CallIInstanceMemberAttrLenKw)(DeeTypeObject *tp_invoker,
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




INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_MEMBER */
N_len(type_member_boundattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
                             struct type_member const *chain, DeeObject *self,
                             ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		return type_member_bound(chain, self);
	}
	return -2;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_MEMBER */
N_len(type_member_delattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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
N_len(type_member_setattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
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


INTERN WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* METHOD */
N_len(type_method_hasattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
                           struct type_method const *chain, ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmethod(cache, decl, hash, chain);
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3)) bool DCALL
NLen(DeeType_HasInstanceMethodAttr)(DeeTypeObject *tp_invoker,
                                    DeeTypeObject *tp_self,
                                    ATTR_ARG, dhash_t hash) {
	struct type_method const *chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* GETSET */
N_len(type_getset_hasattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
                           struct type_getset const *chain, ATTR_ARG, dhash_t hash) {
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addgetset(cache, decl, hash, chain);
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3)) bool DCALL
NLen(DeeType_HasInstanceGetSetAttr)(DeeTypeObject *tp_invoker,
                                    DeeTypeObject *tp_self,
                                    ATTR_ARG, dhash_t hash) {
	struct type_getset const *chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		if (!NAMEEQ(chain->gs_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* MEMBER */
N_len(type_member_hasattr)(struct Dee_membercache *cache, DeeTypeObject *decl,
                           struct type_member const *chain, ATTR_ARG, dhash_t hash) {
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addmember(cache, decl, hash, chain);
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2, 3)) bool DCALL
NLen(DeeType_HasInstanceMemberAttr)(DeeTypeObject *tp_invoker,
                                    DeeTypeObject *tp_self,
                                    ATTR_ARG, dhash_t hash) {
	struct type_member const *chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		if (!NAMEEQ(chain->m_name))
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache, tp_self, hash, chain);
		return true;
	}
	return false;
}

DECL_END

#undef S
#undef N_len
#undef NLen
#undef IFELSE
#undef ATTR_ARG
#undef NAMEEQ
#undef DEFINE_MRO_ATTRLEN_FUNCTIONS
