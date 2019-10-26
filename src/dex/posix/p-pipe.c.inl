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
#ifndef GUARD_DEX_POSIX_P_PIPE_C_INL
#define GUARD_DEX_POSIX_P_PIPE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN


/* Figure out how to implement `pipe()' */
#undef p_pipe_USE_PIPE
#undef p_pipe_USE_CREATEPIPE
#undef p_pipe_USE_STUB
#if defined(CONFIG_HAVE_pipe) || defined(CONFIG_HAVE_pipe2)
#define p_pipe_USE_PIPE 1
#elif defined(CONFIG_HAVE__open_osfhandle) && defined(CONFIG_HOST_WINDOWS)
#define p_pipe_USE_CREATEPIPE 1
#else
#define p_pipe_USE_STUB 1
#endif


/* Figure out how to implement `pipe2()' */
#undef p_pipe2_USE_PIPE2
#undef p_pipe2_USE_CREATEPIPE
#undef p_pipe2_USE_PIPE
#undef p_pipe2_USE_STUB
#if defined(CONFIG_HAVE_pipe2)
#define p_pipe2_USE_PIPE2 1
#elif (defined(CONFIG_HAVE_pipe) && defined(CONFIG_HAVE_fcntl) && \
       defined(CONFIG_HAVE_F_SETFD) && defined(CONFIG_HAVE_FD_CLOEXEC) && \
       defined(O_CLOEXEC) && (!defined(O_NONBLOCK) || defined(CONFIG_HAVE_F_SETFL)))
#elif defined(CONFIG_HAVE__open_osfhandle) && defined(CONFIG_HOST_WINDOWS)
#define p_pipe2_USE_CREATEPIPE 1
#elif !defined(O_CLOEXEC) && !defined(O_NONBLOCK) && !defined(p_pipe_USE_STUB)
#define p_pipe2_USE_PIPE 1
#else
#define p_pipe2_USE_STUB 1
#endif




/************************************************************************/
/* pipe()                                                               */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("pipe", "->?T2?Dint?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe_f(size_t argc, DeeObject **argv);
#define POSIX_PIPE_DEF { "pipe", (DeeObject *)&posix_pipe, MODSYM_FNORMAL, DOC("->?T2?Dint?Dint") },
#define POSIX_PIPE_DEF_DOC(doc) { "pipe", (DeeObject *)&posix_pipe, MODSYM_FNORMAL, DOC("->?T2?Dint?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_pipe, posix_pipe_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":pipe"))
	    goto err;
	return posix_pipe_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe_f_impl(void)
//[[[end]]]
{
#ifdef p_pipe_USE_PIPE
	DREF DeeObject *result;
	int error;
	int fds[2];
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_pipe
	error = pipe(fds);
#else /* CONFIG_HAVE_pipe */
	error = pipe2(fds, 0);
#endif /* !CONFIG_HAVE_pipe */
	DBG_ALIGNMENT_ENABLE();
	if (error < 0) {
		error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "pipe")
		/* TODO: Other errors */
		DeeError_SysThrowf(&DeeError_SystemError, error,
		                   "Failed to create pipe");
		goto err;
	}
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
	if unlikely(!result)
		goto err_fds;
	return result;
err_fds:
	DBG_ALIGNMENT_DISABLE();
	close(fds[1]);
	close(fds[0]);
	DBG_ALIGNMENT_ENABLE();
err:
	return NULL;
#endif /* p_pipe_USE_PIPE */

#ifdef p_pipe_USE_CREATEPIPE
	DREF DeeObject *result;
	HANDLE hRead, hWrite;
	int fds[2];
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if (!CreatePipe(&hRead, &hWrite, NULL, 0)) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError))
			goto again;
		DeeError_SysThrowf(&DeeError_SystemError, dwError,
		                   "Failed to create pipe");
		goto err;
	}
	/* On unix, pipe handles are inheritable by default */
	if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
		goto err_hWritehRead_nterror;
	if (!SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
		goto err_hWritehRead_nterror;
	fds[0] = _open_osfhandle((intptr_t)(uintptr_t)hRead, O_RDONLY);
	if unlikely(fds[0] < 0)
		goto err_hWritehRead_errno;
	fds[1] = _open_osfhandle((intptr_t)(uintptr_t)hWrite, O_WRONLY);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(fds[1] < 0)
		goto err_hWritefds0_errno;
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
	if unlikely(!result)
		goto err_fds;
	return result;
err_fds:
	DBG_ALIGNMENT_DISABLE();
	close(fds[1]);
	close(fds[0]);
	goto err;
err_hWritefds0_errno:
	DeeError_SysThrowf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                   "Failed to create pipe");
	close(fds[0]);
	goto err_hWrite;
err_hWritehRead_errno:
	DeeError_SysThrowf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                   "Failed to create pipe");
	goto err_hWritehRead;
err_hWritehRead_nterror:
	DeeError_SysThrowf(&DeeError_SystemError, GetLastError(),
	                   "Failed to create pipe");
err_hWritehRead:
	CloseHandle(hRead);
err_hWrite:
	CloseHandle(hWrite);
	DBG_ALIGNMENT_ENABLE();
err:
	return NULL;
#endif /* p_pipe_USE_CREATEPIPE */

#ifdef p_pipe_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	posix_err_unsupported("pipe");
	return NULL;
#endif /* p_pipe_USE_STUB */
}





