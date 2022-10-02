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
#ifndef GUARD_DEX_POSIX_P_PWD_C_INL
#define GUARD_DEX_POSIX_P_PWD_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN



#undef posix_gettmp_USE_nt_GetTempPath
#undef posix_gettmp_USE_P_tmpdir
#ifdef CONFIG_HOST_WINDOWS
#define posix_gettmp_USE_nt_GetTempPath
#else /* CONFIG_HOST_WINDOWS */
#ifndef P_tmpdir
#define P_tmpdir "/tmp"
#endif /* !P_tmpdir */
#define posix_gettmp_USE_P_tmpdir
#endif /* !CONFIG_HOST_WINDOWS */



#undef posix_gethostname_USE_nt_GetComputerName
#undef posix_gethostname_USE_gethostname
#undef posix_gethostname_USE_read_etc_hostname
#undef posix_gethostname_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_gethostname_USE_nt_GetComputerName
#elif defined(CONFIG_HAVE_gethostname)
#define posix_gethostname_USE_gethostname
#elif !defined(DEESYSTEM_FILE_USE_STUB)
#define posix_gethostname_USE_read_etc_hostname
#else /* ... */
#define posix_gethostname_USE_STUB
#endif /* !... */



#undef posix_chdir_USE_nt_SetCurrentDirectory
#undef posix_chdir_USE_chdir
#undef posix_chdir_USE_wchdir
/* TODO: posix_chdir_use_setenv_abspath */
#undef posix_chdir_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_chdir_USE_nt_SetCurrentDirectory
#elif defined(CONFIG_HAVE_wchdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_chdir_USE_wchdir
#elif defined(CONFIG_HAVE_chdir)
#define posix_chdir_USE_chdir
#elif defined(CONFIG_HAVE_wchdir)
#define posix_chdir_USE_wchdir
#else /* ... */
#define posix_chdir_USE_STUB
#endif /* !... */



#undef posix_fchdir_USE_fchdir
#undef posix_fchdir_USE_posix_chdir
#undef posix_fchdir_USE_STUB
#ifdef posix_chdir_USE_nt_SetCurrentDirectory
#define posix_fchdir_USE_posix_chdir
#elif defined(CONFIG_HAVE_fchdir)
#define posix_fchdir_USE_fchdir
#elif !defined(posix_chdir_USE_STUB)
#define posix_fchdir_USE_posix_chdir
#else /* ... */
#define posix_fchdir_USE_STUB
#endif /* !... */



#undef posix_fchdirat_USE_fchdirat
#undef posix_fchdirat_USE_posix_chdir
#undef posix_fchdirat_USE_STUB
#ifdef posix_chdir_USE_nt_SetCurrentDirectory
#define posix_fchdirat_USE_posix_chdir
#elif defined(CONFIG_HAVE_fchdirat)
#define posix_fchdirat_USE_fchdirat
#define posix_fchdirat_USE_posix_chdir
#elif !defined(posix_chdir_USE_STUB)
#define posix_fchdirat_USE_posix_chdir
#else /* ... */
#define posix_fchdirat_USE_STUB
#endif /* !... */





#ifndef posix_getenv_USE_STUB
PRIVATE DEFINE_STRING(posix_tmpdir_0, "TMPDIR");
PRIVATE DEFINE_STRING(posix_tmpdir_1, "TMP");
PRIVATE DEFINE_STRING(posix_tmpdir_2, "TEMP");
PRIVATE DEFINE_STRING(posix_tmpdir_3, "TEMPDIR");
PRIVATE DeeStringObject *posix_tmpdir_varnames[] = {
	(DeeStringObject *)&posix_tmpdir_0,
	(DeeStringObject *)&posix_tmpdir_1,
	(DeeStringObject *)&posix_tmpdir_2,
	(DeeStringObject *)&posix_tmpdir_3
};
#endif /* !posix_getenv_USE_STUB */

