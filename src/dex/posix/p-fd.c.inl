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
#ifndef GUARD_DEX_POSIX_P_FD_C_INL
#define GUARD_DEX_POSIX_P_FD_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
MODULE_NAME = "posix";
local orig_stdout = File.stdout;

include("p-fd-constants.def");

local allDecls = [];

function fgii(name, doc = none) {
	allDecls.append(name);
	gii(name, doc: doc);
}

fgii("STDIN_FILENO");
fgii("STDOUT_FILENO");
fgii("STDERR_FILENO");


File.stdout = orig_stdout;
print "#define POSIX_FD_DEFS \\";
for (local x: allDecls)
	print("\tPOSIX_", x, "_DEF \\");
print "/" "**" "/";

]]]*/
#include "p-fd-constants.def"
#define POSIX_FD_DEFS \
	POSIX_STDIN_FILENO_DEF \
	POSIX_STDOUT_FILENO_DEF \
	POSIX_STDERR_FILENO_DEF \
/**/
/*[[[end]]]*/



/* Figure out how to implement `isatty()' */
#undef posix_isatty_USE_isatty
#undef posix_isatty_USE_STUB
#ifdef CONFIG_HAVE_isatty
#define posix_isatty_USE_isatty
#else /* CONFIG_HAVE_isatty */
#define posix_isatty_USE_STUB
#endif /* !CONFIG_HAVE_isatty */



/* Figure out how to implement `umask()' */
#undef posix_umask_USE_umask
#undef posix_umask_USE_STUB
#ifdef CONFIG_HAVE_umask
#define posix_umask_USE_umask
#else /* CONFIG_HAVE_umask */
#define posix_umask_USE_STUB
#endif /* !CONFIG_HAVE_umask */



/* Figure out how to implement `dup()' */
#undef posix_dup_USE_dup
#undef posix_dup_USE_STUB
#ifdef CONFIG_HAVE_dup
#define posix_dup_USE_dup
#else /* CONFIG_HAVE_dup2 */
#define posix_dup_USE_STUB
#endif /* !CONFIG_HAVE_dup2 */



/* Figure out how to implement `dup2()' */
#undef posix_dup2_USE_dup2
#undef posix_dup2_USE_STUB
#ifdef CONFIG_HAVE_dup2
#define posix_dup2_USE_dup2
#else /* CONFIG_HAVE_dup2 */
#define posix_dup2_USE_STUB
#endif /* !CONFIG_HAVE_dup2 */



/* Figure out how to implement `dup3()' */
#undef posix_dup3_USE_dup3
#undef posix_dup3_USE_dup2
#undef posix_dup3_USE_dup2_AND_fcntl
#undef posix_dup3_USE_dup2_AND_SetHandleInformation
#undef posix_dup3_USE_STUB
#if defined(CONFIG_HAVE_dup3)
#define posix_dup3_USE_dup3
#elif !defined(CONFIG_HAVE_O_CLOEXEC) && !defined(O_NONBLOCK) && !defined(posix_dup2_USE_STUB)
#define posix_dup3_USE_dup2
#elif (defined(CONFIG_HAVE_dup2) && defined(CONFIG_HAVE_fcntl) &&                \
       (!defined(CONFIG_HAVE_O_CLOEXEC) || (defined(CONFIG_HAVE_F_SETFD) &&      \
                                            defined(CONFIG_HAVE_FD_CLOEXEC))) && \
       (!defined(CONFIG_HAVE_O_NONBLOCK) || defined(CONFIG_HAVE_F_SETFL)))
#define posix_dup3_USE_dup2_AND_fcntl
#elif (defined(CONFIG_HAVE_dup2) && defined(CONFIG_HAVE_get_osfhandle) && \
       defined(CONFIG_HOST_WINDOWS) && defined(CONFIG_HAVE_O_CLOEXEC))
#define posix_dup3_USE_dup2_AND_SetHandleInformation
#else /* ... */
#define posix_dup3_USE_STUB
#endif /* !... */



