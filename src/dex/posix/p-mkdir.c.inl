/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_MKDIR_C_INL
#define GUARD_DEX_POSIX_P_MKDIR_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Figure out how to implement `mkdir()' */
#undef posix_mkdir_USE_nt_CreateDirectory
#undef posix_mkdir_USE_wmkdir
#undef posix_mkdir_USE_mkdir
#undef posix_mkdir_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_mkdir_USE_nt_CreateDirectory
#elif defined(CONFIG_HAVE_wmkdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_mkdir_USE_wmkdir
#elif defined(CONFIG_HAVE_mkdir)
#define posix_mkdir_USE_mkdir
#elif defined(CONFIG_HAVE_wmkdir)
#define posix_mkdir_USE_wmkdir
#else /* ... */
#define posix_mkdir_USE_STUB
#endif /* !... */



/* Figure out how to implement `fmkdirat()' */
#undef posix_fmkdirat_USE_wfmkdirat
#undef posix_fmkdirat_USE_wmkdirat
#undef posix_fmkdirat_USE_fmkdirat
#undef posix_fmkdirat_USE_mkdirat
#undef posix_fmkdirat_USE_posix_mkdir
#undef posix_fmkdirat_USE_STUB
#if defined(CONFIG_HAVE_wfmkdirat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_fmkdirat_USE_wfmkdirat
#define posix_fmkdirat_USE_posix_mkdir
#elif defined(CONFIG_HAVE_fmkdirat)
#define posix_fmkdirat_USE_fmkdirat
#define posix_fmkdirat_USE_posix_mkdir
#elif defined(CONFIG_HAVE_wfmkdirat)
#define posix_fmkdirat_USE_wfmkdirat
#define posix_fmkdirat_USE_posix_mkdir
#elif defined(CONFIG_HAVE_wmkdirat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_fmkdirat_USE_wmkdirat
#define posix_fmkdirat_USE_posix_mkdir
#elif defined(CONFIG_HAVE_mkdirat)
#define posix_fmkdirat_USE_mkdirat
#define posix_fmkdirat_USE_posix_mkdir
#elif defined(CONFIG_HAVE_wmkdirat)
#define posix_fmkdirat_USE_wmkdirat
#define posix_fmkdirat_USE_posix_mkdir
#elif !defined(posix_mkdir_USE_STUB)
#define posix_fmkdirat_USE_posix_mkdir
#else /* ... */
#define posix_fmkdirat_USE_STUB
#endif /* !... */



/* Figure out how to implement `mkdirat()' */
#undef posix_mkdirat_USE_posix_fmkdirat
#undef posix_mkdirat_USE_STUB
#if !defined(posix_fmkdirat_USE_STUB)
#define posix_mkdirat_USE_posix_fmkdirat
#else /* ... */
#define posix_mkdirat_USE_STUB
#endif /* !... */




