/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
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
#include <deemon/util/lock.h>

DECL_BEGIN

/* Proxy types used to drive variable keywords in user-code functions.
 * The basic idea here is to wrap either a Kwds+Argv pair, or a general
 * purpose mapping-like object (possibly with additional optimization for
 * _RoDict objects) which simply propagates all access to the underlying
 * mapping, but filters out a certain set of keys such that it appears as
 * though those keys were not part of the mapping itself. */


struct string_object;
typedef struct {
	struct string_object *blve_str; /* [0..1] The keyword name that is being blacklisted.
	                                 * `NULL' is used to identify unused/sentinel entries.
	                                 * NOTE: Even when non-NULL, this field does not hold
	                                 *       a reference, as all possible strings are already
	                                 *       referenced via `:blvk_code->co_keywords' */
} BlackListVarkwdsEntry;

typedef struct {
	/* Variable keywords mapping-like object for Kwds+Argv */
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                            blvk_lock;  /* Lock for this kwds wrappers. */
#endif /* !CONFIG_NO_THREADS */
	DREF struct code_object                       *blvk_code;  /* [1..1][const] The code object who's keyword arguments should
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
	size_t                                         blvk_ckwc;  /* [const][!0] Number of black-listed keywords */
	struct string_object *const                   *blvk_ckwv;  /* [1..1][const][1..blvk_ckwc][const] Vector of black-listed keywords. */
	DREF DeeKwdsObject                            *blvk_kwds;  /* [1..1][const] The mapping for kwds-to-argument-index.
	                                                            * NOTE: This kwds object always has a `kw_size' that is
	                                                            *       non-ZERO. - When trying to construct a `BlackListVarkwds'
	                                                            *       object from an empty keyword list, an empty mapping
	                                                            *       will be returned instead.
	                                                            * NOTE: If revived during unsharing, the object in this field
	                                                            *       gets incref'd, meaning that before that point, this
	                                                            *       field doesn't actually carry a reference. */
	DREF DeeObject                               **blvk_argv;  /* [1..1][const][0..blvk_kwds->kw_size][owned][lock(blvk_lock)]
	                                                            * Shared argument list to which indices from `blvk_kwds'
	                                                            * map. - When a keyword-enabled user-function which made
	                                                            * use its variable keyword argument returns, this vector
	                                                            * gets unshared (meaning that if varkwds continue to exist,
	                                                            * this vector gets replaced with either a copy of itself,
	                                                            * or with a NULL-pointer, if the copy fails to be allocated) */
	size_t                                         blvk_load;  /* [lock(blvk_lock, INCREMENT_ONLY)][<= blvk_ckwc]
	                                                            * Index of the next keyword which has yet to be loaded into
	                                                            * the `blvk_blck' hash-set for blacklisted identifiers. */
	size_t                                         blvk_mask;  /* [!0][const] Hash-mask for `blvk_blck' */
	COMPILER_FLEXIBLE_ARRAY(BlackListVarkwdsEntry, blvk_blck); /* [lock(blvk_lock)][0..blvk_mask+1]
	                                                            * Hash-vector of loaded, black-listed keywords. */
} BlackListVarkwds;
#define BlackListVarkwds_BLCKNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)

#define BlackListVarkwds_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->blvk_lock)
#define BlackListVarkwds_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->blvk_lock)
#define BlackListVarkwds_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->blvk_lock)
#define BlackListVarkwds_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->blvk_lock)
#define BlackListVarkwds_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->blvk_lock)
#define BlackListVarkwds_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->blvk_lock)
#define BlackListVarkwds_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->blvk_lock)
#define BlackListVarkwds_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->blvk_lock)
#define BlackListVarkwds_LockRead(self)       Dee_atomic_rwlock_read(&(self)->blvk_lock)
#define BlackListVarkwds_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->blvk_lock)
#define BlackListVarkwds_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->blvk_lock)
#define BlackListVarkwds_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->blvk_lock)
#define BlackListVarkwds_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->blvk_lock)
#define BlackListVarkwds_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->blvk_lock)
#define BlackListVarkwds_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->blvk_lock)
#define BlackListVarkwds_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->blvk_lock)


INTDEF DeeTypeObject BlackListVarkwds_Type;
#define BlackListVarkwds_Check(x)      DeeObject_InstanceOfExact(x, &BlackListVarkwds_Type) /* `_BlackListVarkwds' is final */
#define BlackListVarkwds_CheckExact(x) DeeObject_InstanceOfExact(x, &BlackListVarkwds_Type)



typedef struct {
	OBJECT_HEAD
	DWEAK struct kwds_entry *blki_iter; /* [1..1] The next entry to iterate. */
	struct kwds_entry       *blki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
	DREF BlackListVarkwds   *blki_map;  /* [1..1][const] The associated keywords mapping. */
} BlackListVarkwdsIterator;