#ifdef posix_gettmp_USE_P_tmpdir
PRIVATE DEFINE_STRING(posix_tmpdir_default, P_tmpdir);
#endif /* posix_gettmp_USE_P_tmpdir */

#ifdef posix_gethostname_USE_gethostname
#ifndef CONFIG_HAVE_strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */
#endif /* posix_gethostname_USE_gethostname */




/*[[[deemon import("_dexutils").gw("getcwd", "->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getcwd_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getcwd_f(size_t argc, DeeObject *const *argv);
#define POSIX_GETCWD_DEF { "getcwd", (DeeObject *)&posix_getcwd, MODSYM_FNORMAL, DOC("->?Dstring") },
#define POSIX_GETCWD_DEF_DOC(doc) { "getcwd", (DeeObject *)&posix_getcwd, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_getcwd, posix_getcwd_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getcwd_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":getcwd"))
		goto err;
	return posix_getcwd_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getcwd_f_impl(void)
/*[[[end]]]*/
{
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (DeeSystem_PrintPwd(&printer, false))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}



/*[[[deemon import("_dexutils").gw("gettmp", "->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gettmp_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gettmp_f(size_t argc, DeeObject *const *argv);
#define POSIX_GETTMP_DEF { "gettmp", (DeeObject *)&posix_gettmp, MODSYM_FNORMAL, DOC("->?Dstring") },
#define POSIX_GETTMP_DEF_DOC(doc) { "gettmp", (DeeObject *)&posix_gettmp, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_gettmp, posix_gettmp_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gettmp_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":gettmp"))
		goto err;
	return posix_gettmp_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gettmp_f_impl(void)
/*[[[end]]]*/
{
	/* Check for environment variables */
#ifndef posix_getenv_USE_STUB
	{
		size_t i;
		for (i = 0; i < COMPILER_LENOF(posix_tmpdir_varnames); ++i) {
			DREF DeeObject *result;
			result = posix_environ_getenv(posix_tmpdir_varnames[i], Dee_None);
			if (result != Dee_None)
				return result;
			Dee_DecrefNokill(Dee_None);
		}
	}
#endif /* !posix_getenv_USE_STUB */

#ifdef posix_gettmp_USE_nt_GetTempPath
#define NEED_nt_GetTempPath
	return nt_GetTempPath();
#endif /* posix_gettmp_USE_nt_GetTempPath */

#ifdef posix_gettmp_USE_P_tmpdir
	return_reference_((DeeObject *)&posix_tmpdir_default);
#endif /* posix_gettmp_USE_P_tmpdir */
}



