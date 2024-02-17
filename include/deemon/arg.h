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
#ifndef GUARD_DEEMON_ARG_H
#define GUARD_DEEMON_ARG_H 1

#include "api.h"

#include "object.h"
#include "util/lock.h"

DECL_BEGIN


#ifdef DEE_SOURCE
#define dee_kwds_entry          kwds_entry
#define Dee_string_object       string_object
#define dee_kwds_object         kwds_object
#define dee_kwds_mapping_object kwds_mapping_object
#define dee_kwargs              kwds_args
#define dee_keyword             keyword
#define DEFINE_KWDS             Dee_DEFINE_KWDS
#define K                       Dee_KEYWORD
#define KS                      Dee_KEYWORD_STR
#define KEX                     Dee_KEYWORD_EX
#define KEND                    Dee_KEYWORD_END
#define DEFINE_KWLIST           Dee_DEFINE_KWLIST
#endif /* DEE_SOURCE */



/* An extension to `Dee_Unpackf', explicitly for unpacking elements from function arguments.
 * Format language syntax:
 *     using Dee_Unpackf::object;
 *     __main__   ::= [(object  // Process regular objects, writing values to pointers passed through varargs.
 *                    | '|'     // Marker: The remainder of the format is optional.
 *                      )...]
 *                    [':' <function_name>] // Optional, trailing function name (Used in error messages)
 *     ;
 * Example usage:
 * >> // function my_function(int a, int b, int c = 5) -> int;
 * >> // @return: * : The sum of `a', `b' and `c'
 * >> PRIVATE WUNUSED ATTR_INS(2, 1) NONNULL((1)) DREF DeeObject *DCALL
 * >> my_function(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
 * >>     int a, b, c = 5;
 * >>     if (DeeArg_Unpack(argc, argv, "dd|d:my_function", &a, &b, &c))
 * >>         return NULL;
 * >>     return DeeInt_NewInt(a + b + c);
 * >> }
 */
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((3)) int
DeeArg_Unpack(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv,
              char const *__restrict format, ...);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((3)) int DCALL
DeeArg_VUnpack(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv,
               char const *__restrict format, va_list args);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeArg_Unpack(argc, argv, ...)           __builtin_expect(DeeArg_Unpack(argc, argv, __VA_ARGS__), 0)
#define DeeArg_VUnpack(argc, argv, format, args) __builtin_expect(DeeArg_VUnpack(argc, argv, format, args), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */



struct Dee_string_object;
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


typedef struct dee_kwds_mapping_object DeeKwdsMappingObject;
struct dee_kwds_mapping_object {
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
	 * >>	    DeeKwdsMapping_GetArgv(kw) == argv + argc) {
	 * >>		// If we're the ones owning the keywords-mapping, we must also decref() it.
	 * >>		DeeKwdsMapping_Decref(kw);
	 * >>	} else {
	 * >>		Dee_Decref(kw);
	 * >>	}
	 * >>	return result;
	 * >> }
	 * NOTE: The construction of a wrapper as used above can be automated
	 *       by calling `DREF DeeObject *kw = DeeArg_GetKw(&argc, argv, kw)',
	 *       with the cleanup then being implemented by `DeeArg_PutKw(argc, argv, kw)'
	 * >> DREF DeeObject *DCALL
	 * >> foo(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	 * >> 	DREF DeeObject *result;
	 * >> 	kw = DeeArg_GetKw(&argc, argv, kw);
	 * >> 	if unlikely(!kw)
	 * >> 		return NULL;
	 * >> 	result = bar(argc, argv, kw);
	 * >> 	DeeArg_PutKw(argc, argv, kw);
	 * >> 	return result;
	 * >> }
	 */
	Dee_OBJECT_HEAD
	DREF DeeKwdsObject *kmo_kwds; /* [1..1][const] The Keyword arguments.
	                               * NOTE: If revived during unsharing, the object in this field
	                               *       gets incref'd, meaning that before that point, this
	                               *       field doesn't actually carry a reference. */
	DREF DeeObject    **kmo_argv; /* [1..1][const][0..kmo_kwds->kw_size][lock(kmo_lock)][owned] Vector of
	                               * argument objects. (shared until `DeeKwdsMapping_Decref()' is called)
	                               * NOTE: May be NULL, even when `kmo_kwds->kw_size' is non-zero, in which
	                               *       case code should operate as though `kmo_kwds->kw_size' was zero. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t kmo_lock; /* Lock for unsharing the argument vector. */
