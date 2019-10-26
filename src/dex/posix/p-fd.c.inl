/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEX_POSIX_P_FD_C_INL
#define GUARD_DEX_POSIX_P_FD_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN

/*[[[deemon
import * from deemon;
import * from _dexutils;
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
for (local x: allDecls) {
	print "\tPOSIX_",;
	print x,;
	print "_DEF \\";
}
print "/" "**" "/";

]]]*/
#include "p-fd-constants.def"
#define POSIX_FD_DEFS \
	POSIX_STDIN_FILENO_DEF \
	POSIX_STDOUT_FILENO_DEF \
	POSIX_STDERR_FILENO_DEF \
/**/
//[[[end]]]


/* Figure out how to implement `umask()' */
#undef posix_umask_USE_UMASK
#undef posix_umask_USE_STUB
#ifdef CONFIG_HAVE_umask
#define posix_umask_USE_UMASK 1
#else /* CONFIG_HAVE_umask2 */
#define posix_umask_USE_STUB 1
#endif /* !CONFIG_HAVE_umask2 */


/* Figure out how to implement `dup()' */
#undef posix_dup_USE_DUP
#undef posix_dup_USE_STUB
#ifdef CONFIG_HAVE_dup
#define posix_dup_USE_DUP 1
#else /* CONFIG_HAVE_dup2 */
#define posix_dup_USE_STUB 1
#endif /* !CONFIG_HAVE_dup2 */


/* Figure out how to implement `dup2()' */
#undef posix_dup2_USE_DUP2
#undef posix_dup2_USE_STUB
#ifdef CONFIG_HAVE_dup2
#define posix_dup2_USE_DUP2 1
#else /* CONFIG_HAVE_dup2 */
#define posix_dup2_USE_STUB 1
#endif /* !CONFIG_HAVE_dup2 */


/* Figure out how to implement `dup3()' */
#undef posix_dup3_USE_DUP3
#undef posix_dup3_USE_DUP2
#undef posix_dup3_USE_DUP2_FCNTL
#undef posix_dup3_USE_DUP2_SETHANDLEINFORMATION
#undef posix_dup3_USE_STUB
#if defined(CONFIG_HAVE_dup3)
#define posix_dup3_USE_DUP3 1
#elif !defined(CONFIG_HAVE_O_CLOEXEC) && !defined(O_NONBLOCK) && !defined(posix_dup2_USE_STUB)
#define posix_dup3_USE_DUP2 1
#elif defined(CONFIG_HAVE_dup2) && defined(CONFIG_HAVE_fcntl) && \
    (!defined(CONFIG_HAVE_O_CLOEXEC) || (defined(CONFIG_HAVE_F_SETFD) && defined(CONFIG_HAVE_FD_CLOEXEC))) && \
    (!defined(CONFIG_HAVE_O_NONBLOCK) || defined(CONFIG_HAVE_F_SETFL))
#define posix_dup3_USE_DUP2_FCNTL 1
#elif defined(CONFIG_HAVE_dup2) && defined(CONFIG_HAVE__get_osfhandle) && \
      defined(CONFIG_HOST_WINDOWS) && defined(CONFIG_HAVE_O_CLOEXEC)
#define posix_dup3_USE_DUP2_SETHANDLEINFORMATION 1
#else
#define posix_dup3_USE_STUB 1
#endif



/* Figure out how to implement `close()' */
#undef posix_close_USE_CLOSE
#undef posix_close_USE_STUB
#ifdef CONFIG_HAVE_close
#define posix_close_USE_CLOSE 1
#else /* CONFIG_HAVE_close */
#define posix_close_USE_STUB 1
#endif /* !CONFIG_HAVE_close */





/************************************************************************/
/* isatty()                                                             */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("isatty", "fd:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_isatty_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isatty_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_ISATTY_DEF { "isatty", (DeeObject *)&posix_isatty, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dbool") },
#define POSIX_ISATTY_DEF_DOC(doc) { "isatty", (DeeObject *)&posix_isatty, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_isatty, posix_isatty_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isatty_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "d:isatty", &fd))
	    goto err;
	return posix_isatty_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_isatty_f_impl(int fd)
//[[[end]]]
{
#ifdef CONFIG_HAVE_isatty
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = isatty(fd);
	DBG_ALIGNMENT_ENABLE();
	if (!result) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "isatty")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		if (error != EINVAL
#ifdef ENOTTY
		    && error != ENOTTY
#endif /* ENOTTY */
		    ) {
			DeeError_SysThrowf(&DeeError_SystemError, error,
			                   "Failed to check isatty for %d", fd);
		}
		goto err;
	}
	return_bool_(result != 0);
