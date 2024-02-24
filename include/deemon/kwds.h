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
#ifndef GUARD_DEEMON_KWDS_H
#define GUARD_DEEMON_KWDS_H 1

#include "api.h"

#include "object.h"
#include "util/lock.h"

DECL_BEGIN


#ifdef DEE_SOURCE
#define dee_kwds_entry    kwds_entry
#define Dee_string_object string_object
#define dee_kwds_object   kwds_object
#define dee_kwargs        kwds_args
#define dee_keyword       keyword
#define DEFINE_KWDS       Dee_DEFINE_KWDS
#define Dee_code_object   code_object
#endif /* DEE_SOURCE */

struct Dee_string_object;
struct Dee_code_object;

struct dee_kwds_entry {
	DREF struct Dee_string_object *ke_name;  /* [1..1][SENTINAL(NULL)] Keyword name. */
	Dee_hash_t                     ke_hash;  /* [== Dee_HashStr(ke_name)][valid_if(ke_name)] Hash of this keyword. */
	size_t                         ke_index; /* [< kw_size:][valid_if(ke_name)]
	                                          * Argument vector index of this keyword.
	                                          * NOTE: This index is applied as an offset _after_ positional
	                                          *       arguments, meaning that `0' is the first non-positional
	                                          *       argument, aka. `(argc - :kw_size) + 0' */
};

typedef struct dee_kwds_object DeeKwdsObject;
struct dee_kwds_object {
	/* This type of object is used by user-code to
	 * re-map the designation of optional arguments.
	 * When given, unpacking the an argument list will
	 * look at the last couple of names of arguments and
	 * perform a lookup of them within this object:
	 * Usercode:
	 * >> foo(10, 20, func: operator +);
	 * ASM:
	 * >> push  $10
	 * >> push  $20
	 * >> push  extern @operators:@__pooad
	 * >> call  extern @...:@foo, #3, const @{ func: 0 }
	 * foo (in Usercode):
	 * >> function foo(x, y, func = none) -> func(x, y);
	 * foo (in C):
	 * >> PRIVATE WUNUSED DREF DeeObject *DCALL
	 * >> foo(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	 * >>     DeeObject *x, *y, *func = Dee_None;
	 * >>     PRIVATE struct dee_keyword kwlist[] = { K(x), K(y), K(func), KEND };
	 * >>     if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "oo|o:foo", &x, &y, &func))
	 * >>         goto err;
	 * >>     return DeeObject_CallPack(func, 2, x, y);
	 * >> err:
	 * >>     return -1;
	 * >> }
	 * With that in mind, indices of a Kwds object refer to the
	 * last `kw_size' arguments of the associated argument list,
	 * thus meaning that calling a function with keyword arguments
	 * has no performance down-sides, since a regular, invocation
	 * (regardless of the presence of keyword labels) results
	 * in zero object allocations!
	 * WARNING: A Kwds object in itself (while also being mapping-like)
	 *          isn't actually a mapping for the objects bound to keywords,
	 *          but rather for the non-positional argument indices used by
	 *          those keywords. - It's actually {(string, int)...}-like
	 *          However, you can easily construct a {(string, Object)...}-like
	 *          mapping by calling `DeeKwdsMapping_New()' (see below) */
	Dee_OBJECT_HEAD
	size_t                                         kw_size; /* [const] The number of valid entries in `kw_map'. */
	size_t                                         kw_mask; /* [const] Mask for keyword names. */
	COMPILER_FLEXIBLE_ARRAY(struct dee_kwds_entry, kw_map); /* [const] Keyword name->index map. */
};
#define DeeKwds_MAPNEXT(i, perturb) \
	((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)

#define Dee_DEFINE_KWDS(name, kw_size_, kw_mask_, ...) \
	struct {                                           \
		Dee_OBJECT_HEAD                                \
		size_t kw_size;                                \
		size_t kw_mask;                                \
		struct dee_kwds_entry kw_map[(kw_mask_) + 1];  \
	} name = {                                         \
		Dee_OBJECT_HEAD_INIT(&DeeKwds_Type),           \
		kw_size_,                                      \
		kw_mask_,                                      \
		__VA_ARGS__                                    \
	}


DDATDEF DeeTypeObject DeeKwds_Type;
#define DeeKwds_SIZE(ob)       ((DeeKwdsObject *)Dee_REQUIRES_OBJECT(ob))->kw_size
#define DeeKwds_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeKwds_Type) /* _Kwds is final */
#define DeeKwds_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeKwds_Type)


#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED DREF DeeObject *DCALL
DeeKwds_NewWithHint(size_t num_items);

/* Append a new entry for `name'.
 * NOTE: The keywords argument index is set to the old number of
 *       keywords that had already been defined previously. */
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) int
(DCALL DeeKwds_AppendStringLenHash)(DREF DeeObject **__restrict p_self,
                                    char const *__restrict name,
                                    size_t name_len, Dee_hash_t hash);
