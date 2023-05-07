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
#ifndef GUARD_DEX_POSIX_P_SYMLINK_C_INL
#define GUARD_DEX_POSIX_P_SYMLINK_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN

/* Figure out how to implement `symlink()' */
#undef posix_symlink_USE_nt_CreateSymbolicLinkAuto
#undef posix_symlink_USE_wsymlink
#undef posix_symlink_USE_symlink
#undef posix_symlink_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_symlink_USE_nt_CreateSymbolicLinkAuto
#elif defined(CONFIG_HAVE_wsymlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_symlink_USE_wsymlink
#elif defined(CONFIG_HAVE_symlink)
#define posix_symlink_USE_symlink
#elif defined(CONFIG_HAVE_wsymlink)
#define posix_symlink_USE_wsymlink
#else /* ... */
#define posix_symlink_USE_STUB
#endif /* !... */


/* Figure out how to implement `fsymlinkat()' */
#undef posix_fsymlinkat_USE_wfsymlinkat
#undef posix_fsymlinkat_USE_wsymlinkat
#undef posix_fsymlinkat_USE_fsymlinkat
#undef posix_fsymlinkat_USE_symlinkat
#undef posix_fsymlinkat_USE_posix_symlink
#undef posix_fsymlinkat_USE_STUB
#if defined(CONFIG_HAVE_wfsymlinkat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_fsymlinkat_USE_wfsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_fsymlinkat)
#define posix_fsymlinkat_USE_fsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_wfsymlinkat)
#define posix_fsymlinkat_USE_wfsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_wsymlinkat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_fsymlinkat_USE_wsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_symlinkat)
#define posix_fsymlinkat_USE_symlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif defined(CONFIG_HAVE_wsymlinkat)
#define posix_fsymlinkat_USE_wsymlinkat
#define posix_fsymlinkat_USE_posix_symlink
#elif !defined(posix_symlink_USE_STUB)
#define posix_fsymlinkat_USE_posix_symlink
#else /* ... */
#define posix_fsymlinkat_USE_STUB
#endif /* !... */


/* Figure out how to implement `symlinkat()' */
#undef posix_symlinkat_USE_posix_fsymlinkat
#undef posix_symlinkat_USE_STUB
#if !defined(posix_fsymlinkat_USE_STUB)
#define posix_symlinkat_USE_posix_fsymlinkat
#else /* ... */
#define posix_symlinkat_USE_STUB
#endif /* !... */




