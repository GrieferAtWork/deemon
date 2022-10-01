/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_PWD_C_INL
#define GUARD_DEX_POSIX_P_PWD_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN



#undef posix_gettmp_USE_nt_GetTempPath
#undef posix_gettmp_USE_P_tmpdir
#ifdef CONFIG_HOST_WINDOWS
#define posix_gettmp_USE_nt_GetTempPath
#else /* CONFIG_HOST_WINDOWS */
#ifndef P_tmpdir
#define P_tmpdir "/tmp"
#endif /* !P_tmpdir */
#define posix_gettmp_USE_P_tmpdir
#endif /* !CONFIG_HOST_WINDOWS */



#undef posix_gethostname_USE_nt_GetComputerName
#undef posix_gethostname_USE_gethostname
#undef posix_gethostname_USE_read_etc_hostname
#undef posix_gethostname_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_gethostname_USE_nt_GetComputerName
#elif defined(CONFIG_HAVE_gethostname)
#define posix_gethostname_USE_gethostname
#elif !defined(DEESYSTEM_FILE_USE_STUB)
#define posix_gethostname_USE_read_etc_hostname
#else /* ... */
#define posix_gethostname_USE_STUB
#endif /* !... */







#ifndef posix_getenv_USE_STUB
PRIVATE DEFINE_STRING(posix_tmpdir_0, "TMPDIR");
PRIVATE DEFINE_STRING(posix_tmpdir_1, "TMP");
PRIVATE DEFINE_STRING(posix_tmpdir_2, "TEMP");
PRIVATE DEFINE_STRING(posix_tmpdir_3, "TEMPDIR");
PRIVATE DeeStringObject *posix_tmpdir_varnames[] = {
	(DeeStringObject *)&posix_tmpdir_0,
	(DeeStringObject *)&posix_tmpdir_1,
	(DeeStringObject *)&posix_tmpdir_2,
	(DeeStringObject *)&posix_tmpdir_3
};
#endif /* !posix_getenv_USE_STUB */

#ifdef posix_gettmp_USE_P_tmpdir
PRIVATE DEFINE_STRING(posix_tmpdir_default, P_tmpdir);
#endif /* posix_gettmp_USE_P_tmpdir */

#ifdef posix_gethostname_USE_gethostname
#ifndef CONFIG_HAVE_strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */
#endif /* posix_gethostname_USE_gethostname */




/*[[[deemon import("_dexutils").gw("getcwd", "->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getcwd_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getcwd_f(size_t argc, DeeObject *const *argv);
#define POSIX_GETCWD_DEF { "getcwd", (DeeObject *)&posix_getcwd, MODSYM_FNORMAL, DOC("->?Dstring") },
#define POSIX_GETCWD_DEF_DOC(doc) { "getcwd", (DeeObject *)&posix_getcwd, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_getcwd, posix_getcwd_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getcwd_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":getcwd"))
		goto err;
	return posix_getcwd_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getcwd_f_impl(void)
/*[[[end]]]*/
{
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (DeeSystem_PrintPwd(&printer, false))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}



/*[[[deemon import("_dexutils").gw("gettmp", "->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gettmp_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gettmp_f(size_t argc, DeeObject *const *argv);
#define POSIX_GETTMP_DEF { "gettmp", (DeeObject *)&posix_gettmp, MODSYM_FNORMAL, DOC("->?Dstring") },
#define POSIX_GETTMP_DEF_DOC(doc) { "gettmp", (DeeObject *)&posix_gettmp, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_gettmp, posix_gettmp_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gettmp_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":gettmp"))
		goto err;
	return posix_gettmp_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gettmp_f_impl(void)
/*[[[end]]]*/
{
	/* Check for environment variables */
#ifndef posix_getenv_USE_STUB
	{
		size_t i;
		for (i = 0; i < COMPILER_LENOF(posix_tmpdir_varnames); ++i) {
			DREF DeeObject *result;
			result = posix_environ_getenv(posix_tmpdir_varnames[i], Dee_None);
			if (result != Dee_None)
				return result;
			Dee_DecrefNokill(Dee_None);
		}
	}
#endif /* !posix_getenv_USE_STUB */

#ifdef posix_gettmp_USE_nt_GetTempPath
#define NEED_nt_GetTempPath
	return nt_GetTempPath();
#endif /* posix_gettmp_USE_nt_GetTempPath */

#ifdef posix_gettmp_USE_P_tmpdir
	return_reference_((DeeObject *)&posix_tmpdir_default);
#endif /* posix_gettmp_USE_P_tmpdir */
}



