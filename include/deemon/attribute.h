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
#ifndef GUARD_DEEMON_ATTRIBUTE_H
#define GUARD_DEEMON_ATTRIBUTE_H 1

#include "api.h"

#include "object.h"

#ifdef CONFIG_NO_LONGJMP_ENUMATTR
#undef CONFIG_LONGJMP_ENUMATTR
#else /* CONFIG_NO_LONGJMP_ENUMATTR */
#include "system-features.h"
#include <hybrid/spcall.h>

#ifndef SPCALL_NORETURN
/* We need SPCALL_NORETURN() in order to make the initial jump to a secondary stack. */
#undef CONFIG_LONGJMP_ENUMATTR
#else /* SPCALL_NORETURN */
/* Enable special handling to use setjmp() / longjmp() to yield attributes,
 * turning tp_enumattr() into a re-entrant function call.
 *
 * The specifics are a bit complicated, but essentially we setup a small
 * stack that is kept alongside the enumattr object, together with 2
 * jmp_buf-ers that we use to enter/leave the `tp_enumattr()' function.
 *
 * The alternative (and fallback) to this is to save _all_ attributes at
 * once, using a dynamically allocated vector that can then be enumerated.
 * NOTE: Even though setjmp() / longjmp() are defined by the C standard,
 *       there is no standardized way of setting the stack-pointer before
 *       calling setjmp(), meaning that this configuration option is still
 *       bound to situations for which we have pre-written assembly to set
 *       the stack-pointer ourselves.
 * NOTE: To prevent any noticeable impact from this configuration option,
 *       `enumattr.Iterator' is never copyable, even when this option is
 *       disabled (where copying it would theoretically be possible). */
#define CONFIG_LONGJMP_ENUMATTR

/* The number of attributes enumerated at once before execution will switch
 * back to user-code, yielding the new attributes until that buffer is exhausted.
 * This is done to mitigate any overhead that calls to setjmp/longjmp may cause. */
#define CONFIG_LONGJMP_ENUMATTR_CLUSTER 16
#endif /* !SPCALL_NORETURN */
#endif /* !CONFIG_NO_LONGJMP_ENUMATTR */

/* #undef CONFIG_LONGJMP_ENUMATTR */

#ifdef CONFIG_LONGJMP_ENUMATTR
#include "system-sjlj.h"
#ifndef DeeSystem_JmpBuf
#undef CONFIG_LONGJMP_ENUMATTR
#endif /* !DeeSystem_JmpBuf */

#ifdef CONFIG_LONGJMP_ENUMATTR
#include "util/lock.h"
#endif /* CONFIG_LONGJMP_ENUMATTR */
#endif /* CONFIG_LONGJMP_ENUMATTR */
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintptr_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_attribute_object         attribute_object
#define Dee_enumattr_object          enumattr_object
#define Dee_enumattr_iterator_object enumattr_iterator_object
#define Dee_attribute_info           attribute_info
#define Dee_attribute_lookup_rules   attribute_lookup_rules
#define attribute_info_fini          Dee_attribute_info_fini
#endif /* DEE_SOURCE */

typedef struct Dee_attribute_object DeeAttributeObject;
typedef struct Dee_enumattr_object DeeEnumAttrObject;
typedef struct Dee_enumattr_iterator_object DeeEnumAttrIteratorObject;

struct Dee_attribute_info {
	DREF DeeObject     *a_decl;     /* [1..1] The type defining the attribute. */
	char const         *a_doc;      /* [if(a_perm & Dee_ATTR_DOCOBJ,
	                                 *     DREF(COMPILER_CONTAINER_OF(., DeeStringObject, s_str)))]
	                                 * [0..1] The documentation string of the attribute (when known).
	                                 * NOTE: This may also be an empty string, which should be
	                                 *       interpreted as no documentation string being there at all.
	                                 * NOTE: When the `Dee_ATTR_DOCOBJ' flag is set, then this is actually
	                                 *       the `DeeString_STR()' of a string objects, to which a
	                                 *       reference is being held. */
	uint16_t            a_perm;     /* Set of `Dee_ATTR_*' flags, describing the attribute's behavior. */
	DREF DeeTypeObject *a_attrtype; /* [0..1] The typing of this attribute. */
};
#define Dee_attribute_info_docobj(self) \
	COMPILER_CONTAINER_OF((self)->a_doc, DeeStringObject, s_str)
#define Dee_attribute_info_fini(self)                             \
	(Dee_Decref((self)->a_decl), Dee_XDecref((self)->a_attrtype), \
	 ((self)->a_perm & Dee_ATTR_DOCOBJ)                           \
	 ? Dee_Decref(Dee_attribute_info_docobj(self))                \
	 : (void)0)