#endif /* !CONFIG_NO_THREADS */
};

DDATDEF DeeTypeObject DeeKwdsMapping_Type;
#define DeeKwdsMapping_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeKwdsMapping_Type) /* `_KwdsMapping' is final */
#define DeeKwdsMapping_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeKwdsMapping_Type)

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

#define DeeKwdsMapping_GetKwds(ob) (((DeeKwdsMappingObject *)Dee_REQUIRES_OBJECT(ob))->kmo_kwds)
#define DeeKwdsMapping_GetArgv(ob) (((DeeKwdsMappingObject *)Dee_REQUIRES_OBJECT(ob))->kmo_argv)

/* Construct a keywords-mapping object from a given `kwds' object,
 * as well as an argument vector that will be shared with the mapping.
 * The returned object then a mapping {(string, Object)...} for the
 * actual argument values passed to the function.
 * NOTE: The caller must later invoke `DeeKwdsMapping_Decref()' in order
 *       to clean up the returned object. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwdsMapping_New(/*Kwds*/ DeeObject *kwds,
                   DeeObject *const *argv);

/* Unshare the argument vector from a keywords-mapping object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
DFUNDEF NONNULL((1)) void DCALL DeeKwdsMapping_Decref(DREF DeeObject *__restrict self);

#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeKwdsMapping_HasItemStringHash(DeeObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL DeeKwdsMapping_HasItemStringLenHash(DeeObject *__restrict self, char const *__restrict name, size_t namesize, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeKwdsMapping_GetItemStringHash(DeeObject *__restrict self, char const *__restrict name, Dee_hash_t hash);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DREF DeeObject *DCALL DeeKwdsMapping_GetItemStringLenHash(DeeObject *__restrict self, char const *__restrict name, size_t namesize, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL DeeKwdsMapping_GetItemStringHashDef(DeeObject *self, char const *__restrict name, Dee_hash_t hash, DeeObject *def);
INTDEF WUNUSED ATTR_INS(2, 3) NONNULL((1, 5)) DREF DeeObject *DCALL DeeKwdsMapping_GetItemStringLenHashDef(DeeObject *self, char const *__restrict name, size_t namesize, Dee_hash_t hash, DeeObject *def);
#define DeeKwdsMapping_HasItemString(self, name)                      DeeKwdsMapping_HasItemStringHash(self, name, Dee_HashStr(name))
#define DeeKwdsMapping_HasItemStringLen(self, name, namesize)         DeeKwdsMapping_HasItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeKwdsMapping_GetItemString(self, name)                      DeeKwdsMapping_GetItemStringHash(self, name, Dee_HashStr(name))
#define DeeKwdsMapping_GetItemStringDef(self, name, def)              DeeKwdsMapping_GetItemStringHashDef(self, name, Dee_HashStr(name), def)
#define DeeKwdsMapping_GetItemStringLen(self, name, namesize)         DeeKwdsMapping_GetItemStringLenHash(self, name, namesize, Dee_HashPtr(name, namesize))
#define DeeKwdsMapping_GetItemStringLenDef(self, name, namesize, def) DeeKwdsMapping_GetItemStringLenHashDef(self, name, namesize, Dee_HashPtr(name, namesize), def)
#endif /* CONFIG_BUILDING_DEEMON */

typedef struct dee_kwargs DeeKwArgs;
struct dee_kwargs {
	size_t            kwa_kwused; /* # of used keyword arguments (assuming that any argument is loaded 1 time at most) */
	DeeObject *const *kwa_kwargv; /* [0..1] Positional arguments to supplement `kwa_kw' (or NULL if unused). */
	DeeObject        *kwa_kw;     /* [0..1] Keyword arguments descriptor / mapping. */
};

/* Check if there *may* still be more keyword arguments available in `self'. */
#define DeeKwArgs_MaybeHaveMoreArgs(self) \
	((self)->kwa_kw && (!DeeKwds_Check((self)->kwa_kw) || ((self)->kwa_kwused < DeeKwds_SIZE((self)->kwa_kw))))