/*[[[deemon import("_dexutils").gw("gethostname", "->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gethostname_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gethostname_f(size_t argc, DeeObject *const *argv);
#define POSIX_GETHOSTNAME_DEF { "gethostname", (DeeObject *)&posix_gethostname, MODSYM_FNORMAL, DOC("->?Dstring") },
#define POSIX_GETHOSTNAME_DEF_DOC(doc) { "gethostname", (DeeObject *)&posix_gethostname, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_gethostname, posix_gethostname_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gethostname_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":gethostname"))
		goto err;
	return posix_gethostname_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gethostname_f_impl(void)
/*[[[end]]]*/
{
#ifdef posix_gethostname_USE_nt_GetComputerName
#define NEED_nt_GetComputerName
	return nt_GetComputerName();
#endif /* posix_gethostname_USE_nt_GetComputerName */

#ifdef posix_gethostname_USE_gethostname
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif /* !HOST_NAME_MAX */
#if (HOST_NAME_MAX + 0) <= 0
#undef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#elif (HOST_NAME_MAX + 0) <= 16
#undef HOST_NAME_MAX
#define HOST_NAME_MAX 16
#endif /* !HOST_NAME_MAX */
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	size_t buflen = HOST_NAME_MAX;
	char *newbuf, *buf;
	int error;
	buf = unicode_printer_alloc_utf8(&printer, buflen);
	if unlikely(!buf)
		goto err_printer;
	DBG_ALIGNMENT_DISABLE();
	while (gethostname(buf, buflen) < 0) {
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#if defined(EINVAL) && defined(ENAMETOOLONG)
		if (error != EINVAL && error != ENAMETOOLONG)
			goto err;
#elif defined(EINVAL)
		if (error != EINVAL)
			goto err;
#endif /* ... */
#if defined(EINVAL) && defined(ENAMETOOLONG)
		if (error == EINVAL)
#endif /* EINVAL && ENAMETOOLONG */
		{
			if (buflen >= 0x10000)
				goto err_generic;
		}
		buflen *= 2;
		newbuf = unicode_printer_resize_utf8(&printer, buf, buflen);
		if unlikely(!newbuf)
			goto err;
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if unlikely(unicode_printer_commit_utf8(&printer, buf, strnlen(buf, buflen)) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_generic:
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
	                          "Failed to determine host name");
err:
	unicode_printer_free_utf8(&printer, buf);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* posix_gethostname_USE_gethostname */

#ifdef posix_gethostname_USE_read_etc_hostname
	DREF DeeObject *file, *bytes, *result;
	file = DeeFile_OpenString("/etc/hostname", OPEN_FRDONLY, 0);
	if unlikely(!file)
		goto err;
	bytes = DeeFile_ReadBytes(file, (size_t)-1, true);
	Dee_Decref(file);
	if unlikely(!bytes)
		goto err;
	result = DeeString_NewUtf8((char const *)DeeBytes_DATA(bytes),
	                           DeeBytes_SIZE(bytes),
	                           STRING_ERROR_FREPLAC);
	Dee_Decref(bytes);
	return result;
err:
	return NULL;
#endif /* posix_gethostname_USE_read_etc_hostname */

#ifdef posix_gethostname_USE_STUB
#define NEED_posix_err_unsupported
	posix_err_unsupported("gethostname");
	return NULL;
#endif /* posix_gethostname_USE_STUB */
}



/*[[[deemon import("_dexutils").gw("chdir", "path:?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_chdir_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_chdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_CHDIR_DEF { "chdir", (DeeObject *)&posix_chdir, MODSYM_FNORMAL, DOC("(path:?Dstring)") },
#define POSIX_CHDIR_DEF_DOC(doc) { "chdir", (DeeObject *)&posix_chdir, MODSYM_FNORMAL, DOC("(path:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_chdir, posix_chdir_f);
#ifndef POSIX_KWDS_PATH_DEFINED
#define POSIX_KWDS_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path, { K(path), KEND });
#endif /* !POSIX_KWDS_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_chdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path, "o:chdir", &path))
		goto err;
	return posix_chdir_f_impl(path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_chdir_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_chdir_USE_nt_SetCurrentDirectory
	int result;
again:
#define NEED_nt_SetCurrentDirectory
	result = nt_SetCurrentDirectory(path);
	if unlikely(result != 0) {
		DWORD dwError;
		if unlikely(result < 0)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (DeeNTSystem_IsNotDir(dwError))
			goto do_throw_not_dir;
		if (dwError == ERROR_ACCESS_DENIED) {
			DWORD dwAttributes;
			/* Check if the path is actually a directory. */
#define NEED_nt_GetFileAttributes
			result = nt_GetFileAttributes(path, &dwAttributes);
			if (result < 0)
				goto err;
			if (result == 0 && !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
do_throw_not_dir:
#define NEED_err_nt_path_not_dir
				err_nt_path_not_dir(dwError, path);
				goto err;
			}
		}
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
#define NEED_err_nt_path_not_found
			err_nt_path_not_found(dwError, path);
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define NEED_err_nt_path_no_access
			err_nt_path_no_access(dwError, path);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
			                        "Failed to change the current working directory to %r",
			                        path);
		}
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_chdir_USE_nt_SetCurrentDirectory */

