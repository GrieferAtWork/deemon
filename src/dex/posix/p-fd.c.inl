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
#ifdef CONFIG_HAVE_umask
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = umask(mask);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "umask")
		DeeError_SysThrowf(&DeeError_SystemError, error,
		                   "Failed set umask");
		goto err;
	}
	return DeeInt_NewInt(result);
err:
#else /* CONFIG_HAVE_umask */
#define NEED_ERR_UNSUPPORTED 1
	(void)mask;
	posix_err_unsupported("umask");
#endif /* !CONFIG_HAVE_umask */
	return NULL;
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
#ifdef CONFIG_HAVE_dup
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
#else /* CONFIG_HAVE_dup */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	posix_err_unsupported("dup");
#endif /* !CONFIG_HAVE_dup */
	return NULL;
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
	return posix_dup2_f_impl(oldfd,newfd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup2_f_impl(int oldfd, int newfd)
//[[[end]]]
{
#ifdef CONFIG_HAVE_dup2
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = dup2(oldfd, newfd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "dup2")
		HANDLE_EBADF(error, err, "Invalid handle %d", oldfd)
		DeeError_SysThrowf(&DeeError_SystemError, error,
		                   "Failed to dup %d", oldfd);
		goto err;
	}
	return DeeInt_NewInt(result);
err:
#else /* CONFIG_HAVE_dup2 */
#define NEED_ERR_UNSUPPORTED 1
	(void)oldfd;
	(void)newfd;
	posix_err_unsupported("dup2");
#endif /* !CONFIG_HAVE_dup2 */
	return NULL;
}




/************************************************************************/
/* dup3()                                                               */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("dup3", "oldfd:d,newfd:d,flags:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup3_f_impl(int oldfd, int newfd, int flags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup3_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_DUP3_DEF { "dup3", (DeeObject *)&posix_dup3, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint,flags:?Dint)->?Dint") },
#define POSIX_DUP3_DEF_DOC(doc) { "dup3", (DeeObject *)&posix_dup3, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint,flags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_dup3, posix_dup3_f);
#ifndef POSIX_KWDS_OLDFD_NEWFD_FLAGS_DEFINED
#define POSIX_KWDS_OLDFD_NEWFD_FLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_oldfd_newfd_flags, { K(oldfd), K(newfd), K(flags), KEND });
#endif /* !POSIX_KWDS_OLDFD_NEWFD_FLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_dup3_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int oldfd;
	int newfd;
	int flags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldfd_newfd_flags, "ddd:dup3", &oldfd, &newfd, &flags))
	    goto err;
	return posix_dup3_f_impl(oldfd,newfd,flags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_dup3_f_impl(int oldfd, int newfd, int flags)
//[[[end]]]
{
#if defined(CONFIG_HAVE_dup3) || \
    (defined(CONFIG_HAVE_dup2) && defined(CONFIG_HAVE__get_osfhandle) && defined(CONFIG_HOST_WINDOWS))
	int result;
#ifndef CONFIG_HAVE_dup3
	if (flags & ~O_CLOEXEC) {
		DeeSystem_SetErrno(EINVAL);
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid flags for dup3 %#x",
		                flags);
		goto err;
	}
#endif /* !CONFIG_HAVE_dup3 */
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_dup3
	result = dup3(oldfd, newfd, flags);
#else /* CONFIG_HAVE_dup3 */
	result = dup2(oldfd, newfd);
	if (result >= 0) {
		SetHandleInformation((HANDLE)(uintptr_t)_get_osfhandle(result),
		                     HANDLE_FLAG_INHERIT,
		                     (flags & O_CLOEXEC)
		                     ? 0
		                     : HANDLE_FLAG_INHERIT);
	}
#endif /* !CONFIG_HAVE_dup3 */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "dup3")
		HANDLE_EBADF(error, err, "Invalid handle %d", oldfd)
		HANDLE_EINVAL(error, err, "Invalid flags for dup3 %#x", flags)
		DeeError_SysThrowf(&DeeError_SystemError, error,
		                   "Failed to dup %d", oldfd);
		goto err;
	}
	return DeeInt_NewInt(result);
err:
#else /* HAVE_DUP3 || _MSC_VER */
#define NEED_ERR_UNSUPPORTED 1
	(void)oldfd;
	(void)newfd;
	(void)flags;
	posix_err_unsupported("dup3");
#endif /* !HAVE_DUP3 && !_MSC_VER */
	return NULL;
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
#ifdef CONFIG_HAVE_close
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
#else /* CONFIG_HAVE_close */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	posix_err_unsupported("close");
#endif /* !CONFIG_HAVE_close */
	return NULL;
}




DECL_END

#endif /* !GUARD_DEX_POSIX_P_FD_C_INL */