/* Initialize `self' to load keyword arguments.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeKwArgs_Init(DeeKwArgs *__restrict self, size_t *__restrict p_argc,
               DeeObject *const *argv, DeeObject *kw);

/* Indicate that you're doing loading arguments from `self'.
 * This function asserts that `kwa_kwused == #kwa_kw' so-as
 * to ensure that `kwa_kw' doesn't contain any unused keyword
 * arguments.
 * @param: positional_argc: The value of `*p_argc' after `DeeKwArgs_Init()' returned.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeKwArgs_Done(DeeKwArgs *__restrict self,
               size_t positional_argc,
               char const *function_name);

/* Lookup a named keyword argument from `self'
 * @return: * :   Reference to named keyword argument.
 * @return: NULL: An error was thrown.*/
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwArgs_GetStringHash(DeeKwArgs *__restrict self,
                        char const *__restrict name,
                        Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 3) NONNULL((1)) DREF DeeObject *DCALL
DeeKwArgs_GetStringLenHash(DeeKwArgs *__restrict self,
                           char const *__restrict name,
                           size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeKwArgs_GetStringHashDef(DeeKwArgs *__restrict self,
                           char const *__restrict name,
                           Dee_hash_t hash, DeeObject *def);
DFUNDEF WUNUSED ATTR_INS(2, 3) NONNULL((1, 5)) DREF DeeObject *DCALL
DeeKwArgs_GetStringLenHashDef(DeeKwArgs *__restrict self,
                              char const *__restrict name,
                              size_t namelen, Dee_hash_t hash,
                              DeeObject *def);
#define DeeKwArgs_GetString(self, name)                     DeeKwArgs_GetStringHash(self, name, Dee_HashStr(name))
#define DeeKwArgs_GetStringLen(self, name, namelen)         DeeKwArgs_GetStringLenHash(self, name, namelen, Dee_HashPtr(name, namelen))
#define DeeKwArgs_GetStringDef(self, name, def)             DeeKwArgs_GetStringHashDef(self, name, Dee_HashStr(name), def)
#define DeeKwArgs_GetStringLenDef(self, name, namelen, def) DeeKwArgs_GetStringLenHashDef(self, name, namelen, Dee_HashPtr(name, namelen), def)



/* Construct/access keyword arguments passed to a function as a
 * high-level {string: Object}-like mapping that is bound to the
 * actually mapped arguments. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeArg_GetKw(size_t *__restrict p_argc, DeeObject *const *argv, DeeObject *kw);
DFUNDEF ATTR_INS(2, 1) NONNULL((3)) void DCALL
DeeArg_PutKw(size_t argc, DeeObject *const *argv, DREF DeeObject *kw);

/* In a keyword-enabled function, return the argument associated with a given
 * `name', or throw a TypeError exception or return `def' if not provided. */
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4)) DREF DeeObject *DCALL
DeeArg_GetKwStringHash(size_t argc, DeeObject *const *argv, DeeObject *kw,
                       char const *__restrict name, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 1) ATTR_INS(4, 5) DREF DeeObject *DCALL
DeeArg_GetKwStringLenHash(size_t argc, DeeObject *const *argv, DeeObject *kw,
                          char const *__restrict name, size_t namelen, Dee_hash_t hash);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 6)) DREF DeeObject *DCALL
DeeArg_GetKwStringHashDef(size_t argc, DeeObject *const *argv,
                          DeeObject *kw, char const *__restrict name,
                          Dee_hash_t hash, DeeObject *def);
DFUNDEF WUNUSED ATTR_INS(2, 1) ATTR_INS(4, 5) NONNULL((7)) DREF DeeObject *DCALL
DeeArg_GetKwStringLenHashDef(size_t argc, DeeObject *const *argv,
                             DeeObject *kw, char const *__restrict name,
                             size_t namelen, Dee_hash_t hash, DeeObject *def);
#define DeeArg_GetKwString(argc, argv, kw, name)                     DeeArg_GetKwStringHash(argc, argv, kw, name, Dee_HashStr(name))
#define DeeArg_GetKwStringLen(argc, argv, kw, name, namelen)         DeeArg_GetKwStringLenHash(argc, argv, kw, name, namelen, Dee_HashPtr(name, namelen))
#define DeeArg_GetKwStringDef(argc, argv, kw, name, def)             DeeArg_GetKwStringHashDef(argc, argv, kw, name, Dee_HashStr(name), def)
#define DeeArg_GetKwStringLenDef(argc, argv, kw, name, namelen, def) DeeArg_GetKwStringLenHashDef(argc, argv, kw, name, namelen, Dee_HashPtr(name, namelen), def)



