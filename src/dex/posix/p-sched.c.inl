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
#ifndef GUARD_DEX_POSIX_P_SCHED_C_INL
#define GUARD_DEX_POSIX_P_SCHED_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN

/* Figure out how to implement `system()' */
#undef posix_system_USE_wsystem
#undef posix_system_USE_system
#undef posix_system_USE_fork_AND_wexec
#undef posix_system_USE_fork_AND_exec
#undef posix_system_USE_STUB
#if defined(CONFIG_HAVE_wsystem) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_system_USE_wsystem
#elif defined(CONFIG_HAVE_vfork) && defined(CONFIG_HAVE_wexecv) && \
      defined(CONFIG_HAVE_waitpid) && defined(CONFIG_HAVE__Exit) && \
      defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_system_USE_fork_AND_wexec
#elif defined(CONFIG_HAVE_system)
#define posix_system_USE_system
#elif defined(CONFIG_HAVE_vfork) && defined(CONFIG_HAVE_execv) && \
      defined(CONFIG_HAVE_waitpid) && defined(CONFIG_HAVE__Exit)
#define posix_system_USE_fork_AND_exec
#elif defined(CONFIG_HAVE_wsystem)
#define posix_system_USE_wsystem
#elif defined(CONFIG_HAVE_vfork) && defined(CONFIG_HAVE_wexecv) && \
      defined(CONFIG_HAVE_waitpid) && defined(CONFIG_HAVE__Exit)
#define posix_system_USE_fork_AND_wexec
#else /* ... */
#define posix_system_USE_STUB
#endif /* !... */

/* Figure out how to implement `getpid()' */
#undef posix_getpid_USE_getpid
#undef posix_getpid_USE_STUB
#ifdef CONFIG_HAVE_getpid
#define posix_getpid_USE_getpid
#else /* CONFIG_HAVE_getpid */
#define posix_getpid_USE_STUB
#endif /* !CONFIG_HAVE_getpid */




/************************************************************************/
/* system()                                                             */
/************************************************************************/

#if defined(posix_system_USE_wsystem) || defined(posix_system_USE_fork_AND_wexec) || defined(__DEEMON__)
/*[[[deemon import("rt.dexutils").gw("system", "command:c:wchar_t[]->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_system_f_impl(dwchar_t const *command);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_SYSTEM_DEF { "system", (DeeObject *)&posix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint") },
#define POSIX_SYSTEM_DEF_DOC(doc) { "system", (DeeObject *)&posix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_system, posix_system_f);
#ifndef POSIX_KWDS_COMMAND_DEFINED
#define POSIX_KWDS_COMMAND_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_command, { K(command), KEND });
#endif /* !POSIX_KWDS_COMMAND_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	dwchar_t const *command_str;
	DeeStringObject *command;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_command, "o:system", &command))
		goto err;
	if (DeeObject_AssertTypeExact(command, &DeeString_Type))
		goto err;
	command_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)command);
	if unlikely(!command_str)
		goto err;
	return posix_system_f_impl(command_str);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_system_f_impl(dwchar_t const *command)
//[[[end]]]
#endif /* posix_system_USE_wsystem || posix_system_USE_fork_AND_wexec */
#if (!defined(posix_system_USE_wsystem) && !defined(posix_system_USE_fork_AND_wexec)) || defined(__DEEMON__)
/*[[[deemon import("rt.dexutils").gw("system", "command:c:char[]->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_system_f_impl(/*utf-8*/ char const *command);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_SYSTEM_DEF { "system", (DeeObject *)&posix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint") },
#define POSIX_SYSTEM_DEF_DOC(doc) { "system", (DeeObject *)&posix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_system, posix_system_f);
#ifndef POSIX_KWDS_COMMAND_DEFINED
#define POSIX_KWDS_COMMAND_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_command, { K(command), KEND });
#endif /* !POSIX_KWDS_COMMAND_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/*utf-8*/ char const *command_str;
	DeeStringObject *command;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_command, "o:system", &command))
		goto err;
	if (DeeObject_AssertTypeExact(command, &DeeString_Type))
		goto err;
	command_str = DeeString_AsUtf8((DeeObject *)command);
	if unlikely(!command_str)
		goto err;
	return posix_system_f_impl(command_str);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_system_f_impl(/*utf-8*/ char const *command)
//[[[end]]]
#endif /* !posix_system_USE_wsystem && !posix_system_USE_fork_AND_wexec */
{
#if (defined(posix_system_USE_wsystem) || \
     defined(posix_system_USE_system))
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_system_USE_wsystem
	result = wsystem(command);
#else /* posix_system_USE_wsystem */
	result = system(command);
#endif /* !posix_system_USE_wsystem */
	DBG_ALIGNMENT_ENABLE();
	EINTR_HANDLE(DeeSystem_GetErrno(), again, err)
	return DeeInt_NewInt(result);
#ifdef EINTR
err:
	return NULL;
#endif /* EINTR */
#endif /* posix_system_USE_wsystem || posix_system_USE_system */

#if defined(posix_system_USE_fork_AND_wexec) || defined(posix_system_USE_fork_AND_exec)
	int cpid, error;
	int status;
