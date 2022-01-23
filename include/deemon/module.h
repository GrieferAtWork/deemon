/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_MODULE_H
#define GUARD_DEEMON_MODULE_H 1

#include "api.h"

#include <hybrid/typecore.h>

#include <stdbool.h>
#include <stddef.h>

#include "object.h"
#include "util/lock.h"

#ifdef CONFIG_BUILDING_DEEMON
#include "gc.h"
#endif /* CONFIG_BUILDING_DEEMON */

#ifdef GUARD_DEEMON_EXECUTE_MODPATH_C
#include "list.h"
#endif /* GUARD_DEEMON_EXECUTE_MODPATH_C */

DECL_BEGIN

/*
 * System module paths:
 *   - import util;
 * Search for the following files in all library paths:
 *   - ${LIBPATH}/.util.dec
 *   - ${LIBPATH}/util.dee
 *   - ${LIBPATH}/util.dll / ${LIBPATH}/util.so
 * System module path with indirection:
 *   - import sys.types;
 * Search for the following files in all library paths:
 *   - ${LIBPATH}/sys/.types.dec
 *   - ${LIBPATH}/sys/types.dee
 *   - ${LIBPATH}/sys/types.dll / ${LIBPATH}/sys/types.so
 *
 * Relative module paths:
 *   - import .util;
 * Search in the folder of the file containing the `import' directive:
 *   - ${fs.headof(__FILE__)}/.util.dec
 *   - ${fs.headof(__FILE__)}/util.dee
 *   - ${fs.headof(__FILE__)}/util.dll / ${fs.headof(__FILE__)}/util.so
 *
 * Relative module paths from parent directory:
 *   - import ..parent_module;
 * Search in the parent folder of the folder containing
 * the file containing the `import' directive:
 *   - ${fs.headof(__FILE__)}/../.util.dec
 *   - ${fs.headof(__FILE__)}/../util.dee
 *   - ${fs.headof(__FILE__)}/../util.dll / ${fs.headof(__FILE__)}/../util.so
 *
 * Relative module paths from parent directory with indirection:
 *   - import ..sys.types;
 * Search in the parent folder of the folder containing
 * the file containing the `import' directive:
 *   - ${fs.headof(__FILE__)}/../sys/.types.dec
 *   - ${fs.headof(__FILE__)}/../sys/types.dee
 *   - ${fs.headof(__FILE__)}/../sys/types.dll / ${fs.headof(__FILE__)}/../sys/types.so
 *
 *
 * Note that in the event of relative paths and system paths overlapping,
 * a module that could then be addressed with 2 different names must still
 * refer to the same module and must only be opened once:
 *
 * ${LIBPATH}/foo.dee:
 * >> global x = import .bar; // Import a relative module from within a library path file.
 * ${LIBPATH}/bar.dee:
 * >> global value = "Exported string from bar";
 *
 * ${TEMPDIR}/my_script.dee:
 * >> import foo; // This will load the module `bar' as a file-cached module `${LIBPATH}/bar.dee'
 * >> import bar; // This will upgrade the file-cached module `${LIBPATH}/bar.dee' to a global module `bar'.
 * >> import object from deemon;
 * >>
 * >> print foo.x.value; // `Exported string from bar'
 * >> print bar.value;   // `Exported string from bar'
 * >>
 * >> assert Object.id(foo.x) == Object.id(bar), "Must be the same object";
 * >> assert Object.id(foo.x.value) == Object.id(bar.value), "Must be the same object";
 */

#ifdef DEE_SOURCE
#define Dee_string_object         string_object
#define Dee_code_object           code_object
#define Dee_module_object         module_object
#define Dee_module_path_object    module_path_object
#define Dee_module_symbol         module_symbol
#define Dee_thread_object         thread_object
#define Dee_compiler_error_object compiler_error_object
#define Dee_compiler_options      compiler_options
#define MODSYM_FNORMAL            Dee_MODSYM_FNORMAL
#define MODSYM_FREADONLY          Dee_MODSYM_FREADONLY
#define MODSYM_FCONSTEXPR         Dee_MODSYM_FCONSTEXPR
#define MODSYM_FALIAS             Dee_MODSYM_FALIAS
#define MODSYM_FHIDDEN            Dee_MODSYM_FHIDDEN
#define MODSYM_FPROPERTY          Dee_MODSYM_FPROPERTY
#define MODSYM_FEXTERN            Dee_MODSYM_FEXTERN
#define MODSYM_FMASK              Dee_MODSYM_FMASK
#define MODSYM_FNAMEOBJ           Dee_MODSYM_FNAMEOBJ
#define MODSYM_FDOCOBJ            Dee_MODSYM_FDOCOBJ
#define MODULE_PROPERTY_GET       Dee_MODULE_PROPERTY_GET
#define MODULE_PROPERTY_DEL       Dee_MODULE_PROPERTY_DEL
#define MODULE_PROPERTY_SET       Dee_MODULE_PROPERTY_SET
#define MODULE_SYMBOL_EQUALS      Dee_MODULE_SYMBOL_EQUALS
#define MODULE_SYMBOL_GETNAMESTR  Dee_MODULE_SYMBOL_GETNAMESTR
#define MODULE_SYMBOL_GETNAMELEN  Dee_MODULE_SYMBOL_GETNAMELEN
#define MODULE_SYMBOL_GETDOCSTR   Dee_MODULE_SYMBOL_GETDOCSTR
#define MODULE_SYMBOL_GETDOCLEN   Dee_MODULE_SYMBOL_GETDOCLEN
#define MODULE_FNORMAL            Dee_MODULE_FNORMAL
#ifndef CONFIG_NO_DEC
#define MODULE_FHASCTIME          Dee_MODULE_FHASCTIME
#endif /* !CONFIG_NO_DEC */
#define MODULE_FLOADING           Dee_MODULE_FLOADING
#define MODULE_FDIDLOAD           Dee_MODULE_FDIDLOAD
#define MODULE_FINITIALIZING      Dee_MODULE_FINITIALIZING
#define MODULE_FDIDINIT           Dee_MODULE_FDIDINIT
#define MODULE_HASHST             Dee_MODULE_HASHST
#define MODULE_HASHNX             Dee_MODULE_HASHNX
#define MODULE_HASHIT             Dee_MODULE_HASHIT
#define DEC_FNORMAL               Dee_DEC_FNORMAL
#define DEC_FDISABLE              Dee_DEC_FDISABLE
#define DEC_FLOADOUTDATED         Dee_DEC_FLOADOUTDATED
#define DEC_FUNTRUSTED            Dee_DEC_FUNTRUSTED
#endif /* DEE_SOURCE */

struct Dee_string_object;
struct Dee_code_object;
typedef struct Dee_module_object DeeModuleObject;
typedef struct Dee_module_path_object DeeModulePathObject;

#define Dee_MODSYM_FNORMAL         0x0000 /* Normal symbol flags. */
#define Dee_MODSYM_FREADONLY       0x0001 /* Don't allow write-access to this symbol.
                                           * When set, attempting to write/delete to this symbol will cause
                                           * a compiler-error (except for the first assignment when the symbol
                                           * is part of the calling module), or attempting to write/delete at
                                           * runtime and a non-NULL value has already been assigned.
                                           * When this flag and `MODSYM_FCONSTEXPR' are both set, then the compiler
                                           * is allowed (but not required) to initialize the module, then propagate
                                           * this symbol's actual value as a compile-time constant expression, should
                                           * that value be one of the following (and for a sequence, containing only such):
                                           *   - DeeInt_Type
                                           *   - DeeString_Type
                                           *   - DeeNone_Type
                                           *   - DeeBool_Type
                                           *   - DeeObject_Type
                                           *   - DeeList_Type
                                           *   - DeeTuple_Type
                                           *   - DeeHashSet_Type
                                           *   - DeeDict_Type
                                           *   - DeeRoDict_Type
                                           *   - DeeRoSet_Type
                                           * NOTE: All white-listed types encode _exact_ matches
                                           *      (aka. `DeeObject_InstanceOfExact()', rather than `DeeObject_InstanceOf()')
                                           * NOTE: Some more additions are made for few more special objects
                                           *       that are not documented here, but the idea should be clear:
                                           *       Nothing that may produce side-effects in an obvious fashion,
                                           *       and nothing that is too complex, or wouldn't make sense.
                                           * WARNING: Despite all of these rules, the basic initialization that
                                           *          leads to some specific value being assigned still remains
                                           *          in the hands of user, meaning that it is the job of a module
                                           *          to make sure that exported constants be always assigned the
                                           *          same values.
                                           * WARNING: This flag cannot be enforced when user-code assembly is used
                                           *          to modify the value of an external/global symbol, meaning that
                                           *          you must still always assume that any module member no longer
                                           *          contains the proper value. */
