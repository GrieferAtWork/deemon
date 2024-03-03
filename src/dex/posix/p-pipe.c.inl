/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_PIPE_C_INL
#define GUARD_DEX_POSIX_P_PIPE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN


/* Figure out how to implement `pipe()' */
#undef posix_pipe_USE_pipe
#undef posix_pipe_USE_CreatePipe
#undef posix_pipe_USE_STUB
#ifdef CONFIG_HAVE_pipe
#define posix_pipe_USE_pipe
#elif defined(CONFIG_HAVE_open_osfhandle) && defined(CONFIG_HOST_WINDOWS)
#define posix_pipe_USE_CreatePipe
#else /* ... */
#define posix_pipe_USE_STUB
#endif /* !... */


/* Figure out how to implement `pipe2()' */
#undef posix_pipe2_USE_pipe2
#undef posix_pipe2_USE_pipe_AND_fcntl
#undef posix_pipe2_USE_CreatePipe
#undef posix_pipe2_USE_pipe
#undef posix_pipe2_USE_STUB
#if defined(CONFIG_HAVE_pipe2)
#define posix_pipe2_USE_pipe2
#elif !defined(CONFIG_HAVE_O_CLOEXEC) && !defined(O_NONBLOCK) && !defined(posix_pipe_USE_STUB)
#define posix_pipe2_USE_pipe
#elif (defined(CONFIG_HAVE_pipe) && defined(CONFIG_HAVE_fcntl) &&                                                \
       (!defined(CONFIG_HAVE_O_CLOEXEC) || (defined(CONFIG_HAVE_F_SETFD) && defined(CONFIG_HAVE_FD_CLOEXEC))) && \
       (!defined(CONFIG_HAVE_O_NONBLOCK) || defined(CONFIG_HAVE_F_SETFL)))
#define posix_pipe2_USE_pipe_AND_fcntl
#elif defined(CONFIG_HAVE__open_osfhandle) && defined(CONFIG_HOST_WINDOWS)
#define posix_pipe2_USE_CreatePipe
#else /* ... */
#define posix_pipe2_USE_STUB
#endif /* !... */




/************************************************************************/
/* pipe()                                                               */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("pipe", "->?T2?Dint?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe_f(size_t argc, DeeObject *const *argv);
#define POSIX_PIPE_DEF { "pipe", (DeeObject *)&posix_pipe, MODSYM_FREADONLY, DOC("->?T2?Dint?Dint") },
#define POSIX_PIPE_DEF_DOC(doc) { "pipe", (DeeObject *)&posix_pipe, MODSYM_FREADONLY, DOC("->?T2?Dint?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_pipe, posix_pipe_f, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":pipe"))
		goto err;
	return posix_pipe_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe_f_impl(void)
/*[[[end]]]*/
{
#ifdef posix_pipe_USE_pipe
	DREF DeeObject *result;
	int error;
	int fds[2];
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	error = pipe(fds);
	DBG_ALIGNMENT_ENABLE();
	if (error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err)
		HANDLE_ENOSYS(error, err, "pipe")
		/* TODO: Other errors */
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to create pipe");
		goto err;
	}
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
	if unlikely(!result)
		goto err_fds;
	return result;
err_fds:
#ifdef CONFIG_HAVE_close
	DBG_ALIGNMENT_DISABLE();
	(void)close(fds[1]);
	(void)close(fds[0]);
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_close */
err:
	return NULL;
#endif /* posix_pipe_USE_pipe */

#ifdef posix_pipe_USE_CreatePipe
	DREF DeeObject *result;
	HANDLE hRead, hWrite;
	int fds[2];
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if (!CreatePipe(&hRead, &hWrite, NULL, 0)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError))
			goto again;
		DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
		                        "Failed to create pipe");
		goto err;
	}
	/* On unix, pipe handles are inheritable by default */
	if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
		goto err_hWritehRead_nterror;
	if (!SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
		goto err_hWritehRead_nterror;
	fds[0] = open_osfhandle((intptr_t)(uintptr_t)hRead, O_RDONLY);
	if unlikely(fds[0] < 0)
		goto err_hWritehRead_errno;
	fds[1] = open_osfhandle((intptr_t)(uintptr_t)hWrite, O_WRONLY);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(fds[1] < 0)
		goto err_hWritefds0_errno;
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
	if unlikely(!result)
		goto err_fds;
	return result;
err_fds:
#ifdef CONFIG_HAVE_close
	DBG_ALIGNMENT_DISABLE();
	(void)close(fds[1]);
	(void)close(fds[0]);
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_close */
	goto err;
err_hWritefds0_errno:
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                          "Failed to create pipe");
#ifdef CONFIG_HAVE_close
	DBG_ALIGNMENT_DISABLE();
	(void)close(fds[0]);
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_close */
	goto err_hWrite;