/* Figure out how to implement `close()' */
#undef posix_close_USE_close
#undef posix_close_USE_STUB
#ifdef CONFIG_HAVE_close
#define posix_close_USE_close
#else /* CONFIG_HAVE_close */
#define posix_close_USE_STUB
#endif /* !CONFIG_HAVE_close */





/************************************************************************/
/* isatty()                                                             */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("isatty", "fd:unix:fd->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_isatty_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isatty_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ISATTY_DEF { "isatty", (DeeObject *)&posix_isatty, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile)->?Dbool") },
#define POSIX_ISATTY_DEF_DOC(doc) { "isatty", (DeeObject *)&posix_isatty, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_isatty, &posix_isatty_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isatty_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "o:isatty", &fd))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_isatty_f_impl(fd_fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_isatty_f_impl(int fd)
/*[[[end]]]*/
{
#ifdef posix_isatty_USE_isatty
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	/* BUG BUG BUG: Cygwin doesn't set errno when isatty()
	 *              returns `0' because file isn't a tty */
	DeeSystem_SetErrno(ENOTTY);
#endif /* __CYGWIN__ || __CYGWIN32__ */
	result = isatty(fd);
	if (!result) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeSystem_IF_E2(error, ENOTTY, EINVAL, return_false);
		EINTR_HANDLE(error, again, err);
		HANDLE_ENOSYS(error, err, "isatty");
		HANDLE_EBADF(error, err, "Invalid handle %d", fd);
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to check isatty for %d",
		                          fd);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return_true;
err:
	return NULL;
#endif /* posix_isatty_USE_isatty */

#ifdef posix_isatty_USE_STUB
#define NEED_posix_err_unsupported
	(void)fd;
	posix_err_unsupported("isatty");
	return NULL;
#endif /* !posix_isatty_USE_STUB */
}




/************************************************************************/
/* umask()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("umask", "mask:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_umask_f_impl(int mask);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_umask_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_UMASK_DEF { "umask", (DeeObject *)&posix_umask, MODSYM_FREADONLY, DOC("(mask:?Dint)->?Dint") },
#define POSIX_UMASK_DEF_DOC(doc) { "umask", (DeeObject *)&posix_umask, MODSYM_FREADONLY, DOC("(mask:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_umask, &posix_umask_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_MASK_DEFINED
#define POSIX_KWDS_MASK_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_mask, { KEX("mask", 0xc3b4302b, 0x933f153b40dd4379), KEND });
#endif /* !POSIX_KWDS_MASK_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_umask_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int mask;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_mask, "d:umask", &mask))
		goto err;
	return posix_umask_f_impl(mask);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_umask_f_impl(int mask)
/*[[[end]]]*/
{
#ifdef posix_umask_USE_umask
	DREF DeeObject *result;
	int oldmask;
	DBG_ALIGNMENT_DISABLE();
	oldmask = umask(mask);
	DBG_ALIGNMENT_ENABLE();
	result = DeeInt_NewInt(oldmask);
	if unlikely(!result) {
		DBG_ALIGNMENT_DISABLE();
		umask(oldmask);
		DBG_ALIGNMENT_ENABLE();
	}
	return NULL;
#endif /* posix_umask_USE_umask */

#ifdef posix_umask_USE_STUB
#define NEED_posix_err_unsupported
	(void)mask;
	posix_err_unsupported("umask");
	return NULL;
#endif /* posix_umask_USE_STUB */
}




/************************************************************************/
/* dup()                                                                */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("dup", "fd:unix:fd->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_DUP_DEF { "dup", (DeeObject *)&posix_dup, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile)->?Dint") },
#define POSIX_DUP_DEF_DOC(doc) { "dup", (DeeObject *)&posix_dup, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_dup, &posix_dup_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "o:dup", &fd))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_dup_f_impl(fd_fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup_f_impl(int fd)
/*[[[end]]]*/
{
#ifdef posix_dup_USE_dup
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = dup(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err)
		HANDLE_ENOSYS(error, err, "dup")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to dup %d", fd);
		goto err;
	}
	return DeeInt_NewInt(result);