INTDEF DeeTypeObject BlackListVarkwdsIterator_Type;


/* Helper functions & RT optimization bindings. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_IsBlackListed(BlackListVarkwds *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_IsBlackListedStringHash(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_IsBlackListedStringLenHash(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_HasItem(BlackListVarkwds *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_HasItemStringHash(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListVarkwds_HasItemStringLenHash(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListVarkwds_GetItem(BlackListVarkwds *self, DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListVarkwds_GetItemStringHash(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListVarkwds_GetItemStringLenHash(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL BlackListVarkwds_GetItemDef(BlackListVarkwds *__restrict self, DeeObject *__restrict name, DeeObject *__restrict def);
INTDEF WUNUSED DREF DeeObject *DCALL BlackListVarkwds_GetItemStringHashDef(BlackListVarkwds *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED DREF DeeObject *DCALL BlackListVarkwds_GetItemStringLenHashDef(BlackListVarkwds *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, DeeObject *__restrict def);
#define BlackListVarkwds_IsBlackListedString(self, name)                BlackListVarkwds_IsBlackListedStringHash(self, name, Dee_HashStr(name))
#define BlackListVarkwds_IsBlackListedStringLen(self, name, namesize)   BlackListVarkwds_IsBlackListedStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define BlackListVarkwds_HasItemString(self, name)                      BlackListVarkwds_HasItemStringHash(self, name, Dee_HashStr(name))
#define BlackListVarkwds_HasItemStringLen(self, name, namesize)         BlackListVarkwds_HasItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define BlackListVarkwds_GetItemString(self, name)                      BlackListVarkwds_GetItemStringHash(self, name, Dee_HashStr(name))
#define BlackListVarkwds_GetItemStringLen(self, name, namesize)         BlackListVarkwds_GetItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define BlackListVarkwds_GetItemStringDef(self, name, def)              BlackListVarkwds_GetItemStringHashDef(self, name, Dee_HashStr(name), def)
#define BlackListVarkwds_GetItemStringLenDef(self, name, namesize, def) BlackListVarkwds_GetItemStringLenHashDef(self, name, namesize, Dee_HashPtr(name, namesize), def)


/* Construct a new mapping for keywords that follows the black-listing scheme.
 * NOTE: If `kwds' is empty, return `Dee_EmptyMapping' instead.
 * NOTE: If `code' doesn't specify any keywords, return `DeeKwdsMapping_New()' instead.
 * Otherwise, the caller must decref the returned object using `BlackListVarkwds_Decref()'
 * -> This function is used to filter keyword arguments from varkwds when 
 *    kwargs argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListVarkwds { "something_else" : "foobar" }'
 *    >> foo(x: 10, something_else: "foobar");
 */
INTDEF WUNUSED DREF DeeObject *DCALL
BlackListVarkwds_New(struct code_object *__restrict code,
                     size_t positional_argc,
                     DeeKwdsObject *__restrict kwds,
                     DeeObject *const *argv);

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
	Dee_atomic_rwlock_t                            blm_lock;  /* Lock for this kwds wrappers. */
#endif /* !CONFIG_NO_THREADS */
	DREF struct code_object                       *blm_code;  /* [1..1][const] The code object who's keyword arguments should
	                                                           *               be blacklisted from the the resulting mapping.
	                                                           * NOTE: This code doesn't always takes at least 1 argument, and
	                                                           *       always specifies its keywords. When constructing a
	                                                           *       `BlackListVarkwds' object with code not doing this, a
	                                                           *       `DeeKwdsMappingObject' object will be returned instead,
	                                                           *       which maps keywords to arguments without including a
	                                                           *       blacklist of arguments which are not to be mapped.
	                                                           * NOTE: If revived during unsharing, the object in this field
	                                                           *       gets incref'd, meaning that before that point, this
	                                                           *       field doesn't actually carry a reference. */
	size_t                                         blm_ckwc;  /* [const][!0] Number of black-listed keywords */
	struct string_object *const                   *blm_ckwv;  /* [1..1][const][1..blm_ckwc][const] Vector of black-listed keywords. */
	DREF DeeObject                                *blm_kw;    /* [1..1][const] The underlying mapping which is being affected. */
	size_t                                         blm_load;  /* [lock(blm_lock, INCREMENT_ONLY)][<= blm_ckwc]
	                                                           * Index of the next keyword which has yet to be loaded into
	                                                           * the `blm_blck' hash-set for blacklisted identifiers. */
	size_t                                         blm_mask;  /* [!0][const] Hash-mask for `blm_blck' */
	COMPILER_FLEXIBLE_ARRAY(BlackListVarkwdsEntry, blm_blck); /* [lock(blm_lock)][0..blm_mask+1]
	                                                           * Hash-vector of loaded, black-listed keywords. */
} BlackListMapping;
#define BlackListMapping_BLCKNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)

