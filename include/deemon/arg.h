/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_ARG_H
#define GUARD_DEEMON_ARG_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif

DECL_BEGIN


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
 * >> DREF DeeObject *DCALL
 * >> my_function(DeeObject *__restrict UNUSED(self),
 * >>             size_t argc, DeeObject **__restrict argv) {
 * >>     int a,b,c = 5;
 * >>     if (DeeArg_Unpack(argc,argv,"dd|d:my_function",&a,&b,&c))
 * >>         return NULL;
 * >>     return DeeInt_NewInt(a+b+c);
 * >> }
 */
DFUNDEF int DeeArg_Unpack(size_t argc, DeeObject **__restrict argv, char const *__restrict format, ...);
DFUNDEF int DCALL DeeArg_VUnpack(size_t argc, DeeObject **__restrict argv, char const *__restrict format, va_list args);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeArg_Unpack(argc,argv,...)            __builtin_expect(DeeArg_Unpack(argc,argv,__VA_ARGS__),0)
#define DeeArg_VUnpack(argc,argv,format,args)   __builtin_expect(DeeArg_VUnpack(argc,argv,format,args),0)
#endif /* !__NO_builtin_expect */
#endif





struct kwds_entry {
    DREF struct string_object *ke_name;  /* [1..1][SENTINAL(NULL)] Keyword name. */
    dhash_t                    ke_hash;  /* [== hash_str(ke_name)][valid_if(ke_name)] Hash of this keyword. */
    size_t                     ke_index; /* [< kw_size:][valid_if(ke_name)]
                                          * Argument vector index of this keyword.
                                          * NOTE: This index is applied as an offset _after_ positional
                                          *       arguments, meaning that `0' is the first non-positional
                                          *       argument, aka. `(argc - :kw_size) + 0' */
};

typedef struct kwds_object DeeKwdsObject;
struct kwds_object {
    /* This type of object is used by user-code to
     * re-map the designation of optional arguments.
     * When given, unpacking the an argument list will
     * look at the last couple of names of arguments and
     * perform a lookup of them within this object:
     * Usercode:
     * >> foo(10,20,func: operator +);
     * ASM:
     * >> push  $10
     * >> push  $20
     * >> push  extern @operators:@__pooad
     * >> call  extern @...:@foo, #3, const @{ func: 0 }
     * foo (in C):
     * >> PRIVATE DREF DeeObject *DCALL
     * >> foo(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
     * >>     DeeObject *x,*y,*func = Dee_None;
     * >>     PRIVATE struct keyword kwlist[] = { K(x), K(y), K(func), KEND };
     * >>     if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"oo|o:foo",&x,&y,&func))
     * >>         return -1;
     * >>     return DeeObject_CallPack(func,2,x,y);
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
     *          those keywords. - It's actually {(string,int)...}-like
     *          However, you can easily construct a {(string,object)...}-like
     *          mapping by calling `DeeKwdsMapping_New()' (see below) */
    OBJECT_HEAD
    size_t            kw_size;      /* [const] The number of valid entries in `kw_map'. */
    size_t            kw_mask;      /* [const] Mask for keyword names. */
    struct kwds_entry kw_map[1024]; /* [const] Keyword name->index map. */
};
#define DeeKwds_MAPNEXT(i,perturb) ((i) = (((i) << 2) + (i) + (perturb) + 1),(perturb) >>= 5)

DDATDEF DeeTypeObject DeeKwds_Type;
#define DeeKwds_SIZE(ob)       ((DeeKwdsObject *)REQUIRES_OBJECT(ob))->kw_size
#define DeeKwds_Check(ob)      DeeObject_InstanceOfExact(ob,&DeeKwds_Type) /* _kwds is final */
#define DeeKwds_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeKwds_Type)


#ifdef CONFIG_BUILDING_DEEMON
INTDEF DREF DeeObject *DCALL DeeKwds_NewWithHint(size_t num_items);
/* Append a new entry for `name'.
 * NOTE: The keywords argument index is set to the old number of
 *       keywords that had already been defined previously. */
INTDEF int (DCALL DeeKwds_Append)(DREF DeeObject **__restrict pself,
                                  char const *__restrict name,
                                  size_t name_len, dhash_t hash);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeKwds_Append(pself,name,name_len,hash) __builtin_expect(DeeKwds_Append(pself,name,name_len,hash),0)
#endif /* !__NO_builtin_expect */
#endif
#endif