/************************************************************************/
/* pipe2()                                                              */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("pipe2", "oflags:d->?T2?Dint?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe2_f_impl(int oflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe2_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_PIPE2_DEF { "pipe2", (DeeObject *)&posix_pipe2, MODSYM_FNORMAL, DOC("(oflags:?Dint)->?T2?Dint?Dint") },
#define POSIX_PIPE2_DEF_DOC(doc) { "pipe2", (DeeObject *)&posix_pipe2, MODSYM_FNORMAL, DOC("(oflags:?Dint)->?T2?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_pipe2, posix_pipe2_f);
#ifndef POSIX_KWDS_OFLAGS_DEFINED
#define POSIX_KWDS_OFLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_oflags, { K(oflags), KEND });
#endif /* !POSIX_KWDS_OFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pipe2_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int oflags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oflags, "d:pipe2", &oflags))
	    goto err;
	return posix_pipe2_f_impl(oflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pipe2_f_impl(int oflags)
//[[[end]]]
{
#if defined(p_pipe2_USE_PIPE2) || defined(p_pipe2_USE_PIPE_FCNTL)
	DREF DeeObject *result;
	int error;
	int fds[2];
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef p_pipe2_USE_PIPE2
	error = pipe2(fds, oflags);
#else /* p_pipe2_USE_PIPE2 */
#ifdef O_NONBLOCK
	if (oflags & ~(O_CLOEXEC | O_NONBLOCK))
#else /* O_NONBLOCK */
	if (oflags & ~(O_CLOEXEC))
#endif /* !O_NONBLOCK */
	{
		DeeSystem_SetErrno(EINVAL);
		DBG_ALIGNMENT_ENABLE();
		return_none;
	}
	error = pipe(fds);
	if (error >= 0) {
		if (oflags & O_CLOEXEC) {
			error = fcntl(fds[0], F_SETFD, FD_CLOEXEC);
			if unlikely(error < 0)
				goto err_fds_errno;
			error = fcntl(fds[1], F_SETFD, FD_CLOEXEC);
			if unlikely(error < 0)
				goto err_fds_errno;
		}
#ifdef O_NONBLOCK
		if (oflags & O_NONBLOCK) {
			error = fcntl(fds[0], F_SETFL, O_NONBLOCK);
			if unlikely(error < 0)
				goto err_fds_errno;
			error = fcntl(fds[1], F_SETFL, O_NONBLOCK);
			if unlikely(error < 0)
				goto err_fds_errno;
		}
#endif /* O_NONBLOCK */
	}
#endif /* !p_pipe2_USE_PIPE2 */
	DBG_ALIGNMENT_ENABLE();
	if (error < 0) {
		error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "pipe")
		/* TODO: Other errors */
		DeeError_SysThrowf(&DeeError_SystemError, error,
		                   "Failed to create pipe");
		goto err;
	}
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
	if unlikely(!result)
		goto err_fds;
	return result;
#ifndef HAVE_PIPE2
err_fds_errno:
#ifdef EINTR
	if (DeeSystem_GetErrno() == EINTR) {
		DBG_ALIGNMENT_DISABLE();
		close(fds[1]);
		close(fds[0]);
		DBG_ALIGNMENT_ENABLE();
		goto again;
	}
#endif /* EINTR */
#endif /* !HAVE_PIPE2 */
err_fds:
	DBG_ALIGNMENT_DISABLE();
	close(fds[1]);
	close(fds[0]);
	DBG_ALIGNMENT_ENABLE();
err:
	return NULL;
#endif /* p_pipe2_USE_PIPE2 || p_pipe2_USE_PIPE_FCNTL */

#ifdef p_pipe2_USE_CREATEPIPE
	DREF DeeObject *result;
	HANDLE hRead, hWrite;
	int fds[2];
	if (oflags & ~(O_CLOEXEC)) {
		DeeSystem_SetErrno(EINVAL);
		return_none;
	}
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if (!CreatePipe(&hRead, &hWrite, NULL, 0)) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError))
			goto again;
		DeeError_SysThrowf(&DeeError_SystemError, dwError,
		                   "Failed to create pipe");
		goto err;
	}
	if (!(oflags & O_CLOEXEC)) {
		if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
			goto err_hWritehRead_nterror;
		if (!SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
			goto err_hWritehRead_nterror;
	}
	fds[0] = _open_osfhandle((intptr_t)(uintptr_t)hRead, O_RDONLY);
	if unlikely(fds[0] < 0)
		goto err_hWritehRead_errno;
	fds[1] = _open_osfhandle((intptr_t)(uintptr_t)hWrite, O_WRONLY);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(fds[1] < 0)
		goto err_hWritefds0_errno;
	result = DeeTuple_Newf("dd", fds[0], fds[1]);
	if unlikely(!result)
		goto err_fds;
	return result;
err_fds:
	DBG_ALIGNMENT_DISABLE();
	close(fds[1]);
	close(fds[0]);
	goto err;
err_hWritefds0_errno:
	DeeError_SysThrowf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                   "Failed to create pipe");
	close(fds[0]);
	goto err_hWrite;
err_hWritehRead_errno:
	DeeError_SysThrowf(&DeeError_SystemError, DeeSystem_GetErrno(),
	                   "Failed to create pipe");
	goto err_hWritehRead;
err_hWritehRead_nterror:
	DeeError_SysThrowf(&DeeError_SystemError, GetLastError(),
	                   "Failed to create pipe");
err_hWritehRead:
	CloseHandle(hRead);
err_hWrite:
	CloseHandle(hWrite);
	DBG_ALIGNMENT_ENABLE();
err:
	return NULL;
#endif /* p_pipe2_USE_CREATEPIPE */

#ifdef p_pipe2_USE_PIPE
	(void)oflags;
	return posix_pipe_f_impl();
#endif /* p_pipe2_USE_PIPE */

#ifdef p_pipe2_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)oflags;
	posix_err_unsupported("pipe2");
	return NULL;
#endif /* p_pipe2_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_PIPE_C_INL */