EINTR_LABEL(again)
	cpid = vfork();
	if (cpid == 0) {
#ifdef posix_system_USE_fork_AND_wexec
#define FORKEXEC_CHART dwchar_t
#define FORKEXEC_EXEC  wexecv
#else /* posix_system_USE_fork_AND_wexec */
#define FORKEXEC_CHART char
#define FORKEXEC_EXEC  execv
#endif /* !posix_system_USE_fork_AND_wexec */
		FORKEXEC_CHART const *argv[4];
		PRIVATE FORKEXEC_CHART const dash_c[] = { '-', 'c', 0 };
		PRIVATE FORKEXEC_CHART const bin_sh[] = { '/', 'b', 'i', 'n', '/', 's', 'h', 0 };
		PRIVATE FORKEXEC_CHART const bin_bash[] = { '/', 'b', 'i', 'n', '/', 'b', 'a', 's', 'h', 0 };
		PRIVATE FORKEXEC_CHART const bin_dash[] = { '/', 'b', 'i', 'n', '/', 'd', 'a', 's', 'h', 0 };
		PRIVATE FORKEXEC_CHART const bin_ash[] = { '/', 'b', 'i', 'n', '/', 'a', 's', 'h', 0 };
		PRIVATE FORKEXEC_CHART const bin_busybox[] = { '/', 'b', 'i', 'n', '/', 'b', 'u', 's', 'y', 'b', 'o', 'x', 0 };
		argv[1] = dash_c;
		argv[2] = command;
		argv[3] = NULL;
		argv[0] = bin_sh + 5;
		FORKEXEC_EXEC(bin_sh, argv);
		argv[0] = bin_bash + 5;
		FORKEXEC_EXEC(bin_bash, argv);
		argv[0] = bin_sh + 5;
		FORKEXEC_EXEC(bin_busybox, argv);
		argv[0] = bin_dash + 5;
		FORKEXEC_EXEC(bin_dash, argv);
		argv[0] = bin_ash + 5;
		FORKEXEC_EXEC(bin_ash, argv);
		/* NOTE: system() must return ZERO(0) if no command processor is available. */
		_Exit(command ? 127 : 0);
#undef FORKEXEC_CHART
#undef FORKEXEC_EXEC
	}
	if (cpid < 0) {
		status = -1;
	} else {
		for (;;) {
			error = waitpid(cpid, &status, 0);
			if (error == cpid)
				break;
			if (error >= 0)
				continue;
			EINTR_HANDLE(DeeSystem_GetErrno(), again, err)
		}
		status = WIFEXITED(status)
		         ? WEXITSTATUS(status)
		         :;
	}
	return DeeInt_NewInt(status);
#ifdef EINTR
err:
	return NULL;
#endif /* EINTR */
#endif /* posix_system_USE_fork_AND_wexec || posix_system_USE_fork_AND_exec */

#ifdef posix_system_USE_STUB
#define NEED_posix_err_unsupported
	(void)command;
	posix_err_unsupported("system");
	return NULL;
#endif /* posix_system_USE_STUB */

}




/************************************************************************/
/* sched_yield()                                                        */
/************************************************************************/

/*[[[deemon import("rt.dexutils").gw("sched_yield", "", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_sched_yield_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sched_yield_f(size_t argc, DeeObject *const *argv);
#define POSIX_SCHED_YIELD_DEF { "sched_yield", (DeeObject *)&posix_sched_yield, MODSYM_FNORMAL, DOC("()") },
#define POSIX_SCHED_YIELD_DEF_DOC(doc) { "sched_yield", (DeeObject *)&posix_sched_yield, MODSYM_FNORMAL, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_sched_yield, posix_sched_yield_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sched_yield_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sched_yield"))
		goto err;
	return posix_sched_yield_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_sched_yield_f_impl(void)
//[[[end]]]
{
	SCHED_YIELD();
	return_none;
}




/************************************************************************/
/* getpid()                                                             */
/************************************************************************/

/*[[[deemon import("rt.dexutils").gw("getpid", "->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getpid_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getpid_f(size_t argc, DeeObject *const *argv);
#define POSIX_GETPID_DEF { "getpid", (DeeObject *)&posix_getpid, MODSYM_FNORMAL, DOC("->?Dint") },
#define POSIX_GETPID_DEF_DOC(doc) { "getpid", (DeeObject *)&posix_getpid, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_getpid, posix_getpid_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getpid_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":getpid"))
		goto err;
	return posix_getpid_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getpid_f_impl(void)
//[[[end]]]
{
#ifdef posix_getpid_USE_getpid
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = getpid();
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		if (error != 0) {
			EINTR_HANDLE(error, again, err)
			HANDLE_ENOSYS(error, err, "getpid")
			DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
			                          "Failed to get pid");
			goto err;
		}
	}
	return DeeInt_NewInt(result);
err:
	return NULL;
#endif /* posix_getpid_USE_getpid */

#ifdef posix_getpid_USE_STUB
#define NEED_posix_err_unsupported
	(void)command;
	posix_err_unsupported("getpid");
	return NULL;
#endif /* !posix_getpid_USE_STUB */
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_SCHED_C_INL */