#define DeeKwds_AppendStringLen(p_self, name, name_len) DeeKwds_AppendStringLenHash(p_self, name, name_len, Dee_HashPtr(name, name_len))
#define DeeKwds_AppendStringHash(p_self, name, hash)    DeeKwds_AppendStringLenHash(p_self, name, strlen(name), hash)
#define DeeKwds_AppendString(p_self, name)              DeeKwds_AppendStringHash(p_self, name, Dee_HashStr(name))

INTDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeKwds_Append)(DREF DeeObject **__restrict p_self,
                       DeeObject *__restrict name);

/* Return the keyword-entry associated with `keyword_index'
 * The caller must ensure that `keyword_index < DeeKwds_SIZE(self)' */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) struct dee_kwds_entry *DCALL
DeeKwds_GetByIndex(DeeObject *__restrict self, size_t keyword_index);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeKwds_AppendStringLenHash(p_self, name, name_len, hash) \
	__builtin_expect(DeeKwds_AppendStringLenHash(p_self, name, name_len, hash), 0)
#define DeeKwds_Append(p_self, name) \
	__builtin_expect(DeeKwds_Append(p_self, name), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */
#endif /* CONFIG_BUILDING_DEEMON */


typedef struct {
	/* A SharedMapping-like proxy object that can be used to wrap
	 * an argument list vector alongside a KwdsObject in order to
	 * construct a mapping-like wrapper for keyword arguments:
	 * >> DREF DeeObject *DCALL
	 * >> foo(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	 * >>	DREF DeeObject *result;
	 * >>	if (kw) {
	 * >>		if (DeeKwds_Check(kw)) {
	 * >>			size_t num_keywords;
	 * >>			num_keywords = DeeKwds_SIZE(kw);
	 * >>			if (num_keywords > argc) {
	 * >>				// Argument list is too short of the given keywords
	 * >>				DeeError_Throwf(&DeeError_TypeError, ...);
	 * >>				return NULL;
	 * >>			}
	 * >>			argc -= num_keywords;
	 * >>			// Turn keywords and arguments into a proper mapping-like object.
	 * >>			kw = DeeKwdsMapping_New(kw, argv + argc);
	 * >>			if unlikely(!kw)
	 * >>				return NULL;
	 * >>		} else {
	 * >>			// The given `kw' already is a mapping-
	 * >>			// like object for named arguments.
	 * >>			Dee_Incref(kw);
	 * >>		}
	 * >>	} else {
	 * >>		kw = Dee_EmptyMapping;
	 * >>		Dee_Incref(kw);
	 * >>	}
	 * >>	result = bar(argc, argv, kw);
	 * >>	if (DeeKwdsMapping_Check(kw) &&
	 * >>	    DeeKwdsMapping_ARGV(kw) == argv + argc) {
	 * >>		// If we're the ones owning the keywords-mapping, we must also decref() it.
	 * >>		DeeKwdsMapping_Decref(kw);
	 * >>	} else {
	 * >>		Dee_Decref(kw);
	 * >>	}
	 * >>	return result;
	 * >> }
	 * NOTE: The construction of a wrapper as used above can be automated
	 *       by calling `DREF DeeObject *kw = DeeKwMapping_New(&argc, argv, kw)',
	 *       with the cleanup then being implemented by `DeeKwMapping_Decref(argc, argv, kw)'
	 * >> DREF DeeObject *DCALL
	 * >> foo(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	 * >> 	DREF DeeObject *result;
	 * >> 	kw = DeeKwMapping_New(&argc, argv, kw);
	 * >> 	if unlikely(!kw)
	 * >> 		return NULL;
	 * >> 	result = bar(argc, argv, kw);
	 * >> 	DeeKwMapping_Decref(argc, argv, kw);
	 * >> 	return result;
	 * >> }
	 */
	Dee_OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                       kmo_lock;  /* Lock for this kwds wrapper. */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeKwdsObject                       *kmo_kwds;  /* [1..1][const] The Keyword arguments.
	                                                      * NOTE: If revived during unsharing, the object in this field
	                                                      *       gets incref'd, meaning that before that point, this
	                                                      *       field doesn't actually carry a reference. */
	DREF DeeObject                          **kmo_argv;  /* [1..1][kmo_kwds->kw_size][lock(kmo_lock)] The Keyword arguments. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, kmo_args); /* [1..1][kmo_kwds->kw_size][const] Backup storage for `kmo_argv' */
} DeeKwdsMappingObject;

#define DeeKwdsMapping_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->kmo_lock)
#define DeeKwdsMapping_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->kmo_lock)
#define DeeKwdsMapping_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->kmo_lock)
#define DeeKwdsMapping_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->kmo_lock)
#define DeeKwdsMapping_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->kmo_lock)
#define DeeKwdsMapping_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->kmo_lock)
#define DeeKwdsMapping_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->kmo_lock)
#define DeeKwdsMapping_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->kmo_lock)
#define DeeKwdsMapping_LockRead(self)       Dee_atomic_rwlock_read(&(self)->kmo_lock)
#define DeeKwdsMapping_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->kmo_lock)
#define DeeKwdsMapping_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->kmo_lock)
#define DeeKwdsMapping_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->kmo_lock)
#define DeeKwdsMapping_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->kmo_lock)
#define DeeKwdsMapping_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->kmo_lock)
#define DeeKwdsMapping_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->kmo_lock)
#define DeeKwdsMapping_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->kmo_lock)

DDATDEF DeeTypeObject DeeKwdsMapping_Type;
#define DeeKwdsMapping_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeKwdsMapping_Type) /* `_KwdsMapping' is final */
#define DeeKwdsMapping_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeKwdsMapping_Type)
#define DeeKwdsMapping_KWDS(ob)       ((DeeKwdsMappingObject *)Dee_REQUIRES_OBJECT(ob))->kmo_kwds
#define DeeKwdsMapping_ARGV(ob)       ((DeeKwdsMappingObject *)Dee_REQUIRES_OBJECT(ob))->kmo_argv

/* Construct a keywords-mapping object from a given `kwds' object,
 * as well as an argument vector that will be shared with the mapping.
 * The returned object then a mapping {(string, Object)...} for the
 * actual argument values passed to the function.
 * NOTE: The caller must later invoke `DeeKwdsMapping_Decref()' in order
 *       to clean up the returned object. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwdsMapping_New(/*Kwds*/ DeeObject *kwds,
                   DeeObject *const *kw_argv);

