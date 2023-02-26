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
#ifndef GUARD_DEX_POSIX_P_SYNC_C_INL
#define GUARD_DEX_POSIX_P_SYNC_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN

/* Figure out how to implement `fsync()' */
#undef posix_fsync_USE_fsync
#undef posix_fsync_USE_STUB
#ifdef CONFIG_HAVE_fsync
#define posix_fsync_USE_fsync
#else /* ... */
#define posix_fsync_USE_STUB
#endif /* !... */

/* Figure out how to implement `fdatasync()' */
#undef posix_fdatasync_USE_fdatasync
#undef posix_fdatasync_USE_fsync
#undef posix_fdatasync_USE_STUB
#ifdef CONFIG_HAVE_fdatasync
#define posix_fdatasync_USE_fdatasync
#elif !defined(posix_fsync_USE_STUB)
#define posix_fdatasync_USE_fsync
#else /* ... */
#define posix_fdatasync_USE_STUB
#endif /* !... */





/************************************************************************/
/* sync()                                                               */
/************************************************************************/

/*[[[deemon import("rt.dexutils").gw("sync", "", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_sync_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sync_f(size_t argc, DeeObject *const *argv);
#define POSIX_SYNC_DEF { "sync", (DeeObject *)&posix_sync, MODSYM_FNORMAL, DOC("()") },
#define POSIX_SYNC_DEF_DOC(doc) { "sync", (DeeObject *)&posix_sync, MODSYM_FNORMAL, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_sync, posix_sync_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sync_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sync"))
		goto err;
	return posix_sync_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_sync_f_impl(void)
//[[[end]]]
{
#ifdef CONFIG_HAVE_sync
	sync();
	return_none;
#else  /* CONFIG_HAVE_sync */
	/* deemon.File.System.sync(); */
	return DeeObject_CallAttrString((DeeObject *)&DeeSystemFile_Type,
	                                "sync", 0, NULL);
#endif /* !CONFIG_HAVE_sync */
}





/************************************************************************/
/* fsync()                                                              */
/************************************************************************/

/*[[[deemon import("rt.dexutils").gw("fsync", "fd:unix:fd", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsync_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsync_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FSYNC_DEF { "fsync", (DeeObject *)&posix_fsync, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile)") },
#define POSIX_FSYNC_DEF_DOC(doc) { "fsync", (DeeObject *)&posix_fsync, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fsync, posix_fsync_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsync_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "o:fsync", &fd))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_fsync_f_impl(fd_fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsync_f_impl(int fd)
//[[[end]]]
{
#ifdef posix_fsync_USE_fsync
	int result;
	EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = fsync(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err)
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		HANDLE_ENOSYS(error, err, "fsync")
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to sync %d", fd);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_fsync_USE_fsync */

#ifdef posix_fsync_USE_STUB
#define NEED_posix_err_unsupported
	(void)fd;
	posix_err_unsupported("fsync");
	return NULL;
#endif /* !posix_fsync_USE_STUB */
}





/************************************************************************/
/* fdatasync()                                                          */
/************************************************************************/

/*[[[deemon import("rt.dexutils").gw("fdatasync", "fd:unix:fd", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fdatasync_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fdatasync_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FDATASYNC_DEF { "fdatasync", (DeeObject *)&posix_fdatasync, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile)") },
#define POSIX_FDATASYNC_DEF_DOC(doc) { "fdatasync", (DeeObject *)&posix_fdatasync, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fdatasync, posix_fdatasync_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fdatasync_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "o:fdatasync", &fd))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_fdatasync_f_impl(fd_fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fdatasync_f_impl(int fd)
//[[[end]]]
{
#ifdef posix_fdatasync_USE_fdatasync
	int result;
	EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = fdatasync(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err)
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		HANDLE_ENOSYS(error, err, "fdatasync")
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to sync %d", fd);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_fdatasync_USE_fdatasync */

#ifdef posix_fdatasync_USE_fsync
	return posix_fsync_f_impl(fd);
#endif /* !posix_fdatasync_USE_fsync */

#ifdef posix_fdatasync_USE_STUB
#define NEED_posix_err_unsupported
	(void)fd;
	posix_err_unsupported("fdatasync");
	return NULL;
#endif /* !CONFIG_HAVE_fdatasync */
}



DECL_END

#endif /* !GUARD_DEX_POSIX_P_SYNC_C_INL */
