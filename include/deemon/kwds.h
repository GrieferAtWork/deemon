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

/*
 * ============= KEYWORD COMPONENTS OVERVIEW :: C API UTILS =============
 *
 * DeeKwArgs
 * - Usage:
 *   - Helper for implementing dynamic (but still fixed) keyword arguments in C.
 *   - >> DeeKwArgs kwargs;
 *     >> DO(DeeKwArgs_Init(&kwargs, &argc, argv, kw));
 *     >> HANDLE_POSITION_ARGS(argc, argv); // "argc" was updated to exclude kw-through-argv (s.a. `DeeKwdsObject')
 *     >> DeeObject *a = DeeKwArgs_GetItemNRStringDef(&kwargs, "arg1", Dee_None);
 *     >> DeeObject *b = DeeKwArgs_GetItemNRStringDef(&kwargs, "arg2", Dee_None);
 *     >> DeeObject *c = DeeKwArgs_GetItemNRStringDef(&kwargs, "arg3", Dee_None);
 *     >> DO(DeeKwArgs_Done(&kwargs, argc)); // Asserts that all keyword arguments were used
 *
 * DeeArg_GetKwNR
 * - Usage:
 *   - Direct function to load a specific keyword argument in C
 *   - >> DeeObject *a = DeeArg_GetKwNRStringDef(argc, argv, kw, "arg1", Dee_None);
 *     >> DeeObject *b = DeeArg_GetKwNRStringDef(argc, argv, kw, "arg2", Dee_None);
 *     >> DeeObject *c = DeeArg_GetKwNRStringDef(argc, argv, kw, "arg3", Dee_None);
 *
 * DeeKw_Wrap
 * - Usage:
 *   - Must be called to wrap a generic mapping-like object for the purpose of using
 *     that object as a "kw" argument in a call to (e.g.) `DeeObject_CallKw()'
 *   - In user-code, the `ASM_CAST_VARKWDS' instruction calls this function
 *   - This function checks if the given "kwds" is kw-capable (DeeObject_IsKw),
 *     and if it isn't, it returns "DeeCachedDict_New(kwds)".
 *   - kw-capable means that the object supports `DeeKw_GetItemNR*', which is the
 *     set of low-level functions used for loading keyword arguments.
 *
 * DeeKw_GetItemNR
 * - Usage:
 *   - Don't use unless you know what you're doing.
 *   - Used to load keyword arguments from kw-capable kw-objects, but does *NOT*
 *     support `DeeKwdsObject' (which must be handled by the caller explicitly)
 *   - The passed "kw" must be `DeeObject_IsKw(kw) && !DeeKwds_Check(kw)'
 *
 * DeeKwBlackList_New
 * - Usage:
 *   - Used by the runtime interpreter when "ASM_PUSH_VARKWDS" is used.
 *   - This function figures out how to correctly package argc/argv/kw into
 *     a deemon object that user-code is able to understand, whilst also
 *     filtering and keyword arguments that were loaded into positional args.
 *   - The returned object must be decref'd using `DeeKwBlackList_Decref()'
 *     before "argc/argv/kw" go out-of-scope. After that call, the object
 *     was either destroyed, or shared caches were unshared (which is the
 *     case when varkwds live longer than the function's score; i.e. when
 *     varkwds are returned, passed to a thread, written to a Cell or global,
 *     etc...)
 *
 * DeeKwBlackList_Decref
 * - See DeeKwBlackList_New
 *
 *
 *
 * ============= KEYWORD COMPONENTS OVERVIEW :: OBJECT TYPES =============
 *
 * DeeKwdsObject extends Mapping {string: int}
 * - Behavior:
 *   - May be passed as "DeeObject *kw"
 *   - Keyword names are constant, and values are passed in "argv"
 *   - The mapped "int index" maps to the value as "argv[argc - DeeKwds_SIZE(kw) + index]"
 *   - When used, the last "DeeKwds_SIZE(kw)" elements of "argv"
 *     must not be considered "normal" positional arguments
 * - Usage:
 *   - The default way of passing keyword arguments in deemon usercode
 *   - >> local x = s.replace("a", "b", max: 42);
 *     - Call is generated as (argc = 3, argv = {"a", "b", 42}, kw = {"max": 0})
 *
 * DeeKwdsMappingObject extends Mapping {string: Object}
 * - Behavior:
 *   - Combines a "DeeKwdsObject" with "DeeObject **argv" to form a normal
 *     deemon Mapping object which can be used to access variable keyword
 *     arguments.
 * - Usage:
 *   - >> function foo(**kwds) -> kwds;
 *     >> print repr foo(a: 10, b: 20);
 *     - Call is generated as (argc = 2, argv = {10, 20}, kw = {"a": 0, "b": 1})
 *     - "foo" returns a "DeeKwdsMappingObject"
 *     - Prints "{ "a": 10, "b": 20 }"
 *
 * DeeBlackListKwdsObject extends Mapping {string: Object}
 * - Behavior:
 *   - Same as "DeeKwdsMappingObject", but apply an extra filter to deny the
 *     existence of keyword arguments that were loaded into positional args.
 *   - Semantically the same as using `DeeBlackListKwObject' to wrap `DeeKwdsMappingObject'
 * - Usage:
 *   - >> function foo(a, **kwds) -> kwds;
 *     >> print repr foo(a: 10, b: 20);
 *     - Call is generated as (argc = 2, argv = {10, 20}, kw = {"a": 0, "b": 1})
 *     - "foo" returns a "DeeBlackListKwdsObject"
 *     - Prints "{ "b": 20 }"
 *
 * DeeBlackListKwObject extends Mapping {string: Object}
 * - Behavior:
 *   - Same as "DeeBlackListKwdsObject", but used when the internal "kw" passed
 *     to the user-function wasn't `DeeKwdsObject', but some other kw-capable
 *     mapping object.
 * - Usage:
 *   - >> function foo(a, **kwds) -> kwds;
 *     >> print repr foo(**{ "a": 10, "b": 20 });
 *     - Call is generated as (argc = 0, argv = {}, kw = {"a": 10, "b": 20})
 *     - "foo" returns a "DeeBlackListKwObject"
 *     - Prints "{ "b": 20 }"
 */