/************************************************************************/
/* symlink()                                                            */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("symlink", "text:?Dstring,path:?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_symlink_f_impl(DeeObject *text, DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_symlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_SYMLINK_DEF { "symlink", (DeeObject *)&posix_symlink, MODSYM_FNORMAL, DOC("(text:?Dstring,path:?Dstring)") },
#define POSIX_SYMLINK_DEF_DOC(doc) { "symlink", (DeeObject *)&posix_symlink, MODSYM_FNORMAL, DOC("(text:?Dstring,path:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_symlink, &posix_symlink_f);
#ifndef POSIX_KWDS_TEXT_PATH_DEFINED
#define POSIX_KWDS_TEXT_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_text_path, { K(text), K(path), KEND });
#endif /* !POSIX_KWDS_TEXT_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_symlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *text;
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_text_path, "oo:symlink", &text, &path))
		goto err;
	return posix_symlink_f_impl(text, path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_symlink_f_impl(DeeObject *text, DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_symlink_USE_nt_CreateSymbolicLinkAuto
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(text, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
	error = nt_CreateSymbolicLinkAuto(path, text);
#define NEED_nt_CreateSymbolicLinkAuto
	if likely(error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}
	err_nt_symlink(dwError, text, path);
#define NEED_err_nt_symlink
err:
	return NULL;
#endif /* posix_symlink_USE_nt_CreateSymbolicLinkAuto */

#if defined(posix_symlink_USE_wsymlink) || defined(posix_symlink_USE_symlink)
	int error;
#ifdef posix_symlink_USE_wsymlink
	dwchar_t *wide_text, *wide_path;
#endif /* posix_symlink_USE_wsymlink */
#ifdef posix_symlink_USE_symlink
	char *utf8_text, *utf8_path;
#endif /* posix_symlink_USE_symlink */
	if (DeeObject_AssertTypeExact(text, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_symlink_USE_wsymlink
	wide_text = (dwchar_t *)DeeString_AsWide(text);
	if unlikely(!wide_text)
		goto err;
	wide_path = (dwchar_t *)DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_symlink_USE_wsymlink */
#ifdef posix_symlink_USE_symlink
	utf8_text = DeeString_AsUtf8(text);
	if unlikely(!utf8_text)
		goto err;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_symlink_USE_symlink */

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_symlink_USE_wsymlink
	error = wsymlink((wchar_t *)wide_text, (wchar_t *)wide_path);
#endif /* posix_symlink_USE_wsymlink */
#ifdef posix_symlink_USE_symlink
	error = symlink(utf8_text, utf8_path);
#endif /* posix_symlink_USE_symlink */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error != 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_symlink(error, text, path);
#define NEED_err_unix_symlink
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_symlink_USE_wsymlink || posix_symlink_USE_symlink */

#ifdef posix_symlink_USE_STUB
#define NEED_posix_err_unsupported
	(void)text;
	(void)path;
	posix_err_unsupported("symlink");
	return NULL;
#endif /* posix_symlink_USE_STUB */
}




/************************************************************************/
/* fsymlinkat()                                                         */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("fsymlinkat", "text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsymlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsymlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FSYMLINKAT_DEF { "fsymlinkat", (DeeObject *)&posix_fsymlinkat, MODSYM_FNORMAL, DOC("(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)") },
#define POSIX_FSYMLINKAT_DEF_DOC(doc) { "fsymlinkat", (DeeObject *)&posix_fsymlinkat, MODSYM_FNORMAL, DOC("(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fsymlinkat, &posix_fsymlinkat_f);
#ifndef POSIX_KWDS_TEXT_DFD_PATH_ATFLAGS_DEFINED
#define POSIX_KWDS_TEXT_DFD_PATH_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_text_dfd_path_atflags, { K(text), K(dfd), K(path), K(atflags), KEND });
#endif /* !POSIX_KWDS_TEXT_DFD_PATH_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fsymlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *text;
	DeeObject *dfd;
	DeeObject *path;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_text_dfd_path_atflags, "ooo|u:fsymlinkat", &text, &dfd, &path, &atflags))
		goto err;
	return posix_fsymlinkat_f_impl(text, dfd, path, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fsymlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_fsymlinkat_USE_posix_symlink
	DREF DeeObject *abspath, *result;
#if (defined(posix_fsymlinkat_USE_wfsymlinkat) || defined(posix_fsymlinkat_USE_wsymlinkat) || \
     defined(posix_fsymlinkat_USE_fsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat))
	if (!DeeString_Check(dfd) &&
#if defined(posix_fsymlinkat_USE_wsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat)
	    atflags == 0 &&
#endif /* posix_fsymlinkat_USE_wsymlinkat || posix_fsymlinkat_USE_symlinkat */
	    1) {
#if defined(posix_fsymlinkat_USE_wfsymlinkat) || defined(posix_fsymlinkat_USE_wsymlinkat)
		dwchar_t *wide_text, *wide_path;
#endif /* posix_fsymlinkat_USE_wfsymlinkat || posix_fsymlinkat_USE_wsymlinkat */
#if defined(posix_fsymlinkat_USE_fsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat)
		char *utf8_text, *utf8_path;
#endif /* posix_fsymlinkat_USE_fsymlinkat || posix_fsymlinkat_USE_symlinkat */
		int error;
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
#if defined(posix_fsymlinkat_USE_wfsymlinkat) || defined(posix_fsymlinkat_USE_wsymlinkat)
		wide_text = (dwchar_t *)DeeString_AsWide(text);
		if unlikely(!wide_text)
			goto err;
		wide_path = (dwchar_t *)DeeString_AsWide(path);
		if unlikely(!wide_path)
			goto err;
#endif /* posix_fsymlinkat_USE_wfsymlinkat || posix_fsymlinkat_USE_wsymlinkat */
#if defined(posix_fsymlinkat_USE_fsymlinkat) || defined(posix_fsymlinkat_USE_symlinkat)
		utf8_text = DeeString_AsUtf8(text);
		if unlikely(!utf8_text)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
#endif /* posix_fsymlinkat_USE_fsymlinkat || posix_fsymlinkat_USE_symlinkat */
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
#if defined(posix_fsymlinkat_USE_wfsymlinkat)
		error = wfsymlinkat(wide_text, os_dfd, wide_path, atflags);
#elif defined(posix_fsymlinkat_USE_wsymlinkat)
		error = wsymlinkat(wide_text, os_dfd, wide_path);
#elif defined(posix_fsymlinkat_USE_fsymlinkat)
		error = fsymlinkat(utf8_text, os_dfd, utf8_path, atflags);
#elif defined(posix_fsymlinkat_USE_symlinkat)
		error = symlinkat(utf8_text, os_dfd, utf8_path);
#endif /* ... */
		if likely(error == 0) {
			DBG_ALIGNMENT_ENABLE();
			return_none;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		/* fallthru to the fallback path below */
	}
#endif /* posix_symlinkat_USE_wsymlinkat || posix_symlinkat_USE_symlinkat */
	abspath = posix_dfd_abspath(dfd, path, atflags);
#define NEED_posix_dfd_abspath
	if unlikely(!abspath)
		goto err;
	result = posix_symlink_f_impl(text, abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fsymlinkat_USE_posix_symlink */

#ifdef posix_fsymlinkat_USE_STUB
#define NEED_posix_err_unsupported
	(void)text;
	(void)dfd;
	(void)path;
	(void)atflags;
	posix_err_unsupported("fsymlinkat");
	return NULL;
#endif /* posix_fsymlinkat_USE_STUB */
}


/************************************************************************/
/* symlinkat()                                                          */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("symlinkat", "text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_symlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_symlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_SYMLINKAT_DEF { "symlinkat", (DeeObject *)&posix_symlinkat, MODSYM_FNORMAL, DOC("(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)") },
#define POSIX_SYMLINKAT_DEF_DOC(doc) { "symlinkat", (DeeObject *)&posix_symlinkat, MODSYM_FNORMAL, DOC("(text:?Dstring,dfd:?X3?DFile?Dint?Dstring,path:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_symlinkat, &posix_symlinkat_f);
#ifndef POSIX_KWDS_TEXT_DFD_PATH_DEFINED
#define POSIX_KWDS_TEXT_DFD_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_text_dfd_path, { K(text), K(dfd), K(path), KEND });
#endif /* !POSIX_KWDS_TEXT_DFD_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_symlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *text;
	DeeObject *dfd;
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_text_dfd_path, "ooo:symlinkat", &text, &dfd, &path))
		goto err;
	return posix_symlinkat_f_impl(text, dfd, path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_symlinkat_f_impl(DeeObject *text, DeeObject *dfd, DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_symlinkat_USE_posix_fsymlinkat
	return posix_fsymlinkat_f_impl(text, dfd, path, 0);
#endif /* posix_symlinkat_USE_posix_fsymlinkat */

#ifdef posix_symlinkat_USE_STUB
#define NEED_posix_err_unsupported
	(void)text;
	(void)dfd;
	(void)path;
	posix_err_unsupported("symlinkat");
	return NULL;
#endif /* posix_symlinkat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_SYMLINK_C_INL */
