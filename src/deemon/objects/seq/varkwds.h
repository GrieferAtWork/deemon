/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_H
#define GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_H 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/rwlock.h>

DECL_BEGIN

/* Proxy types used to drive variable keywords in user-code functions.
 * The basic idea here is to wrap either a Kwds+Argv pair, or a general
 * purpose mapping-like object (possibly with additional optimization for
 * _RoDict objects) which simply propagates all access to the underlying
 * mapping, but filters out a certain set of keys such that it appears as
 * though those keys were not part of the mapping itself. */


typedef struct {
	struct string_object  *ve_str;  /* [0..1] The keyword name that is being blacklisted.
	                                 * `NULL' is used to identify unused/sentinel entires.
	                                 * NOTE: Even when non-NULL, this field does not hold
	                                 *       a reference, as all possible strings are already
	                                 *       referenced via `:vk_code->co_keywords' */
} BlackListVarkwdsEntry;

typedef struct {
	/* Variable keywords mapping-like object for Kwds+Argv */
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	rwlock_t               vk_lock; /* Lock for this kwds wrappers. */
#endif /* !CONFIG_NO_THREADS */
	DREF struct code_object
	                      *vk_code; /* [1..1][const] The code object who's keyword arguments should
	                                 *               be blacklisted from the the resulting mapping.
	                                 * NOTE: This code doesn't always takes at least 1 argument, and
	                                 *       always specifies its keywords. When constructing an
	                                 *      `BlackListVarkwds' object with code not doing this, a
	                                 *      `DeeKwdsMappingObject' object will be returned instead,
	                                 *       which maps keywords to arguments without including a
	                                 *       blacklist of arguments which are not to be mapped.
	                                 * NOTE: If revived during unsharing, the object in this field
	                                 *       gets incref'd, meaning that before that point, this
	                                 *       field doesn't actually carry a reference. */
	size_t                 vk_ckwc; /* [const][!0] Number of black-listed keywords */
	struct string_object *const
	                      *vk_ckwv; /* [1..1][const][1..vk_ckwc][const] Vector of black-listed keywords. */
	DREF DeeKwdsObject    *vk_kwds; /* [1..1][const] The mapping for kwds-to-argument-index.
	                                 * NOTE: This kwds object always has a `kw_size' that is
	                                 *       non-ZERO. - When trying to construct a `BlackListVarkwds'
	                                 *       object from an empty keyword list, an empty mapping
	                                 *       will be returned instead.
	                                 * NOTE: If revived during unsharing, the object in this field
	                                 *       gets incref'd, meaning that before that point, this
	                                 *       field doesn't actually carry a reference. */
	DREF DeeObject       **vk_argv; /* [1..1][const][0..vk_kwds->kw_size][owned][lock(vk_lock)]
	                                 * Shared argument list to which indices from `vk_kwds'
	                                 * map. - When a keyword-enabled user-function which made
	                                 * use its variable keyword argument returns, this vector
	                                 * gets unshared (meaning that if varkwds continue to exist,
	                                 * this vector gets replaced with either a copy of itself,
	                                 * or with a NULL-pointer, if the copy fails to be allocated) */
	size_t                 vk_load; /* [lock(vk_lock,INCREMENT_ONLY)][<= vk_ckwc]
	                                 * Index of the next keyword which has yet to be loaded into
	                                 * the `vk_blck' hash-set for blacklisted identifiers. */
	size_t                 vk_mask; /* [!0][const] Hash-mask for `vk_blck' */
	BlackListVarkwdsEntry  vk_blck[1024]; /* [lock(vk_lock)][0..vk_mask+1]
	                                       * Hash-vector of loaded, black-listed keywords. */
} BlackListVarkwds;
#define BlackListVarkwds_BLCKNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)


INTDEF DeeTypeObject BlackListVarkwds_Type;
#define BlackListVarkwds_Check(x)      DeeObject_InstanceOfExact(x, &BlackListVarkwds_Type) /* `_BlackListVarkwds' is final */
#define BlackListVarkwds_CheckExact(x) DeeObject_InstanceOfExact(x, &BlackListVarkwds_Type)



typedef struct {
	OBJECT_HEAD
	ATOMIC_DATA struct kwds_entry *ki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry             *ki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
	DREF BlackListVarkwds         *ki_map;  /* [1..1][const] The associated keywords mapping. */
} BlackListVarkwdsIterator;

INTDEF DeeTypeObject BlackListVarkwdsIterator_Type;


/* Helper functions & RT optimization bindings. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_IsBlackListed(BlackListVarkwds *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_IsBlackListedString(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_IsBlackListedStringLen(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_HasItem(BlackListVarkwds *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_HasItemString(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_HasItemStringLen(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListVarkwds_GetItem(BlackListVarkwds *self, DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListVarkwds_GetItemString(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListVarkwds_GetItemStringLen(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL BlackListVarkwds_GetItemDef(BlackListVarkwds *__restrict self, DeeObject *__restrict name, DeeObject *__restrict def);
INTDEF WUNUSED DREF DeeObject *DCALL BlackListVarkwds_GetItemStringDef(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED DREF DeeObject *DCALL BlackListVarkwds_GetItemStringLenDef(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, DeeObject *__restrict def);


/* Construct a new mapping for keywords that follows the black-listing scheme.
 * NOTE: If `kwds' is empty, return `Dee_EmptyMapping' instead.
 * NOTE: If `code' doesn't specify any keywords, return `DeeKwdsMapping_New()' instead.
 * Otherwise, the caller must decref the returned object using `BlackListVarkwds_Decref()'
 * -> This function is used to filter keyword arguments from varkwds when 
 *    kwargs argument protocol is used:
 *    >> function foo(x,y?,**kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListVarkwds { "something_else" : "foobar" }'
 *    >> foo(x: 10, something_else: "foobar");
 */
