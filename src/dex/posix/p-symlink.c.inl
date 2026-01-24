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
#ifndef GUARD_DEX_POSIX_P_SYMLINK_C_INL
#define GUARD_DEX_POSIX_P_SYMLINK_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_ReleaseSystemMemory */
#include <deemon/arg.h>             /* DEFINE_KWLIST, DeeArg_UnpackStructKw */
#include <deemon/dex.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

/* Figure out how to implement `symlink()' */
#undef posix_symlink_USE_nt_CreateSymbolicLinkAuto
#undef posix_symlink_USE_wsymlink
#undef posix_symlink_USE_symlink
#undef posix_symlink_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_symlink_USE_nt_CreateSymbolicLinkAuto
#elif defined(CONFIG_HAVE_wsymlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_symlink_USE_wsymlink
#elif defined(CONFIG_HAVE_symlink)
#define posix_symlink_USE_symlink
#elif defined(CONFIG_HAVE_wsymlink)
#define posix_symlink_USE_wsymlink
#else /* ... */
#define posix_symlink_USE_STUB
#endif /* !... */



/* Figure out how to implement `fsymlinkat()' */
#undef posix_fsymlinkat_USE_wfsymlinkat
#undef posix_fsymlinkat_USE_wsymlinkat
#undef posix_fsymlinkat_USE_fsymlinkat
#undef posix_fsymlinkat_USE_symlinkat
#undef posix_fsymlinkat_USE_posix_symlink
#undef posix_fsymlinkat_USE_STUB
#if defined(CONFIG_HAVE_wfsymlinkat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_fsymlinkat_USE_wfsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_fsymlinkat)
#define posix_fsymlinkat_USE_fsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_wfsymlinkat)
#define posix_fsymlinkat_USE_wfsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_wsymlinkat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_fsymlinkat_USE_wsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_symlinkat)
#define posix_fsymlinkat_USE_symlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_wsymlinkat)
#define posix_fsymlinkat_USE_wsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif !defined(posix_symlink_USE_STUB)
#define posix_fsymlinkat_USE_posix_symlink
#else /* ... */
#define posix_fsymlinkat_USE_STUB
#endif /* !... */



/* Figure out how to implement `symlinkat()' */
#undef posix_symlinkat_USE_posix_fsymlinkat
#undef posix_symlinkat_USE_STUB
#if !defined(posix_fsymlinkat_USE_STUB)
#define posix_symlinkat_USE_posix_fsymlinkat
#else /* ... */
#define posix_symlinkat_USE_STUB
#endif /* !... */




/************************************************************************/
/* symlink()                                                            */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("_symlink", "text:?Dstring,path:?Dstring", libname: "posix"); ]]]*/
#define POSIX__SYMLINK_DEF          DEX_MEMBER_F("_symlink", &posix__symlink, DEXSYM_READONLY, "(text:?Dstring,path:?Dstring)"),
#define POSIX__SYMLINK_DEF_DOC(doc) DEX_MEMBER_F("_symlink", &posix__symlink, DEXSYM_READONLY, "(text:?Dstring,path:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix__symlink_f_impl(DeeObject *text, DeeObject *path);
#ifndef DEFINED_kwlist__text_path
#define DEFINED_kwlist__text_path
PRIVATE DEFINE_KWLIST(kwlist__text_path, { KEX("text", 0xc624ae24, 0x2a28a0084dd3a743), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__text_path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__symlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *text;
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__text_path, "oo:_symlink", &args))
		goto err;
	return posix__symlink_f_impl(args.text, args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix__symlink, &posix__symlink_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix__symlink_f_impl(DeeObject *text, DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_symlink_USE_nt_CreateSymbolicLinkAuto
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(text, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
	error = nt_CreateSymbolicLinkAuto(path, text);
#define NEED_nt_CreateSymbolicLinkAuto
	if likely(error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt() == 0)
			goto again;
		goto err;
	} else if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_ReleaseSystemMemory())
			goto again;
		goto err;
	}
	err_nt_symlink(dwError, text, path);
#define NEED_err_nt_symlink
err:
	return NULL;
#endif /* posix_symlink_USE_nt_CreateSymbolicLinkAuto */

#if defined(posix_symlink_USE_wsymlink) || defined(posix_symlink_USE_symlink)
	int error;
#ifdef posix_symlink_USE_wsymlink
	Dee_wchar_t const *wide_text, *wide_path;
#endif /* posix_symlink_USE_wsymlink */
#ifdef posix_symlink_USE_symlink
	char const *utf8_text, *utf8_path;