/*[[[deemon import("_dexutils").gw("gethostname", "->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gethostname_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gethostname_f(size_t argc, DeeObject *const *argv);
#define POSIX_GETHOSTNAME_DEF { "gethostname", (DeeObject *)&posix_gethostname, MODSYM_FNORMAL, DOC("->?Dstring") },
#define POSIX_GETHOSTNAME_DEF_DOC(doc) { "gethostname", (DeeObject *)&posix_gethostname, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_gethostname, posix_gethostname_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_gethostname_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":gethostname"))
		goto err;
	return posix_gethostname_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_gethostname_f_impl(void)
/*[[[end]]]*/
{
#ifdef posix_gethostname_USE_nt_GetComputerName
#define NEED_nt_GetComputerName
	return nt_GetComputerName();
#endif /* posix_gethostname_USE_nt_GetComputerName */

#ifdef posix_gethostname_USE_gethostname
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif /* !HOST_NAME_MAX */
#if (HOST_NAME_MAX + 0) <= 0
#undef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#elif (HOST_NAME_MAX + 0) <= 16
#undef HOST_NAME_MAX
#define HOST_NAME_MAX 16
#endif /* !HOST_NAME_MAX */
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	size_t buflen = HOST_NAME_MAX;
	char *newbuf, *buf;
	int error;
	buf = unicode_printer_alloc_utf8(&printer, buflen);
	if unlikely(!buf)
		goto err_printer;
	DBG_ALIGNMENT_DISABLE();
	while (gethostname(buf, buflen) < 0) {
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#if defined(EINVAL) && defined(ENAMETOOLONG)
		if (error != EINVAL && error != ENAMETOOLONG)
			goto err;
#elif defined(EINVAL)
		if (error != EINVAL)
			goto err;
#endif /* ... */
#if defined(EINVAL) && defined(ENAMETOOLONG)
		if (error == EINVAL)
#endif /* EINVAL && ENAMETOOLONG */
		{
			if (buflen >= 0x10000)
				goto err_generic;
		}
		buflen *= 2;
		newbuf = unicode_printer_resize_utf8(&printer, buf, buflen);
		if unlikely(!newbuf)
			goto err;
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if unlikely(unicode_printer_commit_utf8(&printer, buf, strnlen(buf, buflen)) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_generic:
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
	                          "Failed to determine host name");
err:
	unicode_printer_free_utf8(&printer, buf);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* posix_gethostname_USE_gethostname */

#ifdef posix_gethostname_USE_read_etc_hostname
	DREF DeeObject *file, *bytes, *result;
	file = DeeFile_OpenString("/etc/hostname", OPEN_FRDONLY, 0);
	if unlikely(!file)
		goto err;
	bytes = DeeFile_ReadBytes(file, (size_t)-1, true);
	Dee_Decref(file);
	if unlikely(!bytes)
		goto err;
	result = DeeString_NewUtf8((char const *)DeeBytes_DATA(bytes),
	                           DeeBytes_SIZE(bytes),
	                           STRING_ERROR_FREPLAC);
	Dee_Decref(bytes);
	return result;
err:
	return NULL;
#endif /* posix_gethostname_USE_read_etc_hostname */

#ifdef posix_gethostname_USE_STUB
#define NEED_posix_err_unsupported
	return posix_err_unsupported("gethostname");
#endif /* posix_gethostname_USE_STUB */
}

/* TODO: chdir(path:?Dstring) */
/* TODO: fchdir(fd:?X2?DFile?Dint) */
/* TODO: fchdirat(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint) */

DECL_END

#endif /* !GUARD_DEX_POSIX_P_PWD_C_INL */