/* Unshare the argument vector from a keywords-mapping object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
DFUNDEF NONNULL((1)) void DCALL
DeeKwdsMapping_Decref(DREF /*KwdsMapping*/ DeeObject *__restrict self);

#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeKwdsMapping_HasItemStringHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL DeeKwdsMapping_HasItemStringLenHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, size_t namesize, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_GetItemNR(DeeKwdsMappingObject *__restrict self, /*string*/ DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_GetItemNRStringHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DeeObject *DCALL DeeKwdsMapping_GetItemNRStringLenHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, size_t namesize, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL DeeKwdsMapping_GetItemNRDef(DeeKwdsMappingObject *__restrict self, /*string*/ DeeObject *__restrict name, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL DeeKwdsMapping_GetItemNRStringHashDef(DeeKwdsMappingObject *__restrict self, char const *__restrict name, Dee_hash_t hash, DeeObject *def);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1, 5)) DeeObject *DCALL DeeKwdsMapping_GetItemNRStringLenHashDef(DeeKwdsMappingObject *__restrict self, char const *__restrict name, size_t namesize, Dee_hash_t hash, DeeObject *def);
#define DeeKwdsMapping_HasItemString(self, name)                        DeeKwdsMapping_HasItemStringHash(self, name, Dee_HashStr(name))
#define DeeKwdsMapping_HasItemStringLen(self, name, namesize)           DeeKwdsMapping_HasItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeKwdsMapping_GetItemNRString(self, name)                      DeeKwdsMapping_GetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeKwdsMapping_GetItemNRStringDef(self, name, def)              DeeKwdsMapping_GetItemNRStringHashDef(self, name, Dee_HashStr(name), def)
#define DeeKwdsMapping_GetItemNRStringLen(self, name, namesize)         DeeKwdsMapping_GetItemNRStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeKwdsMapping_GetItemNRStringLenDef(self, name, namesize, def) DeeKwdsMapping_GetItemNRStringLenHashDef(self, name, namesize, Dee_HashPtr(name, namesize), def)
#endif /* CONFIG_BUILDING_DEEMON */


/* Construct/access keyword arguments passed to a function as a
 * high-level {string: Object}-like mapping that is bound to the
 * actually mapped arguments. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwMapping_New(size_t *__restrict p_argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF ATTR_INS(2, 1) NONNULL((3)) void DCALL
DeeKwMapping_Decref(size_t argc, DeeObject *const *argv, DREF DeeObject *kw);


typedef struct dee_kwargs DeeKwArgs;
struct dee_kwargs {
	size_t            kwa_kwused; /* # of used keyword arguments (assuming that any argument is loaded 1 time at most) */
	DeeObject *const *kwa_kwargv; /* [0..1] Positional arguments to supplement `kwa_kw' (or NULL if unused). */
	DeeObject        *kwa_kw;     /* [0..1] Keyword arguments descriptor / mapping (supports `DeeObject_IsKw()') */
};

