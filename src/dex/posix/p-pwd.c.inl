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
#ifndef GUARD_DEX_POSIX_P_PWD_C_INL
#define GUARD_DEX_POSIX_P_PWD_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/notify.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h> /* DeeSystem_HAVE_FS_DRIVES */
#include <deemon/thread.h>

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN



/* Figure out how to implement `gettmp()' */
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



/* Figure out how to implement `gethostname()' */
#undef posix_gethostname_USE_nt_GetComputerName
#undef posix_gethostname_USE_gethostname
#undef posix_gethostname_USE_read_etc_hostname
#undef posix_gethostname_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_gethostname_USE_nt_GetComputerName
#elif defined(CONFIG_HAVE_gethostname)
#define posix_gethostname_USE_gethostname
#elif !defined(DeeSystem_FILE_USE_STUB) && !defined(DeeSystem_HAVE_FS_DRIVES)
#define posix_gethostname_USE_read_etc_hostname
#else /* ... */
#define posix_gethostname_USE_STUB
#endif /* !... */



/* Figure out how to implement `chdir()' */
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



/* Figure out how to implement `fchdir()' */
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



/* Figure out how to implement `fchdirat()' */
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
/*[[[deemon
import PRIVATE_DEFINE_STRING from rt.gen.string;
PRIVATE_DEFINE_STRING("posix_tmpdir_0", "TMPDIR");
PRIVATE_DEFINE_STRING("posix_tmpdir_1", "TMP");
PRIVATE_DEFINE_STRING("posix_tmpdir_2", "TEMP");
PRIVATE_DEFINE_STRING("posix_tmpdir_3", "TEMPDIR");
]]]*/
PRIVATE DEFINE_STRING_EX(posix_tmpdir_0, "TMPDIR", 0xb180f71d, 0x712d0adb9f9c3cf6);
PRIVATE DEFINE_STRING_EX(posix_tmpdir_1, "TMP", 0x22c9bd9f, 0x4937abfbb3440315);
PRIVATE DEFINE_STRING_EX(posix_tmpdir_2, "TEMP", 0x3e83d711, 0x481e7bd833183107);
PRIVATE DEFINE_STRING_EX(posix_tmpdir_3, "TEMPDIR", 0x728deb7f, 0x4b0984866be1d645);
/*[[[end]]]*/
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
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */
#endif /* posix_gethostname_USE_gethostname */