struct dee_keyword {
	char const *k_name; /* [1..1][SENTINAL(NULL)] Keyword name. */
	Dee_hash_t  k_hash; /* [== Dee_HashStr(ke_name)]
	                     * Hash of this keyword (or (Dee_hash_t)-1 when not yet calculated).
	                     * Filled in the first time the keyword is used. */
};

#define Dee_KEYWORD(x)               { #x, (Dee_hash_t)-1 }
#define Dee_KEYWORD_STR(s)           { s, (Dee_hash_t)-1 }
#define Dee_KEYWORD_EX(s, h32, h64)  { s, _Dee_HashSelectC(h32, h64) }
#define Dee_KEYWORD_END              { NULL }
#define Dee_DEFINE_KWLIST(name, ...) struct dee_keyword name[] = __VA_ARGS__

/* Same as the regular unpack functions above, however these are enabled to
 * support keyword lists in the event that the calling function has been
 * provided with a keyword object (`kw').
 * -> When `DeeKwds_Check(kw)' is true, keyword argument objects are passed
 *    through the regular argument vector, located within the range
 *   `argc - kw->kw_size .. argc - 1' (if `kw->kw_size > argc', a TypeError is thrown),
 *    using names from `kwlist + NUM_POSITIONAL' to match association.
 * -> Otherwise, positional arguments are also parsed regularly, before
 *    using `DeeObject_GetItemStringHash()' to lookup argument names starting
 *    at `kwlist + NUM_POSITIONAL', counting how may arguments were actually
 *    found (and failing if a non-optional argument wasn't given), before
 *    finally using `DeeObject_Size()' to see how many keyword-arguments
 *    were given by the keyword-list, and throwing an error if more were
 *    given than what was actually used. */
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 5)) int
DeeArg_UnpackKw(size_t argc, DeeObject *const *argv,
                DeeObject *kw, struct dee_keyword *__restrict kwlist,
                char const *__restrict format, ...);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 5)) int DCALL
DeeArg_VUnpackKw(size_t argc, DeeObject *const *argv,
                 DeeObject *kw, struct dee_keyword *__restrict kwlist,
                 char const *__restrict format, va_list args);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeKwArgs_Init(self, p_argc, argv, kw)                 __builtin_expect(DeeKwArgs_Init(self, p_argc, argv, kw), 0)
#define DeeKwArgs_Done(self, positional_argc, function_name)   __builtin_expect(DeeKwArgs_Done(self, positional_argc, function_name), 0)
#define DeeArg_UnpackKw(argc, argv, kw, kwlist, ...)           __builtin_expect(DeeArg_UnpackKw(argc, argv, kw, kwlist, __VA_ARGS__), 0)
#define DeeArg_VUnpackKw(argc, argv, kw, kwlist, format, args) __builtin_expect(DeeArg_VUnpackKw(argc, argv, kw, kwlist, format, args), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */



