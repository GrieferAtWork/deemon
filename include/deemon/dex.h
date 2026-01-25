/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
/*!export **/
/*!export DEX_**/
/*!export DEXSYM_**/
/*!export Dee_DEX_**/
/*!export Dee_DEXSYM_**/
/*!export _Dee_MODULE_DEXDATA_**/
/*!export -_Dee_PRIVATE_**/
#ifndef GUARD_DEEMON_DEX_H
#define GUARD_DEEMON_DEX_H 1 /*!export-*/

#include "api.h"

#ifndef CONFIG_NO_DEX
#include <hybrid/typecore.h> /* __*_TYPE__ */

#include "gc.h"     /* Dee_gc_head_link, _Dee_GC_HEAD_UNTRACKED_INIT */
#include "module.h" /* DeeModuleDex_Type, DeeModuleObject, Dee_MODSYM_F*, Dee_MODULE_FNORMAL, Dee_MODULE_INIT_UNINITIALIZED, Dee_MODULE_STRUCT, Dee_module_object, Dee_module_symbol, _Dee_MODULE_INIT_mo_lock */
#include "object.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL */
#include <stdint.h>  /* uintptr_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define DEXSYM_NORMAL      Dee_DEXSYM_NORMAL
#define DEXSYM_READONLY    Dee_DEXSYM_READONLY
#define DEXSYM_CONSTEXPR   Dee_DEXSYM_CONSTEXPR
#define DEXSYM_ALIAS       Dee_DEXSYM_ALIAS
#define DEXSYM_HIDDEN      Dee_DEXSYM_HIDDEN
#define DEXSYM_PROPERTY    Dee_DEXSYM_PROPERTY
#ifndef CONFIG_BUILDING_DEEMON
#define DEX_BEGIN          Dee_DEX_BEGIN
#define DEX_MEMBER         Dee_DEX_MEMBER
#define DEX_MEMBER_F       Dee_DEX_MEMBER_F
#define DEX_MEMBER_NODOC   Dee_DEX_MEMBER_NODOC
#define DEX_MEMBER_F_NODOC Dee_DEX_MEMBER_F_NODOC
#define DEX_GETSET_F       Dee_DEX_GETSET_F
#define DEX_GETTER_F       Dee_DEX_GETTER_F
#define DEX_GETSET         Dee_DEX_GETSET
#define DEX_GETTER         Dee_DEX_GETTER
#define DEX_GETSET_F_NODOC Dee_DEX_GETSET_F_NODOC
#define DEX_GETTER_F_NODOC Dee_DEX_GETTER_F_NODOC
#define DEX_GETSET_NODOC   Dee_DEX_GETSET_NODOC
#define DEX_GETTER_NODOC   Dee_DEX_GETTER_NODOC
#define DEX_END            Dee_DEX_END
#endif /* !CONFIG_BUILDING_DEEMON */
#endif /* DEE_SOURCE */


/* Possible values for DEX symbol flags */
#define Dee_DEXSYM_NORMAL    Dee_MODSYM_FNORMAL
#define Dee_DEXSYM_READONLY  Dee_MODSYM_FREADONLY
#define Dee_DEXSYM_CONSTEXPR Dee_MODSYM_FCONSTEXPR
#define Dee_DEXSYM_ALIAS     Dee_MODSYM_FALIAS
#define Dee_DEXSYM_HIDDEN    Dee_MODSYM_FHIDDEN
#define Dee_DEXSYM_PROPERTY  Dee_MODSYM_FPROPERTY

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
struct Dee_dex_symbol {
	char const               *ds_name;   /* [1..1][SENTINEL(NULL)] Name of this symbol. */
	/*utf-8*/ char const     *ds_doc;    /* [0..1] An optional documentation string. */
	DeeObject                *ds_obj;    /* [0..1] The initial value of this symbol. */
	__UINTPTR_QUARTER_TYPE__  ds_flags;  /* Set of `Dee_DEXSYM_*'. */
	__UINTPTR_QUARTER_TYPE__  _ds_temp;  /* Used internally during initialization */
	__UINTPTR_HALF_TYPE__     _ds_index; /* Used internally during initialization */
};