err_hWritehRead_errno:
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                          "Failed to create pipe");
	goto err_hWritehRead;
err_hWritehRead_nterror:
	DeeNTSystem_ThrowErrorf(&DeeError_SystemError, GetLastError(),
	                        "Failed to create pipe");
err_hWritehRead:
	CloseHandle(hRead);
err_hWrite:
	CloseHandle(hWrite);
	DBG_ALIGNMENT_ENABLE();
err:
	return NULL;
#endif /* posix_pipe_USE_CreatePipe */

#ifdef posix_pipe_USE_STUB
#define NEED_posix_err_unsupported
	posix_err_unsupported("pipe");
	return NULL;
#endif /* posix_pipe_USE_STUB */
}





/************************************************************************/
/* pipe2()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("pipe2", "oflags:d->?T2?Dint?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe2_f_impl(int oflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe2_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_PIPE2_DEF { "pipe2", (DeeObject *)&posix_pipe2, MODSYM_FREADONLY, DOC("(oflags:?Dint)->?T2?Dint?Dint") },
#define POSIX_PIPE2_DEF_DOC(doc) { "pipe2", (DeeObject *)&posix_pipe2, MODSYM_FREADONLY, DOC("(oflags:?Dint)->?T2?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_pipe2, &posix_pipe2_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_OFLAGS_DEFINED
#define POSIX_KWDS_OFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oflags, { KEX("oflags", 0xbe92b5be, 0x4f84e498f7c9d171), KEND });
#endif /* !POSIX_KWDS_OFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe2_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int oflags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oflags, "d:pipe2", &oflags))
		goto err;
	return posix_pipe2_f_impl(oflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe2_f_impl(int oflags)
/*[[[end]]]*/
{
#if defined(posix_pipe2_USE_pipe2) || defined(posix_pipe2_USE_pipe_AND_fcntl)
	DREF DeeObject *result;
	int error;
	int fds[2];
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();

#ifdef posix_pipe2_USE_pipe2
	error = pipe2(fds, oflags);
	if unlikely(error < 0)
		goto handle_system_error;
#endif /* posix_pipe2_USE_pipe2 */

#ifdef posix_pipe2_USE_pipe_AND_fcntl
	/* Validate the given `oflags' */
#if defined(CONFIG_HAVE_O_NONBLOCK) && defined(CONFIG_HAVE_O_CLOEXEC)
	if (oflags & ~(O_CLOEXEC | O_NONBLOCK))
#elif defined(CONFIG_HAVE_O_NONBLOCK)
	if (oflags & ~(O_NONBLOCK))
#else /* ... */
	if (oflags & ~(O_CLOEXEC))
#endif /* !... */
	{
		DeeSystem_SetErrno(EINVAL);
		DBG_ALIGNMENT_ENABLE();
		goto handle_system_error;
	}
	error = pipe(fds);
	if (error < 0)
		goto handle_system_error;

	/* Apply O_CLOEXEC */
#ifdef CONFIG_HAVE_O_CLOEXEC
	if (oflags & O_CLOEXEC) {
		error = fcntl(fds[0], F_SETFD, FD_CLOEXEC);
		if unlikely(error < 0)
			goto handle_system_error_fds;
		error = fcntl(fds[1], F_SETFD, FD_CLOEXEC);
		if unlikely(error < 0)
			goto handle_system_error_fds;
	}
#endif /* CONFIG_HAVE_O_CLOEXEC */

	/* Apply O_NONBLOCK */
#ifdef CONFIG_HAVE_O_NONBLOCK
	if (oflags & O_NONBLOCK) {
		error = fcntl(fds[0], F_SETFL, O_NONBLOCK);
		if unlikely(error < 0)
			goto handle_system_error_fds;
		error = fcntl(fds[1], F_SETFL, O_NONBLOCK);
		if unlikely(error < 0)
			goto handle_system_error_fds;
	}
#endif /* CONFIG_HAVE_O_NONBLOCK */
#endif /* posix_pipe2_USE_pipe_AND_fcntl */

	DBG_ALIGNMENT_ENABLE();
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
#ifdef CONFIG_HAVE_close
	if unlikely(!result) {
		DBG_ALIGNMENT_DISABLE();
		(void)close(fds[1]);
		(void)close(fds[0]);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_close */
	return result;

#ifdef posix_pipe2_USE_pipe_AND_fcntl
handle_system_error_fds:
#ifdef CONFIG_HAVE_close
	error = DeeSystem_GetErrno();
	(void)close(fds[1]);
	(void)close(fds[0]);
	DeeSystem_SetErrno(error);
#endif /* CONFIG_HAVE_close */
#endif /* posix_pipe2_USE_pipe_AND_fcntl */
handle_system_error:
	error = DeeSystem_GetErrno();
	DBG_ALIGNMENT_ENABLE();
	EINTR_HANDLE(error, again, err)
	HANDLE_ENOSYS(error, err, "pipe2")
	HANDLE_EINVAL(error, err, "Invalid oflags for pipe2 %#x", oflags)
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
	                          "Failed to create pipe");
err:
	return NULL;
#endif /* posix_pipe2_USE_pipe2 || posix_pipe2_USE_pipe_AND_fcntl */

#ifdef posix_pipe2_USE_CreatePipe
	DREF DeeObject *result;
	HANDLE hRead, hWrite;
	int fds[2];
#ifdef CONFIG_HAVE_O_CLOEXEC
	if (oflags & ~(O_CLOEXEC))
#else /* CONFIG_HAVE_O_CLOEXEC */
	if (oflags != 0)
#endif /* !CONFIG_HAVE_O_CLOEXEC */
	{
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid oflags for pipe2 %#x",
		                oflags);
		goto err;
	}