typedef struct kwds_mapping_object DeeKwdsMappingObject;
struct kwds_mapping_object {
    /* A SharedMapping-like proxy object that can be used to wrap
     * an argument list vector alongside a KwdsObject in order to
     * construct a mapping-like wrapper for keyword arguments:
     * >> DREF DeeObject *DCALL
     * >> foo(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
     * >>     DREF DeeObject *result;
     * >>     if (kw) {
     * >>         if (DeeKwds_Check(kw)) {
     * >>             size_t num_keywords;
     * >>             num_keywords = DeeKwds_SIZE(kw);
     * >>             if (num_keywords > argc) {
     * >>                 // Argument list is too short of the given keywords
     * >>                 DeeError_Throwf(&DeeError_TypeError,...);
     * >>                 return NULL;
     * >>             }
     * >>             argc -= num_keywords;
     * >>             // Turn keywords and arguments into a proper mapping-like object.
     * >>             kw = DeeKwdsMapping_New(kw,argv + argc);
     * >>             if unlikely(!kw) return NULL;
     * >>         } else {
     * >>             // The given `kw' already is a mapping-
     * >>             // like object for named arguments.
     * >>             Dee_Incref(kw);
     * >>         }
     * >>     } else {
     * >>         kw = Dee_EmptyMapping;
     * >>         Dee_Incref(kw);
     * >>     }
     * >>     result = bar(argc,argv,kw);
     * >>     if (DeeKwdsMapping_Check(kw) &&
     * >>       ((DeeKwdsMappingObject *)kw)->kmo_argv == argv + argc) {
     * >>         // If we're the ones owning the keywords-mapping, we must also decref() it.
     * >>         DeeKwdsMapping_Decref(kw);
     * >>     } else {
     * >>         Dee_Decref(kw);
     * >>     }
     * >>     return result;
     * >> }
     * NOTE: The construction of a wrapper as used above can be automated
     *       by calling `DREF DeeObject *kw = DeeArg_GetKw(&argc,argv,kw)',
     *       with the cleanup then being implemented by `DeeArg_PutKw(argc,argv,kw)'
     * >> DREF DeeObject *DCALL
     * >> foo(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
     * >>     DREF DeeObject *result;
     * >>     kw = DeeArg_GetKw(&argc,argv,kw);
     * >>     if unlikely(!kw) return NULL;
     * >>     result = bar(argc,argv,kw);
     * >>     DeeArg_PutKw(argc,argv,kw);
     * >>     return result;
     * >> }
     */
    OBJECT_HEAD
    DREF DeeKwdsObject *kmo_kwds; /* [1..1][const] The Keyword arguments.
                                   * NOTE: If revived during unsharing, the object in this field
                                   *       gets incref'd, meaning that before that point, this
                                   *       field doesn't actually carry a reference. */
    DREF DeeObject    **kmo_argv; /* [1..1][const][0..kmo_kwds->kw_size][lock(kmo_lock)][owned] Vector of
                                   * argument objects. (shared until `DeeKwdsMapping_Decref()' is called)
                                   * NOTE: May be NULL, even when `kmo_kwds->kw_size' is non-zero, in which
                                   *       case code should operate as though `kmo_kwds->kw_size' was zero. */
#ifndef CONFIG_NO_THREADS
    rwlock_t            kmo_lock; /* Lock for unsharing the argument vector. */
#endif
};

DDATDEF DeeTypeObject DeeKwdsMapping_Type;
#define DeeKwdsMapping_Check(ob)      DeeObject_InstanceOfExact(ob,&DeeKwdsMapping_Type) /* `_KwdsMapping' is final */
#define DeeKwdsMapping_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeKwdsMapping_Type)

/* Construct a keywords-mapping object from a given `kwds' object,
 * as well as an argument vector that will be shared with the mapping.
 * The returned object then a mapping {(string,object)...} for the
 * actual argument values passed to the function.
 * NOTE: The caller must later invoke `DeeKwdsMapping_Decref()' in order
 *       to clean up the returned object. */
DFUNDEF DREF DeeObject *DCALL
DeeKwdsMapping_New(/*Kwds*/DeeObject *__restrict kwds,
                   DeeObject **__restrict argv);

/* Unshare the argument vector from a keywords-mapping object, automatically
 * constructing a copy if all contained objects if `self' is being shared,
 * or destroying `self' without touching the argument vector if not. */
DFUNDEF void DCALL DeeKwdsMapping_Decref(DREF DeeObject *__restrict self);