union Dee_module_buildid;
struct Dee_module_dexinfo;
struct Dee_module_dexdata {
	struct Dee_module_object       *mdx_module;  /* [1..1][const] Associated DEX module descriptor */
	struct Dee_dex_symbol const    *mdx_export;  /* [1..mdx_module->mo_globalc][const] Raw export table of this DEX module */
	union Dee_module_buildid const *mdx_buildid; /* [0..1][const] 16-byte Build ID of the DEX module (if available) -- set to
	                                              * "ADDR(.note.gnu.build-id) + 16" if available, or to 16 random bytes if
	                                              * that is possible */
	char const                     *mdx_buildts; /* [0..1][const] Build timestamp string (optional, takes the form of `__DATE__ "|" __TIME__') */
	void *_mdx_pad1[4]; /* For future expansion (must be 0-initialized by DEX modules) */

	/* [0..1][const] Optional initializer/finalizer/clear callbacks. */
	WUNUSED_T int (DCALL *mdx_init)(void);
	void (DCALL *mdx_fini)(void);
	bool (DCALL *mdx_clear)(void);

	/* Internal fields... */
	void                           *mdx_handle; /* [?..?][const][owned] System-specific library handle (filled in during loading) */
	struct Dee_module_dexinfo      *mdx_info;   /* [0..1][const] Used internally. Initialize to "NULL" in DEX modules */
	void *_mdx_pad2[3]; /* For future expansion (must be 0-initialized by DEX modules) */
};

#if defined(CONFIG_BUILDING_DEEMON) || defined(CONFIG_BUILDING_DEX)
#ifdef CONFIG_HAVE___dex_start____AND___end
INTDEF __BYTE_TYPE__ __dex_start__[]; /*!export-*/
INTDEF __BYTE_TYPE__ _end[];          /*!export-*/
#define _Dee_MODULE_DEXDATA_INIT_LOADBOUNDS __dex_start__, _end - 1, { NULL, NULL, NULL }
#else /* CONFIG_HAVE___dex_start____AND___end */
#define _Dee_MODULE_DEXDATA_INIT_LOADBOUNDS NULL, NULL, { NULL, NULL, NULL }
#endif /* !CONFIG_HAVE___dex_start____AND___end */

#ifndef __ATTR_WEAK_IS_ATTR_SELECTANY
#define _Dee_PRIVATE_ELF_ATTR_WEAK __ATTR_WEAK
#else /* !__ATTR_WEAK_IS_ATTR_SELECTANY */
#define _Dee_PRIVATE_ELF_ATTR_WEAK /* nothing */
#endif /* __ATTR_WEAK_IS_ATTR_SELECTANY */

#ifdef CONFIG_HAVE___dex_buildid__
INTDEF _Dee_PRIVATE_ELF_ATTR_WEAK __BYTE_TYPE__ __dex_buildid__[]; /*!export-*/
#define _Dee_MODULE_DEXDATA_INIT_BUILDID (union Dee_module_buildid const *)(__dex_buildid__ + 16)
#elif defined(CONFIG_HAVE___dex_builduuid64__)
DECL_END
#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */
DECL_BEGIN
INTDEF _Dee_PRIVATE_ELF_ATTR_WEAK __BYTE_TYPE__ __dex_builduuid64_0__[]; /*!export-*/
INTDEF _Dee_PRIVATE_ELF_ATTR_WEAK __BYTE_TYPE__ __dex_builduuid64_1__[]; /*!export-*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK      \
	PRIVATE __UINT64_TYPE__ const _dex_buildid[2] = { \
		(__UINT64_TYPE__)__dex_builduuid64_1__,       \
		(__UINT64_TYPE__)__dex_builduuid64_0__        \
	}
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define _Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK      \
	PRIVATE __UINT64_TYPE__ const _dex_buildid[2] = { \
		(__UINT64_TYPE__)__dex_builduuid64_0__,       \
		(__UINT64_TYPE__)__dex_builduuid64_1__        \
	}
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
#define _Dee_MODULE_DEXDATA_INIT_BUILDID (union Dee_module_buildid const *)_dex_buildid
#elif defined(CONFIG_HAVE___dex_builduuid32__)
DECL_END
#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */
DECL_BEGIN
INTDEF _Dee_PRIVATE_ELF_ATTR_WEAK __BYTE_TYPE__ __dex_builduuid32_0__[]; /*!export-*/
INTDEF _Dee_PRIVATE_ELF_ATTR_WEAK __BYTE_TYPE__ __dex_builduuid32_1__[]; /*!export-*/
INTDEF _Dee_PRIVATE_ELF_ATTR_WEAK __BYTE_TYPE__ __dex_builduuid32_2__[]; /*!export-*/
INTDEF _Dee_PRIVATE_ELF_ATTR_WEAK __BYTE_TYPE__ __dex_builduuid32_3__[]; /*!export-*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK      \
	PRIVATE __UINT32_TYPE__ const _dex_buildid[4] = { \
		(__UINT32_TYPE__)__dex_builduuid32_3__,       \
		(__UINT32_TYPE__)__dex_builduuid32_2__,       \
		(__UINT32_TYPE__)__dex_builduuid32_1__,       \
		(__UINT32_TYPE__)__dex_builduuid32_0__        \
	}
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define _Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK      \
	PRIVATE __UINT32_TYPE__ const _dex_buildid[4] = { \
		(__UINT32_TYPE__)__dex_builduuid32_0__,       \
		(__UINT32_TYPE__)__dex_builduuid32_1__,       \
		(__UINT32_TYPE__)__dex_builduuid32_2__,       \
		(__UINT32_TYPE__)__dex_builduuid32_3__        \
	}
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
#define _Dee_MODULE_DEXDATA_INIT_BUILDID (union Dee_module_buildid const *)_dex_buildid
#elif defined(CONFIG_HOST_WINDOWS) && defined(__PE__)
#define _Dee_MODULE_DEXDATA_INIT_BUILDID NULL /* Need neither since we can use "TimeDateStamp" from the PE header */
#else /* ... */
#define _Dee_MODULE_DEXDATA_INIT_BUILDID NULL
#if defined(__DATE__) && defined(__TIME__)
#define _Dee_MODULE_DEXDATA_INIT_BUILDTS __DATE__ "|" __TIME__
#endif /* __DATE__ && __TIME__ */
#endif /* !... */
#undef _Dee_PRIVATE_ELF_ATTR_WEAK