/* Check if there *may* still be more keyword arguments available in `self'. */
#define DeeKwArgs_MaybeHaveMoreArgs(self) \
	((self)->kwa_kw && (!DeeKwds_Check((self)->kwa_kw) || ((self)->kwa_kwused < DeeKwds_SIZE((self)->kwa_kw))))

/* Initialize `self' to load keyword arguments.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeKwArgs_Init)(DeeKwArgs *__restrict self, size_t *__restrict p_argc,
                       DeeObject *const *argv, DeeObject *kw);

/* Indicate that you're doing loading arguments from `self'.
 * This function asserts that `kwa_kwused == #kwa_kw' so-as
 * to ensure that `kwa_kw' doesn't contain any unused keyword
 * arguments.
 * @param: positional_argc: The value of `*p_argc' after `DeeKwArgs_Init()' returned.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeKwArgs_Done)(DeeKwArgs *__restrict self,
                       size_t positional_argc,
                       char const *function_name);

/* Lookup a named keyword argument from `self'
 * @return: * :   Reference to named keyword argument.
 * @return: NULL: An error was thrown.*/
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_GetItemNR(DeeKwArgs *__restrict self, /*string*/ DeeObject *__restrict name);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_GetItemNRStringHash(DeeKwArgs *__restrict self, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DeeObject *DCALL DeeKwArgs_GetItemNRStringLenHash(DeeKwArgs *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL DeeKwArgs_GetItemNRDef(DeeKwArgs *__restrict self, /*string*/ DeeObject *__restrict name, DeeObject *def);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL DeeKwArgs_GetItemNRStringHashDef(DeeKwArgs *__restrict self, char const *__restrict name, Dee_hash_t hash, DeeObject *def);
DFUNDEF WUNUSED ATTR_INS(2, 3) NONNULL((1, 5)) DeeObject *DCALL DeeKwArgs_GetItemNRStringLenHashDef(DeeKwArgs *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash, DeeObject *def);
#define DeeKwArgs_GetItemNRString(self, name)                     DeeKwArgs_GetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeKwArgs_GetItemNRStringLen(self, name, namelen)         DeeKwArgs_GetItemNRStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
#define DeeKwArgs_GetItemNRStringDef(self, name, def)             DeeKwArgs_GetItemNRStringHashDef(self, name, Dee_HashStr(name), def)
#define DeeKwArgs_GetItemNRStringLenDef(self, name, namelen, def) DeeKwArgs_GetItemNRStringLenHashDef(self, name, namelen, Dee_HashPtr(name, namelen), def)


/* In a keyword-enabled function, return the argument associated with a given
 * `name', or throw a TypeError exception or return `def' if not provided.
 *
 * Use these functions when you're uncertain if "kw" is non-NULL or might be
 * `DeeKwds_Check()'. If you're certain that `kw != NULL && !DeeKwds_Check(kw)',
 * you can also use the set of `DeeKw_GetItemNR*' functions below.
 *
 * IMPORTANT: These functions to *NOT* return references! */
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL DeeArg_GetKwNR(size_t argc, DeeObject *const *argv, DeeObject *kw, /*string*/ DeeObject *__restrict name);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL DeeArg_GetKwNRStringHash(size_t argc, DeeObject *const *argv, DeeObject *kw, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 1) ATTR_INS(4, 5) DeeObject *DCALL DeeArg_GetKwNRStringLenHash(size_t argc, DeeObject *const *argv, DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 5)) DeeObject *DCALL DeeArg_GetKwNRDef(size_t argc, DeeObject *const *argv, DeeObject *kw, /*string*/ DeeObject *__restrict name, DeeObject *def);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 6)) DeeObject *DCALL DeeArg_GetKwNRStringHashDef(size_t argc, DeeObject *const *argv, DeeObject *kw, char const *__restrict name, Dee_hash_t hash, DeeObject *def);
DFUNDEF WUNUSED ATTR_INS(2, 1) ATTR_INS(4, 5) NONNULL((7)) DeeObject *DCALL DeeArg_GetKwNRStringLenHashDef(size_t argc, DeeObject *const *argv, DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash, DeeObject *def);
#define DeeArg_GetKwNRString(argc, argv, kw, name)                     DeeArg_GetKwNRStringHash(argc, argv, kw, name, Dee_HashStr(name))
#define DeeArg_GetKwNRStringLen(argc, argv, kw, name, namelen)         DeeArg_GetKwNRStringLenHash(argc, argv, kw, name, namelen, Dee_HashPtr(name, namelen))
#define DeeArg_GetKwNRStringDef(argc, argv, kw, name, def)             DeeArg_GetKwNRStringHashDef(argc, argv, kw, name, Dee_HashStr(name), def)
#define DeeArg_GetKwNRStringLenDef(argc, argv, kw, name, namelen, def) DeeArg_GetKwNRStringLenHashDef(argc, argv, kw, name, namelen, Dee_HashPtr(name, namelen), def)