#define Dee_MODSYM_FCONSTEXPR      0x0002 /* May be combined with `MODSYM_FREADONLY' to allow the compiler to
                                           * propagate this symbol as a constant expression at compile-time,
                                           * so-long as its runtime-value matches the criteria detailed above.
                                           * NOTE: Regardless of this flag, the compiler mustn't propagate
                                           *       the assigned value when `MODSYM_FPROPERTY' it set. */
#define Dee_MODSYM_FALIAS          0x0004 /* This symbol is aliasing another.
                                           * This flag is handled by `DeeModule_GlobalName()', which will
                                           * try to continue searching for another member with the same index,
                                           * but without this flag set.
                                           * Should such a member not exist, return one of the aliases already
                                           * encountered at random. */
#define Dee_MODSYM_FHIDDEN         0x0008 /* Don't enumerate this symbol in export listings or documentations.
                                           * The only way to access it is to know that it exists and call it by
                                           * name (or take apart the source binary to learn what may be there).
                                           * This flag is used by some hidden (and implementation-specific)
                                           * helper functions found in the `deemon' module which the compiler
                                           * is allowed to generate helper class to for stuff that doesn't
                                           * deserve its own opcode due to how rare its occurrence is... */
#define Dee_MODSYM_FPROPERTY       0x0010 /* The symbol is a property. */
#define Dee_MODSYM_FEXTERN         0x0020 /* Refers to an global variable slot for a different module (would allow for forwarding/aliasing)
                                           * Using this, we could easily implement something along the lines of
                                           * `global import foo = bar from baz;' vs. `local import foo = bar from baz;'
                                           * (Default visibility would be `local') */
#define Dee_MODSYM_FMASK           0x003f /* Mask of known MODSYM_F* flags (those that are allowed by DEC files). */
#define Dee_MODSYM_FNAMEOBJ        0x4000 /* The symbol's name is actually a reference to a string object's `s_str' */
#define Dee_MODSYM_FDOCOBJ         0x8000 /* The symbol's doc is actually a reference to a string object's `s_str'
                                           * NOTE: When this flag is set, `ss_doc' is always [1..1] */

#define Dee_MODULE_PROPERTY_GET    0 /* Index offset for property get callbacks. */
#define Dee_MODULE_PROPERTY_DEL    1 /* Index offset for property del callbacks. */
#define Dee_MODULE_PROPERTY_SET    2 /* Index offset for property set callbacks. */

struct Dee_module_symbol {
	/* For the sake of DEX modules, `ss_doc' should be allowed to be a `char const *', with
	 * one of the symbol flags being used to indicate if it's actually an object, which must
	 * be cleaned by `Dee_Decref(COMPILER_CONTAINER_OF(ss_doc, DeeStringObject, s_str))' */
	char const                *ss_name;   /* [0..1] Name of this symbol (NULL marks the sentinel) */
	char const                *ss_doc;    /* [0..1] An optional documentation string. */
	Dee_hash_t                 ss_hash;   /* [== Dee_HashStr(ss_name)] Hash-value of this symbol. */
	uint16_t                   ss_flags;  /* Set of `MODSYM_F*'. */
	union {
		uint16_t               ss_index;  /* [< :mo_globalc] The index of this symbol in the `:mo_globalv' vector.
		                                   * NOTE: In the case of a property, either 1 or 3 indices are allocated,
		                                   *       3 if `MODSYM_FREADONLY' isn't set, and 1 if it is. */
		struct {
			uint16_t           ss_symid;  /* [< :mo_importv[ss_impid]->mo_globalc]
			                               * The index of this symbol in the module's `:mo_globalv' vector.
			                               * NOTE: Must be at the same offset as `ss_index' */
			uint16_t           ss_impid;  /* [< :mo_importc] The index of the referenced module in the import vector. */
		}                      ss_extern; /* [valid_if(MODSYM_FEXTERN)] External symbol reference. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define ss_index  _dee_aunion.ss_index
#define ss_extern _dee_aunion.ss_extern
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#define Dee_MODULE_SYMBOL_EQUALS(x, name, size)                \
	(memcmp((x)->ss_name, name, (size) * sizeof(char)) == 0 && \
	 (x)->ss_name[size] == 0)
#define Dee_MODULE_SYMBOL_GETNAMESTR(x) ((x)->ss_name)
#define Dee_MODULE_SYMBOL_GETNAMELEN(x) (((x)->ss_flags & MODSYM_FNAMEOBJ) ? DeeString_SIZE(COMPILER_CONTAINER_OF((x)->ss_name, DeeStringObject, s_str)) : strlen((x)->ss_name))
#define Dee_MODULE_SYMBOL_GETDOCSTR(x)  ((x)->ss_doc)
#define Dee_MODULE_SYMBOL_GETDOCLEN(x)  (((x)->ss_flags & MODSYM_FDOCOBJ) ? DeeString_SIZE(COMPILER_CONTAINER_OF((x)->ss_doc, DeeStringObject, s_str)) : strlen((x)->ss_doc))
#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED DREF struct Dee_string_object *DCALL module_symbol_getnameobj(struct Dee_module_symbol *__restrict self);
INTDEF WUNUSED DREF struct Dee_string_object *DCALL module_symbol_getdocobj(struct Dee_module_symbol *__restrict self);
#else /* CONFIG_BUILDING_DEEMON */
#define Dee_module_symbol_getnameobj(x)   ((DeeStringObject *)(((x)->ss_flags & MODSYM_FNAMEOBJ) ? DeeObject_NewRef((DeeObject *)COMPILER_CONTAINER_OF((x)->ss_name, DeeStringObject, s_str)) : DeeString_NewWithHash((x)->ss_name, (x)->ss_hash)))
#define Dee_module_symbol_getdocobj(x)    ((DeeStringObject *)(((x)->ss_flags & MODSYM_FDOCOBJ) ? DeeObject_NewRef((DeeObject *)COMPILER_CONTAINER_OF((x)->ss_doc, DeeStringObject, s_str)) : DeeString_NewUtf8((x)->ss_doc, strlen((x)->ss_doc), Dee_STRING_ERROR_FIGNORE)))
#ifdef DEE_SOURCE
#define module_symbol_getnameobj   Dee_module_symbol_getnameobj
#define module_symbol_getdocobj    Dee_module_symbol_getdocobj
#endif /* DEE_SOURCE */
#endif /* !CONFIG_BUILDING_DEEMON */



#define Dee_MODULE_FNORMAL           0x0000 /* Normal module flags. */
#ifndef CONFIG_NO_DEC
#define Dee_MODULE_FHASCTIME         0x0800 /* [lock(WRITE_ONCE)] The `mo_ctime' field has been initialized. */
#endif /* !CONFIG_NO_DEC */
#define Dee_MODULE_FLOADING          0x1000 /* [lock(ATOMIC)] The module is currently being loaded. */
#define Dee_MODULE_FDIDLOAD          0x2000 /* [lock(WRITE_ONCE)] The module has been loaded. */
#define Dee_MODULE_FINITIALIZING     0x4000 /* [lock(ATOMIC)] The module is currently being initialized. */
#define Dee_MODULE_FDIDINIT          0x8000 /* [lock(WRITE_ONCE)] The module has been initialized (it's root code has been run).
                                             * NOTE: Once this flag has been set, it can be assumed that all imports
                                             *       of this module have also been initialized successfully. */

struct Dee_module_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec-386.S' */
	Dee_OBJECT_HEAD /* GC Object. */
	DREF struct Dee_string_object
	                            *mo_name;      /* [1..1][const] Name of this module (e.g.: `foo'). */
	DeeModuleObject            **mo_pself;     /* [1..1][0..1][lock(INTERN(modules_lock))] Module self-pointer in the module file hash-table.
	                                            * [if(MODULE_FDIDINIT, DREF)] When the module has been initialized, this becomes a reference to keep cached modules alive. */
	DeeModuleObject             *mo_next;      /* [0..1][valid_if(mo_pself)][lock(INTERN(modules_lock))] Next module with the same modulated `mo_path' hash (module file hash-table). */
	DREF struct Dee_string_object
	                            *mo_path;      /* [0..1][lock(MODULE_FLOADING)][const_if(MODULE_FDIDLOAD)] The absolute filename of this module's source file. */
#ifdef CONFIG_HOST_WINDOWS
	Dee_hash_t                   mo_pathhash;  /* The case-insensitive hash for `mo_path' */
#endif /* CONFIG_HOST_WINDOWS */
	DeeModuleObject            **mo_globpself; /* [1..1][0..1][lock(INTERN(modules_glob_lock))] Module self-pointer in the global module hash-table.*/
	DeeModuleObject             *mo_globnext;  /* [0..1][valid_if(mo_globpself)][lock(INTERN(modules_glob_lock))] Next global module with the same modulated `mo_name'. */
	uint16_t                     mo_importc;   /* [lock(MODULE_FLOADING)][const_if(MODULE_FDIDLOAD)] The total number of other modules imported by this one. */
	uint16_t                     mo_globalc;   /* [lock(MODULE_FLOADING)][const_if(MODULE_FDIDLOAD)] The total number of global symbols slots provided by this module. */
	uint16_t                     mo_flags;     /* [const] Module flags (Set of `MODULE_F*') */
	uint16_t                     mo_bucketm;   /* [lock(MODULE_FLOADING)][const_if(MODULE_FDIDLOAD)] Mask that should be applied to hash values before indexing `mo_bucketv'. */
	struct Dee_module_symbol    *mo_bucketv;   /* [1..mo_bucketm+1][owned_if(!= empty_module_buckets)][const]
	                                            * Hash-vector for translating a string into a `uint16_t' index for `mo_globalv'.
	                                            * This is where module symbol names are stored and also used to
	                                            * implement symbol access by name at runtime. */
#define Dee_MODULE_HASHST(self, hash)  ((hash) & ((DeeModuleObject *)Dee_REQUIRES_OBJECT(self))->mo_bucketm)
#define Dee_MODULE_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define Dee_MODULE_HASHIT(self, i)     (((DeeModuleObject *)Dee_REQUIRES_OBJECT(self))->mo_bucketv + ((i) & ((DeeModuleObject *)(self))->mo_bucketm))
	DREF DeeModuleObject *const *mo_importv;   /* [1..1][const_if(MODULE_FDIDLOAD)][0..rs_importc][lock(MODULE_FLOADING)][const_if(MODULE_FDIDLOAD)][owned] Vector of other modules imported by this one. */
	DREF DeeObject             **mo_globalv;   /* [0..1][lock(mo_lock)][0..mo_globalc][valid_if(MODULE_FDIDLOAD)][owned] Vector of module-private global variables. */
	DREF struct Dee_code_object *mo_root;      /* [0..1][lock(mo_lock)][const_if(MODULE_FDIDLOAD)] Root code object (Also used as constructor).
	                                            * HINT: Other code objects are addressed through constant/static variables.
	                                            * HINT: When this field has been assigned a non-NULL value, it can be assumed that `MODULE_FDIDLOAD' has been set! */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t          mo_lock;      /* Lock for this module. */
	struct Dee_thread_object    *mo_loader;    /* [?..1][valid_if((MODULE_FLOADING && !MODULE_FDIDLOAD) ||
	                                            *                 (MODULE_FINITIALIZING && !MODULE_FDIDINIT))]
	                                            * The thread currently loading/initializing this module.
	                                            * This is used to prevent a deadlock upon load recursion,
	                                            * but will allow for waiting when the load is done by
	                                            * another thread.
	                                            * WARNING: It may however deadlock when 2 threads try to
	                                            *          load/initialize modules when interlocked with
	                                            *          each other... ('Don't know how I could prevent
	                                            *          this short of putting a big 'ol lock around
	                                            *          everything that may initialize a module, when
	                                            *          doing so would kind-of defeat the purpose of
	                                            *          making everything thread-safe...)
	                                            *     XXX: The lock would have to be re-entrant & global and
	                                            *          be held during `DeeObject_Call(init_function, 0, NULL)'
	                                            *          inside of `DeeModule_RunInit()' */
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEC
	uint64_t                     mo_ctime;     /* [valid_if(MODULE_FDIDLOAD && MODULE_FHASCTIME)]
	                                            * Time (in milliseconds since 01.01.1970)
	                                            * when compilation of the module finished.
	                                            * NOTE: Never equal to (uint64_t)-1 */
#endif /* !CONFIG_NO_DEC */
	Dee_WEAKREF_SUPPORT
};


/* Lock/unlock the symbols of an interactive module object.
 * If `self' is a regular module, these are no-ops. */
#ifndef CONFIG_NO_THREADS
DFUNDEF NONNULL((1)) void DCALL DeeModule_LockSymbols(DeeModuleObject *__restrict self);
DFUNDEF NONNULL((1)) void DCALL DeeModule_UnlockSymbols(DeeModuleObject *__restrict self);
#else /* !CONFIG_NO_THREADS */
#define DeeModule_LockSymbols(self)   (void)0
#define DeeModule_UnlockSymbols(self) (void)0
#endif /* CONFIG_NO_THREADS */


/* The module of builtin objects accessible by opening `deemon'. */
DFUNDEF WUNUSED ATTR_CONST ATTR_RETNONNULL DeeModuleObject *DCALL DeeModule_GetDeemon(void);

#ifdef CONFIG_BUILDING_DEEMON
/* A stub module-object named `' (empty string), and pointing to `empty_code'. */
struct static_module_struct {
	/* Even though never tracked, static modules still need the GC header for visiting. */
	struct gc_head_raw m_head;
	DeeModuleObject    m_module;
};

#ifdef __INTELLISENSE__
INTDEF DeeModuleObject empty_module;
INTDEF DeeModuleObject deemon_module;
#else /* __INTELLISENSE__ */
INTDEF struct static_module_struct empty_module_head;
INTDEF struct static_module_struct deemon_module_head;
#define empty_module   empty_module_head.m_module
#define deemon_module  deemon_module_head.m_module
#endif /* !__INTELLISENSE__ */

#endif /* CONFIG_BUILDING_DEEMON */

DDATDEF DeeTypeObject DeeModule_Type;
#define DeeModule_Check(ob)      DeeObject_InstanceOf(ob, &DeeModule_Type)
#define DeeModule_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeModule_Type)

/* Create a new module object that has yet to be initialized or loaded. */
DFUNDEF WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_New(/*String*/ DeeObject *__restrict name);
DFUNDEF WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_NewString(/*utf-8*/ char const *__restrict name, size_t namelen);

/* Returns the name of the given module, which is either the module's
 * global name in the event of it being global (s.a. `DeeModule_IsGlobal()'),
 * or the last identifying segment of a relative import operation when the
 * module was been imported as a relative module. */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL /* TODO */
DeeModule_GetName(/*Module*/ DeeObject *__restrict self);

/* Return the name by which a given module `base' may address `self' in an
 * import() operation. - If the 2 modules are closely related, or when `self'
 * isn't a global module, the returned string is always in the form of `..foo.bar', which
 * would result from a relative path `FILEOF(self) == PATHOF(base) + "../foo/bar.dee"'.
 * When `self' is a global module, that can't be addressed relatively,
 * that module's global name is always returned.
 * Otherwise, when `self' can be addressed both globally, and relatively,
 * the relative name is chosen if it doesn't contain more than `max_rel_distance'
 * `.' characters.
 * @return: * :   A string containing a valid name for `self', as seen from `base'
 * @return: NULL: An error occurred.
 * @param: max_rel_distance: How to deal with modules that can be
 *                           reached both globally, as well as locally.
 *                           - 0: Always return global names.
 *                           - 1: Only return relative names for modules stored in the same folder
 *                           - 2: Don't traverse more than 1 folder for relative names.
 *                           - ...
 * @throw: ValueError: `self' isn't a global module, and there exists no relative
 *                      module name that could describe how `self' could be reached
 *                      from within `base'.
 *                      On unix, this never happens, but on windows this can happen
 *                      when `self' and `base' are stored on different drives. */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL /* TODO */
DeeModule_GetRelName(/*Module*/ DeeObject *self,
                     /*Module*/ DeeObject *base,
                     unsigned int max_rel_distance);
#define Dee_MODULE_RELNAME_DEFAULT_MAX_REL_DISTANCE 2 /* Default value for `max_rel_distance', as used for the
                                                       * purposes of dec module dependency encoding. */


/* Evaluates to `true' if `self' is a global module (i.e.: has been accessed via
 * the module library path), or `false' if `self' was imported as a relative module,
 * or through other means, such as the module being assigned with __MAIN__, or having
 * been created via special means.
 * NOTE: A global module will remain global for its entire lifetime, while a non-global
 *       module may be turned into a global one by being imported using its correct
 *       global name:
 * ${LIBPATH}/modules/foo.dee:
 * >> global bar = import(".bar");
 * ${LIBPATH}/modules/bar.dee:
 * >> ...
 * main_file.dee:
 * >> local foo = import("modules.foo");
 * >> print foo.__name__;     // "modules.foo"
 * >> print foo.bar.__name__; // "bar" (relative name)
 * >> local bar = import("modules.bar"); // The correct global name
 * >> assert bar === foo.bar; // Required to be the same object
 * >> print foo.__name__;     // "modules.foo"
 * >> print foo.bar.__name__; // "modules.bar" (Updated to become a global module)
 */
#define DeeModule_IsGlobal(self) (((DeeModuleObject *)Dee_REQUIRES_OBJECT(self))->mo_globpself != NULL)


struct Dee_compiler_error_object;
/* An optional callback that is invoked immediately before a compiler error is thrown.
 * This function's usual purpose is to immediately print the error to stderr,
 * though theoretically, it could also be used to do something different.
 * @param: fatality_mode: One of `COMPILER_ERROR_FATALITY_*'
 * @return: -1: An error occurred during handling.
 * @return:  0: The error was acknowledged and compilation may continue
 *              if `fatality_mode' is `Dee_COMPILER_ERROR_FATALITY_WARNING'
 *              or `Dee_COMPILER_ERROR_FATALITY_ERROR'.
 * @return:  1: Disregard `Dee_COMPILER_ERROR_FATALITY_ERROR' and continue compilation
 *              as though the error model was `Dee_COMPILER_ERROR_FATALITY_WARNING'.
 *              WARNING: This overrules the user-configuration set by `#pragma warning'
 * @return:  2: Disregard both `Dee_COMPILER_ERROR_FATALITY_FATAL' and
 *             `Dee_COMPILER_ERROR_FATALITY_ERROR' and interpret the error
 *              as a warning, only to be included in a multi-compiler-error
 *              if another compiler error is processed fatally at a later point.
 *              WARNING: This overrules the user-configuration set by `#pragma warning'
 * @return:  3: Ignore the error completely, so long as its fatality_mode isn't
 *             `Dee_COMPILER_ERROR_FATALITY_FORCEFATAL'. This means that it won't be
 *              thrown or scheduled as a warning, but simply discarded as though
 *              it never happened in the first place.
 * NOTES:
 *   - When no handler is set, the behavior is the same as though it always returned `0'
 *   - Upon entry, `error->ce_mode == fatality_mode'
 *   - Depending on return value, `error->ce_mode' is re-written before being saved. */
typedef WUNUSED NONNULL((1)) int
(DCALL *Dee_compiler_error_handler_t)(struct Dee_compiler_error_object *__restrict error,
                                      int fatality_mode, void *arg);
#define Dee_COMPILER_ERROR_FATALITY_WARNING    0 /* The error is a mere warning and will not cause the
                                                  * the compiler to fail, and neither will it thrown
                                                  * once the function returns.
                                                  * However, should a future error cause the compiler
                                                  * to fail, this warning will still be included in
                                                  * the resulting `DeeCompileErrorObject' error. */
#define Dee_COMPILER_ERROR_FATALITY_ERROR      1 /* The error will be fatal to prevent the compiler
                                                  * from successfully generating working user-code.
                                                  * However, compilation will continue for the time
                                                  * being, in order to collect further errors/warnings
                                                  * until a certain limit has been reached, or until
                                                  * a fatal/force-fatal error is encountered. */
#define Dee_COMPILER_ERROR_FATALITY_FATAL      2 /* The error is fatal, but handling exist to continue
                                                  * parsing code. Yet the user expects compilation to
                                                  * stop without the parser/assembler continuing to
                                                  * produce further errors. */
#define Dee_COMPILER_ERROR_FATALITY_FORCEFATAL 3 /* The error must be processed as fatal. */


/* General-purpose, optional compiler options that
 * can be specified whenever a module is loaded. */
struct Dee_compiler_options {
	struct Dee_compiler_options  *co_inner;             /* [0..1] Options used for compiling modules imported by this one. */
	struct Dee_string_object     *co_pathname;          /* [0..1] A filename used to resolve #include and relative import directives. */
	struct Dee_string_object     *co_filename;          /* [0..1] The filename that should appear in debug information when referring to `input_file'.
	                                                     *        This is also the filename returned by `__FILE__' and `__BASEFILE__',
	                                                     *        if not otherwise overwritten using `#line' */
	struct Dee_string_object     *co_rootname;          /* [0..1] The name of the root code object (as set in DDI) */
	WUNUSED int           (DCALL *co_setup)(void *arg); /* [0..1] Called once the compiler has been enabled.
	                                                     *        This callback can be used to perform additional compiler
	                                                     *        initialization, such as defining macros/assertions, or
	                                                     *        adding addition include paths.
	                                                     *        NOTE: Invocation happens after other compiler options have
	                                                     *              all been set and the initial source file has been
	                                                     *              pushed onto the #include-stack, though the first
	                                                     *              token hasn't been yielded, yet.
	                                                     * @return: >= 0: Setup was successful.
	                                                     * @return: < 0: `DeeModule_LoadSourceStream()' will fail with the same error. */
	void                         *co_setup_arg;         /* [?..?] Argument to `co_setup' */
	Dee_compiler_error_handler_t  co_error_handler;     /* [0..1] Called before compiler errors are processed.
	                                                     * This function's usual purpose is to print the error
	                                                     * during live-compilation mode, but can also be used
	                                                     * to put a twist on how errors are actually processed. */
	void                         *co_error_arg;         /* [?..?] Argument to `co_error_handler' */
	uint16_t                      co_tabwidth;          /* The width of tabulators, or `0' to use the hard-coded default. */
	uint16_t                      co_compiler;          /* Set of `COMPILER_F*' from `<deemon/compiler/compiler.h>'. */
	uint16_t                      co_parser;            /* Set of `PARSE_F*'    from `<deemon/compiler/lexer.h>' */
	uint16_t                      co_optimizer;         /* Set of `OPTIMIZE_F*' from `<deemon/compiler/ast.h>' */
	uint16_t                      co_unwind_limit;      /* Limit control for loop unwinding: The max amount of times that
	                                                     * a constant loop may be unwound. (Set to ZERO(0) to disable) */
	uint16_t                      co_assembler;         /* Set of `ASM_F*'      from `<deemon/compiler/assembler.h>' */
#define Dee_DEC_FNORMAL           0x0000                /* Normal DEC operations mode. */
#define Dee_DEC_FDISABLE          0x0001                /* Do not load DEC files. */
#define Dee_DEC_FLOADOUTDATED     0x0002                /* Don't check dependencies and imported modules for having changed
	                                                     * since the dec file has been created (can lead to inconsistencies
	                                                     * and invalid global variable indices).
	                                                     * This flag also suppresses the need of non-module dependencies to
	                                                     * even exist (such as the module's original source file), though
	                                                     * executing a DEC file without the original source at hand is not
	                                                     * intended behavior per-s�. */
#define Dee_DEC_FUNTRUSTED        0x0004                /* The origin of the DEC source is not trusted.
	                                                     * When this flag is set, all generated code objects have the `CODE_FASSEMBLY'
	                                                     * flag set, as well as have their text followed by `INSTRLEN_MAX'
	                                                     * bytes of `ASM_RET_NONE' instruction (aka. trailing ZERO-bytes).
	                                                     * This way, the contained assembly will be unable to escape its
	                                                     * own text segment which would otherwise pose a security risk when
	                                                     * untrusted code was able to start executing random instruction
	                                                     * based on unrelated, arbitrary data in host memory (HeartBleed anyone?) */
	uint16_t                      co_decloader;         /* Set of `DEC_F*' (unused when deemon was built with `CONFIG_NO_DEC') */
	uint16_t                      co_decwriter;         /* Set of `DEC_WRITE_F*' from `<deemon/compiler/dec.h>' (unused when deemon was built with `CONFIG_NO_DEC') */
	DeeObject                    *co_decoutput;         /* [0..1] Dec output location (ignored when `ASM_FNODEC' is set)
	                                                     *  - Filename (string) of the generated `.dec' file.
	                                                     *  - Stream (any other object) into which to write the contents of the dec file.
	                                                     *  - When `NULL', the filename is selected such that it will be used to
	                                                     *    quickly load module object when one of the dec-enabled `DeeModule_OpenGlobal*'
	                                                     *    functions is used (aka. `<source_path>/.<source_file>.dec')
	                                                     * Note that when passing a string, that file will be overwritten, should it already exists. */
};