again:
	DBG_ALIGNMENT_DISABLE();
	if (!CreatePipe(&hRead, &hWrite, NULL, 0)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
		                        "Failed to create pipe");
		goto err;
	}
#ifdef CONFIG_HAVE_O_CLOEXEC
	if (!(oflags & O_CLOEXEC))
#endif /* CONFIG_HAVE_O_CLOEXEC */
	{
		if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
			goto err_hWritehRead_nterror;
		if (!SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
			goto err_hWritehRead_nterror;
	}
	fds[0] = open_osfhandle((intptr_t)(uintptr_t)hRead, O_RDONLY);
	if unlikely(fds[0] < 0)
		goto err_hWritehRead_errno;
	fds[1] = open_osfhandle((intptr_t)(uintptr_t)hWrite, O_WRONLY);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(fds[1] < 0)
		goto err_hWritefds0_errno;
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
#ifdef CONFIG_HAVE_close
	if unlikely(!result) {
		DBG_ALIGNMENT_DISABLE();
		(void)close(fds[1]);
		(void)close(fds[0]);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_close */
	return result;
err_hWritefds0_errno:
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                          "Failed to create pipe");
#ifdef CONFIG_HAVE_close
	(void)close(fds[0]);
#endif /* CONFIG_HAVE_close */
	goto err_hWrite;
err_hWritehRead_errno:
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                          "Failed to create pipe");
	goto err_hWritehRead;
err_hWritehRead_nterror:
	DeeNTSystem_ThrowLastErrorf(&DeeError_SystemError,
	                            "Failed to create pipe");
err_hWritehRead:
	CloseHandle(hRead);
err_hWrite:
	CloseHandle(hWrite);
	DBG_ALIGNMENT_ENABLE();
err:
	return NULL;
#endif /* posix_pipe2_USE_CreatePipe */

#ifdef posix_pipe2_USE_pipe
	if unlikely(oflags != 0) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid oflags for pipe2 %#x",
		                oflags);
		return NULL;
	}
	return posix_pipe_f_impl();
#endif /* posix_pipe2_USE_pipe */

#ifdef posix_pipe2_USE_STUB
#define NEED_posix_err_unsupported
	(void)oflags;
	posix_err_unsupported("pipe2");
	return NULL;
#endif /* posix_pipe2_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_PIPE_C_INL */