err:
#else /* CONFIG_HAVE_isatty */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	posix_err_unsupported("isatty");
#endif /* !CONFIG_HAVE_isatty */
	return NULL;
}




/************************************************************************/
/* umask()                                                              */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("umask", "mask:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_umask_f_impl(int mask);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_umask_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_UMASK_DEF { "umask", (DeeObject *)&posix_umask, MODSYM_FNORMAL, DOC("(mask:?Dint)->?Dint") },
#define POSIX_UMASK_DEF_DOC(doc) { "umask", (DeeObject *)&posix_umask, MODSYM_FNORMAL, DOC("(mask:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_umask, posix_umask_f);
#ifndef POSIX_KWDS_MASK_DEFINED
#define POSIX_KWDS_MASK_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_mask, { K(mask), KEND });
#endif /* !POSIX_KWDS_MASK_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_umask_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int mask;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_mask, "d:umask", &mask))
	    goto err;
	return posix_umask_f_impl(mask);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_umask_f_impl(int mask)
//[[[end]]]
{
#ifdef posix_umask_USE_UMASK
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
#endif /* posix_umask_USE_UMASK */

#ifdef posix_umask_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)mask;
	posix_err_unsupported("umask");
	return NULL;
#endif /* posix_umask_USE_STUB */
}




/************************************************************************/
/* dup()                                                                */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("dup", "fd:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_DUP_DEF { "dup", (DeeObject *)&posix_dup, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dint") },
#define POSIX_DUP_DEF_DOC(doc) { "dup", (DeeObject *)&posix_dup, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_dup, posix_dup_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "d:dup", &fd))
	    goto err;
	return posix_dup_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup_f_impl(int fd)
//[[[end]]]
{
#ifdef posix_dup_USE_DUP
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = dup(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "dup")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeError_SysThrowf(&DeeError_SystemError, error,
		                   "Failed to dup %d", fd);
		goto err;
	}
	return DeeInt_NewInt(result);
err:
	return NULL;
#endif /* posix_dup_USE_DUP */

#ifdef posix_dup_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	posix_err_unsupported("dup");
	return NULL;
#endif /* !posix_dup_USE_STUB */
}




/************************************************************************/
/* dup2()                                                               */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("dup2", "oldfd:d,newfd:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup2_f_impl(int oldfd, int newfd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup2_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_DUP2_DEF { "dup2", (DeeObject *)&posix_dup2, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint)->?Dint") },
#define POSIX_DUP2_DEF_DOC(doc) { "dup2", (DeeObject *)&posix_dup2, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_dup2, posix_dup2_f);
#ifndef POSIX_KWDS_OLDFD_NEWFD_DEFINED
#define POSIX_KWDS_OLDFD_NEWFD_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_oldfd_newfd, { K(oldfd), K(newfd), KEND });
#endif /* !POSIX_KWDS_OLDFD_NEWFD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup2_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int oldfd;
	int newfd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldfd_newfd, "dd:dup2", &oldfd, &newfd))
	    goto err;
	return posix_dup2_f_impl(oldfd, newfd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup2_f_impl(int oldfd, int newfd)
//[[[end]]]
{
#ifdef posix_dup2_USE_DUP2
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = dup2(oldfd, newfd);
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "dup2")
		HANDLE_EBADF(error, err, "Invalid handle %d", oldfd)
		DeeError_SysThrowf(&DeeError_SystemError, error,
		                   "Failed to dup %d", oldfd);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewInt(result);
err:
	return NULL;
#endif /* posix_dup2_USE_DUP2 */

#ifdef posix_dup2_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)oldfd;
	(void)newfd;
	posix_err_unsupported("dup2");
	return NULL;
#endif /* !posix_dup2_USE_STUB */
}