/*[[[deemon import("rt.gen.dexutils").gw("getcwd", "->?Dstring", libname: "posix");]]]*/
#define POSIX_GETCWD_DEF          DEX_MEMBER_F("getcwd", &posix_getcwd, DEXSYM_READONLY, "->?Dstring"),
#define POSIX_GETCWD_DEF_DOC(doc) DEX_MEMBER_F("getcwd", &posix_getcwd, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getcwd_f_impl(void);
PRIVATE DEFINE_CMETHOD0(posix_getcwd, &posix_getcwd_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getcwd_f_impl(void)
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



/*[[[deemon import("rt.gen.dexutils").gw("gettmp", "->?Dstring", libname: "posix");]]]*/
#define POSIX_GETTMP_DEF          DEX_MEMBER_F("gettmp", &posix_gettmp, DEXSYM_READONLY, "->?Dstring"),
#define POSIX_GETTMP_DEF_DOC(doc) DEX_MEMBER_F("gettmp", &posix_gettmp, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gettmp_f_impl(void);
PRIVATE DEFINE_CMETHOD0(posix_gettmp, &posix_gettmp_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gettmp_f_impl(void)
/*[[[end]]]*/
{
	/* Check for environment variables */
#ifndef posix_getenv_USE_STUB
	{
		size_t i;
		for (i = 0; i < COMPILER_LENOF(posix_tmpdir_varnames); ++i) {
			DREF DeeObject *result;
			result = posix_environ_trygetenv(posix_tmpdir_varnames[i]);
			if (result != ITER_DONE)
				return result;
		}
	}
#endif /* !posix_getenv_USE_STUB */

#ifdef posix_gettmp_USE_nt_GetTempPath
	return nt_GetTempPath();
#define NEED_nt_GetTempPath
#endif /* posix_gettmp_USE_nt_GetTempPath */

#ifdef posix_gettmp_USE_P_tmpdir
	return_reference_((DeeObject *)&posix_tmpdir_default);
#endif /* posix_gettmp_USE_P_tmpdir */
}



/*[[[deemon import("rt.gen.dexutils").gw("gethostname", "->?Dstring", libname: "posix");]]]*/
#define POSIX_GETHOSTNAME_DEF          DEX_MEMBER_F("gethostname", &posix_gethostname, DEXSYM_READONLY, "->?Dstring"),
#define POSIX_GETHOSTNAME_DEF_DOC(doc) DEX_MEMBER_F("gethostname", &posix_gethostname, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gethostname_f_impl(void);
PRIVATE DEFINE_CMETHOD0(posix_gethostname, &posix_gethostname_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gethostname_f_impl(void)
/*[[[end]]]*/
{
#ifdef posix_gethostname_USE_nt_GetComputerName
	return nt_GetComputerName();
#define NEED_nt_GetComputerName
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
	file = DeeFile_OpenString("/etc/hostname", OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
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
	posix_err_unsupported("gethostname");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_gethostname_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("chdir", "path:?Dstring", libname: "posix");]]]*/
#define POSIX_CHDIR_DEF          DEX_MEMBER_F("chdir", &posix_chdir, DEXSYM_READONLY, "(path:?Dstring)"),
#define POSIX_CHDIR_DEF_DOC(doc) DEX_MEMBER_F("chdir", &posix_chdir, DEXSYM_READONLY, "(path:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_chdir_f_impl(DeeObject *path);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_chdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:chdir", &args))
		goto err;
	return posix_chdir_f_impl(args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_chdir, &posix_chdir_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_chdir_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_chdir_USE_nt_SetCurrentDirectory
	int result;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
	result = nt_SetCurrentDirectory(path);
#define NEED_nt_SetCurrentDirectory
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
			result = nt_GetFileAttributes(path, &dwAttributes);
#define NEED_nt_GetFileAttributes
			if (result < 0)
				goto err;
			if (result == 0 && !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
do_throw_not_dir:
				err_nt_path_not_dir(dwError, path);
#define NEED_err_nt_path_not_dir
				goto err;
			}
		}
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
			err_nt_path_not_found(dwError, path);
#define NEED_err_nt_path_not_found
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			err_nt_path_no_access(dwError, path);
#define NEED_err_nt_path_no_access
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
			                        "Failed to change the current working directory to %r",
			                        path);
		}
		goto err;
	}
	if (DeeNotify_BroadcastClass(Dee_NOTIFICATION_CLASS_PWD))
		goto err;
	return_none;
err:
	return NULL;
#endif /* posix_chdir_USE_nt_SetCurrentDirectory */

#if defined(posix_chdir_USE_chdir) || defined(posix_chdir_USE_wchdir)
#ifdef posix_chdir_USE_chdir
	char const *utf8_path;
#endif /* posix_chdir_USE_chdir */
#ifdef posix_chdir_USE_wchdir
	Dee_wchar_t const *wide_path;
#endif /* ..posix_chdir_USE_wchdir */

	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_chdir_USE_chdir
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_chdir_USE_chdir */
#ifdef posix_chdir_USE_wchdir
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_chdir_USE_wchdir */

EINTR_ENOMEM_LABEL(again)
#ifdef posix_chdir_USE_chdir
	if unlikely(chdir((char *)utf8_path) != 0)
#elif defined(posix_chdir_USE_wchdir)
	if unlikely(wchdir((wchar_t *)wide_path) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_chdir(error, path);
#define NEED_err_unix_chdir
		goto err;
	}
	if (DeeNotify_BroadcastClass(Dee_NOTIFICATION_CLASS_PWD))
		goto err;
	return_none;
err:
	return NULL;
#endif /* posix_chdir_USE_chdir || posix_chdir_USE_wchdir */