err:
	return NULL;
#endif /* posix_dup_USE_dup */

#ifdef posix_dup_USE_STUB
#define NEED_posix_err_unsupported
	(void)fd;
	posix_err_unsupported("dup");
	return NULL;
#endif /* !posix_dup_USE_STUB */
}




/************************************************************************/
/* dup2()                                                               */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("dup2", "oldfd:unix:fd,newfd:unix:fd->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup2_f_impl(int oldfd, int newfd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup2_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_DUP2_DEF { "dup2", (DeeObject *)&posix_dup2, MODSYM_FREADONLY, DOC("(oldfd:?X2?Dint?DFile,newfd:?X2?Dint?DFile)->?Dint") },
#define POSIX_DUP2_DEF_DOC(doc) { "dup2", (DeeObject *)&posix_dup2, MODSYM_FREADONLY, DOC("(oldfd:?X2?Dint?DFile,newfd:?X2?Dint?DFile)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_dup2, &posix_dup2_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_OLDFD_NEWFD_DEFINED
#define POSIX_KWDS_OLDFD_NEWFD_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oldfd_newfd, { KEX("oldfd", 0x5a92fcdb, 0x3de145419f68339e), KEX("newfd", 0xd4ea987d, 0xe1d56bf670fa4fe3), KEND });
#endif /* !POSIX_KWDS_OLDFD_NEWFD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup2_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int oldfd_fd;
	DeeObject *oldfd;
	int newfd_fd;
	DeeObject *newfd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldfd_newfd, "oo:dup2", &oldfd, &newfd))
		goto err;
	oldfd_fd = DeeUnixSystem_GetFD(oldfd);
	if unlikely(oldfd_fd == -1)
		goto err;
	newfd_fd = DeeUnixSystem_GetFD(newfd);
	if unlikely(newfd_fd == -1)
		goto err;
	return posix_dup2_f_impl(oldfd_fd, newfd_fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup2_f_impl(int oldfd, int newfd)
/*[[[end]]]*/
{
#ifdef posix_dup2_USE_dup2
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = dup2(oldfd, newfd);
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(error, again, err)
		HANDLE_ENOSYS(error, err, "dup2")
		HANDLE_EBADF(error, err, "Invalid handle %d", oldfd)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to dup %d", oldfd);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewInt(result);
err:
	return NULL;
#endif /* posix_dup2_USE_dup2 */

#ifdef posix_dup2_USE_STUB
#define NEED_posix_err_unsupported
	(void)oldfd;
	(void)newfd;
	posix_err_unsupported("dup2");
	return NULL;
#endif /* !posix_dup2_USE_STUB */
}




/************************************************************************/
/* dup3()                                                               */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("dup3", "oldfd:unix:fd,newfd:unix:fd,oflags:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup3_f_impl(int oldfd, int newfd, int oflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup3_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_DUP3_DEF { "dup3", (DeeObject *)&posix_dup3, MODSYM_FREADONLY, DOC("(oldfd:?X2?Dint?DFile,newfd:?X2?Dint?DFile,oflags:?Dint)->?Dint") },
#define POSIX_DUP3_DEF_DOC(doc) { "dup3", (DeeObject *)&posix_dup3, MODSYM_FREADONLY, DOC("(oldfd:?X2?Dint?DFile,newfd:?X2?Dint?DFile,oflags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_dup3, &posix_dup3_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_OLDFD_NEWFD_OFLAGS_DEFINED
#define POSIX_KWDS_OLDFD_NEWFD_OFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oldfd_newfd_oflags, { KEX("oldfd", 0x5a92fcdb, 0x3de145419f68339e), KEX("newfd", 0xd4ea987d, 0xe1d56bf670fa4fe3), KEX("oflags", 0xbe92b5be, 0x4f84e498f7c9d171), KEND });
#endif /* !POSIX_KWDS_OLDFD_NEWFD_OFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup3_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int oldfd_fd;
	DeeObject *oldfd;
	int newfd_fd;
	DeeObject *newfd;
	int oflags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldfd_newfd_oflags, "ood:dup3", &oldfd, &newfd, &oflags))
		goto err;
	oldfd_fd = DeeUnixSystem_GetFD(oldfd);
	if unlikely(oldfd_fd == -1)
		goto err;
	newfd_fd = DeeUnixSystem_GetFD(newfd);
	if unlikely(newfd_fd == -1)
		goto err;
	return posix_dup3_f_impl(oldfd_fd, newfd_fd, oflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup3_f_impl(int oldfd, int newfd, int oflags)
/*[[[end]]]*/
{

#if (defined(posix_dup3_USE_dup3) ||       \
     defined(posix_dup3_USE_dup2_AND_fcntl) || \
     defined(posix_dup3_USE_dup2_AND_SetHandleInformation))
	DREF DeeObject *result;
	int error;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();

#ifdef posix_dup3_USE_dup3
	error = dup3(oldfd, newfd, oflags);
	if unlikely(error < 0)
		goto handle_system_error;
#endif /* posix_dup3_USE_dup3 */

#if (defined(posix_dup3_USE_dup2_AND_fcntl) || \
     defined(posix_dup3_USE_dup2_AND_SetHandleInformation))
	/* Validate the given `oflags' */