#ifdef CONFIG_BUILDING_DEEMON
INTDEF bool DCALL DeeKwdsMapping_HasItemString(DeeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF bool DCALL DeeKwdsMapping_HasItemStringLen(DeeObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF DREF DeeObject *DCALL DeeKwdsMapping_GetItemString(DeeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF DREF DeeObject *DCALL DeeKwdsMapping_GetItemStringDef(DeeObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict def);
INTDEF DREF DeeObject *DCALL DeeKwdsMapping_GetItemStringLen(DeeObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash);
INTDEF DREF DeeObject *DCALL DeeKwdsMapping_GetItemStringLenDef(DeeObject *__restrict self, char const *__restrict name, size_t namesize, dhash_t hash, DeeObject *__restrict def);
#endif


/* Construct/access keyword arguments passed to a function as a
 * high-level {(string,object)...}-like mapping that is bound to
 * the actually mapped arguments. */
DFUNDEF DREF DeeObject *DCALL DeeArg_GetKw(size_t *__restrict pargc, DeeObject **__restrict argv, DeeObject *kw);
DFUNDEF void DCALL DeeArg_PutKw(size_t argc, DeeObject **__restrict argv, DeeObject *__restrict kw);

/* In a keyword-enabled function, return the argument associated with a given
 * `name', or throw a TypeError exception or return `def' if not provided. */
DFUNDEF DREF DeeObject *DCALL
DeeArg_GetKwString(size_t argc, DeeObject **__restrict argv, DeeObject *kw,
                   char const *__restrict name);
DFUNDEF DREF DeeObject *DCALL
DeeArg_GetKwStringLen(size_t argc, DeeObject **__restrict argv, DeeObject *kw,
                      char const *__restrict name, size_t namelen, dhash_t hash);
DFUNDEF DREF DeeObject *DCALL
DeeArg_GetKwStringDef(size_t argc, DeeObject **__restrict argv,
                      DeeObject *kw, char const *__restrict name,
                      DeeObject *__restrict def);
DFUNDEF DREF DeeObject *DCALL
DeeArg_GetKwStringLenDef(size_t argc, DeeObject **__restrict argv,
                         DeeObject *kw, char const *__restrict name,
                         size_t namelen, dhash_t hash,
                         DeeObject *__restrict def);



struct keyword {
    char const *k_name; /* [1..1][SENTINAL(NULL)] Keyword name. */
    dhash_t     k_hash; /* [== hash_str(ke_name)]
                         * Hash of this keyword (or (dhash_t)-1 when not yet calculated).
                         * Filled in the first time the keyword is used. */
};
#define K(x)  { #x, (dhash_t)-1 }
#define KS(s) { s, (dhash_t)-1 }
#define KEND  { NULL }
#define DEFINE_KWLIST(name,...) struct keyword name[] = __VA_ARGS__


/* Same as the regular unpack functions above, however these are enabled to
 * support keyword lists in the event that the calling function has been
 * provided with a keyword object (`kw').
 * -> When `DeeKwds_Check(kw)' is true, keyword argument objects are passed
 *    through the regular argument vector, located within the range
 *   `argc - kw->kw_size .. argc - 1' (if `kw->kw_size > argc', a TypeError is thrown),
 *    using names from `kwlist + NUM_POSITIONAL' to match association.
 * -> Otherwise, positional arguments are also parsed regularly, before
 *    using `DeeObject_GetItemString()' to lookup argument names starting
 *    at `kwlist + NUM_POSITIONAL', counting how may arguments were actually
 *    found (and failing if a non-optional argument wasn't given), before
 *    finally using `DeeObject_Size()' to see how many keyword-arguments
 *    were given by the keyword-list, and throwing an error if more were
 *    given than what was actually used. */
DFUNDEF int DeeArg_UnpackKw(size_t argc, DeeObject **__restrict argv,
                            DeeObject *kw, struct keyword *__restrict kwlist,
                            char const *__restrict format, ...);
DFUNDEF int DCALL DeeArg_VUnpackKw(size_t argc, DeeObject **__restrict argv,
                                   DeeObject *kw, struct keyword *__restrict kwlist,
                                   char const *__restrict format, va_list args);

/* Same as `DeeArg_UnpackKw', but don't throw errors if argument types,
 * or argument counts don't match (which would otherwise throw a TypeError),
 * and return `1' instead.
 * @return:  1: Unpacking failed (argument list and keyword don't match requirements)
 * @return:  0: Unpacking was successful.
 * @return: -1: An error occurred. */
DFUNDEF int DeeArg_TryUnpackKw(size_t argc, DeeObject **__restrict argv,
                               DeeObject *kw, struct keyword *__restrict kwlist,
                               char const *__restrict format, ...);
DFUNDEF int DCALL DeeArg_VTryUnpackKw(size_t argc, DeeObject **__restrict argv,
                                      DeeObject *kw, struct keyword *__restrict kwlist,
                                      char const *__restrict format, va_list args);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeArg_UnpackKw(argc,argv,kw,kwlist,...)            __builtin_expect(DeeArg_UnpackKw(argc,argv,kw,kwlist,__VA_ARGS__),0)
#define DeeArg_VUnpackKw(argc,argv,kw,kwlist,format,args)   __builtin_expect(DeeArg_VUnpackKw(argc,argv,kw,kwlist,format,args),0)
#endif /* !__NO_builtin_expect */
#endif


DECL_END


#endif /* !GUARD_DEEMON_ARG_H */
