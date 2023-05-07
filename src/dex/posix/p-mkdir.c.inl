/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_MKDIR_C_INL
#define GUARD_DEX_POSIX_P_MKDIR_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

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
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_mkdir_f_impl(DeeObject *path, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_mkdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_MKDIR_DEF { "mkdir", (DeeObject *)&posix_mkdir, MODSYM_FNORMAL, DOC("(path:?Dstring,mode:?Dint=!0755)") },
#define POSIX_MKDIR_DEF_DOC(doc) { "mkdir", (DeeObject *)&posix_mkdir, MODSYM_FNORMAL, DOC("(path:?Dstring,mode:?Dint=!0755)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_mkdir, &posix_mkdir_f);
#ifndef POSIX_KWDS_PATH_MODE_DEFINED
#define POSIX_KWDS_PATH_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path_mode, { K(path), K(mode), KEND });
#endif /* !POSIX_KWDS_PATH_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_mkdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	unsigned int mode = 0755;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path_mode, "o|u:mkdir", &path, &mode))
		goto err;
	return posix_mkdir_f_impl(path, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_mkdir_f_impl(DeeObject *path, unsigned int mode)
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
	dwchar_t *wide_path;
#endif /* posix_mkdir_USE_wmkdir */
#ifdef posix_mkdir_USE_mkdir
	char *utf8_path;
#endif /* posix_mkdir_USE_mkdir */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_mkdir_USE_wmkdir
	wide_path = (dwchar_t *)DeeString_AsWide(path);
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
	error = mkdir(utf8_path, mode);
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fmkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fmkdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FMKDIRAT_DEF { "fmkdirat", (DeeObject *)&posix_fmkdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:?Dint=!0755,atflags:?Dint=!0)") },
#define POSIX_FMKDIRAT_DEF_DOC(doc) { "fmkdirat", (DeeObject *)&posix_fmkdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:?Dint=!0755,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fmkdirat, &posix_fmkdirat_f);
#ifndef POSIX_KWDS_DFD_PATH_MODE_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_MODE_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_mode_atflags, { K(dfd), K(path), K(mode), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_MODE_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fmkdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	unsigned int mode = 0755;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_mode_atflags, "oo|uu:fmkdirat", &dfd, &path, &mode, &atflags))
		goto err;
	return posix_fmkdirat_f_impl(dfd, path, mode, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fmkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode, unsigned int atflags)
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
		dwchar_t *wide_path;
#endif /* posix_fmkdirat_USE_wfmkdirat || posix_fmkdirat_USE_wmkdirat */
#if defined(posix_fmkdirat_USE_fmkdirat) || defined(posix_fmkdirat_USE_mkdirat)
		char *utf8_path;
#endif /* posix_fmkdirat_USE_fmkdirat || posix_fmkdirat_USE_mkdirat */
		int error;
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
#if defined(posix_fmkdirat_USE_wfmkdirat) || defined(posix_fmkdirat_USE_wmkdirat)
		wide_path = (dwchar_t *)DeeString_AsWide(path);
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
		error = wfmkdirat(os_dfd, wide_path, mode, atflags);
#elif defined(posix_fmkdirat_USE_wmkdirat)
		error = wmkdirat(os_dfd, wide_path, mode);
#elif defined(posix_fmkdirat_USE_fmkdirat)
		error = fmkdirat(os_dfd, utf8_path, mode, atflags);
#elif defined(posix_fmkdirat_USE_mkdirat))
		error = mkdirat(os_dfd, utf8_path, mode);
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
	abspath = posix_dfd_abspath(dfd, path, atflags);
#define NEED_posix_dfd_abspath
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
	(void)path;
	(void)mode;
	posix_err_unsupported("fmkdirat");
	return NULL;
#endif /* posix_fmkdirat_USE_STUB */
}


/************************************************************************/
/* mkdirat()                                                            */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("mkdirat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:u=0755", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_mkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_mkdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_MKDIRAT_DEF { "mkdirat", (DeeObject *)&posix_mkdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:?Dint=!0755)") },
#define POSIX_MKDIRAT_DEF_DOC(doc) { "mkdirat", (DeeObject *)&posix_mkdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:?Dint=!0755)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_mkdirat, &posix_mkdirat_f);
#ifndef POSIX_KWDS_DFD_PATH_MODE_DEFINED
#define POSIX_KWDS_DFD_PATH_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_mode, { K(dfd), K(path), K(mode), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_mkdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	unsigned int mode = 0755;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_mode, "oo|u:mkdirat", &dfd, &path, &mode))
		goto err;
	return posix_mkdirat_f_impl(dfd, path, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_mkdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int mode)
/*[[[end]]]*/
{
#ifdef posix_mkdirat_USE_posix_fmkdirat
	return posix_fmkdirat_f_impl(dfd, path, mode, 0);
#endif /* posix_mkdirat_USE_posix_fmkdirat */

#ifdef posix_mkdirat_USE_STUB
#define NEED_posix_err_unsupported
	(void)path;
	(void)mode;
	posix_err_unsupported("mkdirat");
	return NULL;
#endif /* posix_mkdirat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_MKDIR_C_INL */