/* Load the given module from a filestream opened for a source file.
 * @param: self:       The module that should be loaded.
 * @param: input_file: A file object to be used as input stream.
 * @param: start_line: The starting line number of the input stream (zero-based)
 * @param: start_col:  The starting column offset of the input stream (zero-based)
 * @return: -1:        An error occurred and was thrown.
 * @return:  0:        Successfully loaded the given module.
 * @return:  1:        The module has already been loaded/was loading but has finished now.
 * @return:  2:        The module is already being loaded in the calling thread.
 * This is the main interface for manually loading modules, as
 * well as compiling & linking source code that may not be found
 * as files within the real filesystem.
 * NOTE: I highly encourage you to set `options->co_pathname'
 *       to a file within the folder that should be used to
 *       resolve relative imports and #include statements,
 *       as without this information given, the process
 *       working directory will be used instead. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_LoadSourceStream(/*Module*/ DeeObject *__restrict self,
                           /*File*/ DeeObject *__restrict input_file,
                           int start_line, int start_col,
                           struct Dee_compiler_options *options);

/* Given the filename of a module source file, load it
 * and create a new module from the contained source code.
 * NOTE: In case the module has been loaded before,
 *       return the already-loaded instance instead.
 * NOTE: In case the module is currently being loaded in the calling
 *       thread, that same partially loaded module is returned, meaning
 *       that the caller can easily check for `MODULE_FLOADING && !MODULE_FDIDLOAD'
 * @param: module_global_name: When non-NULL, use this as the module's actual name.
 *                             Also: register the module as a global module under this name when given.
 *                             When not given, the module isn't registered globally, and the
 *                             name of the module will be deduced from its `source_pathname'
 * @param: source_pathname:    The filename of the source file that should be opened.
 *                             When `NULL', simply use the absolute variant of `DeeString_AsUtf8(source_pathname)'
 * @param: throw_error:        When true, throw an error if the module couldn't be
 *                             found and return `NULL', otherwise return `ITER_DONE'.
 * @return: ITER_DONE:        `throw_error' is `true' and `source_pathname' could not be found. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenSourceFile(/*String*/ DeeObject *__restrict source_pathname,
                         /*String*/ DeeObject *module_global_name,
                         struct Dee_compiler_options *options,
                         bool throw_error);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenSourceFileString(/*utf-8*/ char const *__restrict source_pathname, size_t source_pathsize,
                               /*utf-8*/ char const *module_global_name, size_t module_global_namesize,
                               struct Dee_compiler_options *options, bool throw_error);