#ifndef _Dee_MODULE_DEXDATA_INIT_BUILDTS
#define _Dee_MODULE_DEXDATA_INIT_BUILDTS NULL
#endif /* !_Dee_MODULE_DEXDATA_INIT_BUILDTS */
#ifndef _Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK
#define _Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK /* nothing */
#endif /* !_Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK */
#endif /* CONFIG_BUILDING_DEEMON || CONFIG_BUILDING_DEX */

/* Helpers for defining DEX exports from C */
#ifdef CONFIG_BUILDING_DEX
#ifdef CONFIG_TRACE_REFCHANGES
#define _Dee_PRIVATE_DEX_OBJECT_HEAD_INIT 1, &DeeModuleDex_Type, DEE_REFTRACKER_UNTRACKED
#else /* CONFIG_TRACE_REFCHANGES */
#define _Dee_PRIVATE_DEX_OBJECT_HEAD_INIT 1, &DeeModuleDex_Type
#endif /* !CONFIG_TRACE_REFCHANGES */

#define Dee_DEX_BEGIN \
	INTERN struct Dee_dex_symbol _dex_symbols[] = {
#define Dee_DEX_MEMBER(name, obj, doc)           Dee_DEX_MEMBER_F(name, obj, Dee_DEXSYM_NORMAL, doc)
#define Dee_DEX_MEMBER_F(name, obj, flags, doc)  { name, DOC(doc), Dee_AsObject(obj), (flags) & ~(Dee_DEXSYM_PROPERTY), 0, 0 }
#define Dee_DEX_MEMBER_NODOC(name, obj)          Dee_DEX_MEMBER(name, obj, NULL)
#define Dee_DEX_MEMBER_F_NODOC(name, obj, flags) Dee_DEX_MEMBER_F(name, obj, flags, NULL)
#define Dee_DEX_GETSET_F(name, get, del, set, flags, doc)                                                \
	{ name, DOC(doc), Dee_AsObject(get), Dee_DEXSYM_PROPERTY | ((flags) & ~Dee_DEXSYM_READONLY), 0, 0 }, \
	{ NULL, NULL, Dee_AsObject(del), Dee_DEXSYM_NORMAL, 0, 0 },                                          \
	{ NULL, NULL, Dee_AsObject(set), Dee_DEXSYM_NORMAL, 0, 0 }
#define Dee_DEX_GETTER_F(name, get, flags, doc) \
	{ name, DOC(doc), Dee_AsObject(get), Dee_DEXSYM_READONLY | Dee_DEXSYM_PROPERTY | (flags), 0, 0 }
#define Dee_DEX_GETSET(name, get, del, set, doc)           Dee_DEX_GETSET_F(name, get, del, set, Dee_DEXSYM_NORMAL, doc)
#define Dee_DEX_GETTER(name, get, doc)                     Dee_DEX_GETTER_F(name, get, Dee_DEXSYM_NORMAL, doc)
#define Dee_DEX_GETSET_F_NODOC(name, get, del, set, flags) Dee_DEX_GETSET_F(name, get, del, set, flags, NULL)
#define Dee_DEX_GETTER_F_NODOC(name, get, flags)           Dee_DEX_GETTER_F(name, get, flags, NULL)
#define Dee_DEX_GETSET_NODOC(name, get, del, set)          Dee_DEX_GETSET(name, get, del, set, NULL)
#define Dee_DEX_GETTER_NODOC(name, get)                    Dee_DEX_GETTER(name, get, NULL)

#define _Dee_PRIVATE_NEXT_POWER_OF_2_IMPL_8(x)  (x | (x >> 1) | (x >> 2) | (x >> 3) | (x >> 4) | (x >> 5) | (x >> 6) | (x >> 7))
#define _Dee_PRIVATE_NEXT_POWER_OF_2_IMPL_16(x) (_Dee_PRIVATE_NEXT_POWER_OF_2_IMPL_8(x) | _Dee_PRIVATE_NEXT_POWER_OF_2_IMPL_8((x >> 8)))
#define _Dee_PRIVATE_NEXT_POWER_OF_2_16(x) ((_Dee_PRIVATE_NEXT_POWER_OF_2_IMPL_16((x - 1))) + 1)

#define Dee_DEX_END(init, fini, clear)                                      \
	};                                                                      \
	STATIC_ASSERT_MSG(COMPILER_LENOF(_dex_symbols) > 0,                     \
	                  "A DEX module must have at least 1 export");          \
	enum { _DEX_BUCKETM = _Dee_PRIVATE_NEXT_POWER_OF_2_16(COMPILER_LENOF(_dex_symbols) + 1) - 1 }; \
	PRIVATE struct Dee_module_symbol _dex_bucketv[_DEX_BUCKETM + 1] = {};   \
	Dee_MODULE_STRUCT(_dex_object_raw, COMPILER_LENOF(_dex_symbols));       \
	struct _dex_object {                                                    \
		struct Dee_gc_head_link m_head;                                     \
		struct _dex_object_raw  m_dex;                                      \
	};                                                                      \
	EXPDEF struct _dex_object DEX;                                          \
	_Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK;                               \
	PRIVATE struct Dee_module_dexdata _dex_data = {                         \
		/* .mdx_module  = */ (struct Dee_module_object *)&DEX.m_dex,        \
		/* .mdx_export  = */ _dex_symbols,                                  \
		/* .mdx_buildid = */ _Dee_MODULE_DEXDATA_INIT_BUILDID,              \
		/* .mdx_buildts = */ _Dee_MODULE_DEXDATA_INIT_BUILDTS,              \
		/* ._mdx_pad1   = */ { NULL, NULL, NULL, NULL },                    \
		/* .mdx_init    = */ init,                                          \
		/* .mdx_fini    = */ fini,                                          \
		/* .mdx_clear   = */ clear,                                         \
		/* .mdx_handle  = */ NULL, /* Init doesn't matter */                \
		/* .mdx_info    = */ NULL,                                          \
		/* ._mdx_pad2   = */ { NULL, NULL, NULL }                           \
	};                                                                      \
	PUBLIC struct _dex_object DEX = {{ _Dee_GC_HEAD_UNTRACKED_INIT }, {     \
		_Dee_PRIVATE_DEX_OBJECT_HEAD_INIT,                                  \
		/* .mo_absname = */ NULL, /* Filled at runtime... */                \
		/* .mo_absnode = */ { NULL, NULL, NULL },                           \
		/* .mo_libname = */ { NULL, { NULL }, { NULL, NULL, NULL }, NULL }, \
		/* .mo_dir     = */ NULL,                                           \
		/* .mo_init    = */ Dee_MODULE_INIT_UNINITIALIZED,                  \
		/* .mo_buildid = */ { {0} },                                        \
		/* .mo_flags   = */ Dee_MODULE_FNORMAL,                             \
		/* .mo_importc = */ 0,                                              \
		/* .mo_globalc = */ COMPILER_LENOF(_dex_symbols),                   \
		/* .mo_bucketm = */ _DEX_BUCKETM,                                   \
		/* .mo_bucketv = */ _dex_bucketv,                                   \
		_Dee_MODULE_INIT_mo_lock                                            \
		Dee_WEAKREF_SUPPORT_INIT,                                           \
		/* .mo_moddata = */ { &_dex_data },                                 \
		/* .mo_importv = */ NULL,                                           \
		_Dee_MODULE_DEXDATA_INIT_LOADBOUNDS,                                \
		/* .mo_globalv = */ { /* ... */ }                                   \
	}}