#endif /* posix_symlink_USE_symlink */
	if (DeeObject_AssertTypeExact(text, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_symlink_USE_wsymlink
	wide_text = DeeString_AsWide(text);
	if unlikely(!wide_text)
		goto err;
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_symlink_USE_wsymlink */
#ifdef posix_symlink_USE_symlink
	utf8_text = DeeString_AsUtf8(text);
	if unlikely(!utf8_text)
		goto err;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_symlink_USE_symlink */

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_symlink_USE_wsymlink
	error = wsymlink((wchar_t *)wide_text, (wchar_t *)wide_path);
#endif /* posix_symlink_USE_wsymlink */
#ifdef posix_symlink_USE_symlink
	error = symlink((char *)utf8_text, (char *)utf8_path);
#endif /* posix_symlink_USE_symlink */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error != 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_symlink(error, text, path);
#define NEED_err_unix_symlink
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_symlink_USE_wsymlink || posix_symlink_USE_symlink */

#ifdef posix_symlink_USE_STUB
#define NEED_posix_err_unsupported
	(void)text;
	(void)path;
	posix_err_unsupported("symlink");
	return NULL;
#endif /* posix_symlink_USE_STUB */
}




/************************************************************************/
/* fsymlinkat()                                                         */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("_fsymlinkat", "text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix"); ]]]*/
#define POSIX__FSYMLINKAT_DEF          DEX_MEMBER_F("_fsymlinkat", &posix__fsymlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)"),
#define POSIX__FSYMLINKAT_DEF_DOC(doc) DEX_MEMBER_F("_fsymlinkat", &posix__fsymlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix__fsymlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path, unsigned int atflags);
#ifndef DEFINED_kwlist__text_dfd_path_atflags
#define DEFINED_kwlist__text_dfd_path_atflags
PRIVATE DEFINE_KWLIST(kwlist__text_dfd_path_atflags, { KEX("text", 0xc624ae24, 0x2a28a0084dd3a743), KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__text_dfd_path_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__fsymlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *text;
		DeeObject *dfd;
		DeeObject *path;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__text_dfd_path_atflags, "ooo|u:_fsymlinkat", &args))
		goto err;
	return posix__fsymlinkat_f_impl(args.text, args.dfd, args.path, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix__fsymlinkat, &posix__fsymlinkat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix__fsymlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_fsymlinkat_USE_posix_symlink
	DREF DeeObject *abspath, *result;
#if (defined(posix_fsymlinkat_USE_wfsymlinkat) || defined(posix_fsymlinkat_USE_wsymlinkat) || \
     defined(posix_fsymlinkat_USE_fsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat))
	if (!DeeString_Check(dfd) &&
#if defined(posix_fsymlinkat_USE_wsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat)
	    atflags == 0 &&
#endif /* posix_fsymlinkat_USE_wsymlinkat || posix_fsymlinkat_USE_symlinkat */
	    1) {
#if defined(posix_fsymlinkat_USE_wfsymlinkat) || defined(posix_fsymlinkat_USE_wsymlinkat)
		Dee_wchar_t const *wide_text, *wide_path;
#endif /* posix_fsymlinkat_USE_wfsymlinkat || posix_fsymlinkat_USE_wsymlinkat */
#if defined(posix_fsymlinkat_USE_fsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat)
		char const *utf8_text, *utf8_path;
#endif /* posix_fsymlinkat_USE_fsymlinkat || posix_fsymlinkat_USE_symlinkat */
		int error;
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
#if defined(posix_fsymlinkat_USE_wfsymlinkat) || defined(posix_fsymlinkat_USE_wsymlinkat)
		wide_text = DeeString_AsWide(text);
		if unlikely(!wide_text)
			goto err;
		wide_path = DeeString_AsWide(path);
		if unlikely(!wide_path)
			goto err;
#endif /* posix_fsymlinkat_USE_wfsymlinkat || posix_fsymlinkat_USE_wsymlinkat */
#if defined(posix_fsymlinkat_USE_fsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat)
		utf8_text = DeeString_AsUtf8(text);
		if unlikely(!utf8_text)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
#endif /* posix_fsymlinkat_USE_fsymlinkat || posix_fsymlinkat_USE_symlinkat */
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
#if defined(posix_fsymlinkat_USE_wfsymlinkat)
		error = wfsymlinkat((wchar_t *)wide_text, os_dfd, (wchar_t *)wide_path, atflags);
#elif defined(posix_fsymlinkat_USE_wsymlinkat)
		error = wsymlinkat((wchar_t *)wide_text, os_dfd, (wchar_t *)wide_path);
#elif defined(posix_fsymlinkat_USE_fsymlinkat)
		error = fsymlinkat((char *)utf8_text, os_dfd, (char *)utf8_path, atflags);
#elif defined(posix_fsymlinkat_USE_symlinkat)
		error = symlinkat((char *)utf8_text, os_dfd, (char *)utf8_path);
#endif /* ... */
		if likely(error == 0) {
			DBG_ALIGNMENT_ENABLE();
			return_none;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		/* fallthru to the fallback path below */
	}
#endif /* posix_symlinkat_USE_wsymlinkat || posix_symlinkat_USE_symlinkat */
	abspath = posix_dfd_makepath(dfd, path, atflags);
#define NEED_posix_dfd_makepath
	if unlikely(!abspath)
		goto err;
	result = posix__symlink_f_impl(text, abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fsymlinkat_USE_posix_symlink */

#ifdef posix_fsymlinkat_USE_STUB
#define NEED_posix_err_unsupported
	(void)text;
	(void)dfd;
	(void)path;
	(void)atflags;
	posix_err_unsupported("fsymlinkat");
	return NULL;
#endif /* posix_fsymlinkat_USE_STUB */
}


/************************************************************************/
/* symlinkat()                                                          */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("_symlinkat", "text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring", libname: "posix"); ]]]*/
#define POSIX__SYMLINKAT_DEF          DEX_MEMBER_F("_symlinkat", &posix__symlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)"),
#define POSIX__SYMLINKAT_DEF_DOC(doc) DEX_MEMBER_F("_symlinkat", &posix__symlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix__symlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path);
#ifndef DEFINED_kwlist__text_dfd_path
#define DEFINED_kwlist__text_dfd_path
PRIVATE DEFINE_KWLIST(kwlist__text_dfd_path, { KEX("text", 0xc624ae24, 0x2a28a0084dd3a743), KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__text_dfd_path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__symlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *text;
		DeeObject *dfd;
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__text_dfd_path, "ooo:_symlinkat", &args))
		goto err;
	return posix__symlinkat_f_impl(args.text, args.dfd, args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix__symlinkat, &posix__symlinkat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix__symlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_symlinkat_USE_posix_fsymlinkat
	return posix__fsymlinkat_f_impl(text, dfd, path, 0);
#endif /* posix_symlinkat_USE_posix_fsymlinkat */

#ifdef posix_symlinkat_USE_STUB
#define NEED_posix_err_unsupported
	(void)text;
	(void)dfd;
	(void)path;
	posix_err_unsupported("symlinkat");
	return NULL;
#endif /* posix_symlinkat_USE_STUB */
}