#if defined(CONFIG_HAVE_O_NONBLOCK) && defined(CONFIG_HAVE_O_CLOEXEC)
	if (oflags & ~(O_CLOEXEC | O_NONBLOCK))
#elif defined(CONFIG_HAVE_O_NONBLOCK)
	if (oflags & ~(O_NONBLOCK))
#else
	if (oflags & ~(O_CLOEXEC))
#endif
	{
		DeeSystem_SetErrno(EINVAL);
		goto handle_system_error;
	}
	error = dup2(oldfd, newfd);
	if (error < 0)
		goto handle_system_error;

#ifdef posix_dup3_USE_dup2_AND_fcntl
	/* Apply O_CLOEXEC */
#ifdef CONFIG_HAVE_O_CLOEXEC
	if (oflags & O_CLOEXEC) {
		error = fcntl(newfd, F_SETFD, FD_CLOEXEC);
		if unlikely(error < 0)
			goto handle_system_error_nfd;
	}
#endif /* CONFIG_HAVE_O_CLOEXEC */

	/* Apply O_NONBLOCK */
#ifdef CONFIG_HAVE_O_NONBLOCK
	if (oflags & O_NONBLOCK) {
		error = fcntl(newfd, F_SETFL, O_NONBLOCK);
		if unlikely(error < 0)
			goto handle_system_error_nfd;
	}
#endif /* CONFIG_HAVE_O_NONBLOCK */
#endif /* posix_dup3_USE_dup2_AND_fcntl */

#ifdef posix_dup3_USE_dup2_AND_SetHandleInformation
#ifdef CONFIG_HAVE_O_CLOEXEC
	{
		HANDLE hNewFd;
again_setinfo:
		hNewFd = (HANDLE)(uintptr_t)get_osfhandle(newfd);
		if (hNewFd == INVALID_HANDLE_VALUE) {
			OPT_close(newfd);
			DBG_ALIGNMENT_ENABLE();
			DeeError_Throwf(&DeeError_FileClosed, "Invalid fd %d", newfd);
			goto err;
		}
		if (!SetHandleInformation(hNewFd,
		                          HANDLE_FLAG_INHERIT,
		                          (oflags & O_CLOEXEC)
		                          ? 0
		                          : HANDLE_FLAG_INHERIT)) {
			DWORD dwError;
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err;
				DBG_ALIGNMENT_DISABLE();
				goto again_setinfo;
			}
			DBG_ALIGNMENT_DISABLE();
			OPT_close(newfd);
			DBG_ALIGNMENT_ENABLE();
			DeeNTSystem_ThrowErrorf(NULL, dwError,
			                        "Failed to set handle information for handle %p",
			                        hNewFd);
			goto err;
		}
	}