#endif /* CONFIG_BUILDING_DEX */


#ifdef CONFIG_BUILDING_DEEMON
/* Open loaded system "dex_handle" as a module object. The DEX module will have
 * already been hooked into "module_abstree_root", as well as having had its
 * "mo_dexdata" fully initialized.
 * If the system indicates that "dex_handle" had already been loaded under some
 * other name, the module corresponding to the existing load-instance will be
 * returned instead. Related to this, note that there is no "flags" parameter,
 * since this function intentionally ignores "DeeModule_IMPORT_F_ANONYM".
 *
 * @param: absname: The absolute, normalized filesystem name where "dex_handle"
 *                  was loaded from, with its trailing .dll/.so removed (as such,
 *                  this is the name under which a new DEX module should appear
 *                  within `module_abstree_root')
 * @param: dex_handle: The system library handle, as returned by `DeeSystem_DlOpenString()'
 * @return: * :   The newly loaded DEX module.
 * @return: NULL: An error was thrown (e.g. "dex_handle" does not refer to a DEX module) */
INTDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenDex(/*inherit(always)*/ /*utf-8*/ char *__restrict absname,
                  /*inherit(always)*/ void *dex_handle);

/* Invoke the "mdx_clear" operator on every loaded DEX module. */
INTDEF bool DCALL DeeModule_ClearDexModuleCaches(void);