#if defined(posix_chdir_USE_chdir) || defined(posix_chdir_USE_wchdir)
#ifdef posix_chdir_USE_chdir
	char *utf8_path;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#elif defined(posix_chdir_USE_wchdir)
	dwchar_t *wide_path;
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* ... */
EINTR_LABEL(again)
#ifdef posix_chdir_USE_chdir
	if unlikely(chdir(utf8_path) != 0)
#elif defined(posix_chdir_USE_wchdir)
	if (wchdir(wide_path) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err);
#define NEED_err_unix_chdir
		err_unix_chdir(error, path);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_chdir_USE_chdir || posix_chdir_USE_wchdir */

#ifdef posix_chdir_USE_STUB
#define NEED_posix_err_unsupported
	(void)path;
	posix_err_unsupported("chdir");
	return NULL;
#endif /* posix_chdir_USE_STUB */
}



/*[[[deemon import("_dexutils").gw("fchdir", "fd:?X2?DFile?Dint", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchdir_f_impl(DeeObject *fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHDIR_DEF { "fchdir", (DeeObject *)&posix_fchdir, MODSYM_FNORMAL, DOC("(fd:?X2?DFile?Dint)") },
#define POSIX_FCHDIR_DEF_DOC(doc) { "fchdir", (DeeObject *)&posix_fchdir, MODSYM_FNORMAL, DOC("(fd:?X2?DFile?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchdir, posix_fchdir_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "o:fchdir", &fd))
		goto err;
	return posix_fchdir_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchdir_f_impl(DeeObject *fd)
/*[[[end]]]*/
{
#ifdef posix_fchdir_USE_fchdir
	int os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		goto err;
EINTR_LABEL(again)
	if unlikely(fchdir(os_fd) != 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err);
#ifdef EBADF
		if (error == EBADF) {
#define NEED_err_unix_handle_closed
			err_unix_handle_closed(error, fd);
			goto err;
		}
#endif /* EBADF */
#define NEED_err_unix_chdir
		err_unix_chdir(error, fd);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_fchdir_USE_fchdir */

#ifdef posix_fchdir_USE_posix_chdir
	DREF DeeObject *abspath, *result;
#define NEED_posix_fd_abspath
	abspath = posix_fd_abspath(fd);
	if unlikely(!abspath)
		goto err;
	result = posix_chdir_f_impl(abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchdir_USE_posix_chdir */

#ifdef posix_fchdir_USE_STUB
#define NEED_posix_err_unsupported
	(void)fd;
	posix_err_unsupported("fchdir");
	return NULL;
#endif /* posix_fchdir_USE_STUB */
}



/*[[[deemon import("_dexutils").gw("fchdirat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHDIRAT_DEF { "fchdirat", (DeeObject *)&posix_fchdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)") },
#define POSIX_FCHDIRAT_DEF_DOC(doc) { "fchdirat", (DeeObject *)&posix_fchdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchdirat, posix_fchdirat_f);
#ifndef POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_atflags, { K(dfd), K(path), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_atflags, "oo|u:fchdirat", &dfd, &path, &atflags))
		goto err;
	return posix_fchdirat_f_impl(dfd, path, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_fchdirat_USE_posix_chdir
	DREF DeeObject *abspath, *result;
#ifdef posix_fchdirat_USE_fchdirat
	if (!DeeString_Check(dfd) && DeeString_Check(path)) {
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char *utf8_path;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
EINTR_LABEL(again)
		if (fchdirat(os_fd, utf8_path, atflags) == 0)
			return_none;
		HANDLE_EINTR(DeeSystem_GetErrno(), again, err);
		/* fallthru to the fallback path below */
	}
#endif /* posix_fchdirat_USE_fchdirat */

#define NEED_posix_dfd_abspath
	abspath = posix_dfd_abspath(dfd, path, atflags);
	if unlikely(!abspath)
		goto err;
	result = posix_chdir_f_impl(abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchdirat_USE_posix_chdir */

#ifdef posix_fchdirat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)path;
	(void)atflags;
	posix_err_unsupported("fchdirat");
	return NULL;
#endif /* posix_fchdirat_USE_STUB */
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_PWD_C_INL */