INTDEF WUNUSED DREF DeeObject *DCALL
BlackListVarkwds_New(struct code_object *__restrict code,
                     size_t positional_argc,
                     DeeKwdsObject *__restrict kwds,
                     DeeObject **argv);

/* Unshare the argument vector from a blacklist-varkwds object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
INTDEF void DCALL BlackListVarkwds_Decref(DREF DeeObject *__restrict self);


/* Safely decref the contents of a non-NULL `struct code_frame_kwds::fk_varkwds' field. */
#define VARKWDS_DECREF(x)             \
	(DeeKwdsMapping_CheckExact(x)     \
	 ? DeeKwdsMapping_Decref(x)       \
	 : BlackListVarkwds_CheckExact(x) \
	   ? BlackListVarkwds_Decref(x)   \
	   : Dee_Decref(x))





typedef struct {
	/* Variable keywords mapping-like object for general-purpose mapping-like objects. */
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	rwlock_t               bm_lock; /* Lock for this kwds wrappers. */
#endif /* !CONFIG_NO_THREADS */
	DREF struct code_object
	                      *bm_code; /* [1..1][const] The code object who's keyword arguments should
	                                 *               be blacklisted from the the resulting mapping.
	                                 * NOTE: This code doesn't always takes at least 1 argument, and
	                                 *       always specifies its keywords. When constructing a
	                                 *      `BlackListVarkwds' object with code not doing this, a
	                                 *      `DeeKwdsMappingObject' object will be returned instead,
	                                 *       which maps keywords to arguments without including a
	                                 *       blacklist of arguments which are not to be mapped.
	                                 * NOTE: If revived during unsharing, the object in this field
	                                 *       gets incref'd, meaning that before that point, this
	                                 *       field doesn't actually carry a reference. */
	size_t                 bm_ckwc; /* [const][!0] Number of black-listed keywords */
	struct string_object *const
	                      *bm_ckwv; /* [1..1][const][1..vk_ckwc][const] Vector of black-listed keywords. */
	DREF DeeObject        *bm_kw;   /* [1..1][const] The underlying mapping which is being affected.
	                                 * NOTE: If revived during unsharing, the object in this field
	                                 *       gets incref'd, meaning that before that point, this
	                                 *       field doesn't actually carry a reference. */
	size_t                 bm_load; /* [lock(vk_lock,INCREMENT_ONLY)][<= bm_ckwc]
	                                 * Index of the next keyword which has yet to be loaded into
	                                 * the `vk_blck' hash-set for blacklisted identifiers. */
	size_t                 bm_mask; /* [!0][const] Hash-mask for `vk_blck' */
	BlackListVarkwdsEntry  bm_blck[1024]; /* [lock(vk_lock)][0..vk_mask+1]
	                                       * Hash-vector of loaded, black-listed keywords. */
} BlackListMapping;
#define BlackListMapping_BLCKNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)


INTDEF DeeTypeObject BlackListMapping_Type;
#define BlackListMapping_Check(x)      DeeObject_InstanceOfExact(x, &BlackListMapping_Type) /* `_BlackListMapping' is final */
#define BlackListMapping_CheckExact(x) DeeObject_InstanceOfExact(x, &BlackListMapping_Type)


INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListMapping_IsBlackListed(BlackListMapping *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListMapping_IsBlackListedString(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListMapping_IsBlackListedStringLen(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL BlackListMapping_BoundItem(BlackListMapping *__restrict self, DeeObject *__restrict name, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL BlackListMapping_BoundItemString(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL BlackListMapping_BoundItemStringLen(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListMapping_GetItem(BlackListMapping *self, DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListMapping_GetItemString(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListMapping_GetItemStringLen(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL BlackListMapping_GetItemDef(BlackListMapping *__restrict self, DeeObject *__restrict name, DeeObject *__restrict def);
INTDEF WUNUSED DREF DeeObject *DCALL BlackListMapping_GetItemStringDef(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED DREF DeeObject *DCALL BlackListMapping_GetItemStringLenDef(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, DeeObject *__restrict def);


/* Construct a new mapping for a general-purpose mapping that follows the black-listing scheme.
 * NOTE: If `code' doesn't specify any keywords, re-return `kw' unmodified.
 * -> The returned objects can be used for any kind of mapping, such that in the
 *    case of kwmappings, `BlackListMapping_New(code,DeeKwdsMapping_New(kwds,argv))'
 *    would produce the semantically equivalent of `BlackListVarkwds_New(code,kwds,argv)'
 * -> This function is used to filter keyword arguments from varkwds when the general
 *    purpose keyword argument protocol is used:
 *    >> function foo(x,y?,**kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListMapping { "something_else" : "foobar" }'
 *    >> foo(**{ "x" : 10, "something_else" : "foobar" });
 */
INTDEF WUNUSED DREF DeeObject *DCALL
BlackListMapping_New(struct code_object *__restrict code,
                     size_t positional_argc,
                     DeeObject *__restrict kw);



typedef struct {
	OBJECT_HEAD
	DREF DeeObject        *mi_iter; /* [1..1][const] An iterator for the underlying `mi_map->bm_kw'. */
	DREF BlackListMapping *mi_map;  /* [1..1][const] The general-purpose blacklist mapping being iterated. */
} BlackListMappingIterator;

INTDEF DeeTypeObject BlackListMappingIterator_Type;



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_H */