/* Construct a module from a memory source-code blob.
 * NOTE: Unlike `DeeModule_OpenSourceFile()', this function will not bind `source_pathname'
 *       to the returned module, meaning that the module object returned will be entirely
 *       anonymous, except for when `module_name' was passed as non-NULL, in which case
 *       the returned module will be made available as a global import with that same name,
 *       and be available for later addressing using `DeeModule_OpenGlobal()'
 * @param: data:            A pointer to the raw source-code that should be parsed as
 *                          the deemon source for the module.
 * @param: data_size:       The size of the `data' blob (in characters)
 * @param: source_pathname: The filename of the source file from which data (supposedly) originates.
 *                          Used by `#include' directives, as well as `__FILE__' and ddi information.
 *                          When NULL, an empty string is used internally, which results in the current
 *                          directory being used as base for relative imports.
 * @param: module_name:     When non-NULL, use this as the module's actual name.
 *                          Also: register the module as a global module.
 * @param: start_line:      The starting line number of the data blob (zero-based)
 * @param: start_col:       The starting column offset of the data blob (zero-based)
 * @param: options:         An optional set of extended compiler options. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenSourceMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                           int start_line, int start_col, struct Dee_compiler_options *options,
                           /*String*/ DeeObject *source_pathname,
                           /*String*/ DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenSourceMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                 int start_line, int start_col, struct Dee_compiler_options *options,
                                 /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                 /*utf-8*/ char const *module_name, size_t module_namesize);