/* Interface for working with generic keyword argument mappings.
 *
 * The `DeeObject *kw' argument of functions is special, in that it essentially
 * implements "DeeObject_GetItem()" in such a way that the returned object doesn't
 * need to be a reference. This is important because it means that DeeArg_UnpackKw
 * can extract objects from keyword arguments, without the caller needing to store
 * those references somewhere to decref them later.
 *
 * When calling (e.g.) `DeeObject_CallKw()', you have to make that the object you
 * pass as "kw" is either NULL, or fulfills `DeeObject_IsKw(kw)'. On the other side,
 * if you're given an object through "kw", you can be certain that it fulfills the
 * requirement of `DeeObject_IsKw(kw)'. If you are uncertain if some given object
 * fulfils the requirement of `DeeObject_IsKw()', and want to make sure that it does
 * by replacing it with a wrapper that *does* fulfil `DeeObject_IsKw()', you can use
 * `DeeKw_Wrap()' to wrap that object.
 *
 * When an object fulfills `DeeObject_IsKw(ob)', the `DeeKw_*' API of functions can
 * be used with that object. */
#define DeeType_IsKw(tp)   ((tp)->tp_features & Dee_TF_KW)
#define DeeObject_IsKw(ob) DeeType_IsKw(Dee_TYPE(ob))