/************************************************************************/
/* dup3()                                                               */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("dup3", "oldfd:d,newfd:d,oflags:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup3_f_impl(int oldfd, int newfd, int oflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup3_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_DUP3_DEF { "dup3", (DeeObject *)&posix_dup3, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint,oflags:?Dint)->?Dint") },
#define POSIX_DUP3_DEF_DOC(doc) { "dup3", (DeeObject *)&posix_dup3, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint,oflags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_dup3, posix_dup3_f);
#ifndef POSIX_KWDS_OLDFD_NEWFD_OFLAGS_DEFINED
#define POSIX_KWDS_OLDFD_NEWFD_OFLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_oldfd_newfd_oflags, { K(oldfd), K(newfd), K(oflags), KEND });
#endif /* !POSIX_KWDS_OLDFD_NEWFD_OFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup3_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int oldfd;
	int newfd;
	int oflags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldfd_newfd_oflags, "ddd:dup3", &oldfd, &newfd, &oflags))
	    goto err;
	return posix_dup3_f_impl(oldfd, newfd, oflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup3_f_impl(int oldfd, int newfd, int oflags)
//[[[end]]]
{

#if defined(posix_dup3_USE_DUP3) || \
    defined(posix_dup3_USE_DUP2_FCNTL) || \
    defined(posix_dup3_USE_DUP2_SETHANDLEINFORMATION)
	DREF DeeObject *result;
	int error;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();

#ifdef posix_dup3_USE_DUP3
	error = dup3(oldfd, newfd, oflags);
	if unlikely(error < 0)
		goto handle_system_error;
#endif /* posix_dup3_USE_DUP3 */

#if defined(posix_dup3_USE_DUP2_FCNTL) || \
    defined(posix_dup3_USE_DUP2_SETHANDLEINFORMATION)
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

#ifdef posix_dup3_USE_DUP2_FCNTL
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
#endif /* posix_dup3_USE_DUP2_FCNTL */

#ifdef posix_dup3_USE_DUP2_SETHANDLEINFORMATION
#ifdef CONFIG_HAVE_O_CLOEXEC
	{
		HANDLE hNewFd;
again_setinfo:
		hNewFd = (HANDLE)(uintptr_t)_get_osfhandle(newfd);
		if (hNewFd == INVALID_HANDLE_VALUE) {
#ifdef CONFIG_HAVE_close
			close(newfd);
#endif /* CONFIG_HAVE_close */
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
#ifdef CONFIG_HAVE_close
			DBG_ALIGNMENT_DISABLE();
			close(newfd);
			DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_close */
			DeeNTSystem_ThrowErrorf(NULL, dwError,
			                        "Failed to set handle information for handle %p",
			                        hNewFd);
			goto err;
		}
	}
#endif /* CONFIG_HAVE_O_CLOEXEC */
#endif /* posix_dup3_USE_DUP2_SETHANDLEINFORMATION */
#endif /* posix_dup3_USE_DUP2_FCNTL || posix_dup3_USE_DUP2_SETHANDLEINFORMATION */

	DBG_ALIGNMENT_ENABLE();
	result = DeeInt_NewInt(newfd);
#ifdef CONFIG_HAVE_close
	if unlikely(!result) {
		DBG_ALIGNMENT_DISABLE();
		close(newfd);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_close */
	return result;

#ifdef posix_dup3_USE_DUP2_FCNTL
handle_system_error_nfd:
#ifdef CONFIG_HAVE_close
	error = DeeSystem_GetErrno();
	close(newfd);
	DeeSystem_SetErrno(error);
#endif /* CONFIG_HAVE_close */
#endif /* posix_dup3_USE_DUP2_FCNTL */
handle_system_error:
	error = DeeSystem_GetErrno();
	DBG_ALIGNMENT_ENABLE();
	HANDLE_EINTR(error, again, err)
	HANDLE_ENOSYS(error, err, "dup3")
	HANDLE_EBADF(error, err, "Invalid fd %d", oldfd)
	HANDLE_EINVAL(error, err, "Invalid oflags for dup3 %#x", oflags)
	DeeError_SysThrowf(&DeeError_SystemError, error,
	                   "Failed to duplicate file descriptor %d into %d",
	                   oldfd, newfd);
err:
	return NULL;
#endif /* posix_dup3_USE_DUP3 || posix_dup3_USE_DUP2_FCNTL */

#ifdef posix_dup3_USE_DUP2
	if unlikely(oflags != 0) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid oflags for dup3 %#x",
		                oflags);
		return NULL;
	}
	return posix_dup2_f_impl(oldfd, newfd);
#endif /* posix_dup3_USE_DUP2 */

#ifdef posix_dup3_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
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

/*[[[deemon import("_dexutils").gw("close", "fd:d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_close_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_close_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_CLOSE_DEF { "close", (DeeObject *)&posix_close, MODSYM_FNORMAL, DOC("(fd:?Dint)") },
#define POSIX_CLOSE_DEF_DOC(doc) { "close", (DeeObject *)&posix_close, MODSYM_FNORMAL, DOC("(fd:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_close, posix_close_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_close_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "d:close", &fd))
	    goto err;
	return posix_close_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_close_f_impl(int fd)
//[[[end]]]
{
#ifdef posix_close_USE_CLOSE
	int error;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	error = close(fd);
	DBG_ALIGNMENT_ENABLE();
	if (error < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "close")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to close %d", fd);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_close_USE_CLOSE */

#ifdef posix_close_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	posix_err_unsupported("close");
	return NULL;
#endif /* posix_close_USE_STUB */
}




DECL_END

#endif /* !GUARD_DEX_POSIX_P_FD_C_INL */
