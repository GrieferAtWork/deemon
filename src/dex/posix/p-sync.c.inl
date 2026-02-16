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
#ifndef GUARD_DEX_POSIX_P_SYNC_C_INL
#define GUARD_DEX_POSIX_P_SYNC_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_UnpackStructKw */
#include <deemon/dex.h>             /* DEXSYM_READONLY, DEX_MEMBER_F */
#include <deemon/error.h>           /* DeeError_FSError */
#include <deemon/file.h>            /* DeeSystemFile_Type */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_CallAttrString, Dee_AsObject */
#include <deemon/objmethod.h>       /*  */
#include <deemon/system-features.h> /* CONFIG_HAVE_*, DeeSystem_GetErrno, fdatasync, fsync, sync */
#include <deemon/system.h>          /* DeeUnixSystem_* */
#include <deemon/type.h>            /* METHOD_FNORMAL */

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */

#include <stddef.h> /* NULL, size_t */

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
#undef posix_fdatasync_USE_posix_fsync
#undef posix_fdatasync_USE_STUB
#ifdef CONFIG_HAVE_fdatasync
#define posix_fdatasync_USE_fdatasync
#elif !defined(posix_fsync_USE_STUB)
#define posix_fdatasync_USE_posix_fsync
#else /* ... */
#define posix_fdatasync_USE_STUB
#endif /* !... */





/************************************************************************/
/* sync()                                                               */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("sync", "", libname: "posix"); ]]]*/
#define POSIX_SYNC_DEF          DEX_MEMBER_F("sync", &posix_sync, DEXSYM_READONLY, "()"),
#define POSIX_SYNC_DEF_DOC(doc) DEX_MEMBER_F("sync", &posix_sync, DEXSYM_READONLY, "()\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sync_f_impl(void);
PRIVATE DEFINE_CMETHOD0(posix_sync, &posix_sync_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sync_f_impl(void)
/*[[[end]]]*/
{
#ifdef CONFIG_HAVE_sync
	sync();
	return_none;
#else  /* CONFIG_HAVE_sync */
	/* deemon.File.System.sync(); */
	return DeeObject_CallAttrString(Dee_AsObject(&DeeSystemFile_Type.ft_base),
	                                "sync", 0, NULL);
#endif /* !CONFIG_HAVE_sync */
}





/************************************************************************/
/* fsync()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("fsync", "fd:unix:fd", libname: "posix"); ]]]*/
#define POSIX_FSYNC_DEF          DEX_MEMBER_F("fsync", &posix_fsync, DEXSYM_READONLY, "(fd:?X2?Dint?DFile)"),
#define POSIX_FSYNC_DEF_DOC(doc) DEX_MEMBER_F("fsync", &posix_fsync, DEXSYM_READONLY, "(fd:?X2?Dint?DFile)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsync_f_impl(int fd);
#ifndef DEFINED_kwlist__fd
#define DEFINED_kwlist__fd
PRIVATE DEFINE_KWLIST(kwlist__fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !DEFINED_kwlist__fd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsync_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_fd;
	} args;
	int fd;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fd, "o:fsync", &args))
		goto err;
	if unlikely((fd = DeeUnixSystem_GetFD(args.raw_fd)) == -1)
		goto err;
	return posix_fsync_f_impl(fd);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fsync, &posix_fsync_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsync_f_impl(int fd)
/*[[[end]]]*/
{
#ifdef posix_fsync_USE_fsync
	int result;
	again:
	DBG_ALIGNMENT_DISABLE();
	result = fsync(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		DeeUnixSystem_HandleGenericError(error, err, again);
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

/*[[[deemon import("rt.gen.dexutils").gw("fdatasync", "fd:unix:fd", libname: "posix"); ]]]*/
#define POSIX_FDATASYNC_DEF          DEX_MEMBER_F("fdatasync", &posix_fdatasync, DEXSYM_READONLY, "(fd:?X2?Dint?DFile)"),
#define POSIX_FDATASYNC_DEF_DOC(doc) DEX_MEMBER_F("fdatasync", &posix_fdatasync, DEXSYM_READONLY, "(fd:?X2?Dint?DFile)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fdatasync_f_impl(int fd);
#ifndef DEFINED_kwlist__fd
#define DEFINED_kwlist__fd
PRIVATE DEFINE_KWLIST(kwlist__fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !DEFINED_kwlist__fd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fdatasync_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_fd;
	} args;
	int fd;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fd, "o:fdatasync", &args))
		goto err;
	if unlikely((fd = DeeUnixSystem_GetFD(args.raw_fd)) == -1)
		goto err;
	return posix_fdatasync_f_impl(fd);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fdatasync, &posix_fdatasync_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fdatasync_f_impl(int fd)
/*[[[end]]]*/
{
#ifdef posix_fdatasync_USE_fdatasync
	int result;
	again:
	DBG_ALIGNMENT_DISABLE();
	result = fdatasync(fd);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		DeeUnixSystem_HandleGenericError(error, err, again);
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

#ifdef posix_fdatasync_USE_posix_fsync
	return posix_fsync_f_impl(fd);
#endif /* !posix_fdatasync_USE_posix_fsync */

#ifdef posix_fdatasync_USE_STUB
#define NEED_posix_err_unsupported
	(void)fd;
	posix_err_unsupported("fdatasync");
	return NULL;
#endif /* !CONFIG_HAVE_fdatasync */
}



DECL_END

#endif /* !GUARD_DEX_POSIX_P_SYNC_C_INL */