#ifdef posix_chdir_USE_STUB
	(void)path;
	posix_err_unsupported("chdir");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_chdir_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("fchdir", "fd:?X2?DFile?Dint", libname: "posix");]]]*/
#define POSIX_FCHDIR_DEF          DEX_MEMBER_F("fchdir", &posix_fchdir, DEXSYM_READONLY, "(fd:?X2?DFile?Dint)"),
#define POSIX_FCHDIR_DEF_DOC(doc) DEX_MEMBER_F("fchdir", &posix_fchdir, DEXSYM_READONLY, "(fd:?X2?DFile?Dint)\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_fchdir_f_impl(DeeObject *fd);
#ifndef DEFINED_kwlist__fd
#define DEFINED_kwlist__fd
PRIVATE DEFINE_KWLIST(kwlist__fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !DEFINED_kwlist__fd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *fd;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fd, "o:fchdir", &args))
		goto err;
	return posix_fchdir_f_impl(args.fd);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fchdir, &posix_fchdir_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_fchdir_f_impl(DeeObject *fd)
/*[[[end]]]*/
{
#ifdef posix_fchdir_USE_fchdir
	int os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		goto err;
EINTR_LABEL(again)
	if unlikely(fchdir(os_fd) != 0) {
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
#ifdef EBADF
		if (error == EBADF) {
			err_unix_handle_closed(error, fd);
#define NEED_err_unix_handle_closed
			goto err;
		}
#endif /* EBADF */
		err_unix_chdir(error, fd);
#define NEED_err_unix_chdir
		goto err;
	}
	if (DeeNotify_BroadcastClass(Dee_NOTIFICATION_CLASS_PWD))
		goto err;
	return_none;
err:
	return NULL;
#endif /* posix_fchdir_USE_fchdir */

#ifdef posix_fchdir_USE_posix_chdir
	DREF DeeObject *abspath, *result;
	abspath = posix_fd_makepath(fd);
#define NEED_posix_fd_makepath
	if unlikely(!abspath)
		goto err;
	result = posix_chdir_f_impl(abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchdir_USE_posix_chdir */

#ifdef posix_fchdir_USE_STUB
	(void)fd;
	posix_err_unsupported("fchdir");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_fchdir_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("fchdirat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix");]]]*/
#define POSIX_FCHDIRAT_DEF          DEX_MEMBER_F("fchdirat", &posix_fchdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)"),
#define POSIX_FCHDIRAT_DEF_DOC(doc) DEX_MEMBER_F("fchdirat", &posix_fchdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_fchdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
#ifndef DEFINED_kwlist__dfd_path_atflags
#define DEFINED_kwlist__dfd_path_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_path_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_path_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *path;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_path_atflags, "oo|u:fchdirat", &args))
		goto err;
	return posix_fchdirat_f_impl(args.dfd, args.path, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fchdirat, &posix_fchdirat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_fchdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_fchdirat_USE_posix_chdir
	DREF DeeObject *abspath, *result;
#ifdef posix_fchdirat_USE_fchdirat
	if (!DeeString_Check(dfd) && DeeString_Check(path)) {
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char const *utf8_path;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
EINTR_LABEL(again)
		if (fchdirat(os_dfd, (char *)utf8_path, atflags) == 0) {
			if (DeeNotify_BroadcastClass(Dee_NOTIFICATION_CLASS_PWD))
				goto err;
			return_none;
		}
		EINTR_HANDLE(DeeSystem_GetErrno(), again, err);
		/* fallthru to the fallback path below */
	}
#endif /* posix_fchdirat_USE_fchdirat */

#define NEED_posix_dfd_makepath
	abspath = posix_dfd_makepath(dfd, path, atflags);
	if unlikely(!abspath)
		goto err;
	result = posix_chdir_f_impl(abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchdirat_USE_posix_chdir */

#ifdef posix_fchdirat_USE_STUB
	(void)dfd;
	(void)path;
	(void)atflags;
	posix_err_unsupported("fchdirat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_fchdirat_USE_STUB */
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_PWD_C_INL */
