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
#ifndef GUARD_DEX_POSIX_P_SCHED_C_INL
#define GUARD_DEX_POSIX_P_SCHED_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN




/************************************************************************/
/* system()                                                             */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("system", "command:c:char[]->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_system_f_impl(/*utf-8*/ char const *command);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_SYSTEM_DEF { "system", (DeeObject *)&posix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint") },
#define POSIX_SYSTEM_DEF_DOC(doc) { "system", (DeeObject *)&posix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_system, posix_system_f);
#ifndef POSIX_KWDS_COMMAND_DEFINED
#define POSIX_KWDS_COMMAND_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_command, { K(command), KEND });
#endif /* !POSIX_KWDS_COMMAND_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_system_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
{
#ifdef CONFIG_HAVE_system
	int result;
#ifdef EINTR
again:
#endif /* EINTR */
	if (DeeThread_CheckInterrupt())
		goto err;
	DBG_ALIGNMENT_DISABLE();
	result = system(command);
	DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
	if (result < 0 && DeeSystem_GetErrno() == EINTR)
		goto again;
#endif /* EINTR */
	return DeeInt_NewInt(result);
err:
	return NULL;
#else /* CONFIG_HAVE_system */
#define NEED_ERR_UNSUPPORTED 1
	(void)command;
	posix_err_unsupported("system");
	return NULL;
#endif /* !CONFIG_HAVE_system */
}




/************************************************************************/
/* sched_yield()                                                        */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("sched_yield", "", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_sched_yield_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sched_yield_f(size_t argc, DeeObject **argv);
#define POSIX_SCHED_YIELD_DEF { "sched_yield", (DeeObject *)&posix_sched_yield, MODSYM_FNORMAL, DOC("()") },
#define POSIX_SCHED_YIELD_DEF_DOC(doc) { "sched_yield", (DeeObject *)&posix_sched_yield, MODSYM_FNORMAL, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_sched_yield, posix_sched_yield_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_sched_yield_f(size_t argc, DeeObject **argv) {
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

/*[[[deemon import("_dexutils").gw("getpid", "->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getpid_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getpid_f(size_t argc, DeeObject **argv);
#define POSIX_GETPID_DEF { "getpid", (DeeObject *)&posix_getpid, MODSYM_FNORMAL, DOC("->?Dint") },
#define POSIX_GETPID_DEF_DOC(doc) { "getpid", (DeeObject *)&posix_getpid, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_getpid, posix_getpid_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getpid_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":getpid"))
	    goto err;
	return posix_getpid_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getpid_f_impl(void)
//[[[end]]]
{
#ifdef CONFIG_HAVE_getpid
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = getpid();
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		if (error != 0) {
			HANDLE_EINTR(error, again, err)
			HANDLE_ENOSYS(error, err, "getpid")
			DeeError_SysThrowf(&DeeError_SystemError, error,
			                   "Failed to get pid");
			goto err;
		}
	}
	return DeeInt_NewInt(result);
err:
	return NULL;
#else /* CONFIG_HAVE_getpid */
#define NEED_ERR_UNSUPPORTED 1
	(void)command;
	posix_err_unsupported("getpid");
	return NULL;
#endif /* !CONFIG_HAVE_getpid */
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_SCHED_C_INL */