/* Unload all loaded DEX modules. */
INTDEF void DCALL DeeModule_UnloadAllDexModules(void);
#endif /* CONFIG_BUILDING_DEEMON */


#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
typedef struct Dee_dex_object DeeDexObject;

struct Dee_dex_symbol {
	char const           *ds_name;  /* [1..1][SENTINEL(NULL)] Name of this symbol. */
	DeeObject            *ds_obj;   /* [0..1] The initial value of this symbol. */
	uintptr_t             ds_flags; /* Set of `Dee_DEXSYM_*'. */
	/*utf-8*/ char const *ds_doc;   /* [0..1] An optional documentation string. */
};



/* Helpers for defining DEX exports from C */
#ifndef CONFIG_BUILDING_DEEMON
#define Dee_DEX_BEGIN \
	PRIVATE struct Dee_dex_symbol _dex_symbols[] = {
#define Dee_DEX_MEMBER(name, obj, doc)           Dee_DEX_MEMBER_F(name, obj, Dee_DEXSYM_NORMAL, doc)
#define Dee_DEX_MEMBER_F(name, obj, flags, doc)  { name, Dee_AsObject(obj), (flags) & ~(Dee_DEXSYM_PROPERTY), DOC(doc) }
#define Dee_DEX_MEMBER_NODOC(name, obj)          Dee_DEX_MEMBER(name, obj, NULL)
#define Dee_DEX_MEMBER_F_NODOC(name, obj, flags) Dee_DEX_MEMBER_F(name, obj, flags, NULL)
#define Dee_DEX_GETSET_F(name, get, del, set, flags, doc)                                          \
	{ name, Dee_AsObject(get), Dee_DEXSYM_PROPERTY | ((flags) & ~Dee_DEXSYM_READONLY), DOC(doc) }, \
	{ NULL, Dee_AsObject(del), Dee_DEXSYM_NORMAL, NULL },                                          \
	{ NULL, Dee_AsObject(set), Dee_DEXSYM_NORMAL, NULL }
#define Dee_DEX_GETTER_F(name, get, flags, doc) \
	{ name, Dee_AsObject(get), Dee_DEXSYM_READONLY | Dee_DEXSYM_PROPERTY | (flags), DOC(doc) }
#define Dee_DEX_GETSET(name, get, del, set, doc)           Dee_DEX_GETSET_F(name, get, del, set, Dee_DEXSYM_NORMAL, doc)
#define Dee_DEX_GETTER(name, get, doc)                     Dee_DEX_GETTER_F(name, get, Dee_DEXSYM_NORMAL, doc)
#define Dee_DEX_GETSET_F_NODOC(name, get, del, set, flags) Dee_DEX_GETSET_F(name, get, del, set, flags, NULL)
#define Dee_DEX_GETTER_F_NODOC(name, get, flags)           Dee_DEX_GETTER_F(name, get, flags, NULL)
#define Dee_DEX_GETSET_NODOC(name, get, del, set)          Dee_DEX_GETSET(name, get, del, set, NULL)
#define Dee_DEX_GETTER_NODOC(name, get)                    Dee_DEX_GETTER(name, get, NULL)
#define Dee_DEX_END(init, fini, clear)   \
		{ NULL, NULL, 0, NULL }          \
	};                                   \
	STATIC_ASSERT_MSG(COMPILER_LENOF(_dex_symbols) > 0,            \
	                  "A dex module must have at least 1 export"); \
	PUBLIC struct Dee_dex DEX = {        \
		/* .d_symbols = */ _dex_symbols, \
		/* .d_init    = */ init,         \
		/* .d_fini    = */ fini,         \
		/* .d_clear   = */ clear         \
	}