#define BlackListMapping_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->blm_lock)
#define BlackListMapping_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->blm_lock)
#define BlackListMapping_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->blm_lock)
#define BlackListMapping_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->blm_lock)
#define BlackListMapping_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->blm_lock)
#define BlackListMapping_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->blm_lock)
#define BlackListMapping_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->blm_lock)
#define BlackListMapping_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->blm_lock)
#define BlackListMapping_LockRead(self)       Dee_atomic_rwlock_read(&(self)->blm_lock)
#define BlackListMapping_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->blm_lock)
#define BlackListMapping_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->blm_lock)
#define BlackListMapping_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->blm_lock)
#define BlackListMapping_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->blm_lock)
#define BlackListMapping_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->blm_lock)
#define BlackListMapping_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->blm_lock)
#define BlackListMapping_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->blm_lock)


INTDEF DeeTypeObject BlackListMapping_Type;
#define BlackListMapping_Check(x)      DeeObject_InstanceOfExact(x, &BlackListMapping_Type) /* `_BlackListMapping' is final */
#define BlackListMapping_CheckExact(x) DeeObject_InstanceOfExact(x, &BlackListMapping_Type)


INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListMapping_IsBlackListed(BlackListMapping *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListMapping_IsBlackListedStringHash(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL BlackListMapping_IsBlackListedStringLenHash(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL BlackListMapping_BoundItem(BlackListMapping *__restrict self, DeeObject *__restrict name, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL BlackListMapping_BoundItemStringHash(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL BlackListMapping_BoundItemStringLenHash(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListMapping_GetItem(BlackListMapping *self, DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListMapping_GetItemStringHash(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL BlackListMapping_GetItemStringLenHash(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL BlackListMapping_GetItemDef(BlackListMapping *__restrict self, DeeObject *__restrict name, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL BlackListMapping_GetItemStringHashDef(BlackListMapping *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL BlackListMapping_GetItemStringLenHashDef(BlackListMapping *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, DeeObject *__restrict def);
#define BlackListMapping_IsBlackListedString(self, name)                         BlackListMapping_IsBlackListedStringHash(self, name, Dee_HashStr(name))
#define BlackListMapping_IsBlackListedStringLen(self, name, namesize)            BlackListMapping_IsBlackListedStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define BlackListMapping_BoundItemString(self, name, allow_missing)              BlackListMapping_BoundItemStringHash(self, name, Dee_HashStr(name), allow_missing)
#define BlackListMapping_BoundItemStringLen(self, name, namesize, allow_missing) BlackListMapping_BoundItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize), allow_missing)
#define BlackListMapping_GetItemString(self, name)                               BlackListMapping_GetItemStringHash(self, name, Dee_HashStr(name))
#define BlackListMapping_GetItemStringLen(self, name, namesize)                  BlackListMapping_GetItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define BlackListMapping_GetItemStringDef(self, name, def)                       BlackListMapping_GetItemStringHashDef(self, name, Dee_HashStr(name), def)
#define BlackListMapping_GetItemStringLenDef(self, name, namesize, def)          BlackListMapping_GetItemStringLenHashDef(self, name, namesize, Dee_HashPtr(name, namesize), def)


/* Construct a new mapping for a general-purpose mapping that follows the black-listing scheme.
 * NOTE: If `code' doesn't specify any keywords, re-return `kw' unmodified.
 * -> The returned objects can be used for any kind of mapping, such that in the
 *    case of kwmappings, `BlackListMapping_New(code, DeeKwdsMapping_New(kwds, argv))'
 *    would produce the semantically equivalent of `BlackListVarkwds_New(code, kwds, argv)'
 * -> This function is used to filter keyword arguments from varkwds when the general
 *    purpose keyword argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListMapping { "something_else" : "foobar" }'
 *    >> foo(**{ "x" : 10, "something_else" : "foobar" });
 */
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
BlackListMapping_New(struct code_object *__restrict code,
                     size_t positional_argc,
                     DeeObject *__restrict kw);



typedef struct {
	OBJECT_HEAD
	DREF DeeObject        *mi_iter; /* [1..1][const] An iterator for the underlying `mi_map->blm_kw'. */
	DREF BlackListMapping *mi_map;  /* [1..1][const] The general-purpose blacklist mapping being iterated. */
} BlackListMappingIterator;

INTDEF DeeTypeObject BlackListMappingIterator_Type;



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_VARKWDS_H */