/* Check if `kwds' fulfills `DeeObject_IsKw()', and if not, wrap it as a generic
 * kw-capable wrapper that calls forward to `DeeObject_GetItem(kwds)' when keywords
 * are queried, but then caches returned references such that the keyword consumer
 * doesn't need to keep track of them. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeKw_Wrap(DeeObject *__restrict kwds);

/* Lookup keyword arguments. These functions may be used to extract keyword arguments
 * when the caller knows that `kw != NULL && DeeObject_IsKw(kw) && !DeeKwds_Check(kw)'.
 *
 * IMPORTANT: These functions to *NOT* return references! */
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_GetItemNR(DeeObject *__restrict kw, /*string*/ DeeObject *__restrict name);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_GetItemNRStringHash(DeeObject *__restrict kw, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DeeObject *DCALL DeeKw_GetItemNRStringLenHash(DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL DeeKw_GetItemNRDef(DeeObject *__restrict kw, /*string*/ DeeObject *__restrict name, DeeObject *def);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL DeeKw_GetItemNRStringHashDef(DeeObject *kw, char const *__restrict name, Dee_hash_t hash, DeeObject *def);
DFUNDEF WUNUSED ATTR_INS(2, 3) NONNULL((1, 5)) DeeObject *DCALL DeeKw_GetItemNRStringLenHashDef(DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash, DeeObject *def);
#define DeeKw_GetItemNRString(kw, name)                     DeeKw_GetItemNRStringHash(kw, name, Dee_HashStr(name))
#define DeeKw_GetItemNRStringLen(kw, name, namelen)         DeeKw_GetItemNRStringLenHash(kw, name, namelen, Dee_HashPtr(name, namelen))
#define DeeKw_GetItemNRStringDef(kw, name, def)             DeeKw_GetItemNRStringHashDef(kw, name, Dee_HashStr(name), def)
#define DeeKw_GetItemNRStringLenDef(kw, name, namelen, def) DeeKw_GetItemNRStringLenHashDef(kw, name, namelen, Dee_HashPtr(name, namelen), def)


/* Construct the canonical wrapper for "**kwds" in a user-defined function,
 * such that the names of positional arguments passed via "kw" are black-listed.
 *
 * The returned object is always kw-capable, and this function is used in code
 * such as this:
 * >> function foo(a, b, **kwds) {
 * >>     return kwds;
 * >> }
 * >> print repr foo(**{ "a": 10, "b": 20, "c": 30 }); // prints '{ "c": 30 }' (because "a" and "b" are black-listed)
 *
 * This function is the high-level wrapper around:
 * - DeeBlackListKwds_New
 * - DeeBlackListKw_New
 * - DeeKwdsMapping_New
 *
 * IMPORTANT: The returned object must be decref'd using `DeeKwBlackList_Decref()'
 *            once the function that created it returns. */
DFUNDEF WUNUSED NONNULL((1, 3)) ATTR_INS(4, 2) DREF DeeObject *DCALL
DeeKwBlackList_New(struct Dee_code_object *__restrict code,
                   size_t positional_argc,
                   DeeObject *const *positional_argv,
                   DeeObject *__restrict kw);

/* Unshare pointers to "argv" if "self" is still shared. */
DFUNDEF NONNULL((1)) void DCALL
DeeKwBlackList_Decref(DREF DeeObject *__restrict self);






/************************************************************************/
/* Misc., specialized wrappers for keyword arguments                    */
/************************************************************************/

/* Proxy types used to drive variable keywords in user-code functions.
 * The basic idea here is to wrap either a Kwds+Argv pair, or a general
 * purpose mapping-like object (possibly with additional optimization for
 * _RoDict objects) which simply propagates all access to the underlying
 * mapping, but filters out a certain set of keys such that it appears as
 * though those keys were not part of the mapping itself. */

typedef struct {
	struct Dee_string_object *blve_str; /* [0..1] The keyword name that is being blacklisted.
	                                     * `NULL' is used to identify unused/sentinel entries.
	                                     * NOTE: Even when non-NULL, this field does not hold
	                                     *       a reference, as all possible strings are already
	                                     *       referenced via `:blkd_code->co_keywords' */
} DeeBlackListKwdsEntry;



typedef struct {
	/* Variable keywords mapping-like object for Kwds+Argv */
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                            blkd_lock;  /* Lock for this kwds wrapper. */
#endif /* !CONFIG_NO_THREADS */
	DREF struct Dee_code_object                   *blkd_code;  /* [1..1][const] The code object who's keyword arguments should
	                                                            *               be blacklisted from the the resulting mapping.
	                                                            * NOTE: This code doesn't always takes at least 1 argument, and
	                                                            *       always specifies its keywords. When constructing an
	                                                            *      `DeeBlackListKwdsObject' object with code not doing this, a
	                                                            *      `DeeKwdsMappingObject' object will be returned instead,
	                                                            *       which maps keywords to arguments without including a
	                                                            *       blacklist of arguments which are not to be mapped.
	                                                            * NOTE: If revived during unsharing, the object in this field
	                                                            *       gets incref'd, meaning that before that point, this
	                                                            *       field doesn't actually carry a reference. */
	size_t                                         blkd_ckwc;  /* [const][!0] Number of black-listed keywords */
	struct Dee_string_object *const               *blkd_ckwv;  /* [1..1][const][1..blkd_ckwc][const] Vector of black-listed keywords. */
	DREF DeeKwdsObject                            *blkd_kwds;  /* [1..1][const] The mapping for kwds-to-argument-index.
	                                                            * NOTE: This kwds object always has a `kw_size' that is
	                                                            *       non-ZERO. - When trying to construct a `DeeBlackListKwdsObject'
	                                                            *       object from an empty keyword list, an empty mapping
	                                                            *       will be returned instead.
	                                                            * NOTE: If revived during unsharing, the object in this field
	                                                            *       gets incref'd, meaning that before that point, this
	                                                            *       field doesn't actually carry a reference. */
	DREF DeeObject                               **blkd_argv;  /* [1..1][const][0..blkd_kwds->kw_size][lock(blkd_lock)]
	                                                            * Shared argument list to which indices from `blkd_kwds' map.*/
	size_t                                         blkd_load;  /* [lock(blkd_lock, INCREMENT_ONLY)][<= blkd_ckwc]
	                                                            * Index of the next keyword which has yet to be loaded into
	                                                            * the `blkd_blck' hash-set for blacklisted identifiers. */
	size_t                                         blkd_mask;  /* [!0][const] Hash-mask for `blkd_blck' */
	COMPILER_FLEXIBLE_ARRAY(DeeBlackListKwdsEntry, blkd_blck); /* [lock(blkd_lock)][0..blkd_mask+1]
	                                                            * Hash-vector of loaded, black-listed keywords. */
//	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *,      blkd_args); /* Storage space for "blkd_argv" */
} DeeBlackListKwdsObject;
#define DeeBlackListKwds_KWDS(self)           ((DeeBlackListKwdsObject *)Dee_REQUIRES_OBJECT(self))->blkd_kwds
#define DeeBlackListKwds_BLCKNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)

#define DeeBlackListKwds_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->blkd_lock)
#define DeeBlackListKwds_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->blkd_lock)
#define DeeBlackListKwds_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->blkd_lock)
#define DeeBlackListKwds_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->blkd_lock)
#define DeeBlackListKwds_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->blkd_lock)
#define DeeBlackListKwds_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->blkd_lock)
#define DeeBlackListKwds_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->blkd_lock)
#define DeeBlackListKwds_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->blkd_lock)
#define DeeBlackListKwds_LockRead(self)       Dee_atomic_rwlock_read(&(self)->blkd_lock)
#define DeeBlackListKwds_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->blkd_lock)
#define DeeBlackListKwds_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->blkd_lock)
#define DeeBlackListKwds_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->blkd_lock)
#define DeeBlackListKwds_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->blkd_lock)
#define DeeBlackListKwds_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->blkd_lock)
#define DeeBlackListKwds_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->blkd_lock)
#define DeeBlackListKwds_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->blkd_lock)

DFUNDEF DeeTypeObject DeeBlackListKwds_Type;
#define DeeBlackListKwds_Check(x)      DeeObject_InstanceOfExact(x, &DeeBlackListKwds_Type) /* `_DeeBlackListKwds' is final */
#define DeeBlackListKwds_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBlackListKwds_Type)



/* Helper functions & RT optimization bindings. */
#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKwds_IsBlackListed(DeeBlackListKwdsObject *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKwds_IsBlackListedStringHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL DeeBlackListKwds_IsBlackListedStringLenHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKwds_HasItemStringHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL DeeBlackListKwds_HasItemStringLenHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_GetItemNR(DeeBlackListKwdsObject *__restrict self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_GetItemNRStringHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DeeObject *DCALL DeeBlackListKwds_GetItemNRStringLenHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL DeeBlackListKwds_GetItemNRDef(DeeBlackListKwdsObject *__restrict self, DeeObject *__restrict name, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_GetItemNRStringHashDef(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DeeObject *DCALL DeeBlackListKwds_GetItemNRStringLenHashDef(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, DeeObject *__restrict def);
#define DeeBlackListKwds_IsBlackListedString(self, name)                  DeeBlackListKwds_IsBlackListedStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKwds_IsBlackListedStringLen(self, name, namesize)     DeeBlackListKwds_IsBlackListedStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeBlackListKwds_HasItemString(self, name)                        DeeBlackListKwds_HasItemStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKwds_HasItemStringLen(self, name, namesize)           DeeBlackListKwds_HasItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeBlackListKwds_GetItemNRString(self, name)                      DeeBlackListKwds_GetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKwds_GetItemNRStringLen(self, name, namesize)         DeeBlackListKwds_GetItemNRStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeBlackListKwds_GetItemNRStringDef(self, name, def)              DeeBlackListKwds_GetItemNRStringHashDef(self, name, Dee_HashStr(name), def)
#define DeeBlackListKwds_GetItemNRStringLenDef(self, name, namesize, def) DeeBlackListKwds_GetItemNRStringLenHashDef(self, name, namesize, Dee_HashPtr(name, namesize), def)
#endif /* CONFIG_BUILDING_DEEMON */

/* Construct a new mapping for keywords that follows the black-listing scheme.
 * NOTE: If `kwds' is empty, return `Dee_EmptyMapping' instead.
 * NOTE: If `code' doesn't specify any keywords, return `DeeKwdsMapping_New()' instead.
 * Otherwise, the caller must decref the returned object using `DeeBlackListKwds_Decref()'
 * -> This function is used to filter keyword arguments from varkwds when 
 *    kwargs argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_DeeBlackListKwds { "something_else" : "foobar" }'
 *    >> foo(x: 10, something_else: "foobar");
 */
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeBlackListKwds_New(struct Dee_code_object *__restrict code,
                     size_t positional_argc, DeeObject *const *kw_argv,
                     DeeKwdsObject *__restrict kwds);

/* Unshare the argument vector from a blacklist-varkwds object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
DFUNDEF NONNULL((1)) void DCALL
DeeBlackListKwds_Decref(DREF DeeObject *__restrict self);






typedef struct {
	/* Variable keywords mapping-like object for general-purpose mapping-like objects. */
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                            blkw_lock;  /* Lock for this kwds wrapper. */
#endif /* !CONFIG_NO_THREADS */
	DREF struct Dee_code_object                   *blkw_code;  /* [1..1][const] The code object who's keyword arguments should
	                                                            *               be blacklisted from the the resulting mapping.
	                                                            * NOTE: This code doesn't always takes at least 1 argument, and
	                                                            *       always specifies its keywords. When constructing a
	                                                            *       `DeeBlackListKwdsObject' object with code not doing this, a
	                                                            *       `DeeKwdsMappingObject' object will be returned instead,
	                                                            *       which maps keywords to arguments without including a
	                                                            *       blacklist of arguments which are not to be mapped.
	                                                            * NOTE: If revived during unsharing, the object in this field
	                                                            *       gets incref'd, meaning that before that point, this
	                                                            *       field doesn't actually carry a reference. */
	size_t                                         blkw_ckwc;  /* [const][!0] Number of black-listed keywords */
	struct Dee_string_object *const               *blkw_ckwv;  /* [1..1][const][1..blkw_ckwc][const] Vector of black-listed keywords. */
	DREF DeeObject                                *blkw_kw;    /* [1..1][const] The underlying mapping which is being affected. */
	size_t                                         blkw_load;  /* [lock(blkw_lock, INCREMENT_ONLY)][<= blkw_ckwc]
	                                                            * Index of the next keyword which has yet to be loaded into
	                                                            * the `blkw_blck' hash-set for blacklisted identifiers. */
	size_t                                         blkw_mask;  /* [!0][const] Hash-mask for `blkw_blck' */
	COMPILER_FLEXIBLE_ARRAY(DeeBlackListKwdsEntry, blkw_blck); /* [lock(blkw_lock)][0..blkw_mask+1]
	                                                               * Hash-vector of loaded, black-listed keywords. */
} DeeBlackListKwObject;
#define DeeBlackListKw_KW(self)             ((DeeBlackListKwObject *)Dee_REQUIRES_OBJECT(self))->blkw_kw
#define DeeBlackListKw_BLCKNEXT(i, perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1), (perturb) >>= 5)

#define DeeBlackListKw_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->blkw_lock)
#define DeeBlackListKw_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->blkw_lock)
#define DeeBlackListKw_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->blkw_lock)
#define DeeBlackListKw_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->blkw_lock)
#define DeeBlackListKw_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->blkw_lock)
#define DeeBlackListKw_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->blkw_lock)
#define DeeBlackListKw_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->blkw_lock)
#define DeeBlackListKw_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->blkw_lock)
#define DeeBlackListKw_LockRead(self)       Dee_atomic_rwlock_read(&(self)->blkw_lock)
#define DeeBlackListKw_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->blkw_lock)
#define DeeBlackListKw_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->blkw_lock)
#define DeeBlackListKw_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->blkw_lock)
#define DeeBlackListKw_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->blkw_lock)
#define DeeBlackListKw_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->blkw_lock)
#define DeeBlackListKw_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->blkw_lock)
#define DeeBlackListKw_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->blkw_lock)

DDATDEF DeeTypeObject DeeBlackListKw_Type;
#define DeeBlackListKw_Check(x)      DeeObject_InstanceOfExact(x, &DeeBlackListKw_Type) /* `_DeeBlackListKw' is final */
#define DeeBlackListKw_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBlackListKw_Type)

#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKw_IsBlackListed(DeeBlackListKwObject *__restrict self, /*string*/ DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKw_IsBlackListedStringHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKw_IsBlackListedStringLenHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeBlackListKw_BoundItemStringHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, dhash_t hash, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeBlackListKw_BoundItemStringLenHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, bool allow_missing);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeBlackListKw_HasItemStringHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeBlackListKw_HasItemStringLenHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_GetItemNR(DeeBlackListKwObject *__restrict self, /*string*/ DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_GetItemNRStringHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_GetItemNRStringLenHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL DeeBlackListKw_GetItemNRDef(DeeBlackListKwObject *__restrict self, /*string*/ DeeObject *__restrict name, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL DeeBlackListKw_GetItemNRStringHashDef(DeeBlackListKwObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DeeObject *DCALL DeeBlackListKw_GetItemNRStringLenHashDef(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, DeeObject *__restrict def);
#define DeeBlackListKw_IsBlackListedString(self, name)                         DeeBlackListKw_IsBlackListedStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKw_IsBlackListedStringLen(self, name, namesize)            DeeBlackListKw_IsBlackListedStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeBlackListKw_BoundItemString(self, name, allow_missing)              DeeBlackListKw_BoundItemStringHash(self, name, Dee_HashStr(name), allow_missing)
#define DeeBlackListKw_BoundItemStringLen(self, name, namesize, allow_missing) DeeBlackListKw_BoundItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize), allow_missing)
#define DeeBlackListKw_GetItemNRString(self, name)                             DeeBlackListKw_GetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKw_GetItemNRStringLen(self, name, namesize)                DeeBlackListKw_GetItemNRStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeBlackListKw_GetItemNRStringDef(self, name, def)                     DeeBlackListKw_GetItemNRStringHashDef(self, name, Dee_HashStr(name), def)
#define DeeBlackListKw_GetItemNRStringLenDef(self, name, namesize, def)        DeeBlackListKw_GetItemNRStringLenHashDef(self, name, namesize, Dee_HashPtr(name, namesize), def)
#endif /* CONFIG_BUILDING_DEEMON */


/* Construct a new mapping for a general-purpose mapping that follows the black-listing scheme.
 * NOTE: If `code' doesn't specify any keywords, re-return `kw' unmodified.
 * -> The returned objects can be used for any kind of mapping, such that in the
 *    case of kwmappings, `DeeBlackListKw_New(code, DeeKwdsMapping_New(kwds, argv))'
 *    would produce the semantically equivalent of `DeeBlackListKwds_New(code, kwds, argv)'
 * -> This function is used to filter keyword arguments from varkwds when the general
 *    purpose keyword argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_DeeBlackListKw { "something_else" : "foobar" }'
 *    >> foo(**{ "x" : 10, "something_else" : "foobar" });
 */
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeBlackListKw_New(struct Dee_code_object *__restrict code,
                   size_t positional_argc,
                   DeeObject *__restrict kw);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeKwArgs_Init(self, p_argc, argv, kw)               __builtin_expect(DeeKwArgs_Init(self, p_argc, argv, kw), 0)
#define DeeKwArgs_Done(self, positional_argc, function_name) __builtin_expect(DeeKwArgs_Done(self, positional_argc, function_name), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_KWDS_H */