#ifdef DEE_SOURCE
#define dee_kwds_entry    kwds_entry
#define Dee_string_object string_object
#define dee_kwds_object   kwds_object
#define dee_kwargs        kwds_args
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
	 * >>     PRIVATE DEFINE_KWLIST(kwlist, { K(x), K(y), K(func), KEND });
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

/* Translate an argument keyword name into its index within at given `DeeKwdsObject *self'.
 * When `self' doesn't contain a descriptor for `name', no error is thrown, and `(size_t)-1'
 * is returned instead. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL DeeKwds_IndexOf(DeeObject const *self, /*string*/ DeeObject *name);
DFUNDEF ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL DeeKwds_IndexOfStringHash(DeeObject const *__restrict self, char const *__restrict name, Dee_hash_t hash);
DFUNDEF ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL DeeKwds_IndexOfStringLenHash(DeeObject const *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);


#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED DREF DeeObject *DCALL
DeeKwds_NewWithHint(size_t num_items);

/* Append a new entry for `name'.
 * NOTE: The keywords argument index is set to the old number of
 *       keywords that had already been defined previously. */
INTDEF WUNUSED NONNULL((1, 2)) int
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
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_GetItemNR(DeeKwdsMappingObject *self, /*string*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_GetItemNRStringHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_GetItemNRStringLenHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_TryGetItemNR(DeeKwdsMappingObject *self, /*string*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_TryGetItemNRStringHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwdsMapping_TryGetItemNRStringLenHash(DeeKwdsMappingObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
#define DeeKwdsMapping_GetItemNRString(self, name)                DeeKwdsMapping_GetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeKwdsMapping_GetItemNRStringLen(self, name, namelen)    DeeKwdsMapping_GetItemNRStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
#define DeeKwdsMapping_TryGetItemNRString(self, name)             DeeKwdsMapping_TryGetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeKwdsMapping_TryGetItemNRStringLen(self, name, namelen) DeeKwdsMapping_TryGetItemNRStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
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
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_GetItemNR(DeeKwArgs *self, /*string*/ DeeObject *name);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_GetItemNRStringHash(DeeKwArgs *__restrict self, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_GetItemNRStringLenHash(DeeKwArgs *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_TryGetItemNR(DeeKwArgs *self, /*string*/ DeeObject *name);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_TryGetItemNRStringHash(DeeKwArgs *__restrict self, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKwArgs_TryGetItemNRStringLenHash(DeeKwArgs *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
#define DeeKwArgs_GetItemNRString(self, name)                     DeeKwArgs_GetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeKwArgs_GetItemNRStringLen(self, name, namelen)         DeeKwArgs_GetItemNRStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
#define DeeKwArgs_TryGetItemNRString(self, name)                  DeeKwArgs_TryGetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeKwArgs_TryGetItemNRStringLen(self, name, namelen)      DeeKwArgs_TryGetItemNRStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))

/* Helpers allowing the caller to specify a default value. */
LOCAL WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
DeeKwArgs_GetItemNRDef(DeeKwArgs *self, /*string*/ DeeObject *name, DeeObject *def) {
	DeeObject *result = DeeKwArgs_TryGetItemNR(self, name);
	if (result == ITER_DONE)
		result = def;
	return result;
}

LOCAL WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL
DeeKwArgs_GetItemNRStringHashDef(DeeKwArgs *__restrict self, char const *__restrict name,
                                 Dee_hash_t hash, DeeObject *def) {
	DeeObject *result = DeeKwArgs_TryGetItemNRStringHash(self, name, hash);
	if (result == ITER_DONE)
		result = def;
	return result;
}

LOCAL WUNUSED NONNULL((1, 2, 5)) DeeObject *DCALL
DeeKwArgs_GetItemNRStringLenHashDef(DeeKwArgs *__restrict self, char const *__restrict name,
                                    size_t namelen, Dee_hash_t hash, DeeObject *def) {
	DeeObject *result = DeeKwArgs_TryGetItemNRStringLenHash(self, name, namelen, hash);
	if (result == ITER_DONE)
		result = def;
	return result;
}

#define DeeKwArgs_GetItemNRStringDef(self, name, def) \
	DeeKwArgs_GetItemNRStringHashDef(self, name, Dee_HashStr(name), def)
#define DeeKwArgs_GetItemNRStringLenDef(self, name, namelen, def) \
	DeeKwArgs_GetItemNRStringLenHashDef(self, name, namelen, Dee_HashPtr(name, namelen), def)


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
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL DeeArg_GetKwNRStringLenHash(size_t argc, DeeObject *const *argv, DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL DeeArg_TryGetKwNR(size_t argc, DeeObject *const *argv, DeeObject *kw, /*string*/ DeeObject *__restrict name);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL DeeArg_TryGetKwNRStringHash(size_t argc, DeeObject *const *argv, DeeObject *kw, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4)) DeeObject *DCALL DeeArg_TryGetKwNRStringLenHash(size_t argc, DeeObject *const *argv, DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash);
#define DeeArg_GetKwNRString(argc, argv, kw, name)                DeeArg_GetKwNRStringHash(argc, argv, kw, name, Dee_HashStr(name))
#define DeeArg_GetKwNRStringLen(argc, argv, kw, name, namelen)    DeeArg_GetKwNRStringLenHash(argc, argv, kw, name, namelen, Dee_HashPtr(name, namelen))
#define DeeArg_TryGetKwNRString(argc, argv, kw, name)             DeeArg_TryGetKwNRStringHash(argc, argv, kw, name, Dee_HashStr(name))
#define DeeArg_TryGetKwNRStringLen(argc, argv, kw, name, namelen) DeeArg_TryGetKwNRStringLenHash(argc, argv, kw, name, namelen, Dee_HashPtr(name, namelen))


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
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeKw_WrapInherited(/*inherit(on_success)*/ DREF DeeObject *__restrict kwds);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeKw_ForceWrap(DeeObject *__restrict kwds);

/* Lookup keyword arguments. These functions may be used to extract keyword arguments
 * when the caller knows that `kw != NULL && DeeObject_IsKw(kw) && !DeeKwds_Check(kw)'.
 *
 * IMPORTANT: These functions to *NOT* return references! */
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_GetItemNR(DeeObject *__restrict kw, /*string*/ DeeObject *__restrict name);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_GetItemNRStringHash(DeeObject *__restrict kw, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_GetItemNRStringLenHash(DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_TryGetItemNR(DeeObject *kw, /*string*/ DeeObject *name);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_TryGetItemNRStringHash(DeeObject *kw, char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeKw_TryGetItemNRStringLenHash(DeeObject *kw, char const *__restrict name, size_t namelen, Dee_hash_t hash);
#define DeeKw_GetItemNRString(kw, name)                DeeKw_GetItemNRStringHash(kw, name, Dee_HashStr(name))
#define DeeKw_GetItemNRStringLen(kw, name, namelen)    DeeKw_GetItemNRStringLenHash(kw, name, namelen, Dee_HashPtr(name, namelen))
#define DeeKw_TryGetItemNRString(kw, name)             DeeKw_TryGetItemNRStringHash(kw, name, Dee_HashStr(name))
#define DeeKw_TryGetItemNRStringLen(kw, name, namelen) DeeKw_TryGetItemNRStringLenHash(kw, name, namelen, Dee_HashPtr(name, namelen))


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
#define DeeBlackListKwds_Check(x)      DeeObject_InstanceOfExact(x, &DeeBlackListKwds_Type) /* `_BlackListKwds' is final */
#define DeeBlackListKwds_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBlackListKwds_Type)



/* Helper functions & RT optimization bindings. */
#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKwds_IsBlackListed(DeeBlackListKwdsObject *self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKwds_IsBlackListedStringHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKwds_IsBlackListedStringLenHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_GetItemNR(DeeBlackListKwdsObject *self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_GetItemNRStringHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_GetItemNRStringLenHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_TryGetItemNR(DeeBlackListKwdsObject *self, DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_TryGetItemNRStringHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKwds_TryGetItemNRStringLenHash(DeeBlackListKwdsObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
#define DeeBlackListKwds_IsBlackListedString(self, name)             DeeBlackListKwds_IsBlackListedStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKwds_IsBlackListedStringLen(self, name, namelen) DeeBlackListKwds_IsBlackListedStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
#define DeeBlackListKwds_GetItemNRString(self, name)                 DeeBlackListKwds_GetItemNRStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKwds_GetItemNRStringLen(self, name, namelen)     DeeBlackListKwds_GetItemNRStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
#endif /* CONFIG_BUILDING_DEEMON */

/* Construct a new mapping for keywords that follows the black-listing scheme.
 * The caller must decref the returned object using `DeeBlackListKwds_Decref()'
 * -> This function is used to filter keyword arguments from varkwds when 
 *    kwargs argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListKwds { "something_else" : "foobar" }'
 *    >> foo(x: 10, something_else: "foobar"); */
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
#define DeeBlackListKw_Check(x)      DeeObject_InstanceOfExact(x, &DeeBlackListKw_Type) /* `_BlackListKw' is final */
#define DeeBlackListKw_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBlackListKw_Type)

#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKw_IsBlackListed(DeeBlackListKwObject *self, /*string*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKw_IsBlackListedStringHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeBlackListKw_IsBlackListedStringLenHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_GetItemNR(DeeBlackListKwObject *self, /*string*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_GetItemNRStringHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_GetItemNRStringLenHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_TryGetItemNR(DeeBlackListKwObject *self, /*string*/ DeeObject *name);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_TryGetItemNRStringHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeBlackListKw_TryGetItemNRStringLenHash(DeeBlackListKwObject *__restrict self, char const *__restrict name, size_t namelen, Dee_hash_t hash);
#define DeeBlackListKw_IsBlackListedString(self, name)             DeeBlackListKw_IsBlackListedStringHash(self, name, Dee_HashStr(name))
#define DeeBlackListKw_IsBlackListedStringLen(self, name, namelen) DeeBlackListKw_IsBlackListedStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
#endif /* CONFIG_BUILDING_DEEMON */


/* Construct a new mapping for a general-purpose mapping that follows the black-listing scheme.
 * -> The returned objects can be used for any kind of mapping, such that in the
 *    case of kwmappings, `DeeBlackListKw_New(code, DeeKwdsMapping_New(kwds, argv))'
 *    would produce the semantically equivalent of `DeeBlackListKwds_New(code, kwds, argv)'
 * -> This function is used to filter keyword arguments from varkwds when the general
 *    purpose keyword argument protocol is used:
 *    >> function foo(x, y?, **kwds) {
 *    >>     print type kwds, repr kwds;
 *    >> }
 *    >> // Prints `_BlackListKw { "something_else" : "foobar" }'
 *    >> foo(**{ "x" : 10, "something_else" : "foobar" }); */
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