#if defined(posix_symlink_USE_nt_CreateSymbolicLinkAuto) || defined(__DEEMON__)
/* The `nt_CreateSymbolicLinkAuto()' impl works just as intended.
 * However, it works a little bit *too well*:
 * >> try mkdir("out"); catch (...);
 * >> with (local fp = File.open("out/data.txt", "w"))
 * >>     fp.write("DATA!");
 * >> symlink(r"out/data.txt", "out-data1.txt");
 * >> symlink(r"out\data.txt", "out-data2.txt");
 * >> local fp1 = try File.open("out-data1.txt", "r") catch (...) none;
 * >> local fp2 = try File.open("out-data2.txt", "r") catch (...) none;
 * >> print repr fp1; // "none" (on windows) -- because '/' isn't parsed by windows in symlinks
 * >> print repr fp2;
 *
 * Solution: we define `_symlink()' as the *true* symlink function
 *           we define `symlink()' as an alias that does OS-specific fix-ups on link text
 *
 * Fun fact:
 * In the above example, windows itself is unable to open `out-data1.txt'.
 * However, cygwin *is* actually able to open that file (iow: cygwin's
 * path evaluation engine *does* actually accept '/' as an alias for '\'
 * in native windows symlinks)
 */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_nt_symlink_normalize_text(DeeObject *__restrict text) {
	struct unicode_printer printer;
	char const *utf8_text, *iter, *end, *flush_start;
	if (DeeObject_AssertTypeExact(text, &DeeString_Type))
		goto err;
	/* Quick check: if the input text doesn't contain
	 * any forward-slashes, we've got nothing to fix! */
	if (!memchr(DeeString_STR(text), '/', DeeString_SIZE(text) * sizeof(char)))
		return_reference_(text);
	utf8_text = DeeString_AsUtf8(text);
	if unlikely(!utf8_text)
		goto err;
	unicode_printer_init(&printer);
	iter = flush_start = utf8_text;
	end  = utf8_text + WSTR_LENGTH(utf8_text);
	while (iter < end) {
		char ch = *iter;
		if (ch == '/') {
			if unlikely(unicode_printer_print(&printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err_printer;
			if unlikely(unicode_printer_putascii(&printer, '\\'))
				goto err_printer;
			++iter;
			flush_start = iter;
		} else {
			++iter;
		}
	}
	if unlikely(unicode_printer_print(&printer, flush_start, (size_t)(end - flush_start)) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("symlink", "text:?Dstring,path:?Dstring", libname: "posix"); ]]]*/
#define POSIX_SYMLINK_DEF          DEX_MEMBER_F("symlink", &posix_symlink, DEXSYM_READONLY, "(text:?Dstring,path:?Dstring)"),
#define POSIX_SYMLINK_DEF_DOC(doc) DEX_MEMBER_F("symlink", &posix_symlink, DEXSYM_READONLY, "(text:?Dstring,path:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_symlink_f_impl(DeeObject *text, DeeObject *path);
#ifndef DEFINED_kwlist__text_path
#define DEFINED_kwlist__text_path
PRIVATE DEFINE_KWLIST(kwlist__text_path, { KEX("text", 0xc624ae24, 0x2a28a0084dd3a743), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__text_path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_symlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *text;
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__text_path, "oo:symlink", &args))
		goto err;
	return posix_symlink_f_impl(args.text, args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_symlink, &posix_symlink_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_symlink_f_impl(DeeObject *text, DeeObject *path)
/*[[[end]]]*/
{
	DREF DeeObject *normal_text, *result;
	normal_text = posix_nt_symlink_normalize_text(text);
	if unlikely(!normal_text)
		goto err;
	result = posix__symlink_f_impl(normal_text, path);
	Dee_Decref(normal_text);
	return result;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("fsymlinkat", "text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix"); ]]]*/