/* Very similar to `DeeModule_OpenSourceMemory()', and used to implement it,
 * however source data is made available using a stream object derived
 * from `File from deemon'
 * @param: source_stream:   A File object from which source code will be read.
 * @param: source_pathname: The filename of the source file from which data (supposedly) originates.
 *                          Used by `#include' directives, as well as `__FILE__' and ddi information.
 *                          When NULL, an empty string is used internally, which results in the current
 *                          directory being used as base for relative imports.
 * @param: module_name:     When non-NULL, use this as the module's actual name.
 *                          Also: register the module as a global module.
 * @param: start_line:      The starting line number of the data blob (zero-based)
 * @param: start_col:       The starting column offset of the data blob (zero-based)
 * @param: options:         An optional set of extended compiler options. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenSourceStream(/*File*/ DeeObject *source_stream,
                           int start_line, int start_col, struct Dee_compiler_options *options,
                           /*String*/ DeeObject *source_pathname,
                           /*String*/ DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenSourceStreamString(/*File*/ DeeObject *source_stream,
                                 int start_line, int start_col, struct Dee_compiler_options *options,
                                 /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                 /*utf-8*/ char const *module_name, size_t module_namesize);



/* Construct an interactive module
 * Interactive modules are kind-of special, in that they parse,
 * compile, assemble, and link source code a-statement-at-a-time.
 * Additionally, the code that they produce is executed when you
 * iterate the returned module (_InteractiveModule) object, at
 * which point it will return the value of expressions found in
 * the root-scope of the associated source code.
 * NOTE: When constructing an iterator for the returned module,
 *       the iterator cannot be rewound or copied, and will modify
 *       the internal state of the interactive module itself, as
 *       well as various other objects associated with it.
 *       For these reasons, interactive modules do not inherit from
 *      `sequence', similar to how file objects don't either.
 * PARSING vs. NON-BLOCKING:
 *     - The parser will block until either sufficient tokens have been
 *       made available to construct a full statement (that is guarantied
 *       to be complete), or until the source stream has indicated EOF by
 *       having its read() operator return ZERO.
 *     - As a language, deemon wasn't ~really~ designed to be used in an
 *       interactive fashion, which is why there are some caveats to this:
 *       >> if (foo)
 *       >>     print "bar";
 *       This might seem like a simple and complete statement, so you'd
 *       probably expect the parser to stop blocking feeding it this.
 *       However, I have to disappoint you, because I didn't should you
 *       the full source code:
 *       >> if (foo)
 *       >>     print "bar";
 *       >> else {
 *       >>     print "baz";
 *       >> }
 *       Yeah... As you can see, the parser needed to know if the token
 *       following the true-branch was the keyword else, who's presence
 *       then managed to alter the result of the code generated by the
 *       statement as a whole.
 *       There are a couple of other places with the same problem, such
 *       as try-statements which need to know the trailing token to see
 *       if its `catch' or `finally', in which case another guard would
 *       have to be defined.
 *       However, when dealing with an untrusted, or non-validated data
 *       source, you could force statement completion by balancing unclosed
 *       strings and slash-star style comments, before appending 2 `;'
 *       characters to ensure that an incomplete statement is closed,
 *       while also providing another token that may be needed by the
 *       statement to ensure that no trailing code still exists.
 * LINE-TERMINATED STATEMENTS:
 *     - The ability of interactive compilation often wakes the need of having
 *       a way of interpreting line-feeds as the statement-termination token,
 *       which in the case of deemon is `;'
 *     - Deemon does provide such functionality, which can be enabled
 *       by setting the `PARSE_FLFSTMT' flag in `options->co_parser'.
 *       HINT: The commandline version of deemon exposes this flag as `-C[no-]lfstmt'
 *       HINT: This flag is also enabled by default when `NULL' is passed for `options'
 *       The general rule when it comes to which line-feeds are interpreted
 *       as alias to `;' is fairly simple, in that any non-escaped line-feed
 *       following a non-empty statement, that doesn't appear within parenthesis,
 *       sequence-braces (s.a. abstract sequence syntax), or array-brackets
 *       will be interpreted the same way a `;' would:
 *       >> local x = 10            // OK (plain, old expression)
 *       >> print x                 // OK (plain, old statement)
 *       >> print "x = {}".format({
 *       >>      x
 *       >> })                      // OK (linefeeds in sequence-braces don't count)
 *       An exception to improve usability of language features:
 *       >> local x = ({            // statements found in expressions can also use
 *       >>                         // linefeeds, despite the surrounding parenthesis
 *       >>     local y = 10        // OK (plain, old expression)
 *       >>     y * 2               // OK (plain, old expression)
 *       >> })                      // OK (plain, old expression)
 *       However, an exception to these rules are leading line-feeds following, or
 *       preceding statement-block-prefix-token-sequences, which simply discard
 *       any leading line-feeds:
 *       >> if (foo())     // This does what you think it would, in that
 *       >>                // `print bar' is tuels the true-block of this if-statement,
 *       >>                // despite the fact that there are leading line-feeds.
 *       >> {
 *       >>     print bar
 *       >> }
 *       >> else           // This else still belongs to the `if' above, despite the
 *       >>                // preceiding line-feed
 *       >>     print baz  // This is still the false-block of the if-statement,
 *       >>                // despite the leading line-feeds
 *       >> print "after"  // This statement is no longer apart of the if-statement above.
 *       The same behavior applies to the following statements, with ignored line-feeds
 *       marked as `[]', and `...' meaning another statement or expression:
 *        - `while(...)[] ...'
 *        - `with(...)[] ...'
 *        - `switch(...)[] ...'
 *        - `case ...:[] ...'
 *        - `default:[] ...'
 *        - `...:[] ...' // C-style label
 *        - `try[] ... []finally[] ...'
 *        - `try[] ... []catch[](...)[] ...'
 *        - `do[] ... []while[](...)'
 *        - `if[](...)[] ...'
 *        - `if[](...)[] ... []else[] ...'
 *        - `for(...: ...)[] ... '
 *        - `foreach(...: ...)[] ... '
 *        - `for(...[];[]...[];[]...)[] ... ' // NOTE: Line-feeds can also be used instead of `;' in for-expressions
 *        - `import ... ,[] ...'              // Allow line-feeds after commas in import lists
 *        - `import ... ,[] ... from ...'
 *        - `class ... []{ ... }'
 *        - `function ... [] -> ...'
 *        - `function ... [] { ... }'
 *        - `[] ...'  // Empty lines are ignored (symbolizes line-feeds found before some other statement)
 *       Another simply way of thinking about it is that line-feeds are ignored
 *       before and after any location where a { ... }-style statement could be used.
 *       WARNING: The expression-variants of these statements do not accept
 *                sporadic line-feeds within them, however you can simply wrap
 *                those in parenthesis to force linefeeds to be ignored.
 * RESTRICTIONS:
 *     - If the streamed source code attempts to access the current code object,
 *       the returned object will be a snapshot of the active assembly at the
 *       time when the access was made. This snapshot will not be updated as
 *       additional assembly is generated from new source code.
 *     - Modules of this type will never set the `MODULE_FDIDLOAD' flag, meaning
 *       that they will never finish ~loading~ in the sense that they will
 *       relinquish their right to modify the module's `mo_importv', `mo_globalv',
 *      `mo_root', `mo_bucketv', etc... fields.
 *     - Interactive code should refrain from starting new threads.
 *       TODO: Currently, not abiding by this rule will result in hard undefined
 *             behavior, potentially resulting in deemon crashing completely.
 *             >> This is a bug that must be resolved at some point!
 *             The problem are the `mo_globalv', `mo_importv' and `mo_bucketv'
 *             vectors of modules, which an interactive module will modify and
 *             replace whenever it pleases while holding an internal lock.
 *             The problem here lies in the fact that due to race conditions,
 *             as well as the fact that some code assumes that these fields
 *             are constant once a module is loaded, also making the assumption
 *             that any module that has made it all the way to the interpreter
 *             will have been completely loaded.
 *             This isn't a situation that should even be able to arise, but
 *             the special locking should be documented, and there should be
 *             a guaranty that modules found in the `mo_importv' vector can
 *             never be interactive modules.
 * @param: source_pathname: The filename of the source file from which data (supposedly) originates.
 *                          Used by `#include' directives, as well as `__FILE__' and ddi information.
 *                          When NULL, an empty string is used internally, which results in the current
 *                          directory being used as base for relative imports.
 * @param: module_name:     When non-NULL, use this as the module's actual name.
 *                          Note however that the module is never made available globally.
 * @param: source_stream:   A stream from which source code is read, which is then compiled immediately.
 * @param: start_line:      The starting line number when compiling code. (zero-based)
 * @param: start_col:       The starting column number when compiling code. (zero-based)
 * @param: options:         A set of compiler options applicable for compiled code.
 *                          Note however that certain options have no effect, such
 *                          as the fact that peephole and other optimizations are
 *                          forced to be disabled, or DEC files are never generated,
 *                          all for reasons that should be quite obvious.
 * @param: mode:            The way that contained source code is able to yield
 *                          expressions to the iterator the interactive module.
 *                          Set of `MODULE_INTERACTIVE_MODE_F*'
 * @param: argv:            An optional tuple object (when NULL, `Dee_EmptyTuple' is used),
 *                          that is passed to module code as arguments (i.e. `[...]').
 * @param: default_symbols: A mapping-like object of type `{(string, Object)...}', that
 *                          contains a set of pre-defined variables that should be made
 *                          available to the interactive source code by use of global
 *                          variables.
 *                          Each item contained within is used to define a global variable
 *                          using the key (which must be a string) as name, and the associated
 *                          value as the initializer to which the resulting global variable
 *                          will be bound prior to the initial launch of interactive assembly.
 *                          Thus, provided symbols are made available by name, left to-be used
 *                          by the module however it pleases. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenInteractive(/*File*/ DeeObject *source_stream, unsigned int mode,
                          int start_line, int start_col, struct Dee_compiler_options *options,
                          /*String*/ DeeObject *source_pathname, /*String*/ DeeObject *module_name,
                          /*Tuple*/ DeeObject *argv, /*Mapping{(string, Object)...}*/ DeeObject *default_symbols);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenInteractiveString(/*File*/ DeeObject *source_stream, unsigned int mode,
                                int start_line, int start_col, struct Dee_compiler_options *options,
                                /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                /*utf-8*/ char const *module_name, size_t module_namesize,
                                /*Tuple*/ DeeObject *argv, /*Mapping{(string, Object)...}*/ DeeObject *default_symbols);