/* Optimized format sequences for `Dee_Unpackf()' and `DeeArg_Unpack[Kw]()' */
#define DEE_UNPu8   "I8u"
#define DEE_UNPd8   "I8d"
#define DEE_UNPu16  "I16u"
#define DEE_UNPd16  "I16d"
#define DEE_UNPu32  "I32u"
#define DEE_UNPd32  "I32d"
#define DEE_UNPu64  "I64u"
#define DEE_UNPd64  "I64d"
#define DEE_UNPu128 "I128u"
#define DEE_UNPd128 "I128d"
#ifdef __SIZEOF_LONG_LONG__
#if __SIZEOF_LONG_LONG__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "llu"
#define DEE_UNPd64 "lld"
#elif __SIZEOF_LONG_LONG__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "llu"
#define DEE_UNPd32 "lld"
#elif __SIZEOF_LONG_LONG__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "llu"
#define DEE_UNPd16 "lld"
#endif /* __SIZEOF_LONG_LONG__ == ... */
#endif /* __SIZEOF_LONG_LONG__ */
#if __SIZEOF_CHAR__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "hhu"
#define DEE_UNPd64 "hhd"
#elif __SIZEOF_CHAR__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "hhu"
#define DEE_UNPd32 "hhd"
#elif __SIZEOF_CHAR__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "hhu"
#define DEE_UNPd16 "hhd"
#endif /* __SIZEOF_CHAR__ == ... */
#if __SIZEOF_LONG__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "lu"
#define DEE_UNPd64 "ld"
#elif __SIZEOF_LONG__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "lu"
#define DEE_UNPd32 "ld"
#elif __SIZEOF_LONG__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "lu"
#define DEE_UNPd16 "ld"
#elif __SIZEOF_LONG__ == 1
#undef DEE_UNPu8
#undef DEE_UNPd8
#define DEE_UNPu8 "lu"
#define DEE_UNPd8 "ld"
#endif /* __SIZEOF_LONG__ == ... */
#if __SIZEOF_SHORT__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "hu"
#define DEE_UNPd64 "hd"
#elif __SIZEOF_SHORT__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "hu"
#define DEE_UNPd32 "hd"
#elif __SIZEOF_SHORT__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "hu"
#define DEE_UNPd16 "hd"
#elif __SIZEOF_SHORT__ == 1
#undef DEE_UNPu8
#undef DEE_UNPd8
#define DEE_UNPu8 "hu"
#define DEE_UNPd8 "hd"
#endif /* __SIZEOF_SHORT__ == ... */
#if __SIZEOF_INT__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "u"
#define DEE_UNPd64 "d"
#elif __SIZEOF_INT__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "u"
#define DEE_UNPd32 "d"
#elif __SIZEOF_INT__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "u"
#define DEE_UNPd16 "d"
#elif __SIZEOF_INT__ == 1
#undef DEE_UNPu8
#undef DEE_UNPd8
#define DEE_UNPu8 "u"
#define DEE_UNPd8 "d"
#endif /* __SIZEOF_INT__ == ... */
#define DEE_PRIVATE_UNPu1         DEE_UNPu8
#define DEE_PRIVATE_UNPd1         DEE_UNPd8
#define DEE_PRIVATE_UNPu2         DEE_UNPu16
#define DEE_PRIVATE_UNPd2         DEE_UNPd16
#define DEE_PRIVATE_UNPu4         DEE_UNPu32
#define DEE_PRIVATE_UNPd4         DEE_UNPd32
#define DEE_PRIVATE_UNPu8         DEE_UNPu64
#define DEE_PRIVATE_UNPd8         DEE_UNPd64
#define DEE_PRIVATE_UNPuN(sizeof) DEE_PRIVATE_UNPu##sizeof
#define DEE_PRIVATE_UNPdN(sizeof) DEE_PRIVATE_UNPd##sizeof
#define DEE_UNPuN(sizeof)         DEE_PRIVATE_UNPuN(sizeof)
#define DEE_UNPdN(sizeof)         DEE_PRIVATE_UNPdN(sizeof)

/* Helpful aliases */
#define DEE_UNPuSIZ DEE_UNPuN(__SIZEOF_SIZE_T__)
#define DEE_UNPdSIZ DEE_UNPdN(__SIZEOF_SIZE_T__)
#define DEE_UNPuPTR DEE_UNPuN(__SIZEOF_POINTER__)
#define DEE_UNPdPTR DEE_UNPdN(__SIZEOF_POINTER__)
#define DEE_UNPuB   DEE_UNPu8
#define DEE_UNPdB   DEE_UNPd8

/* Unescaped names. */
#ifdef DEE_SOURCE
#define UNPuB   DEE_UNPuB
#define UNPdB   DEE_UNPdB
#define UNPu8   DEE_UNPu8
#define UNPd8   DEE_UNPd8
#define UNPu16  DEE_UNPu16
#define UNPd16  DEE_UNPd16
#define UNPu32  DEE_UNPu32
#define UNPd32  DEE_UNPd32
#define UNPu64  DEE_UNPu64
#define UNPd64  DEE_UNPd64
#define UNPu128 DEE_UNPu128
#define UNPd128 DEE_UNPd128
#define UNPuN   DEE_UNPuN
#define UNPdN   DEE_UNPdN
#define UNPuSIZ DEE_UNPuSIZ
#define UNPdSIZ DEE_UNPdSIZ
#define UNPuPTR DEE_UNPuPTR
#define UNPdPTR DEE_UNPdPTR
#endif /* DEE_SOURCE */

DECL_END

#endif /* !GUARD_DEEMON_ARG_H */