/************************************************************************/
/* mkdir()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("mkdir", "path:?Dstring,mode:u=0755", libname: "posix"); ]]]*/
#define POSIX_MKDIR_DEF          DEX_MEMBER_F("mkdir", &posix_mkdir, DEXSYM_READONLY, "(path:?Dstring,mode=!0755)"),
#define POSIX_MKDIR_DEF_DOC(doc) DEX_MEMBER_F("mkdir", &posix_mkdir, DEXSYM_READONLY, "(path:?Dstring,mode=!0755)\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_mkdir_f_impl(DeeObject *path, unsigned int mode);
#ifndef DEFINED_kwlist__path_mode
#define DEFINED_kwlist__path_mode
PRIVATE DEFINE_KWLIST(kwlist__path_mode, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__path_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_mkdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
		unsigned int mode;
	} args;
	args.mode = 0755;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_mode, "o|u:mkdir", &args))
		goto err;
	return posix_mkdir_f_impl(args.path, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_mkdir, &posix_mkdir_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_mkdir_f_impl(DeeObject *path, unsigned int mode)
/*[[[end]]]*/
{
#ifdef posix_mkdir_USE_nt_CreateDirectory
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
#define NEED_nt_CreateDirectory
	error = nt_CreateDirectory(path, NULL); /* TODO: Initial security attributes. */
	if likely(error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}
#define NEED_err_nt_mkdir
	err_nt_mkdir(dwError, path, mode);
err:
	return NULL;
#endif /* posix_mkdir_USE_nt_CreateDirectory */

#if defined(posix_mkdir_USE_wmkdir) || defined(posix_mkdir_USE_mkdir)
	int error;
#ifdef posix_mkdir_USE_wmkdir
	Dee_wchar_t const *wide_path;
#endif /* posix_mkdir_USE_wmkdir */
#ifdef posix_mkdir_USE_mkdir
	char const *utf8_path;
#endif /* posix_mkdir_USE_mkdir */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_mkdir_USE_wmkdir
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_mkdir_USE_wmkdir */
#ifdef posix_mkdir_USE_mkdir
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_mkdir_USE_mkdir */

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_mkdir_USE_wmkdir
	error = wmkdir((wchar_t *)wide_path, mode);
#endif /* posix_mkdir_USE_wmkdir */
#ifdef posix_mkdir_USE_mkdir
	error = mkdir((char *)utf8_path, mode);
#endif /* posix_mkdir_USE_mkdir */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error != 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_mkdir(error, path, mode);
#define NEED_err_unix_mkdir
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_mkdir_USE_wmkdir || posix_mkdir_USE_mkdir */

#ifdef posix_mkdir_USE_STUB
#define NEED_posix_err_unsupported
	(void)path;
	(void)mode;
	posix_err_unsupported("mkdir");
	return NULL;
#endif /* posix_mkdir_USE_STUB */
}




/************************************************************************/
/* fmkdirat()                                                           */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("fmkdirat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:u=0755,atflags:u=0", libname: "posix"); ]]]*/
#define POSIX_FMKDIRAT_DEF          DEX_MEMBER_F("fmkdirat", &posix_fmkdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode=!0755,atflags=!0)"),
#define POSIX_FMKDIRAT_DEF_DOC(doc) DEX_MEMBER_F("fmkdirat", &posix_fmkdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode=!0755,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_fmkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode, unsigned int atflags);
#ifndef DEFINED_kwlist__dfd_path_mode_atflags
#define DEFINED_kwlist__dfd_path_mode_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_path_mode_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_path_mode_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fmkdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *path;
		unsigned int mode;
		unsigned int atflags;
	} args;
	args.mode = 0755;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_path_mode_atflags, "oo|uu:fmkdirat", &args))
		goto err;
	return posix_fmkdirat_f_impl(args.dfd, args.path, args.mode, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fmkdirat, &posix_fmkdirat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_fmkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_fmkdirat_USE_posix_mkdir
	DREF DeeObject *abspath, *result;
#if (defined(posix_fmkdirat_USE_wfmkdirat) || defined(posix_fmkdirat_USE_wmkdirat) || \
     defined(posix_fmkdirat_USE_fmkdirat) || defined(posix_fmkdirat_USE_mkdirat))
	if (!DeeString_Check(dfd) &&
#if defined(posix_fmkdirat_USE_wmkdirat) || defined(posix_fmkdirat_USE_mkdirat)
	    atflags == 0 &&
#endif /* posix_fmkdirat_USE_wmkdirat || posix_fmkdirat_USE_mkdirat */
	    1) {
#if defined(posix_fmkdirat_USE_wfmkdirat) || defined(posix_fmkdirat_USE_wmkdirat)
		Dee_wchar_t const *wide_path;
#endif /* posix_fmkdirat_USE_wfmkdirat || posix_fmkdirat_USE_wmkdirat */
#if defined(posix_fmkdirat_USE_fmkdirat) || defined(posix_fmkdirat_USE_mkdirat)
		char const *utf8_path;
#endif /* posix_fmkdirat_USE_fmkdirat || posix_fmkdirat_USE_mkdirat */
		int error;
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
#if defined(posix_fmkdirat_USE_wfmkdirat) || defined(posix_fmkdirat_USE_wmkdirat)
		wide_path = DeeString_AsWide(path);
		if unlikely(!wide_path)
			goto err;
#endif /* posix_fmkdirat_USE_wfmkdirat || posix_fmkdirat_USE_wmkdirat */
#if defined(posix_fmkdirat_USE_fmkdirat) || defined(posix_fmkdirat_USE_mkdirat)
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
#endif /* posix_fmkdirat_USE_fmkdirat || posix_fmkdirat_USE_mkdirat */
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
#if defined(posix_fmkdirat_USE_wfmkdirat)
		error = wfmkdirat(os_dfd, (wchar_t *)wide_path, mode, atflags);
#elif defined(posix_fmkdirat_USE_wmkdirat)
		error = wmkdirat(os_dfd, (wchar_t *)wide_path, mode);
#elif defined(posix_fmkdirat_USE_fmkdirat)
		error = fmkdirat(os_dfd, (char *)utf8_path, mode, atflags);
#elif defined(posix_fmkdirat_USE_mkdirat)
		error = mkdirat(os_dfd, (char *)utf8_path, mode);
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
#endif /* posix_mkdirat_USE_wmkdirat || posix_mkdirat_USE_mkdirat */
	abspath = posix_dfd_makepath(dfd, path, atflags);
#define NEED_posix_dfd_makepath
	if unlikely(!abspath)
		goto err;
	result = posix_mkdir_f_impl(abspath, mode);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fmkdirat_USE_posix_mkdir */

#ifdef posix_fmkdirat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)path;
	(void)mode;
	(void)atflags;
	posix_err_unsupported("fmkdirat");
	return NULL;
#endif /* posix_fmkdirat_USE_STUB */
}


/************************************************************************/
/* mkdirat()                                                            */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("mkdirat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:u=0755", libname: "posix"); ]]]*/
#define POSIX_MKDIRAT_DEF          DEX_MEMBER_F("mkdirat", &posix_mkdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode=!0755)"),
#define POSIX_MKDIRAT_DEF_DOC(doc) DEX_MEMBER_F("mkdirat", &posix_mkdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode=!0755)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_mkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode);
#ifndef DEFINED_kwlist__dfd_path_mode
#define DEFINED_kwlist__dfd_path_mode
PRIVATE DEFINE_KWLIST(kwlist__dfd_path_mode, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__dfd_path_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_mkdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *path;
		unsigned int mode;
	} args;
	args.mode = 0755;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_path_mode, "oo|u:mkdirat", &args))
		goto err;
	return posix_mkdirat_f_impl(args.dfd, args.path, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_mkdirat, &posix_mkdirat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_mkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode)
/*[[[end]]]*/
{
#ifdef posix_mkdirat_USE_posix_fmkdirat
	return posix_fmkdirat_f_impl(dfd, path, mode, 0);
#endif /* posix_mkdirat_USE_posix_fmkdirat */

#ifdef posix_mkdirat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)path;
	(void)mode;
	posix_err_unsupported("mkdirat");
	return NULL;
#endif /* posix_mkdirat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_MKDIR_C_INL */