struct Dee_attribute_object {
	/* Wrapper object for attribute information provided to `denum_t' */
	Dee_OBJECT_HEAD
	char const               *a_name; /* [if(a_info.a_perm & Dee_ATTR_NAMEOBJ,
	                                   *     DREF(COMPILER_CONTAINER_OF(., DeeStringObject, s_str)))]
	                                   * [1..1] The name of the attribute. */
	struct Dee_attribute_info a_info; /* [const] Attribute information. */
};

struct Dee_enumattr_object {
	Dee_OBJECT_HEAD
	DREF DeeTypeObject        *ea_type;    /* [1..1][const] The starting type level from which attributes should be enumerated. */
	DREF DeeObject            *ea_obj;     /* [0..1][const] The object in association of which attributes are enumerated. */
#ifndef CONFIG_LONGJMP_ENUMATTR
	size_t                     ea_attrc;   /* [const] The amount of attributes found. */
	DREF DeeAttributeObject  **ea_attrv;   /* [1..1][0..ea_attrc][const] The amount of attributes found. */
#endif /* !CONFIG_LONGJMP_ENUMATTR */
};

struct Dee_enumattr_iterator_object {
	Dee_OBJECT_HEAD
	DREF DeeEnumAttrObject   *ei_seq;              /* [1..1] The enumattr object that is being iterated. */
#ifdef CONFIG_LONGJMP_ENUMATTR
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t         ei_lock;             /* Lock for accessing the iterator. */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeAttributeObject  *ei_buffer[CONFIG_LONGJMP_ENUMATTR_CLUSTER]; /* [?..1][*][lock(ei_lock)] Attribute cluster buffer. */
	DREF DeeAttributeObject **ei_bufpos;           /* [0..1][in(ei_buffer)][lock(ei_lock)] Pointer to the next unused (in-enum) or full (outside-enum) item.
	                                                * NOTE: Initially, this pointer is set to NULL.
	                                                * NOTE: To indicate exhaustion, this pointer to set to ITER_DONE. */
	DeeSystem_JmpBuf          ei_break;            /* [lock(ei_lock)] Jump buffer to jump into to break execution (and yield more items). */
	DeeSystem_JmpBuf          ei_continue;         /* [lock(ei_lock)] Jump buffer to jump into to continue execution (and collect more items). */
	uintptr_t                 ei_stack[512 + 256]; /* A small 3/6K-stack used when running the `tp_enumattr' operator. */
#else /* CONFIG_LONGJMP_ENUMATTR */
	DREF DeeAttributeObject **ei_iter;             /* [1..1][in(ei_seq->ea_attrv)][atomic]
	                                                * Pointer to the next item that should be yielded.
	                                                * When exhausted, this point is equal to `ei_end' */
	DREF DeeAttributeObject **ei_end;              /* [1..1][== ei_seq->ea_attrv+ei_seq->ea_attrc][const] Vector end pointer. */
#endif /* !CONFIG_LONGJMP_ENUMATTR */
};

#ifdef CONFIG_LONGJMP_ENUMATTR
#define DeeEnumAttrIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->ei_lock)
#define DeeEnumAttrIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->ei_lock)
#define DeeEnumAttrIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->ei_lock)
#define DeeEnumAttrIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->ei_lock)
#define DeeEnumAttrIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->ei_lock)
#define DeeEnumAttrIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->ei_lock)
#endif /* CONFIG_LONGJMP_ENUMATTR */

DDATDEF DeeTypeObject DeeAttribute_Type;        /* `Attribute from deemon' */
DDATDEF DeeTypeObject DeeEnumAttr_Type;         /* `enumattr from deemon' */
DDATDEF DeeTypeObject DeeEnumAttrIterator_Type; /* `(enumattr from deemon).Iterator' */
#define DeeEnumAttr_Check(x)      DeeObject_InstanceOfExact(x, &DeeEnumAttr_Type) /* `enumattr' is final */
#define DeeEnumAttr_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeEnumAttr_Type)


struct Dee_attribute_lookup_rules {
	char const *alr_name;       /* [1..1] The name of the attribute to look up. */
	Dee_hash_t  alr_hash;       /* [== Dee_HashStr(alr_name)] Hash of `alr_name' */
	DeeObject  *alr_decl;       /* [0..1] When non-NULL, only consider attributes declared by this object. */
	uint16_t    alr_perm_mask;  /* Only consider attributes who's permissions
	                             * match `(perm & perm_mask) == perm_value' */
	uint16_t    alr_perm_value; /* Permissions value for `alr_perm_mask' */
};

/* Lookup the descriptor for an attribute, given a set of lookup rules.
 * @param: rules: The result of follow for the lookup.
 * @return:  0: Successfully queried the attribute.
 *              The given `result' was filled, and the must finalize
 *              it through use of `attribute_info_fini()'.
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeObject_FindAttr(DeeTypeObject *tp_self, DeeObject *self,
                   struct Dee_attribute_info *__restrict result,
                   struct Dee_attribute_lookup_rules const *__restrict rules);

DECL_END

#endif /* !GUARD_DEEMON_ATTRIBUTE_H */