#define POSIX_FSYMLINKAT_DEF          DEX_MEMBER_F("fsymlinkat", &posix_fsymlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)"),
#define POSIX_FSYMLINKAT_DEF_DOC(doc) DEX_MEMBER_F("fsymlinkat", &posix_fsymlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix_fsymlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path, unsigned int atflags);
#ifndef DEFINED_kwlist__text_dfd_path_atflags
#define DEFINED_kwlist__text_dfd_path_atflags
PRIVATE DEFINE_KWLIST(kwlist__text_dfd_path_atflags, { KEX("text", 0xc624ae24, 0x2a28a0084dd3a743), KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__text_dfd_path_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsymlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *text;
		DeeObject *dfd;
		DeeObject *path;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__text_dfd_path_atflags, "ooo|u:fsymlinkat", &args))
		goto err;
	return posix_fsymlinkat_f_impl(args.text, args.dfd, args.path, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fsymlinkat, &posix_fsymlinkat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix_fsymlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
	DREF DeeObject *normal_text, *result;
	normal_text = posix_nt_symlink_normalize_text(text);
	if unlikely(!normal_text)
		goto err;
	result = posix__fsymlinkat_f_impl(normal_text, dfd, path, atflags);
	Dee_Decref(normal_text);
	return result;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("symlinkat", "text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring", libname: "posix"); ]]]*/
#define POSIX_SYMLINKAT_DEF          DEX_MEMBER_F("symlinkat", &posix_symlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)"),
#define POSIX_SYMLINKAT_DEF_DOC(doc) DEX_MEMBER_F("symlinkat", &posix_symlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix_symlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path);
#ifndef DEFINED_kwlist__text_dfd_path
#define DEFINED_kwlist__text_dfd_path
PRIVATE DEFINE_KWLIST(kwlist__text_dfd_path, { KEX("text", 0xc624ae24, 0x2a28a0084dd3a743), KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__text_dfd_path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_symlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *text;
		DeeObject *dfd;
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__text_dfd_path, "ooo:symlinkat", &args))
		goto err;
	return posix_symlinkat_f_impl(args.text, args.dfd, args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_symlinkat, &posix_symlinkat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL posix_symlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path)
/*[[[end]]]*/
{
	return posix_fsymlinkat_f_impl(text, dfd, path, 0);
}


#else /* posix_symlink_USE_nt_CreateSymbolicLinkAuto */
#define POSIX_SYMLINK_DEF             DEX_MEMBER_F("symlink", &posix__symlink, DEXSYM_READONLY, "(text:?Dstring,path:?Dstring)"),
#define POSIX_SYMLINK_DEF_DOC(doc)    DEX_MEMBER_F("symlink", &posix__symlink, DEXSYM_READONLY, "(text:?Dstring,path:?Dstring)\n" doc),
#define POSIX_FSYMLINKAT_DEF          DEX_MEMBER_F("fsymlinkat", &posix__fsymlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)"),
#define POSIX_FSYMLINKAT_DEF_DOC(doc) DEX_MEMBER_F("fsymlinkat", &posix__fsymlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)\n" doc),
#define POSIX_SYMLINKAT_DEF           DEX_MEMBER_F("symlinkat", &posix__symlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)"),
#define POSIX_SYMLINKAT_DEF_DOC(doc)  DEX_MEMBER_F("symlinkat", &posix__symlinkat, DEXSYM_READONLY, "(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)\n" doc),
#endif /* !posix_symlink_USE_nt_CreateSymbolicLinkAuto */


DECL_END

#endif /* !GUARD_DEX_POSIX_P_SYMLINK_C_INL */