#endif /* !CONFIG_BUILDING_DEEMON */



struct Dee_dex {
	/* The extension descriptor structure that must be
	 * exported by the extension module under the name `DEX'. */
	struct Dee_dex_symbol *d_symbols; /* [0..1] The vector of exported symbols.
	                                   * NOTE: Indices in this vector are re-used as global variable numbers. */
	/* Optional initializer/finalizer callbacks.
	 * When non-NULL, `d_init()' is invoked after globals have been.
	 * Extension modules are only unloaded before deemon itself terminates.
	 * NOTE: When executed, the `Dee_MODULE_FDIDINIT' hasn't been set yet,
	 *       but will be as soon as the function returns a value of ZERO(0).
	 *       Any other return value can be used to indicate an error having
	 *       been thrown, which in turn will cause the caller to propagate
	 *       said error. */
	WUNUSED_T int (DCALL *d_init)(void);
	/* WARNING: `d_fini()' must not attempt to add more references to `self'.
	 *           When an extension module is supposed to get unloaded, it _has_
	 *           to be unloaded and there is no way around that! */
	void (DCALL *d_fini)(void);

	/* Called during the GC-cleanup phase near the end of deemon's execution cycle.
	 * This function should be implemented to clear global caches or object hooks.
	 * @return: true:  Something was cleared.
	 * @return: false: Nothing was cleared. (Same as not implementing this callback) */
	bool (DCALL *d_clear)(void);
};

struct Dee_dex_object {
	DeeModuleObject    d_module;       /* The underlying module. */
	struct Dee_dex    *d_dex;          /* [1..1][const_if(Dee_MODULE_FDIDLOAD)] The dex definition table exported by this extension.
	                                    * NOTE: This pointer is apart of the extension's static address space. */
	void              *d_handle;       /* [?..?][const_if(Dee_MODULE_FDIDLOAD)] System-specific library handle. */
	DeeDexObject     **d_pself;        /* [1..1][== self][0..1][lock(INTERN(dex_lock))] Dex self-pointer. */
	DREF DeeDexObject *d_next;         /* [0..1][lock(INTERN(dex_lock))] Extension initialized before this one.
	                                    * During finalization, extensions are unloaded in reverse order. */
};

DDATDEF DeeTypeObject DeeDex_Type;
#define DeeDex_Check(ob)      DeeObject_InstanceOf(ob, &DeeDex_Type)
#define DeeDex_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeDex_Type)

#ifndef CONFIG_BUILDING_DEEMON
/* Implemented by the extension:
 * >> PUBLIC struct Dee_dex DEX = { ... }; */
EXPDEF struct Dee_dex DEX;
#endif /* CONFIG_BUILDING_DEEMON */

#ifdef CONFIG_BUILDING_DEEMON
/* Try to load an extension file.
 * NOTE: This isn't where the dex gets initialized!
 * @return:  0: The extension was successfully loaded.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1)) int DCALL
dex_load_handle(DeeDexObject *__restrict self,
                /*inherited(always)*/ void *handle,
                DeeObject *__restrict input_file);

/* Initialize the given dex module. */
INTDEF WUNUSED NONNULL((1)) int DCALL dex_initialize(DeeDexObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDex_New(DeeObject *__restrict name);

/* Clear global data caches of all loaded dex modules. */
INTDEF bool DCALL DeeDex_Cleanup(void);

/* Unload all loaded dex modules. */
INTDEF void DCALL DeeDex_Finalize(void);
#endif /* CONFIG_BUILDING_DEEMON */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_END
#endif /* !CONFIG_NO_DEX */

#endif /* !GUARD_DEEMON_DEX_H */