#endif /* CONFIG_HAVE_O_CLOEXEC */
#endif /* posix_dup3_USE_dup2_AND_SetHandleInformation */
#endif /* posix_dup3_USE_dup2_AND_fcntl || posix_dup3_USE_dup2_AND_SetHandleInformation */

	DBG_ALIGNMENT_ENABLE();
	result = DeeInt_NewInt(newfd);
	if unlikely(!result) {
		DBG_ALIGNMENT_DISABLE();
		OPT_close(newfd);
		DBG_ALIGNMENT_ENABLE();
	}
	return result;

#ifdef posix_dup3_USE_dup2_AND_fcntl
handle_system_error_nfd:
	error = DeeSystem_GetErrno();
	OPT_close(newfd);
	DeeSystem_SetErrno(error);
#endif /* posix_dup3_USE_dup2_AND_fcntl */
handle_system_error:
	error = DeeSystem_GetErrno();
	DBG_ALIGNMENT_ENABLE();
	EINTR_HANDLE(error, again, err)
	HANDLE_ENOSYS(error, err, "dup3")
	HANDLE_EBADF(error, err, "Invalid fd %d", oldfd)
	HANDLE_EINVAL(error, err, "Invalid oflags for dup3 %#x", oflags)
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
	                          "Failed to duplicate file descriptor %d into %d",
	                          oldfd, newfd);
err:
	return NULL;
#endif /* posix_dup3_USE_dup3 || posix_dup3_USE_dup2_AND_fcntl */

#ifdef posix_dup3_USE_dup2
	if unlikely(oflags != 0) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid oflags for dup3 %#x",
		                oflags);
		return NULL;
	}
	return posix_dup2_f_impl(oldfd, newfd);
#endif /* posix_dup3_USE_dup2 */

#ifdef posix_dup3_USE_STUB
#define NEED_posix_err_unsupported
	(void)oldfd;
	(void)newfd;
	(void)oflags;
	posix_err_unsupported("dup3");
	return NULL;
#endif /* posix_dup3_USE_STUB */
}





/************************************************************************/
/* close()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("close", "fd:unix:fd", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_close_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_close_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_CLOSE_DEF { "close", (DeeObject *)&posix_close, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile)") },
#define POSIX_CLOSE_DEF_DOC(doc) { "close", (DeeObject *)&posix_close, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_close, &posix_close_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_close_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "o:close", &fd))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_close_f_impl(fd_fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_close_f_impl(int fd)
/*[[[end]]]*/
{
#ifdef posix_close_USE_close
	int error;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	error = close(fd);
	DBG_ALIGNMENT_ENABLE();
	if (error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err)
		HANDLE_ENOSYS(error, err, "close")
		/* For some reason, msvc somethings returns
		 * ENOENT... (usually when attached to a debugger) */
#if defined(CONFIG_HOST_WINDOWS) && !defined(__CYGWIN__)
		HANDLE_EBADF_ENOENT(error, err, "Bad fd %d", fd)
#else /* CONFIG_HOST_WINDOWS && !__CYGWIN__ */
		HANDLE_EBADF(error, err, "Bad fd %d", fd)
#endif /* !CONFIG_HOST_WINDOWS || __CYGWIN__ */
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to close fd %d", fd);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_close_USE_close */

#ifdef posix_close_USE_STUB
#define NEED_posix_err_unsupported
	(void)fd;
	posix_err_unsupported("close");
	return NULL;
#endif /* posix_close_USE_STUB */
}




DECL_END

#endif /* !GUARD_DEX_POSIX_P_FD_C_INL */
