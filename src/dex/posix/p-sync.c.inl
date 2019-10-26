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
#ifndef GUARD_DEX_POSIX_P_SYNC_C_INL
#define GUARD_DEX_POSIX_P_SYNC_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN





/************************************************************************/
/* sync()                                                               */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("sync", "", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_sync_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sync_f(size_t argc, DeeObject **argv);
#define POSIX_SYNC_DEF { "sync", (DeeObject *)&posix_sync, MODSYM_FNORMAL, DOC("()") },
#define POSIX_SYNC_DEF_DOC(doc) { "sync", (DeeObject *)&posix_sync, MODSYM_FNORMAL, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_sync, posix_sync_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sync_f(size_t argc, DeeObject **argv) {
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
#else /* CONFIG_HAVE_sync */
	/* deemon.File.System.sync(); */
	return DeeObject_CallAttrString((DeeObject *)&DeeSystemFile_Type,
	                                "sync", 0, NULL);
#endif /* !CONFIG_HAVE_sync */
}





/************************************************************************/
/* fsync()                                                              */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("fsync", "fd:d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsync_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsync_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_FSYNC_DEF { "fsync", (DeeObject *)&posix_fsync, MODSYM_FNORMAL, DOC("(fd:?Dint)") },
#define POSIX_FSYNC_DEF_DOC(doc) { "fsync", (DeeObject *)&posix_fsync, MODSYM_FNORMAL, DOC("(fd:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fsync, posix_fsync_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsync_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "d:fsync", &fd))
	    goto err;
	return posix_fsync_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsync_f_impl(int fd)
//[[[end]]]
{
	int result;
#ifdef CONFIG_HAVE_fsync
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = fsync(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		HANDLE_ENOSYS(error, err, "fsync")
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to sync %d", fd);
		goto err;
	}
	return_none;
err:
#else /* CONFIG_HAVE_fsync */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	posix_err_unsupported("fsync");
#endif /* !CONFIG_HAVE_fsync */
	return NULL;
}





/************************************************************************/
/* fdatasync()                                                          */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("fdatasync", "fd:d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fdatasync_f_impl(int fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fdatasync_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_FDATASYNC_DEF { "fdatasync", (DeeObject *)&posix_fdatasync, MODSYM_FNORMAL, DOC("(fd:?Dint)") },
#define POSIX_FDATASYNC_DEF_DOC(doc) { "fdatasync", (DeeObject *)&posix_fdatasync, MODSYM_FNORMAL, DOC("(fd:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fdatasync, posix_fdatasync_f);
#ifndef POSIX_KWDS_FD_DEFINED
#define POSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd, { K(fd), KEND });
#endif /* !POSIX_KWDS_FD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fdatasync_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd, "d:fdatasync", &fd))
	    goto err;
	return posix_fdatasync_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fdatasync_f_impl(int fd)
//[[[end]]]
{
#ifdef CONFIG_HAVE_fdatasync
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = fdatasync(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		HANDLE_ENOSYS(error, err, "fdatasync")
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to sync %d", fd);
		goto err;
	}
	return_none;
err:
#else /* CONFIG_HAVE_fdatasync */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	posix_err_unsupported("fdatasync");
#endif /* !CONFIG_HAVE_fdatasync */
	return NULL;
}



DECL_END

#endif /* !GUARD_DEX_POSIX_P_SYNC_C_INL */
