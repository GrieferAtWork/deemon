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
#ifndef GUARD_DEEMON_EXECUTE_MODPATH_C
#define GUARD_DEEMON_EXECUTE_MODPATH_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/code.h>
#include <deemon/dec.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/heap.h>
#include <deemon/method-hints.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/types.h>
#include <deemon/util/atomic-ref.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/nrlock.h>

#include <hybrid/align.h>           /* IS_POWER_OF_TWO */
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/host.h>            /* __i386__, __x86_64__ */
#include <hybrid/typecore.h>        /* __BYTE_TYPE__, __SIZEOF_SIZE_T__, __UINTPTR_HALF_TYPE__ */

#include <stdarg.h>  /* va_end, va_list, va_start */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint16_t, uint32_t, uint64_t, uintptr_t */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HAVE_LINK_H
#include <link.h>
#endif /* CONFIG_HAVE_LINK_H */

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else
#   define PATH_MAX 260
#endif
#endif /* !PATH_MAX */

#ifndef DLOPEN_NULL_FLAGS
#if defined(CONFIG_HAVE_RTLD_GLOBAL)
#define DLOPEN_NULL_FLAGS RTLD_GLOBAL
#elif defined(CONFIG_HAVE_RTLD_LOCAL)
#define DLOPEN_NULL_FLAGS RTLD_LOCAL
#else /* ... */
#define DLOPEN_NULL_FLAGS 0
#endif /* !... */
#endif /* !DLOPEN_NULL_FLAGS */

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#include <deemon/dex.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/mapfile.h>

#include <hybrid/sequence/rbtree.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifndef CONFIG_EXPERIMENTAL_MMAP_DEC
#include <deemon/compiler/dec.h>
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifdef __ARCH_PAGESIZE_MIN
#define SAMEPAGE(a, b) (((uintptr_t)(a) & ~(__ARCH_PAGESIZE_MIN - 1)) == ((uintptr_t)(b) & ~(__ARCH_PAGESIZE_MIN - 1)))
#else /* __ARCH_PAGESIZE_MIN */
#define SAMEPAGE(a, b) false
#endif /* !__ARCH_PAGESIZE_MIN */

#ifdef DeeSystem_HAVE_FS_ICASE
#ifndef CONFIG_HAVE_strcasecmp
#define CONFIG_HAVE_strcasecmp
#undef strcasecmp
#define strcasecmp dee_strcasecmp
DeeSystem_DEFINE_strcasecmp(dee_strcasecmp)
#endif /* !CONFIG_HAVE_strcasecmp */

#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#undef memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */

#define fs_strcmp(a, b)    strcasecmp(a, b)
#define fs_memcmp(a, b, n) memcasecmp(a, b, n)
#define fs_bcmp(a, b, n)   memcasecmp(a, b, n)
#else /* DeeSystem_HAVE_FS_ICASE */
#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#define fs_strcmp(a, b)    strcmp(a, b)
#define fs_memcmp(a, b, n) memcmp(a, b, n)
#define fs_bcmp(a, b, n)   bcmp(a, b, n)
#endif /* !DeeSystem_HAVE_FS_ICASE */

#define FS_DeeString_EqualsSTR(lhs, rhs)             \
	(DeeString_SIZE(lhs) == DeeString_SIZE(rhs) &&   \
	 fs_bcmp(DeeString_STR(lhs), DeeString_STR(rhs), \
	         DeeString_SIZE(lhs) * sizeof(char)) == 0)

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
FS_DeeString_LessSTR(DeeStringObject *lhs,
                     DeeStringObject *rhs) {
	size_t lhs_len = DeeString_SIZE(lhs);
	size_t rhs_len = DeeString_SIZE(rhs);
	size_t common = lhs_len < rhs_len ? lhs_len : rhs_len;
	int diff = fs_memcmp(DeeString_STR(lhs), DeeString_STR(rhs), common * sizeof(char));
	if (diff != 0)
		return diff < 0;
	return lhs_len < rhs_len;
}



/* Figure out mechanisms that (in addition to `__dex_start__' and `_end')
 * can be used to determine the bounding min/max address bounds of some
 * dex module, and (when those bounds shouldn't be used), figure out how
 * we can ask the OS what module is located at some specific address. */

/* Supported OS-APIs for getting an opaque identifier(IDENT) for the SHLIB at a specific address(ADDR) */
#undef Dee_DEXATADDR_USE__GetModuleHandleExW       /* GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, ADDR, &IDENT) */
#undef Dee_DEXATADDR_USE__dlgethandle              /* IDENT = dlgethandle(ADDR) */
#undef Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP /* IDENT = dlgethandle(ADDR) */
#undef Dee_DEXATADDR_USE__dladdr__dli_fname        /* dladdr(ADDR) -> IDENT = Dl_info::dli_fname */

/* Supported OS-APIs for determining the bounds of a DEX module.
 * If none of these are supported / work at runtime:
 * - DEX modules are allowed to pre-initialize their module's `mo_minaddr' / `mo_maxaddr' fields
 *   (as is the case when the dex module was built using a compiler/linker combo that supports
 *   `CONFIG_HAVE___dex_start____AND___end'). If these fields are non-NULL and appear to make
 *   sense (in regards to the module's "DEX" symbol), then `Dee_DEXBOUNDS_USE__*' isn't used at
 *   all and the pre-set fields are taken as the truth
 * - If the DEX does not pre-initialize `mo_minaddr' / `mo_maxaddr', and none of these methods
 *   are supported / work at runtime, then:
 *   - Try to determine the "IDENT" of the module from `Dee_DEXATADDR_USE__*' APIs above.
 *     - If this works, use binary search on the host address space to determine the probable
 *       bounds of the DEX module ("probable" because we might miss or fail to notice gaps in
 *       the DEX SHLIB's memory map). As such, this binary search is only used to initialize
 *       the module's `mo_minaddr' / `mo_maxaddr' fields since those can also be accessed
 *       directly.
 *     - At this, set the module's `_Dee_MODULE_FNOADDR' flag and insert the module into
 *       "dex_byaddr_tree", which works somewhat different from "module_byaddr_tree", in
 *       that it uses the `Dee_DEXATADDR_USE__*'-based `Dee_dexataddr_t' as its key.
 *       This tree will still be searched by `DeeModule_OfPointer()', so it effectively
 *       behaves the same as "module_byaddr_tree", and we're able to support configurations:
 *       - Where system SHLIB memory mappings contain holes
 *       - Where the system's SHLIB only allows `Dee_DEXATADDR_USE__*' to be implemented
 *   - If none of the `Dee_DEXATADDR_USE__*' implementations are supported or work,
 *     that is is rather unfortunate, but we still try to accommodate this case:
 *     - In this case, `DeeModule_InitDexBounds_with_exports()' is used to initialize
 *       `mo_minaddr' / `mo_maxaddr' using a best-effort approach by looking at the
 *       bounds of everything pointed at by the module's "DEX" symbol.
 *       The bounds determined using this method are then considered as truthy and are
 *       the only qualification that will be usable to detect the module's bounds.
 *
 * NOTES:
 * - The fallback `DeeModule_InitDexBounds_with_exports()' impl is VERY MUCH prone to failure
 * - Both `Dee_DEXBOUNDS_USE__*' amd `Dee_DEXATADDR_USE__*' can deal with multiple implementations
 *   being active at the same time. The more, the merrier (and the better the chance that get it
 *   right), so as many implementations as possible should be active.
 */
#undef Dee_DEXBOUNDS_USE__GetModuleInformation /* GetModuleInformation(Dee_module_dexdata::mdx_handle) */
#undef Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP /* dl_iterate_phdr -> dl_phdr_info::dlpi_addr == link_map::l_addr -> use PHDR bounds */
#undef Dee_DEXBOUNDS_USE__dl_iterate_phdr      /* dl_iterate_phdr -> Find module with PHDRs containing dlsym("DEX") -> use PHDR bounds */
#undef Dee_DEXBOUNDS_USE__xdlmodule_info       /* KOSmk3: xdlmodule_info */

#ifdef DeeSystem_DlOpen_USE_LoadLibrary

#ifdef CONFIG_HOST_WINDOWS
#define Dee_DEXBOUNDS_USE__GetModuleInformation
#define Dee_DEXATADDR_USE__GetModuleHandleExW
#endif /* CONFIG_HOST_WINDOWS */

#elif defined(DeeSystem_DlOpen_USE_dlopen)

#ifdef CONFIG_HAVE_dl_iterate_phdr
#if defined(CONFIG_HAVE_dladdr1__RTLD_DL_LINKMAP) && defined(CONFIG_HAVE_struct__link_map__l_addr)
#define Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP
#endif /* CONFIG_HAVE_dladdr1__RTLD_DL_LINKMAP && CONFIG_HAVE_struct__link_map__l_addr */
#define Dee_DEXBOUNDS_USE__dl_iterate_phdr
#endif /* CONFIG_HAVE_dl_iterate_phdr */

#if defined(__KOS_VERSION__) && (__KOS_VERSION__ >= 300 && __KOS_VERSION__ < 400)
#define Dee_DEXBOUNDS_USE__xdlmodule_info
#endif /* __KOS_VERSION__ >= 300 && __KOS_VERSION__ < 400 */

#ifdef CONFIG_HAVE_dlgethandle
#define Dee_DEXATADDR_USE__dlgethandle
#endif /* CONFIG_HAVE_dlgethandle */

#ifdef CONFIG_HAVE_dladdr1__RTLD_DL_LINKMAP
#define Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP
#endif /* CONFIG_HAVE_dladdr1__RTLD_DL_LINKMAP */

#ifdef CONFIG_HAVE_dladdr
#define Dee_DEXATADDR_USE__dladdr__dli_fname
#endif /* CONFIG_HAVE_dladdr */

#endif /* ... */





/* Implement `Dee_dexataddr()' using OS APIs (if possible) */
#undef HAVE_Dee_dexataddr_t
#undef HAVE_Dee_dexataddr_t_SINGLE_IMPL
#undef HAVE_Dee_dexataddr_t_IS_POINTER

#define HAVE_Dee_dexataddr_t_IS_POINTER
#if (defined(Dee_DEXATADDR_USE__GetModuleHandleExW) +       \
     defined(Dee_DEXATADDR_USE__dlgethandle) +              \
     defined(Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP) + \
     defined(Dee_DEXATADDR_USE__dladdr__dli_fname)) == 1
#define HAVE_Dee_dexataddr_t_SINGLE_IMPL
#endif /* ... */
#if (defined(Dee_DEXATADDR_USE__GetModuleHandleExW) +       \
     defined(Dee_DEXATADDR_USE__dlgethandle) +              \
     defined(Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP) + \
     defined(Dee_DEXATADDR_USE__dladdr__dli_fname)) > 1
#undef HAVE_Dee_dexataddr_t_IS_POINTER
#endif /* ... */

typedef struct {
#ifdef Dee_DEXATADDR_USE__GetModuleHandleExW
#define HAVE_Dee_dexataddr_t
	HANDLE daa_ntModule; /* [0..1] */
#endif /* Dee_DEXATADDR_USE__GetModuleHandleExW */
#ifdef Dee_DEXATADDR_USE__dlgethandle
#define HAVE_Dee_dexataddr_t
	void *daa_dlhandle;  /* [0..1] */
#endif /* Dee_DEXATADDR_USE__dlgethandle */
#ifdef Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP
#define HAVE_Dee_dexataddr_t
	void *daa_linkmap;   /* [0..1] */
#endif /* Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP */
#ifdef Dee_DEXATADDR_USE__dladdr__dli_fname
#define HAVE_Dee_dexataddr_t
#ifndef CONFIG_HAVE_dlinfo__dli_fname__is_size_ptr
#undef HAVE_Dee_dexataddr_t_IS_POINTER
#endif /* !CONFIG_HAVE_dlinfo__dli_fname__is_size_ptr */
	byte_t daa_dlinfo[sizeof(((Dl_info *)0)->dli_fname)];
#define Dee_dexataddr__dlinfo__getname(self) \
	((Dl_info *)((self)->daa_dlinfo - offsetof(Dl_info, dli_fname)))->dli_fname
#define Dee_dexataddr__dlinfo__setinfo(self, v)                                    \
	(void)memcpy((self)->daa_dlinfo, (byte_t *)(v) + offsetof(Dl_info, dli_fname), \
	             sizeof(((Dl_info *)0)->dli_fname));
#endif /* Dee_DEXATADDR_USE__dladdr__dli_fname */
#ifndef HAVE_Dee_dexataddr_t
#undef HAVE_Dee_dexataddr_t_IS_POINTER
	char _daa_placeholder;
#endif /* !HAVE_Dee_dexataddr_t */
} Dee_dexataddr_t;

#ifdef HAVE_Dee_dexataddr_t
#ifdef Dee_DEXATADDR_USE__dladdr__dli_fname
#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */
#endif /* Dee_DEXATADDR_USE__dladdr__dli_fname */

/* Compare `lhs' and `rhs'
 * Both of the given info-blocks must be initialized */
LOCAL WUNUSED NONNULL((1, 2)) int DCALL
Dee_dexataddr_compare(Dee_dexataddr_t const *__restrict lhs,
                      Dee_dexataddr_t const *__restrict rhs) {
#ifdef HAVE_Dee_dexataddr_t_SINGLE_IMPL
#ifdef Dee_DEXATADDR_USE__GetModuleHandleExW
	return Dee_Compare((uintptr_t)lhs->daa_ntModule,
	                   (uintptr_t)rhs->daa_ntModule);
#elif defined(Dee_DEXATADDR_USE__dlgethandle)
	return Dee_Compare((uintptr_t)lhs->daa_dlhandle,
	                   (uintptr_t)rhs->daa_dlhandle);
#elif defined(Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP)
	return Dee_Compare((uintptr_t)lhs->daa_linkmap,
	                   (uintptr_t)rhs->daa_linkmap);
#elif defined(Dee_DEXATADDR_USE__dladdr__dli_fname)
	return strcmp(Dee_dexataddr__dlinfo__getname(lhs),
	              Dee_dexataddr__dlinfo__getname(rhs));
#else /* ... */
#error "Bad configuration"
#endif /* !... */
#else /* HAVE_Dee_dexataddr_t_SINGLE_IMPL */
#ifdef Dee_DEXATADDR_USE__GetModuleHandleExW
	if (lhs->daa_ntModule && rhs->daa_ntModule) {
		return Dee_Compare((uintptr_t)lhs->daa_ntModule,
		                   (uintptr_t)rhs->daa_ntModule);
	} else
#endif /* Dee_DEXATADDR_USE__GetModuleHandleExW */
#ifdef Dee_DEXATADDR_USE__dlgethandle
	if (lhs->daa_dlhandle && rhs->daa_dlhandle) {
		return Dee_Compare((uintptr_t)lhs->daa_dlhandle,
		                   (uintptr_t)rhs->daa_dlhandle);
	} else
#endif /* Dee_DEXATADDR_USE__dlgethandle */
#ifdef Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP
	if (lhs->daa_linkmap && rhs->daa_linkmap) {
		return Dee_Compare((uintptr_t)lhs->daa_linkmap,
		                   (uintptr_t)rhs->daa_linkmap);
	} else
#endif /* Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP */
#ifdef Dee_DEXATADDR_USE__dladdr__dli_fname
	if (Dee_dexataddr__dlinfo__getname(lhs) && Dee_dexataddr__dlinfo__getname(rhs)) {
		return strcmp(Dee_dexataddr__dlinfo__getname(lhs),
		              Dee_dexataddr__dlinfo__getname(rhs));
	} else
#endif /* Dee_DEXATADDR_USE__dladdr__dli_fname */
	{
		return memcmp(lhs, rhs, sizeof(Dee_dexataddr_t));
	}
#endif /* !HAVE_Dee_dexataddr_t_SINGLE_IMPL */
}


/* Initialize "self" with information about the module at "addr".
 * @return: true:  Success
 * @return: false: Failure */
LOCAL WUNUSED NONNULL((1)) bool DCALL
Dee_dexataddr_init_fromaddr(Dee_dexataddr_t *__restrict self, void const *addr) {
#ifndef HAVE_Dee_dexataddr_t_SINGLE_IMPL
	bzero(self, sizeof(*self));
#endif /* !HAVE_Dee_dexataddr_t_SINGLE_IMPL */
#ifdef Dee_DEXATADDR_USE__GetModuleHandleExW
	{
		BOOL bOk;
		DBG_ALIGNMENT_DISABLE();
		bOk = GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		                         GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		                         (LPCWSTR)addr, (HMODULE *)&self->daa_ntModule);
		DBG_ALIGNMENT_ENABLE();
		if (bOk)
			return true;
	}
#endif /* Dee_DEXATADDR_USE__GetModuleHandleExW */
#ifdef Dee_DEXATADDR_USE__dlgethandle
	if ((lhs->daa_dlhandle = (void *)dlgethandle(addr, DLGETHANDLE_FNORMAL)) != NULL)
		return true;
#endif /* Dee_DEXATADDR_USE__dlgethandle */
#ifdef Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP
	{
		Dl_info dli;
		struct link_map *ptr_lm = NULL;
		if (dladdr1(addr, &dli, (void **)&ptr_lm, RTLD_DL_LINKMAP) && ptr_lm) {
			self->daa_linkmap = (void *)ptr_lm;
			return true;
		}
	}
#endif /* Dee_DEXATADDR_USE__dladdr1__RTLD_DL_LINKMAP */
#ifdef Dee_DEXATADDR_USE__dladdr__dli_fname
	{
		Dl_info dli;
		if (dladdr(addr, &dli) && dli.dli_fname && *dli.dli_fname) {
//			Dee_DPRINTF("[debug] dladdr(%p) -> %q\n", addr, dli.dli_fname);
			Dee_dexataddr__dlinfo__setinfo(self, &dli);
			return true;
		}
	}
#endif /* Dee_DEXATADDR_USE__dladdr__dli_fname */
	return false;
}
#endif /* HAVE_Dee_dexataddr_t */








/* Implement `DeeModule_InitDexBounds()' using OS APIs (if possible) */
#ifdef Dee_DEXBOUNDS_USE__GetModuleInformation
typedef struct {
	LPVOID lpBaseOfDll;
	DWORD SizeOfImage;
	LPVOID EntryPoint;
} NT_MODULEINFO;

typedef int (WINAPI *LPGETMODULEINFORMATION)(void *hProcess, void *hModule,
                                             NT_MODULEINFO *lpmodinfo, uint32_t cb);
PRIVATE LPGETMODULEINFORMATION pdyn_GetModuleInformation = NULL;
PRIVATE char const name_GetModuleInformation[] = "GetModuleInformation";
PRIVATE LPGETMODULEINFORMATION DCALL get_GetModuleInformation(void) {
	HMODULE hMod;
	static WCHAR const wKernel32[] = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
	static WCHAR const wPsapi[] = { 'P', 'S', 'A', 'P', 'I', 0 };
	static WCHAR const wKernel32_dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };
	static WCHAR const wPsapi_dll[] = { 'P', 's', 'A', 'p', 'i', '.', 'd', 'l', 'l', 0 };
	LPGETMODULEINFORMATION result = atomic_read(&pdyn_GetModuleInformation);
	HMODULE hModule;
	if (ITER_ISOK((uintptr_t)(Dee_funptr_t)result))
		return result;
	if ((uintptr_t)(Dee_funptr_t)result == (uintptr_t)ITER_DONE)
		return NULL;
	hModule = GetModuleHandleW(wKernel32);
	if (hModule) {
		*(FARPROC *)&result = GetProcAddress(hModule, name_GetModuleInformation);
		if (result != NULL)
			goto remember_result;
	}
	hModule = GetModuleHandleW(wPsapi);
	if (hModule) {
		*(FARPROC *)&result = GetProcAddress(hModule, name_GetModuleInformation);
		if (result != NULL)
			goto remember_result;
	}
	hMod = LoadLibraryW(wKernel32_dll);
	if (hMod != NULL) {
		*(FARPROC *)&result = GetProcAddress(hMod, name_GetModuleInformation);
		if (result != NULL)
			goto remember_result;
		(void)FreeLibrary(hMod);
	}
	hMod = LoadLibraryW(wPsapi_dll);
	if (hMod != NULL) {
		*(FARPROC *)&result = GetProcAddress(hMod, name_GetModuleInformation);
		if (result != NULL)
			goto remember_result;
		(void)FreeLibrary(hMod);
	}
	Dee_DPRINTF("[RT][dex] Warning: Unable to locate 'GetModuleInformation' in 'Kernel32.dll' or 'PsApi.dll': %R",
	            DeeSystem_DlError());
	result = (LPGETMODULEINFORMATION)(Dee_funptr_t)(uintptr_t)(void *)ITER_DONE;
	atomic_write(&pdyn_GetModuleInformation, result);
	return NULL;
remember_result:
	atomic_write(&pdyn_GetModuleInformation, result);
	return result;
}
#endif /* Dee_DEXBOUNDS_USE__GetModuleInformation */



#undef HAVE_DeeModule_InitDexBounds
#ifdef Dee_DEXBOUNDS_USE__GetModuleInformation
#define HAVE_DeeModule_InitDexBounds
#endif /* Dee_DEXBOUNDS_USE__GetModuleInformation */
#ifdef Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP
#define HAVE_DeeModule_InitDexBounds
#endif /* Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP */
#ifdef Dee_DEXBOUNDS_USE__dl_iterate_phdr
#define HAVE_DeeModule_InitDexBounds
#endif /* Dee_DEXBOUNDS_USE__dl_iterate_phdr */
#ifdef Dee_DEXBOUNDS_USE__xdlmodule_info
#define HAVE_DeeModule_InitDexBounds
#endif /* Dee_DEXBOUNDS_USE__xdlmodule_info */
#ifndef HAVE_DeeModule_InitDexBounds
#define DeeModule_InitDexBounds(self) false
#else /* !HAVE_DeeModule_InitDexBounds */

#ifdef Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP
struct initialize_dexdata_minmax_iterate_with_linkmap_data {
	DeeModuleObject *iddmmiwld_mod;  /* [1..1] The module that we're looking for */
	uintptr_t        iddmmiwld_addr; /* struct link_map::l_addr */
};
PRIVATE int
initialize_dexdata_minmax_iterate_with_linkmap_cb(struct dl_phdr_info *info,
                                                  size_t size, void *cookie) {
	__UINTPTR_HALF_TYPE__ i;
	byte_t const *phdr_minaddr, *phdr_maxaddr;
	struct initialize_dexdata_minmax_iterate_with_linkmap_data *data;
	DeeModuleObject *mod;
	data = (struct initialize_dexdata_minmax_iterate_with_linkmap_data *)cookie;
	if unlikely(size < COMPILER_OFFSETAFTER(struct dl_phdr_info, dlpi_phnum))
		return 0;
	if (data->iddmmiwld_addr != (uintptr_t)info->dlpi_addr)
		return 0; /* Some other module... */
	mod = data->iddmmiwld_mod;
	phdr_minaddr = (byte_t const *)-1;
	phdr_maxaddr = (byte_t const *)0;
	for (i = 0; i < info->dlpi_phnum; ++i) {
		byte_t const *minaddr = (byte_t const *)info->dlpi_phdr[i].p_vaddr + (uintptr_t)info->dlpi_addr;
		byte_t const *maxaddr = minaddr + (uintptr_t)info->dlpi_phdr[i].p_memsz - 1;
		if (phdr_minaddr > minaddr)
			phdr_minaddr = minaddr;
		if (phdr_maxaddr < maxaddr)
			phdr_maxaddr = maxaddr;
	}

	/* Update min/max bounds */
	if (mod->mo_minaddr > phdr_minaddr)
		mod->mo_minaddr = phdr_minaddr;
	if (mod->mo_maxaddr < phdr_maxaddr)
		mod->mo_maxaddr = phdr_maxaddr;
	return 0;
}
#endif /* Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP */


#ifdef Dee_DEXBOUNDS_USE__dl_iterate_phdr
PRIVATE int
initialize_dexdata_minmax_iterate_cb(struct dl_phdr_info *info,
                                     size_t size, void *cookie) {
	__UINTPTR_HALF_TYPE__ i;
	byte_t const *phdr_minaddr, *phdr_maxaddr;
	DeeModuleObject *mod = (DeeModuleObject *)cookie;
	if unlikely(size < COMPILER_OFFSETAFTER(struct dl_phdr_info, dlpi_phnum))
		return 0;
	phdr_minaddr = (byte_t const *)-1;
	phdr_maxaddr = (byte_t const *)0;
	for (i = 0; i < info->dlpi_phnum; ++i) {
		byte_t const *minaddr = (byte_t const *)info->dlpi_phdr[i].p_vaddr + (uintptr_t)info->dlpi_addr;
		byte_t const *maxaddr = minaddr + (uintptr_t)info->dlpi_phdr[i].p_memsz - 1;
		if (phdr_minaddr > minaddr)
			phdr_minaddr = minaddr;
		if (phdr_maxaddr < maxaddr)
			phdr_maxaddr = maxaddr;
	}

	if ((byte_t const *)mod < phdr_minaddr)
		return 0; /* Some other module... */
	if ((byte_t const *)mod > phdr_maxaddr)
		return 0; /* Some other module... */

	/* Update min/max bounds */
	if (mod->mo_minaddr > phdr_minaddr)
		mod->mo_minaddr = phdr_minaddr;
	if (mod->mo_maxaddr < phdr_maxaddr)
		mod->mo_maxaddr = phdr_maxaddr;
	return 0;
}
#endif /* Dee_DEXBOUNDS_USE__dl_iterate_phdr */


PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeModule_InitDexBounds(DeeModuleObject *__restrict self) {
#ifdef Dee_DEXBOUNDS_USE__GetModuleInformation
	LPGETMODULEINFORMATION pGetModuleInformation = get_GetModuleInformation();
	if (pGetModuleInformation) {
		BOOL bOk;
		NT_MODULEINFO modinfo;
		bzero(&modinfo, sizeof(modinfo));
		DBG_ALIGNMENT_DISABLE();
		bOk = (*get_GetModuleInformation())(GetCurrentProcess(),
		                                    self->mo_moddata.mo_dexdata->mdx_handle,
		                                    &modinfo, sizeof(modinfo));
		if (!bOk) {
			DWORD dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			Dee_DPRINTF("[RT][dex] Warning: Failed to GetModuleInformation() for module %q: %u",
			            self->mo_absname, (unsigned int)dwError);
		} else {
			DBG_ALIGNMENT_ENABLE();
			self->mo_minaddr = (byte_t const *)modinfo.lpBaseOfDll;
			self->mo_maxaddr = (byte_t const *)modinfo.lpBaseOfDll + modinfo.SizeOfImage - 1;
			if (self->mo_minaddr < self->mo_maxaddr)
				return true;
		}
	}
#endif /* Dee_DEXBOUNDS_USE__GetModuleInformation */

#ifdef Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP
	{
		Dl_info dli;
		struct link_map *ptr_lm = NULL;
		if (dladdr1((void *)self, &dli, (void **)&ptr_lm, RTLD_DL_LINKMAP) && ptr_lm) {
			struct initialize_dexdata_minmax_iterate_with_linkmap_data data;
			data.iddmmiwld_addr = ptr_lm->l_addr;
			data.iddmmiwld_mod  = self;
			self->mo_minaddr = (byte_t const *)-1;
			self->mo_maxaddr = (byte_t const *)0;
			(void)dl_iterate_phdr(&initialize_dexdata_minmax_iterate_with_linkmap_cb, (void *)&data);
			if likely(self->mo_minaddr <= self->mo_maxaddr)
				return true;
			Dee_DPRINTF("[RT][dex] Warning: Failed to dl_iterate_phdr()-with-link_map-find module %q", self->mo_absname);
		}
	}
#endif /* Dee_DEXBOUNDS_USE__dl_iterate_phdr__AND__dladdr1__RTLD_DL_LINKMAP */

#ifdef Dee_DEXBOUNDS_USE__dl_iterate_phdr
	self->mo_minaddr = (byte_t const *)-1;
	self->mo_maxaddr = (byte_t const *)0;
	(void)dl_iterate_phdr(&initialize_dexdata_minmax_iterate_cb, (void *)self);
	if likely(self->mo_minaddr <= self->mo_maxaddr)
		return true;
	Dee_DPRINTF("[RT][dex] Warning: Failed to dl_iterate_phdr()-find module %q", self->mo_absname);
#endif /* Dee_DEXBOUNDS_USE__dl_iterate_phdr */

#ifdef Dee_DEXBOUNDS_USE__xdlmodule_info
	{
		struct module_basic_info info;
		if (xdlmodule_info(self->mdx_handle, MODULE_INFO_CLASS_BASIC, &info, sizeof(info)) < sizeof(info)) {
			Dee_DPRINTF("[RT][dex] Warning: Failed to xdlmodule_info() for module %q", self->mo_absname);
		} else {
			self->mo_minaddr = (byte_t const *)info.mi_segstart;
			self->mo_maxaddr = (byte_t const *)info.mi_segend - 1;
			return true;
		}
	}
#endif /* Dee_DEXBOUNDS_USE__xdlmodule_info */

	(void)self;
	return false;
}
#endif /* HAVE_DeeModule_InitDexBounds */


#ifdef HAVE_Dee_dexataddr_t
/* Use "Dee_dexataddr_t" to initialize dex-bounds */
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
DeeModule_InitDexBounds_with_dexataddr(DeeModuleObject *__restrict self,
                                       Dee_dexataddr_t const *__restrict ref) {
	Dee_dexataddr_t check;
	byte_t *minaddr_lo, *minaddr_hi;
	byte_t *endaddr_lo, *endaddr_hi;
	minaddr_lo = (byte_t *)0;
	minaddr_hi = (byte_t *)self;
	while (minaddr_lo < minaddr_hi) {
		byte_t *mid = minaddr_lo + (size_t)(minaddr_hi - minaddr_lo) / 2;
		if (Dee_dexataddr_init_fromaddr(&check, mid) &&
		    Dee_dexataddr_compare(ref, &check) == 0) {
			minaddr_hi = mid;
		} else {
			minaddr_lo = mid + 1;
		}
	}

	endaddr_lo = (byte_t *)self + offsetof(DeeModuleObject, mo_globalv) +
	             ((size_t)self->mo_globalc * sizeof(DeeObject *));
	endaddr_hi = (byte_t *)-1; /* Technically should +1 of this, but that's impossible */
	while (endaddr_lo < endaddr_hi) {
		byte_t *mid = endaddr_lo + (size_t)(endaddr_hi - endaddr_lo) / 2;
		if (Dee_dexataddr_init_fromaddr(&check, mid) &&
		    Dee_dexataddr_compare(ref, &check) == 0) {
			endaddr_lo = mid + 1;
		} else {
			endaddr_hi = mid;
		}
	}

	if (minaddr_hi < endaddr_lo) {
		self->mo_minaddr = minaddr_hi;
		self->mo_maxaddr = endaddr_lo - 1;
		return true;
	}
	return false;
}
#endif /* HAVE_Dee_dexataddr_t */


DECL_END

#define RBTREE(name)            module_abstree_##name
#define RBTREE_T                struct module_object
#define RBTREE_Tkey             char const *
#define RBTREE_NODEFIELD        mo_absnode
#define RBTREE_GETKEY(self)     (self)->mo_absname
#define RBTREE_KEY_LO(a, b)     (fs_strcmp(a, b) < 0)
#define RBTREE_KEY_EQ(a, b)     (fs_strcmp(a, b) == 0)
#define RBTREE_ISRED(self)      (atomic_read(&(self)->mo_flags) & Dee_MODULE_FABSRED)
#define RBTREE_SETRED(self)     atomic_or(&(self)->mo_flags, Dee_MODULE_FABSRED)
#define RBTREE_SETBLACK(self)   atomic_and(&(self)->mo_flags, ~Dee_MODULE_FABSRED)
#define RBTREE_FLIPCOLOR(self)  atomic_xor(&(self)->mo_flags, Dee_MODULE_FABSRED)
#define RBTREE_CC               DFCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#define RBTREE_OMIT_REMOVE
#define RBTREE_WANT_NEXTNODE
#include <hybrid/sequence/rbtree-abi.h>

#define RBTREE(name)            module_libtree_##name
#define RBTREE_T                struct Dee_module_libentry
#define RBTREE_Tkey             struct Dee_string_object *
#define RBTREE_NODEFIELD        mle_node
#define RBTREE_GETKEY(self)     (self)->mle_name
#define RBTREE_KEY_LO(a, b)     FS_DeeString_LessSTR(a, b)
#define RBTREE_KEY_EQ(a, b)     FS_DeeString_EqualsSTR(a, b)
#define RBTREE_REDFIELD         mle_dat.mle_red
#define RBTREE_REDBIT           1
#define RBTREE_CC               DFCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#define RBTREE_OMIT_REMOVE
#define RBTREE_WANT_NEXTNODE
#define RBTREE_WANT_NEXTAFTER
#include <hybrid/sequence/rbtree-abi.h>

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
INTERN Dee_atomic_rwlock_t module_abstree_lock = Dee_ATOMIC_RWLOCK_INIT;
INTERN Dee_atomic_rwlock_t module_libtree_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define module_abstree_lock_reading()    Dee_atomic_rwlock_reading(&module_abstree_lock)
#define module_abstree_lock_writing()    Dee_atomic_rwlock_writing(&module_abstree_lock)
#define module_abstree_lock_tryread()    Dee_atomic_rwlock_tryread(&module_abstree_lock)
#define module_abstree_lock_trywrite()   Dee_atomic_rwlock_trywrite(&module_abstree_lock)
#define module_abstree_lock_canread()    Dee_atomic_rwlock_canread(&module_abstree_lock)
#define module_abstree_lock_canwrite()   Dee_atomic_rwlock_canwrite(&module_abstree_lock)
#define module_abstree_lock_waitread()   Dee_atomic_rwlock_waitread(&module_abstree_lock)
#define module_abstree_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&module_abstree_lock)
#define module_abstree_lock_read()       Dee_atomic_rwlock_read(&module_abstree_lock)
#define module_abstree_lock_write()      Dee_atomic_rwlock_write(&module_abstree_lock)
#define module_abstree_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&module_abstree_lock)
#define module_abstree_lock_upgrade()    Dee_atomic_rwlock_upgrade(&module_abstree_lock)
#define module_abstree_lock_downgrade()  Dee_atomic_rwlock_downgrade(&module_abstree_lock)
#define module_abstree_lock_endwrite()   Dee_atomic_rwlock_endwrite(&module_abstree_lock)
#define module_abstree_lock_endread()    Dee_atomic_rwlock_endread(&module_abstree_lock)
#define module_abstree_lock_end()        Dee_atomic_rwlock_end(&module_abstree_lock)

#define module_libtree_lock_reading()    Dee_atomic_rwlock_reading(&module_libtree_lock)
#define module_libtree_lock_writing()    Dee_atomic_rwlock_writing(&module_libtree_lock)
#define module_libtree_lock_tryread()    Dee_atomic_rwlock_tryread(&module_libtree_lock)
#define module_libtree_lock_trywrite()   Dee_atomic_rwlock_trywrite(&module_libtree_lock)
#define module_libtree_lock_canread()    Dee_atomic_rwlock_canread(&module_libtree_lock)
#define module_libtree_lock_canwrite()   Dee_atomic_rwlock_canwrite(&module_libtree_lock)
#define module_libtree_lock_waitread()   Dee_atomic_rwlock_waitread(&module_libtree_lock)
#define module_libtree_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&module_libtree_lock)
#define module_libtree_lock_read()       Dee_atomic_rwlock_read(&module_libtree_lock)
#define module_libtree_lock_write()      Dee_atomic_rwlock_write(&module_libtree_lock)
#define module_libtree_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&module_libtree_lock)
#define module_libtree_lock_upgrade()    Dee_atomic_rwlock_upgrade(&module_libtree_lock)
#define module_libtree_lock_downgrade()  Dee_atomic_rwlock_downgrade(&module_libtree_lock)
#define module_libtree_lock_endwrite()   Dee_atomic_rwlock_endwrite(&module_libtree_lock)
#define module_libtree_lock_endread()    Dee_atomic_rwlock_endread(&module_libtree_lock)
#define module_libtree_lock_end()        Dee_atomic_rwlock_end(&module_libtree_lock)


/* [0..n][lock(module_abstree_lock)] Tree of module abs names (absolute, normalized filesystem paths) */
INTERN RBTREE_ROOT(Dee_module_object) module_abstree_root = NULL;

/* [0..n][lock(module_libtree_lock)] Tree of module lib names (based on `DeeModule_GetLibPath()') */
INTERN RBTREE_ROOT(Dee_module_libentry) module_libtree_root = NULL;

/* [0..1] The currently set deemon LIBPATH */
PRIVATE Dee_ATOMIC_REF(DeeTupleObject) deemon_path = Dee_ATOMIC_REF_INIT(NULL);



/* Define structure for runtime-only dex info */
#ifdef HAVE_Dee_dexataddr_t
#define HAVE_Dee_module_dexinfo
struct Dee_module_dexinfo {
	Dee_dexataddr_t ddi_ataddr;
#ifdef HAVE_Dee_dexataddr_t_IS_POINTER
#define HAVE_Dee_module_dexinfo_IS_POINTER
#endif /* HAVE_Dee_dexataddr_t_IS_POINTER */
};

#ifdef HAVE_Dee_module_dexinfo_IS_POINTER
#ifdef __INTELLISENSE__
#define Dee_module_dexdata__getinfo(self) ((struct Dee_module_dexinfo *)&(self)->mdx_info)
#else /* __INTELLISENSE__ */
#define Dee_module_dexdata__getinfo(self) \
	((struct Dee_module_dexinfo *)((byte_t *)(self) + offsetof(struct Dee_module_dexdata, mdx_info)))
#endif /* !__INTELLISENSE__ */
#else /* HAVE_Dee_module_dexinfo_IS_POINTER */
#define Dee_module_dexdata__getinfo(self)    ((self)->mdx_info)
#define Dee_module_dexdata__setinfo(self, v) (void)((self)->mdx_info = (v))
#define Dee_module_dexinfo_alloc()    DeeObject_MALLOC(struct Dee_module_dexinfo)
#define Dee_module_dexinfo_free(self) DeeObject_FREE(self)
#endif /* !HAVE_Dee_module_dexinfo_IS_POINTER */
#endif /* HAVE_Dee_dexataddr_t */



#ifndef CONFIG_NO_THREADS
INTERN Dee_atomic_rwlock_t module_byaddr_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define module_byaddr_lock_reading()    Dee_atomic_rwlock_reading(&module_byaddr_lock)
#define module_byaddr_lock_writing()    Dee_atomic_rwlock_writing(&module_byaddr_lock)
#define module_byaddr_lock_tryread()    Dee_atomic_rwlock_tryread(&module_byaddr_lock)
#define module_byaddr_lock_trywrite()   Dee_atomic_rwlock_trywrite(&module_byaddr_lock)
#define module_byaddr_lock_canread()    Dee_atomic_rwlock_canread(&module_byaddr_lock)
#define module_byaddr_lock_canwrite()   Dee_atomic_rwlock_canwrite(&module_byaddr_lock)
#define module_byaddr_lock_waitread()   Dee_atomic_rwlock_waitread(&module_byaddr_lock)
#define module_byaddr_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&module_byaddr_lock)
#define module_byaddr_lock_read()       Dee_atomic_rwlock_read(&module_byaddr_lock)
#define module_byaddr_lock_write()      Dee_atomic_rwlock_write(&module_byaddr_lock)
#define module_byaddr_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&module_byaddr_lock)
#define module_byaddr_lock_upgrade()    Dee_atomic_rwlock_upgrade(&module_byaddr_lock)
#define module_byaddr_lock_downgrade()  Dee_atomic_rwlock_downgrade(&module_byaddr_lock)
#define module_byaddr_lock_endwrite()   Dee_atomic_rwlock_endwrite(&module_byaddr_lock)
#define module_byaddr_lock_endread()    Dee_atomic_rwlock_endread(&module_byaddr_lock)
#define module_byaddr_lock_end()        Dee_atomic_rwlock_end(&module_byaddr_lock)


/* [0..n][lock(module_byaddr_lock)]
 * Tree used by dee module, and dex modules without "_Dee_MODULE_FNOADDR"
 * NOTE: The "maybe(DREF)" here refers to the fact that:
 *       - DeeModuleDee_Type are **NOT** DREF
 *       - DeeModuleDex_Type are DREF
 *       (other types of modules not appear in this tree) */
#undef HAVE_module_byaddr_tree_STATIC_INIT
#ifdef CONFIG_HAVE___dex_start____AND___end
#define HAVE_module_byaddr_tree_STATIC_INIT
INTERN RBTREE_ROOT(/*maybe(DREF)*/ Dee_module_object) module_byaddr_tree = &DeeModule_Deemon;
#else /* CONFIG_HAVE___dex_start____AND___end */
INTERN RBTREE_ROOT(/*maybe(DREF)*/ Dee_module_object) module_byaddr_tree = NULL;
#endif /* !CONFIG_HAVE___dex_start____AND___end */

#define RBTREE(name)           module_byaddr_##name
#define RBTREE_T               DeeModuleObject
#define RBTREE_Tkey            byte_t const *
#define RBTREE_GETMINKEY(self) (self)->mo_minaddr
#define RBTREE_GETMAXKEY(self) (self)->mo_maxaddr
#define RBTREE_NODEFIELD       mo_adrnode
#define RBTREE_ISRED(self)     (atomic_read(&(self)->mo_flags) & Dee_MODULE_FADRRED)
#define RBTREE_SETRED(self)    atomic_or(&(self)->mo_flags, Dee_MODULE_FADRRED)
#define RBTREE_SETBLACK(self)  atomic_and(&(self)->mo_flags, ~Dee_MODULE_FADRRED)
#define RBTREE_FLIPCOLOR(self) atomic_xor(&(self)->mo_flags, Dee_MODULE_FADRRED)
#define RBTREE_CC              DFCALL
#define RBTREE_NOTHROW         NOTHROW
#define RBTREE_DECL            PRIVATE
#define RBTREE_IMPL            PRIVATE
#define RBTREE_WANT_RLOCATE
#define RBTREE_OMIT_REMOVE
#define RBTREE_WANT_NEXTNODE
DECL_END
#include <hybrid/sequence/rbtree-abi.h>
DECL_BEGIN

#ifdef HAVE_Dee_dexataddr_t
/* Return the `Dee_dexataddr_t *' descriptor of "self". Caller must ensure
 * that `self' is a DEX module, and that `_Dee_MODULE_FNOADDR' is set. */
#define DeeModule_GetDexAtAddr(self) \
	(&Dee_module_dexdata__getinfo((self)->mo_moddata.mo_dexdata)->ddi_ataddr)

/* [0..n][lock(module_byaddr_lock)] */
INTERN RBTREE_ROOT(DREF Dee_module_object) dex_byaddr_tree = NULL;
#define RBTREE(name)           dex_byaddr_##name
#define RBTREE_T               DeeModuleObject
#define RBTREE_Tkey            Dee_dexataddr_t const *
#define RBTREE_GETKEY(self)    DeeModule_GetDexAtAddr(self)
#define RBTREE_KEY_LO(a, b)    (Dee_dexataddr_compare(a, b) < 0)
#define RBTREE_KEY_EQ(a, b)    (Dee_dexataddr_compare(a, b) == 0)
#define RBTREE_NODEFIELD       mo_adrnode
#define RBTREE_ISRED(self)     (atomic_read(&(self)->mo_flags) & Dee_MODULE_FADRRED)
#define RBTREE_SETRED(self)    atomic_or(&(self)->mo_flags, Dee_MODULE_FADRRED)
#define RBTREE_SETBLACK(self)  atomic_and(&(self)->mo_flags, ~Dee_MODULE_FADRRED)
#define RBTREE_FLIPCOLOR(self) atomic_xor(&(self)->mo_flags, Dee_MODULE_FADRRED)
#define RBTREE_CC              DFCALL
#define RBTREE_NOTHROW         NOTHROW
#define RBTREE_DECL            PRIVATE
#define RBTREE_IMPL            PRIVATE
#define RBTREE_OMIT_REMOVE
#define RBTREE_OMIT_REMOVENODE /* Not needed... */
#define RBTREE_WANT_NEXTNODE
DECL_END
#include <hybrid/sequence/rbtree-abi.h>
DECL_BEGIN
#endif /* HAVE_Dee_dexataddr_t */



struct dexrange {
	byte_t          *dr_minaddr;
	byte_t          *dr_maxaddr;
	DeeModuleObject *dr_mod;
};

#undef DEXRANGE_DEBUG
#if !defined(NDEBUG) && 1
#define DEXRANGE_DEBUG
#endif

PRIVATE NONNULL((1)) void DCALL
dexrange_union(struct dexrange *__restrict self,
               void const *addr, size_t num_bytes) {
	byte_t *extra_minaddr = (byte_t *)addr;
	byte_t *extra_maxaddr = extra_minaddr + num_bytes - 1;
#ifndef NDEBUG
	if (self->dr_mod != &DeeModule_Deemon) {
		DeeModuleObject *existing;
		existing = module_byaddr_rlocate(module_byaddr_tree, extra_minaddr, extra_maxaddr);
		ASSERTF(!existing, "Unexpected overlap at %p-%p with %p-%p from %q",
		        extra_minaddr, extra_maxaddr,
		        existing->mo_minaddr, existing->mo_maxaddr,
		        existing->mo_absname);
	}
#endif /* !NDEBUG */
#ifdef DEXRANGE_DEBUG
	if (self->dr_minaddr > extra_minaddr || self->dr_maxaddr < extra_maxaddr)
		Dee_DPRINTF("DEBUG: %q: Add range %p-%p\n", self->dr_mod->mo_absname, extra_minaddr, extra_maxaddr);
#endif /* DEXRANGE_DEBUG */
	if (self->dr_minaddr > extra_minaddr)
		self->dr_minaddr = extra_minaddr;
	if (self->dr_maxaddr < extra_maxaddr)
		self->dr_maxaddr = extra_maxaddr;
}

PRIVATE NONNULL((1)) bool DCALL
dexrange_tryunion(struct dexrange *__restrict self,
                  void const *addr, size_t num_bytes) {
	if (self->dr_mod != &DeeModule_Deemon) {
		byte_t *extra_minaddr = (byte_t *)addr;
		byte_t *extra_maxaddr = extra_minaddr + num_bytes - 1;
		if (module_byaddr_rlocate(module_byaddr_tree, extra_minaddr, extra_maxaddr))
			return false;
	}
	dexrange_union(self, addr, num_bytes);
	return true;
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_cstr(struct dexrange *__restrict self, char const *cstr) {
	dexrange_union(self, cstr, (strlen(cstr) + 1) * sizeof(char));
}

PRIVATE NONNULL((1)) void DCALL
dexrange_union_cstr_opt(struct dexrange *__restrict self, char const *cstr) {
	if (cstr != NULL)
		dexrange_union_cstr(self, cstr);
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_funcptr(struct dexrange *__restrict self, Dee_funptr_t ptr) {
	/* Many operators can **always** be implementing using exports from the
	 * demeon core. As such, always check that "ptr" isn't already part of
	 * another module. */
	if (self->dr_mod != &DeeModule_Deemon) {
		if (module_byaddr_locate(module_byaddr_tree, (byte_t *)(void const *)ptr))
			return; /* Skip if known to be part of a different module */
	}
#ifdef DEXRANGE_DEBUG
	if (self->dr_minaddr > (byte_t *)(void const *)ptr || self->dr_maxaddr < (byte_t *)(void const *)ptr)
		Dee_DPRINTF("DEBUG: %q: Add range %p\n", self->dr_mod->mo_absname, ptr);
#endif /* DEXRANGE_DEBUG */
	if (self->dr_minaddr > (byte_t *)(void const *)ptr)
		self->dr_minaddr = (byte_t *)(void const *)ptr;
	if (self->dr_maxaddr < (byte_t *)(void const *)ptr)
		self->dr_maxaddr = (byte_t *)(void const *)ptr;
}

PRIVATE NONNULL((1)) void DCALL
dexrange_union_funcptr_opt(struct dexrange *__restrict self, Dee_funptr_t ptr) {
	if (ptr != NULL)
		dexrange_union_funcptr(self, ptr);
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_object_r(DeeObject *__restrict ob, void *arg);

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_vtable(struct dexrange *self, void const *vt, size_t sizeof_vt) {
	/* The deemon core exports some V-tables (like "DeeObject_GenericCmpByAddr"),
	 * so don't enumerate vtable contents of the vtable itself is known to be
	 * part of another module. */
	if (dexrange_tryunion(self, vt, sizeof_vt)) {
		size_t i;
		Dee_funptr_t const *vtable = (Dee_funptr_t const *)vt;
		for (i = 0; i < sizeof_vt; i += sizeof(Dee_funptr_t), ++vtable)
			dexrange_union_funcptr_opt(self, *vtable);
	}
}

PRIVATE NONNULL((1)) void DCALL
dexrange_union_vtable_opt(struct dexrange *self, void const *vt, size_t sizeof_vt) {
	if (vt != NULL)
		dexrange_union_vtable(self, vt, sizeof_vt);
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_type_method(struct dexrange *self, struct type_method const *method) {
	dexrange_union_cstr(self, method->m_name);
	dexrange_union_cstr_opt(self, method->m_doc);
	dexrange_union_funcptr(self, (Dee_funptr_t)method->m_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_type_getset(struct dexrange *self, struct type_getset const *getset) {
	dexrange_union_cstr(self, getset->gs_name);
	dexrange_union_cstr_opt(self, getset->gs_doc);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)getset->gs_get);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)getset->gs_del);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)getset->gs_set);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)getset->gs_bound);
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_type_member(struct dexrange *self, struct type_member const *member) {
	dexrange_union_cstr(self, member->m_name);
	dexrange_union_cstr_opt(self, member->m_doc);
	if (Dee_TYPE_MEMBER_ISCONST(member))
		dexrange_union_object_r(member->m_desc.md_const, self);
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_type_method_hint(struct dexrange *self, struct type_method_hint const *method_hint) {
	dexrange_union_funcptr(self, method_hint->tmh_func);
}

PRIVATE NONNULL((1)) void DCALL
dexrange_union_type_methods_opt(struct dexrange *self, struct type_method const *methods) {
	if (methods) {
		size_t i;
		for (i = 0; methods[i].m_name; ++i)
			dexrange_union_type_method(self, &methods[i]);
		dexrange_union(self, methods, i * sizeof(*methods));
	}
}

PRIVATE NONNULL((1)) void DCALL
dexrange_union_type_getsets_opt(struct dexrange *self, struct type_getset const *getsets) {
	if (getsets) {
		size_t i;
		for (i = 0; getsets[i].gs_name; ++i)
			dexrange_union_type_getset(self, &getsets[i]);
		dexrange_union(self, getsets, i * sizeof(*getsets));
	}
}

PRIVATE NONNULL((1)) void DCALL
dexrange_union_type_members_opt(struct dexrange *self, struct type_member const *members) {
	if (members) {
		size_t i;
		for (i = 0; members[i].m_name; ++i)
			dexrange_union_type_member(self, &members[i]);
		dexrange_union(self, members, i * sizeof(*members));
	}
}

PRIVATE NONNULL((1)) void DCALL
dexrange_union_type_method_hints_opt(struct dexrange *self, struct type_method_hint const *method_hints) {
	if (method_hints) {
		size_t i;
		for (i = 0; method_hints[i].tmh_func; ++i)
			dexrange_union_type_method_hint(self, &method_hints[i]);
		dexrange_union(self, method_hints, i * sizeof(*method_hints));
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_typeobject(DeeTypeObject *tp, struct dexrange *self) {
	dexrange_union_cstr_opt(self, tp->tp_name);
	dexrange_union_cstr_opt(self, tp->tp_doc);
	if (tp->tp_base)
		dexrange_union_object_r(Dee_AsObject(tp->tp_base), self);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_ctor);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_copy_ctor);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_deep_ctor);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_any_ctor);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_any_ctor_kw);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_serialize);
	if (tp->tp_init.tp_alloc.tp_free) {
		dexrange_union_funcptr(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_free);
		dexrange_union_funcptr(self, (Dee_funptr_t)tp->tp_init.tp_alloc.tp_alloc);
	}
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_dtor);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_assign);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_move_assign);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_deepload);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_init.tp_destroy);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_cast.tp_str);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_cast.tp_repr);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_cast.tp_bool);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_cast.tp_print);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_cast.tp_printrepr);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_visit);
	dexrange_union_vtable_opt(self, tp->tp_gc, sizeof(*tp->tp_gc));
	dexrange_union_vtable_opt(self, tp->tp_math, sizeof(*tp->tp_math));
	dexrange_union_vtable_opt(self, tp->tp_cmp, sizeof(*tp->tp_cmp));
	dexrange_union_vtable_opt(self, tp->tp_seq, sizeof(*tp->tp_seq));
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_iter_next);
	dexrange_union_vtable_opt(self, tp->tp_iterator, sizeof(*tp->tp_iterator));
	dexrange_union_vtable_opt(self, tp->tp_attr, sizeof(*tp->tp_attr));
	dexrange_union_vtable_opt(self, tp->tp_with, sizeof(*tp->tp_with));
	if (tp->tp_buffer) {
		dexrange_union(self, tp->tp_buffer, sizeof(*tp->tp_buffer));
		dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_buffer->tp_getbuf);
	}
	dexrange_union_type_methods_opt(self, tp->tp_methods);
	dexrange_union_type_getsets_opt(self, tp->tp_getsets);
	dexrange_union_type_members_opt(self, tp->tp_members);
	dexrange_union_type_methods_opt(self, tp->tp_class_methods);
	dexrange_union_type_getsets_opt(self, tp->tp_class_getsets);
	dexrange_union_type_members_opt(self, tp->tp_class_members);
	dexrange_union_type_method_hints_opt(self, tp->tp_method_hints);
	dexrange_union_funcptr_opt(self, (Dee_funptr_t)tp->tp_call);
	dexrange_union_vtable_opt(self, tp->tp_callable, sizeof(*tp->tp_callable));
	if (tp->tp_operators_size)
		dexrange_union(self, tp->tp_operators, tp->tp_operators_size * sizeof(*tp->tp_operators));
}

PRIVATE ATTR_NOINLINE NONNULL((1, 2)) void DCALL
dexrange_union_object(DeeObject *__restrict ob, void *arg) {
	size_t instance_size;
	DeeTypeObject *tp;
	struct dexrange *me = (struct dexrange *)arg;
	if (me->dr_mod != &DeeModule_Deemon) {
		if (module_byaddr_locate(module_byaddr_tree, (byte_t *)ob))
			return; /* Skip if known to be part of a different module */
	}
	instance_size = DeeType_GetInstanceSize(Dee_TYPE(ob));
	if (instance_size == 0)
		instance_size = sizeof(DeeObject);
	dexrange_union(me, ob, instance_size);

	/* Special handling for certain types that are often exported from DEX modules. */
	tp = Dee_TYPE(ob);
	if (tp == &DeeCMethod_Type || tp == &DeeCMethod0_Type ||
	    tp == &DeeCMethod1_Type || tp == &DeeKwCMethod_Type) {
		DeeCMethodObject *obj = (DeeCMethodObject *)ob;
		dexrange_union_funcptr(me, (Dee_funptr_t)obj->cm_func.cmf_meth);
	} else if (DeeType_IsTypeType(tp)) {
		dexrange_union_typeobject((DeeTypeObject *)ob, me);
	} else if (DeeType_Extends(tp, &DeeModule_Type)) {
		/* Skip... */
	} else {
		/* Recursively visit other (sub-)objects reachable from "ob"
		 * However, do so using "dexrange_union_object_r" to prevent
		 * recursion if the object's range was already visited.
		 *
		 * This is actually not a very good way to do this. We might
		 * get better results if we used "dexrange_union_object" and
		 * had an address-set to keep track of already-visited objects:
		 * For 3 objects laid out as "A B C D", with only "A" and "C"
		 * exported from the DEX, and pointers C->B->D, we won't be
		 * able to find "D" since we won't visit "B", because it is
		 * already fully contained with the address-range previously
		 * set-up by "A" and "C".
		 *
		 * But since this whole idea of looking at module exports to
		 * get ~some~ idea of the associated module's boundaries is
		 * already a wholly flawed approach (there can **always** be
		 * pointers we missed and thus didn't associated with that
		 * module), this is "good enough" for a fallback mechanism.
		 *
		 * Because in general: there should always be some OS-specific
		 * was to determine which module some pointer is associated
		 * with, which is all we need to not have to get here. */
		DeeObject_Visit(ob, &dexrange_union_object_r, me);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
dexrange_union_object_r(DeeObject *__restrict ob, void *arg) {
	struct dexrange *me = (struct dexrange *)arg;
	if ((byte_t *)ob >= me->dr_minaddr &&
	    (byte_t *)ob <= me->dr_maxaddr)
		return; /* Already tracked... */
	dexrange_union_object(ob, me);
}

PRIVATE NONNULL((2)) void DCALL
dexrange_union_object_opt(DeeObject *ob, void *arg) {
	if (ob != NULL)
		dexrange_union_object(ob, arg);
}


/* Fallback function used to calculate dex bounds using the dex module's export table */
PRIVATE NONNULL((1)) void DCALL
DeeModule_InitDexBounds_with_exports(DeeModuleObject *__restrict self) {
	uint16_t i;
	struct dexrange range;
	struct Dee_module_dexdata *dd = self->mo_moddata.mo_dexdata;
	struct Dee_dex_symbol const *exports;
	range.dr_minaddr = (byte_t *)self;
	range.dr_maxaddr = (byte_t *)self + offsetof(DeeModuleObject, mo_globalv) +
	                   ((size_t)self->mo_globalc * sizeof(DeeObject *)) - 1;
#ifdef DEXRANGE_DEBUG
	Dee_DPRINTF("DEBUG: %q: Add range %p-%p\n", self->mo_absname, range.dr_minaddr, range.dr_maxaddr);
#endif /* DEXRANGE_DEBUG */
	range.dr_mod = self;
	dexrange_union(&range, dd, sizeof(*dd));
	exports = dd->mdx_export;
	if (exports) {
		dexrange_union(&range, exports, self->mo_globalc * sizeof(struct Dee_dex_symbol));
		for (i = 0; i < self->mo_globalc; ++i) {
			struct Dee_dex_symbol const *sym = &exports[i];
			dexrange_union_cstr_opt(&range, sym->ds_name);
			dexrange_union_cstr_opt(&range, sym->ds_doc);
			dexrange_union_object_opt(sym->ds_obj, &range);
		}
	}
	if (self->mo_bucketv) {
		struct Dee_module_symbol *buckets = self->mo_bucketv;
		dexrange_union(&range, buckets, (self->mo_bucketm + 1) * sizeof(struct Dee_module_symbol));
		for (i = 0; i <= self->mo_bucketm; ++i) {
			struct Dee_module_symbol *sym = &buckets[i];
			dexrange_union_cstr_opt(&range, sym->ss_name);
			dexrange_union_cstr_opt(&range, sym->ss_doc);
		}
	}
	for (i = 0; i < self->mo_globalc; ++i)
		dexrange_union_object_opt(self->mo_globalv[i], &range);
	self->mo_minaddr = range.dr_minaddr;
	self->mo_maxaddr = range.dr_maxaddr;
}



/* Dex data for the deemon core. */
#undef deemon_dexdata__mdx_module__IS_STATIC
#if defined(DeeSystem_DlOpen_USE_LoadLibrary) && defined(__PE__) && defined(_MSC_VER)
extern /*IMAGE_DOS_HEADER*/ __BYTE_TYPE__ const __ImageBase[];
#define deemon_dexdata__mdx_module__INIT (void *)__ImageBase
#define deemon_dexdata__mdx_module__IS_STATIC
#else /* ... */
#define deemon_dexdata__mdx_module__INIT NULL
#endif /* !... */

#if defined(HAVE_Dee_module_dexinfo) && !defined(HAVE_Dee_module_dexinfo_IS_POINTER)
PRIVATE struct Dee_module_dexinfo deemon_dexinfo = {};
#endif /* HAVE_Dee_module_dexinfo && !HAVE_Dee_module_dexinfo_IS_POINTER */

_Dee_MODULE_DEXDATA_INIT_BUILDID_PREHOOK;

INTERN struct Dee_module_dexdata deemon_dexdata = {
	/* .mdx_module  = */ &DeeModule_Deemon,
	/* .mdx_export  = */ NULL,
	/* .mdx_buildid = */ _Dee_MODULE_DEXDATA_INIT_BUILDID,
	/* .mdx_buildts = */ _Dee_MODULE_DEXDATA_INIT_BUILDTS,
	/* ._mdx_pad1   = */ { NULL, NULL, NULL, NULL },
	/* .mdx_init    = */ NULL,
	/* .mdx_fini    = */ NULL,
	/* .mdx_clear   = */ NULL,
	/* .mdx_handle  = */ deemon_dexdata__mdx_module__INIT,
#if defined(HAVE_Dee_module_dexinfo) && !defined(HAVE_Dee_module_dexinfo_IS_POINTER)
	/* .mdx_info    = */ &deemon_dexinfo,
#else /* HAVE_Dee_module_dexinfo && !HAVE_Dee_module_dexinfo_IS_POINTER */
	/* .mdx_info    = */ NULL,
#endif /* !HAVE_Dee_module_dexinfo || HAVE_Dee_module_dexinfo_IS_POINTER */
	/* ._mdx_pad2   = */ { NULL, NULL, NULL }
};


/* Called during finalization of the associated dex module */
INTERN NONNULL((1)) void DCALL
Dee_module_dexdata_fini(struct Dee_module_dexdata *__restrict self) {
	(void)self;
	COMPILER_IMPURE();
#ifdef Dee_module_dexinfo_alloc
	Dee_module_dexinfo_free(Dee_module_dexdata__getinfo(self));
#endif /* Dee_module_dexinfo_alloc */
}



/* Ensure that "module_byaddr_tree" has been initialized. */
#ifdef HAVE_module_byaddr_tree_STATIC_INIT
#define module_byaddr_is_initialized()          1
#define module_byaddr_ensure_initialized()      (void)0
#define module_byaddr_ensure_initialized_impl() (void)0
#else /* HAVE_module_byaddr_tree_STATIC_INIT */
#ifdef HAVE_Dee_dexataddr_t
#define module_byaddr_is_initialized() (module_byaddr_tree != NULL || dex_byaddr_tree != NULL)
#else /* HAVE_Dee_dexataddr_t */
#define module_byaddr_is_initialized() (module_byaddr_tree != NULL)
#endif /* !HAVE_Dee_dexataddr_t */
#define module_byaddr_ensure_initialized() \
	(void)(likely(module_byaddr_is_initialized()) || (module_byaddr_ensure_initialized_impl(), 1))
PRIVATE ATTR_NOINLINE void DCALL
module_byaddr_ensure_initialized_impl(void) {
	/* Load the deemon core dex module into "module_byaddr_tree" */
	ASSERT(module_byaddr_lock_writing());
	ASSERT(!module_byaddr_is_initialized());

	/* Initialize "deemon_dexdata.mdx_handle" if it wasn't already initialized statically. */
#ifndef deemon_dexdata__mdx_module__IS_STATIC
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
	deemon_dexdata.mdx_handle = GetModuleHandleW(NULL)
#elif defined(DeeSystem_DlOpen_USE_dlopen)
	deemon_dexdata.mdx_handle = dlopen(NULL, DLOPEN_NULL_FLAGS);
#endif /* DeeSystem_DlOpen_USE_dlopen */
#endif /* deemon_dexdata__mdx_module__IS_STATIC */

	/* Load different sub-components */
	if (!DeeModule_InitDexBounds(&DeeModule_Deemon)) {
#ifdef HAVE_Dee_dexataddr_t
		struct Dee_module_dexinfo *p_deemon_dexinfo = Dee_module_dexdata__getinfo(&deemon_dexdata);
		if (Dee_dexataddr_init_fromaddr(&p_deemon_dexinfo->ddi_ataddr, &DeeModule_Deemon)) {
			DeeModule_Deemon.mo_flags |= _Dee_MODULE_FNOADDR;
			if (!DeeModule_InitDexBounds_with_dexataddr(&DeeModule_Deemon,
			                                            &p_deemon_dexinfo->ddi_ataddr))
				goto load_from_export_table;
		} else
#endif /* HAVE_Dee_dexataddr_t */
		{
#ifdef HAVE_Dee_dexataddr_t
load_from_export_table:
#endif /* HAVE_Dee_dexataddr_t */
			DeeModule_InitDexBounds_with_exports(&DeeModule_Deemon);
			Dee_DPRINTF("[RT][dex] Warning: Unable to determine load range or byaddr info "
			            /**/ "for deemon core. Using address range %p-%p obtained "
			            /**/ "from scan of module exports\n",
			            DeeModule_Deemon.mo_minaddr, DeeModule_Deemon.mo_maxaddr);
		}
	}

	/* Set by-addr tree to consist of exactly the deemon core. */
#ifdef HAVE_Dee_dexataddr_t
	if (DeeModule_Deemon.mo_flags & _Dee_MODULE_FNOADDR) {
		dex_byaddr_tree = &DeeModule_Deemon;
		Dee_DPRINTF("[RT][dex] Add deemon core. Exposed address range is %p-%p\n",
		            DeeModule_Deemon.mo_minaddr, DeeModule_Deemon.mo_maxaddr);
	} else
#endif /* HAVE_Dee_dexataddr_t */
	{
		module_byaddr_tree = &DeeModule_Deemon;
		Dee_DPRINTF("[RT][dex] Add deemon core at %p-%p\n",
		            DeeModule_Deemon.mo_minaddr, DeeModule_Deemon.mo_maxaddr);
	}
}
#endif /* !HAVE_module_byaddr_tree_STATIC_INIT */




/* As defined by Dee_DEX_END()... */
struct DEX {
	struct Dee_gc_head_link       m_head;
	Dee_MODULE_STRUCT(/**/, 1024) m_dex;
};

PRIVATE NONNULL((1, 3)) void DCALL
dex_add_symbol(struct Dee_module_symbol *bucketv, uint16_t bucketm,
               struct Dee_dex_symbol const *symbol, uint16_t index) {
	Dee_hash_t i, perturb, hash;
	hash    = Dee_HashStr(symbol->ds_name);
	perturb = i = hash & bucketm;
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *target = &bucketv[i & bucketm];
		if (target->ss_name)
			continue;
		target->ss_name  = symbol->ds_name;
		target->ss_doc   = symbol->ds_doc;
		target->ss_index = index;
		target->ss_hash  = hash;
		target->ss_flags = symbol->ds_flags;
		ASSERT(!(symbol->ds_flags & (MODSYM_FNAMEOBJ | MODSYM_FDOCOBJ)));
		break;
	}
}

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
INTERN WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenDex(/*inherit(always)*/ /*utf-8*/ char *__restrict absname,
                  /*inherit(always)*/ void *dex_handle) {
	struct DEX *dex;
	struct Dee_module_dexdata *dexdata;
	DeeModuleObject *existing_module;
	DREF DeeModuleObject *result;
#ifdef Dee_module_dexinfo_alloc
	struct Dee_module_dexinfo *dexinfo = Dee_module_dexinfo_alloc();
	if unlikely(!dexinfo)
		goto err_no_dexinfo;
#elif defined(HAVE_Dee_dexataddr_t)
	struct Dee_module_dexinfo *dexinfo;
#endif /* ... */

again_acquire_locks:
	module_abstree_lock_write();
	if (!module_byaddr_lock_trywrite()) {
		module_abstree_lock_endwrite();
		module_byaddr_lock_write();
		if (!module_abstree_lock_trywrite()) {
			module_byaddr_lock_endwrite();
			goto again_acquire_locks;
		}
	}
#define LOCAL_unlock()              \
	(module_byaddr_lock_endwrite(), \
	 module_abstree_lock_endwrite())

	/* Check if we've already loaded a module at the specified "absname" */
	result = module_abstree_locate(module_abstree_root, absname);
	if unlikely(result) {
		if (Dee_IncrefIfNotZero(result)) {
			LOCAL_unlock();
#ifdef Dee_module_dexinfo_alloc
			Dee_module_dexinfo_free(dexinfo);
#endif /* Dee_module_dexinfo_alloc */
			Dee_Free(absname);
			DeeSystem_DlClose(dex_handle);
			return result;
		}
		ASSERT(result->mo_absnode.rb_par != DeeModule_IMPORT_ENOENT);
		module_abstree_removenode(&module_abstree_root, result);
		result->mo_absnode.rb_par = DeeModule_IMPORT_ENOENT;
	}

	/* Lookup "DEX" descriptor export. */
	dex = (struct DEX *)DeeSystem_DlSym(dex_handle, "DEX");
	if unlikely(!dex)
		dex = (struct DEX *)DeeSystem_DlSym(dex_handle, "_DEX");
	if unlikely(!dex)
		goto err_unlock_not_dex;

	/* Check if we've already loaded this dex module... */
	module_byaddr_ensure_initialized();
	existing_module = module_byaddr_locate(module_byaddr_tree, (byte_t *)dex);
	if (existing_module) {
		if (Dee_IncrefIfNotZero(existing_module)) {
			Dee_DPRINTF("[RT][dex] Warning: Found existing module %q at %p-%p overlapping with "
			            /**/ "\"DEX\" symbol of %q at %p. This is very weird a probably a bug\n",
			            existing_module->mo_absname,
			            existing_module->mo_minaddr,
			            existing_module->mo_maxaddr,
			            absname, dex);
handle_existing_module:
			LOCAL_unlock();
#ifdef Dee_module_dexinfo_alloc
			Dee_module_dexinfo_free(dexinfo);
#endif /* Dee_module_dexinfo_alloc */

			/* If "existing_module" is a DEE module, then our caller will eventually
			 * fail an assertion check in `DeeModule_OpenFile_impl2()', because the
			 * module will probably already have a name...
			 * >> remember_dir_module:
			 * >> 		ASSERT(!result->mo_absname);
			 * >> 		if (flags & _DeeModule_IMPORT_F_NO_INHERIT_FILENAME) { */
			ASSERTF(Dee_TYPE(existing_module) == &DeeModuleDex_Type,
			        "Existing module %q (while loading %q handle %p) isn't a dex module (is %q). "
			        "If we didn't already assert that here, another assertion would just fail later.",
			        existing_module->mo_absname, absname, dex_handle,
			        DeeType_GetName(Dee_TYPE(existing_module)));
			Dee_Free(absname);
			DeeSystem_DlClose(dex_handle);
			return existing_module;
		}
		module_byaddr_removenode(&module_byaddr_tree, existing_module);
		existing_module->mo_adrnode.rb_par = DeeModule_IMPORT_ENOENT;
	}

	/* Sanity check: does this look like a valid dex object? */
	result = (DREF DeeModuleObject *)&dex->m_dex;
	if unlikely(result->ob_type != &DeeModuleDex_Type)
		goto err_unlock_corrupt_dex;
	dexdata = result->mo_moddata.mo_dexdata;
	if unlikely(!dexdata)
		goto err_unlock_corrupt_dex;
	if unlikely(dexdata->mdx_module != result)
		goto err_unlock_corrupt_dex;
	if unlikely(!IS_POWER_OF_TWO(result->mo_bucketm + 1))
		goto err_unlock_corrupt_dex;
	if unlikely(!result->mo_bucketv)
		goto err_unlock_corrupt_dex;

	/* Check if the dex module is already loaded. */
	if (result->mo_absname && Dee_IncrefIfNotZero(result)) {
		LOCAL_unlock();
#ifdef Dee_module_dexinfo_alloc
		Dee_module_dexinfo_free(dexinfo);
#endif /* Dee_module_dexinfo_alloc */
		Dee_Free(absname);
		DeeSystem_DlClose(dex_handle);
		return result;
	}

	/* Remember dynamically allocated "dexinfo" data */
#ifdef Dee_module_dexinfo_alloc
	dexdata->mdx_info = dexinfo;
#elif defined(HAVE_Dee_dexataddr_t)
	dexinfo = Dee_module_dexdata__getinfo(dexdata);
#endif /* ... */

	/* Initialize common data of the module. */
	result->ob_refcnt   = 2;          /* +1: return; +1: module_byaddr_tree */
	result->mo_absname  = absname;    /* Inherit reference */
	result->mo_init     = Dee_MODULE_INIT_UNINITIALIZED;
	result->mo_flags    = Dee_MODULE_FNORMAL;
	result->mo_dir      = NULL;
	dexdata->mdx_handle = dex_handle; /* Inherit reference */

	/* Initialize OS-specific data of the module... */
	if (result->mo_minaddr && result->mo_maxaddr &&
	    result->mo_maxaddr != (byte_t *)-1 &&
	    result->mo_minaddr < result->mo_maxaddr &&
	    (result->mo_minaddr <= (byte_t *)(result) &&
	     result->mo_maxaddr >= (byte_t *)(result + 1) - 1)) {
		/* Address range already initialized */
	} else if (DeeModule_InitDexBounds(result)) {
		/* Exact address range could be determined from OS */
#ifdef HAVE_Dee_dexataddr_t
	} else if (Dee_dexataddr_init_fromaddr(&dexinfo->ddi_ataddr, dex)) {
		/* Use the alternate "dex_byaddr_tree" */
		result->mo_flags |= _Dee_MODULE_FNOADDR;
		if (!DeeModule_InitDexBounds_with_dexataddr(result, &dexinfo->ddi_ataddr))
			DeeModule_InitDexBounds_with_exports(result);
#endif /* HAVE_Dee_dexataddr_t */
	} else {
		DeeModule_InitDexBounds_with_exports(result);
		Dee_DPRINTF("[RT][dex] Warning: Unable to determine load range or byaddr info "
		            /**/ "for DEX module %q. Using address range %p-%p obtained "
		            /**/ "from scan of module exports\n",
		            absname, result->mo_minaddr, result->mo_maxaddr);
	}

	/* Initialize the module's "mo_globalv" and "mo_bucketv" tables (if not already done). */
	if (dexdata->mdx_export) {
		uint16_t i, c = result->mo_globalc;
		uint16_t m = result->mo_bucketm;
		struct Dee_module_symbol *v = result->mo_bucketv;
		struct Dee_dex_symbol const *e = dexdata->mdx_export;
		for (i = 0; i < c;) {
			struct Dee_dex_symbol const *sym = &e[i];
			result->mo_globalv[i] = sym->ds_obj;
			dex_add_symbol(v, m, sym, i);
			++i;
			if ((sym->ds_flags & (Dee_DEXSYM_PROPERTY | Dee_DEXSYM_READONLY)) ==
			    /*            */ (Dee_DEXSYM_PROPERTY)) {
				result->mo_globalv[i] = sym[1].ds_obj;
				++i;
				result->mo_globalv[i] = sym[2].ds_obj;
				++i;
			}
		}
		dexdata->mdx_export = NULL;
	}

	/* Safety-check that there really isn't anything already
	 * within the detected address range.
	 *
	 * Only if nothing is there, can we insert the new module! */
#ifdef HAVE_Dee_dexataddr_t
	if (result->mo_flags & _Dee_MODULE_FNOADDR) {
		existing_module = dex_byaddr_locate(dex_byaddr_tree, &dexinfo->ddi_ataddr);
		if (existing_module) {
			struct Dee_module_dexdata *existing_dexdata;
			ASSERT_OBJECT_TYPE_EXACT(existing_module, &DeeModuleDex_Type);
			Dee_Incref(existing_module);
			existing_dexdata = existing_module->mo_moddata.mo_dexdata;
			if (existing_dexdata->mdx_handle != dex_handle) {
				Dee_DPRINTF("[RT][dex] Warning: Found existing module %q (handle %p) sharing its at-addr "
				            /**/ "key with with %q (handle %p). This is very weird a probably a bug\n",
				            existing_module->mo_absname, existing_dexdata->mdx_handle,
				            absname, dex_handle);
			}
			goto handle_existing_module;
		}
		Dee_DPRINTF("[RT][dex] Add dex module: %q. Exposed address range is %p-%p\n",
		            absname, result->mo_minaddr, result->mo_maxaddr);
		dex_byaddr_insert(&dex_byaddr_tree, result);
	} else
#endif /* HAVE_Dee_dexataddr_t */
	{
		existing_module = module_byaddr_rlocate(module_byaddr_tree,
		                                        result->mo_minaddr,
		                                        result->mo_maxaddr);
		if unlikely(existing_module) {
			if (Dee_IncrefIfNotZero(existing_module)) {
				Dee_DPRINTF("[RT][dex] Warning: Found existing module %q at %p-%p overlapping "
				            /**/ "with %q at %p-%p. This is very weird a probably a bug\n",
				            existing_module->mo_absname,
				            existing_module->mo_minaddr,
				            existing_module->mo_maxaddr,
				            absname, result->mo_minaddr, result->mo_maxaddr);
				goto handle_existing_module;
			}
			module_byaddr_removenode(&module_byaddr_tree, existing_module);
			existing_module->mo_adrnode.rb_par = DeeModule_IMPORT_ENOENT;
		}
		Dee_DPRINTF("[RT][dex] Add dex module at %p-%p: %q\n",
		            result->mo_minaddr, result->mo_maxaddr, absname);
		module_byaddr_insert(&module_byaddr_tree, result);

#ifdef Dee_module_dexinfo_alloc
		/* If "dexinfo" was allocated but not used, then free it again */
		ASSERT(dexdata->mdx_info == dexinfo);
		dexdata->mdx_info = NULL;
		Dee_module_dexinfo_free(dexinfo);
#endif /* ... */
	}

	/* Register the module globally... */
	Dee_XIncrefv(result->mo_globalv, result->mo_globalc);
	module_abstree_insert(&module_abstree_root, result);
	result = DeeGC_TRACK(DeeModuleObject, result);
	LOCAL_unlock();
	return result;
#ifdef WANT_err_unlock_dlerror
#undef WANT_err_unlock_dlerror
err_unlock_dlerror:
	LOCAL_unlock();
	DeeError_Throwf(&DeeError_RuntimeError,
	                "Failed to load mapping infos for %q: %s",
	                absname, DeeSystem_DlError());
	goto err;
#endif /* WANT_err_unlock_dlerror */
err_unlock_corrupt_dex:
	LOCAL_unlock();
	DeeError_Throwf(&DeeError_RuntimeError,
	                "Shared library file %q exports a corrupt descriptor table",
	                absname);
	goto err;
err_unlock_not_dex:
	LOCAL_unlock();
	DeeError_Throwf(&DeeError_RuntimeError,
	                "Shared library file %q does not export a descriptor table",
	                absname);
err:
#ifdef Dee_module_dexinfo_alloc
	Dee_module_dexinfo_free(dexinfo);
err_no_dexinfo:
#endif /* Dee_module_dexinfo_alloc */
	Dee_Free(absname);
	DeeSystem_DlClose(dex_handle);
	return NULL;
#undef LOCAL_unlock
}


PRIVATE NONNULL((1)) void DCALL
DeeModule_ClearDexModuleCaches_prepare_at(DeeModuleObject *__restrict root) {
again:
	atomic_and(&root->mo_flags, ~_Dee_MODULE_FCLEARED);
	if (root->mo_adrnode.rb_lhs) {
		if (root->mo_adrnode.rb_rhs)
			DeeModule_ClearDexModuleCaches_prepare_at(root->mo_adrnode.rb_rhs);
		root = root->mo_adrnode.rb_lhs;
		goto again;
	}
	if (root->mo_adrnode.rb_rhs) {
		root = root->mo_adrnode.rb_rhs;
		goto again;
	}
}

PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
DeeModule_ClearDexModuleCaches_find_uncleared(DeeModuleObject *__restrict root) {
again:
	if (Dee_TYPE(root) == &DeeModuleDex_Type) {
		if (!(atomic_read(&root->mo_flags) & _Dee_MODULE_FCLEARED) &&
		    root->mo_moddata.mo_dexdata->mdx_clear != NULL) {
			atomic_or(&root->mo_flags, _Dee_MODULE_FCLEARED);
			return root;
		}
	}
	if (root->mo_adrnode.rb_lhs) {
		if (root->mo_adrnode.rb_rhs) {
			DeeModuleObject *result;
			result = DeeModule_ClearDexModuleCaches_find_uncleared(root->mo_adrnode.rb_rhs);
			if (result)
				return result;
		}
		root = root->mo_adrnode.rb_lhs;
		goto again;
	}
	if (root->mo_adrnode.rb_rhs) {
		root = root->mo_adrnode.rb_rhs;
		goto again;
	}
	return NULL;
}

PRIVATE bool DCALL DeeModule_ClearDexModuleCaches_locked(void) {
	bool result = false;
	DeeModuleObject *root;
	DeeModuleObject *todo;
	module_byaddr_lock_read();
#ifdef HAVE_Dee_dexataddr_t
	if ((root = dex_byaddr_tree) != NULL)
		DeeModule_ClearDexModuleCaches_prepare_at(root);
#endif /* !HAVE_Dee_dexataddr_t */
	if ((root = module_byaddr_tree) != NULL)
		DeeModule_ClearDexModuleCaches_prepare_at(root);
again:
#ifdef HAVE_Dee_dexataddr_t
	if ((root = dex_byaddr_tree) != NULL &&
	    (todo = DeeModule_ClearDexModuleCaches_find_uncleared(root)) != NULL)
		goto handle_todo_module;
#endif /* !HAVE_Dee_dexataddr_t */
	if ((root = module_byaddr_tree) != NULL &&
	    (todo = DeeModule_ClearDexModuleCaches_find_uncleared(root)) != NULL) {
#ifdef HAVE_Dee_dexataddr_t
handle_todo_module:
#endif /* HAVE_Dee_dexataddr_t */
		Dee_Incref(todo);
		module_byaddr_lock_endread();
		ASSERT(Dee_TYPE(todo) == &DeeModuleDex_Type);
		ASSERT(todo->mo_moddata.mo_dexdata->mdx_clear);
		result |= (*todo->mo_moddata.mo_dexdata->mdx_clear)();
		Dee_Decref_unlikely(todo);
		module_byaddr_lock_read();
		goto again;
	}
	module_byaddr_lock_endread();
	return result;
}

PRIVATE Dee_nrshared_lock_t dex_clear_cache_lock = DEE_NRSHARED_LOCK_INIT;

/* Invoke the "mdx_clear" operator on every loaded DEX module. */
INTERN bool DCALL DeeModule_ClearDexModuleCaches(void) {
	bool result;
	int status = Dee_nrshared_lock_acquire_noint(&dex_clear_cache_lock);
	if (status != Dee_NRLOCK_OK)
		return false; /* Don't allow re-entrancy! */
	result = DeeModule_ClearDexModuleCaches_locked();
	Dee_nrshared_lock_release(&dex_clear_cache_lock);
	return result;
}

#define mo_nextdex mo_adrnode.rb_par

PRIVATE NONNULL((1, 2)) void DCALL
serialize_tree_without_deemon(DeeModuleObject *root,
                              DREF DeeModuleObject **p_dexlist,
                              DeeModuleObject **p_deelist) {
	DeeModuleObject *lhs, *rhs;
again:
	lhs = root->mo_adrnode.rb_lhs;
	rhs = root->mo_adrnode.rb_rhs;
	if (root != &DeeModule_Deemon) {
		if (Dee_TYPE(root) == &DeeModuleDee_Type) {
			root->mo_nextdex = *p_deelist;
			*p_deelist = root;
		} else {
			ASSERT(Dee_TYPE(root) == &DeeModuleDex_Type);
			root->mo_nextdex = *p_dexlist;
			*p_dexlist = root;
		}
	}
	if (lhs != NULL) {
		if (rhs != NULL)
			serialize_tree_without_deemon(rhs, p_dexlist, p_deelist);
		root = lhs;
		goto again;
	}
	if (rhs != NULL) {
		root = rhs;
		goto again;
	}
}

/* Unload all loaded DEX modules. */
INTERN void DCALL DeeModule_UnloadAllDexModules(void) {
	DREF DeeModuleObject *dex_list = NULL;
	DeeModuleObject *dee_list = NULL;
	module_byaddr_lock_write();
	if (module_byaddr_tree)
		serialize_tree_without_deemon(module_byaddr_tree, &dex_list, &dee_list);
#ifdef HAVE_Dee_dexataddr_t
	if (dex_byaddr_tree)
		serialize_tree_without_deemon(dex_byaddr_tree, &dex_list, &dee_list);
	if (DeeModule_Deemon.mo_flags & _Dee_MODULE_FNOADDR) {
		dex_byaddr_tree = &DeeModule_Deemon;
	} else
#endif /* HAVE_Dee_dexataddr_t */
	{
		module_byaddr_tree = &DeeModule_Deemon;
	}
	atomic_and(&DeeModule_Deemon.mo_flags, ~Dee_MODULE_FADRRED);
	DeeModule_Deemon.mo_adrnode.rb_lhs = NULL;
	DeeModule_Deemon.mo_adrnode.rb_rhs = NULL;
	DeeModule_Deemon.mo_adrnode.rb_par = NULL;
	while (dee_list) {
		DeeModuleObject *next = dee_list->mo_nextdex;
		module_byaddr_insert(&module_byaddr_tree, dee_list);
		dee_list = next;
	}
	module_byaddr_lock_endwrite();

	/* Drop references that were stored in the tree. */
	while (dex_list) {
		DeeModuleObject *next = dex_list->mo_nextdex;
		Dee_Decref_likely(dex_list);
		dex_list = next;
	}
}


#if !defined(CONFIG_NO_DEC) && defined(CONFIG_EXPERIMENTAL_MMAP_DEC)
/* Unbind from `module_byaddr_tree' */
INTERN NONNULL((1)) void DCALL
module_dee_unbind(DeeModuleObject *__restrict self) {
	module_byaddr_lock_write();
	if (self->mo_adrnode.rb_par != DeeModule_IMPORT_ENOENT)
		module_byaddr_removenode(&module_byaddr_tree, self);
	module_byaddr_lock_endwrite();
}
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */



#ifndef CONFIG_NO_DEX
/* Special handling needed to deal with the "@NN"
 * suffix caused by STDCALL functions on i386-pe */
#undef NEED_DeeModule_GetNativeSymbol_AT_SUFFIX
#if defined(__i386__) && !defined(__x86_64__) && defined(__PE__)
#define NEED_DeeModule_GetNativeSymbol_AT_SUFFIX
#endif /* __i386__ && !__x86_64__ && __PE__ */

PRIVATE WUNUSED NONNULL((1, 2)) void *DCALL
DeeSystem_DlSym_with_decoration(void *handle, char const *__restrict name) {
	void *result = DeeSystem_DlSym(handle, name);
	if (!result) {
#ifdef NEED_DeeModule_GetNativeSymbol_AT_SUFFIX
		/* Try again after inserting an underscore. */
		char *temp_name;
		size_t namelen = strlen(name);
#ifdef Dee_MallocaNoFailc
		Dee_MallocaNoFailc(temp_name, namelen + 6, sizeof(char));
#else /* Dee_MallocaNoFailc */
		temp_name = (char *)Dee_Mallocac(namelen + 6, sizeof(char));
		if unlikely(!temp_name) {
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			return NULL; /* ... Technically not correct, but if memory has gotten
			              *     this low, that's the last or the user's problems. */
		}
#endif /* !Dee_MallocaNoFailc */
		memcpyc(temp_name + 1, name, namelen + 1, sizeof(char));
		temp_name[0] = '_';
		result = DeeSystem_DlSym(handle, temp_name);
		if (!result) {
			/* Try to append (in order): "@0", "@4", "@8", "@12", "@16", "@20", "@24", "@28", "@32" */
			size_t n;
			for (n = 0; n <= 32; n += 4) {
				Dee_sprintf(temp_name + 1 + namelen, "@%" PRFuSIZ, n);

				/* Try without leading '_' */
				result = DeeSystem_DlSym(handle, temp_name + 1);
				if (result)
					break;

				/* Try with leading '_' */
				result = DeeSystem_DlSym(handle, temp_name);
				if (result)
					break;
			}
		}
		Dee_Freea(temp_name);

#else /* NEED_DeeModule_GetNativeSymbol_AT_SUFFIX */
		/* Try again after inserting an underscore. */
		if (SAMEPAGE(name, name - 1) && name[-1] == '_') {
			result = DeeSystem_DlSym(handle, name - 1);
		} else {
			char *temp_name;
			size_t namelen = strlen(name);
#ifdef Dee_MallocaNoFailc
			Dee_MallocaNoFailc(temp_name, namelen + 2, sizeof(char));
#else /* Dee_MallocaNoFailc */
			temp_name = (char *)Dee_Mallocac(namelen + 2, sizeof(char));
			if unlikely(!temp_name) {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				return NULL; /* ... Technically not correct, but if memory has gotten
				              *     this low, that's the last or the user's problems. */
			}
#endif /* !Dee_MallocaNoFailc */
			memcpyc(temp_name + 1, name,
			        namelen + 1,
			        sizeof(char));
			temp_name[0] = '_';
			result = DeeSystem_DlSym(handle, temp_name);
			Dee_Freea(temp_name);
		}
#endif /* !NEED_DeeModule_GetNativeSymbol_AT_SUFFIX */
	}
	return result;
}
#endif /* !CONFIG_NO_DEX */


/* Return the export address of a native symbol exported from a dex `self'.
 * When `self' isn't a dex, but a regular module, or if the symbol wasn't found, return `NULL'.
 * NOTE: Because native symbols cannot appear in user-defined modules,
 *       in the interest of keeping native functionality to its bare
 *       minimum, any code making using of this function should contain
 *       a fallback that calls a global symbol of the module, rather
 *       than a native symbol:
 * >> static int (*p_add)(int x, int y) = NULL;
 * >> if (!p_add) *(void **)&p_add = DeeModule_GetNativeSymbol(IMPORTED_MODULE, "add");
 * >> // Fallback: Invoke a member attribute `add' if the native symbol doesn't exist.
 * >> if (!p_add) return DeeObject_CallAttrStringf(IMPORTED_MODULE, "add", "dd", x, y);
 * >> // Invoke the native symbol.
 * >> return DeeInt_NewInt((*p_add)(x, y)); */
PUBLIC WUNUSED NONNULL((1, 2)) void *DCALL
DeeModule_GetNativeSymbol(DeeModuleObject *__restrict self,
                          char const *__restrict name) {
#ifndef CONFIG_NO_DEX
	if (Dee_TYPE(self) == &DeeModuleDex_Type) {
		struct Dee_module_dexdata *dexdata;
		dexdata = self->mo_moddata.mo_dexdata;
#ifndef deemon_dexdata__mdx_module__IS_STATIC
		if unlikely(dexdata->mdx_handle == DeeSystem_DlOpen_FAILED) {
			/* In case "self" is the deemon core module,
			 * lazily initialize the system handle. */
			module_byaddr_lock_write();
			module_byaddr_ensure_initialized();
			module_byaddr_lock_endwrite();
		}
#endif /* !deemon_dexdata__mdx_module__IS_STATIC */
		return DeeSystem_DlSym_with_decoration(dexdata->mdx_handle, name);
	}
#endif /* !CONFIG_NO_DEX */
	(void)self;
	(void)name;
	COMPILER_IMPURE();
	return NULL;
}




/* Unbind "self" from relevant trees.
 * Caller must ensure that `self->mo_absname != NULL' */
INTERN NONNULL((1)) void DCALL
module_unbind(DeeModuleObject *__restrict self) {
	ASSERT(self->mo_absname);
	module_abstree_lock_write();
	if (self->mo_absnode.rb_par != DeeModule_IMPORT_ENOENT)
		module_abstree_removenode(&module_abstree_root, self);
	module_abstree_lock_endwrite();
	if (self->mo_libname.mle_name) {
		struct Dee_module_libentry *iter = &self->mo_libname;
		module_libtree_lock_write();
		if unlikely(!self->mo_libname.mle_name) {
			module_libtree_lock_endwrite();
		} else {
			ASSERT(Dee_module_libentry_getmodule(iter) == self);
			module_libtree_removenode(&module_libtree_root, iter);
			while ((iter = iter->mle_next) != NULL) {
				ASSERT(Dee_module_libentry_getmodule(iter) == self);
				module_libtree_removenode(&module_libtree_root, iter);
			}
			module_libtree_lock_endwrite();
			iter = &self->mo_libname;
			Dee_Decref(iter->mle_name);
			iter = iter->mle_next;
			while (iter) {
				struct Dee_module_libentry *next;
				ASSERT(Dee_module_libentry_getmodule(iter) == self);
				next = iter->mle_next;
				Dee_Decref(iter->mle_name);
				Dee_module_libentry_free(iter);
				iter = next;
			}
		}
	}
}


/* Remove all LibPath nodes of "self" (caller is holding libpath lock) */
PRIVATE NONNULL((1)) void DCALL
DeeModule_RemoveAllLibPathNodes_locked(DeeModuleObject *__restrict self) {
	struct Dee_module_libentry *entry = &self->mo_libname;
	ASSERT(entry->mle_name);
	for (;;) {
		struct Dee_module_libentry *next = entry->mle_next;
		ASSERT(Dee_module_libentry_getmodule(entry) == self);
		module_libtree_removenode(&module_libtree_root, entry);
		/* FIXME: This may invoke user-code (string-fini-hooks),
		 *        but we're still holding "module_libtree_lock" */
		Dee_Decref_unlikely(entry->mle_name);
		if (entry != &self->mo_libname)
			Dee_module_libentry_free(entry);
		if (!next)
			break;
		entry = next;
	}

	/* Re-initialize libname descriptor of "self" as empty. */
	self->mo_libname.mle_dat.mle_mod = self;
	self->mo_libname.mle_name = NULL;
	self->mo_libname.mle_next = NULL;
}

/* Recursively find+remove entries for global libnames of modules starting with "removed_path" */
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
DeeModule_HandleRemovedLibPath_locked_at(struct Dee_module_libentry *__restrict node,
                                         char const *removed_path, size_t removed_path_size) {
	DeeModuleObject *mod;
again:
	mod = Dee_module_libentry_getmodule(node);
	ASSERT(mod->mo_absname);
	if (fs_bcmp(mod->mo_absname, removed_path, removed_path_size) == 0 &&
	    mod->mo_absname[removed_path_size] == DeeSystem_SEP) {
		/* Found a global module who's filename is a descendant of "removed_path" */
		DeeModule_RemoveAllLibPathNodes_locked(mod);
		return true;
	}

	/* Recursively enumerate all other nodes... */
	if (node->mle_node.rb_lhs) {
		if (node->mle_node.rb_rhs) {
			if (DeeModule_HandleRemovedLibPath_locked_at(node->mle_node.rb_rhs,
			                                             removed_path, removed_path_size))
				return true;
		}
		node = node->mle_node.rb_lhs;
		goto again;
	}
	if (node->mle_node.rb_rhs) {
		node = node->mle_node.rb_rhs;
		goto again;
	}
	return false;
}

PRIVATE NONNULL((1)) void DCALL
DeeModule_HandleRemovedLibPath_locked(char const *removed_path,
                                      size_t removed_path_size) {
	ASSERT(module_libtree_lock_writing());
	while (module_libtree_root) {
		if (!DeeModule_HandleRemovedLibPath_locked_at(module_libtree_root,
		                                              removed_path,
		                                              removed_path_size))
			break;
	}
}


/* Recursively find+remove entries for global libnames of modules starting with "removed_path" */
PRIVATE NONNULL((1)) void DCALL
DeeModule_ClearLibAllFlag_locked_at(struct Dee_module_libentry *__restrict node) {
	DeeModuleObject *mod;
again:
	mod = Dee_module_libentry_getmodule(node);
	ASSERT(mod->mo_absname);
	atomic_and(&mod->mo_flags, ~_Dee_MODULE_FLIBALL);
	/* Recursively enumerate all other nodes... */
	if (node->mle_node.rb_lhs) {
		if (node->mle_node.rb_rhs)
			DeeModule_ClearLibAllFlag_locked_at(node->mle_node.rb_rhs);
		node = node->mle_node.rb_lhs;
		goto again;
	}
	if (node->mle_node.rb_rhs) {
		node = node->mle_node.rb_rhs;
		goto again;
	}
}

PRIVATE void DCALL DeeModule_ClearLibAllFlag_locked(void) {
	ASSERT(module_libtree_lock_writing());
	if (module_libtree_root)
		DeeModule_ClearLibAllFlag_locked_at(module_libtree_root);
}


#ifndef CONFIG_NO_DEC
/* Load+relocation a .dec file into memory.
 * @param: dec_dirname:     Absolute filename of the corresponding ".dee" file (ZERO-terminated)
 * @param: dec_dirname_len: Offset (in bytes) from "dec_dirname" where the filename portion begins
 * @param: dee_file_last_modified: Timestamp when the ".dee" file was last modified
 * @return: * :        Success (module is anonymous "mo_absname == NULL" and not-yet-tracked)
 * @return: ITER_DONE: Failed to load dec file (probably corrupt)
 * @return: NULL:      Error was thrown. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF /*untracked*/ DeeModuleObject *DCALL
DeeModule_OpenDecFile_impl(/*inherit(always)*/ DREF DeeObject *dec_stream,
                           /*utf-8*/ char const *__restrict dec_dirname, size_t dec_dirname_len,
                           struct Dee_compiler_options *options, uint64_t dee_file_last_modified) {
	DREF DeeModuleObject *result;
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	struct DeeMapFile fmap;
	int fmap_status;

	/* Map file into memory */
	while (dec_dirname_len && dec_dirname[dec_dirname_len - 1] == DeeSystem_SEP)
		--dec_dirname_len;
	fmap_status = DeeMapFile_InitFile(&fmap, dec_stream, 0, 0, DFILE_LIMIT, 0, DeeMapFile_F_READALL);
	Dee_Decref_likely(dec_stream);
	if unlikely(fmap_status)
		return NULL;

	/* Load file mapping as a .dec file */
	result = DeeDec_OpenFile(&fmap, dec_dirname, dec_dirname_len,
	                         DeeModule_IMPORT_F_CTXDIR,
	                         options, dee_file_last_modified);

	/* Cleanup on error */
	if unlikely(!ITER_ISOK(result))
		DeeMapFile_Fini(&fmap);
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	while (dec_dirname_len && dec_dirname[dec_dirname_len - 1] == DeeSystem_SEP)
		--dec_dirname_len;
	result = DeeModule_OpenDec(dec_stream, options, dec_dirname, dec_dirname_len, dee_file_last_modified);
	Dee_Decref_likely(dec_stream);
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
	return result;
}
#endif /* !CONFIG_NO_DEC */

#define _DeeModule_IMPORT_F_NO_INHERIT_FILENAME 0x1000 /* Do not inherit `abs_filename' (via `Dee_Free()') */
#ifndef CONFIG_NO_DEC
#define _DeeModule_IMPORT_F_IS_DEE_FILE         0x2000 /* Given "abs_filename" ends with ".dee" */
#else /* !CONFIG_NO_DEC */
#define _DeeModule_IMPORT_F_IS_DEE_FILE         0
#endif /* CONFIG_NO_DEC */
#define _DeeModule_IMPORT_F_OUT_IS_RAW_FILE     0x4000 /* The final module was loaded from the raw filename */

/* Mask of internal flags */
#define _DeeModule_IMPORT_F_MASK               \
	(_DeeModule_IMPORT_F_NO_INHERIT_FILENAME | \
	 _DeeModule_IMPORT_F_IS_DEE_FILE |         \
	 _DeeModule_IMPORT_F_OUT_IS_RAW_FILE)
STATIC_ASSERT((DeeModule_IMPORT_F_ENOENT & _DeeModule_IMPORT_F_MASK) == 0);
STATIC_ASSERT((DeeModule_IMPORT_F_FILNAM & _DeeModule_IMPORT_F_MASK) == 0);
STATIC_ASSERT((DeeModule_IMPORT_F_CTXDIR & _DeeModule_IMPORT_F_MASK) == 0);
STATIC_ASSERT((DeeModule_IMPORT_F_ANONYM & _DeeModule_IMPORT_F_MASK) == 0);
STATIC_ASSERT((DeeModule_IMPORT_F_ERECUR & _DeeModule_IMPORT_F_MASK) == 0);
#ifndef CONFIG_NO_DEX
STATIC_ASSERT((DeeModule_IMPORT_F_NOLDEX & _DeeModule_IMPORT_F_MASK) == 0);
#endif /* !CONFIG_NO_DEX */
#ifndef CONFIG_NO_DEC
STATIC_ASSERT((DeeModule_IMPORT_F_NOLDEC & _DeeModule_IMPORT_F_MASK) == 0);
STATIC_ASSERT((DeeModule_IMPORT_F_NOGDEC & _DeeModule_IMPORT_F_MASK) == 0);
#endif /* !CONFIG_NO_DEC */

/* # of extra characters to pre-alloc in "abs_filename"
 * (unless `_DeeModule_IMPORT_F_NO_INHERIT_FILENAME' is set) */
#ifdef CONFIG_NO_DEC
#define DeeModule_OpenFile_impl4_EXTRA_CHARS 0
#else /* CONFIG_NO_DEC */
#define DeeModule_OpenFile_impl4_EXTRA_CHARS 1 /* for the leading "." in dec filenames */
#endif /* !CONFIG_NO_DEC */

/* Space needed for extra ".dee" / ".dll" / ".so" at the end */
#define DeeModule_OpenFile_impl3_EXTRA_CHARS \
	(COMPILER_STRLEN(DeeSystem_SOEXT) > 4 ? COMPILER_STRLEN(DeeSystem_SOEXT) : 4)
#define DeeModule_OpenFile_impl_EXTRA_CHARS                                      \
	(DeeModule_OpenFile_impl4_EXTRA_CHARS > DeeModule_OpenFile_impl3_EXTRA_CHARS \
	 ? DeeModule_OpenFile_impl4_EXTRA_CHARS                                      \
	 : DeeModule_OpenFile_impl3_EXTRA_CHARS)
enum{ DeeModule_OpenFile_impl_EXTRA_CHARS_ = DeeModule_OpenFile_impl_EXTRA_CHARS };
#undef DeeModule_OpenFile_impl_EXTRA_CHARS
#define DeeModule_OpenFile_impl_EXTRA_CHARS DeeModule_OpenFile_impl_EXTRA_CHARS_


INTDEF struct module_symbol empty_module_buckets[];

PRIVATE NONNULL((1)) void DCALL
module_destroy_untracked(DREF /*untracked*/ DeeModuleObject *self) {
#ifndef CONFIG_NO_DEX
	ASSERT(Dee_TYPE(self) != &DeeModuleDex_Type);
#endif /* !CONFIG_NO_DEX */
	ASSERT(self->mo_absname == NULL);
	if (Dee_TYPE(self) == &DeeModuleDee_Type) {
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
		DeeDec_DestroyUntracked(self);
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
		DREF DeeCodeObject *root;
		ASSERT(self->mo_dir == NULL);
		Dee_XDecrefv(self->mo_globalv, self->mo_globalc);
		root = self->mo_moddata.mo_rootcode;
		self->mo_moddata.mo_rootcode = NULL;
		Dee_Decref(root);
		Dee_Decrefv(self->mo_importv, self->mo_importc);
		Dee_Free((void *)self->mo_importv);
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
	} else {
		ASSERT(self->mo_dir == NULL);
		ASSERT(Dee_TYPE(self) == &DeeModuleDir_Type);
		ASSERT(self->mo_importc == 0);
//		ASSERT(self->mo_importv == NULL); /* Not alllocated */
		ASSERT(self->mo_globalc == 0);
		ASSERT(self->mo_bucketm == 0);
		ASSERT(self->mo_bucketv == empty_module_buckets);
	}
	DeeGCObject_Free(self);
}

#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
INTDEF NONNULL((1)) void DCALL
DeeDec_heapregion_destroy(struct Dee_heapregion *__restrict self);

/* Free relocation-only data from the image mapping of `DeeDec_Ehdr'.
 *
 * - Dee_DEC_TYPE_RELOC:
 *   For EHDRs created by `DeeDec_Relocate()', this will try to munmap()
 *   or realloc_in_place() all data of `self' that comes after the end
 *   of the file's object heap (iow: will truncate `self->e_mapping' to
 *   have a size of `offsetof(DeeDec_Ehdr, e_heap) + self->e_heap.hr_size')
 *
 * - Dee_DEC_TYPE_IMAGE:
 *   For EHDRs created by `DeeDecWriter_PackModule()', this frees the
 *   relocation tables that were stolen from the associated `DeeDecWriter'
 *   and only kept within the dec's EHDR for `DeeDec_DestroyUntracked()'
 *   to be able to undo incref()s that had been done.
 *   Because 'Dee_DEC_TYPE_RELOC' may be converted to this type of EHDR,
 *   this type will also try to truncate `self->e_mapping'. */
INTDEF NONNULL((1)) void DCALL
DeeDec_Ehdr_FreeRelocationData(DeeDec_Ehdr *__restrict self);

/* Start GC-tracking (and allowing reverse address lookup of) "self", as returned by:
 * - DeeDec_OpenFile()
 * - DeeDec_Relocate()
 * - DeeDecWriter_PackModule() */
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF /*tracked*/ struct Dee_module_object *DCALL
DeeDec_Track(DREF /*untracked*/ struct Dee_module_object *__restrict self) {
	Dec_Ehdr *ehdr = DeeDec_Ehdr_FromModule(self);
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeModuleDee_Type);
	ASSERT(ehdr->e_heap.hr_destroy == &DeeDec_heapregion_destroy);
	ASSERT(DeeHeap_GetRegionOf(DeeGC_Head(self)) == &ehdr->e_heap);

	/* munmap() / realloc_in_place() unused, trailing memory
	 * from the dec image (i.e.: memory containing relocation
	 * info and dependency strings; iow: memory that's no longer
	 * needed now that the module's been fully loaded) */
	DeeDec_Ehdr_FreeRelocationData(ehdr);

	module_byaddr_lock_write();
	module_byaddr_ensure_initialized();

	/* Insert module into by-address tree so allow reverse module lookup by-address */
	ASSERT(DeeMapFile_GetAddr(&ehdr->e_mapping) == (void *)ehdr);
	self->mo_minaddr = (byte_t *)ehdr;
	self->mo_maxaddr = (byte_t *)ehdr + DeeMapFile_GetSize(&ehdr->e_mapping) - 1;
	module_byaddr_insert(&module_byaddr_tree, self);

	/* Link in GC objects (if there are any) */
#if __SIZEOF_SIZE_T__ == 4
#define IMAGE_GC_HEADTAIL_MATCH_RELOC                                                                                                  \
	((offsetof(Dec_Ehdr, e_typedata.td_image.ei_offsetof_gchead) == DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_gchead) && \
	 (offsetof(Dec_Ehdr, e_typedata.td_image.ei_offsetof_gctail) == DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_gctail))
#else /* __SIZEOF_SIZE_T__ == 4 */
#define IMAGE_GC_HEADTAIL_MATCH_RELOC 0
#endif /* __SIZEOF_SIZE_T__ == 4 */
	DeeGC_TrackMany_Lock();
	if (ehdr->e_type == Dee_DEC_TYPE_IMAGE && !IMAGE_GC_HEADTAIL_MATCH_RELOC) {
		struct gc_head *gc_head = (struct gc_head *)((byte_t *)ehdr + ehdr->e_typedata.td_image.ei_offsetof_gchead);
		struct gc_head *gc_tail = (struct gc_head *)((byte_t *)ehdr + ehdr->e_typedata.td_image.ei_offsetof_gctail);
		ASSERT(ehdr->e_typedata.td_image.ei_offsetof_gchead);
		ASSERT(ehdr->e_typedata.td_image.ei_offsetof_gctail);
		DeeGC_TrackMany_Exec(DeeGC_Object(gc_head), DeeGC_Object(gc_tail));
	} else {
		struct gc_head *gc_head = (struct gc_head *)((byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_gchead);
		struct gc_head *gc_tail = (struct gc_head *)((byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_gctail);
		ASSERT(ehdr->e_typedata.td_reloc.er_offsetof_gchead);
		ASSERT(ehdr->e_typedata.td_reloc.er_offsetof_gctail);
		DeeGC_TrackMany_Exec(DeeGC_Object(gc_head), DeeGC_Object(gc_tail));
	}
	DeeGC_TrackMany_Unlock();
#undef IMAGE_GC_HEADTAIL_MATCH_RELOC

	/* Add debug info to every object from the module's heap (s.a. runtime/heap.c:gcscan__pointer())
	 *    -> Needed so the GC-based leak detector is able to recursively scan the bodies of
	 *       static heap allocations of the module, as well as display those allocations as
	 *       memory leaks if they aren't referenced anywhere.
	 *    -> Also needed so failure to free the entire heap counts as memory leaks. */
	DeeDbgHeap_AddHeapRegion(&ehdr->e_heap, self->mo_absname);
	module_byaddr_lock_endwrite();
	return self;
}
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */

#define DeeModule_DestroyAnonymousDirectory(self) \
	(Dee_DecrefNokill(&DeeModuleDir_Type), DeeGCObject_Free(self))

#define sizeof_DeeModuleDir offsetof(DeeModuleObject, mo_moddata)
PRIVATE WUNUSED DREF /*untracked*/ DeeModuleObject *DCALL
DeeModule_CreateAnonymousDirectory(void) {
	DREF /*untracked*/ DeeModuleObject *result;
	result = (DREF /*untracked*/ DeeModuleObject *)DeeGCObject_Malloc(sizeof_DeeModuleDir);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeModuleDir_Type);
	result->mo_absname = NULL; /* Will be filled in by caller */
	result->mo_libname.mle_dat.mle_mod = result;
	result->mo_libname.mle_name = NULL;
	result->mo_dir = NULL; /* Will be initialized lazily */
	result->mo_init = Dee_MODULE_INIT_INITIALIZED;
	result->mo_buildid.mbi_word64[0] = 0;
	result->mo_buildid.mbi_word64[1] = 0;
	result->mo_flags = Dee_MODULE_FABSFILE | Dee_MODULE_FHASBUILDID;
	result->mo_importc = 0;
	result->mo_globalc = 0;
	result->mo_bucketm = 0;
	result->mo_bucketv = empty_module_buckets;
	Dee_atomic_rwlock_init(&result->mo_lock);
	weakref_support_init(result);
done:
	return result;
}

#ifndef CONFIG_NO_DEC
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) size_t DCALL
DeeFile_WriteDecEhdr(DeeObject *__restrict stream,
                     Dec_Ehdr *__restrict ehdr) {
	size_t result;
	/* Write the actual file with embedded pointers set to "NULL".
	 * This is done for multiple reasons:
	 *  #1: When the Build ID MD5 is calculated, these fields are also "NULL"
	 *  #2: By not embedding nonsensical pointers within the file, output becomes more reproducible */
	void (DCALL *saved_hr_destroy)(struct Dee_heapregion *__restrict self) = ehdr->e_heap.hr_destroy;
	struct DeeMapFile saved_e_mapping = ehdr->e_mapping;
	ehdr->e_heap.hr_destroy = NULL;
	bzero(&ehdr->e_mapping, sizeof(ehdr->e_mapping));

	/* Write data to file */
	result = DeeFile_WriteAll(stream, ehdr, ehdr->e_typedata.td_reloc.er_offsetof_eof);

	/* Restore runtime info. */
	ehdr->e_heap.hr_destroy = saved_hr_destroy;
	ehdr->e_mapping = saved_e_mapping;
	return result;
}
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */
#endif /* !CONFIG_NO_DEC */

PRIVATE WUNUSED NONNULL((1)) DREF /*untracked*/ DeeModuleObject *DCALL
DeeModule_OpenFile_impl4(/*utf-8*/ char *__restrict abs_filename, size_t abs_filename_length,
                         unsigned int flags, struct Dee_compiler_options *options) {
	struct Dee_compiler_options used_options;
	DREF /*untracked*/ DeeModuleObject *result;
	DREF DeeObject *source_stream;
	DeeThreadObject *caller;
	struct Dee_import_frame frame;

#ifndef CONFIG_NO_DEC
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	bool has_broken_dec_file = false;
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	if ((flags & (_DeeModule_IMPORT_F_IS_DEE_FILE | DeeModule_IMPORT_F_NOLDEC)) ==
	    /*    */ (_DeeModule_IMPORT_F_IS_DEE_FILE)) {
		size_t pathsize, basesize;
		uint64_t dee_file_last_modified;
		DREF DeeObject *dec_stream;
		char *basename;
		ASSERT(abs_filename_length >= 4);
		ASSERT(abs_filename[abs_filename_length - 4] == '.');
		ASSERT(abs_filename[abs_filename_length - 3] == 'd');
		ASSERT(abs_filename[abs_filename_length - 2] == 'e');
		ASSERT(abs_filename[abs_filename_length - 1] == 'e');

		basename = (char *)memrchr(abs_filename, DeeSystem_SEP, abs_filename_length);
		if likely(basename) {
			++basename;
		} else {
			/* Probably shouldn't ever get here... */
			basename = abs_filename;
		}
		/* Form the filename for a .dec file */
		basesize = (size_t)((abs_filename + abs_filename_length) - basename);

		/* Move up "module_name.de" by 1 (don't move the
		 * trailing "e\0", which'll be replaced with "c\0") */
		memmoveupc(basename + 1, basename, basesize - 1, sizeof(char));
		basename[0] = '.';
		basename[basesize + 0] = 'c';
		basename[basesize + 1] = '\0';

		/* Open file */
		dec_stream = DeeFile_OpenString(abs_filename, OPEN_FRDONLY | OPEN_FCLOEXEC, 0);

		/* Restore the normal ".dee" file extension. */
		memmovedownc(basename, basename + 1, basesize - 1, sizeof(char));
		basename[basesize - 1] = 'e';
		basename[basesize - 0] = '\0';

		/* Check if file could be opened. */
		if (!ITER_ISOK(dec_stream)) {
			if unlikely(!dec_stream)
				goto err;
			goto no_dec_file;
		}

		/* Get last-modified timestamp of .dee file (to
		 * check if source changed since last compilation) */
		dee_file_last_modified = DeeSystem_GetLastModifiedString(abs_filename);
		if unlikely(dee_file_last_modified == (uint64_t)-1) {
			Dee_Decref_likely(dec_stream);
			goto err;
		}

		/* Handle special case: when we're unable to determine the source file's last-modified
		 * timestamp, that can only be because that source file no longer exists (or is somehow
		 * inaccessible to us?). Anyways: in either case, the ".dec" file should be deleted (so
		 * there aren't any leftovers in the likely case of the source having been deleted), and
		 * if the source actually still exists, then it must be re-compiled! */
		if unlikely(dee_file_last_modified == 0) {
			int status;
			Dee_Decref_likely(dec_stream);
			Dee_DPRINTF("[LD][dec %q] Cannot query timestamp of associated source file. "
			            /**/ "Assume file was deleted, and remove .dec file\n",
			            abs_filename);
			memmoveupc(basename + 1, basename, basesize - 1, sizeof(char));
			basename[0] = '.';
			basename[basesize + 0] = 'c';
			basename[basesize + 1] = '\0';
			status = DeeSystem_UnlinkString(abs_filename, false);
			memmovedownc(basename, basename + 1, basesize - 1, sizeof(char));
			basename[basesize - 1] = 'e';
			basename[basesize - 0] = '\0';
			if (status < 0)
				goto err;
			goto no_dec_file;
		}

		/* Check open file status */
		pathsize = (size_t)(basename - abs_filename);
		Dee_DPRINTF("[LD][dec %q] Loading dec file\n", abs_filename);
		result = DeeModule_OpenDecFile_impl(dec_stream, abs_filename, pathsize,
		                                    options, dee_file_last_modified);
//		Dee_Decref(dec_stream); /* Inherited by `DeeModule_OpenDecFile_impl()' */
		if (result != (DREF DeeModuleObject *)ITER_DONE)
			return result;
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
		has_broken_dec_file = true;
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	}
no_dec_file:
#endif /* !CONFIG_NO_DEC */

	/* Try to open the .dee file so we can compile it. */
	source_stream = DeeFile_OpenString(abs_filename, OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
	if (!ITER_ISOK(source_stream)) {
		ASSERT(source_stream == Dee_AsObject(DeeModule_IMPORT_ERROR) ||
		       source_stream == Dee_AsObject(DeeModule_IMPORT_ENOENT));
		if unlikely(source_stream != Dee_AsObject(DeeModule_IMPORT_ERROR) &&
		            !(flags & DeeModule_IMPORT_F_ENOENT)) {
			err_file_not_found_string(abs_filename);
			source_stream = Dee_AsObject(DeeModule_IMPORT_ERROR);
		}
		return (DeeModuleObject *)source_stream;
	}

	/* Compile "source_stream" as an anonymous module. */
	if (!options) {
		bzero(&used_options, sizeof(used_options));
	} else {
		memcpy(&used_options, options, sizeof(used_options));
	}
	if (used_options.co_pathname == NULL)
		used_options.co_pathname = abs_filename;

	/* Deal with recursion, where the calling thread is already in the process
	 * of compiling the same source file (i.e. we're a nested call with an
	 * identical 'abs_filename' string within the same thread) */
	caller = DeeThread_Self();
	frame.if_absfile = abs_filename;
	frame.if_prev = caller->t_import_curr;
	if (flags & DeeModule_IMPORT_F_ERECUR) {
		struct Dee_import_frame *iter = frame.if_prev;
		for (; iter; iter = iter->if_prev) {
			if unlikely(fs_strcmp(iter->if_absfile, abs_filename) == 0) {
				Dee_Decref_likely(source_stream);
				return DeeModule_IMPORT_ERECUR;
			}
		}
	}
	caller->t_import_curr = &frame;
	/* XXX: Similar to how we switch stacks when executing user-code, that should
	 *      also be done here every 2-3 nested imports (or maybe even: every time,
	 *      starting with the 2nd nested import; iow: when "frame.if_prev != NULL") */

#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	/* Can only generate .dec files for .dee files */
	if (!(flags & _DeeModule_IMPORT_F_IS_DEE_FILE))
		flags |= DeeModule_IMPORT_F_NOGDEC;

	{
		DeeDecWriter writer;
		DeeDec_Ehdr *ehdr;
#if DeeModule_IMPORT_F_NOGDEC == DeeDecWriter_F_NRELOC
		if unlikely(DeeDecWriter_Init(&writer, flags & DeeModule_IMPORT_F_NOGDEC))
			goto err_compile;
#else /* DeeModule_IMPORT_F_NOGDEC == DeeDecWriter_F_NRELOC */
		if unlikely(DeeDecWriter_Init(&writer, flags & DeeModule_IMPORT_F_NOGDEC
		                                       ? DeeDecWriter_F_NRELOC
		                                       : DeeDecWriter_F_NORMAL))
			goto err_compile;
#endif /* DeeModule_IMPORT_F_NOGDEC != DeeDecWriter_F_NRELOC */

		/* Compile source code and write module to "writer" */
		if unlikely(DeeExec_CompileModuleStream_impl((DeeSerial *)&writer, source_stream,
		                                             0, 0, DeeExec_RUNMODE_DEFAULT,
		                                             &used_options, NULL))
			goto err_compile_writer;

		/* Pack written module into an EHDR */
		ehdr = DeeDecWriter_PackEhdr(&writer, abs_filename, abs_filename_length,
		                             flags & ~DeeModule_IMPORT_F_CTXDIR);
		if unlikely(!ehdr)
			goto err_compile_writer;

		/* If not disabled, emit a .dec file */
#ifndef CONFIG_NO_DEC
		if (!(flags & DeeModule_IMPORT_F_NOGDEC)) {
			ASSERT(abs_filename_length >= 4);
			ASSERT(abs_filename[abs_filename_length - 4] == '.');
			ASSERT(abs_filename[abs_filename_length - 3] == 'd');
			ASSERT(abs_filename[abs_filename_length - 2] == 'e');
			ASSERT(abs_filename[abs_filename_length - 1] == 'e');
			if (ehdr->e_type == Dee_DEC_TYPE_RELOC) {
				/* generate a .dec file */
				size_t basesize;
				char *basename;
				DREF DeeObject *output_stream;
				Dee_DPRINTF("[LD][dec %q] Writing dec file\n", abs_filename);

				basename = (char *)memrchr(abs_filename, DeeSystem_SEP, abs_filename_length);
				basename = basename ? basename + 1 : /* Probably shouldn't ever get here... */ abs_filename;

				/* Form the filename for a .dec file */
				basesize = (size_t)((abs_filename + abs_filename_length) - basename);

				/* Move up "module_name.de" by 1 (don't move the
				 * trailing "e\0", which'll be replaced with "c\0") */
				memmoveupc(basename + 1, basename, basesize - 1, sizeof(char));
				basename[0] = '.';
				basename[basesize + 0] = 'c';
				basename[basesize + 1] = '\0';

				/* Output file */
				output_stream = DeeFile_OpenString(abs_filename,
				                                   OPEN_FWRONLY | OPEN_FCREAT |
				                                   OPEN_FTRUNC | OPEN_FHIDDEN |
				                                   OPEN_FCLOEXEC,
				                                   0644);
				if likely(output_stream) {
					size_t write_status = DeeFile_WriteDecEhdr(output_stream, ehdr);
					Dee_Decref_likely(output_stream);
					if unlikely(write_status == (size_t)-1)
						output_stream = NULL;
				}

				/* Restore the normal ".dee" file extension. */
				memmovedownc(basename, basename + 1, basesize - 1, sizeof(char));
				basename[basesize - 1] = 'e';
				basename[basesize - 0] = '\0';
	
				if unlikely(!output_stream) {
					if (!DeeError_Handled(Dee_ERROR_HANDLED_NORMAL))
						goto err_compile_writer_ehdr;
					/* Failed to generate dec file... :( */
				}
			} else if (has_broken_dec_file) {
				int status;
				size_t basesize;
				char *basename;
				Dee_DPRINTF("[LD][dec %q] Deleting broken dec file\n", abs_filename);
				basename = (char *)memrchr(abs_filename, DeeSystem_SEP, abs_filename_length);
				basename = basename ? basename + 1 : /* Probably shouldn't ever get here... */ abs_filename;
				basesize = (size_t)((abs_filename + abs_filename_length) - basename);
				memmoveupc(basename + 1, basename, basesize - 1, sizeof(char));
				basename[0] = '.';
				basename[basesize + 0] = 'c';
				basename[basesize + 1] = '\0';
				status = DeeSystem_UnlinkString(abs_filename, false);
				memmovedownc(basename, basename + 1, basesize - 1, sizeof(char));
				basename[basesize - 1] = 'e';
				basename[basesize - 0] = '\0';
				if unlikely(status < 0)
					goto err_compile_writer_ehdr;
			}
		}
#endif /* !CONFIG_NO_DEC */

		/* Pack ehdr into a proper module. */
		result = DeeDecWriter_PackModule(&writer, ehdr);

		/* Finalize dec writer */
		DeeDecWriter_Fini(&writer);

		__IF0 {
#ifndef CONFIG_NO_DEC
err_compile_writer_ehdr:
			DeeDec_Ehdr_Destroy(ehdr);
#endif /* !CONFIG_NO_DEC */
err_compile_writer:
			DeeDecWriter_Fini(&writer);
err_compile:
			result = DeeModule_IMPORT_ERROR;
		}
	}
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	result = DeeExec_CompileModuleStream_impl(source_stream, 0, 0,
	                                          DeeExec_RUNMODE_DEFAULT,
	                                          &used_options, NULL);
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
	caller->t_import_curr = frame.if_prev;
	Dee_Decref_likely(source_stream);

	if (result == DeeModule_IMPORT_ERROR) {
		if ((flags & (_DeeModule_IMPORT_F_OUT_IS_RAW_FILE | _DeeModule_IMPORT_F_IS_DEE_FILE)) ==
		    /*    */ (_DeeModule_IMPORT_F_OUT_IS_RAW_FILE)) {
			/* If compilation failed because "source_stream"
			 * is a directory, create a directory module. */
			if (DeeError_Catch(&DeeError_IsDirectory))
				return DeeModule_CreateAnonymousDirectory();
		}
		goto err;
	}

#ifndef CONFIG_EXPERIMENTAL_MMAP_DEC
#ifndef CONFIG_NO_DEC
	if ((flags & (_DeeModule_IMPORT_F_IS_DEE_FILE | DeeModule_IMPORT_F_NOGDEC)) ==
	    /*....*/ (_DeeModule_IMPORT_F_IS_DEE_FILE)) {
		/* generate a .dec file */
		int status;
		size_t basesize;
		char *basename;
		ASSERT(abs_filename_length >= 4);
		ASSERT(abs_filename[abs_filename_length - 4] == '.');
		ASSERT(abs_filename[abs_filename_length - 3] == 'd');
		ASSERT(abs_filename[abs_filename_length - 2] == 'e');
		ASSERT(abs_filename[abs_filename_length - 1] == 'e');

		basename = (char *)memrchr(abs_filename, DeeSystem_SEP, abs_filename_length);
		if likely(basename) {
			++basename;
		} else {
			/* Probably shouldn't ever get here... */
			basename = abs_filename;
		}

		/* Form the filename for a .dec file */
		basesize = (size_t)((abs_filename + abs_filename_length) - basename);

		/* Move up "module_name.de" by 1 (don't move the
		 * trailing "e\0", which'll be replaced with "c\0") */
		memmoveupc(basename + 1, basename, basesize - 1, sizeof(char));
		basename[0] = '.';
		basename[basesize + 0] = 'c';
		basename[basesize + 1] = '\0';

		/* Open file */
		status = DeeCompiler_LockWrite();
		if likely(status == 0) {
			status = dec_create(result, abs_filename);
			DeeCompiler_LockEndWrite();
		}

		/* Restore the normal ".dee" file extension. */
		memmovedownc(basename, basename + 1, basesize - 1, sizeof(char));
		basename[basesize - 1] = 'e';
		basename[basesize - 0] = '\0';

		if unlikely(status) {
			module_destroy_untracked(result);
			result = DeeModule_IMPORT_ERROR;
		}
	}
#endif /* !CONFIG_NO_DEC */
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
	return result;
err:
	return DeeModule_IMPORT_ERROR;
}

PRIVATE WUNUSED NONNULL((1)) DREF /*untracked_if(Dee_TYPE(return) != DeeModuleDex_Type)*/ DeeModuleObject *DCALL
DeeModule_OpenFile_impl3(/*utf-8*/ char **p_abs_filename, size_t abs_filename_length,
                         unsigned int *p_flags, struct Dee_compiler_options *options) {
	DREF DeeModuleObject *result;
	char *abs_filename_end;
	if (*p_flags & (_DeeModule_IMPORT_F_IS_DEE_FILE | DeeModule_IMPORT_F_FILNAM))
		return DeeModule_OpenFile_impl4(*p_abs_filename, abs_filename_length, *p_flags, options);
	if (*p_flags & _DeeModule_IMPORT_F_NO_INHERIT_FILENAME) {
		char *copy = (char *)Dee_Mallocc(abs_filename_length + DeeModule_OpenFile_impl_EXTRA_CHARS + 1, sizeof(char));
		if unlikely(!copy)
			goto err;
		memcpyc(copy, *p_abs_filename, abs_filename_length + 1, sizeof(char));
		*p_flags &= ~_DeeModule_IMPORT_F_NO_INHERIT_FILENAME;
		*p_abs_filename = copy;
	}

	/* Try to append a trailing ".dee" to the filename and try to open that */
	abs_filename_end = *p_abs_filename + abs_filename_length;
	*abs_filename_end++ = '.';
	*abs_filename_end++ = 'd';
	*abs_filename_end++ = 'e';
	*abs_filename_end++ = 'e';
	*abs_filename_end = '\0';
	abs_filename_length += 4;
	result = DeeModule_OpenFile_impl4(*p_abs_filename, abs_filename_length,
	                                  *p_flags | DeeModule_IMPORT_F_ENOENT |
	                                  _DeeModule_IMPORT_F_IS_DEE_FILE,
	                                  options);
	/* If the ".dee" file is (for some reason) a directory, move on to check other file types */
	if (result == DeeModule_IMPORT_ERROR && DeeError_Catch(&DeeError_IsDirectory))
		result = DeeModule_IMPORT_ENOENT;
	if (result == DeeModule_IMPORT_ENOENT) {
		abs_filename_end -= 4;
#ifndef CONFIG_NO_DEX
		if (!(*p_flags & DeeModule_IMPORT_F_NOLDEX)) {
			void *dex_handle;
			/* Attempt to load a dex module after appending ".dll" or ".so" to `abs_filename_end' */
			strcpy(abs_filename_end, DeeSystem_SOEXT);
			dex_handle = DeeSystem_DlOpenString(*p_abs_filename);
			if (dex_handle != DeeSystem_DlOpen_FAILED) {
				if unlikely(!dex_handle)
					goto err;

				/* Indicate to our caller that they shouldn't Dee_Free(*p_abs_filename)
				 * Reason being: the call to `DeeModule_OpenDex()' below will always
				 *               inherit that string. */
				*p_flags |= _DeeModule_IMPORT_F_NO_INHERIT_FILENAME;

				/* Truncate the trailing file-extension "DeeSystem_SOEXT" from the filename. */
				*abs_filename_end = '\0';

				/* load module from "dex_handle". This will also hook the module
				 * into "module_abstree_root" if this is the first time this lib
				 * file was loaded; which differs from the regular case where the
				 * act of linking a module into "module_abstree_root" would be
				 * done by our caller, and only if `DeeModule_IMPORT_F_ANONYM'
				 * wasn't set. Reason being: dex modules cannot be loaded using
				 * an anonymous module. */
				return DeeModule_OpenDex(*p_abs_filename, dex_handle);
			}
		}
#endif /* !CONFIG_NO_DEX */
		*abs_filename_end = '\0';

		/* Try to open the specified filename as-is (caller might be trying to open a directory or similar) */
		*p_flags |= _DeeModule_IMPORT_F_OUT_IS_RAW_FILE;
		result = DeeModule_OpenFile_impl4(*p_abs_filename, abs_filename_length, *p_flags, options);
	}
	return result;
err:
	return DeeModule_IMPORT_ERROR;
}

PRIVATE WUNUSED NONNULL((1)) DREF /*tracked*/ DeeModuleObject *DCALL
DeeModule_OpenFile_impl2(/*inherit_if(!(flags & _DeeModule_IMPORT_F_NO_INHERIT_FILENAME))*/
                         /*utf-8*/ char *__restrict abs_filename, size_t abs_filename_length,
                         unsigned int flags, struct Dee_compiler_options *options) {
	static char const deemon_ext[] = ".dee";
	DREF DeeModuleObject *result;
	ASSERT(abs_filename[abs_filename_length] == '\0');
#ifndef CONFIG_NO_DEX
	/* Check if the filename ends with "DeeSystem_SOEXT". If so, try to open as a dex module */
	if (!(flags & DeeModule_IMPORT_F_NOLDEX) &&
	    abs_filename_length >= COMPILER_STRLEN(DeeSystem_SOEXT) &&
	    fs_bcmp(abs_filename + abs_filename_length - COMPILER_STRLEN(DeeSystem_SOEXT),
	            DeeSystem_SOEXT, COMPILER_STRLEN(DeeSystem_SOEXT) * sizeof(char)) == 0) {
		void *dex_handle = DeeSystem_DlOpenString(abs_filename);
		if (dex_handle != DeeSystem_DlOpen_FAILED) {
			if unlikely(!dex_handle)
				goto err;
			if ((flags & (_DeeModule_IMPORT_F_IS_DEE_FILE | _DeeModule_IMPORT_F_NO_INHERIT_FILENAME)) ==
			    /*    */ (_DeeModule_IMPORT_F_IS_DEE_FILE | _DeeModule_IMPORT_F_NO_INHERIT_FILENAME)) {
				/* When "_DeeModule_IMPORT_F_IS_DEE_FILE" is the case, then we **need** to
				 * write to "abs_filename" in order to form the relevant .dec filename. */
				char *copy = (char *)Dee_Mallocc(abs_filename_length + DeeModule_OpenFile_impl_EXTRA_CHARS + 1, sizeof(char));
				if unlikely(!copy) {
					DeeSystem_DlClose(dex_handle);
					goto err;
				}
				memcpyc(copy, abs_filename, abs_filename_length + 1, sizeof(char));
				flags &= ~_DeeModule_IMPORT_F_NO_INHERIT_FILENAME;
				abs_filename = copy;
			}
			/* Truncate the trailing file-extension "DeeSystem_SOEXT" from the filename. */
			abs_filename[abs_filename_length - COMPILER_STRLEN(DeeSystem_SOEXT)] = '\0';

			/* Load module from "dex_handle" (s.a. similar code in `DeeModule_OpenFile_impl3()') */
			return DeeModule_OpenDex(abs_filename, dex_handle);
		}
	}
#endif /* !CONFIG_NO_DEX */

	if (abs_filename_length >= COMPILER_STRLEN(deemon_ext) &&
	    fs_bcmp(abs_filename + abs_filename_length - COMPILER_STRLEN(deemon_ext),
	            deemon_ext, COMPILER_STRLEN(deemon_ext) * sizeof(char)) == 0)
		flags |= _DeeModule_IMPORT_F_IS_DEE_FILE;
#ifndef CONFIG_NO_DEC
	if ((flags & (_DeeModule_IMPORT_F_IS_DEE_FILE | _DeeModule_IMPORT_F_NO_INHERIT_FILENAME)) ==
	    /*    */ (_DeeModule_IMPORT_F_IS_DEE_FILE | _DeeModule_IMPORT_F_NO_INHERIT_FILENAME)) {
		/* When "_DeeModule_IMPORT_F_IS_DEE_FILE" is the case, then we **need** to
		 * write to "abs_filename" in order to form the relevant .dec filename. */
		char *copy = (char *)Dee_Mallocc(abs_filename_length + DeeModule_OpenFile_impl_EXTRA_CHARS + 1, sizeof(char));
		if unlikely(!copy)
			goto err;
		memcpyc(copy, abs_filename, abs_filename_length + 1, sizeof(char));
		flags &= ~_DeeModule_IMPORT_F_NO_INHERIT_FILENAME;
		abs_filename = copy;
	}
#endif /* !CONFIG_NO_DEC */

	/* Skip "module_abstree_*" if the module is being imported anonymously. */
	if (flags & DeeModule_IMPORT_F_ANONYM) {
		result = DeeModule_OpenFile_impl3((char **)&abs_filename, abs_filename_length, &flags, options);
		if (DeeModule_IMPORT_ISOK(result)) {
#ifndef CONFIG_NO_DEX
			if (Dee_TYPE(result) == &DeeModuleDex_Type)
				goto free_abs_filename_and_return_result;
#endif /* !CONFIG_NO_DEX */
			if (Dee_TYPE(result) == &DeeModuleDir_Type)
				goto remember_dir_module;
			if ((flags & (_DeeModule_IMPORT_F_OUT_IS_RAW_FILE)) ||
			    (flags & (DeeModule_IMPORT_F_FILNAM | _DeeModule_IMPORT_F_IS_DEE_FILE)) ==
			    /*....*/ (DeeModule_IMPORT_F_FILNAM))
				result->mo_flags |= Dee_MODULE_FABSFILE;
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
			result = DeeDec_Track(result);
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
			result = DeeGC_TRACK(DeeModuleObject, result);
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
		}
		goto free_abs_filename_and_return_result;
	}

	/* Check if the file is already in-cache. */
	module_abstree_lock_read();
	if (flags & _DeeModule_IMPORT_F_IS_DEE_FILE) {
		ASSERT(abs_filename_length >= 4);
		abs_filename[abs_filename_length - 4] = '\0';
		result = module_abstree_locate(module_abstree_root, abs_filename);
		abs_filename[abs_filename_length - 4] = '.';
	} else {
		result = module_abstree_locate(module_abstree_root, abs_filename);
	}
	if (result && Dee_IncrefIfNotZero(result)) {
		module_abstree_lock_endread();
		goto free_abs_filename_and_return_result;
	}
	module_abstree_lock_endread();

	/* Load a new .dec/.dee/DT_DIR module */
	result = DeeModule_OpenFile_impl3((char **)&abs_filename, abs_filename_length, &flags, options);

	/* On success, insert the module into the "module_abstree_*" tree. */
	if (DeeModule_IMPORT_ISOK(result)) {
		DREF DeeModuleObject *existing;
#ifndef CONFIG_NO_DEX
		/* When loading a dex module, linking in that module is already (and always) done */
		if (Dee_TYPE(result) == &DeeModuleDex_Type)
			goto free_abs_filename_and_return_result;
#endif /* !CONFIG_NO_DEX */
		if (flags & _DeeModule_IMPORT_F_IS_DEE_FILE)
			abs_filename_length -= 4; /* The trailing ".dee" isn't included in "mo_absname". */
remember_dir_module:
		ASSERT(!result->mo_absname);
		if (flags & _DeeModule_IMPORT_F_NO_INHERIT_FILENAME) {
			char *copy = (char *)Dee_Mallocc(abs_filename_length + 1, sizeof(char));
			if unlikely(!copy)
				goto err_r;
			memcpyc(copy, abs_filename, abs_filename_length, sizeof(char));
			abs_filename = copy;
		}
		abs_filename[abs_filename_length] = '\0'; /* Force 0-termination */
		result->mo_absname = abs_filename; /* Inherit */
		if ((flags & (_DeeModule_IMPORT_F_OUT_IS_RAW_FILE)) ||
		    (flags & (DeeModule_IMPORT_F_FILNAM | _DeeModule_IMPORT_F_IS_DEE_FILE)) ==
		    /*....*/ (DeeModule_IMPORT_F_FILNAM))
			result->mo_flags |= Dee_MODULE_FABSFILE;

		/* Insert the module into the global "module_abstree_*" tree.
		 *
		 * During this step, also do another check to ensure that the
		 * module in question isn't already loaded, or if it is: that
		 * the existing instance has already been destroyed (meaning
		 * that we can just remove + replace it). */
		module_abstree_lock_write();
		existing = module_abstree_locate(module_abstree_root, abs_filename);
		if unlikely(existing) {
			if (Dee_IncrefIfNotZero(existing)) {
				module_abstree_lock_endwrite();
				module_destroy_untracked(result);
				Dee_Free(abs_filename);
				return existing;
			}
			ASSERT(existing->mo_absnode.rb_par != DeeModule_IMPORT_ENOENT);
			module_abstree_removenode(&module_abstree_root, existing);
			existing->mo_absnode.rb_par = DeeModule_IMPORT_ENOENT;
		}
		module_abstree_insert(&module_abstree_root, result);
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
		if (Dee_TYPE(result) == &DeeModuleDee_Type) {
			/* TODO: This call blocking-acquires a lock to "module_byaddr_lock_write()"
			 *       while we're already holding "module_abstree_lock_write()". Instead,
			 *       if both locks are needed, then do so above (to guaranty that there
			 *       won't be any deadlocks, which is currently only guarantied because
			 *       no other piece of code exists that acquires these locks in reverse
			 *       order while also blocking) */
			result = DeeDec_Track(result);
		} else
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */
		{
			result = DeeGC_TRACK(DeeModuleObject, result);
		}
		module_abstree_lock_endwrite();
	} else {
free_abs_filename_and_return_result:
		if (!(flags & _DeeModule_IMPORT_F_NO_INHERIT_FILENAME))
			Dee_Free(abs_filename);
	}
	return result;
err_r:
	module_destroy_untracked(result);
err:
	if (!(flags & _DeeModule_IMPORT_F_NO_INHERIT_FILENAME))
		Dee_Free(abs_filename);
	return DeeModule_IMPORT_ERROR;
}

/* Append "filename" to "pathname", whilst normalizing it.
 * @param: p_must_be_directory: Set to "true" if resulting path must be a directory for it to be loadable. */
PRIVATE WUNUSED NONNULL((1, 3, 5, 6)) /*utf-8*/ char *DCALL
cat_and_normalize_paths(/*utf-8*/ char const *__restrict pathname, size_t pathname_size,
                        /*utf-8*/ char const *__restrict filename, size_t filename_size,
                        size_t *__restrict p_result_strlen,
                        bool *__restrict p_must_be_directory) {
	char const *filename_end;
	size_t final_result_size;
	size_t result_size = pathname_size + 1 + filename_size;
	char *dst, *result = (char *)Dee_Mallocc(result_size + DeeModule_OpenFile_impl_EXTRA_CHARS + 1, sizeof(char));
	if unlikely(!result)
		goto err;
	dst = (char *)mempcpyc(result, pathname, pathname_size, sizeof(char));
	if ((dst <= result) || dst[-1] != DeeSystem_SEP)
		*dst++ = DeeSystem_SEP;
	filename_end = filename + filename_size;
	/* Skip leading whitespace */
	filename = unicode_skipspaceutf8_n(filename, filename_end);
#ifdef DeeSystem_HAVE_FS_DRIVES
	/* Check if "filename" indicates that it is relative to the current drive's root */
	if (filename < filename_end && DeeSystem_IsSep(*filename)) {
		dst = result;
		if likely(pathname_size >= 2 && dst[1] == ':')
			dst += 2;
	}
#endif /* DeeSystem_HAVE_FS_DRIVES */
	while (filename < filename_end) {
		char ch = *filename++;
		switch (ch) {
		case '.':
			if (dst[-1] == DeeSystem_SEP) {
				/* Deal with "." and ".." path references */
				char const *filename_after_space;
				filename_after_space = unicode_skipspaceutf8_n(filename, filename_end);
				if ((filename_after_space >= filename_end) || DeeSystem_IsSep(*filename_after_space)) {
					/* current-directory-reference */
					--dst;
					filename = filename_after_space;
					continue;
				} else if ((size_t)(filename_end - filename) >= 1 && *filename == '.') {
					filename_after_space = unicode_skipspaceutf8_n(filename + 1, filename_end);
					if (filename_after_space >= filename_end || DeeSystem_IsSep(*filename_after_space)) {
						/* parent-directory-reference */
#ifdef DeeSystem_HAVE_FS_DRIVES
						/* Don't go beyond the drive-prefix (the special root-of-all-drives
						 * module can only be reached using the relative-module-name syntax;
						 * iow: by doing `import("......")' ("." repeated until you'd go 1
						 * past the root of your current drive)) */
						char *min_base = result + 2;
#else /* DeeSystem_HAVE_FS_DRIVES */
						char *min_base = result;
#endif /* !DeeSystem_HAVE_FS_DRIVES */
						/* Position "dst" after the previous "/" */
						do {
							--dst;
						} while (dst > min_base && dst[-1] != DeeSystem_SEP);

						/* Position "filename" **after** the associated "/" */
						filename = filename_after_space + 1;
						filename = unicode_skipspaceutf8_n(filename, filename_end);
						continue;
					}
				}
			}
			*dst++ = '.';
			break;
#ifdef DeeSystem_ALTSEP
		case DeeSystem_ALTSEP:
			ch = DeeSystem_SEP;
			ATTR_FALLTHROUGH
#endif /* DeeSystem_ALTSEP */
		case DeeSystem_SEP:
			/* Trim already-written whitespace */
			dst = unicode_skipspaceutf8_rev_n(dst, result);
			if (dst > result && dst[-1] == DeeSystem_SEP) {
				dst = unicode_skipspaceutf8_rev_n(dst - 1, result);
				ASSERT(!(dst > result && dst[-1] == DeeSystem_SEP));
			}

			/* Skip upcoming whitespace */
			for (;;) {
				filename = unicode_skipspaceutf8_n(filename, filename_end);
				if (filename >= filename_end)
					break;
				if (!DeeSystem_IsSep(*filename))
					break;
				++filename;
			}
			ATTR_FALLTHROUGH
		default:
			*dst++ = ch;
			break;
		}
	}
	/* Strip trailing "/" and (if present) set `*p_must_be_directory = true' */
	if (dst > result && dst[-1] == DeeSystem_SEP) {
		*p_must_be_directory = true;
		--dst;
	}
	ASSERT(dst <= result || dst[-1] != DeeSystem_SEP);
	ASSERT(dst <= result || !DeeUni_IsSpace(dst[-1]));
	*dst = '\0';
	final_result_size = (size_t)(dst - result);
	if unlikely(final_result_size < result_size) {
		dst = (char *)Dee_TryReallocc(result, final_result_size + DeeModule_OpenFile_impl_EXTRA_CHARS + 1, sizeof(char));
		if likely(dst)
			result = dst;
	}
	*p_result_strlen = final_result_size;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_CreateDirectory(/*inherit(always)*/ /*utf-8*/ char *__restrict abs_dirname) {
	DREF /*tracked*/ DeeModuleObject *result;
	DeeModuleObject *existing_result;
	ASSERTF(!*abs_dirname || abs_dirname[strlen(abs_dirname) - 1] != DeeSystem_SEP,
	        "Directory names must *NOT* end with trailing slashes!");
	result = DeeModule_CreateAnonymousDirectory();
	if unlikely(!result)
		goto done;
	result->mo_absname = abs_dirname; /* Inherit */
	module_abstree_lock_write();
	existing_result = module_abstree_locate(module_abstree_root, abs_dirname);
	if unlikely(existing_result) {
		if unlikely(Dee_IncrefIfNotZero(existing_result)) {
			module_abstree_lock_endwrite();
			DeeModule_DestroyAnonymousDirectory(result);
			Dee_Free(abs_dirname);
			return existing_result;
		}
		ASSERT(existing_result->mo_absnode.rb_par != DeeModule_IMPORT_ENOENT);
		module_abstree_removenode(&module_abstree_root, existing_result);
		existing_result->mo_absnode.rb_par = DeeModule_IMPORT_ENOENT;
	}
	module_abstree_insert(&module_abstree_root, result);
	result = DeeGC_TRACK(DeeModuleObject, result);
	module_abstree_lock_endwrite();
done:
	return result;
}

PRIVATE WUNUSED DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_OpenDirectory_checked(/*inherit(always)*/ /*utf-8*/ char *__restrict abs_dirname) {
	DREF /*tracked*/ DeeModuleObject *result;
	/* Check if the specified directory has already been opened. */
	module_abstree_lock_read();
	result = module_abstree_locate(module_abstree_root, abs_dirname);
	if (result && Dee_IncrefIfNotZero(result)) {
		module_abstree_lock_endread();
		Dee_Free(abs_dirname);
		return result;
	}
	module_abstree_lock_endread();
	return do_DeeModule_CreateDirectory(abs_dirname);
}

#if DeeSystem_SEP == '\\'
#define DeeSystem_SEP_S_ESCAPED "\\\\"
#else /* DeeSystem_SEP == '\\' */
#define DeeSystem_SEP_S_ESCAPED DeeSystem_SEP_S
#endif /* DeeSystem_SEP != '\\' */


PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_module_is_not_dir(char *__restrict absname, DeeTypeObject *existing_module_type) {
	char const *extension = ".dee";
	if (existing_module_type == &DeeModuleDex_Type)
		extension = DeeSystem_SOEXT;
	return DeeError_Throwf(&DeeError_NoDirectory,
	                       "Cannot open directory module \"%#q" DeeSystem_SEP_S_ESCAPED "\": a file \"%#q%s\" exists",
	                       absname, absname, extension);
}

/* Verify that a directory exists, and open it as a cached `DeeModuleDir_Type' */
PRIVATE WUNUSED DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_OpenDirectory(/*inherit(always)*/ /*utf-8*/ char *__restrict abs_dirname,
                           size_t abs_dirname_length, unsigned int flags) {
	int status;
	DREF /*tracked*/ DeeModuleObject *result;
	ASSERT(abs_dirname[abs_dirname_length] == '\0');

	/* Check if the specified directory has already been opened. */
	module_abstree_lock_read();
	result = module_abstree_locate(module_abstree_root, abs_dirname);
	if (result && Dee_IncrefIfNotZero(result)) {
		module_abstree_lock_endread();
		Dee_Free(abs_dirname);
		if unlikely(Dee_TYPE(result) != &DeeModuleDir_Type) {
			err_module_is_not_dir(result->mo_absname, Dee_TYPE(result));
			Dee_Decref_unlikely(result);
			result = NULL;
		}
		return result;
	}
	module_abstree_lock_endread();

	/* Verify that:
	 * - posix.stat.isdir(abs_dirname)
	 * - !posix.stat.isreg(abs_dirname + ".dee")
	 * #ifndef CONFIG_NO_DEX
	 * - !posix.stat.isreg(abs_dirname + DeeSystem_SOEXT)
	 * #endif // !CONFIG_NO_DEX
	 * (Note the use of "stat" instead of "lstat": we look at symlink **targets**)
	 *
	 * If not:
	 * - return `DeeModule_IMPORT_ENOENT' if `flags & DeeModule_IMPORT_F_ENOENT'
	 * - throw `DeeError_NoDirectory' if "abs_dirname" is actually a regular file
	 * - throw `DeeError_FileNotFound' if "abs_dirname" doesn't exist at all
	 * - throw `DeeError_NoDirectory' if one of the other files exists */
	status = DeeSystem_GetFileTypeString(abs_dirname);
	if (status != DeeSystem_GetFileType_T_DIR) {
		if (status == DeeSystem_GetFileType_T_REG) {
			DeeError_Throwf(&DeeError_NoDirectory,
			                "Cannot open directory module \"%#q" DeeSystem_SEP_S_ESCAPED "\": "
			                "path before last \"" DeeSystem_SEP_S_ESCAPED "\" is a regular file",
			                abs_dirname);
		} else if (status == DeeSystem_GetFileType_T_NONE) {
			if (flags & DeeModule_IMPORT_F_ENOENT) {
				Dee_Free(abs_dirname);
				return DeeModule_IMPORT_ENOENT;
			}
			err_file_not_found_string(abs_dirname);
		} else {
			ASSERT(status == DeeSystem_GetFileType_ERR);
		}
		goto err_abs_dirname;
	}

	/* Verify that no ".dee" file exists (but if it's a directory, that's fine) */
	abs_dirname[abs_dirname_length + 0] = '.';
	abs_dirname[abs_dirname_length + 1] = 'd';
	abs_dirname[abs_dirname_length + 2] = 'e';
	abs_dirname[abs_dirname_length + 3] = 'e';
	abs_dirname[abs_dirname_length + 4] = '\0';
	status = DeeSystem_GetFileTypeString(abs_dirname);
	abs_dirname[abs_dirname_length] = '\0';
	if (status != DeeSystem_GetFileType_T_NONE &&
	    status != DeeSystem_GetFileType_T_DIR) {
		if (status != DeeSystem_GetFileType_ERR)
			err_module_is_not_dir(abs_dirname, &DeeModuleDee_Type);
		goto err_abs_dirname;
	}

	/* Verify that no ".so" file exists (but if it's a directory, that's fine) */
#ifndef CONFIG_NO_DEX
	strcpy(abs_dirname + abs_dirname_length, DeeSystem_SOEXT);
	status = DeeSystem_GetFileTypeString(abs_dirname);
	abs_dirname[abs_dirname_length] = '\0';
	if (status != DeeSystem_GetFileType_T_NONE &&
	    status != DeeSystem_GetFileType_T_DIR) {
		if (status != DeeSystem_GetFileType_ERR)
			err_module_is_not_dir(abs_dirname, &DeeModuleDex_Type);
		goto err_abs_dirname;
	}
#endif /* !CONFIG_NO_DEX */

	return do_DeeModule_OpenDirectory_checked(abs_dirname);
err_abs_dirname:
	Dee_Free(abs_dirname);
/*err:*/
	return DeeModule_IMPORT_ERROR;
}

PRIVATE WUNUSED DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_OpenFsRoot(/*inherit(always)*/ char *__restrict empty_absname) {
	ASSERT(*empty_absname == '\0');
	return do_DeeModule_OpenDirectory_checked(empty_absname);
}

PRIVATE WUNUSED NONNULL((1)) DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_OpenFile(/*utf-8*/ char const *__restrict filename, size_t filename_size, DeeStringObject *filename_ob,
                      /*utf-8*/ char const *context_absname, size_t context_absname_size, DeeStringObject *context_absname_ob,
                      unsigned int flags, struct Dee_compiler_options *options) {
	char *normal_filename;
	size_t normal_filename_size;
	DREF /*tracked*/ DeeModuleObject *result;
	bool must_be_directory;

	/* Check if the given "filename" is already absolute */
	if (DeeSystem_IsNormalAndAbsolute(filename, filename_size)) {
		char *filename_dup;
		if (filename_size && filename[filename_size - 1] == DeeSystem_SEP) {
			/* Path ends with a trailing slash -> needs to be a directory */
			--filename_size;
			ASSERT(!filename_size || filename[filename_size - 1] != DeeSystem_SEP);
			filename_dup = (char *)Dee_Mallocc(filename_size + 1, sizeof(char));
			if unlikely(!filename_dup)
				goto err;
			*(char *)mempcpyc(filename_dup, filename, filename_size, sizeof(char)) = '\0';
			return do_DeeModule_OpenDirectory(filename_dup, filename_size, flags);
		}
		if (filename_ob || (SAMEPAGE(filename, filename + filename_size) && filename[filename_size] == '\0'))
			return DeeModule_OpenFile_impl2((char *)filename, filename_size, flags | _DeeModule_IMPORT_F_NO_INHERIT_FILENAME, options);
		filename_dup = (char *)Dee_Mallocc(filename_size + DeeModule_OpenFile_impl_EXTRA_CHARS + 1, sizeof(char));
		if unlikely(!filename_dup)
			goto err;
		*(char *)mempcpyc(filename_dup, filename, filename_size, sizeof(char)) = '\0';
		return DeeModule_OpenFile_impl2(filename_dup, filename_size, flags, options);
	} else if (DeeSystem_IsAbsN(filename, filename_size)) {
		/* Don't need "context_absname" -- just need to normalize "filename" */
use_absolute_filename:
		if (filename_ob) {
			filename_ob = (DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(Dee_AsObject(filename_ob));
		} else {
			DREF DeeObject *raw_filename;
			raw_filename = DeeString_NewUtf8(filename, filename_size, STRING_ERROR_FSTRICT);
			if unlikely(!raw_filename)
				goto err;
			filename_ob = (DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(raw_filename);
			Dee_Decref_likely(raw_filename);
		}
		if unlikely(!filename_ob)
			goto err;
		filename = DeeString_AsUtf8(Dee_AsObject(filename_ob));
		if unlikely(!filename)
			goto err_filename_ob;
		result = DeeModule_OpenFile_impl2((char *)filename, WSTR_LENGTH(filename),
		                                  flags | _DeeModule_IMPORT_F_NO_INHERIT_FILENAME,
		                                  options);
		Dee_Decref_likely(filename_ob);
		return result;
	}

	if (!(flags & DeeModule_IMPORT_F_CTXDIR)) {
		/* Trim "context_absname_size" to get rid of everything after the last slash. */
		while (context_absname_size >= 1 && !DeeSystem_IsSep(context_absname[context_absname_size - 1]))
			--context_absname_size;
		while (context_absname_size >= 1 && DeeSystem_IsSep(context_absname[context_absname_size - 1]))
			--context_absname_size;
	}

	/* Check if the given "context_absname" is just referring to the current working directory... */
	if (context_absname_size == 0 || (context_absname_size == 1 && context_absname[0] == '.'))
		goto use_absolute_filename;

	/* Make sure that "context_absname" is absolute. If it isn't,
	 * then it must be made so using the current working directory. */
	if (DeeSystem_IsNormalAndAbsolute(context_absname, context_absname_size)) {
		context_absname_ob = NULL;
	} else {
		if (context_absname_ob && (flags & DeeModule_IMPORT_F_CTXDIR)) {
			context_absname_ob = (DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(Dee_AsObject(context_absname_ob));
		} else {
			DREF DeeObject *raw_context_absname;
			raw_context_absname = DeeString_NewUtf8(context_absname, context_absname_size, STRING_ERROR_FSTRICT);
			if unlikely(!raw_context_absname)
				goto err;
			context_absname_ob = (DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(raw_context_absname);
			Dee_Decref_likely(raw_context_absname);
		}
		if unlikely(!context_absname_ob)
			goto err;
		context_absname = DeeString_AsUtf8(Dee_AsObject(context_absname_ob));
		if unlikely(!context_absname)
			goto err_context_absname_ob;
		context_absname_size = WSTR_LENGTH(context_absname);
	}

	/* Concat the context path with the filename to get a normalized filename. */
	must_be_directory = false;
	normal_filename = cat_and_normalize_paths(context_absname, context_absname_size,
	                                          filename, filename_size,
	                                          &normal_filename_size,
	                                          &must_be_directory);
	if unlikely(!normal_filename)
		goto err_xcontext_absname_ob;

	/* Deal with case where the effective "normal_filename" would have ended
	 * with a trailing slash: in this case, assert that the directory exists,
	 * and open it as a `DeeModuleDir_Type' */
	if (must_be_directory)
		return do_DeeModule_OpenDirectory(normal_filename, normal_filename_size, flags);

	/* Use the normalized filename to open a module */
	result = DeeModule_OpenFile_impl2(normal_filename, normal_filename_size, flags, options);
	Dee_XDecref_likely(context_absname_ob);
	return result;
err_xcontext_absname_ob:
	if (context_absname_ob) {
err_context_absname_ob:
		Dee_Decref_likely(context_absname_ob);
	}
	goto err;
err_filename_ob:
	Dee_Decref_likely(filename_ob);
err:
	return DeeModule_IMPORT_ERROR;
}



/* Append "import_str" to "pathname", converting the "...foo.bar" to "../../foo/bar"
 * (though the returned string is normalized, meaning ".." path segments are removed) */
PRIVATE WUNUSED NONNULL((1, 4, 6, 8)) /*utf-8*/ char *DCALL
cat_and_normalize_import(/*utf-8*/ char const *__restrict pathname, size_t pathname_size, DeeStringObject *pathname_ob,
                         /*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                         size_t *__restrict p_result_strlen, unsigned int flags,
                         bool *__restrict p_must_be_directory) {
	char const *import_str_orig = import_str;
	char const *import_str_end;
	char *result, *result_end;
	size_t result_maxlen;
	ASSERT(import_str_size >= 1);

	if (!(flags & DeeModule_IMPORT_F_CTXDIR)) {
		/* Trim "pathname_size" to get rid of everything after the last slash. */
		while (pathname_size >= 1 && !DeeSystem_IsSep(pathname[pathname_size - 1]))
			--pathname_size;
		/* Special case: if the context path somehow becomes empty here, then it mustn't count as the FS root! */
		if unlikely(!pathname_size)
			goto make_absolute_path;
		while (pathname_size >= 1 && DeeSystem_IsSep(pathname[pathname_size - 1]))
			--pathname_size;
	}

	/* Check if "pathname" is a properly normalized, absolute path.
	 * Note the special case when "pathname_size == 0", which gets handled
	 * further down below and is used to form the filenames of filesystem
	 * root siblings! */
	if (DeeSystem_IsNormalAndAbsolute(pathname, pathname_size) || !pathname_size) {
		pathname_ob = NULL;
	} else {
make_absolute_path:
		if (pathname_ob && (flags & DeeModule_IMPORT_F_CTXDIR)) {
			pathname_ob = (DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(Dee_AsObject(pathname_ob));
		} else {
			DREF DeeObject *raw_pathname;
			raw_pathname = DeeString_NewUtf8(pathname, pathname_size, STRING_ERROR_FSTRICT);
			if unlikely(!raw_pathname)
				goto err;
			pathname_ob = (DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(raw_pathname);
			Dee_Decref_likely(raw_pathname);
		}
		if unlikely(!pathname_ob)
			goto err;
		pathname = DeeString_AsUtf8(Dee_AsObject(pathname_ob));
		if unlikely(!pathname)
			goto err_pathname_ob;
		pathname_size = WSTR_LENGTH(pathname);
	}

	/* Allocate a buffer for the result string. For this purpose, we know that the
	 * string can't be longer than "pathname_size + import_str_size" chars long, though
	 * "DeeModule_OpenFile_impl4_EXTRA_CHARS + DeeModule_OpenFile_impl3_EXTRA_CHARS"
	 * must also be allocated for file extensions */
	result_maxlen = pathname_size + import_str_size +
	                DeeModule_OpenFile_impl4_EXTRA_CHARS +
	                DeeModule_OpenFile_impl3_EXTRA_CHARS;
	result = (char *)Dee_Mallocc(result_maxlen + 1, sizeof(char));
	if unlikely(!result)
		goto err_pathname_ob;

	/* Write the base path */
	result_end = (char *)mempcpyc(result, pathname, pathname_size, sizeof(char));
	Dee_XDecref_likely(pathname_ob);

	/* Trim trailing slashes */
	while ((result_end > result) && result_end[-1] == DeeSystem_SEP)
		--result_end;

	/* Deal with leading parent-directory references */
	import_str_end = import_str + import_str_size;
	for (;;) {
		ASSERT(import_str < import_str_end);
		ASSERTF(!DeeSystem_IsSep(*import_str),
		        "Presence of SEPs should have caused "
		        "caller to take a different path!");
		if (*import_str != '.')
			break;

		/* Move up path by 1 element */
		++import_str;
		if (import_str >= import_str_end) {
			/* No sub-directory references
			 * -> given import string ends with "."
			 * -> Do not remove the last parent-directory reference:
			 *    in /home/me/script.dee
			 *    import(".")     ->  "/home/me/script.dee"
			 *    import(".foo")  ->  "/home/me/foo.dee"
			 *    import("..")    ->  "/home/me"            // << notice how the trailing "/me" isn't removed
			 *    import("..foo") ->  "/home/foo.dee"
			 * -> indicated module **must** be a directory */
			*p_must_be_directory = true;
			goto done_after_directory_chain;
		}

		if unlikely(result_end <= result) {
			/* Nope: that'd be one too far (the FS root itself cannot be
			 *       sub-referenced, since sub-references are always in
			 *       regards to neighbors, and the root itself can't have
			 *       neighbors since that would imply multiple roots) */
#if 0
			if (*import_str != '.')
				break;
#endif
			DeeError_Throwf(&DeeError_ValueError,
			                "Bad module path \".%#$q\" relative to %$q tries to reach beyond filesystem root",
			                (size_t)(import_str_end - import_str_orig),
			                import_str_orig, pathname_size, pathname);
			goto err_r;
		}
		while ((result_end > result) && result_end[-1] != DeeSystem_SEP)
			--result_end;
		while ((result_end > result) && result_end[-1] == DeeSystem_SEP)
			--result_end;
	}

	/* Deal with sub-directory references */
	for (;;) {
		char const *segment_end;
		size_t segment_len;
		ASSERT(import_str < import_str_end);
		ASSERTF(!DeeSystem_IsSep(*import_str),
		        "Presence of SEPs should have caused "
		        "caller to take a different path!");
		segment_end = (char const *)memchr(import_str, '.', (size_t)(import_str_end - import_str) * sizeof(char));
		if (segment_end == NULL)
			segment_end = import_str_end;
		if ((import_str != Dee_unicode_skipspaceutf8_n(import_str, segment_end)) ||
		    (segment_end != Dee_unicode_skipspaceutf8_rev_n(segment_end, import_str))) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Illegal leading/trailing whitespace in import "
			                "string segment %$q of \".%#$q\" relative to %$q",
			                (size_t)(segment_end - import_str), import_str,
			                (size_t)(import_str_end - import_str_orig),
			                import_str_orig, pathname_size, pathname);
			goto err_r;
		}
		segment_end = Dee_unicode_skipspaceutf8_rev_n(segment_end, import_str);
		segment_len = (size_t)(segment_end - import_str);
		if unlikely(segment_len == 0) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Bad module path \".%#$q\" relative to %$q contains empty path segment at %$q",
			                (size_t)(import_str_end - import_str_orig),
			                import_str_orig, pathname_size, pathname,
			                (size_t)(import_str_end - import_str), import_str);
			goto err_r;
		}
#ifdef DeeSystem_HAVE_FS_DRIVES
		if (result_end <= result) {
			/* Write new drive-string */
			if unlikely(segment_len != 1) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Length of drive name %$q is not 1 character in \".%#$q\" relative to %$q",
				                segment_len, import_str,
				                (size_t)(import_str_end - import_str_orig),
				                import_str_orig, pathname_size, pathname);
				goto err_r;
			}
			ASSERT(result_end == result);
			*result_end++ = (char)(unsigned char)toupper((unsigned int)*import_str);
			*result_end++ = ':';
		} else
#endif /* DeeSystem_HAVE_FS_DRIVES */
		{
			*result_end++ = DeeSystem_SEP;
			result_end = (char *)mempcpyc(result_end, import_str, segment_len, sizeof(char));
		}
		import_str = segment_end;
		/* Skip the discovered "." to get to the start of the next segment. */
		++import_str;
		if (import_str >= import_str_end) {
			if (import_str == import_str_end) {
				/* It is an error for the string to end with a dangling "." */
				--import_str;
				DeeError_Throwf(&DeeError_ValueError,
				                "Bad module path \".%#$q\" relative to %$q ends with trailing '.'",
				                (size_t)(import_str_end - import_str_orig),
				                import_str_orig, pathname_size, pathname);
				goto err_r;
			}
			break;
		}
	}
done_after_directory_chain:

	/* Finalize filename and calculate offsets */
	*result_end = '\0';
	result_maxlen = (size_t)(result_end - result);
	*p_result_strlen = result_maxlen;
	result_maxlen += DeeModule_OpenFile_impl4_EXTRA_CHARS;
	result_maxlen += DeeModule_OpenFile_impl3_EXTRA_CHARS;
	result_end = (char *)Dee_TryReallocc(result, result_maxlen + 1, sizeof(char));
	if likely(result_end)
		result = result_end;
	return result;
err_r:
	Dee_Free(result);
	goto err;
err_pathname_ob:
	Dee_XDecref_likely(pathname_ob);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 5)) DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_OpenGlobal_impl(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                             unsigned int flags, struct Dee_compiler_options *options,
                             DeeTupleObject *__restrict libpath) {
	size_t i;
	char *absname;
	DREF /*tracked*/ DeeModuleObject *result;
	/* Check for special case: given "libpath" is empty. */
	if unlikely(DeeTuple_IsEmpty(libpath)) {
		if (flags & DeeModule_IMPORT_F_ENOENT)
			return DeeModule_IMPORT_ENOENT;
		DeeError_Throwf(&DeeError_FileNotFound,
		                "Cannot import %$q: libpath is empty",
		                import_str_size, import_str);
		goto err;
	}

	flags |= DeeModule_IMPORT_F_CTXDIR; /* Context is LIBPATH, which are always directories */
	ASSERT(!(flags & _DeeModule_IMPORT_F_NO_INHERIT_FILENAME));
	for (i = 0;;) {
		size_t absname_len;
		unsigned int used_flags;
		DeeStringObject *path;
		char const *path_utf8;
		bool must_be_directory;
		ASSERT(i < DeeTuple_SIZE(libpath));
		path = (DeeStringObject *)DeeTuple_GET(libpath, i);
		path_utf8 = DeeString_AsUtf8(Dee_AsObject(path));
		if unlikely(!path_utf8)
			goto err;
		used_flags = flags | DeeModule_IMPORT_F_ENOENT;
		if (i >= (DeeTuple_SIZE(libpath) - 1))
			used_flags = flags; /* Last libpath uses ENOENT rules given by caller */
		must_be_directory = false;
		absname = cat_and_normalize_import(path_utf8, WSTR_LENGTH(path_utf8), path,
		                                   import_str, import_str_size, &absname_len,
		                                   used_flags, &must_be_directory);
		if unlikely(!absname)
			goto err;
		if unlikely(must_be_directory) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid import string %$q is global but ends with '.'",
			                import_str_size, import_str);
			goto err_absname;
		}
		result = DeeModule_OpenFile_impl2(absname, absname_len, flags, options);
		if (result != DeeModule_IMPORT_ENOENT)
			break;
		++i;
		if (i >= DeeTuple_SIZE(libpath))
			break;
	}
	return result;
err_absname:
	Dee_Free(absname);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_OpenGlobal(/*utf-8*/ char const *__restrict import_str, size_t import_str_size, DeeStringObject *import_str_ob,
                        unsigned int flags, struct Dee_compiler_options *options) {
	DREF /*tracked*/ DeeModuleObject *result;
	DeeTupleObject *libpath;

	/* Check if the module has already been loaded. */
	if (!(flags & DeeModule_IMPORT_F_ANONYM)) {
		struct Dee_module_libentry *libentry;
		if (!import_str_ob) {
			import_str_ob = (DeeStringObject *)DeeString_NewUtf8(import_str, import_str_size, STRING_ERROR_FSTRICT);
			if unlikely(!import_str_ob)
				goto err;
			result = do_DeeModule_OpenGlobal(import_str, import_str_size, import_str_ob, flags, options);
			Dee_Decref_unlikely(import_str_ob);
			return result;
		}
		module_libtree_lock_read();
		libentry = module_libtree_locate(module_libtree_root, import_str_ob);
		if (libentry) {
			result = Dee_module_libentry_getmodule(libentry);
			if (Dee_IncrefIfNotZero(result)) {
				module_libtree_lock_endread();
				return result;
			}
		}
		module_libtree_lock_endread();
	}

	/* Use DeeModule_GetLibPath() + cat_and_normalize_import() to find module */
	libpath = (DREF DeeTupleObject *)DeeModule_GetLibPath();
	if unlikely(!libpath)
		goto err;
	result = do_DeeModule_OpenGlobal_impl(import_str, import_str_size,
	                                      flags, options, libpath);
	Dee_Decref_unlikely(libpath);
	if (DeeModule_IMPORT_ISOK(result) && result->mo_absname != NULL && !(flags & DeeModule_IMPORT_F_ANONYM)) {
		struct Dee_module_libentry *libentry;
		/* Add "import_str" as a libname to "result" (but only if
		 * not already present, and "libpath" is still up-to-date) */
		ASSERT(import_str_ob);
		module_libtree_lock_write();
		libentry = module_libtree_locate(module_libtree_root, import_str_ob);
		if unlikely(libentry) {
			DeeModuleObject *existing_result;
			existing_result = Dee_module_libentry_getmodule(libentry);
			if (Dee_IncrefIfNotZero(existing_result)) {
				module_libtree_lock_endwrite();
				Dee_Decref_unlikely(result);
				return existing_result;
			}
			DeeModule_RemoveAllLibPathNodes_locked(existing_result);
			ASSERT(module_libtree_locate(module_libtree_root, import_str_ob) == NULL);
		}
		if likely((DeeTupleObject *)Dee_atomic_ref_getaddr(&deemon_path) == libpath) {
			if likely(result->mo_libname.mle_name == NULL) {
				/* Can use primary libname slot of "result" */
use_primary_slot:
				result->mo_libname.mle_dat.mle_mod = result;
				result->mo_libname.mle_name = import_str_ob;
				result->mo_libname.mle_next = NULL;
				Dee_Incref(import_str_ob);
				module_libtree_insert(&module_libtree_root, &result->mo_libname);
			} else {
				/* Need a secondary libname entry */
				struct Dee_module_libentry *next;
				next = Dee_module_libentry_tryalloc();
				if unlikely(!next) {
					/* Complicated case: must allocate new libname entry without holding libtree lock */
					module_libtree_lock_endwrite();
					next = Dee_module_libentry_alloc();
					if unlikely(!next)
						goto err_r;
					module_libtree_lock_write();
					libentry = module_libtree_locate(module_libtree_root, import_str_ob);
					if unlikely(libentry) {
						DeeModuleObject *existing_result;
						existing_result = Dee_module_libentry_getmodule(libentry);
						if (Dee_IncrefIfNotZero(existing_result)) {
							module_libtree_lock_endwrite();
							Dee_Decref_unlikely(result);
							Dee_module_libentry_free(next);
							return existing_result;
						}
						DeeModule_RemoveAllLibPathNodes_locked(existing_result);
						ASSERT(module_libtree_locate(module_libtree_root, import_str_ob) == NULL);
					}
					if unlikely((DeeTupleObject *)Dee_atomic_ref_getaddr(&deemon_path) != libpath) {
						module_libtree_lock_endwrite();
						Dee_module_libentry_free(next);
						return result;
					}
					if unlikely(result->mo_libname.mle_name == NULL) {
						Dee_module_libentry_free(next);
						goto use_primary_slot;
					}
				}

				/* Insert secondary libname entry into tree. */
				next->mle_name = import_str_ob;
				Dee_Incref(import_str_ob);
				next->mle_dat.mle_mod = result;
				next->mle_next = result->mo_libname.mle_next;
				result->mo_libname.mle_next = next;
				module_libtree_insert(&module_libtree_root, next);
			}
		}
		module_libtree_lock_endwrite();
	}
	return result;
err_r:
	Dee_Decref_unlikely(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF /*tracked*/ DeeModuleObject *DCALL
do_DeeModule_OpenEx(/*utf-8*/ char const *__restrict import_str, size_t import_str_size, DeeStringObject *import_str_ob,
                    /*utf-8*/ char const *context_absname, size_t context_absname_size, DeeStringObject *context_absname_ob,
                    unsigned int flags, struct Dee_compiler_options *options) {
	DREF /*tracked*/ DeeModuleObject *result;

	/* Check if "import_str" needs to be interpreted as
	 * a system filename, rather than a module path. */
	if ((flags & DeeModule_IMPORT_F_FILNAM) ||
	    memchr(import_str, DeeSystem_SEP, import_str_size)
#ifdef DeeSystem_ALTSEP
	    || memchr(import_str, DeeSystem_ALTSEP, import_str_size)
#endif /* DeeSystem_ALTSEP */
	    ) {
		return do_DeeModule_OpenFile(import_str, import_str_size, import_str_ob,
		                             context_absname, context_absname_size, context_absname_ob,
		                             flags, options);
	}

	/* Make sure that the import string isn't an empty string */
	if unlikely(import_str_size < 1) {
		DeeError_Throwf(&DeeError_ValueError, "An empty string is not a valid module name");
		goto err;
	}

	/* Check if "import_str" is a relative import-string (iow: starts with a leading `.') */
	if (import_str[0] == '.') {
		size_t filename_size;
		char *filename;
		bool must_be_directory;
		/* Check for special case: `import_str == "."'
		 * --> return the module described by "context_absname" */
		if (import_str_size == 1) {
			if (flags & DeeModule_IMPORT_F_CTXDIR) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Cannot determine module \".\" from context directory %$q alone",
				                context_absname_size, context_absname);
				goto err;
			}
			return do_DeeModule_OpenFile(context_absname, context_absname_size, context_absname_ob,
			                             NULL, 0, NULL, flags, options);
		}

		if (!(flags & DeeModule_IMPORT_F_CTXDIR)) {
			if unlikely(!context_absname_size) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Cannot import sibling %$q of filesystem root. Root "
				                "has no siblings (did you mean to import children?)",
				                import_str_size, import_str);
				goto err;
			}
			/* Trim "context_absname_size" to get rid of everything after the last slash. */
			while (context_absname_size >= 1 && !DeeSystem_IsSep(context_absname[context_absname_size - 1]))
				--context_absname_size;
			/* Special case: if the context path somehow becomes empty here, then it mustn't count as the FS root! */
			if likely(context_absname_size)
				flags |= DeeModule_IMPORT_F_CTXDIR;
			while (context_absname_size >= 1 && DeeSystem_IsSep(context_absname[context_absname_size - 1]))
				--context_absname_size;
		}
		++import_str; /* Skip leading "." */
		--import_str_size;
		must_be_directory = false;
		filename = cat_and_normalize_import(context_absname, context_absname_size, context_absname_ob,
		                                    import_str, import_str_size, &filename_size,
		                                    flags, &must_be_directory);
		if unlikely(!filename)
			goto err;
		if (must_be_directory) {
			/* Special handling for directory-module describing the filesystem root. */
			if unlikely(filename_size == 0)
				return do_DeeModule_OpenFsRoot(filename);
#if 0 /* In relative notation, ".." is allowed to open "../../{dirname}.dee"
       * in addition to "../." == "../../{dirname}", so just fallthu to the
       * regular code-path below. */
			return do_DeeModule_OpenDirectory(filename, filename_size, flags);
#endif
		}
		ASSERTF(filename_size, "An empty filename should have caused 'must_be_directory == true'");
		return DeeModule_OpenFile_impl2(filename, filename_size, flags, options);
	}

	/* Special case: "import_str" is the string "deemon" */
	if (import_str_size == 6 && fs_bcmp(import_str, "deemon", 6 * sizeof(char)) == 0) {
		(void)options;
		(void)flags;
		(void)context_absname;
		(void)context_absname_size;
		(void)context_absname_ob;
		result = DeeModule_GetDeemon();
		return_reference_(result);
	}

	/* Final case: "import_str" refers to a LIBPATH module */
	return do_DeeModule_OpenGlobal(import_str, import_str_size, import_str_ob, flags, options);
err:
	return DeeModule_IMPORT_ERROR;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenEx(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                 /*utf-8*/ char const *context_absname, size_t context_absname_size,
                 unsigned int flags, struct Dee_compiler_options *options) {
	ASSERTF(!(flags & _DeeModule_IMPORT_F_MASK), "Internal flags may not be set by API");
	return do_DeeModule_OpenEx(import_str, import_str_size, NULL,
	                           context_absname, context_absname_size, NULL,
	                           flags, options);
}

/* Open a child of a specific module:
 * >> rt      = DeeModule_Open("rt", NULL, DeeModule_IMPORT_F_NORMAL);
 * >> rt_hash = DeeModule_OpenChild(rt, "hash", DeeModule_IMPORT_F_NORMAL);
 * Same as:
 * >> rt_hash = DeeModule_Open("rt.hash", NULL, DeeModule_IMPORT_F_NORMAL);
 *
 * NOTE: These functions ignore the `DeeModule_IMPORT_F_CTXDIR' flag! */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_OpenChild(DeeModuleObject *self,
                    /*String*/ DeeObject *name,
                    unsigned int flags) {
	char const *name_utf8;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	name_utf8 = DeeString_AsUtf8(name);
	if unlikely(!name_utf8)
		goto err;
	return DeeModule_OpenChildEx(self, name_utf8, WSTR_LENGTH(name_utf8), flags, NULL);
err:
	return DeeModule_IMPORT_ERROR;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_OpenChildEx(DeeModuleObject *self,
                      /*utf-8*/ char const *__restrict name, size_t name_size,
                      unsigned int flags, struct Dee_compiler_options *options) {
	char *child_absname, *dst;
	char const *context_absname;
	size_t child_absname_length, context_absname_length;
	DREF DeeModuleObject *result;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	ASSERTF(!(flags & _DeeModule_IMPORT_F_MASK), "Internal flags may not be set by API");
	context_absname = self->mo_absname;
	if unlikely(!context_absname)
		goto err_no_child;
	if (self->mo_flags & Dee_MODULE_FABSFILE) {
		if unlikely(Dee_TYPE(self) != &DeeModuleDir_Type)
			goto err_no_child;
	}
	context_absname_length = strlen(context_absname);
	child_absname_length = context_absname_length + 1 + name_size;
	child_absname = (char *)Dee_Mallocc(child_absname_length +
	                                    DeeModule_OpenFile_impl4_EXTRA_CHARS +
	                                    DeeModule_OpenFile_impl3_EXTRA_CHARS + 1,
	                                    sizeof(char));
	if unlikely(!child_absname)
		goto err;
#ifdef DeeSystem_HAVE_FS_DRIVES
	if (context_absname_length == 0) {
		if unlikely(name_size != 1) {
			Dee_Free(child_absname);
			DeeError_Throwf(&DeeError_ValueError,
			                "Length of drive name %$q is not 1 character",
			                name_size, name);
			goto err;
		}
		dst = child_absname;
		*dst++ = (char)(unsigned char)toupper((unsigned int)*name);
		*dst++ = ':';
	} else
#endif /* DeeSystem_HAVE_FS_DRIVES */
	{
		dst = (char *)mempcpyc(child_absname, context_absname, context_absname_length, sizeof(char));
		*dst++ = DeeSystem_SEP;
		dst = (char *)mempcpyc(dst, name, name_size, sizeof(char));
	}
	*dst = '\0';
	ASSERT((size_t)(dst - child_absname) == child_absname_length);
#if 0
	/* TODO: Same as "DeeModule_OpenFile_impl2()", but without the
	 *       `.endswith(".so")'-part or `.endswith(".dee")'-part,
	 *       and all "_DeeModule_IMPORT_F_IS_DEE_FILE"-related
	 *       handling omitted */
#else
	result = DeeModule_OpenFile_impl2(child_absname, child_absname_length, flags, options);
#endif
	return result;
err_no_child:
	if (flags & DeeModule_IMPORT_F_ENOENT)
		return DeeModule_IMPORT_ENOENT;
	DeeError_Throwf(&DeeError_FileNotFound, "%k has no child %$q", self, name_size, name);
err:
	return DeeModule_IMPORT_ERROR;
}


/* Open a module, given an import string, and another module/path used
 * to resolve relative paths. The given "import_str" can take any of the
 * following forms (assuming that `DeeSystem_SEP' is '/'):
 * - [m] "deemon"                    (DeeModule_GetDeemon())
 * - [m] "net.ftp"                   ("${LIBPATH}/net/ftp.dee")
 * - [m] "net"                       ("${LIBPATH}/net.so")
 * - [m] ".some_module"              (fs.headof(context_absname) + "/some_module.dee")
 * - [m] ".subdir.that_module"       (fs.headof(context_absname) + "/subdir/that_module.dee")
 * - [m] ".subdir"                   (fs.headof(context_absname) + "/subdir")                  DeeModuleDir_Type
 * - [m] "..pardir.subdir"           (fs.headof(context_absname) + "/../pardir/subdir")        DeeModuleDir_Type
 * - [m] "."                         (DeeModule_Open(fs.abspath(context_absname), NULL))       Only allowed if "context_absname" doesn't end with a trailing "/"
 * - [m] "./subdir/that_module"      (fs.headof(context_absname) + "/subdir/that_module.dee")
 * - [f] "./subdir/that_module.dee"  (fs.headof(context_absname) + "/subdir/that_module.dee")
 * - [f] "/opt/deemon/file.dee"      ("/opt/deemon/file.dee")
 * - [m] "/opt/deemon/file"          ("/opt/deemon/file.dee")
 * - [f] "/opt/deemon"               ("/opt/deemon")                                           DeeModuleDir_Type
 *
 * NOTE: When `DeeModule_IMPORT_F_FILNAM' is given, **ONLY** examples
 *       marked as [f] can be loaded (that is: "import_str" is treated
 *       as a native filename (**WITH** extension), rather than the
 *       usual combination of module-name/filename).
 *
 * The given "context_absname" should be the mo_absname-style name of
 * the calling file, or (at the very least) be a string ending with a
 * trailing `DeeSystem_SEP' (in this case, import_str="." will throw
 * an error). When this string isn't actually absolute, it will be
 * made absolute using `DeeSystem_MakeNormalAndAbsolute()'. When it is NULL or
 * an empty string, `DeeSystem_PrintPwd()' is used instead.
 *
 * @return: * :                      The newly opened module
 * @return: DeeModule_IMPORT_ERROR:  An error was thrown
 * @return: DeeModule_IMPORT_ENOENT: `DeeModule_IMPORT_F_ENOENT' was set, and no such file exists
 * @return: DeeModule_IMPORT_ERECUR: `DeeModule_IMPORT_F_ERECUR' was set, and module is already being imported */
PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
do_DeeModule_OpenString(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                        DeeStringObject *import_str_ob,
                        /*Module|String|Type|None*/ DeeObject *context_absname,
                        unsigned int flags) {
	char const *context_absname_utf8;
	size_t context_absname_size;
	DeeStringObject *context_absname_ob = NULL;
	if (context_absname == NULL || DeeNone_Check(context_absname)) {
no_context:
		context_absname_utf8 = NULL;
		context_absname_size = 0;
	} else if (DeeModule_Check(context_absname)) {
		DeeModuleObject *context_mod = (DeeModuleObject *)context_absname;
		/* Special case optimization: `import(".")' -> simply re-return the context module. */
		if (import_str_size == 1 && import_str[0] == '.')
			return_reference_(context_mod);
		context_absname_utf8 = context_mod->mo_absname;
		context_absname_size = context_absname_utf8 ? strlen(context_absname_utf8) : 0;
	} else if (DeeString_Check(context_absname)) {
		context_absname_ob = (DeeStringObject *)context_absname;
		context_absname_utf8 = DeeString_AsUtf8(context_absname);
		if unlikely(!context_absname_utf8)
			goto err;
		context_absname_size = WSTR_LENGTH(context_absname_utf8);
	} else if (DeeType_Check(context_absname)) {
		DREF DeeModuleObject *result;
		context_absname = Dee_AsObject(DeeType_GetModule((DeeTypeObject *)context_absname));
		if (!context_absname)
			goto no_context;
		result = do_DeeModule_OpenString(import_str, import_str_size,
		                                 import_str_ob, context_absname,
		                                 flags);
		Dee_Decref_unlikely(context_absname);
		return result;
	} else {
		DeeObject_TypeAssertFailed3(context_absname, &DeeModule_Type, &DeeString_Type, &DeeType_Type);
		goto err;
	}
	return do_DeeModule_OpenEx(import_str, import_str_size, import_str_ob,
	                           context_absname_utf8, context_absname_size,
	                           context_absname_ob, flags, NULL);
err:
	return DeeModule_IMPORT_ERROR;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_Open(/*String*/ DeeObject *__restrict import_str,
               /*Module|String|Type|None*/ DeeObject *context_absname,
               unsigned int flags) {
	char const *import_str_utf8 = DeeString_AsUtf8(import_str);
	if unlikely(!import_str_utf8)
		goto err;
	return do_DeeModule_OpenString(import_str_utf8, WSTR_LENGTH(import_str_utf8),
	                               (DeeStringObject *)import_str, context_absname,
	                               flags);
err:
	return DeeModule_IMPORT_ERROR;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenString(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                     /*Module|String|Type|None*/ DeeObject *context_absname, unsigned int flags) {
	return do_DeeModule_OpenString(import_str, import_str_size, NULL,
	                               context_absname, flags);
}




/* Import (DeeModule_Open() + DeeModule_Initialize()) a specific module */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_Import(/*String*/ DeeObject *__restrict import_str,
                 /*Module|String|Type|None*/ DeeObject *context_absname,
                 unsigned int flags) {
	DREF DeeModuleObject *result = DeeModule_Open(import_str, context_absname, flags);
	if (likely(DeeModule_IMPORT_ISOK(result)) && unlikely(DeeModule_Initialize(result) < 0))
		Dee_Clear(result);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_ImportString(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                       /*Module|String|Type|None*/ DeeObject *context_absname, unsigned int flags) {
	DREF DeeModuleObject *result = DeeModule_OpenString(import_str, import_str_size, context_absname, flags);
	if (likely(DeeModule_IMPORT_ISOK(result)) && unlikely(DeeModule_Initialize(result) < 0))
		Dee_Clear(result);
	return result;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_ImportEx(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                   /*utf-8*/ char const *context_absname, size_t context_absname_size,
                   unsigned int flags, struct Dee_compiler_options *options) {
	DREF DeeModuleObject *result;
	result = DeeModule_OpenEx(import_str, import_str_size, context_absname,
	                          context_absname_size, flags, options);
	if (likely(DeeModule_IMPORT_ISOK(result)) && unlikely(DeeModule_Initialize(result) < 0))
		Dee_Clear(result);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_ImportChild(DeeModuleObject *self,
                      /*String*/ DeeObject *name,
                      unsigned int flags) {
	DREF DeeModuleObject *result = DeeModule_OpenChild(self, name, flags);
	if (likely(DeeModule_IMPORT_ISOK(result)) && unlikely(DeeModule_Initialize(result) < 0))
		Dee_Clear(result);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_ImportChildEx(DeeModuleObject *self,
                        /*utf-8*/ char const *__restrict name, size_t name_size,
                        unsigned int flags, struct Dee_compiler_options *options) {
	DREF DeeModuleObject *result = DeeModule_OpenChildEx(self, name, name_size, flags, options);
	if (likely(DeeModule_IMPORT_ISOK(result)) && unlikely(DeeModule_Initialize(result) < 0))
		Dee_Clear(result);
	return result;
}






/************************************************************************/
/************************************************************************/
/*                                                                      */
/* Helper APIs for "DeeBuiltin_ImportType"                              */
/*                                                                      */
/************************************************************************/
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
do_DeeModule_ImportGlobal(/*utf-8*/ char const *__restrict import_str,
                          size_t import_str_size,
                          DeeStringObject *import_str_ob) {
	DREF /*tracked*/ DeeModuleObject *result;
	result = do_DeeModule_OpenGlobal(import_str, import_str_size, import_str_ob, DeeModule_IMPORT_F_NORMAL, NULL);
	if likely(result) {
		if (DeeModule_Initialize(result) < 0)
			Dee_Clear(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
do_import_getattr_string_len(char const *__restrict attr, size_t attrlen) {
	/* Special case: "import_str" is the string "deemon" */
	if (attrlen == 6 && fs_bcmp(attr, "deemon", 6 * sizeof(char)) == 0)
		return_reference_(DeeModule_GetDeemon());
	return do_DeeModule_ImportGlobal(attr, attrlen, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
do_import_getattr_string(char const *__restrict attr) {
	/* Special case: "import_str" is the string "deemon" */
	if (fs_strcmp(attr, "deemon") == 0)
		return_reference_(DeeModule_GetDeemon());
	return do_DeeModule_ImportGlobal(attr, strlen(attr), NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
do_import_getattr(DeeObject *__restrict attr) {
	char const *utf8 = DeeString_AsUtf8(attr);
	if unlikely(!utf8)
		goto err;
	if (WSTR_LENGTH(utf8) == 6 && fs_bcmp(utf8, "deemon", 6 * sizeof(char)) == 0)
		return_reference_(DeeModule_GetDeemon());
	return do_DeeModule_ImportGlobal(utf8, WSTR_LENGTH(utf8), (DeeStringObject *)attr);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
import_getattr_string_len_hash(DeeObject *self, char const *attr,
                               size_t attrlen, Dee_hash_t UNUSED(hash)) {
	DREF DeeModuleObject *result;
	result = do_import_getattr_string_len(attr, attrlen);
	if unlikely(!result) {
		DREF DeeObject *fnf;
		if ((fnf = DeeError_CatchError(&DeeError_FileNotFound)) != NULL)
			DeeRT_ErrUnknownAttrStrLenWithCause(self, attr, attrlen, DeeRT_ATTRIBUTE_ACCESS_GET, fnf);
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
import_getattr_string_hash(DeeObject *self, char const *attr, Dee_hash_t UNUSED(hash)) {
	DREF DeeModuleObject *result;
	result = do_import_getattr_string(attr);
	if unlikely(!result) {
		DREF DeeObject *fnf;
		if ((fnf = DeeError_CatchError(&DeeError_FileNotFound)) != NULL)
			DeeRT_ErrUnknownAttrStrWithCause(self, attr, DeeRT_ATTRIBUTE_ACCESS_GET, fnf);
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
import_getattr(DeeObject *self, DeeObject *attr) {
	DREF DeeModuleObject *result;
	result = do_import_getattr(attr);
	if unlikely(!result) {
		DREF DeeObject *fnf;
		if ((fnf = DeeError_CatchError(&DeeError_FileNotFound)) != NULL)
			DeeRT_ErrUnknownAttrWithCause(self, attr, DeeRT_ATTRIBUTE_ACCESS_GET, fnf);
	}
	return result;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
do_DeeModule_BoundGlobal(/*utf-8*/ char const *__restrict import_str,
                         size_t import_str_size,
                         DeeStringObject *import_str_ob) {
	DREF /*tracked*/ DeeModuleObject *result;
	result = do_DeeModule_OpenGlobal(import_str, import_str_size, import_str_ob,
	                                 DeeModule_IMPORT_F_NORMAL |
	                                 DeeModule_IMPORT_F_ENOENT |
	                                 DeeModule_IMPORT_F_ERECUR, NULL);
	if (DeeModule_IMPORT_ISOK(result)) {
		Dee_Decref(result);
		return Dee_BOUND_YES;
	}
	if (result == DeeModule_IMPORT_ENOENT)
		return Dee_BOUND_MISSING;
	if (result == DeeModule_IMPORT_ERECUR)
		return Dee_BOUND_NO; /* Present, but not yet bound (because currently being initialized in calling thread) */
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_import_boundattr_string_len(char const *__restrict attr, size_t attrlen) {
	/* Special case: "import_str" is the string "deemon" */
	if (attrlen == 6 && fs_bcmp(attr, "deemon", 6 * sizeof(char)) == 0)
		return Dee_BOUND_YES;
	return do_DeeModule_BoundGlobal(attr, attrlen, NULL);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_import_boundattr_string(char const *__restrict attr) {
	/* Special case: "import_str" is the string "deemon" */
	if (fs_strcmp(attr, "deemon") == 0)
		return Dee_BOUND_YES;
	return do_DeeModule_BoundGlobal(attr, strlen(attr), NULL);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_import_boundattr(DeeObject *__restrict attr) {
	char const *utf8 = DeeString_AsUtf8(attr);
	if unlikely(!utf8)
		goto err;
	if (WSTR_LENGTH(utf8) == 6 && fs_bcmp(utf8, "deemon", 6 * sizeof(char)) == 0)
		return Dee_BOUND_YES;
	return do_DeeModule_BoundGlobal(utf8, WSTR_LENGTH(utf8), (DeeStringObject *)attr);
err:
	return Dee_BOUND_ERR;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
import_boundattr_string_len_hash(DeeObject *UNUSED(self), char const *attr,
                                 size_t attrlen, Dee_hash_t UNUSED(hash)) {
	return do_import_boundattr_string_len(attr, attrlen);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
import_boundattr_string_hash(DeeObject *UNUSED(self), char const *attr, Dee_hash_t UNUSED(hash)) {
	return do_import_boundattr_string(attr);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
import_boundattr(DeeObject *UNUSED(self), DeeObject *attr) {
	return do_import_boundattr(attr);
}








/************************************************************************/
/************************************************************************/
/*                                                                      */
/* Module name APIs                                                     */
/*                                                                      */
/************************************************************************/
/************************************************************************/

#ifndef CONFIG_HAVE_strrchr
#define CONFIG_HAVE_strrchr
#undef strrchr
#define strrchr dee_strrchr
DeeSystem_DEFINE_strrchr(dee_strrchr)
#endif /* !CONFIG_HAVE_strrchr */

/* Return the module's human-readable "short" name, that is everything after
 * the last '/' (or '\') within `DeeModule_GetAbsName()', or the string
 * "<anonymous module>" if `DeeModule_GetAbsName() == NULL' */
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GetShortName(DeeModuleObject *__restrict self) {
	char const *result = self->mo_absname;
	if (result) {
		result = strrchr(result, DeeSystem_SEP);
		if (result) /* Should always be non-NULL at this point! */
			++result;
	} else if (self == &DeeModule_Deemon) {
		result = STR_deemon;
	}
	return result;
}

/* Return the absolute, normalized filename that the module was loaded from,
 * `ITER_DONE' if `DeeModule_GetAbsName() == NULL'. This function combines
 * the module's 'Dee_MODULE_FABSFILE' flag together with its typing in order
 * to reconstruct the original filename that the module was loaded from.
 *
 * Examples:
 * - E:\projects\deemon\lib\rt.dll       (after opening 'E:\projects\deemon\lib\rt.dll')
 * - /opt/deemon/lib/net.so              (after opening '/opt/deemon/lib/net.so')
 * - /home/me/projects/deemon/script.dee (after opening '/home/me/projects/deemon/script.dee')
 * - /home/me/projects/readme.txt        (after opening '/home/me/projects/readme.txt' with `DeeModule_IMPORT_F_FILNAM')
 *
 * @return: * : The module's original, absolute, normalized source filename.
 * @return: ITER_DONE: Module is anonymous and doesn't have a source filename.
 * @return: NULL:      An error was thrown. */
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetFileName(DeeModuleObject *__restrict self) {
	DREF DeeStringObject *result;
	char const *extension, *absname = self->mo_absname;
	size_t extension_len, abslen;
	char *dst;
	if (!absname)
		return ITER_DONE;
	abslen = strlen(absname);
	if ((self->mo_flags & Dee_MODULE_FABSFILE) || Dee_TYPE(self) == &DeeModuleDir_Type)
		return DeeString_NewUtf8(absname, abslen, STRING_ERROR_FIGNORE);
#ifndef CONFIG_NO_DEX
	if (Dee_TYPE(self) == &DeeModuleDex_Type) {
		extension = DeeSystem_SOEXT;
		extension_len = COMPILER_STRLEN(DeeSystem_SOEXT);
	} else
#endif /* !CONFIG_NO_DEX */
	{
		extension = ".dee";
		extension_len = 4;
	}
	result = (DREF DeeStringObject *)DeeString_NewBuffer(abslen + extension_len);
	if unlikely(!result)
		goto err;
	dst = DeeString_GetBuffer(result);
	dst = (char *)mempcpyc(dst, absname, abslen, sizeof(char));
	memcpyc(dst, extension, extension_len, sizeof(char));
	return DeeString_SetUtf8(Dee_AsObject(result), STRING_ERROR_FIGNORE);
err:
	return NULL;
}


/* Same as `DeeModule_GetRelName()', but allows you to specify the context
 * path in the same manner as can be specified by `DeeModule_OpenEx()':
 * - When `DeeModule_RELNAME_F_CTXDIR' is not given, "context_absname"
 *   should be the mo_absname-style name of the calling file, or (at
 *   the very least) be a string ending with a trailing `DeeSystem_SEP'.
 * - When `DeeModule_RELNAME_F_CTXDIR' is given, "context_absname" is
 *   treated as the directory relative to which the returned path will
 *   be printed.
 * - When `self' is anonymous or cannot be opened without the use of
 *   the `DeeModule_IMPORT_F_FILNAM' flag, then `ITER_DONE' is returned.
 * - When the last part of the module name (after the last '/') contains
 *   a '.' (e.g. '/home/me/projects/foo/script.v1.dee'), then `ITER_DONE'
 *   is also returned, since no relative module name can be formed. The
 *   same also happens when any part of the path that would appear within
 *   the relative module path contains a '.'.
 * - When this string isn't actually absolute, it will be made absolute
 *   using `DeeSystem_MakeNormalAndAbsolute()'. When it is NULL or an
 *   empty string, `DeeSystem_PrintPwd()' is used instead.
 *
 * Examples:
 * - .rt           (self='E:\projects\deemon\lib\rt.dll', context_absname='E:\projects\deemon\lib\doc.dee' + DeeModule_RELNAME_F_NORMAL)
 * - .rt           (self='E:\projects\deemon\lib\rt.dll', context_absname='E:\projects\deemon\lib' + DeeModule_RELNAME_F_CTXDIR)
 * - rt            (self='E:\projects\deemon\lib\rt.dll', context_absname='<ignored>' + DeeModule_RELNAME_F_LIBNAM)
 * - ..foo.script  (self='/home/me/projects/foo/script.dee, context_absname='/home/me/projects/bar/script.dee' + DeeModule_RELNAME_F_NORMAL)
 * - ITER_DONE     (self='/home/me/projects/readme.txt', context_absname='<ignored>' + <ignored>))
 *
 * @param: flags: Set of `DeeModule_RELNAME_F_*'
 * @return: * :        The module's name, written relative to `context_absname'
 * @return: ITER_DONE: The given module is anonymous or has its `Dee_MODULE_FABSFILE' flag set
 * @return: NULL:      An error was thrown. */

PRIVATE ATTR_PURE WUNUSED size_t DCALL
count_fs_seps(char const *iter, char const *end) {
	size_t result = 0;
	while (iter < end) {
		char ch = *iter++;
		if (ch == DeeSystem_SEP)
			++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
do_DeeModule_PrintRelNameEx_impl2(DeeModuleObject *__restrict self,
                                  Dee_formatprinter_t printer, void *arg,
                                  /*utf-8*/ char const *context_absname,
                                  size_t context_absname_size,
                                  unsigned int flags) {
	Dee_ssize_t result, temp;
	size_t num_dots;
	char const *module_absname = self->mo_absname;
	char const *context_absname_end = context_absname + context_absname_size;
	char const *used_module_absname = module_absname;
	char const *used_context_absname = context_absname;
	char const *orig_context_absname = context_absname;
	ASSERT(module_absname);

	/* Skip common prefix between "module_absname" and "context_absname".
	 * If the filesystem is case-insensitive, then we have to be, too. */
	for (;;) {
		uint32_t mod_char, ctx_char;
		if (*module_absname == '\0') {
			if (context_absname >= context_absname_end) {
				if (!(flags & DeeModule_RELNAME_F_CTXDIR))
					return (*printer)(arg, ".", 1); /* Special case: the module itself */
				break;
			}
			mod_char = 0;
			if (context_absname <= orig_context_absname) {
				if (*context_absname == DeeSystem_SEP)
					++context_absname;
do_handle_parent_directory:
				/* Special case: "module_absname" is a parent directory of "context_absname" */
				num_dots = count_fs_seps(context_absname, context_absname_end) + 2;
				if (flags & DeeModule_RELNAME_F_CTXDIR)
					++num_dots;
				return DeeFormat_Repeat(printer, arg, '.', num_dots);
			}
			ctx_char = unicode_readutf8_n(&context_absname, context_absname_end);
			if (ctx_char == DeeSystem_SEP)
				goto do_handle_parent_directory;
		} else {
			mod_char = unicode_readutf8(&module_absname);
			ctx_char = unicode_readutf8_n(&context_absname, context_absname_end);
		}
		if (mod_char != ctx_char) {
#ifdef DeeSystem_HAVE_FS_ICASE
			mod_char = DeeUni_ToLower(mod_char);
			ctx_char = DeeUni_ToLower(ctx_char);
			if (mod_char == ctx_char) {
				/* Keep going! */
			} else
#endif /* DeeSystem_HAVE_FS_ICASE */
			{
				break;
			}
		}
		if (mod_char == DeeSystem_SEP) {
			used_module_absname  = module_absname;
			used_context_absname = context_absname;
		}
	}

	/* Unless the 'DeeModule_RELNAME_F_CTXDIR' flag is given, `context_absname'
	 * actually refers to some file **within** the relevant context directory. */
	if (!(flags & DeeModule_RELNAME_F_CTXDIR)) {
		while (context_absname_end > used_context_absname &&
		       context_absname_end[-1] != DeeSystem_SEP)
			--context_absname_end;
		while (context_absname_end > used_context_absname &&
		       context_absname_end[-1] == DeeSystem_SEP)
			--context_absname_end;
	}

	module_absname  = used_module_absname;
	context_absname = used_context_absname;
	context_absname_size = (size_t)(context_absname_end - context_absname);

	/* At this point, "module_absname" and "context_absname" are relative paths
	 * that both original from the same, common-ancestor directory. On windows,
	 * there is still the special case where they might reside on different drives,
	 * though for this purpose, we still allow relative references to different
	 * drives by defining a (virtual) folder above (e.g.) "C:\\" that is reachable
	 * by adding another ".."-reference whilst already at the root of a drive:
	 * - self:            "C:\\script[.dee]"
	 * - context_absname: "D:\\other_script[.dee]"
	 *
	 * Result: "..c.script" */

	/* Check the remainder of 'module_absname' (after all common prefixes
	 * have been removed) for still containing any '.' character. If it
	 * does, then we have to return '0' without printing anything, since
	 * this means that some segment of the relative path would need to
	 * contain a '.' character as its segment name, which cannot appear
	 * in the relative module name format that we're supposed to generate */
	if (strchr(module_absname, '.'))
		return 0;

	/* To start out with, print "." 1+ the number of "/" in 'context_absname'
	 * But: Must always print 1 extra, leading dot so the module name becomes
	 *      relative */
	num_dots = count_fs_seps(context_absname, context_absname_end) + 1;
	if (context_absname < context_absname_end)
		++num_dots;
	result = DeeFormat_Repeat(printer, arg, '.', num_dots);
	if unlikely(result < 0)
		goto done;

#ifdef DeeSystem_HAVE_FS_DRIVES
	/* Special handling when `module_absname' starts with a drive prefix */
	if (DeeSystem_IsAbs(module_absname)) {
		char drive_char = (char)tolower((unsigned char)module_absname[0]);
		module_absname += 2; /* "C:" (such that "module_absname" now starts with '\') */
		ASSERT(module_absname[0] == DeeSystem_SEP || !*module_absname);
		temp = (*printer)(arg, &drive_char, 1);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
#endif /* !DeeSystem_HAVE_FS_DRIVES */

	/* Now print the entirety of `module_absname', but replace '/' with '.' */
	while (*module_absname) {
		char const *chunk_end = module_absname;
		while (*chunk_end && *chunk_end != DeeSystem_SEP)
			++chunk_end;
		temp = (*printer)(arg, module_absname, (size_t)(chunk_end - module_absname));
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		module_absname = chunk_end;
		if (!*module_absname)
			break;
		++module_absname; /* Skip the slash */
		ASSERT(*module_absname != DeeSystem_SEP);
		ASSERT(*module_absname);

		/* Now print the "." used to replace the '/' */
		temp = (*printer)(arg, ".", 1);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
done:
	return result;
err_temp:
	return temp;
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
do_DeeModule_PrintRelNameEx_impl(DeeModuleObject *__restrict self,
                                 Dee_formatprinter_t printer, void *arg,
                                 /*utf-8*/ char const *context_absname,
                                 size_t context_absname_size,
                                 DeeStringObject *context_absname_ob,
                                 unsigned int flags) {
	Dee_ssize_t result;
	if (DeeSystem_IsNormalAndAbsolute(context_absname, context_absname_size) || !context_absname_size) {
		return do_DeeModule_PrintRelNameEx_impl2(self, printer, arg, context_absname,
		                                         context_absname_size, flags);
	}
	if (context_absname_ob) {
		context_absname_ob = (DREF DeeStringObject *)DeeSystem_MakeNormalAndAbsolute((DeeObject *)context_absname_ob);
	} else {
		DREF DeeObject *temp_string;
		temp_string = DeeString_NewUtf8(context_absname,
		                                context_absname_size,
		                                STRING_ERROR_FSTRICT);
		if unlikely(!temp_string)
			goto err;
		context_absname_ob = (DREF DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(temp_string);
		Dee_Decref_likely(temp_string);
	}
	if unlikely(!context_absname_ob)
		goto err;
	context_absname = DeeString_AsUtf8((DeeObject *)context_absname_ob);
	if unlikely(!context_absname)
		goto err_context_absname_ob;
	context_absname_size = WSTR_LENGTH(context_absname);
	result = do_DeeModule_PrintRelNameEx_impl2(self, printer, arg, context_absname,
	                                           context_absname_size, flags);
	Dee_Decref_likely(context_absname_ob);
	return result;
err_context_absname_ob:
	Dee_Decref_likely(context_absname_ob);
err:
	return -1;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeModule_CanHaveRelName(DeeModuleObject const *__restrict self) {
	if (self->mo_absname == NULL)
		return false;
	if (self->mo_flags & Dee_MODULE_FABSFILE) {
		if (Dee_TYPE(self) != &DeeModuleDir_Type)
			return false;
	}
	return true;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
do_DeeModule_PrintRelNameEx(DeeModuleObject *__restrict self,
                            Dee_formatprinter_t printer, void *arg,
                            /*utf-8*/ char const *context_absname,
                            size_t context_absname_size,
                            DeeStringObject *context_absname_ob,
                            unsigned int flags) {
	if (flags & DeeModule_RELNAME_F_LIBNAM) {
		DREF /*String*/ DeeObject *result;
		result = DeeModule_GetLibName(self, 0);
		if (result != ITER_DONE) {
			Dee_ssize_t status;
			status = DeeString_PrintUtf8(result, printer, arg);
			Dee_Decref_unlikely(result);
			return status;
		}
	}
	if (!DeeModule_CanHaveRelName(self))
		return 0;
	return do_DeeModule_PrintRelNameEx_impl(self, printer, arg, context_absname,
	                                        context_absname_size, context_absname_ob,
	                                        flags);
}

PRIVATE WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
do_DeeModule_GetRelNameEx(DeeModuleObject *__restrict self,
                          /*utf-8*/ char const *context_absname,
                          size_t context_absname_size,
                          DeeStringObject *context_absname_ob,
                          unsigned int flags) {
	Dee_ssize_t status;
	struct unicode_printer printer;
	if (flags & DeeModule_RELNAME_F_LIBNAM) {
		DREF /*String*/ DeeObject *result;
		result = DeeModule_GetLibName(self, 0);
		if (result != ITER_DONE)
			return result;
	}
	if (!DeeModule_CanHaveRelName(self))
		return ITER_DONE;
	unicode_printer_init(&printer);
	status = do_DeeModule_PrintRelNameEx_impl(self, &unicode_printer_print, &printer,
	                                          context_absname, context_absname_size,
	                                          context_absname_ob, flags);
	if unlikely(status < 0)
		goto err_printer;
	if unlikely(status == 0) {
		unicode_printer_fini(&printer);
		return ITER_DONE;
	}
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}

/* Return the relative import name of `self' when accessed from a file or module
 * `context_absname'. For more information, see `DeeModule_GetRelNameEx()'.
 * 
 * @param: flags: Set of `DeeModule_RELNAME_F_*'
 * @return: * :        The module's name, written relative to `context_absname'
 * @return: ITER_DONE: The given module is anonymous or has its `Dee_MODULE_FABSFILE' flag set
 * @return: NULL:      An error was thrown. */
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetRelName(DeeModuleObject *__restrict self,
                     /*Module|String|Type|None*/ DeeObject *context_absname,
                     unsigned int flags) {
	char const *context_absname_utf8;
	size_t context_absname_size;
	DeeStringObject *context_absname_ob = NULL;
	if (context_absname == NULL || DeeNone_Check(context_absname)) {
no_context:
		context_absname_utf8 = NULL;
		context_absname_size = 0;
	} else if (DeeModule_Check(context_absname)) {
		DeeModuleObject *context_mod = (DeeModuleObject *)context_absname;
		if (self == context_mod)
			return_reference(&str_dot);
		context_absname_utf8 = context_mod->mo_absname;
		context_absname_size = context_absname_utf8 ? strlen(context_absname_utf8) : 0;
		flags &= ~DeeModule_RELNAME_F_CTXDIR;
	} else if (DeeString_Check(context_absname)) {
		context_absname_ob = (DeeStringObject *)context_absname;
		context_absname_utf8 = DeeString_AsUtf8(context_absname);
		if unlikely(!context_absname_utf8)
			goto err;
		context_absname_size = WSTR_LENGTH(context_absname_utf8);
	} else if (DeeType_Check(context_absname)) {
		DREF /*String*/ DeeObject *result;
		context_absname = Dee_AsObject(DeeType_GetModule((DeeTypeObject *)context_absname));
		if (!context_absname)
			goto no_context;
		result = DeeModule_GetRelName(self, context_absname, flags);
		Dee_Decref_unlikely(context_absname);
		return result;
	} else {
		DeeObject_TypeAssertFailed3(context_absname, &DeeModule_Type, &DeeString_Type, &DeeType_Type);
		goto err;
	}
	return do_DeeModule_GetRelNameEx(self,
	                                 context_absname_utf8, context_absname_size,
	                                 context_absname_ob, flags);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetRelNameEx(DeeModuleObject *__restrict self,
                       /*utf-8*/ char const *context_absname,
                       size_t context_absname_size, unsigned int flags) {
	return do_DeeModule_GetRelNameEx(self, context_absname,
	                                 context_absname_size,
	                                 NULL, flags);
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeModule_PrintRelNameEx(DeeModuleObject *__restrict self,
                         Dee_formatprinter_t printer, void *arg,
                         /*utf-8*/ char const *context_absname,
                         size_t context_absname_size, unsigned int flags) {
	return do_DeeModule_PrintRelNameEx(self, printer, arg,
	                                   context_absname, context_absname_size,
	                                   NULL, flags);
}

/* Possible return values for:
 * - module_try_add_missing_libname_or_unlock
 * - module_try_add_missing_libnames_or_unlock */
#define MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK    0    /* Success (write-lock is still held) */
#define MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__ERROR (-1) /* Error was thrown (write-lock was released) */
#define MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__AGAIN 1    /* Try again (write-lock was released) */

PRIVATE NONNULL((1)) void DCALL
replace_slash_with_dot(DeeStringObject *__restrict self) {
	size_t i, length = DeeString_WLEN(self);
	for (i = 0; i < length; ++i) {
		uint32_t ch = DeeString_GetChar(self, i);
		if (ch == DeeSystem_SEP)
			DeeString_SetChar(self, i, '.');
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_try_add_missing_libname_string_or_unlock(DeeModuleObject *__restrict self,
                                                char const *libname_with_slash_for_dots) {
	struct Dee_module_libentry *libentry;
	DREF DeeStringObject *missing_libname;
	/* Create a new UTF-8 string for "libname_with_slash_for_dots".
	 * Once that string has been created, we modify its contents to
	 * replace '/' with '.' */
	missing_libname = (DREF DeeStringObject *)DeeString_TryNewUtf8(libname_with_slash_for_dots,
	                                                               strlen(libname_with_slash_for_dots),
	                                                               STRING_ERROR_FIGNORE);
	if unlikely(!missing_libname) {
		module_libtree_lock_endwrite();
		missing_libname = (DREF DeeStringObject *)DeeString_NewUtf8(libname_with_slash_for_dots,
		                                                            strlen(libname_with_slash_for_dots),
		                                                            STRING_ERROR_FIGNORE);
		if (!missing_libname)
			return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__ERROR;
		Dee_DecrefDokill(missing_libname);
		return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__AGAIN;
	}
	ASSERT(!DeeObject_IsShared(missing_libname));
	replace_slash_with_dot(missing_libname);

	/* Check if "missing_libname" might already be present in "module_libtree_root". */
	{
		struct Dee_module_libentry *existing_entry;
		existing_entry = module_libtree_locate(module_libtree_root, missing_libname);
		if (existing_entry) {
			/* Entry already exists (possibly for another module) -> can't add */
			Dee_DecrefDokill(missing_libname);
			return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK;
		}
	}

	/* Find or allocate a free "struct Dee_module_libentry"-slot */
	libentry = &self->mo_libname;
	if (libentry->mle_name) {
		/* Pre-allocated entry is already in-use -> must allocate a secondary one. */
		libentry = Dee_module_libentry_tryalloc();
		if unlikely(!libentry) {
			module_libtree_lock_endwrite();
			libentry = Dee_module_libentry_alloc();
			Dee_DecrefDokill(missing_libname);
			if (!libentry)
				return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__ERROR;
			Dee_module_libentry_free(libentry);
			return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__AGAIN;
		}
	}

	/* Initialize the new "struct Dee_module_libentry"-slot */
	libentry->mle_name        = missing_libname; /* Inherit reference */
	libentry->mle_dat.mle_mod = self;
	libentry->mle_next        = NULL;
	if (libentry != &self->mo_libname) {
		libentry->mle_next = self->mo_libname.mle_next;
		self->mo_libname.mle_next = libentry;
	}

	/* Add the new "struct Dee_module_libentry"-slot to "module_libtree_root". */
	module_libtree_insert(&module_libtree_root, libentry);

	/* Done! */
	return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
streq_replace_slash_with_dot(char const *lhs,
                             char const *rhs_with_slashes) {
	char ch_lhs, ch_rhs;
	for (;;) {
		ch_lhs = *lhs++;
		ch_rhs = *rhs_with_slashes++;
		if (ch_lhs != ch_rhs) {
			if (ch_lhs == '.' && ch_rhs == DeeSystem_SEP) {
				/* OK! */
			} else {
				break;
			}
		} else {
			if (!ch_lhs)
				return true;
		}
	}
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
module_has_libname_replace_slash_with_dot(DeeModuleObject *__restrict self,
                                          char const *libname_with_slashes) {
	struct Dee_module_libentry *iter = &self->mo_libname;
	if (!iter->mle_name)
		return false;
	do {
		char const *name_utf8;
		ASSERT(iter->mle_name);
		name_utf8 = DeeString_AsUtf8((DeeObject *)iter->mle_name);
		ASSERTF(name_utf8, "Should have been pre-loaded by caller");
		if (streq_replace_slash_with_dot(name_utf8, libname_with_slashes))
			return true;
	} while ((iter = iter->mle_next) != NULL);
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_try_add_missing_libname_or_unlock(DeeModuleObject *__restrict self,
                                         char const *path_utf8_wstr) {
	size_t path_utf8_size = WSTR_LENGTH(path_utf8_wstr);
	char const *module_absname = self->mo_absname;
	ASSERT(module_absname);
	if (strlen(module_absname) < path_utf8_size)
		return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK; /* Our module isn't in this path! */
	if (fs_bcmp(module_absname, path_utf8_wstr, path_utf8_size) != 0)
		return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK; /* Our module isn't in this path! */
	if (module_absname[path_utf8_size] != DeeSystem_SEP)
		return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK; /* Our module isn't in this path! */
	module_absname += path_utf8_size + 1;

	/* At this point, we must simply try to add a libname
	 * entry for `module_absname.replace("/", ".")', but
	 * only if that `"." !in module_absname' */
	if (strchr(module_absname, '.'))
		return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK; /* Path won't work as a libname! */

	/* Check if "self" already has "module_absname" as one of its libnames */
	if (module_has_libname_replace_slash_with_dot(self, module_absname))
		return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK; /* Already added */

	return module_try_add_missing_libname_string_or_unlock(self, module_absname);
}

/* Try to add all missing lib names to "self" */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_try_add_missing_libnames_or_unlock(DeeModuleObject *__restrict self,
                                          DeeTupleObject *__restrict libpath) {
	size_t i;
	ASSERT(self->mo_absname);
	for (i = 0; i < DeeTuple_SIZE(libpath); ++i) {
		int status;
		DeeStringObject *path = (DeeStringObject *)DeeTuple_GET(libpath, i);
		char const *utf8 = DeeString_AsUtf8(Dee_AsObject(path));
		ASSERTF(utf8, "Should have been pre-loaded by caller");
		status = module_try_add_missing_libname_or_unlock(self, utf8);
		if (status != MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK)
			return status;
	}
	return MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK;
}

/* Acquire a lock to `module_libtree_lock_read()' whilst simultaneously
 * ensuring that all possible libpath entries of "self" has been allocated,
 * or at the very least up to- and including the `load_until'th one. */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
module_lock_and_load_libnames(DeeModuleObject *__restrict self, size_t load_until) {
	DREF DeeTupleObject *libpath;
	uint16_t flags;
again:
	module_libtree_lock_read();
	if (self->mo_absname == NULL)
		return 0; /* Module can't have any lib names */
	flags = atomic_read(&self->mo_flags);
	if (flags & (Dee_MODULE_FABSFILE | _Dee_MODULE_FLIBALL)) {
		/* All possible lib names have already been loaded for
		 * this module, or this module can't have any lib names. */
		if (flags & _Dee_MODULE_FLIBALL)
			return 0;
		if (Dee_TYPE(self) != &DeeModuleDir_Type)
			return 0;
	}
	/* Fast-pass: is the libname list loaded far enough? */
	if (load_until != (size_t)-1 && self->mo_libname.mle_name) {
		struct Dee_module_libentry *iter;
		if (load_until == 0)
			return 0;
		iter = self->mo_libname.mle_next;
		while (iter) {
			ASSERT(iter->mle_name);
			if (!--load_until)
				return 0;
			iter = iter->mle_next;
		}
	}
	module_libtree_lock_endread();
	libpath = (DREF DeeTupleObject *)DeeModule_GetLibPath();
	if unlikely(!libpath)
		goto err;
again_lock_libtree:
	module_libtree_lock_write();
	ASSERT(self->mo_absname != NULL);
	if likely(!(atomic_read(&self->mo_flags) & _Dee_MODULE_FLIBALL)) {
		int status;
		/* Check that "libpath" is still up-to-date */
		if unlikely((DeeTupleObject *)Dee_atomic_ref_getaddr(&deemon_path) != libpath)
			goto unlock_write_and_try_again;

		/* Enumerate "libpath" and build missing "struct Dee_module_libentry" for
		 * every path where "self" is a child of the path's sub-directory, and
		 * where no other module exists that already carries the same name. */
		status = module_try_add_missing_libnames_or_unlock(self, libpath);
		switch (status) {
		case MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__OK:
			break;
		case MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__ERROR:
			goto err_libpath;
		case MODULE_TRY_ADD_MISSING_LIBNAMES_OR_UNLOCK__AGAIN:
			goto again_lock_libtree;
		default: __builtin_unreachable();
		}

		/* Remember that all possible lib names have been loaded for this module. */
		atomic_or(&self->mo_flags, _Dee_MODULE_FLIBALL);
	}
	if unlikely(!Dee_DecrefIfNotOne(libpath)) {
unlock_write_and_try_again:
		module_libtree_lock_endwrite();
		Dee_Decref_likely(libpath);
		goto again;
	}
	module_libtree_lock_downgrade();
	return 0;
err_libpath:
	Dee_Decref_likely(libpath);
err:
	return -1;
}

/* Ensure that all possible lib (global) names for `self' have been
 * determined (using paths from `DeeModule_SetLibPath()'), then return
 * the `index'th (0-based) one of them.
 * - A special case is made for the builtin `DeeModule_Deemon',
 *   which always has exactly `1' lib name "deemon".
 * - When the same module may be accessible from multiple lib paths,
 *   then the order in which its possible absolute names are listed
 *   is undefined.
 * - When `DEEMON_PATH' is set-up such that multiple modules might
 *   hold the same lib name, only one of them will (and this function
 *   will also list them for only that one module), though it is
 *   undefined which of those modules that will be.
 *
 * Examples:
 * - rt.gen.unpack        (self='/opt/deemon/lib/rt/gen/unpack.dee', DEEMON_PATH="/opt/deemon/lib", index=0)
 * - ITER_DONE            (self='/home/me/projects/foo/script.dee', DEEMON_PATH="/opt/deemon/lib", index=<ignored>)
 * - lib.rt.gen.unpack    (self='/opt/deemon/lib/rt/gen/unpack.dee', DEEMON_PATH="/opt/deemon:/opt/deemon/lib", index=0)
 * - rt.gen.unpack        (self='/opt/deemon/lib/rt/gen/unpack.dee', DEEMON_PATH="/opt/deemon:/opt/deemon/lib", index=1)
 *
 * @return: * :        The module's index'th lib name, written relative to `context_absname'
 * @return: ITER_DONE: The given module is anonymous or has its `Dee_MODULE_FABSFILE'
 *                     flag set, or isn't located in a sub-directory of `DEEMON_PATH',
 *                     or `index' is greater than the module's # of lib names.
 * @return: NULL:      An error was thrown. */
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetLibName(DeeModuleObject *__restrict self, size_t index) {
	DREF /*String*/ DeeObject *result;
	struct Dee_module_libentry *libname;
#if 0 /* Not needed -- see initialization of "DeeModule_Deemon" */
	/* Special case for the builtin "deemon" module */
	if (self == &DeeModule_Deemon) {
		if (index == 0)
			return_reference(&str_deemon);
		return ITER_DONE;
	}
#endif
	if unlikely(module_lock_and_load_libnames(self, index))
		goto err;
	for (libname = &self->mo_libname; index; --index) {
		libname = libname->mle_next;
		if unlikely(!libname)
			goto iter_done;
	}
	result = (DeeObject *)libname->mle_name;
	if unlikely(!result)
		goto iter_done;
	Dee_Incref(result);
	module_libtree_lock_endread();
	return result;
iter_done:
	module_libtree_lock_endread();
	return ITER_DONE;
err:
	return NULL;
}

/* Return 1+ the greatest index that may be passed to `DeeModule_GetLibName()' for the
 * purpose of querying module lib names. Note that calls to `DeeModule_SetLibPath()'
 * (even those made from different threads) may cause the return value of this function
 * to fall out-of-date the second this function does return, so be always be prepared
 * for `DeeModule_GetLibName()' to return `ITER_DONE' even before this limit is reached.
 *
 * @return: 0 : The given module is anonymous or has its `Dee_MODULE_FABSFILE'
 *              flag set, or isn't located in a sub-directory of `DEEMON_PATH'.
 * @return: * : The # of lib names that `self' had at the time of this call.
 * @return: (size_t)-1: An error was thrown. */
PUBLIC WUNUSED NONNULL((1)) size_t DCALL
DeeModule_GetLibNameCount(DeeModuleObject *__restrict self) {
	size_t result = 0;
	struct Dee_module_libentry *libname;
#if 0 /* Not needed -- see initialization of "DeeModule_Deemon" */
	/* Special case for the builtin "deemon" module */
	if (self == &DeeModule_Deemon)
		return 1;
#endif
	if unlikely(module_lock_and_load_libnames(self, (size_t)-1))
		goto err;
	libname = &self->mo_libname;
	if (libname->mle_name) {
		do {
			++result;
		} while ((libname = libname->mle_next) != NULL);
	}
	module_libtree_lock_endread();
	return result;
err:
	return (size_t)-1;
}

DECL_END
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/tpp.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/gc.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/minmax.h>      /* MAX_C */
#include <hybrid/sched/yield.h> /* SCHED_YIELD */

#ifndef CONFIG_NO_DEX
#include <deemon/dex.h>
#endif /* !CONFIG_NO_DEX */

#ifndef CONFIG_NO_DEC
#include <deemon/dec.h>
#endif /* !CONFIG_NO_DEC */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#undef token
#undef tok
#undef yield
#undef yieldnb
#undef yieldnbif
#undef skip

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#define token             TPPLexer_Global.l_token
#define tok               TPPLexer_Global.l_token.t_id
#define yield()           TPPLexer_Yield()
#define yieldnb()         TPPLexer_YieldNB()
#define yieldnbif(allow)  ((allow) ? TPPLexer_YieldNB() : TPPLexer_Yield())
#define skip(expected_tok, ...) unlikely(likely(tok == (expected_tok)) ? (yield() < 0) : parser_skip(expected_tok, __VA_ARGS__))


DECL_BEGIN

/*[[[deemon
print("#if 1");
print("#define HASHOF_str_deemon ", (_Dee_HashSelect from rt.gen.hash)("deemon"));
print("#else");
print("#define HASHOF_str_deemon DeeString_Hash(&str_deemon)");
print("#endif");
]]]*/
#if 1
#define HASHOF_str_deemon _Dee_HashSelectC(0x4579666d, 0xeb3bb684d0ec756)
#else
#define HASHOF_str_deemon DeeString_Hash(&str_deemon)
#endif
/*[[[end]]]*/

INTDEF struct module_symbol empty_module_buckets[];

#define SEP   DeeSystem_SEP
#define SEP_S DeeSystem_SEP_S
#define ISSEP DeeSystem_IsSep
#define ISABS DeeSystem_IsAbs

#ifdef DeeSystem_HAVE_FS_ICASE
#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */
#endif /* DeeSystem_HAVE_FS_ICASE */


#ifdef DeeSystem_HAVE_FS_ICASE
#define fs_memcmp                        memcasecmp
#define fs_bcmp                          memcasecmp
#define fs_hashobj(ob)                   DeeString_HashCase(Dee_AsObject(ob))
#define fs_hashstr(s)                    Dee_HashCaseStr(s)
#define fs_hashutf8(s, n)                Dee_HashCaseUtf8(s, n)
#define fs_hashmodname_equals(mod, hash) 1
#define fs_hashmodpath(mod)              ((mod)->mo_pathihash)
#define fs_hashmodpath_equals(mod, hash) ((mod)->mo_pathihash == (hash))
#else /* DeeSystem_HAVE_FS_ICASE */
#define fs_memcmp                        memcmp
#define fs_bcmp                          bcmp
#define fs_hashobj(ob)                   DeeString_Hash(Dee_AsObject(ob))
#define fs_hashstr(s)                    Dee_HashStr(s)
#define fs_hashutf8(s, n)                Dee_HashUtf8(s, n)
#define fs_hashmodpath(mod)              DeeString_HASH((mod)->mo_path)
#define fs_hashmodname_equals(mod, hash) (DeeString_HASH((mod)->mo_name) == (hash))
#define fs_hashmodpath_equals(mod, hash) (DeeString_HASH((mod)->mo_path) == (hash))
#endif /* !DeeSystem_HAVE_FS_ICASE */

#define DeeString_FS_EQUALS_STR(lhs, rhs)            \
	(DeeString_SIZE(lhs) == DeeString_SIZE(rhs) &&   \
	 fs_bcmp(DeeString_STR(lhs), DeeString_STR(rhs), \
	         DeeString_SIZE(lhs) * sizeof(char)) == 0)
#define DeeString_FS_EQUALS_BUF(lhs, rhs_base, rhs_size) \
	(DeeString_SIZE(lhs) == (rhs_size) &&                \
	 fs_bcmp(DeeString_STR(lhs), rhs_base,               \
	         DeeString_SIZE(lhs) * sizeof(char)) == 0)



/* Begin loading the given module.
 * @return: 0: You're now responsible to load the module.
 * @return: 1: The module has already been loaded.
 * @return: 2: You've already started loading this module. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_BeginLoading(DeeModuleObject *__restrict self) {
	uint16_t flags;
#ifndef CONFIG_NO_THREADS
	DeeThreadObject *caller = DeeThread_Self();
#endif /* !CONFIG_NO_THREADS */
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
begin_loading:
	flags = atomic_fetchor(&self->mo_flags, Dee_MODULE_FLOADING);
	if (flags & Dee_MODULE_FLOADING) {
		/* Module is already being loaded. */
		while ((flags = atomic_read(&self->mo_flags),
		        (flags & (Dee_MODULE_FLOADING | Dee_MODULE_FDIDLOAD)) ==
		        Dee_MODULE_FLOADING)) {
#ifdef CONFIG_NO_THREADS
			return 2;
#else /* CONFIG_NO_THREADS */
			/* Check if the module is being loaded in the current thread. */
			if (self->mo_loader == caller)
				return 2;
#ifdef CONFIG_HOST_WINDOWS
			/* Sleep a bit longer than usually. */
			DBG_ALIGNMENT_DISABLE();
			__NAMESPACE_INT_SYM SleepEx(1000, 0);
			DBG_ALIGNMENT_ENABLE();
#else /* CONFIG_HOST_WINDOWS */
			SCHED_YIELD();
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_NO_THREADS */
		}
		/* If the module has now been marked as having finished loading,
		 * then simply act as though it was us that did it. */
		if (flags & Dee_MODULE_FDIDLOAD)
			return 1;
		goto begin_loading;
	}
#ifndef CONFIG_NO_THREADS
	/* Setup the module to indicate that we're the ones loading it. */
	self->mo_loader = caller;
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
DeeModule_FailLoading(DeeModuleObject *__restrict self) {
	atomic_and(&self->mo_flags, ~(Dee_MODULE_FLOADING));
}

PRIVATE NONNULL((1)) void DCALL
DeeModule_DoneLoading(DeeModuleObject *__restrict self) {
	atomic_or(&self->mo_flags, Dee_MODULE_FDIDLOAD);
}

INTERN WUNUSED NONNULL((1)) int DCALL
TPPFile_SetStartingLineAndColumn(struct TPPFile *__restrict self,
                                 int start_line, int start_col) {
	/* Set the starting-line offset. */
	self->f_textfile.f_lineoff = start_line;
	if (start_col > 0) {
		struct TPPString *pad_text;
		/* Insert some padding white-space to make it look like the first line
		 * starts with a whole bunch of whitespace, thereby adjusting the offset
		 * of the starting column number in the first line. */
		pad_text = TPPString_NewSized((size_t)(unsigned int)start_col);
		if unlikely(!pad_text)
			goto err;
		/* Use space characters to pad text. */
		memset(pad_text->s_text, ' ', pad_text->s_size);
		TPPString_Decref(self->f_text);
		self->f_text  = pad_text; /* Inherit reference */
		self->f_begin = pad_text->s_text;
		self->f_end   = pad_text->s_text + pad_text->s_size;
		/* Start scanning _after_ the padding text (don't produce white-space tokens before then!) */
		self->f_pos = self->f_end;
	}
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_LoadSourceStreamEx(DeeModuleObject *__restrict self,
                             DeeObject *__restrict input_file,
                             int start_line, int start_col,
                             struct compiler_options *options,
                             struct string_object *input_pathname) {
	DREF DeeCompilerObject *compiler;
	struct TPPFile *base_file;
	DREF struct ast *code;
	DREF DeeCodeObject *root_code;
	int result;
	uint16_t assembler_flags;
	uint16_t compiler_flags;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	ASSERT_OBJECT_TYPE(input_file, &DeeFile_Type.ft_base);
	ASSERT_OBJECT_TYPE_EXACT_OPT(input_pathname, &DeeString_Type);
#if 1 /* Always prefer the manual override */
	if (options && options->co_pathname)
		input_pathname = options->co_pathname;
#else
	if (!input_pathname && options)
		input_pathname = options->co_pathname;
#endif
	compiler_flags = COMPILER_FNORMAL;
	if (options)
		compiler_flags = options->co_compiler;

	/* Create a new compiler for the module. */
	compiler = DeeCompiler_New(Dee_AsObject(self), compiler_flags);
	if unlikely(!compiler)
		goto err;

	/* Start working with this compiler. */
	if (COMPILER_BEGIN(compiler))
		goto err_compiler_not_locked;
	base_file = TPPFile_OpenStream((stream_t)input_file,
	                               input_pathname
	                               ? DeeString_STR(input_pathname)
	                               : "");
	if unlikely(!base_file)
		goto err_compiler;

	/* Set the starting-line offset. */
	if (TPPFile_SetStartingLineAndColumn(base_file, start_line, start_col)) {
		TPPFile_Decref(base_file);
		goto err_compiler;
	}

	/* Push the initial source file onto the #include-stack,
	 * and TPP inherit our reference to it. */
	TPPLexer_PushFileInherited(base_file);

	/* Override the name that is used as the
	 * effective display/DDI string of the file. */
	if (options && options->co_filename) {
		struct TPPString *used_name;
		ASSERT_OBJECT_TYPE_EXACT(options->co_filename, &DeeString_Type);
		used_name = TPPString_New(DeeString_STR(options->co_filename),
		                          DeeString_SIZE(options->co_filename));
		if unlikely(!used_name)
			goto err_compiler;
		ASSERT(!base_file->f_textfile.f_usedname);
		base_file->f_textfile.f_usedname = used_name; /* Inherit */
	}
	ASSERT(!current_basescope->bs_name);

	/* Set the name of the current base-scope, which
	 * describes the function of the module's root code. */
	if (options && options->co_rootname) {
		ASSERT_OBJECT_TYPE_EXACT(options->co_rootname, &DeeString_Type);
		current_basescope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(options->co_rootname),
		                                                    DeeString_SIZE(options->co_rootname), 1);
		if unlikely(!current_basescope->bs_name)
			goto err_compiler;
	}

	assembler_flags        = 0;
	inner_compiler_options = NULL;
	if (options) {
		/* Load custom parser/optimizer flags. */
		assembler_flags        = options->co_assembler;
		compiler->cp_options   = options;
		inner_compiler_options = options->co_inner;
		parser_flags           = options->co_parser;
		optimizer_flags        = options->co_optimizer;
		optimizer_unwind_limit = options->co_unwind_limit;
		if (options->co_tabwidth)
			TPPLexer_Current->l_tabsize = (size_t)options->co_tabwidth;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;

		if (options->co_setup) {
			/* Run a custom setup protocol. */
			result = (*options->co_setup)(options->co_setup_arg);
			if unlikely(result < 0) {
				DeeCompiler_End();
				Dee_Decref(compiler);
				DeeCompiler_LockEndWrite();
				return result;
			}
		}
	}

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots = new_unnamed_symbol();
		if unlikely(!dots)
			goto err_compiler;
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_compiler;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
		dots->s_decltype.da_type = DAST_NONE;
		dots->s_type  = SYMBOL_TYPE_ARG;
		dots->s_symid = 0;
		dots->s_flag |= SYMBOL_FALLOC;
		current_basescope->bs_argc    = 1;
		current_basescope->bs_argv[0] = dots;
		current_basescope->bs_varargs = dots;
		current_basescope->bs_flags |= CODE_FVARARGS;
	}

	/* Save the current exception context. */
	parser_start();

	/* Yield the initial token. */
	if unlikely(yield() < 0) {
		code = NULL;
	} else {
		/* Parse statements until the end of the source stream. */
		code = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, TOK_EOF);
	}

	if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
		TPPLexer_ClearIfdefStack();

	/* Rethrow all errors that may have occurred during parsing. */
	if (parser_rethrow(code == NULL)) {
		ast_xdecref(code);
		code = NULL;
	}

	if unlikely(!code)
		goto err_compiler;

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		result = ast_optimize_all(code, false);
		/* Rethrow all errors that may have occurred during optimization. */
		if (parser_rethrow(result != 0))
			result = -1;
		if (result)
			goto err_compiler_ast;
	}

	{
		uint16_t refc;
		struct asm_symbol_ref *refv;
		root_code = code_compile(code, assembler_flags, true, &refc, &refv);
		ASSERT(!root_code || !refc);
		ASSERT(!root_code || !refv);
	}
	ast_decref(code);

	/* Rethrow all errors that may have occurred during text assembly. */
	if (parser_rethrow(root_code == NULL))
		Dee_XClear(root_code);

	/* Check for errors during assembly. */
	if unlikely(!root_code)
		goto err_compiler;

	/* Finally, put together the module itself. */
	result = module_compile(self, root_code, assembler_flags);
	Dee_Decref(root_code);

	/* Rethrow all errors that may have occurred during module linkage. */
	if (parser_rethrow(result != 0))
		result = -1;

	DeeCompiler_End();
	Dee_Decref(compiler);
	DeeCompiler_LockEndWrite();
	return result;
err_compiler_ast:
	ast_decref(code);
err_compiler:
	DeeCompiler_End();
	Dee_Decref(compiler);
	DeeCompiler_LockEndWrite();
err:
	return -1;
err_compiler_not_locked:
	Dee_Decref(compiler);
	goto err;
}

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
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_LoadSourceStream(DeeModuleObject *__restrict self,
                           /*File*/ DeeObject *__restrict input_file,
                           int start_line, int start_col,
                           struct compiler_options *options) {
	int result;
	ASSERT(self != (DeeModuleObject *)input_file);
	result = DeeModule_BeginLoading(self);
	if (result == 0) {
		result = DeeModule_LoadSourceStreamEx(self,
		                                      input_file,
		                                      start_line,
		                                      start_col,
		                                      options,
		                                      NULL);
		if unlikely(result) {
			DeeModule_FailLoading(self);
		} else {
			DeeModule_DoneLoading(self);
		}
	}
	return result;
}







LIST_HEAD(module_object_list, module_object);

/* Filesystem-based module hash table. */
PRIVATE size_t /*               */ modules_c = 0;    /* [lock(modules_lock)] Amount of modules in-cache. */
PRIVATE size_t /*               */ modules_a = 0;    /* [lock(modules_lock)] Allocated hash-map size. */
PRIVATE struct module_object_list *modules_v = NULL; /* [lock(modules_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t modules_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define modules_lock_reading()    Dee_atomic_rwlock_reading(&modules_lock)
#define modules_lock_writing()    Dee_atomic_rwlock_writing(&modules_lock)
#define modules_lock_tryread()    Dee_atomic_rwlock_tryread(&modules_lock)
#define modules_lock_trywrite()   Dee_atomic_rwlock_trywrite(&modules_lock)
#define modules_lock_canread()    Dee_atomic_rwlock_canread(&modules_lock)
#define modules_lock_canwrite()   Dee_atomic_rwlock_canwrite(&modules_lock)
#define modules_lock_waitread()   Dee_atomic_rwlock_waitread(&modules_lock)
#define modules_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&modules_lock)
#define modules_lock_read()       Dee_atomic_rwlock_read(&modules_lock)
#define modules_lock_write()      Dee_atomic_rwlock_write(&modules_lock)
#define modules_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&modules_lock)
#define modules_lock_upgrade()    Dee_atomic_rwlock_upgrade(&modules_lock)
#define modules_lock_downgrade()  Dee_atomic_rwlock_downgrade(&modules_lock)
#define modules_lock_endwrite()   Dee_atomic_rwlock_endwrite(&modules_lock)
#define modules_lock_endread()    Dee_atomic_rwlock_endread(&modules_lock)
#define modules_lock_end()        Dee_atomic_rwlock_end(&modules_lock)

/* Name-based, global module hash table. */
PRIVATE size_t /*               */ modules_glob_c = 0;    /* [lock(modules_glob_lock)] Amount of modules in-cache. */
PRIVATE size_t /*               */ modules_glob_a = 0;    /* [lock(modules_glob_lock)] Allocated hash-map size. */
PRIVATE struct module_object_list *modules_glob_v = NULL; /* [lock(modules_glob_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t modules_glob_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define modules_glob_lock_reading()    Dee_atomic_rwlock_reading(&modules_glob_lock)
#define modules_glob_lock_writing()    Dee_atomic_rwlock_writing(&modules_glob_lock)
#define modules_glob_lock_tryread()    Dee_atomic_rwlock_tryread(&modules_glob_lock)
#define modules_glob_lock_trywrite()   Dee_atomic_rwlock_trywrite(&modules_glob_lock)
#define modules_glob_lock_canread()    Dee_atomic_rwlock_canread(&modules_glob_lock)
#define modules_glob_lock_canwrite()   Dee_atomic_rwlock_canwrite(&modules_glob_lock)
#define modules_glob_lock_waitread()   Dee_atomic_rwlock_waitread(&modules_glob_lock)
#define modules_glob_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&modules_glob_lock)
#define modules_glob_lock_read()       Dee_atomic_rwlock_read(&modules_glob_lock)
#define modules_glob_lock_write()      Dee_atomic_rwlock_write(&modules_glob_lock)
#define modules_glob_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&modules_glob_lock)
#define modules_glob_lock_upgrade()    Dee_atomic_rwlock_upgrade(&modules_glob_lock)
#define modules_glob_lock_downgrade()  Dee_atomic_rwlock_downgrade(&modules_glob_lock)
#define modules_glob_lock_endwrite()   Dee_atomic_rwlock_endwrite(&modules_glob_lock)
#define modules_glob_lock_endread()    Dee_atomic_rwlock_endread(&modules_glob_lock)
#define modules_glob_lock_end()        Dee_atomic_rwlock_end(&modules_glob_lock)


PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_file_module(DeeStringObject *__restrict module_file, Dee_hash_t hash) {
	DeeModuleObject *result = NULL;
	ASSERT(modules_lock_reading());
	if (modules_a) {
		result = LIST_FIRST(&modules_v[hash % modules_a]);
		while (result) {
			ASSERTF(result->mo_path, "All modules found in the file cache must have a path assigned");
			ASSERT_OBJECT_TYPE_EXACT(result->mo_path, &DeeString_Type);
			if (fs_hashmodpath_equals(result, hash) &&
			    DeeString_FS_EQUALS_STR(result->mo_path, module_file))
				break; /* Found it! */
			result = LIST_NEXT(result, mo_link);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_glob_module(DeeStringObject *__restrict module_name) {
	Dee_hash_t hash = fs_hashobj(module_name);
	DeeModuleObject *result = NULL;
	ASSERT(modules_glob_lock_reading());
	if (modules_glob_a) {
		result = LIST_FIRST(&modules_glob_v[hash % modules_glob_a]);
		while (result) {
			ASSERT_OBJECT_TYPE_EXACT(result->mo_name, &DeeString_Type);
			if (fs_hashmodname_equals(result, hash) &&
			    DeeString_FS_EQUALS_STR(result->mo_name, module_name))
				break; /* Found it! */
			result = LIST_NEXT(result, mo_globlink);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_glob_module_str(/*utf-8*/ char const *__restrict module_name_str,
                     size_t module_name_len) {
	Dee_hash_t hash = fs_hashutf8(module_name_str, module_name_len);
	DeeModuleObject *result = NULL;
	ASSERT(modules_glob_lock_reading());
	if (modules_glob_a) {
		result = LIST_FIRST(&modules_glob_v[hash % modules_glob_a]);
		while (result) {
			ASSERT_OBJECT_TYPE_EXACT(result->mo_name, &DeeString_Type);
			if (fs_hashmodname_equals(result, hash) &&
			    DeeString_FS_EQUALS_BUF(result->mo_name, module_name_str, module_name_len))
				break; /* Found it! */
			result = LIST_NEXT(result, mo_globlink);
		}
	}
	return result;
}

PRIVATE bool DCALL rehash_file_modules(void) {
	struct module_object_list *new_vector, *biter, *bend, *dst;
	DeeModuleObject *iter, *next;
	size_t new_size = modules_a * 2;
	ASSERT(modules_lock_writing());
	if unlikely(!new_size)
		new_size = 4;
do_alloc_new_vector:
	new_vector = (struct module_object_list *)Dee_TryCallocc(new_size, sizeof(struct module_object_list));
	if unlikely(!new_vector) {
		if (modules_a != 0)
			return true; /* Don't actually need to rehash. */
		if (new_size != 1) {
			new_size = 1;
			goto do_alloc_new_vector;
		}
		return false;
	}
	ASSERT(new_size != 0);
	bend = (biter = modules_v) + modules_a;
	for (; biter < bend; ++biter) {
		iter = LIST_FIRST(biter);
		while (iter) {
			next = LIST_NEXT(iter, mo_link);
			ASSERTF(iter->mo_path, "All modules found in the file cache must have a path assigned");
			ASSERT_OBJECT_TYPE_EXACT(iter->mo_path, &DeeString_Type);

			/* Re-hash this entry. */
			dst = &new_vector[fs_hashmodpath(iter) % new_size];
			LIST_REMOVE(iter, mo_link);
			LIST_INSERT_HEAD(dst, iter, mo_link);

			/* Continue with the next. */
			iter = next;
		}
	}
	Dee_Free(modules_v);
	modules_v = new_vector;
	modules_a = new_size;
	return true;
}

PRIVATE bool DCALL rehash_glob_modules(void) {
	struct module_object_list *new_vector, *biter, *bend, *dst;
	DeeModuleObject *iter, *next;
	size_t new_size = modules_glob_a * 2;
	ASSERT(modules_glob_lock_writing());
	if unlikely(!new_size)
		new_size = 4;
do_alloc_new_vector:
	new_vector = (struct module_object_list *)Dee_TryCallocc(new_size, sizeof(struct module_object_list));
	if unlikely(!new_vector) {
		if (modules_glob_a != 0)
			return true; /* Don't actually need to rehash. */
		if (new_size != 1) {
			new_size = 1;
			goto do_alloc_new_vector;
		}
		return false;
	}
	ASSERT(new_size != 0);
	bend = (biter = modules_glob_v) + modules_glob_a;
	for (; biter < bend; ++biter) {
		iter = LIST_FIRST(biter);
		while (iter) {
			next = LIST_NEXT(iter, mo_globlink);
			ASSERT_OBJECT_TYPE_EXACT(iter->mo_name, &DeeString_Type);

			/* Re-hash this entry. */
			dst = &new_vector[fs_hashobj(iter->mo_name) % new_size];
			LIST_REMOVE(iter, mo_globlink);
			LIST_INSERT_HEAD(dst, iter, mo_globlink);

			/* Continue with the next. */
			iter = next;
		}
	}
	Dee_Free(modules_glob_v);
	modules_glob_v = new_vector;
	modules_glob_a = new_size;
	return true;
}


PRIVATE NONNULL((1)) bool DCALL
add_file_module(DeeModuleObject *__restrict self) {
	Dee_hash_t hash;
	struct module_object_list *bucket;
	ASSERT(!LIST_ISBOUND(self, mo_link));
	ASSERT_OBJECT_TYPE_EXACT(self->mo_path, &DeeString_Type);
	ASSERT(modules_lock_writing());
	if (modules_c >= modules_a && !rehash_file_modules())
		return false;
	ASSERT(modules_a != 0);
	/* Insert the module into the table. */
	hash = fs_hashmodpath(self);
	Dee_DPRINTF("[RT] Caching module by-file %r\n", self->mo_path);
	bucket = &modules_v[hash % modules_a];
	LIST_INSERT_HEAD(bucket, self, mo_link);
	++modules_c;
	return true;
}

PRIVATE NONNULL((1)) bool DCALL
add_glob_module(DeeModuleObject *__restrict self) {
	Dee_hash_t hash;
	struct module_object_list *bucket;
	ASSERT(!LIST_ISBOUND(self, mo_globlink));
	ASSERT(self->mo_name);
#ifndef CONFIG_NO_THREADS
	ASSERT(modules_glob_lock_writing());
#endif /* !CONFIG_NO_THREADS */
	Dee_DPRINTF("[RT] Cached global module %r loaded from %r\n",
	            self->mo_name, self->mo_path ? self->mo_path : (DeeStringObject *)Dee_EmptyString);
	if (modules_glob_c >= modules_glob_a && !rehash_glob_modules())
		return false;
	ASSERT(modules_glob_a != 0);
	/* Insert the module into the table. */
	hash   = fs_hashobj(Dee_AsObject(self->mo_name));
	bucket = &modules_glob_v[hash % modules_glob_a];
	LIST_INSERT_HEAD(bucket, self, mo_globlink);
	++modules_glob_c;
	return true;
}



INTERN NONNULL((1)) void DCALL
module_unbind(DeeModuleObject *__restrict self) {
	if (LIST_ISBOUND(self, mo_link)) {
		modules_lock_write();
		COMPILER_READ_BARRIER();
		if (LIST_ISBOUND(self, mo_link)) {
			LIST_REMOVE(self, mo_link);
			if (!--modules_c) {
				Dee_Free(modules_v);
				modules_v = NULL;
				modules_a = 0;
			}
			modules_lock_endwrite();
		}
	}
	if (LIST_ISBOUND(self, mo_globlink)) {
		modules_glob_lock_write();
		COMPILER_READ_BARRIER();
		if (LIST_ISBOUND(self, mo_globlink)) {
			LIST_REMOVE(self, mo_globlink);
			if (!--modules_glob_c) {
				Dee_Free(modules_glob_v);
				modules_glob_v = NULL;
				modules_glob_a = 0;
			}
		}
		modules_glob_lock_endwrite();
	}
}

/* Given the filename of a module source file, load it
 * and create a new module from the contained source code.
 * NOTE: In case the module has been loaded before,
 *       return the already-loaded instance instead.
 * NOTE: In case the module is currently being loaded in the calling
 *       thread, that same partially loaded module is returned, meaning
 *       that the caller can easily check for `Dee_MODULE_FLOADING && !Dee_MODULE_FDIDLOAD'
 * @param: module_global_name: When non-NULL, use this as the module's actual name.
 *                             Also: register the module as a global module under this name when given.
 *                             When not given, the module isn't registered globally, and the
 *                             name of the module will be deduced from its `source_pathname'
 * @param: source_pathname:    The filename of the source file that should be opened.
 *                             When `NULL', simply use the absolute variant of `DeeString_AsUtf8(source_pathname)'
 * @param: throw_error:        When true, throw an error if the module couldn't be
 *                             found and return `NULL', otherwise return `ITER_DONE'.
 * @return: ITER_DONE:        `throw_error' is `true' and `source_pathname' could not be found. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenSourceFile(/*String*/ DeeObject *__restrict source_pathname,
                         /*String*/ DeeObject *module_global_name,
                         struct compiler_options *options,
                         bool throw_error) {
	DREF DeeModuleObject *existing_module;
	DREF DeeModuleObject *result;
	DREF DeeStringObject *module_name_ob;
	DREF DeeStringObject *module_path_ob;
	DREF DeeObject *input_stream;
	Dee_hash_t hash;
	ASSERT_OBJECT_TYPE(source_pathname, &DeeString_Type);
	ASSERT_OBJECT_TYPE_OPT(module_global_name, &DeeString_Type);
	module_path_ob = (DREF DeeStringObject *)DeeSystem_MakeNormalAndAbsolute(source_pathname);
	if unlikely(!module_path_ob)
		goto err;

	/* Quick check if this module had already been opened. */
	modules_lock_read();
	hash   = fs_hashobj(module_path_ob);
	result = find_file_module(module_path_ob, hash);
	if (result && Dee_IncrefIfNotZero(result)) {
		modules_lock_endread();
got_result_modulepath:
		Dee_Decref(module_path_ob);
		goto got_result;
	}
	modules_lock_endread();

	/* Also search for an existing instance
	 * of the specified global module name. */
#if 1 /* This is optional */
	if (ITER_ISOK(module_global_name)) {
		modules_glob_lock_read();
		result = find_glob_module((DeeStringObject *)module_global_name);
		if (result && Dee_IncrefIfNotZero(result)) {
			modules_glob_lock_endread();
			goto got_result_modulepath;
		}
		modules_glob_lock_endread();
	}
#endif

	/* Open the module's source file stream. */
	input_stream = DeeFile_Open(Dee_AsObject(module_path_ob), OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
	if unlikely(!ITER_ISOK(input_stream)) {
		result = (DREF DeeModuleObject *)input_stream;
		if (input_stream == ITER_DONE && throw_error) {
			err_file_not_found(Dee_AsObject(module_path_ob));
			result = NULL;
		}
		goto got_result_modulepath;
	}

	/* Create a new module. */
	if (ITER_ISOK(module_global_name)) {
		module_name_ob = (DREF DeeStringObject *)module_global_name;
	} else {
		char const *name_end, *name_start;
		char const *name;
		size_t size;
		name = DeeString_AsUtf8(source_pathname);
		if unlikely(!name)
			goto err_modulepath_inputstream;
		size      = WSTR_LENGTH(name);
		name_end  = name + size;
		name_start = DeeSystem_BaseName(name, size);

		/* Get rid of a file extension in the module name. */
		while (name_end > name_start && name_end[-1] != '.')
			--name_end;
		while (name_end > name_start && name_end[-1] == '.')
			--name_end;
		if (name_end == name_start)
			name_end = name + size;
		module_name_ob = (DREF DeeStringObject *)DeeString_NewUtf8(name_start,
		                                                           (size_t)(name_end - name_start),
		                                                           STRING_ERROR_FIGNORE);
		if unlikely(!module_name_ob)
			goto err_modulepath_inputstream;
	}

	/* Create the new module. */
	result = DeeModule_New(Dee_AsObject(module_name_ob));
	Dee_Decref_unlikely(module_name_ob);
	if unlikely(!result)
		goto err_modulepath_inputstream;

	/* Register the module in the filesystem & global cache. */
	result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef DeeSystem_HAVE_FS_ICASE
	result->mo_pathihash = hash;
#endif /* DeeSystem_HAVE_FS_ICASE */
	result->mo_flags |= Dee_MODULE_FLOADING;
	COMPILER_WRITE_BARRIER();

	/* Cache the new module as part of the filesystem
	 * module cache, as well as the global module cache. */
	if (module_global_name) {
set_file_module_global:
#ifndef CONFIG_NO_THREADS
		modules_lock_write();
		if (!modules_glob_lock_trywrite()) {
			modules_lock_endwrite();
			modules_glob_lock_write();
			if (!modules_lock_trywrite()) {
				modules_glob_lock_endwrite();
				goto set_file_module_global;
			}
		}
#endif /* !CONFIG_NO_THREADS */
		existing_module = find_file_module(module_path_ob, hash);
		if likely(!existing_module)
			existing_module = find_glob_module(module_name_ob);
		if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
			modules_glob_lock_endwrite();
			modules_lock_endwrite();
			Dee_DecrefDokill(result);
			Dee_Decref_likely(input_stream);
			result = existing_module;
			goto try_load_module_after_failure;
		}

		/* Add the module to the file-cache. */
		if ((modules_c >= modules_a && !rehash_file_modules()) ||
		    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
			modules_lock_endwrite();

			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_file_module_global;
			goto err_inputstream_r;
		}
		add_glob_module(result);
		add_file_module(result);
		modules_glob_lock_endwrite();
		modules_lock_endwrite();
	} else {
set_file_module:
		modules_lock_write();
		existing_module = find_file_module(module_path_ob, hash);
		if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
			modules_lock_endwrite();
			Dee_DecrefDokill(result);
			Dee_Decref_likely(input_stream);
			result = existing_module;
try_load_module_after_failure:
			if (DeeModule_BeginLoading(result) == 0)
				goto load_module_after_failure;
			goto got_result;
		}

		/* Add the module to the file-cache. */
		if unlikely(!add_file_module(result)) {
			modules_lock_endwrite();

			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_file_module;
			goto err_inputstream_r;
		}
		modules_lock_endwrite();
	}

load_module_after_failure:
	/* Actually load the module from its source stream. */
	{
		int error;
		error = DeeModule_LoadSourceStreamEx(result,
		                                     input_stream,
		                                     0,
		                                     0,
		                                     options,
		                                     module_path_ob);
		Dee_Decref(input_stream);
		if unlikely(error) {
			DeeModule_FailLoading(result);
			goto err_r;
		}
		DeeModule_DoneLoading(result);
	}
got_result:
	return result;
err_inputstream_r:
	Dee_Decref(input_stream);
err_r:
	Dee_Decref(result);
	goto err;
err_modulepath_inputstream:
	Dee_Decref(input_stream);
/*err_modulepath:*/
	Dee_Decref(module_path_ob);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenSourceFileString(/*utf-8*/ char const *__restrict source_pathname,
                               size_t source_pathsize,
                               /*utf-8*/ char const *module_name,
                               size_t module_namesize,
                               struct compiler_options *options,
                               bool throw_error) {
	DREF DeeModuleObject *result;
	DREF DeeObject *module_name_ob = NULL;
	DREF DeeObject *source_pathname_ob;
	source_pathname_ob = DeeString_NewUtf8(source_pathname,
	                                       source_pathsize,
	                                       STRING_ERROR_FSTRICT);
	if unlikely(!source_pathname_ob)
		goto err;
	if (module_namesize) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeModule_OpenSourceFile(source_pathname_ob,
	                                  module_name_ob,
	                                  options,
	                                  throw_error);
	Dee_XDecref(module_name_ob);
	Dee_Decref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_Decref(source_pathname_ob);
err:
	return NULL;
}


/* Very similar to `DeeModule_OpenSourceMemory()', and used to implement it,
 * however source data is made available using a stream object derived
 * from `File from deemon' */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenSourceStream(/*File*/ DeeObject *source_stream,
                           int start_line, int start_col,
                           struct compiler_options *options,
                           /*String*/ DeeObject *source_pathname,
                           /*String*/ DeeObject *module_name) {
	DREF DeeModuleObject *result;
	int load_error;
	/* Create a new module. */
	if (!module_name) {
		if (source_pathname) {
			char const *name = DeeString_STR(source_pathname);
			size_t size = DeeString_SIZE(source_pathname);
			char const *name_end, *name_start;
			DREF DeeObject *name_object;
			name_end   = name + size;
			name_start = DeeSystem_BaseName(name, size);

			/* Get rid of a file extension in the module name. */
			while (name_end > name_start && name_end[-1] != '.')
				--name_end;
			while (name_end > name_start && name_end[-1] == '.')
				--name_end;
			if (name_end == name_start)
				name_end = name + size;
			name_object = DeeString_NewSized(name_start,
			                                 (size_t)(name_end - name_start));
			if unlikely(!name_object)
				goto err;
			result = DeeModule_New(name_object);
			Dee_Decref(name_object);
		} else {
			result = DeeModule_New(Dee_EmptyString);
		}
	} else {
		DeeModuleObject *existing_module;
		/* Check if the module is already loaded in the global cache. */
		modules_glob_lock_read();
		result = find_glob_module((DeeStringObject *)module_name);
		if (result && Dee_IncrefIfNotZero(result)) {
			modules_glob_lock_endread();
			goto found_existing_module;
		}
		modules_glob_lock_endread();
		/* Create a new module. */
		result = DeeModule_New(module_name);
		/* Add the module to the global module cache. */
set_global_module:
		modules_glob_lock_write();
		existing_module = find_glob_module((DeeStringObject *)module_name);
		if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
			/* The module got created in the mean time. */
			modules_glob_lock_endwrite();
			Dee_Decref(result);
			result = existing_module;
			goto found_existing_module;
		}
		/* Add the module to the global cache. */
		if unlikely(!add_glob_module(result)) {
			modules_glob_lock_endwrite();
			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_global_module;
			goto err_r;
		}
		modules_glob_lock_endwrite();
	}
found_existing_module:
	load_error = DeeModule_BeginLoading(result);
	if (load_error != 0)
		goto done;
	/* If the input stream hasn't been opened yet, open it now. */
	load_error = DeeModule_LoadSourceStreamEx(result,
	                                          source_stream,
	                                          start_line,
	                                          start_col,
	                                          options,
	                                          (DeeStringObject *)source_pathname);
	/* Depending on a load error having occurred, either signify
	 * that the module has been loaded, or failed to be loaded. */
	if unlikely(load_error) {
		DeeModule_FailLoading(result);
		Dee_Clear(result);
	} else {
		DeeModule_DoneLoading(result);
	}
done:
	return result;
err_r:
	Dee_Decref(result);
err:
	result = NULL;
	goto done;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenSourceStreamString(/*File*/ DeeObject *source_stream,
                                 int start_line, int start_col,
                                 struct compiler_options *options,
                                 /*utf-8*/ char const *source_pathname,
                                 size_t source_pathsize,
                                 /*utf-8*/ char const *module_name,
                                 size_t module_namesize) {
	DREF DeeModuleObject *result;
	DREF DeeObject *module_name_ob     = NULL;
	DREF DeeObject *source_pathname_ob = NULL;
	if (source_pathname) {
		source_pathname_ob = DeeString_NewUtf8(source_pathname,
		                                       source_pathsize,
		                                       STRING_ERROR_FSTRICT);
		if unlikely(!source_pathname_ob)
			goto err;
	}
	if (module_name) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeModule_OpenSourceStream(source_stream,
	                                    start_line,
	                                    start_col,
	                                    options,
	                                    source_pathname_ob,
	                                    module_name_ob);
	Dee_XDecref(module_name_ob);
	Dee_XDecref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_XDecref(source_pathname_ob);
err:
	return NULL;
}


/* Construct a module from a memory source-code blob.
 * NOTE: Unlike `DeeModule_OpenSourceFile()', this function will not bind `source_pathname'
 *       to the returned module, meaning that the module object returned will be entirely
 *       anonymous, except for when `module_name' was passed as non-NULL, in which case
 *       the returned module will be made available as a global import with that same name,
 *       and be available for later addressing using `DeeModule_OpenGlobal()'
 * @param: source_pathname: The filename of the source file from which data (supposedly) originates.
 *                          Used by `#include' directives, as well as `__FILE__' and ddi information.
 *                          When NULL, an empty string is used internally, which results in the current
 *                          directory being used as base for relative imports.
 * @param: module_name:     When non-NULL, use this as the module's actual name.
 *                          Also: register the module as a global module.
 * @param: data:            A pointer to the raw source-code that should be parsed as
 *                          the deemon source for the module.
 * @param: data_size:       The size of the `data' blob (in characters)
 * @param: start_line:      The starting line number of the data blob (zero-based)
 * @param: start_col:       The starting column offset of the data blob (zero-based)
 * @param: options:         An optional set of extended compiler options. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenSourceMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                           int start_line, int start_col,
                           struct compiler_options *options,
                           /*String*/ DeeObject *source_pathname,
                           /*String*/ DeeObject *module_name) {
	DREF DeeObject *source_stream;
	DREF DeeModuleObject *result;
	source_stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!source_stream)
		goto err;
	result = DeeModule_OpenSourceStream(source_stream,
	                                    start_line,
	                                    start_col,
	                                    options,
	                                    source_pathname,
	                                    module_name);
	DeeFile_ReleaseMemory(source_stream);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenSourceMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                 int start_line, int start_col, struct compiler_options *options,
                                 /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                 /*utf-8*/ char const *module_name, size_t module_namesize) {
	DREF DeeModuleObject *result;
	DREF DeeObject *module_name_ob     = NULL;
	DREF DeeObject *source_pathname_ob = NULL;
	if (source_pathsize) {
		source_pathname_ob = DeeString_NewUtf8(source_pathname,
		                                       source_pathsize,
		                                       STRING_ERROR_FSTRICT);
		if unlikely(!source_pathname_ob)
			goto err;
	}
	if (module_namesize) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeModule_OpenSourceMemory(data,
	                                    data_size,
	                                    start_line,
	                                    start_col,
	                                    options,
	                                    source_pathname_ob,
	                                    module_name_ob);
	Dee_XDecref(module_name_ob);
	Dee_XDecref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_XDecref(source_pathname_ob);
err:
	return NULL;
}



PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_NewString(/*utf-8*/ char const *__restrict name, size_t namelen) {
	DREF DeeObject *name_object;
	DREF DeeModuleObject *result;
	name_object = DeeString_NewUtf8(name,
	                                namelen,
	                                STRING_ERROR_FSTRICT);
	if unlikely(!name_object)
		goto err;
	result = DeeModule_New(name_object);
	Dee_Decref(name_object);
	return result;
err:
	return NULL;
}

/* Create a new module object that has yet to be initialized or loaded. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_New(/*String*/ DeeObject *__restrict name) {
	DREF DeeModuleObject *result;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	result = DeeGCObject_CALLOC(DeeModuleObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeModule_Type);
	result->mo_name    = (DeeStringObject *)name;
	result->mo_bucketv = empty_module_buckets;
	Dee_atomic_rwlock_cinit(&result->mo_lock);
	Dee_Incref(name);
	weakref_support_init(result);
	return DeeGC_TRACK(DeeModuleObject, result);
err:
	return NULL;
}


PRIVATE ATTR_COLD int DCALL
err_invalid_module_name_s(char const *__restrict module_name, size_t module_namesize) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "%$q is not a valid module name",
	                       module_namesize, module_name);
}

PRIVATE ATTR_COLD int DCALL
err_module_not_found(DeeObject *__restrict module_name) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "Module %r could not be found",
	                       module_name);
}


#if 0
#define IS_VALID_MODULE_CHARACTER(ch)                \
	(!((ch) == '/' || (ch) == '\\' || (ch) == '|' || \
	   (ch) == '&' || (ch) == '~' || (ch) == '%' ||  \
	   (ch) == '$' || (ch) == '?' || (ch) == '!' ||  \
	   (ch) == '*' || (ch) == '\'' || (ch) == '\"'))
#else
#define IS_VALID_MODULE_CHARACTER(ch)                                      \
	((DeeUni_Flags(ch) &                                                   \
	  (UNICODE_ISALPHA | UNICODE_ISLOWER | UNICODE_ISUPPER | UNICODE_ISTITLE | \
	   UNICODE_ISDIGIT | UNICODE_ISSYMSTRT | UNICODE_ISSYMCONT)) ||         \
	 ((ch) == '-' || (ch) == '=' || (ch) == ',' || (ch) == '(' ||          \
	  (ch) == ')' || (ch) == '[' || (ch) == ']' || (ch) == '{' ||          \
	  (ch) == '}' || (ch) == '<' || (ch) == '>' || (ch) == '+'))
#endif

#define Dee_MODULE_OPENINPATH_FNORMAL      0x0000 /* Normal flags */
#define Dee_MODULE_OPENINPATH_FRELMODULE   0x0001 /* The module name is relative */
#define Dee_MODULE_OPENINPATH_FTHROWERROR  0x0002 /* Throw an error if the module isn't found. */


#define SHEXT DeeSystem_SOEXT
#define SHLEN COMPILER_STRLEN(DeeSystem_SOEXT)


PRIVATE WUNUSED DREF DeeModuleObject *DCALL
DeeModule_OpenInPathAbs(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                        /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                        DeeObject *module_global_name,
                        struct compiler_options *options,
                        unsigned int mode) {
	DREF DeeStringObject *module_name_ob;
	DREF DeeStringObject *module_path_ob;
	DREF DeeModuleObject *result;
	char *buf, *dst;
	char const *module_name_start;
	size_t i, len;
	Dee_hash_t hash;
	Dee_DPRINTF("[RT] Searching for %s%k in %$q as %$q\n",
	            module_global_name ? "global module " : STR_module,
	            module_global_name ? module_global_name : Dee_EmptyString,
	            module_pathsize, module_path,
	            module_namesize, module_name);
	{
		size_t buf_alloc;
#if !defined(CONFIG_NO_DEC) && !defined(CONFIG_NO_DEX)
		buf_alloc = module_pathsize + 1 + module_namesize + MAX_C((size_t)5, (size_t)SHLEN) + 1;
#elif !defined(CONFIG_NO_DEC)
		buf_alloc = module_pathsize + 1 + module_namesize + 5 + 1;
#elif !defined(CONFIG_NO_DEX)
		buf_alloc = module_pathsize + 1 + module_namesize + MAX_C((size_t)4, (size_t)SHLEN) + 1;
#else /* ... */
		buf_alloc = module_pathsize + 1 + module_namesize + 4 + 1;
#endif /* !... */
		buf = (char *)Dee_Mallocac(buf_alloc, sizeof(char));
	}
	if unlikely(!buf)
		goto err;
	dst = buf;
	for (i = 0; i < module_pathsize;) {
		char ch = module_path[i++];
		if (!ISSEP(ch)) {
			*dst++ = ch;
			continue;
		}
		while (i < module_pathsize && ISSEP(module_path[i]))
			++i;
		if (dst >= buf + 1 && dst[-1] == '.') {
			/* Skip self-directory references. */
			if (dst == buf + 1 || dst[-2] == SEP) {
				--dst;
				continue;
			}
			if (dst >= buf + 2 && dst[-2] == '.' &&
			    (dst == buf + 2 || dst[-3] == SEP)) {
				/* Skip parent-directory references. */
				dst -= 3;
				while (dst > buf && dst[-1] != SEP)
					--dst;
				if (dst > buf)
					--dst;
				continue;
			}
		}
		*dst++ = SEP;
	}
	if (dst > buf && dst[-1] != SEP)
		*dst++ = SEP;
	/* Step #1: Check for a cached variant of a user-script. */
	module_name_start = module_name;
	for (i = 0; i < module_namesize; ++i) {
		char ch = module_name[i];
		if (ch == '.') {
			if unlikely(module_name_start == module_name + i)
				goto err_bad_module_name; /* Don't allow multiple consecutive dots here! */
			ch                = SEP;
			module_name_start = module_name + i + 1;
		} else if (!IS_VALID_MODULE_CHARACTER(ch)) {
err_bad_module_name:
			err_invalid_module_name_s(module_name, module_namesize);
			goto err_buf;
		}
		dst[i] = ch;
	}
	dst += (size_t)(module_name_start - module_name);
	module_namesize -= (size_t)(module_name_start - module_name);
	module_name = module_name_start;

	dst[module_namesize + 0] = '.';
	dst[module_namesize + 1] = 'd';
	dst[module_namesize + 2] = 'e';
	dst[module_namesize + 3] = 'e';
	dst[module_namesize + 4] = '\0';
	len  = (size_t)(dst - buf) + module_namesize + 4;
	hash = fs_hashutf8(buf, len);
again_search_fs_modules:

	/* Search for modules that have already been cached. */
	modules_lock_read();
	if (modules_a) {
		LIST_FOREACH (result, &modules_v[hash % modules_a], mo_link) {
			char const *utf8_path;
			if (fs_hashmodpath(result) != hash)
				continue;
			utf8_path = DeeString_TryAsUtf8((DeeObject *)result->mo_path);
			if unlikely(!utf8_path) {
				if (!Dee_IncrefIfNotZero(result))
					break;
				modules_lock_endread();
				utf8_path = DeeString_AsUtf8((DeeObject *)result->mo_path);
				if unlikely(!utf8_path)
					goto err_buf_r;
				if (WSTR_LENGTH(utf8_path) == len &&
				    /* TO-DO: Support for mixed LATIN-1/UTF-8 strings */
				    /* TO-DO: UTF-8 case compare! */
				    fs_bcmp(utf8_path, buf, len * sizeof(char)) == 0) {
					goto got_result_set_global;
				}
				Dee_Decref(result);
				goto again_search_fs_modules;
			}
			if (WSTR_LENGTH(utf8_path) != len)
				continue;
			/* TO-DO: Support for mixed LATIN-1/UTF-8 strings */
			/* TO-DO: UTF-8 case compare! */
			if (fs_bcmp(utf8_path, buf, len * sizeof(char)) != 0)
				continue;
			/* Found it! */
			if (!Dee_IncrefIfNotZero(result))
				break;
			modules_lock_endread();
got_result_set_global:
			if (module_global_name && likely(!LIST_ISBOUND(result, mo_globlink))) {
				DeeModuleObject *existing_module;
				/* Cache the module as global (if it wasn't already) */
again_find_existing_global_module:
				modules_glob_lock_write();
				COMPILER_READ_BARRIER();
				if likely(!LIST_ISBOUND(result, mo_globlink)) {
					/* TO-DO: Must change `result->mo_name' to `module_global_name'
					 * ${LIBPATH}/foo/bar.dee:
					 * >> global helper = import(".bar");
					 * ${LIBPATH}/foo/baz.dee:
					 * >> print "Hi, I'm a helper module";
					 * main.dee:
					 * >> local a = import("foo.bar");
					 * >> print a.__name__;            // "foo.bar"
					 * >> print a.helper.__name__;     // "baz"
					 * >> print a.helper.__isglobal__; // false
					 * >> assert a.helper === import("foo.baz");
					 * >> // The re-import as global must changed the name to "foo.baz".
					 * >> // If we don't do this, then "foo.baz" will (incorrectly) become
					 * >> // available as `import("baz")', even though it's file location
					 * >> // in relation to the system library path would require it to be
					 * >> // addressed as "foo.baz".
					 * >> print a.helper.__name__;     // "foo.baz" (currently, and wrongly still "baz")
					 * >> print a.helper.__isglobal__; // true
					 * NOTE: This requires some changes to the runtime, as `mo_name' is
					 *       currently assumed to be `[const]', when that must to be changed to:
					 *       [const_if(mo_globlink != NULL)]
					 *       [lock_if(mo_globlink == NULL, INTERNAL(modules_glob_lock))]
					 */

					existing_module = find_glob_module(result->mo_name);
					if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
						modules_glob_lock_endwrite();
						Dee_Decref_likely(result);
						result = existing_module;
						goto got_result;
					}
					if (!add_glob_module(result)) {
						modules_glob_lock_endwrite();
						if (Dee_CollectMemory(1))
							goto again_find_existing_global_module;
						goto err_buf_r;
					}
				}
				modules_glob_lock_endwrite();
			}
			goto got_result;
		}
	}
	modules_lock_endread();
	if (ITER_ISOK(module_global_name)) {
		module_name_ob = (DREF DeeStringObject *)module_global_name;
		Dee_Incref(module_global_name);
	} else {
		module_name_ob = (DREF DeeStringObject *)DeeString_NewUtf8(module_name,
		                                                           module_namesize,
		                                                           STRING_ERROR_FIGNORE);
		if unlikely(!module_name_ob)
			goto err_buf;
	}

	/* The module hasn't been loaded, yet.
	 * Try to load it now! */
#ifndef CONFIG_NO_DEC
	if (!options || !(options->co_decloader & Dee_DEC_FDISABLE)) {
		/* Step #1: Try to load the module from a pre-compiled .dec file.
		 * By checking this before searching for trying to load a DEX extension,
		 * we allow the user to override extensions with user-code scripts by
		 * simply generating a dec file using `deemon -c', without having to
		 * actually delete the dex library. */
		memmoveupc(dst + 1,
		           dst,
		           module_namesize + 5,
		           sizeof(char));
		dst[0] = '.';
		ASSERT(dst[module_namesize + 1] == '.');
		ASSERT(dst[module_namesize + 2] == 'd');
		ASSERT(dst[module_namesize + 3] == 'e');
		dst[module_namesize + 4] = 'c';
		ASSERT(dst[module_namesize + 5] == '\0');
		{
			DREF DeeObject *dec_stream;
			dec_stream = DeeFile_OpenString(buf, OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
			memmovedownc(dst,
			             dst + 1,
			             module_namesize + 5,
			             sizeof(char));
			ASSERT(dst[module_namesize + 0] == '.');
			ASSERT(dst[module_namesize + 1] == 'd');
			ASSERT(dst[module_namesize + 2] == 'e');
			dst[module_namesize + 3] = 'e';
			ASSERT(dst[module_namesize + 4] == '\0');
			if (dec_stream != ITER_DONE) {
				int error;
				DeeModuleObject *existing_module;
				/* The compiled file _does_ exist! */
				if unlikely(!dec_stream)
					goto err_buf_module_name;
				module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FIGNORE);
				if unlikely(!module_path_ob) {
err_buf_name_dec_stream:
					Dee_Decref_likely(dec_stream);
					goto err_buf_module_name;
				}
				result = DeeModule_New(Dee_AsObject(module_name_ob));
				if unlikely(!result) {
/*err_buf_name_dec_stream_path:*/
					Dee_Decref_likely(module_path_ob);
					goto err_buf_name_dec_stream;
				}
				Dee_Decref_unlikely(module_name_ob);
				result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef DeeSystem_HAVE_FS_ICASE
				result->mo_pathihash = hash;
#else /* DeeSystem_HAVE_FS_ICASE */
				ASSERT(DeeString_Hash(module_path_ob) == hash);
				((DeeStringObject *)module_path_ob)->s_hash = hash;
#endif /* !DeeSystem_HAVE_FS_ICASE */

				result->mo_flags |= Dee_MODULE_FLOADING;
				COMPILER_WRITE_BARRIER();
				/* Cache the new module as part of the filesystem
				 * module cache, as well as the global module cache. */
				if (module_global_name) {
set_dec_file_module_global:
#ifndef CONFIG_NO_THREADS
					modules_lock_write();
					if (!modules_glob_lock_trywrite()) {
						modules_lock_endwrite();
						modules_glob_lock_write();
						if (!modules_lock_trywrite()) {
							modules_glob_lock_endwrite();
							goto set_dec_file_module_global;
						}
					}
#endif /* !CONFIG_NO_THREADS */
					existing_module = find_file_module(module_path_ob, hash);
					if likely(!existing_module)
						existing_module = find_glob_module(module_name_ob);
					if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
						modules_glob_lock_endwrite();
						modules_lock_endwrite();
						Dee_DecrefDokill(result);
						Dee_Decref_likely(dec_stream);
						result = existing_module;
						goto try_load_module_after_dec_failure;
					}
					/* Add the module to the file-cache. */
					if ((modules_c >= modules_a && !rehash_file_modules()) ||
					    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
						modules_lock_endwrite();
						/* Try to collect some memory, then try again. */
						if (Dee_CollectMemory(1))
							goto set_dec_file_module_global;
						Dee_Decref_likely(dec_stream);
						goto err_buf_r;
					}
					add_glob_module(result);
					add_file_module(result);
					modules_glob_lock_endwrite();
					modules_lock_endwrite();
				} else {
set_dec_file_module:
					modules_lock_write();
					existing_module = find_file_module(module_path_ob, hash);
					if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
						modules_lock_endwrite();
						Dee_DecrefDokill(result);
						Dee_Decref_likely(dec_stream);
						result = existing_module;
try_load_module_after_dec_failure:
						if (DeeModule_BeginLoading(result) == 0)
							goto load_module_after_dec_failure;
						goto got_result;
					}
					/* Add the module to the file-cache. */
					if unlikely(!add_file_module(result)) {
						modules_lock_endwrite();
						/* Try to collect some memory, then try again. */
						if (Dee_CollectMemory(1))
							goto set_dec_file_module;
						Dee_Decref_likely(dec_stream);
						goto err_buf_r;
					}
					modules_lock_endwrite();
				}
				error = DeeModule_OpenDec(result, dec_stream, options);
				Dee_Decref_likely(dec_stream);
				if likely(error == 0) {
					/* Successfully loaded the DEC file. */
					DeeModule_DoneLoading(result);
					goto got_result;
				}
				if unlikely(error < 0) {
					/* Hard error. */
					DeeModule_FailLoading(result);
					goto err_buf_r;
				}
load_module_after_dec_failure:
				/* Must try to load the module from its source file. */
				dec_stream = DeeFile_OpenString(buf, OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
				if unlikely(!ITER_ISOK(dec_stream)) {
					DeeModule_FailLoading(result);
					if (dec_stream == ITER_DONE) {
						DeeError_Throwf(&DeeError_FileNotFound,
						                "Missing source file `%s' when the associated dec file does found",
						                buf);
					}
					goto err_buf_r;
				}
				error = DeeModule_LoadSourceStreamEx(result,
				                                     dec_stream,
				                                     0,
				                                     0,
				                                     options,
				                                     result->mo_path);
				Dee_Decref_likely(dec_stream);
				if unlikely(error) {
					DeeModule_FailLoading(result);
					goto err_buf_r;
				}
				DeeModule_DoneLoading(result);
				goto got_result;
			}
		}
	}
#endif /* !CONFIG_NO_DEC */
#ifdef CONFIG_NO_DEX
	module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FSTRICT);
	if unlikely(!module_path_ob)
		goto err_buf;
#ifndef DeeSystem_HAVE_FS_ICASE
	ASSERT(fs_hashutf8(buf, len) == hash);
	((DeeStringObject *)module_path_ob)->s_hash = hash;
#endif /* !DeeSystem_HAVE_FS_ICASE */
#else /* CONFIG_NO_DEX */
	/* Try to load the module from a DEX extension. */
	ASSERT(dst[module_namesize + 0] == '.');
	ASSERT(dst[module_namesize + 1] == 'd');
	ASSERT(dst[module_namesize + 2] == 'e');
	ASSERT(dst[module_namesize + 3] == 'e');
	ASSERT(dst[module_namesize + 4] == '\0');
	{
		void *dex_handle;
		if (SHLEN >= 0 && SHEXT[0] != '.')
			dst[module_namesize + 0] = SHEXT[0];
		if (SHLEN >= 1 && SHEXT[1] != 'd')
			dst[module_namesize + 1] = SHEXT[1];
		if (SHLEN >= 2 && SHEXT[2] != 'e')
			dst[module_namesize + 2] = SHEXT[2];
		if (SHLEN >= 3 && SHEXT[3] != 'e')
			dst[module_namesize + 3] = SHEXT[3];
		__STATIC_IF (SHLEN > 4) {
			memcpyc(&dst[module_namesize + 4],
			        SHEXT + 4,
			        (SHLEN - 4) + 1,
			        sizeof(char));
		}
		dex_handle = DeeSystem_DlOpenString(buf);
		if (dex_handle == DeeSystem_DlOpen_FAILED) {
			if (SHLEN >= 0 && SHEXT[0] != '.')
				dst[module_namesize + 0] = '.';
			if (SHLEN >= 1 && SHEXT[1] != 'd')
				dst[module_namesize + 1] = 'd';
			if (SHLEN >= 2 && SHEXT[2] != 'e')
				dst[module_namesize + 2] = 'e';
			if (SHLEN >= 3 && SHEXT[3] != 'e')
				dst[module_namesize + 3] = 'e';
			__STATIC_IF (SHLEN > 4) {
				dst[module_namesize + 4] = '\0';
			}
			module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FSTRICT);
			if unlikely(!module_path_ob)
				goto err_buf;
#ifndef DeeSystem_HAVE_FS_ICASE
			ASSERT(fs_hashutf8(buf, len) == hash);
			((DeeStringObject *)module_path_ob)->s_hash = hash;
#endif /* !DeeSystem_HAVE_FS_ICASE */
		} else {
			int error;
			DeeModuleObject *existing_module;
			module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf,
			                                                           len - 4 + SHLEN,
			                                                           STRING_ERROR_FSTRICT);
			if unlikely(!module_path_ob) {
				DeeSystem_DlClose(dex_handle);
				goto err_buf_module_name;
			}
			result = (DREF DeeModuleObject *)DeeDex_New(Dee_AsObject(module_name_ob));
			if unlikely(!result) {
				DeeSystem_DlClose(dex_handle);
				goto err_buf_module_name_path;
			}
			Dee_Decref_unlikely(module_name_ob);
			result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef DeeSystem_HAVE_FS_ICASE
			result->mo_pathihash = hash;
#else /* DeeSystem_HAVE_FS_ICASE */
			/* Load the updated path hash (and also force the hash to be pre-cached) */
			hash = fs_hashobj(module_path_ob);
#endif /* !DeeSystem_HAVE_FS_ICASE */
			result->mo_flags |= Dee_MODULE_FLOADING;
			COMPILER_WRITE_BARRIER();

			/* Register the new dex module globally. */
			if (module_global_name) {
set_dex_file_module_global:
#ifndef CONFIG_NO_THREADS
				modules_lock_write();
				if (!modules_glob_lock_trywrite()) {
					modules_lock_endwrite();
					modules_glob_lock_write();
					if (!modules_lock_trywrite()) {
						modules_glob_lock_endwrite();
						goto set_dex_file_module_global;
					}
				}
#endif /* !CONFIG_NO_THREADS */
				existing_module = find_file_module(module_path_ob, hash);
				if likely(!existing_module)
					existing_module = find_glob_module(module_name_ob);
				if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
					modules_glob_lock_endwrite();
					modules_lock_endwrite();
					Dee_DecrefDokill(result);
					DeeSystem_DlClose(dex_handle);
					result = existing_module;
					goto try_load_module_after_dex_failure;
				}
				/* Add the module to the file-cache. */
				if ((modules_c >= modules_a && !rehash_file_modules()) ||
				    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
					modules_lock_endwrite();
					/* Try to collect some memory, then try again. */
					if (Dee_CollectMemory(1))
						goto set_dex_file_module_global;
					DeeSystem_DlClose(dex_handle);
					goto err_buf_r;
				}
				add_glob_module(result);
				add_file_module(result);
				modules_glob_lock_endwrite();
				modules_lock_endwrite();
			} else {
set_dex_file_module:
				modules_lock_write();
				existing_module = find_file_module(module_path_ob, hash);
				if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
					modules_lock_endwrite();
					Dee_DecrefDokill(result);
					DeeSystem_DlClose(dex_handle);
					result = existing_module;
try_load_module_after_dex_failure:
					if (DeeModule_BeginLoading(result) == 0)
						goto load_module_after_dex_failure;
					goto got_result;
				}
				/* Add the module to the file-cache. */
				if unlikely(!add_file_module(result)) {
					modules_lock_endwrite();
					/* Try to collect some memory, then try again. */
					if (Dee_CollectMemory(1))
						goto set_dex_file_module;
					DeeSystem_DlClose(dex_handle);
					goto err_buf_r;
				}
				modules_lock_endwrite();
			}
load_module_after_dex_failure:
			error = dex_load_handle((DeeDexObject *)result,
			                        (void *)dex_handle,
			                        (DeeObject *)result->mo_path);
			if unlikely(error) {
				DeeModule_FailLoading(result);
				goto err_buf_r;
			}
			DeeModule_DoneLoading(result);
			goto got_result;
		}
	}
#endif /* !CONFIG_NO_DEX */
	ASSERT(module_path_ob != NULL);
	/* Load a regular, old source file. */
	{
		DeeModuleObject *existing_module;
		DREF DeeObject *source_stream;
		int error;
		source_stream = DeeFile_Open(Dee_AsObject(module_path_ob), OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
		if unlikely(!ITER_ISOK(source_stream)) {
			Dee_Decref(module_name_ob);
			if (source_stream == ITER_DONE) {
				/* The source file doesn't exist! */
				if (!(mode & Dee_MODULE_OPENINPATH_FTHROWERROR)) {
					Dee_Decref_likely(module_path_ob);
					result = (DREF DeeModuleObject *)ITER_DONE;
					goto got_result;
				}
				err_file_not_found(Dee_AsObject(module_path_ob));
			}
			goto err_buf_module_path;
		}
		result = DeeModule_New(Dee_AsObject(module_name_ob));
		if unlikely(!result) {
/*err_buf_name_source_stream:*/
			Dee_Decref_likely(source_stream);
			goto err_buf_module_name_path;
		}
		Dee_Decref_unlikely(module_name_ob);
		result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef DeeSystem_HAVE_FS_ICASE
		result->mo_pathihash = hash;
#else /* DeeSystem_HAVE_FS_ICASE */
		ASSERT(DeeString_HASHOK(module_path_ob));
		ASSERT(DeeString_HASH(module_path_ob) == hash);
#endif /* !DeeSystem_HAVE_FS_ICASE */
		result->mo_flags |= Dee_MODULE_FLOADING;
		COMPILER_WRITE_BARRIER();

		/* Register the new dex module globally. */
		if (module_global_name) {
set_src_file_module_global:
#ifndef CONFIG_NO_THREADS
			modules_lock_write();
			if (!modules_glob_lock_trywrite()) {
				modules_lock_endwrite();
				modules_glob_lock_write();
				if (!modules_lock_trywrite()) {
					modules_glob_lock_endwrite();
					goto set_src_file_module_global;
				}
			}
#endif /* !CONFIG_NO_THREADS */
			existing_module = find_file_module(module_path_ob, hash);
			if likely(!existing_module)
				existing_module = find_glob_module(module_name_ob);
			if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
				modules_glob_lock_endwrite();
				modules_lock_endwrite();
				Dee_DecrefDokill(result);
				Dee_Decref_likely(source_stream);
				result = existing_module;
				goto try_load_module_after_src_failure;
			}
			/* Add the module to the file-cache. */
			if ((modules_c >= modules_a && !rehash_file_modules()) ||
			    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
				modules_lock_endwrite();
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(1))
					goto set_src_file_module_global;
				Dee_Decref_likely(source_stream);
				goto err_buf_r;
			}
			add_glob_module(result);
			add_file_module(result);
			modules_glob_lock_endwrite();
			modules_lock_endwrite();
		} else {
set_src_file_module:
			modules_lock_write();
			existing_module = find_file_module(module_path_ob, hash);
			if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
				modules_lock_endwrite();
				Dee_DecrefDokill(result);
				Dee_Decref_likely(source_stream);
				result = existing_module;
try_load_module_after_src_failure:
				if (DeeModule_BeginLoading(result) == 0)
					goto load_module_after_src_failure;
				goto got_result;
			}
			/* Add the module to the file-cache. */
			if unlikely(!add_file_module(result)) {
				modules_lock_endwrite();
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(1))
					goto set_src_file_module;
				Dee_Decref_likely(source_stream);
				goto err_buf_r;
			}
			modules_lock_endwrite();
		}
load_module_after_src_failure:
		error = DeeModule_LoadSourceStreamEx(result,
		                                     source_stream,
		                                     0,
		                                     0,
		                                     options,
		                                     result->mo_path);
		Dee_Decref_likely(source_stream);
		if unlikely(error) {
			DeeModule_FailLoading(result);
			goto err_buf_r;
		}
		DeeModule_DoneLoading(result);
		/*goto got_result;*/
	}
got_result:
	Dee_Freea(buf);
	return result;
/*
err_buf_module_name_r:
	Dee_Decref_unlikely(result);*/
err_buf_module_name_path:
	Dee_Decref_likely(module_path_ob);
err_buf_module_name:
	Dee_Decref_likely(module_name_ob);
	goto err_buf;
err_buf_module_path:
	Dee_Decref_likely(module_path_ob);
	goto err_buf;
err_buf_r:
	Dee_Decref_unlikely(result);
err_buf:
	Dee_Freea(buf);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeModuleObject *DCALL
DeeModule_SubOpenInPathAbs(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                           /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                           DeeObject *module_global_name,
                           struct compiler_options *options,
                           unsigned int mode) {
	size_t additional_count;
	/* Walk up the directory path for upwards references in the relative path. */
	additional_count = 0;
	for (;;) {
		while (module_pathsize && ISSEP(module_path[module_pathsize - 1]))
			--module_pathsize;
		if (module_pathsize >= 1 &&
		    module_path[module_pathsize - 1] == '.') {
			if (module_pathsize == 1 || ISSEP(module_path[module_pathsize - 2])) {
				/* Current-directory reference. */
				module_pathsize -= 2;
				continue;
			}
			if (module_pathsize >= 2 &&
			    module_path[module_pathsize - 2] == '.' &&
			    (module_pathsize == 2 || ISSEP(module_path[module_pathsize - 3]))) {
				/* Parent-directory reference. */
				++additional_count;
				module_pathsize -= 3;
				continue;
			}
		}
		if (additional_count) {
			--additional_count;
		} else {
			if (!(mode & Dee_MODULE_OPENINPATH_FRELMODULE))
				break;
			++module_name;
			--module_namesize;
			if (!module_namesize || *module_name != '.')
				break;
		}
		while (module_pathsize && !ISSEP(module_path[module_pathsize - 1]))
			--module_pathsize;
	}
	return DeeModule_OpenInPathAbs(module_path, module_pathsize,
	                               module_name, module_namesize,
	                               module_global_name,
	                               options,
	                               mode);
}


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
 * >> TRY_LOAD_DEX_LIBRARY(joinpath(module_path, module_name + DeeSystem_SOEXT));
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
PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeModuleObject *DCALL
DeeModule_OpenInPath(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                     /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                     /*String*/ DeeObject *module_global_name,
                     struct compiler_options *options,
                     unsigned int mode) {
	if unlikely(!DeeSystem_IsAbsN(module_path, module_pathsize)) {
		/* Must make the given module path absolute. */
		DREF DeeStringObject *abs_path; /*utf-8*/
		char const *abs_utf8;
		DREF DeeModuleObject *result;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if (DeeSystem_PrintPwd(&printer, true) < 0)
			goto err_printer;
		if (unicode_printer_print(&printer, module_path, module_pathsize) < 0)
			goto err_printer;
		abs_path = (DREF DeeStringObject *)unicode_printer_pack(&printer);
		if unlikely(!abs_path)
			goto err;
		abs_utf8 = DeeString_AsUtf8((DeeObject *)abs_path);
		if unlikely(!abs_utf8)
			goto err_abs_path;
		result = DeeModule_SubOpenInPathAbs(abs_utf8,
		                                    WSTR_LENGTH(abs_utf8),
		                                    module_name, module_namesize,
		                                    module_global_name,
		                                    options,
		                                    mode);
		Dee_Decref(abs_path);
		return result;
err_abs_path:
		Dee_Decref(abs_path);
		goto err;
err_printer:
		unicode_printer_fini(&printer);
		goto err;
	}
	return DeeModule_SubOpenInPathAbs(module_path, module_pathsize,
	                                  module_name, module_namesize,
	                                  module_global_name,
	                                  options, mode);
err:
	return NULL;
}


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
 *     >> import Object from deemon;
 *     >> import mymodule;
 *     >> import MyModule;
 *     >> assert mymodule === MyModule;
 *     Note that this code example must only work when the module
 *     system is case-insensitive as well.
 * @param: throw_error: When true, throw an error if the module couldn't be
 *                      found and return `NULL', otherwise return `ITER_DONE'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenGlobal(/*String*/ DeeObject *__restrict module_name,
                     struct compiler_options *options, bool throw_error) {
	DREF DeeObject *path;
	DREF DeeModuleObject *result;
	DeeListObject *paths;
	size_t i;
	/*utf-8*/ char const *module_namestr;
	size_t module_namelen;
	ASSERT_OBJECT_TYPE_EXACT(module_name, &DeeString_Type);

	/* First off: Check if this is a request for the builtin `deemon' module.
	 * NOTE: This check is always done in case-sensitive mode! */
	if (DeeString_SIZE(module_name) == 6 &&
	    DeeString_Hash(module_name) == HASHOF_str_deemon &&
	    bcmpc(DeeString_STR(module_name), STR_deemon, 6, sizeof(char)) == 0) {
		/* Yes, it is. */
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		goto done;
	}

	/* Search for a cache entry for this module in the global module cache. */
	modules_glob_lock_read();
	result = find_glob_module((DeeStringObject *)module_name);
	if (result && Dee_IncrefIfNotZero(result)) {
		modules_glob_lock_endread();
		goto done;
	}
	modules_glob_lock_endread();

	module_namestr = DeeString_AsUtf8(module_name);
	if unlikely(!module_namestr)
		goto err;
	module_namelen = WSTR_LENGTH(module_namestr);

	/* Default case: Must load a new module. */
	paths = DeeModule_GetPath();
	DeeList_LockRead(paths);
	for (i = 0; i < DeeList_SIZE(paths); ++i) {
		path = DeeList_GET(paths, i);
		Dee_Incref(path);
		DeeList_LockEndRead(paths);
		if (DeeString_Check(path)) {
			/*utf-8*/ char const *path_str;
			path_str = DeeString_AsUtf8(path);
			if unlikely(!path_str)
				goto err_path;
			result = DeeModule_OpenInPath(path_str,
			                              WSTR_LENGTH(path_str),
			                              module_namestr,
			                              module_namelen,
			                              module_name,
			                              options,
			                              Dee_MODULE_OPENINPATH_FNORMAL);
			if (result != (DREF DeeModuleObject *)ITER_DONE)
				goto done_path;
		} else {
			/* `path' isn't a string */
		}
		Dee_Decref(path);
		DeeList_LockRead(paths);
	}
	DeeList_LockEndRead(paths);
	if (!throw_error)
		return (DREF DeeModuleObject *)ITER_DONE;
	err_module_not_found(module_name);
err:
	return NULL;
err_path:
	Dee_Decref(path);
	goto err;
done_path:
	Dee_Decref(path);
done:
	return result;
}

PUBLIC WUNUSED DREF DeeModuleObject *DCALL
DeeModule_OpenGlobalString(/*utf-8*/ char const *__restrict module_name,
                           size_t module_namesize,
                           struct compiler_options *options,
                           bool throw_error) {
	DREF DeeObject *module_name_ob;
	DREF DeeObject *path;
	DREF DeeModuleObject *result;
	DeeListObject *paths;
	size_t i;

	/* First off: Check if this is a request for the builtin `deemon' module.
	 * NOTE: This check is always done in case-sensitive mode! */
	if (module_namesize == 6 &&
	    bcmpc(module_name, STR_deemon, module_namesize, sizeof(char)) == 0) {
		/* Yes, it is. */
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		goto done;
	}

	/* Search for a cache entry for this module in the global module cache. */
	modules_glob_lock_read();
	result = find_glob_module_str(module_name, module_namesize);
	if (result && Dee_IncrefIfNotZero(result)) {
		modules_glob_lock_endread();
		goto done;
	}
	modules_glob_lock_endread();

	/* Construct the module name object. */
	module_name_ob = DeeString_NewUtf8(module_name,
	                                   module_namesize,
	                                   STRING_ERROR_FSTRICT);
	if unlikely(!module_name_ob)
		goto err;

	/* Default case: Must load a new module. */
	paths = DeeModule_GetPath();
	DeeList_LockRead(paths);
	for (i = 0; i < DeeList_SIZE(paths); ++i) {
		path = DeeList_GET(paths, i);
		Dee_Incref(path);
		DeeList_LockEndRead(paths);
		if (DeeString_Check(path)) {
			/*utf-8*/ char const *path_str;
			path_str = DeeString_AsUtf8(path);
			if unlikely(!path_str)
				goto err_path_module_name_ob;
			result = DeeModule_OpenInPath(path_str,
			                              WSTR_LENGTH(path_str),
			                              module_name,
			                              module_namesize,
			                              module_name_ob,
			                              options,
			                              Dee_MODULE_OPENINPATH_FNORMAL);
			if (result != (DREF DeeModuleObject *)ITER_DONE)
				goto done_path;
		} else {
			/* `path' isn't a string */
		}
		Dee_Decref(path);
		DeeList_LockRead(paths);
	}
	DeeList_LockEndRead(paths);
	if (!throw_error) {
		Dee_Decref(module_name_ob);
		return (DREF DeeModuleObject *)ITER_DONE;
	}
	err_module_not_found(module_name_ob);
err_module_name_ob:
	Dee_Decref(module_name_ob);
err:
	return NULL;
err_path_module_name_ob:
	Dee_Decref(path);
	goto err_module_name_ob;
done_path:
	Dee_Decref(path);
	Dee_Decref(module_name_ob);
done:
	return result;
}


/* Same as `DeeModule_OpenGlobal()', but automatically call `DeeModule_RunInit()' on the returned module. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_ImportGlobal(/*String*/ DeeObject *__restrict module_name) {
	DREF DeeModuleObject *result;
	result = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!result)
		goto err;
	if unlikely(DeeModule_RunInit(result) < 0)
		goto err_result;
	return result;
err_result:
	Dee_Decref(result);
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeModuleObject *DCALL
DeeModule_ImportGlobalString(/*utf-8*/ char const *__restrict module_name,
                             size_t module_namesize) {
	DREF DeeModuleObject *result;
	result = DeeModule_OpenGlobalString(module_name, module_namesize, NULL, true);
	if unlikely(!result)
		goto err;
	if unlikely(DeeModule_RunInit(result) < 0)
		goto err_result;
	return result;
err_result:
	Dee_Decref(result);
err:
	return NULL;
}



/* Lookup an external symbol.
 * Convenience function (same as `DeeObject_GetAttr(DeeModule_OpenGlobal(...)+DeeModule_RunInit, ...)') */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExtern(/*String*/ DeeObject *__restrict module_name,
                    /*String*/ DeeObject *__restrict global_name) {
	DREF DeeObject *result;
	DREF DeeModuleObject *extern_module;
	extern_module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_GetAttr(Dee_AsObject(extern_module), global_name);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExternString(/*utf-8*/ char const *__restrict module_name,
                          /*utf-8*/ char const *__restrict global_name) {
	DREF DeeObject *result;
	DREF DeeModuleObject *extern_module;
	extern_module = DeeModule_OpenGlobalString(module_name,
	                                           strlen(module_name),
	                                           NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_GetAttrString(Dee_AsObject(extern_module), global_name);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

/* Helper wrapper for `DeeObject_Call(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExtern(/*String*/ DeeObject *__restrict module_name,
                     /*String*/ DeeObject *__restrict global_name,
                     size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeModuleObject *extern_module;
	extern_module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_CallAttr(Dee_AsObject(extern_module), global_name, argc, argv);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExternString(/*utf-8*/ char const *__restrict module_name,
                           /*utf-8*/ char const *__restrict global_name,
                           size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeModuleObject *extern_module;
	extern_module = DeeModule_OpenGlobalString(module_name,
	                                           strlen(module_name),
	                                           NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_CallAttrString(Dee_AsObject(extern_module), global_name, argc, argv);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

/* Helper wrapper for `DeeObject_Callf(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternStringf(/*utf-8*/ char const *__restrict module_name,
                            /*utf-8*/ char const *__restrict global_name,
                            char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeModule_VCallExternStringf(module_name,
	                                      global_name,
	                                      format,
	                                      args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternStringf(/*utf-8*/ char const *__restrict module_name,
                             /*utf-8*/ char const *__restrict global_name,
                             char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	DREF DeeModuleObject *extern_module;
	extern_module = DeeModule_OpenGlobalString(module_name,
	                                           strlen(module_name),
	                                           NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_VCallAttrStringf(Dee_AsObject(extern_module), global_name, format, args);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternf(/*String*/ DeeObject *__restrict module_name,
                      /*String*/ DeeObject *__restrict global_name,
                      char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeModule_VCallExternf(module_name,
	                                global_name,
	                                format,
	                                args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternf(/*String*/ DeeObject *__restrict module_name,
                       /*String*/ DeeObject *__restrict global_name,
                       char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	DREF DeeModuleObject *extern_module;
	extern_module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_VCallAttrf(Dee_AsObject(extern_module), global_name, format, args);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}




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
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_OpenRelative(/*String*/ DeeObject *__restrict module_name,
                       /*utf-8*/ char const *__restrict module_pathname,
                       size_t module_pathsize,
                       struct compiler_options *options,
                       bool throw_error) {
	/*utf-8*/ char const *module_name_str;
	module_name_str = DeeString_AsUtf8(module_name);
	if unlikely(!module_name_str)
		goto err;
	if (module_name_str[0] != '.')
		return DeeModule_OpenGlobal(module_name, options, throw_error);
	return DeeModule_OpenInPath(module_pathname,
	                            module_pathsize,
	                            module_name_str,
	                            WSTR_LENGTH(module_name_str),
	                            NULL,
	                            options,
	                            throw_error ? (Dee_MODULE_OPENINPATH_FRELMODULE | Dee_MODULE_OPENINPATH_FTHROWERROR)
	                                        : (Dee_MODULE_OPENINPATH_FRELMODULE));
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeModuleObject *DCALL
DeeModule_OpenRelativeString(/*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                             /*utf-8*/ char const *__restrict module_pathname, size_t module_pathsize,
                             struct compiler_options *options, bool throw_error) {
	/* Shouldn't happen: Not actually a relative module name. */
	if (!module_namesize || *module_name != '.')
		return DeeModule_OpenGlobalString(module_name, module_namesize, options, throw_error);
	return DeeModule_OpenInPath(module_pathname,
	                            module_pathsize,
	                            module_name,
	                            module_namesize,
	                            NULL,
	                            options,
	                            throw_error ? (Dee_MODULE_OPENINPATH_FRELMODULE | Dee_MODULE_OPENINPATH_FTHROWERROR)
	                                        : (Dee_MODULE_OPENINPATH_FRELMODULE));
}

/* Import a module, with relative module paths being imported in relation to `basemodule'
 * Not that these functions invoke `DeeModule_RunInit()' on the returned module!*/
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_ImportRel(DeeModuleObject *__restrict basemodule,
                    /*String*/ DeeObject *__restrict module_name) {
	DREF DeeModuleObject *result;
	DeeStringObject *path;
	char const *begin, *end;
	ASSERT_OBJECT_TYPE(basemodule, &DeeModule_Type);
	/* Load the path of the currently executing code (for relative imports). */
	path = basemodule->mo_path;
	if unlikely(!path) {
		result = DeeModule_OpenGlobal(module_name, NULL, true);
	} else {
		ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
		begin = DeeString_AsUtf8(Dee_AsObject(path));
		if unlikely(!begin)
			goto err;
		end = begin + WSTR_LENGTH(begin);

		/* Find the end of the current path. */
		while (begin < end && !ISSEP(end[-1]))
			--end;
		result = DeeModule_OpenRelative(module_name,
		                                begin,
		                                (size_t)(end - begin),
		                                NULL,
		                                true);
	}
	if likely(result) {
		if unlikely(DeeModule_RunInit(result) < 0)
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_ImportRelString(DeeModuleObject *__restrict basemodule,
                          /*utf-8*/ char const *__restrict module_name,
                          size_t module_namesize) {
	DREF DeeModuleObject *result;
	DeeStringObject *path;
	char const *begin, *end;
	ASSERT_OBJECT_TYPE(basemodule, &DeeModule_Type);

	/* Load the path of the currently executing code (for relative imports). */
	path = basemodule->mo_path;
	if unlikely(!path) {
		result = DeeModule_OpenGlobalString(module_name, module_namesize, NULL, true);
	} else {
		ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
		begin = DeeString_AsUtf8(Dee_AsObject(path));
		if unlikely(!begin)
			goto err;
		end = begin + WSTR_LENGTH(begin);

		/* Find the end of the current path. */
		while (begin < end && !ISSEP(end[-1]))
			--end;
		result = DeeModule_OpenRelativeString(module_name,
		                                      module_namesize,
		                                      begin,
		                                      (size_t)(end - begin),
		                                      NULL,
		                                      true);
	}
	if likely(result) {
		if unlikely(DeeModule_RunInit(result) < 0)
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


PRIVATE WUNUSED int DCALL module_rehash_globals(void) {
	size_t i, new_mask = (current_rootscope->rs_bucketm << 1) | 1;
	struct module_symbol *new_vec;
	ASSERT(!(new_mask & (new_mask + 1)));
	new_vec = (struct module_symbol *)Dee_Callocc(new_mask + 1,
	                                              sizeof(struct module_symbol));
	if unlikely(!new_vec)
		goto err;
	for (i = 0; i <= current_rootscope->rs_bucketm; ++i) {
		size_t j, perturb;
		struct module_symbol *item;
		item = &current_rootscope->rs_bucketv[i];
		if (!item->ss_name)
			continue;
		perturb = j = item->ss_hash & new_mask;
		for (;; Dee_MODULE_HASHNX(j, perturb)) {
			struct module_symbol *new_item = &new_vec[j & new_mask];
			if (new_item->ss_name)
				continue;

			/* Copy the old item into this new slot. */
			memcpy(new_item, item, sizeof(struct module_symbol));
			break;
		}
	}

	/* Free the old bucket vector and assign the new one */
	Dee_Free(current_rootscope->rs_bucketv);
	current_rootscope->rs_bucketm = (uint16_t)new_mask;
	current_rootscope->rs_bucketv = new_vec;
	return 0;
err:
	return -1;
}

struct module_import_data {
	DeeModuleObject *mid_self;    /* [1..1] The module into which to import. */
	unsigned int     mid_mode;    /* Import mode (s.a. `DeeExec_RUNMODE_*') */
	uint16_t         mid_globala; /* Allocated vector size of `mid_self->mo_globalv' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
module_import_symbol(void *arg, DeeObject *name, DeeObject *value) {
	struct module_import_data *data = (struct module_import_data *)arg;
	DeeModuleObject *self = data->mid_self;
	if unlikely(DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (!(data->mid_mode & DeeExec_RUNMODE_FDEFAULTS_ARE_GLOBALS)) {
		struct TPPKeyword *kwd;
		struct symbol *sym;

		/* Define as local constants. */
		kwd = TPPLexer_LookupKeyword(DeeString_STR(name),
		                             DeeString_SIZE(name),
		                             1);
		if unlikely(!kwd)
			goto err;
		sym = get_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd);
		if unlikely(sym) {
			if (sym->s_type == SYMBOL_TYPE_CONST &&
			    sym->s_const == value)
				return 0; /* Already defined. */
			DeeError_Throwf(&DeeError_KeyError,
			                "Default value for %r has already been defined",
			                name);
			goto err;
		}
		sym = new_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd, NULL);
		if unlikely(!sym)
			goto err;
		sym->s_type  = SYMBOL_TYPE_CONST;
		sym->s_const = value;
		Dee_Incref(value);
	} else {
		Dee_hash_t i, perturb, hash;
		uint16_t addr;

		/* Rehash the global symbol table is need be. */
		if (self->mo_globalc / 2 >= current_rootscope->rs_bucketm &&
		    module_rehash_globals())
			goto err;
		if (self->mo_globalc >= data->mid_globala) {
			DREF DeeObject **new_globalv;
			uint16_t new_globala = data->mid_globala * 2;
			if (!new_globala)
				new_globala = 2;
			ASSERT(new_globala > self->mo_globalc);
			new_globalv = (DREF DeeObject **)Dee_TryReallocc(self->mo_globalv,
			                                                 new_globala,
			                                                 sizeof(DREF DeeObject *));
			if unlikely(!new_globalv) {
				new_globala = self->mo_globalc + 1;
				new_globalv = (DREF DeeObject **)Dee_Reallocc(self->mo_globalv,
				                                              new_globala,
				                                              sizeof(DREF DeeObject *));
				if unlikely(!new_globalv)
					goto err;
			}
			self->mo_globalv  = new_globalv;
			data->mid_globala = new_globala;
		}

		/* Append the symbol initializer */
		addr = self->mo_globalc++;
		self->mo_globalv[addr] = value;
		Dee_Incref(value);

		/* Insert the new object into the symbol table. */
		hash    = DeeString_Hash(name);
		perturb = i = Dee_MODULE_HASHST(self, hash);
		for (;; Dee_MODULE_HASHNX(i, perturb)) {
			struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
			if (item->ss_name)
				continue;

			/* Use this item. */
			item->ss_name  = DeeString_STR(name);
			item->ss_flags = MODSYM_FNAMEOBJ;
			item->ss_doc   = NULL;
			item->ss_hash  = hash;
			item->ss_index = addr;
			Dee_Incref(name);
			break;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
module_import_symbols(DeeModuleObject *self,
                      DeeObject *default_symbols,
                      unsigned int mode,
                      uint16_t *__restrict p_globala) {
	Dee_ssize_t result;
	struct module_import_data mid;
	mid.mid_self    = self;
	mid.mid_mode    = mode;
	mid.mid_globala = *p_globala;
	result = DeeObject_ForeachPair(default_symbols, &module_import_symbol, &mid);
	*p_globala = mid.mid_globala;
	return result;
}


/* Similar to `DeeExec_RunStream()', but rather than directly executing it,
 * return the module used to describe the code that is being executed, or
 * some unspecified, callable object which (when invoked) executes the given
 * input code in one way or another.
 * It is up to the implementation if an associated module should simply be
 * generated, before that module's root is returned, or if the given user-code
 * is only executed when the function is called, potentially allowing for
 * JIT-like execution of simple expressions such as `10 + 20' */
PUBLIC WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
DeeExec_CompileModuleStream(DeeObject *source_stream,
                            unsigned int mode,
                            int start_line, int start_col,
                            struct compiler_options *options,
                            DeeObject *default_symbols,
                            DeeObject *source_pathname,
                            DeeObject *module_name) {
	struct TPPFile *base_file;
	DREF DeeCodeObject *root_code;
	DREF DeeModuleObject *result;
	DREF DeeCompilerObject *compiler;
	DREF struct ast *code;
	uint16_t assembler_flags, result_globala;

	/* Create a new module. */
	if (!module_name) {
		if (source_pathname) {
			char const *name = DeeString_STR(source_pathname);
			size_t size = DeeString_SIZE(source_pathname);
			char const *name_end, *name_start;
			name_end   = name + size;
			name_start = DeeSystem_BaseName(name, size);

			/* Get rid of a file extension in the module name. */
			while (name_end > name_start && name_end[-1] != '.')
				--name_end;
			while (name_end > name_start && name_end[-1] == '.')
				--name_end;
			if (name_end == name_start)
				name_end = name + size;
			module_name = DeeString_NewSized(name_start,
			                                 (size_t)(name_end - name_start));
			if unlikely(!module_name)
				goto err;
		} else {
			module_name = DeeString_NewEmpty();
		}
	} else {
		Dee_Incref(module_name);
	}

	/* Create the new module. */
	result = DeeGCObject_CALLOC(DeeModuleObject);
	if unlikely(!result)
		goto err_module_name;
	result->mo_name    = (DREF DeeStringObject *)module_name; /* Inherit reference. */
	result->mo_bucketv = empty_module_buckets;
	Dee_atomic_rwlock_cinit(&result->mo_lock);
	DeeObject_Init(result, &DeeModule_Type);
	weakref_support_init(result);
	result = DeeGC_TRACK(DeeModuleObject, result);
	result->mo_flags = Dee_MODULE_FLOADING;
#ifndef CONFIG_NO_THREADS
	result->mo_loader = DeeThread_Self();
#endif /* !CONFIG_NO_THREADS */
	compiler = DeeCompiler_New((DeeObject *)result,
	                           options ? options->co_compiler : COMPILER_FNORMAL);
	if unlikely(!compiler)
		goto err_r;
	if (COMPILER_BEGIN(compiler))
		goto err_r_compiler_not_locked;
	base_file = TPPFile_OpenStream((stream_t)source_stream,
	                               source_pathname
	                               ? DeeString_STR(source_pathname)
	                               : "");
	if unlikely(!base_file)
		goto err_r_compiler;

	/* Set the starting-line offset. */
	if (TPPFile_SetStartingLineAndColumn(base_file, start_line, start_col)) {
		TPPFile_Decref(base_file);
		goto err_r_compiler;
	}

	/* Push the initial source file onto the #include-stack,
	 * and TPP inherit our reference to it. */
	TPPLexer_PushFileInherited(base_file);

	/* Override the name that is used as the
	 * effective display/DDI string of the file. */
	if (options && options->co_filename) {
		struct TPPString *used_name;
		ASSERT_OBJECT_TYPE_EXACT(options->co_filename, &DeeString_Type);
		used_name = TPPString_New(DeeString_STR(options->co_filename),
		                          DeeString_SIZE(options->co_filename));
		if unlikely(!used_name)
			goto err_r_compiler;
		ASSERT(!base_file->f_textfile.f_usedname);
		base_file->f_textfile.f_usedname = used_name; /* Inherit */
	}
	ASSERT(!current_basescope->bs_name);

#if 0
	if (!(mode & DeeExec_RUNMODE_FHASPP)) {
		/* Disable preprocessor directives & macros. */
		TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_NO_DIRECTIVES |
		                              TPPLEXER_FLAG_NO_MACROS |
		                              TPPLEXER_FLAG_NO_BUILTIN_MACROS);
	}
#endif

	inner_compiler_options = NULL;
	if (options) {
		/* Set the name of the current base-scope, which
		 * describes the function of the module's root code. */
		if (options->co_rootname) {
			ASSERT_OBJECT_TYPE_EXACT(options->co_rootname, &DeeString_Type);
			current_basescope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(options->co_rootname),
			                                                    DeeString_SIZE(options->co_rootname),
			                                                    1);
			if unlikely(!current_basescope->bs_name)
				goto err_r_compiler;
		}

		compiler->cp_options   = options;
		inner_compiler_options = options->co_inner;
		parser_flags           = options->co_parser;
		optimizer_flags        = options->co_optimizer;
		optimizer_unwind_limit = options->co_unwind_limit;
		if (options->co_tabwidth)
			TPPLexer_Current->l_tabsize = (size_t)options->co_tabwidth;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
		if (options->co_setup) {
			/* Run a custom setup protocol. */
			if unlikely((*options->co_setup)(options->co_setup_arg) < 0)
				goto err_r_compiler;
		}
	}

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots = new_unnamed_symbol();
		if unlikely(!dots)
			goto err_r_compiler;
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_r_compiler;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
		dots->s_decltype.da_type = DAST_NONE;
		dots->s_type  = SYMBOL_TYPE_ARG;
		dots->s_symid = 0;
		dots->s_flag |= SYMBOL_FALLOC;
		current_basescope->bs_argc    = 1;
		current_basescope->bs_argv[0] = dots;
		current_basescope->bs_varargs = dots;
		current_basescope->bs_flags |= CODE_FVARARGS;
	}

	result_globala = 0;
	if (default_symbols) {
		/* Provide default symbols as though they were defined as globals. */
		if unlikely(module_import_symbols(result, default_symbols, mode, &result_globala) < 0)
			goto err_r_compiler;
		current_rootscope->rs_globalc = result->mo_globalc;
	}

	/* Save the current exception context. */
	parser_start();

	/* Yield the initial token. */
	if unlikely(yield() < 0) {
		code = NULL;
	} else {
		/* Parse statements until the end of the source stream. */
		switch (mode & DeeExec_RUNMODE_MASK) {

		default:
			code = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, TOK_EOF);
			break;

		case DeeExec_RUNMODE_STMT:
			code = ast_parse_statement(false);
			goto pack_code_in_return;

		case DeeExec_RUNMODE_EXPR:
			code = ast_parse_comma(AST_COMMA_NORMAL,
			                       AST_FMULTIPLE_KEEPLAST,
			                       NULL);
			goto pack_code_in_return;

		case DeeExec_RUNMODE_FULLEXPR:
			code = ast_parse_comma(AST_COMMA_NORMAL |
			                       AST_COMMA_ALLOWVARDECLS |
			                       AST_COMMA_ALLOWTYPEDECL,
			                       AST_FMULTIPLE_KEEPLAST,
			                       NULL);
pack_code_in_return:
			if likely(code) {
				DREF struct ast *return_ast;
				return_ast = ast_putddi(ast_return(code), &code->a_ddi);
				ast_decref(code);
				code = return_ast;
			}
			break;
		}
	}
	if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
		TPPLexer_ClearIfdefStack();

	/* Rethrow all errors that may have occurred during parsing. */
	if unlikely(parser_rethrow(code == NULL))
		goto err_r_compiler_code;
	if unlikely(!code)
		goto err_r_compiler;

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		int error = ast_optimize_all(code, false);
		/* Rethrow all errors that may have occurred during optimization. */
		if (parser_rethrow(error != 0))
			error = -1;
		if (error)
			goto err_r_compiler_code;
	}

	assembler_flags = ASM_FNORMAL;
	if (options)
		assembler_flags = options->co_assembler;
	{
		uint16_t refc;
		struct asm_symbol_ref *refv;
		root_code = code_compile(code,
		                         assembler_flags,
		                         true,
		                         &refc,
		                         &refv);
		ASSERT(!root_code || !refc);
		ASSERT(!root_code || !refv);
	}
	ast_decref(code);

	/* Rethrow all errors that may have occurred during text assembly. */
	if (parser_rethrow(root_code == NULL))
		Dee_XClear(root_code);

	/* Check for errors during assembly. */
	if unlikely(!root_code)
		goto err_r_compiler;

	/* Finally, put together the module itself. */
	if (current_rootscope->rs_importa != current_rootscope->rs_importc) {
		DREF DeeModuleObject **new_vector;
		new_vector = (DREF DeeModuleObject **)Dee_TryReallocc(current_rootscope->rs_importv,
		                                                      current_rootscope->rs_importc,
		                                                      sizeof(DREF DeeModuleObject *));
		if likely(new_vector)
			current_rootscope->rs_importv = new_vector;
	}
	if (!result->mo_globalv) {
		result->mo_globalv = (DREF DeeObject **)Dee_Callocc(current_rootscope->rs_globalc,
		                                                    sizeof(DREF DeeObject *));
		if unlikely(!result->mo_globalv)
			goto err_r_compiler;
	} else {
		if (current_rootscope->rs_globalc > result_globala) {
			DREF DeeObject **final_globalv;
			final_globalv = (DREF DeeObject **)Dee_Reallocc(result->mo_globalv,
			                                                current_rootscope->rs_globalc,
			                                                sizeof(DREF DeeObject *));
			if unlikely(!final_globalv)
				goto err_r_compiler;
			result->mo_globalv = final_globalv;
		}
		bzeroc(result->mo_globalv + result->mo_globalc,
		       current_rootscope->rs_globalc - result->mo_globalc,
		       sizeof(DREF DeeObject *));
	}
	result->mo_globalc = current_rootscope->rs_globalc;
	result->mo_importc = current_rootscope->rs_importc;
	atomic_or(&result->mo_flags, current_rootscope->rs_flags);
	result->mo_bucketm = current_rootscope->rs_bucketm;
	result->mo_bucketv = current_rootscope->rs_bucketv;
	result->mo_importv = current_rootscope->rs_importv;
	result->mo_root    = root_code; /* Inherit reference. */

	/* Yes, we're just stealing all of these. */
	current_rootscope->rs_importv = NULL;
	current_rootscope->rs_importc = 0;
	current_rootscope->rs_importa = 0;
	current_rootscope->rs_bucketv = empty_module_buckets;
	current_rootscope->rs_bucketm = 0;

	{
		DREF DeeCodeObject *iter, *next;
		iter = current_rootscope->rs_code;
		current_rootscope->rs_code = NULL;
		while (iter) {
			next = iter->co_next;
			iter->co_module = result;
			Dee_Incref(result); /* Create the new module-reference now stored in `iter->co_module'. */
			Dee_Decref(iter);   /* This reference was owned by the chain before. */
			iter = next;
		}
	}

	COMPILER_END();
	Dee_Decref(compiler);
	atomic_or(&result->mo_flags, Dee_MODULE_FDIDLOAD);
	return result;
err_r_compiler_code:
	ast_xdecref(code);
err_r_compiler:
	COMPILER_END();
err_r_compiler_not_locked:
	Dee_Decref(compiler);
err_r:
	atomic_and(&result->mo_flags, ~(Dee_MODULE_FLOADING));
	Dee_Decref_likely(result);
	goto err;
err_module_name:
	Dee_Decref(module_name);
err:
	return NULL;
}

DECL_END
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */





DECL_BEGIN


/* Figure out how to implement `get_default_home()' */
#undef get_default_home_USE_CONFIG_DEEMON_HOME
#undef get_default_home_USE_GetModuleFileNameW
#undef get_default_home_USE_readlink_proc_self_exe
#ifdef CONFIG_DEEMON_HOME
#define get_default_home_USE_CONFIG_DEEMON_HOME
#elif defined(CONFIG_HAVE_dlmodulename) && defined(CONFIG_HAVE_dlopen)
#define get_default_home_USE_dlmodulename
#elif defined(CONFIG_HOST_WINDOWS)
#define get_default_home_USE_GetModuleFileNameW
#elif defined(CONFIG_HAVE_PROCFS)
#define get_default_home_USE_readlink_proc_self_exe
#else /* ... */
#define get_default_home_USE_readlink_proc_self_exe
#endif /* !... */

#ifdef get_default_home_USE_CONFIG_DEEMON_HOME
PRIVATE DEFINE_STRING(default_deemon_home, CONFIG_DEEMON_HOME);
#endif /* get_default_home_USE_CONFIG_DEEMON_HOME */



PRIVATE WUNUSED DREF DeeStringObject *DCALL get_default_home(void) {
#ifndef CONFIG_NO_DEEMON_HOME_ENVIRON
	char const *env;
#ifndef CONFIG_DEEMON_HOME_ENVIRON
#define CONFIG_DEEMON_HOME_ENVIRON "DEEMON_HOME"
#endif /* !CONFIG_DEEMON_HOME_ENVIRON */
	DBG_ALIGNMENT_DISABLE();
	env = getenv(CONFIG_DEEMON_HOME_ENVIRON); /* TODO: system-feature test */
	DBG_ALIGNMENT_ENABLE();
	if (env) {
		DREF DeeStringObject *result;
		size_t len = strlen(env);
		if (len) {
			DREF DeeStringObject *new_result;
			while (len && DeeSystem_IsSep(env[len - 1]))
				--len;
			result = (DREF DeeStringObject *)DeeString_NewUtf8(env, len + 1,
			                                                   STRING_ERROR_FIGNORE);
#define get_default_home_NEED_ERR 1
			if unlikely(!result)
				goto err;
			DeeString_SetChar(result, len - 1, DeeSystem_SEP);
			new_result = (DREF DeeStringObject *)DeeSystem_MakeNormalAndAbsolute((DeeObject *)result);
			Dee_Decref(result);
			return new_result;
		}
	}
#endif /* !CONFIG_NO_DEEMON_HOME_ENVIRON */

#ifdef get_default_home_USE_CONFIG_DEEMON_HOME
#define get_default_home_NO_FALLBACK 1
	return_reference_((DeeStringObject *)&default_deemon_home);
#endif /* get_default_home_USE_CONFIG_DEEMON_HOME */

#ifdef get_default_home_USE_GetModuleFileNameW
	{
		DREF DeeStringObject *result;
		DWORD dwBufSize = PATH_MAX, dwError;
		LPWSTR lpBuffer, lpNewBuffer;
		DREF DeeStringObject *new_result;
		lpBuffer = DeeString_NewWideBuffer(dwBufSize);
#define get_default_home_NEED_ERR 1
		if unlikely(!lpBuffer)
			goto err;
again_chk_intr:
		if (DeeThread_CheckInterrupt()) {
err_buffer:
			DeeString_FreeWideBuffer(lpBuffer);
			goto err;
		}
		for (;;) {
			DBG_ALIGNMENT_DISABLE();
			SetLastError(0);
			dwError = GetModuleFileNameW(NULL, lpBuffer, dwBufSize + 1);
			if (!dwError) {
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (DeeNTSystem_IsIntr(dwError))
					goto again_chk_intr;
				if (DeeNTSystem_IsBufferTooSmall(dwError))
					goto do_increase_buffer;
				DeeString_FreeWideBuffer(lpBuffer);
				goto fallback;
			}
			DBG_ALIGNMENT_ENABLE();
			if (dwError <= dwBufSize) {
				if (dwError < dwBufSize)
					break;
				DBG_ALIGNMENT_DISABLE();
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (!DeeNTSystem_IsBufferTooSmall(dwError))
					break;
			}
			/* Increase buffer size. */
do_increase_buffer:
			dwBufSize *= 2;
			lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwBufSize);
			if unlikely(!lpNewBuffer)
				goto err_buffer;
			lpBuffer = lpNewBuffer;
		}
		/* TODO: Check if the module's file name is a symbolic link.
		 *       If it turns out to be one, follow it! */

		/* Trim the trailing module filename, but keep 1 trailing slash. */
		while (dwError && !DeeSystem_IsSep(lpBuffer[dwError - 1]))
			--dwError;
		while (dwError && DeeSystem_IsSep(lpBuffer[dwError - 1]))
			--dwError;
		lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError + 1);
		result = (DREF DeeStringObject *)DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
		if unlikely(!result)
			goto err;
		new_result = (DREF DeeStringObject *)DeeSystem_MakeNormalAndAbsolute((DeeObject *)result);
		Dee_Decref(result);
		return new_result;
	}
#endif /* get_default_home_USE_GetModuleFileNameW */

#ifdef get_default_home_USE_dlmodulename
	{
		size_t length;
		/* dlopen(NULL)   -> Return a handle to the primary binary
		 *                   This is actually behavior that is mandated by POSIX! */
		void *hProc = dlopen(NULL, DLOPEN_NULL_FLAGS);
		/* dlmodulename() -> KOS extension that returns a module's absolute filename.
		 *                   This one's really the perfect solution, since it doesn't
		 *                   involve any additional system calls being made, or making
		 *                   runtime assumptions such as /proc being mounted. */
		char const *filename = dlmodulename(hProc);
		if unlikely(!filename)
			goto fallback;
		length = strlen(filename);
		/* Trim the actual executable filename (which is likely to be `deemon'),
		 * thus getting the absolute path where the executable is placed (which
		 * is likely to be something along the lines of `/bin/' or `/usr/bin/') */
		while (length && filename[length - 1] != '/')
			--length;
		if (length && unlikely(filename[length - 1] == '/')) {
			/* Strip additional slashes, such that only a single one remains */
			while (length >= 2 && filename[length - 2] == '/')
				--length;
		}
		return (DREF DeeStringObject *)DeeString_NewUtf8(filename,
		                                                 length,
		                                                 STRING_ERROR_FIGNORE);
	}
#endif /* get_default_home_USE_dlmodulename */

#ifdef get_default_home_USE_readlink_proc_self_exe
	size_t length;
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeUnixSystem_PrintLinkString(&printer, "/proc/self/exe");
	if unlikely(error != 0) {
		if (error < 0)
			goto err_printer;
		/* Fallback... */
bad_path:
		unicode_printer_fini(&printer);
		goto fallback;
	}
	length = UNICODE_PRINTER_LENGTH(&printer);
	if unlikely(!length)
		goto bad_path;
	while (length && UNICODE_PRINTER_GETCHAR(&printer, length - 1) != '/')
		--length;
	if unlikely(!length)
		goto fallback;
	while (length && UNICODE_PRINTER_GETCHAR(&printer, length - 1) == '/')
		--length;
	unicode_printer_truncate(&printer, length + 1);
	return (DREF DeeStringObject *)unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* get_default_home_USE_readlink_proc_self_exe */

#ifndef get_default_home_NO_FALLBACK
fallback:

	/* TODO: Check if `main:argv[0]' is an absolute filename. */
	/* TODO: Check if `main:argv[0]' can be found in $PATH. */
	return_reference_(&str_dot);
#endif /* !get_default_home_NO_FALLBACK */

#ifdef get_default_home_NEED_ERR
err:
	return NULL;
#endif /* get_default_home_NEED_ERR */
}


PRIVATE Dee_ATOMIC_REF(DeeStringObject) deemon_home = Dee_ATOMIC_REF_INIT(NULL);

/* Get/Set deemon's home path.
 * The home path is used to locate builtin libraries, as well as extensions.
 * Upon first access, `DeeExec_GetHome()' will pre-initialize the home-path as follows:
 * >> deemon_home = fs.environ["DEEMON_HOME"];
 * >> if (deemon_home !is none) {
 * >>     deemon_home = fs.abspath(deemon_home);
 * >> } else {
 * >>#ifdef CONFIG_DEEMON_HOME
 * >>     deemon_home = CONFIG_DEEMON_HOME;
 * >>#else
 * >>     // Some other os-specific shenanigans here...
 * >>     deemon_home = fs.headof(fs.readlink("/proc/self/exe"));
 * >>#endif
 * >> }
 * >> deemon_home = fs.inctrail(deemon_home);
 * That is: Try to lookup an environment variable `DEEMON_HOME', which
 *          if found is then converted into an absolute filename.
 *          When this variable doesn't exist, behavior depends on how deemon was built.
 *          If it was built with the `CONFIG_DEEMON_HOME' option enabled, that
 *          option is interpreted as a string which is then used as the effective
 *          home path, but if that option was disabled, the folder of deemon's
 *          executable is used as home folder instead.
 * @return: NULL: Failed to determine the home folder (an error was set).
 * NOTE: The home path _MUST_ include a trailing slash! */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL
DeeExec_GetHome(void) {
	DREF DeeStringObject *result;
	result = (DREF DeeStringObject *)Dee_atomic_ref_xget(&deemon_home);
	if (result)
		return Dee_AsObject(result);

	/* Re-create the default home path. */
	result = get_default_home();

	/* Save the generated path in the global variable. */
	if likely(result) {
		Dee_Incref(result);
		while (!Dee_atomic_ref_xcmpxch_inherited(&deemon_home, NULL, (DeeObject *)result)) {
			DREF DeeStringObject *other;
			other = (DREF DeeStringObject *)Dee_atomic_ref_xget(&deemon_home);
			if (other) {
				Dee_Decref(result);
				result = other;
				break;
			}
		}
	}
	return Dee_AsObject(result);
}

/* Set the new home folder, overwriting whatever was set before.
 * HINT: You may pass `NULL' to cause the default home path to be re-created. */
PUBLIC void DCALL
DeeExec_SetHome(/*String*/ DeeObject *new_home) {
	ASSERT_OBJECT_TYPE_EXACT_OPT(new_home, &DeeString_Type);
	Dee_atomic_ref_xset(&deemon_home, new_home);
}



/* List of strings that should be used as base paths when searching for global modules.
 * Access to this list should go through `DeeModule_InitPath()', which will
 * automatically initialize the list to the following default contents upon access:
 *
 * >> Commandline: `-L...' where every occurrance is pre-pended before the home-path.
 *    NOTE: These paths are not added by `DeeModule_InitPath()', but instead
 *          the first encouter of a -L option will call `DeeModule_GetPath()'
 *          before pre-pending the following string at the front of the list,
 *          following other -L paths prepended before then.
 * >> posix.environ.get("DEEMON_PATH", "").split(posix.FS_DELIM)...;
 * >> posix.joinpath(DeeExec_GetHome(), "lib")
 *
 * This list is also used to locate system-include paths for the preprocessor,
 * in that every entry that is a string is an include-path after appending "/include":
 * >> function get_include_paths(): {string...} {
 * >>     for (local x: DeeModule_Path)
 * >>         if (x is string)
 * >>             yield f"{x.rstrip("/")}/include";
 * >> } */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES

/* Ensure that "self" isn't being shared with anything else */
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
DeeTuple_Unshare(/*inherit(always)*/ DREF DeeTupleObject *__restrict self) {
	if (DeeObject_IsShared(self)) {
		size_t count = DeeTuple_SIZE(self);
		DREF DeeTupleObject *copy = DeeTuple_NewUninitialized(count);
		if unlikely(!copy)
			goto err_self;
		Dee_Movrefv(DeeTuple_ELEM(copy), DeeTuple_ELEM(self), count);
		Dee_Decref_unlikely(self);
		self = copy;
	}
	return self;
err_self:
	Dee_Decref_unlikely(self);
	return NULL;
}

/* Remove duplicate strings from "self". Correctly handles `DeeObject_IsShared(self)' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
DeeTuple_RemoveDuplicateStrings(/*inherit(always)*/ DREF DeeTupleObject *__restrict self) {
	size_t i, j;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		DeeStringObject *a = (DeeStringObject *)DeeTuple_GET(self, i);
		for (j = i + 1; j < DeeTuple_SIZE(self);) {
			DeeStringObject *b = (DeeStringObject *)DeeTuple_GET(self, i);
			if (FS_DeeString_EqualsSTR(a, b)) {
				DREF DeeTupleObject *copy;
				copy = DeeTuple_NewUninitialized(DeeTuple_SIZE(self) - 1);
				if unlikely(!copy)
					goto err_self;
				/* Remove string at "j" */
				memcpyc(DeeTuple_ELEM(copy), DeeTuple_ELEM(self), j, sizeof(DeeObject *));
				memcpyc(DeeTuple_ELEM(copy) + j, DeeTuple_ELEM(self) + j + 1,
				        DeeTuple_SIZE(copy) - j, sizeof(DeeObject *));
				Dee_Increfv(DeeTuple_ELEM(copy), DeeTuple_SIZE(copy));
				Dee_Decref_likely(self);
				self = copy; /* Inherit reference */
			} else {
				++j;
			}
		}
	}
	return self;
err_self:
	Dee_Decref_likely(self);
	return NULL;
}

/* Add "string" to "self" if not already present. re-returns "self" if already present. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
DeeTuple_AddDistinctFSString(/*inherit(always)*/ DREF DeeTupleObject *__restrict self,
                             DeeStringObject *__restrict string) {
	size_t i;
	DREF DeeTupleObject *result;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		DeeStringObject *a = (DeeStringObject *)DeeTuple_GET(self, i);
		if (FS_DeeString_EqualsSTR(a, string))
			return self; /* Already present. */
	}
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(self) + 1);
	if unlikely(!result)
		goto err_self;
	Dee_Movrefv(DeeTuple_ELEM(result), DeeTuple_ELEM(self), DeeTuple_SIZE(self));
	DeeTuple_SET(result, DeeTuple_SIZE(self), string);
	Dee_Incref(string);
	return result;
err_self:
	Dee_Decref_likely(self);
	return NULL;
}

/* Remove "string" from "self" if present. re-returns "self" if not present. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
DeeTuple_RemoveDistinctFSString(/*inherit(always)*/ DREF DeeTupleObject *__restrict self,
                                DeeStringObject *__restrict string) {
	size_t i;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		DeeStringObject *a = (DeeStringObject *)DeeTuple_GET(self, i);
		if (FS_DeeString_EqualsSTR(a, string)) {
			DREF DeeTupleObject *result;
			result = DeeTuple_NewUninitialized(DeeTuple_SIZE(self) - 1);
			if unlikely(!result)
				goto err_self;
			/* Remove string at "i" */
			memcpyc(DeeTuple_ELEM(result), DeeTuple_ELEM(self), i, sizeof(DeeObject *));
			memcpyc(DeeTuple_ELEM(result) + i, DeeTuple_ELEM(self) + i + 1,
			        DeeTuple_SIZE(result) - i, sizeof(DeeObject *));
			Dee_Increfv(DeeTuple_ELEM(result), DeeTuple_SIZE(result));
			Dee_Decref_likely(self);
			return result;
		}
	}
	return self; /* Already present. */
err_self:
	Dee_Decref_likely(self);
	return NULL;
}


#undef DeeModule_AppendEnvironPath_USE_STUB
#undef DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW
#undef DeeModule_AppendEnvironPath_USE_wgetenv
#undef DeeModule_AppendEnvironPath_USE_getenv
#undef DeeModule_AppendEnvironPath_USE_wenviron
#undef DeeModule_AppendEnvironPath_USE_environ
#ifdef CONFIG_NO_DEEMON_PATH_ENVIRON
#define DeeModule_AppendEnvironPath_USE_STUB
#elif defined(CONFIG_HOST_WINDOWS)
#define DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW
#elif defined(CONFIG_HAVE_wgetenv) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeModule_AppendEnvironPath_USE_wgetenv
#elif defined(CONFIG_HAVE_getenv)
#define DeeModule_AppendEnvironPath_USE_getenv
#elif defined(CONFIG_HAVE_wgetenv)
#define DeeModule_AppendEnvironPath_USE_wgetenv
#elif defined(CONFIG_HAVE_wenviron) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeModule_AppendEnvironPath_USE_wenviron
#elif defined(CONFIG_HAVE_environ)
#define DeeModule_AppendEnvironPath_USE_environ
#elif defined(CONFIG_HAVE_wenviron)
#define DeeModule_AppendEnvironPath_USE_wenviron
#else /* ... */
#define DeeModule_AppendEnvironPath_USE_STUB
#endif /* ... */


/* If we ever use `environ' for anything, we have to use a lock to access it. */
#if (defined(DeeModule_AppendEnvironPath_USE_wgetenv) ||  \
     defined(DeeModule_AppendEnvironPath_USE_getenv) ||   \
     defined(DeeModule_AppendEnvironPath_USE_wenviron) || \
     defined(DeeModule_AppendEnvironPath_USE_environ))
#if defined(CONFIG_HAVE_ENV_LOCK) && defined(CONFIG_HAVE_ENV_UNLOCK)
#ifdef CONFIG_HAVE_ENVLOCK_H
#include <envlock.h>
#endif /* CONFIG_HAVE_ENVLOCK_H */
#define environ_lock_read()    ENV_LOCK
#define environ_lock_endread() ENV_UNLOCK
#else /* CONFIG_HAVE_ENV_LOCK && CONFIG_HAVE_ENV_UNLOCK */
/* XXX: This should use "dee_environ_lock" from "/src/dex/posix/p-environ.c.inl" */
#define environ_lock_read()    (void)0
#define environ_lock_endread() (void)0
#endif /* !CONFIG_HAVE_ENV_LOCK || !CONFIG_HAVE_ENV_UNLOCK */
#endif /* ... */
#ifndef environ_lock_read
#define environ_lock_read()    (void)0
#define environ_lock_endread() (void)0
#endif /* !environ_lock_read */


#ifdef DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nt_GetEnvironmentVariableW(LPWSTR name) {
	LPWSTR buffer, new_buffer;
	DWORD bufsize = 256, error;
	buffer = DeeString_NewWideBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		environ_lock_read();
		DBG_ALIGNMENT_DISABLE();
		error = GetEnvironmentVariableW(name, buffer, bufsize + 1);
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endread();
		if (!error) {
			/* Error. */
			DeeString_FreeWideBuffer(buffer);
			return ITER_DONE;
		}
		if (error <= bufsize)
			break;
		/* Resize to fit. */
		new_buffer = DeeString_ResizeWideBuffer(buffer, error);
		if unlikely(!new_buffer)
			goto err_buffer;
		buffer  = new_buffer;
		bufsize = error - 1;
	}
	buffer = DeeString_TruncateWideBuffer(buffer, error);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}
#endif /* DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW */

#undef unix_getenv
#ifdef DeeModule_AppendEnvironPath_USE_getenv
#define unix_getenv(name) getenv((char *)(name))
#elif defined(DeeModule_AppendEnvironPath_USE_environ)
PRIVATE char *DCALL unix_getenv(char const *name) {
	char **env = (char **)environ;
	if (env) {
		size_t namelen = strlen(name);
		for (; *env; ++env) {
			char *line = *env;
			if (memcmpc(line, name, namelen, sizeof(char)) == 0 &&
			    line[namelen] == (char)'=') {
				line += namelen;
				line += 1;
				return line;
			}
		}
	}
	return NULL;
}
#endif /* ... */

#undef unix_wgetenv
#ifdef DeeModule_AppendEnvironPath_USE_wgetenv
#define unix_wgetenv(name) ((Dee_wchar_t *)wgetenv((wchar_t *)(name)))
#elif defined(DeeModule_AppendEnvironPath_USE_wenviron)
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
PRIVATE Dee_wchar_t *DCALL unix_wgetenv(Dee_wchar_t const *name) {
	Dee_wchar_t **env = (Dee_wchar_t **)wenviron;
	if (env) {
		size_t namelen = wcslen((wchar_t *)name);
		for (; *env; ++env) {
			Dee_wchar_t *line = *env;
			if (memcmpc(line, name, namelen, sizeof(Dee_wchar_t)) == 0 &&
			    line[namelen] == (Dee_wchar_t)'=') {
				line += namelen;
				line += 1;
				return line;
			}
		}
	}
	return NULL;
}
#endif /* ... */

#undef DeeModule_AppendEnvironPath_USE_WCHAR
#if defined(DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW) || defined(unix_wgetenv)
#define DeeModule_AppendEnvironPath_USE_WCHAR
#endif /* DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW || unix_wgetenv */

#ifndef DeeModule_AppendEnvironPath_USE_STUB
#ifdef DeeModule_AppendEnvironPath_USE_WCHAR
#ifdef CONFIG_DEEMON_PATH_ENVIRON
#undef L
PRIVATE Dee_wchar_t const envname_DEEMON_PATH[] = PP_CAT2(L, CONFIG_DEEMON_PATH_ENVIRON);
#else /* CONFIG_DEEMON_PATH_ENVIRON */
PRIVATE Dee_wchar_t const envname_DEEMON_PATH[] = { 'D', 'E', 'E', 'M', 'O', 'N', '_', 'P', 'A', 'T', 'H', 0 };
#endif /* !CONFIG_DEEMON_PATH_ENVIRON */
#endif /* DeeModule_AppendEnvironPath_USE_WCHAR */

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeModule_AppendEnvironPathItem(struct Dee_tuple_builder *__restrict builder,
                                /*utf-8*/ char const *item, size_t item_size) {
	DREF DeeObject *raw, *normal;
	raw = DeeString_NewUtf8(item, item_size, STRING_ERROR_FIGNORE);
	if unlikely(!raw)
		goto err;
	normal = DeeSystem_MakeNormalAndAbsolute(raw);
	Dee_Decref_likely(raw);
	if unlikely(!normal)
		goto err;
	return Dee_tuple_builder_append_inherited(builder, normal);
err:
	return -1;
}

/* @return: 0 : Nothing was added
 * @return: 1 : Something was added
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_AppendEnvironPath(struct Dee_tuple_builder *__restrict builder) {
	/*utf-8*/ char const *path;
#undef LOCAL_HAVE_path_ob
#ifdef DeeModule_AppendEnvironPath_USE_WCHAR
#ifdef DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW
#define LOCAL_HAVE_path_ob
	DREF DeeObject *path_ob = nt_GetEnvironmentVariableW((LPWSTR)envname_DEEMON_PATH);
	if likely(path_ob == ITER_DONE)
		return 0;
	if unlikely(path_ob == NULL)
		goto err;
#define NEED_err
	path = DeeString_AsUtf8(path_ob);
	if unlikely(!path)
		goto err_path_ob;
#else /* DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW */
	path = unix_wgetenv(envname_DEEMON_PATH);
	if (!path)
		return 0;
#endif /* !DeeModule_AppendEnvironPath_USE_GetEnvironmentVariableW */
#else /* DeeModule_AppendEnvironPath_USE_WCHAR */
#ifdef CONFIG_DEEMON_PATH_ENVIRON
	path = unix_getenv(CONFIG_DEEMON_PATH_ENVIRON);
#else /* CONFIG_DEEMON_PATH_ENVIRON */
	path = unix_getenv("DEEMON_PATH");
#endif /* !CONFIG_DEEMON_PATH_ENVIRON */
	if (!path)
		return 0;
#endif /* !DeeModule_AppendEnvironPath_USE_WCHAR */

	/* Split "path" into segments based on "DeeSystem_DELIM" */
	while (*path) {
		char const *delim = strchr(path, DeeSystem_DELIM);
		if (delim == NULL)
			delim = strend(path);
		if unlikely(DeeModule_AppendEnvironPathItem(builder, path, (size_t)(delim - path)) < 0)
			goto err_path_ob;
		path = delim;
		if (!*path)
			break;
		++path;
	}

#ifdef LOCAL_HAVE_path_ob
	Dee_Decref_likely(path_ob);
#endif /* LOCAL_HAVE_path_ob */
	return 1;
err_path_ob:
#ifdef LOCAL_HAVE_path_ob
#undef LOCAL_HAVE_path_ob
	Dee_Decref_likely(path_ob);
#endif /* LOCAL_HAVE_path_ob */
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return -1;
}
#endif /* !DeeModule_AppendEnvironPath_USE_STUB */

/* Allocate+initialize a new default */
PRIVATE WUNUSED DREF DeeTupleObject *DCALL DeeModule_NewDefaultPath(void) {
#ifndef DeeModule_AppendEnvironPath_USE_STUB
	DREF DeeTupleObject *result;
	struct Dee_tuple_builder builder = Dee_TUPLE_BUILDER_INIT;
	int env_status;
	environ_lock_read();
	env_status = DeeModule_AppendEnvironPath(&builder);
	environ_lock_endread();
	if unlikely(env_status < 0)
		goto err_builder;
#ifdef CONFIG_DEEMON_PATH
#define append_path_cb(str)                                                                 \
	{                                                                                       \
		PRIVATE DEFINE_STRING(_libpath_string, str);                                        \
		if unlikely(Dee_tuple_builder_append(&builder, Dee_AsObject(&_libpath_string)) < 0) \
			goto err_builder;                                                               \
	}
	CONFIG_DEEMON_PATH(append_path_cb)
#undef append_path_cb
#else /* CONFIG_DEEMON_PATH */
	{
		DREF DeeObject *default_path;
		default_path = DeeString_Newf("%Klib", DeeExec_GetHome());
		if unlikely(!default_path)
			goto err_builder;
		if unlikely(Dee_tuple_builder_append_inherited(&builder, default_path) < 0)
			goto err_builder;
	}
#endif /* !CONFIG_DEEMON_PATH */
	result = (DREF DeeTupleObject *)Dee_tuple_builder_pack(&builder);
	if (env_status && result)
		result = DeeTuple_RemoveDuplicateStrings(result);
	return result;
err_builder:
	Dee_tuple_builder_fini(&builder);
	return NULL;
#elif defined(CONFIG_DEEMON_PATH)
#define count_paths_cb(p) +1
#define num_paths (0 CONFIG_DEEMON_PATH(count_paths_cb))
	size_t i;
	DREF DeeTupleObject *result;
	result = DeeTuple_NewUninitialized(num_paths);
	if unlikely(!result)
		goto err;
	i = 0;
#define append_path_cb(str)                                      \
	{                                                            \
		PRIVATE DEFINE_STRING(_libpath_string, str);             \
		DeeTuple_SET(result, i, Dee_AsObject(&_libpath_string)); \
		Dee_Incref(&_libpath_string);                            \
		++i;                                                     \
	}
	CONFIG_DEEMON_PATH(append_path_cb)
#undef append_path_cb
	return result;
err:
	return NULL;
#undef num_paths
#undef count_paths_cb
#else /* ... */
	DREF DeeTupleObject *result;
	DREF DeeObject *default_path;
	default_path = DeeString_Newf("%Klib", DeeExec_GetHome());
	if unlikely(!default_path)
		goto err;
	result = DeeTuple_NewUninitialized(1);
	if unlikely(!result)
		goto err_default_path;
	DeeTuple_SET(result, 0, default_path);
	return result;
err_default_path:
	Dee_Decref_likely(default_path);
err:
	return NULL;
#endif /* !... */
}


INTERN bool DCALL DeeModule_ClearLibPath(void) {
	DREF DeeObject *oldval;
	oldval = Dee_atomic_ref_xxch_inherited(&deemon_path, NULL);
	if (!oldval)
		return false;
	Dee_Decref(oldval);
	return true;
}


/* Check for paths that were removed in "new_libpath". Allowed to assume
 * that `DeeString_AsUtf8()' will never fail for any contained string
 * (meaning that the caller should have pre-loaded all utf-8 reprs of
 * all strings). */
PRIVATE NONNULL((1, 2)) void DCALL
DeeModule_HandleRemovedLibPath_diff_locked(DeeTupleObject *old_libpath,
                                           DeeTupleObject *new_libpath) {
	size_t i_new, i_old;
	for (i_old = 0; i_old < DeeTuple_SIZE(old_libpath); ++i_old) {
		bool exists = false;
		DeeStringObject *oldpath = (DeeStringObject *)DeeTuple_GET(old_libpath, i_old);
		for (i_new = 0; i_new < DeeTuple_SIZE(new_libpath); ++i_new) {
			if (FS_DeeString_EqualsSTR(oldpath, DeeTuple_GET(new_libpath, i_new))) {
				exists = true;
				break;
			}
		}
		if (!exists) {
			/* Path was removed. */
			char const *utf8 = DeeString_AsUtf8((DeeObject *)oldpath);
			ASSERTF(utf8, "Should have been pre-loaded by caller");
			DeeModule_HandleRemovedLibPath_locked(utf8, WSTR_LENGTH(utf8));
		}
	}
}

/* Apply a new (already-sanitized) module path "new_path".
 * @param: old_path: When non-NULL, only apply "new_path" if this is still the current path
 * @param: new_path: New set of (normalized+absolute+distinct) lib paths.
 * @param: remove_hint == NULL:      Any arbitrary path may have been removed
 * @param: remove_hint == ITER_DONE: No paths present in "old_path" were removed
 * @param: remove_hint == *:         Only this specific path (and no others) were removed
 * @return: 1 : "old_path != NULL" and was no longer the currently set path
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((2)) int DCALL
DeeModule_ApplyLibPath(DeeTupleObject *old_path,
                       /*inherit(always)*/ DREF DeeTupleObject *new_path,
                       DeeStringObject *remove_hint) {
	size_t i;
	DREF DeeTupleObject *assumed_old_path;

	/* Technically only necessary when paths are added: clear all flags
	 * indicating that some module has all of its possible lib names
	 * loaded already (since addition of new paths may mean that there
	 * are now more names possible):
	 *
	 * >> old_path: ["/opt/deemon/lib/"]
	 * >> module:   "/opt/deemon/lib/doc.dee"  -> ["doc"]
	 *
	 * >> new_path: ["/opt/deemon/lib/", "/opt/deemon/"]
	 * >> module:   "/opt/deemon/lib/doc.dee"  -> ["doc", "lib.doc"] */
	DeeModule_ClearLibAllFlag_locked();

	if (remove_hint == (DeeStringObject *)ITER_DONE) {
		if (old_path == NULL) {
			Dee_atomic_ref_set_inherited(&deemon_path, (DeeObject *)new_path);
			return 0;
		}
		if (!Dee_atomic_ref_xcmpxch_inherited(&deemon_path, (DeeObject *)old_path, (DeeObject *)new_path))
			return 1; /* "old_path" is no longer up-to-date */
		Dee_Decref_likely(old_path);
		return 0;
	}

	/* Set operation must happen while holding "module_libtree_lock",
	 * but only a single path may have been removed. */
	if (remove_hint) {
		char const *remove_hint_utf8 = DeeString_AsUtf8((DeeObject *)remove_hint);
		if unlikely(!remove_hint_utf8)
			goto err;
		module_libtree_lock_write();
		if (old_path == NULL) {
			Dee_atomic_ref_set_inherited(&deemon_path, (DeeObject *)new_path);
		} else if (Dee_atomic_ref_cmpxch_inherited(&deemon_path, (DeeObject *)old_path, (DeeObject *)new_path)) {
			Dee_Decref_likely(old_path); /* Inherited from "Dee_atomic_ref_cmpxch_inherited" */
		} else {
			module_libtree_lock_endwrite();
			return 1; /* "old_path" is no longer up-to-date */
		}
		DeeModule_HandleRemovedLibPath_locked(remove_hint_utf8, WSTR_LENGTH(remove_hint_utf8));
		module_libtree_lock_endwrite();
		return 0;
	}

again_load_assumed_old_path:
	assumed_old_path = (DeeTupleObject *)Dee_atomic_ref_xget(&deemon_path);
	if (old_path && (old_path != assumed_old_path)) {
		Dee_XDecref_unlikely(assumed_old_path);
		return 1; /* Wrong "old_path" */
	}

	/* Check for special case: no path loaded. */
	if (!assumed_old_path) {
		if (!Dee_atomic_ref_xcmpxch_inherited(&deemon_path, NULL, (DeeObject *)new_path))
			goto again_load_assumed_old_path;
		return 0;
	}

	/* Must pre-load the utf-8 reprs of all path strings because we can't
	 * have the UTF-8 loader fail after "module_libtree_lock_write()". */
	for (i = 0; i < DeeTuple_SIZE(assumed_old_path); ++i) {
		DeeObject *item = DeeTuple_GET(assumed_old_path, i);
		if (!DeeString_AsUtf8(item))
			goto err_assumed_old_path;
	}

#if 0 /* Not needed -- only utf-8 of potentially **REMOVED** paths is needed (with are never in "new_path") */
	/* Also pre-load the utf-8 reprs of all new paths. */
	for (i = 0; i < DeeTuple_SIZE(new_path); ++i) {
		DeeObject *item = DeeTuple_GET(new_path, i);
		if (!DeeString_AsUtf8(item))
			goto err_assumed_old_path;
	}
#endif

	/* Lock libtree (so we can remove global names of modules from removed lib paths) */
	module_libtree_lock_write();

	/* Set new library path */
	if (!Dee_atomic_ref_cmpxch_inherited(&deemon_path,
	                                     (DeeObject *)assumed_old_path,
	                                     (DeeObject *)new_path)) {
		module_libtree_lock_endwrite();
		goto again_load_assumed_old_path;
	}

	/* Remove global names of modules that (might) no longer appear in lib paths. */
	DeeModule_HandleRemovedLibPath_diff_locked(assumed_old_path, new_path);

	/* Release locks + cleanup */
	module_libtree_lock_endwrite();

	/* +1: Dee_atomic_ref_xget()
	 * +1: Dee_atomic_ref_cmpxch_inherited() */
	Dee_Decref_n(assumed_old_path, 2);
	return 0;
err_assumed_old_path:
	Dee_Decref_unlikely(assumed_old_path);
err:
	return -1;
}


/* Return the current module path, which is a tuple of absolute,
 * normalized directory names describing where deemon system
 * modules can be found. (Paths have **NO** trailing '/'!)
 * @return: * :   The module path
 * @return: NULL: Error */
PUBLIC WUNUSED DREF /*Tuple*/ DeeObject *DCALL DeeModule_GetLibPath(void) {
	DREF DeeObject *result;
again:
	result = Dee_atomic_ref_xget(&deemon_path);
	if (result == NULL) {
		/* Lazily generate+cache default path */
		result = Dee_AsObject(DeeModule_NewDefaultPath());
		if (result) {
			Dee_Incref(result);
			if (!Dee_atomic_ref_xcmpxch_inherited(&deemon_path, NULL, result)) {
				Dee_Decref_n(result, 2);
				goto again;
			}
		}
	}
	return result;
}

/* Set (or reset when "new_libpath == NULL") the module path.
 * - Assumes that "new_libpath" is a tuple
 * - Throws an error if any element of "new_libpath" isn't a string
 * - Normalizes given paths using `DeeSystem_MakeNormalAndAbsolute()'
 * - Removes duplicate paths (but retains order of distinct paths)
 * @return: 0 : Success (always returned when "new_libpath == NULL")
 * @return: -1: Error */
PUBLIC WUNUSED int DCALL
DeeModule_SetLibPath(/*Tuple*/ DeeObject *new_libpath) {
	/* Normalize paths. */
	size_t i;
	DREF DeeTupleObject *used_new_libpath;
	ASSERT_OBJECT_TYPE_EXACT(new_libpath, &DeeTuple_Type);

	/* Assert that all items are strings. */
	for (i = 0; i < DeeTuple_SIZE(new_libpath); ++i) {
		DeeObject *path = DeeTuple_GET(new_libpath, i);
		if (DeeObject_AssertTypeExact(path, &DeeString_Type))
			goto err;
	}

	/* Normalize all paths. */
	used_new_libpath = (DREF DeeTupleObject *)new_libpath;
	Dee_Incref(used_new_libpath);
	for (i = 0; i < DeeTuple_SIZE(used_new_libpath); ++i) {
		DeeObject *path = DeeTuple_GET(used_new_libpath, i);
		DREF DeeObject *normal_path = DeeSystem_MakeNormalAndAbsolute(path);
		if likely(normal_path == path) {
			Dee_DecrefNokill(normal_path);
			continue;
		}

		/* Must replace "path" with "normal_path" in "used_new_libpath" */
		used_new_libpath = DeeTuple_Unshare(used_new_libpath);
		if unlikely(!used_new_libpath)
			goto err;

		ASSERT(path == DeeTuple_GET(used_new_libpath, i));
		Dee_DecrefNokill(path); /* *Nokill because still referenced by "new_libpath" */
		DeeTuple_SET(used_new_libpath, i, normal_path); /* Inherit reference */
	}

	used_new_libpath = DeeTuple_RemoveDuplicateStrings(used_new_libpath);
	if unlikely(!used_new_libpath)
		goto err;
	return DeeModule_ApplyLibPath(NULL, used_new_libpath, NULL);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_DeeModule_AddLibPath(DeeStringObject *__restrict path) {
	int status;
	DREF DeeTupleObject *old_libpath, *new_libpath;
again:
	old_libpath = (DREF DeeTupleObject *)DeeModule_GetLibPath();
	if unlikely(!old_libpath)
		goto err;
	new_libpath = DeeTuple_AddDistinctFSString(old_libpath, path);
	if unlikely(!new_libpath)
		goto err;
	if (old_libpath == new_libpath) {
		Dee_Decref_unlikely(new_libpath);
		return 0;
	}
	status = DeeModule_ApplyLibPath(old_libpath, new_libpath, (DeeStringObject *)ITER_DONE);
	if (status != 0) {
		if unlikely(status < 0)
			goto err;
		goto again; /* Try again */
	}
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_DeeModule_RemoveLibPath(DeeStringObject *__restrict path) {
	int status;
	DREF DeeTupleObject *old_libpath, *new_libpath;
again:
	old_libpath = (DREF DeeTupleObject *)DeeModule_GetLibPath();
	if unlikely(!old_libpath)
		goto err;
	new_libpath = DeeTuple_RemoveDistinctFSString(old_libpath, path);
	if unlikely(!new_libpath)
		goto err;
	if (old_libpath == new_libpath) {
		Dee_Decref_unlikely(new_libpath);
		return 0;
	}
	status = DeeModule_ApplyLibPath(old_libpath, new_libpath, path);
	if (status != 0) {
		if unlikely(status < 0)
			goto err;
		goto again; /* Try again */
	}
	return 1;
err:
	return -1;
}

/* Add or remove a new module path
 * @return: 1 : Given path was added (or removed)
 * @return: 0 : Nothing happened (path was already added, or removed)
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_AddLibPath(/*String*/ DeeObject *__restrict path) {
	int result;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	path = DeeSystem_MakeNormalAndAbsolute(path);
	if unlikely(!path)
		goto err;
	result = do_DeeModule_AddLibPath((DeeStringObject *)path);
	Dee_Decref_unlikely(path);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_RemoveLibPath(/*String*/ DeeObject *__restrict path) {
	int result;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	path = DeeSystem_MakeNormalAndAbsolute(path);
	if unlikely(!path)
		goto err;
	result = do_DeeModule_RemoveLibPath((DeeStringObject *)path);
	Dee_Decref_unlikely(path);
	return result;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_AddLibPathString(/*utf-8*/ char const *__restrict path) {
	return DeeModule_AddLibPathStringLen(path, strlen(path));
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_AddLibPathStringLen(/*utf-8*/ char const *__restrict path, size_t path_len) {
	int result;
	DREF DeeObject *pathob;
	pathob = DeeString_NewUtf8(path, path_len, STRING_ERROR_FSTRICT);
	if unlikely(!pathob)
		goto err;
	result = DeeModule_AddLibPath(pathob);
	Dee_Decref(pathob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_RemoveLibPathString(/*utf-8*/ char const *__restrict path) {
	return DeeModule_RemoveLibPathStringLen(path, strlen(path));
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_RemoveLibPathStringLen(/*utf-8*/ char const *__restrict path, size_t path_len) {
	int result;
	DREF DeeObject *pathob;
	pathob = DeeString_NewUtf8(path, path_len, STRING_ERROR_FSTRICT);
	if unlikely(!pathob)
		goto err;
	result = DeeModule_RemoveLibPath(pathob);
	Dee_Decref(pathob);
	return result;
err:
	return -1;
}


#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
PUBLIC DeeListObject DeeModule_Path = {
	OBJECT_HEAD_INIT(&DeeList_Type),
	/* .l_list = */ DEE_OBJECTLIST_INIT
#ifndef CONFIG_NO_THREADS
	,
	/* .l_lock = */ Dee_ATOMIC_RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
};

PRIVATE void DCALL do_init_module_path(void) {
	int error;
#ifndef CONFIG_NO_DEEMON_PATH_ENVIRON
	{
		char const *path;
		DREF DeeObject *path_part;
#ifndef CONFIG_DEEMON_PATH_ENVIRON
#define CONFIG_DEEMON_PATH_ENVIRON "DEEMON_PATH"
#endif /* !CONFIG_DEEMON_PATH_ENVIRON */
		DBG_ALIGNMENT_DISABLE();
		path = getenv(CONFIG_DEEMON_PATH_ENVIRON);
		DBG_ALIGNMENT_ENABLE();
		if (path) {
			while (*path) {
				/* Split the module path. */
				char const *next_path = strchr(path, DeeSystem_DELIM);
				if (next_path) {
					path_part = DeeString_NewUtf8(path,
					                              (size_t)(next_path - path),
					                              STRING_ERROR_FIGNORE);
					++next_path;
				} else {
					next_path = strend(path);
					path_part = DeeString_NewUtf8(path,
					                              (size_t)(next_path - path),
					                              STRING_ERROR_FIGNORE);
				}
				if unlikely(!path_part)
					goto init_error;
				error = DeeList_Append(Dee_AsObject(&DeeModule_Path), path_part);
				Dee_Decref(path_part);
				if unlikely(error)
					goto init_error;
				path = next_path;
			}
		}
	}
#endif /* !CONFIG_NO_DEEMON_PATH_ENVIRON */
#ifdef CONFIG_DEEMON_PATH
#define APPEND_PATH(str)                                        \
	{                                                           \
		PRIVATE DEFINE_STRING(_libpath_string, str);            \
		error = DeeList_Append(Dee_AsObject(&DeeModule_Path),   \
		                       Dee_AsObject(&_libpath_string)); \
		if unlikely(error)                                      \
			goto init_error;                                    \
	}
	CONFIG_DEEMON_PATH(APPEND_PATH)
#undef APPEND_PATH
#else /* CONFIG_DEEMON_PATH */
	/* Add the default path based on deemon-home. */
	{
		DREF DeeObject *default_path;
		default_path = DeeString_Newf("%Klib", DeeExec_GetHome());
		if unlikely(!default_path)
			goto init_error;
		error = DeeList_Append(Dee_AsObject(&DeeModule_Path), default_path);
		Dee_Decref(default_path);
		if unlikely(error)
			goto init_error;
	}
#endif /* !CONFIG_DEEMON_PATH */
	return;
init_error:
	DeeError_Print("Failed to initialize module path",
	               ERROR_PRINT_DOHANDLE);
}



#ifdef CONFIG_NO_THREADS
#define INIT_PENDING 0
#define INIT_COMPLET 1
#else /* CONFIG_NO_THREADS */
#define INIT_PENDING 0
#define INIT_PROGRES 1
#define INIT_COMPLET 2
#endif /* !CONFIG_NO_THREADS */

PRIVATE int module_init_state = INIT_PENDING;
PUBLIC void DCALL DeeModule_InitPath(void) {
	/* Lazily calculate hashes of exported objects upon first access. */
	if unlikely(module_init_state != INIT_COMPLET) {
#ifdef CONFIG_NO_THREADS
		do_init_module_path();
		module_init_state = INIT_COMPLET;
#else /* CONFIG_NO_THREADS */
		COMPILER_READ_BARRIER();
		if (atomic_cmpxch(&module_init_state, INIT_PENDING, INIT_PROGRES)) {
			do_init_module_path();
			atomic_write(&module_init_state, INIT_COMPLET);
		} else {
			while (atomic_read(&module_init_state) != INIT_COMPLET)
				SCHED_YIELD();
		}
#endif /* !CONFIG_NO_THREADS */
	}
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* Given a pointer `ptr' that is either for some statically allocated variable/symbol
 * (as in: a pointer to some statically allocated structure), or is part of some user
 * module's statically allocated memory blob (e.g. the address of a 'DeeStringObject'
 * that is a constant in user-code), try to return a reference for the module that
 * contains this pointer (only when CONFIG_EXPERIMENTAL_MMAP_DEC).
 *
 * @return: * :   A pointer to the module that 'ptr' belongs to.
 * @return: NULL: Given `ptr' is either invalid, heap-allocated, or simply not part
 *                of the deemon core, some dex module, or a some user-code module. */
PUBLIC WUNUSED DREF DeeModuleObject *DCALL
DeeModule_OfPointer(void const *ptr) {
	DREF DeeModuleObject *result;
	/* Check for a static pointer and mmap'd .dec files */
	module_byaddr_lock_read();
#ifndef HAVE_module_byaddr_tree_STATIC_INIT
	if unlikely(!module_byaddr_is_initialized()) {
		module_byaddr_lock_upgrade();
		module_byaddr_ensure_initialized();
		module_byaddr_lock_downgrade();
	}
#endif /* !HAVE_module_byaddr_tree_STATIC_INIT */

	/* Search "module_byaddr_tree" */
	result = module_byaddr_locate(module_byaddr_tree, (byte_t const *)ptr);
	if (result) {
		ASSERT(Dee_TYPE(result) == &DeeModuleDee_Type ||
		       Dee_TYPE(result) == &DeeModuleDex_Type);
		if (Dee_IncrefIfNotZero(result)) {
			module_byaddr_lock_endread();
			return result;
		}
	}

	/* Search "dex_byaddr_tree" (if present) */
#ifdef HAVE_Dee_dexataddr_t
	if (dex_byaddr_tree != NULL) {
		Dee_dexataddr_t at_addr;
		if (Dee_dexataddr_init_fromaddr(&at_addr, ptr)) {
			result = dex_byaddr_locate(dex_byaddr_tree, &at_addr);
			if (result) {
				ASSERT(Dee_TYPE(result) == &DeeModuleDex_Type);
				Dee_Incref(result);
				module_byaddr_lock_endread();
				return result;
			}
		}
	}
#endif /* HAVE_Dee_dexataddr_t */
	module_byaddr_lock_endread();

	/* No such module... */
	(void)ptr;
	return NULL;
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


/* Extension to `DeeModule_OfPointer()' that checks if `ob' is statically allocated
 * within some specific module. But if it isn't, then it looks at the type of `ob'
 * and tries to return the associated module via type-specific means:
 * - DeeType_Type: DeeTypeObject::tp_module
 * - DeeCode_Type: DeeCodeObject::co_module */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OfObject(DeeObject *__restrict ob) {
	DREF DeeModuleObject *result;
	DeeTypeObject *tp;
	ASSERT_OBJECT(ob);

	/* Check for statically allocated pointers first. */
	result = DeeModule_OfPointer(ob);
	if (result)
		return result;

	tp = Dee_TYPE(ob);
	/* Check for certain types of objects */
	if (DeeType_IsTypeType(tp))
		return DeeType_GetModule((DeeTypeObject *)ob);
	if (DeeType_Extends(tp, &DeeModule_Type))
		return_reference_((DeeModuleObject *)ob); /* The module of a module, is just that same module again! */
	if (tp == &DeeCode_Type) {
		DeeCodeObject *me = (DeeCodeObject *)ob;
		result = me->co_module;
		return_reference_(result);
	}

	/* Unable to determine module... (this happens
	 * for temporary heap objects and-the-like) */
	return NULL;
}


#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* Check if `DeeModule_OfPointer(ptr) == self' (but is a bit faster than that).
 * Use this function instead of looking at `mo_minaddr' / `mo_maxaddr', because
 * this function does some necessarily extra handling for certain types of DEX
 * modules that are loaded in multiple segments (in which case it would not be
 * defined if `mo_minaddr' / `mo_maxaddr' is union of all segments, or only some
 * (sub-)set of segments)
 *
 * NOTE: Unlike many other functions, this one can actually still be used while `self'
 *       is being finalized (e.g. while inside of `Dee_module_dexdata::mdx_fini'). It
 *       also guaranties that no user-code will ever be executed (hence the "PURE")
 *
 * @return: true:  Yes, "ptr" is part of "self"
 * @return: false: No, "ptr" is not part of "self" */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeModule_ContainsPointer(DeeModuleObject *__restrict self, void const *ptr) {
	/* Can't be asserted; we get called during finalization of dex modules */
//	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
#ifdef HAVE_Dee_dexataddr_t
	if (self->mo_flags & _Dee_MODULE_FNOADDR) {
		Dee_dexataddr_t daa;
		if (Dee_dexataddr_init_fromaddr(&daa, ptr)) {
			if (Dee_dexataddr_compare(DeeModule_GetDexAtAddr(self), &daa) == 0)
				return true;
		}
		return false;
	}
#endif /* HAVE_Dee_dexataddr_t */
	return (byte_t *)ptr >= self->mo_minaddr &&
	       (byte_t *)ptr <= self->mo_maxaddr;
}


/* Enumerate loaded modules using various different means.
 *
 * DeeModule_EnumerateAbsTree:
 *     Enumerate all non-anonymous modules (i.e. ones with `mo_absname != NULL').
 *
 * DeeModule_EnumerateLibTree:
 *     Enumerate modules via their "lib" names (e.g. "deemon", "rt", etc.)
 *     Note that this only includes modules whose lib-names are loaded **right now**.
 *     If any changes are mading to the module LIBPATH (e.g. `DeeModule_AddLibPath()'
 *     or `DeeModule_RemoveLibPath()' is called), the lib-names of already-loaded
 *     modules will **NOT** be calculated immediatly, but lazily. And a call to
 *     `DeeModule_EnumerateLibTree()' will **NOT** do this lazy calculation.
 *
 * DeeModule_EnumerateAdrTree:
 *     Enumerate modules that reside within the address space (i.e.: have an
 *     address range as per `mo_minaddr' / `mo_maxaddr'). This essentially means
 *     that all `DeeModuleDee_Type' and `DeeModuleDex_Type' modules (including
 *     the core `DeeModule_Deemon' module) will be enumerated.
 *
 * NOTES:
 * - The order in which modules are enumerated is undefined but will not change
 *   for already-enumerated modules (including modules enumerated during a prior
 *   call to these functions).
 * - Every qualifying module loaded at the time the `DeeModule_Enumerate*' call
 *   started, and still-loaded when this call returns has been passed to `*cb'
 *   exactly once. (Modules that are unloaded and then quickly re-loaded may be
 *   enumerated multiple times however)
 * - None of the `DeeModule_Enumerate*' functions can throw errors on their own.
 *   The only way that some negative value can be returned, is from `cb' returning
 *   that same negative value.
 * - The "opt_type_filter" argument can either be "NULL", or one of:
 *   - DeeModuleDee_Type
 *   - DeeModuleDir_Type
 *   - DeeModuleDex_Type
 *   ... to only enumerate modules with that specific typing.
 * - The `*cb' callback is allowed to do anything it wants, including invoking any
 *   user-code, as well as load additional modules. It is however undefined if modules
 *   that were loaded after the `DeeModule_Enumerate*' call started will also be
 *   enumerated.
 *
 * @param: cb:              The callback that should be invoked
 * @param: arg:             Cookie argument that should be passed to
 * @param: start_after:     Start enumeration with whatever module comes after `start_after'.
 *                          When `NULL', start enumeration at the very beginning.
 * @param: opt_type_filter: Only enumerate modules of this type (set to "NULL" to not filter).
 *
 * @return: * : Sum of return values of `*cb'
 * @return: 0 : Either `*cb' always returned `0', or it was never invoked
 * @return: <0: A call to `*cb' returned this same negative value. */
PUBLIC NONNULL((1)) Dee_ssize_t DCALL
DeeModule_EnumerateAbsTree(Dee_module_enumerate_cb_t cb, void *arg,
                           DeeModuleObject *start_after,
                           DeeTypeObject *opt_type_filter) {
	Dee_ssize_t result;
	DREF DeeModuleObject *prev_module;
	if (start_after) {
		prev_module = start_after;
		if unlikely(!prev_module->mo_absname)
			return 0; /* Not part of "module_abstree_root" */
		Dee_Incref(prev_module);
		module_abstree_lock_read();
		result = 0;
	} else {
		module_abstree_lock_read();
		prev_module = module_abstree_root;
		if unlikely(!prev_module)
			goto empty_unlock; /* Empty tree */
		while (prev_module->mo_absnode.rb_lhs)
			prev_module = prev_module->mo_absnode.rb_lhs;
		while ((opt_type_filter && Dee_TYPE(prev_module) != opt_type_filter) ||
		       !Dee_IncrefIfNotZero(prev_module)) {
			prev_module = module_abstree_nextnode(prev_module);
			if unlikely(!prev_module)
				goto empty_unlock; /* Empty tree */
		}
		module_abstree_lock_endread();
		result = (*cb)(arg, prev_module);
		if unlikely(result < 0)
			goto done_decref;
		module_abstree_lock_read();
	}
	for (;;) {
		Dee_ssize_t temp;
		DREF DeeModuleObject *next_module = prev_module;
		for (;;) {
			ASSERT(next_module->mo_absname);
			next_module = module_abstree_nextnode(next_module);
			if (!next_module)
				goto done_decref_unlock;
			ASSERT(next_module->mo_absname);
			if ((!opt_type_filter || Dee_TYPE(next_module) == opt_type_filter) &&
			    Dee_IncrefIfNotZero(next_module))
				break;
		}
		module_abstree_lock_endread();
		Dee_Decref_unlikely(prev_module);
		temp = (*cb)(arg, next_module);
		prev_module = next_module; /* Inherit reference */
		if unlikely(temp < 0) {
			result = temp;
			goto done_decref;
		}
		result += temp;
		module_abstree_lock_read();
	}
done_decref_unlock:
	module_abstree_lock_endread();
done_decref:
	Dee_Decref_unlikely(prev_module);
	return result;
empty_unlock:
	module_abstree_lock_endread();
	return 0;
}

PUBLIC NONNULL((1)) Dee_ssize_t DCALL
DeeModule_EnumerateAdrTree(Dee_module_enumerate_cb_t cb, void *arg,
                           DeeModuleObject *start_after,
                           DeeTypeObject *opt_type_filter) {
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
#define module_type_hasaddr(t) ((t) == &DeeModuleDee_Type || (t) == &DeeModuleDex_Type)
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
#define module_type_hasaddr(t) ((t) == &DeeModuleDex_Type)
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
	Dee_ssize_t result;
	DREF DeeModuleObject *prev_module;
	if (opt_type_filter) {
		if unlikely(!module_type_hasaddr(opt_type_filter))
			return 0; /* Not part of "module_byaddr_tree" / "dex_byaddr_tree" */
	}
	if (start_after) {
		prev_module = start_after;
		if unlikely(!module_type_hasaddr(Dee_TYPE(prev_module)))
			return 0; /* Not part of "module_byaddr_tree" / "dex_byaddr_tree" */
		Dee_Incref(prev_module);
#ifdef HAVE_Dee_dexataddr_t
		if (Dee_TYPE(prev_module) == &DeeModuleDex_Type &&
		    (prev_module->mo_flags & _Dee_MODULE_FNOADDR)) {
			result = 0;
			goto continue_dex_byaddr_tree_after_prev_module; /* Start within "dex_byaddr_tree" */
		}
#endif /* HAVE_Dee_dexataddr_t */
		module_byaddr_lock_read();
		result = 0;
	} else {
		module_byaddr_lock_read();
		prev_module = module_byaddr_tree;
		if unlikely(!prev_module)
			goto empty_unlock; /* Empty tree */
		while (prev_module->mo_adrnode.rb_lhs)
			prev_module = prev_module->mo_adrnode.rb_lhs;
		while ((opt_type_filter && Dee_TYPE(prev_module) != opt_type_filter) ||
		       !Dee_IncrefIfNotZero(prev_module)) {
			prev_module = module_byaddr_nextnode(prev_module);
			if unlikely(!prev_module)
				goto empty_unlock; /* Empty tree */
		}
		module_byaddr_lock_endread();
		result = (*cb)(arg, prev_module);
		if unlikely(result < 0)
			goto done_decref;
		module_byaddr_lock_read();
	}
	for (;;) {
		Dee_ssize_t temp;
		DREF DeeModuleObject *next_module = prev_module;
		for (;;) {
			ASSERT(!(next_module->mo_flags & _Dee_MODULE_FNOADDR));
			next_module = module_byaddr_nextnode(next_module);
			if (!next_module)
				goto done_decref_unlock;
			ASSERT(!(next_module->mo_flags & _Dee_MODULE_FNOADDR));
			if ((!opt_type_filter || Dee_TYPE(next_module) == opt_type_filter) &&
			    Dee_IncrefIfNotZero(next_module))
				break;
		}
		module_byaddr_lock_endread();
		Dee_Decref_unlikely(prev_module);
		temp = (*cb)(arg, next_module);
		prev_module = next_module; /* Inherit reference */
		if unlikely(temp < 0) {
			result = temp;
			goto done_decref;
		}
		result += temp;
		module_byaddr_lock_read();
	}
done_decref_unlock:
#ifdef HAVE_Dee_dexataddr_t
	if (!opt_type_filter || opt_type_filter == &DeeModuleDex_Type) {
		Dee_ssize_t temp;
		DREF DeeModuleObject *next_module;
		if (!Dee_DecrefIfNotOne(prev_module)) {
			module_byaddr_lock_endread();
			Dee_Decref_unlikely(prev_module);
			module_byaddr_lock_read();
		}
scan_dex_byaddr_tree:
		prev_module = dex_byaddr_tree;
		if unlikely(!prev_module)
			goto dex_empty_unlock;
		while (prev_module->mo_adrnode.rb_lhs)
			prev_module = prev_module->mo_adrnode.rb_lhs;
		Dee_Incref(prev_module);
		module_byaddr_lock_endread();
continue_dex_byaddr_tree_at_prev_module:
		temp = (*cb)(arg, prev_module);
		if unlikely(temp < 0) {
			result = temp;
			goto done_decref;
		}
		result += temp;
continue_dex_byaddr_tree_after_prev_module:
		module_byaddr_lock_read();
		ASSERT(prev_module->mo_flags & _Dee_MODULE_FNOADDR);
		next_module = dex_byaddr_nextnode(prev_module);
		if (next_module) {
			ASSERT(next_module->mo_flags & _Dee_MODULE_FNOADDR);
			Dee_Incref(next_module);
			module_byaddr_lock_endread();
			Dee_Decref_unlikely(prev_module);
			prev_module = next_module;
			goto continue_dex_byaddr_tree_at_prev_module;
		}
	}
#endif /* HAVE_Dee_dexataddr_t */
	module_byaddr_lock_endread();
done_decref:
	Dee_Decref_unlikely(prev_module);
	return result;
empty_unlock:
#ifdef HAVE_Dee_dexataddr_t
	if (!opt_type_filter || opt_type_filter == &DeeModuleDex_Type) {
		result = 0;
		goto scan_dex_byaddr_tree;
	}
#endif /* HAVE_Dee_dexataddr_t */
	module_byaddr_lock_endread();
	return 0;
#ifdef HAVE_Dee_dexataddr_t
dex_empty_unlock:
	module_byaddr_lock_endread();
	return result;
#endif /* HAVE_Dee_dexataddr_t */
#undef module_type_hasaddr
}

PUBLIC NONNULL((1)) Dee_ssize_t DCALL
DeeModule_EnumerateLibTree(Dee_module_enumerate_lib_cb_t cb, void *arg,
                           DeeModuleObject *start_after_mod,
                           /*String*/ DeeObject *start_after_name,
                           DeeTypeObject *opt_type_filter) {
	Dee_ssize_t result = 0;
	struct Dee_module_libentry *iter;
	DREF DeeStringObject *prev_name = NULL;
	if (start_after_mod == &DeeModule_Deemon) {
		start_after_mod  = NULL;
		start_after_name = NULL;
	} else if (start_after_mod == NULL) {
		/* Start by enumerating "deemon" itself */
		result = (*cb)(arg, &DeeModule_Deemon, Dee_AsObject(&str_deemon));
		if unlikely(result < 0)
			return result;
	}

	/* Enumerate all regular modules */
	module_libtree_lock_read();
	if (start_after_mod) {
		iter = &start_after_mod->mo_libname;
		if (start_after_name) {
			ASSERT_OBJECT_TYPE_EXACT(start_after_name, &DeeString_Type);
			while (iter->mle_name != (DeeStringObject *)start_after_name) {
				iter = iter->mle_next;
				if (!iter) {
					/* This can happen if the caller's libname got removed */
					prev_name = (DeeStringObject *)start_after_name;
					Dee_Incref(prev_name);
					goto continue_after_prev_name;
				}
			}
		}
		iter = module_libtree_nextnode(iter);
		if unlikely(!iter)
			goto empty_unlock;
	} else {
		iter = module_libtree_root;
		if unlikely(!iter)
			goto empty_unlock;
		while (iter->mle_node.rb_lhs)
			iter = iter->mle_node.rb_lhs;
	}
continue_at_iter:
	do {
		DREF DeeModuleObject *iter_mod = Dee_module_libentry_getmodule(iter);
		if ((!opt_type_filter || Dee_TYPE(iter_mod) == opt_type_filter) &&
		    Dee_IncrefIfNotZero(iter_mod)) {
			Dee_ssize_t temp;
			DREF DeeStringObject *iter_name = iter->mle_name;
			Dee_Incref(iter_name);
			module_libtree_lock_endread();
			Dee_XDecref_unlikely(prev_name);
			temp = (*cb)(arg, iter_mod, (DeeObject *)iter_name);
			Dee_Decref_unlikely(iter_mod);
			prev_name = iter_name;
			if unlikely(temp < 0) {
				result = temp;
				goto done_unlock;
			}
			result += temp;
			module_libtree_lock_read();

			/* Re-discover the entry for "iter_mod" + "iter_name" */
			iter = &iter_mod->mo_libname;
			while (iter->mle_name != iter_name) {
				iter = iter->mle_next;
				if unlikely(!iter) {
					/* This can happen when the entry for "iter" was removed.
					 * In this case, use the reference in "prev_name" to find
					 * the smallest entry that is still `> prev_name' */
					ASSERT(prev_name);
continue_after_prev_name:
					iter = module_libtree_nextafter(module_libtree_root, prev_name);
					if (!iter)
						goto done_unlock;
					goto continue_at_iter;
				}
			}
		}
	} while ((iter = module_libtree_nextnode(iter)) != NULL);
done_unlock:
	module_libtree_lock_endread();
/*done:*/
	Dee_XDecref_unlikely(prev_name);
	return result;
empty_unlock:
	module_libtree_lock_endread();
	return 0;
}

#if 1 /* Needed for GC memory leak detector */
#ifndef HAVE_Dee_dexataddr_t
INTERN RBTREE_ROOT(DREF Dee_module_object) dex_byaddr_tree = NULL;
#endif /* !HAVE_Dee_dexataddr_t */
#endif




/* Convenience wrappers around `DeeModule_Enumerate*' that return whatever
 * module comes after "prev" (if such a module exists), or "NULL" if no such
 * module exists. When "prev" is "NULL", return the first module of that tree. */
PRIVATE NONNULL((2)) Dee_ssize_t DCALL
module_enumerate_next_cb(void *arg, DeeModuleObject *__restrict mod) {
	Dee_Incref(mod);
	*(DeeModuleObject **)arg = mod;
	return -1; /* Stop enumeration */
}

PUBLIC WUNUSED DREF DeeModuleObject *DCALL
DeeModule_NextAbsTree(DeeModuleObject *prev, DeeTypeObject *opt_type_filter) {
	DREF DeeModuleObject *result = NULL;
	DeeModule_EnumerateAbsTree(&module_enumerate_next_cb, &result, prev, opt_type_filter);
	return result;
}

PUBLIC WUNUSED DREF DeeModuleObject *DCALL
DeeModule_NextAdrTree(DeeModuleObject *prev, DeeTypeObject *opt_type_filter) {
	DREF DeeModuleObject *result = NULL;
	DeeModule_EnumerateAdrTree(&module_enumerate_next_cb, &result, prev, opt_type_filter);
	return result;
}

struct module_enumerate_next_lib_data {
	DREF DeeModuleObject *menld_mod;
	DREF DeeObject       *menld_name;
};

PRIVATE NONNULL((2)) Dee_ssize_t DCALL
module_enumerate_next_lib_cb(void *arg, DeeModuleObject *__restrict mod,
                             /*String*/ DeeObject *libname) {
	struct module_enumerate_next_lib_data *data;
	data = (struct module_enumerate_next_lib_data *)arg;
	data->menld_mod  = mod;
	data->menld_name = libname;
	Dee_Incref(mod);
	Dee_Incref(libname);
	return -1; /* Stop enumeration */
}

PUBLIC WUNUSED NONNULL((4)) DREF DeeModuleObject *DCALL
DeeModule_NextLibTree(DeeModuleObject *prev, /*String*/ DeeObject *prev_libname,
                      DeeTypeObject *opt_type_filter,
                      DREF /*String*/ DeeObject **__restrict p_libname) {
	struct module_enumerate_next_lib_data data;
	data.menld_mod  = NULL;
	data.menld_name = NULL;
	DeeModule_EnumerateLibTree(&module_enumerate_next_lib_cb, &data,
	                           prev, prev_libname, opt_type_filter);
	*p_libname = data.menld_name; /* Inherit reference */
	return data.menld_mod;
}




/* Lookup an external symbol.
 * Convenience function (same as `DeeObject_GetAttr(DeeModule_Import(...), ...)') */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExtern(/*String*/ DeeObject *__restrict module_name,
                    /*String*/ DeeObject *__restrict global_name) {
	DREF DeeObject *result;
	DREF DeeModuleObject *mod;
	mod = DeeModule_Import(module_name, NULL, DeeModule_IMPORT_F_NORMAL);
	if unlikely(!mod)
		goto err;
	result = DeeObject_GetAttr(Dee_AsObject(mod), global_name);
	Dee_Decref_unlikely(mod);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExternString(/*utf-8*/ char const *__restrict module_name,
                          /*utf-8*/ char const *__restrict global_name) {
	DREF DeeObject *result;
	DREF DeeModuleObject *mod;
	mod = DeeModule_ImportString(module_name, strlen(module_name),
	                             NULL, DeeModule_IMPORT_F_NORMAL);
	if unlikely(!mod)
		goto err;
	result = DeeObject_GetAttrString(Dee_AsObject(mod), global_name);
	Dee_Decref_unlikely(mod);
	return result;
err:
	return NULL;
}

/* Helper wrapper for `DeeObject_Call(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExtern(/*String*/ DeeObject *__restrict module_name,
                     /*String*/ DeeObject *__restrict global_name,
                     size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeModuleObject *mod;
	mod = DeeModule_Import(module_name, NULL, DeeModule_IMPORT_F_NORMAL);
	if unlikely(!mod)
		goto err;
	result = DeeObject_CallAttr(Dee_AsObject(mod), global_name, argc, argv);
	Dee_Decref_unlikely(mod);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExternString(/*utf-8*/ char const *__restrict module_name,
                           /*utf-8*/ char const *__restrict global_name,
                           size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeModuleObject *mod;
	mod = DeeModule_ImportString(module_name, strlen(module_name),
	                             NULL, DeeModule_IMPORT_F_NORMAL);
	if unlikely(!mod)
		goto err;
	result = DeeObject_CallAttrString(Dee_AsObject(mod), global_name, argc, argv);
	Dee_Decref_unlikely(mod);
	return result;
err:
	return NULL;
}

/* Helper wrapper for `DeeObject_Callf(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternf(/*String*/ DeeObject *__restrict module_name,
                      /*String*/ DeeObject *__restrict global_name,
                      char const *__restrict format, ...) {
	va_list args;
	DREF DeeObject *result;
	va_start(args, format);
	result = DeeModule_VCallExternf(module_name, global_name, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternStringf(/*utf-8*/ char const *__restrict module_name,
                            /*utf-8*/ char const *__restrict global_name,
                            char const *__restrict format, ...) {
	va_list args;
	DREF DeeObject *result;
	va_start(args, format);
	result = DeeModule_VCallExternStringf(module_name, global_name, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternf(/*String*/ DeeObject *__restrict module_name,
                       /*String*/ DeeObject *__restrict global_name,
                       char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	DREF DeeModuleObject *mod;
	mod = DeeModule_Import(module_name, NULL, DeeModule_IMPORT_F_NORMAL);
	if unlikely(!mod)
		goto err;
	result = DeeObject_VCallAttrf(Dee_AsObject(mod), global_name, format, args);
	Dee_Decref_unlikely(mod);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternStringf(/*utf-8*/ char const *__restrict module_name,
                             /*utf-8*/ char const *__restrict global_name,
                             char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	DREF DeeModuleObject *mod;
	mod = DeeModule_ImportString(module_name, strlen(module_name),
	                             NULL, DeeModule_IMPORT_F_NORMAL);
	if unlikely(!mod)
		goto err;
	result = DeeObject_VCallAttrStringf(Dee_AsObject(mod), global_name, format, args);
	Dee_Decref_unlikely(mod);
	return result;
err:
	return NULL;
}

#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODPATH_C */
