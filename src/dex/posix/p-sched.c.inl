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
#ifndef GUARD_DEX_POSIX_P_SCHED_C_INL
#define GUARD_DEX_POSIX_P_SCHED_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DEFINE_KWLIST, DeeArg_UnpackStructKw */
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/sched/yield.h>     /* SCHED_YIELD */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

/* Figure out how to implement `system()' */
#undef posix_system_USE_wsystem
#undef posix_system_USE_system
#undef posix_system_USE_fork_AND_wexec
#undef posix_system_USE_fork_AND_exec
#undef posix_system_USE_posix_spawn
#undef posix_system_USE_wspawnve
#undef posix_system_USE_spawnve
#undef posix_system_USE_ipc_Process
#undef posix_system_USE_STUB
#if defined(CONFIG_HAVE_wsystem) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_system_USE_wsystem
#elif defined(CONFIG_HAVE_wspawnve) && defined(P_WAIT) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS) && 0
#define posix_system_USE_wspawnve /* TODO */
#elif (defined(CONFIG_HAVE_vfork) && defined(CONFIG_HAVE_wexecv) && \
       defined(CONFIG_HAVE_waitpid) && defined(CONFIG_HAVE__Exit) && \
       defined(CONFIG_PREFER_WCHAR_FUNCTIONS))
#define posix_system_USE_fork_AND_wexec
#elif defined(CONFIG_HAVE_system)
#define posix_system_USE_system
#elif defined(CONFIG_HAVE_spawnve) && defined(P_WAIT) && 0
#define posix_system_USE_spawnve /* TODO */
#elif defined(CONFIG_HAVE_SPAWN_H) && defined(CONFIG_HAVE_posix_spawn) && 0
#define posix_system_USE_posix_spawn /* TODO */
#elif (defined(CONFIG_HAVE_vfork) && defined(CONFIG_HAVE_execv) && \
       defined(CONFIG_HAVE_waitpid) && defined(CONFIG_HAVE__Exit))
#define posix_system_USE_fork_AND_exec
#elif defined(CONFIG_HAVE_wsystem)
#define posix_system_USE_wsystem
#elif (defined(CONFIG_HAVE_vfork) && defined(CONFIG_HAVE_wexecv) && \
       defined(CONFIG_HAVE_waitpid) && defined(CONFIG_HAVE__Exit))
#define posix_system_USE_fork_AND_wexec
#elif defined(CONFIG_HAVE_wspawnve) && defined(P_WAIT) && 0
#define posix_system_USE_wspawnve /* TODO */
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
/*[[[deemon import("rt.gen.dexutils").gw("system", "command:c:wchar_t[]->?Dint", libname: "posix"); ]]]*/
#define POSIX_SYSTEM_DEF          DEX_MEMBER_F("system", &posix_system, DEXSYM_READONLY, "(command:?Dstring)->?Dint"),
#define POSIX_SYSTEM_DEF_DOC(doc) DEX_MEMBER_F("system", &posix_system, DEXSYM_READONLY, "(command:?Dstring)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_system_f_impl(Dee_wchar_t const *command);
#ifndef DEFINED_kwlist__command
#define DEFINED_kwlist__command
PRIVATE DEFINE_KWLIST(kwlist__command, { KEX("command", 0xe876e30d, 0x2a35e05780aa8e3b), KEND });
#endif /* !DEFINED_kwlist__command */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		Dee_wchar_t const *command;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__command, "ls:system", &args))
		goto err;
	return posix_system_f_impl(args.command);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_system, &posix_system_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_system_f_impl(Dee_wchar_t const *command)
/*[[[end]]]*/
#endif /* posix_system_USE_wsystem || posix_system_USE_fork_AND_wexec */
#if (!defined(posix_system_USE_wsystem) && !defined(posix_system_USE_fork_AND_wexec)) || defined(__DEEMON__)
/*[[[deemon import("rt.gen.dexutils").gw("system", "command:c:char[]->?Dint", libname: "posix"); ]]]*/
#define POSIX_SYSTEM_DEF          DEX_MEMBER_F("system", &posix_system, DEXSYM_READONLY, "(command:?Dstring)->?Dint"),
#define POSIX_SYSTEM_DEF_DOC(doc) DEX_MEMBER_F("system", &posix_system, DEXSYM_READONLY, "(command:?Dstring)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_system_f_impl(char const *command);
#ifndef DEFINED_kwlist__command
#define DEFINED_kwlist__command
PRIVATE DEFINE_KWLIST(kwlist__command, { KEX("command", 0xe876e30d, 0x2a35e05780aa8e3b), KEND });
#endif /* !DEFINED_kwlist__command */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		char const *command;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__command, "Us:system", &args))
		goto err;
	return posix_system_f_impl(args.command);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_system, &posix_system_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_system_f_impl(char const *command)
/*[[[end]]]*/
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
#define FORKEXEC_CHART wchar_t
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

/*[[[deemon import("rt.gen.dexutils").gw("sched_yield", "", libname: "posix"); ]]]*/
#define POSIX_SCHED_YIELD_DEF          DEX_MEMBER_F("sched_yield", &posix_sched_yield, DEXSYM_READONLY, "()"),
#define POSIX_SCHED_YIELD_DEF_DOC(doc) DEX_MEMBER_F("sched_yield", &posix_sched_yield, DEXSYM_READONLY, "()\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sched_yield_f_impl(void);
PRIVATE DEFINE_CMETHOD0(posix_sched_yield, &posix_sched_yield_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sched_yield_f_impl(void)
/*[[[end]]]*/
{
	SCHED_YIELD();
	return_none;
}




/************************************************************************/
/* getpid()                                                             */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("getpid", "->?Dint", libname: "posix", ispure: true); ]]]*/
#define POSIX_GETPID_DEF          DEX_MEMBER_F("getpid", &posix_getpid, DEXSYM_READONLY, "->?Dint"),
#define POSIX_GETPID_DEF_DOC(doc) DEX_MEMBER_F("getpid", &posix_getpid, DEXSYM_READONLY, "->?Dint\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getpid_f_impl(void);
PRIVATE DEFINE_CMETHOD0(posix_getpid, &posix_getpid_f_impl, METHOD_FPURECALL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getpid_f_impl(void)
/*[[[end]]]*/
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