/* No special syntactical changes are made. - The root scope is a yield-like
 * function, and streamed source code must include `yield'-statements in
 * order to have them be returned by iterating the interactive module. */
#define Dee_MODULE_INTERACTIVE_MODE_FNORMAL        0x0000
/* Default mode: every expression-like statement (i.e. any expression not
 * used as a statement) is implicitly wrapped in a yield-statement, which
 * is then compiled normally. */
#define Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR 0x0001
/* Same as `Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR', but statement-like
 * definitions are yielded, too. */
#define Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTSTMT 0x0002
/* Statements will only operate as yield-expressions if they appear in
 * the base file (that is the file corresponding to `source_stream')
 * This is the default behavior and allows interactive code to #include
 * other files, and have those files be compiled just as intended. */
#define Dee_MODULE_INTERACTIVE_MODE_FONLYBASEFILE  0x0004

DDATDEF DeeTypeObject DeeInteractiveModule_Type;
#define DeeInteractiveModule_Check(ob)      DeeObject_InstanceOf(ob, &DeeInteractiveModule_Type)
#define DeeInteractiveModule_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeInteractiveModule_Type)






/* Return the export address of a native symbol exported from a dex `self'.
 * When `self' isn't a dex, but a regular module, or if the symbol wasn't found, return `NULL'.
 * NOTE: Because native symbols cannot appear in user-defined modules,
 *       in the interest of keeping native functionality to its bare
 *       minimum, any code making using of this function should contain
 *       a fallback that calls a global symbol of the module, rather
 *       than a native symbol:
 * >> static int (*padd)(int x, int y) = NULL;
 * >> if (!padd)
 * >>     *(void **)&padd = DeeModule_GetNativeSymbol(IMPORTED_MODULE, "add");
 * >> // Fallback: Invoke a member attribute `add' if the native symbol doesn't exist.
 * >> if (!padd)
 * >>     return DeeObject_CallAttrStringf(IMPORTED_MODULE, "add", "dd", x, y);
 * >> // Invoke the native symbol.
 * >> return DeeInt_New((*padd)(x, y)); */
DFUNDEF WUNUSED NONNULL((1, 2)) void *DCALL
DeeModule_GetNativeSymbol(/*Module*/ DeeObject *__restrict self,
                          char const *__restrict name);

/* Given a static pointer `ptr' (as in: a pointer to some statically allocated structure),
 * try to determine which DEX module (if not the deemon core itself) was used to declare
 * a structure located at that pointer, and return a reference to that module.
 * If this proves to be impossible, or if `ptr' is an invalid pointer, return `NULL'
 * instead, but don't throw an error.
 * When deemon has been built with `CONFIG_NO_DEX', this function will always return
 * a reference to the builtin `deemon' module.
 * @return: * :   A pointer to the dex module (or to `DeeModule_GetDeemon()') that
 *                contains a static memory segment of which `ptr' is apart of.
 * @return: NULL: Either `ptr' is an invalid pointer, part of a library not loaded
 *                as a module, or points to a heap/stack segment.
 *                No matter the case, no error is thrown for this, meaning that
 *                the caller must decide on how to handle this. */
DFUNDEF WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_FromStaticPointer(void const *ptr);


/* Open a module, given its name in the global module namespace.
 * Global modules use their own cache that differs from the cache
 * used to unify modules through use of their filename.
 * NOTES:
 *   - Global module names are the raw filenames of modules,
 *     excluding an absolute path prefix or extension suffix.
 *   - When searching for global modules, each string from `DeeModule_GetPath()'
 *     is prepended in ascending order until an existing file is found.
 *   - Using this function, dex extensions and `.dec' (DEeemonCompiled) files
 *     can also be opened in addition to `.dee' (source) files, as well
 *     as the deemon's builtin module when `deemon' is passed as `module_name'.
 *   - Rather than using '/' or '\\' to identify separators between folders, you must instead use `.'
 *   - If `module_name' contains any whitespace or punctuation characters,
 *     this function will fail with an `Error.ValueError'.
 *   - If the host's filesystem is case-insensitive, then module
 *     names may be case-insensitive as well. However if this is
 *     the case, the following must always be true for any module:
 *     >> import object from deemon;
 *     >> import mymodule;
 *     >> import MyModule;
 *     >> assert mymodule === MyModule;
 *     Note that this code example must only work when the module
 *     system is case-insensitive as well.
 * @param: throw_error: When true, throw an error if the module couldn't be
 *                      found and return `NULL', otherwise return `ITER_DONE'. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenGlobal(/*String*/ DeeObject *__restrict module_name,
                     struct Dee_compiler_options *options,
                     bool throw_error);
DFUNDEF WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenGlobalString(/*utf-8*/ char const *__restrict module_name,
                           size_t module_namesize,
                           struct Dee_compiler_options *options,
                           bool throw_error);

/* Get a global module that has already been loaded, given its name.
 * If the module hasn't been loaded yet, NULL is returned.
 * NOTE: These functions never throw an error! */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_Get(DeeObject *__restrict module_name);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_GetString(/*utf-8*/ char const *__restrict module_name,
                    size_t module_namesize);


/* Lookup an external symbol.
 * Convenience function (same as `DeeObject_GetAttrString(DeeModule_OpenGlobalString(...), ...)') */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExtern(/*utf-8*/ char const *__restrict module_name,
                    /*utf-8*/ char const *__restrict global_name);

/* Helper wrapper for `DeeObject_Callf(DeeModule_GetExtern(...), ...)',
 * that returns the return value of the call operation. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternf(/*utf-8*/ char const *__restrict module_name,
                      /*utf-8*/ char const *__restrict global_name,
                      char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternf(/*utf-8*/ char const *__restrict module_name,
                       /*utf-8*/ char const *__restrict global_name,
                       char const *__restrict format, va_list args);


/* Open a module using a relative module name
 * `module_name' that is based off of `module_pathname'
 * NOTE: If the given `module_name' doesn't start with a `.'
 *       character, the given `module_pathname' is ignored and the
 *       call is identical to `DeeModule_OpenGlobal(module_name, options)'
 * HINT: The given `module_pathname' is merely prepended
 *       before the module's actual filename.
 * Example:
 * >> DeeModule_OpenRelative("..foo.bar", "src/scripts");  // `src/foo/bar.dee'
 * >> DeeModule_OpenRelative(".sys.types", ".");           // `./sys/types.dee'
 * >> DeeModule_OpenRelative("thread", "foo/bar");         // `${LIBPATH}/thread.dee'
 * NOTE: This function also tries to open DEX modules, as well as `.*.dec' files.
 * @param: throw_error: When true, throw an error if the module couldn't be
 *                      found and return `NULL', otherwise return `ITER_DONE'. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenRelative(/*String*/ DeeObject *__restrict module_name,
                       /*utf-8*/ char const *__restrict module_pathname,
                       size_t module_pathsize,
                       struct Dee_compiler_options *options,
                       bool throw_error);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenRelativeString(/*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                             /*utf-8*/ char const *__restrict module_pathname, size_t module_pathsize,
                             struct Dee_compiler_options *options,
                             bool throw_error);


/* Low-level module import processing function, used for importing modules
 * relative to some given base-path, while also able to process relative module
 * names, as well as support all of the various form in which modules can appear.
 * @param: module_path:        The base path from which to offset `module_name'
 *                             If this path is relative, it will be made absolute to
 *                             the current working directory.
 * @param: module_pathsize:    The length of `module_path' in bytes
 * @param: module_name:        The demangled name of the module to import
 * @param: module_namesize:    The length of `module_name' in bytes
 * @param: module_global_name: The name that should be used to register the module
 *                             in the global module namespace, or `NULL' if the module
 *                             should not be registered as global, or `ITER_DONE' if
 *                             the name should automatically be generated from `module_path'
 *                             NOTE: If another module with the same global name already
 *                                   exists by the time to module gets registered as global,
 *                                   that module will be returned instead!
 * @param: options:            Compiler options detailing how a module should be loaded
 * @param: mode:               The open mode (set of `MODULE_OPENINPATH_F*')
 * Module files are attempted to be opened in the following order:
 * >> SEARCH_MODULE_FILESYSTEM_CACHE(joinpath(module_path, module_name + ".dee"));
 * >>#ifndef CONFIG_NO_DEC
 * >> TRY_LOAD_DEC_FILE(joinpath(module_path, "." + module_name + ".dec"));
 * >>#endif // !CONFIG_NO_DEC
 * >>#ifndef CONFIG_NO_DEX
 * >>#ifdef CONFIG_HOST_WINDOWS
 * >> TRY_LOAD_DEX_LIBRARY(joinpath(module_path, module_name + ".dll"));
 * >>#else
 * >> TRY_LOAD_DEX_LIBRARY(joinpath(module_path, module_name + ".so"));
 * >>#endif
 * >>#endif // !CONFIG_NO_DEX
 * >> TRY_LOAD_SOURCE_FILE(joinpath(module_path, module_name + ".dee"));
 * EXAMPLES:
 * >> char const *path = "/usr/lib/deemon/lib";
 * >> char const *name = "util";
 * >> // Opens:
 * >> //   - /usr/lib/deemon/lib/
 * >> DeeModule_OpenInPath(path, strlen(path),
 * >>                      name, strlen(name),
 * >>                      NULL, NULL,
 * >>                      Dee_MODULE_OPENINPATH_FTHROWERROR);
 * @return: * :        The module that was imported.
 * @return: ITER_DONE: The module could not be found (only when `Dee_MODULE_OPENINPATH_FTHROWERROR' isn't set)
 * @return: NULL:      An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 3)) DREF /*Module*/ DeeObject *DCALL
DeeModule_OpenInPath(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                     /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                     /*String*/ DeeObject *module_global_name,
                     struct Dee_compiler_options *options,
                     unsigned int mode);
#define Dee_MODULE_OPENINPATH_FNORMAL      0x0000 /* Normal flags */
#define Dee_MODULE_OPENINPATH_FRELMODULE   0x0001 /* The module name may be interpreted relatively.
                                                   * When set, each leading `.' characters in `module_name'
                                                   * will remove 1 trailing path node from `module_path'.
                                                   * When not set, leading `.' characters are considered to
                                                   * be apart of the module's name. */
#define Dee_MODULE_OPENINPATH_FTHROWERROR  0x0002 /* If set, throw a FileNotFound error if the module could not be found.
                                                   * Otherwise, return `ITER_DONE' without throwing an error. */


/* Create a new function object for the root code-object of a given module.
 * NOTE: This function will also ensure that all modules
 *       imported by this one have been fully initialized.
 * NOTE: This function automatically calls `DeeModule_InitImports()'
 *       to ensure that the module is ready to be executed.
 * @param: set_initialized: When true, also set the `MODULE_FDIDINIT' flag if
 *                          it, or `MODULE_FINITIALIZING' hasn't been set already
 * @return: * : A callable object which, when invoked, will execute the module's root code,
 *              while passing any arguments given to it to the module's root where they
 *              are available as `...' (3 dots using in an expression)
 *              my_module.dee:
 *              >> print [...];  // [10, 20, 30]
 *              DeeObject_Callf(DeeModule_GetRoot(my_module, true), "ddd", 10, 20, 30);
 * @return: NULL: Failed to create a function object for the module's root code object. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Callable*/ DeeObject *DCALL
DeeModule_GetRoot(/*Module*/ DeeObject *__restrict self,
                  bool set_initialized);


#ifndef CONFIG_NO_DEC
/* Returns the compile-time of a given module (in microseconds
 * since 01.01.1970), or (uint64_t)-1 if an error occurred.
 * NOTE: For Always using this function as sometimes,
 *       the timestamp is calculated lazily.
 * NOTE: If the module's file could not be found, 0 is returned. */
DFUNDEF WUNUSED NONNULL((1)) uint64_t DCALL
DeeModule_GetCTime(/*Module*/ DeeObject *__restrict self);
#endif /* !CONFIG_NO_DEC */

/* Same as `DeeModule_Import', but relative module
 * paths are imported in relation to `basemodule' */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*Module*/ DeeObject *DCALL
DeeModule_ImportRel(/*Module*/ DeeObject *__restrict basemodule,
                    /*String*/ DeeObject *__restrict module_name,
                    struct Dee_compiler_options *options,
                    bool throw_error);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*Module*/ DeeObject *DCALL
DeeModule_ImportRelString(/*Module*/ DeeObject *__restrict basemodule,
                          /*utf-8*/ char const *__restrict module_name,
                          size_t module_namesize,
                          struct Dee_compiler_options *options,
                          bool throw_error);


#ifdef CONFIG_BUILDING_DEEMON
/* Implementation of the builtin `import()' and `module.open()' functions.
 * Using the module declaring the code of the current top execution frame (if it exists),
 * invoke `DeeModule_OpenRelative()' with its path and the given `module_name'.
 * @param: throw_error: When true, throw an error if the module couldn't be
 *                      found and return `NULL', otherwise return `ITER_DONE'. */
INTDEF WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_Import(/*String*/ DeeObject *__restrict module_name,
                 struct Dee_compiler_options *options,
                 bool throw_error);

/* Access global variables of a given module by their name described by a C-string.
 * These functions act and behave just as once would expect, raising errors when
 * appropriate and returning NULL/false/-1 upon error or not knowing the given name. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeModule_GetAttrString(DeeModuleObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeModule_GetAttrStringLen(DeeModuleObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeModule_HasAttrString(DeeModuleObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeModule_HasAttrStringLen(DeeModuleObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_BoundAttrString(DeeModuleObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_BoundAttrStringLen(DeeModuleObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_DelAttrString(DeeModuleObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_DelAttrStringLen(DeeModuleObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeModule_SetAttrString(DeeModuleObject *self, char const *__restrict attr_name, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeModule_SetAttrStringLen(DeeModuleObject *self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeModule_FindAttrString(DeeModuleObject *__restrict self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
#endif /* CONFIG_BUILDING_DEEMON */

/* Lookup the module symbol associated with a given its name or GID.
 * If the symbol could not be found, return `NULL', but _DONT_ throw an error.
 * WARNING: When `self' could potentially be an interactive module, you
 *          must surround a call to any of these functions with a lock that
 *          can be acquired / released using `DeeModule_LockSymbols()' /
 *         `DeeModule_UnlockSymbols()'
 *          Additionally, you must be extremely careful, as an interactive
 *          module may arbitrarily modify its global object table! */
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL DeeModule_GetSymbolString(DeeModuleObject *__restrict self, char const *__restrict attr_name, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL DeeModule_GetSymbolStringLen(DeeModuleObject *__restrict self, char const *__restrict attr_name, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1)) struct Dee_module_symbol *DCALL DeeModule_GetSymbolID(DeeModuleObject *__restrict self, uint16_t gid);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeModule_GetAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol *__restrict symbol);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_BoundAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol *__restrict symbol);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_DelAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol *__restrict symbol);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeModule_SetAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol *__restrict symbol, DeeObject *__restrict value);

/* Return the name of a global variable in the given module.
 * @return: NULL: The given `gid' is not recognized, or the module hasn't finished/started loading yet.
 * @return: * :   The name of the global associated with `gid'.
 *                Note that in the case of aliases existing for `gid', this function prefers not to
 *                return the name of an alias, but that of the original symbol itself, so long as that
 *                symbol actually exist, which if it doesn't, it will return the name of a random alias. */
DFUNDEF WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GlobalName(/*Module*/ DeeObject *__restrict self, uint16_t gid);

/* Try to run the initializer of a module, should it not have been run yet.
 * This function will atomically ensure that the initializer
 * is only run once, and only so in a single thread.
 * Additionally, this function will also call itself recursively on
 * all other modules imported by the given one before actually invoking
 * the module's own initializer.
 *    This is done by calling `DeeModule_InitImports(self)'
 * NOTE: When `DeeModule_GetRoot()' is called with `set_initialized' set to `true', the
 *       module was-initialized flag is set the same way it would be by this function.
 * @throws: Error.RuntimeError: The module has not been loaded yet. (aka. no source code was assigned)
 * @return: -1: An error occurred during initialization.
 * @return:  0: Successfully initialized the module/the module was already initialized.
 * @return:  1: You are already in the process of initializing this module (not an error). */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeModule_RunInit(/*Module*/ DeeObject *__restrict self);

/* Initialize all modules imported by the given one.
 * @throws: Error.RuntimeError: The module has not been loaded yet. (aka. no source code was assigned)
 * @return: -1: An error occurred during initialization.
 * @return:  0: All modules imported by the given one are now initialized. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeModule_InitImports(/*Module*/ DeeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_CODE_H */
