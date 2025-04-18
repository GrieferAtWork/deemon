/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_ACCESS_C_INL
#define GUARD_DEX_POSIX_P_ACCESS_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN
/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
MODULE_NAME = "posix";
local orig_stdout = File.stdout;

include("p-access-constants.def");

local allDecls = [];

function agii(name, doc = none) {
	allDecls.append(name);
	gii(name, doc: doc);
}

agii("R_OK", doc: "Test for read permission");
agii("W_OK", doc: "Test for write permission");
agii("X_OK", doc: "Test for execute permission");
agii("F_OK", doc: "Test for existence");


File.stdout = orig_stdout;
print "#define POSIX_ACCESS_DEFS \\";
for (local x: allDecls)
	print("\tPOSIX_", x, "_DEF \\");
print "/" "**" "/";

]]]*/
#include "p-access-constants.def"
#define POSIX_ACCESS_DEFS \
	POSIX_R_OK_DEF \
	POSIX_W_OK_DEF \
	POSIX_X_OK_DEF \
	POSIX_F_OK_DEF \
/**/
/*[[[end]]]*/



/* Figure out how to implement `access()' */
#undef posix_access_USE_waccess
#undef posix_access_USE_access
#undef posix_access_USE_STUB
#if defined(CONFIG_HAVE_waccess) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_access_USE_waccess
#elif defined(CONFIG_HAVE_access)
#define posix_access_USE_access
#elif defined(CONFIG_HAVE_waccess)
#define posix_access_USE_waccess
#else /* ... */
#define posix_access_USE_STUB
#endif /* !... */



/* Figure out how to implement `euidaccess()' */
#undef posix_euidaccess_USE_euidaccess
#undef posix_euidaccess_USE_STUB
#if defined(CONFIG_HAVE_euidaccess)
#define posix_euidaccess_USE_euidaccess
#else /* ... */
#define posix_euidaccess_USE_STUB
#endif /* !... */



/* Figure out how to implement `faccessat()' */
#undef posix_faccessat_USE_faccessat
#undef posix_faccessat_USE_access
#undef posix_faccessat_USE_STUB
#if defined(CONFIG_HAVE_faccessat)
#define posix_faccessat_USE_faccessat
#elif (!defined(posix_access_USE_STUB) && (!defined(AT_EACCESS) || !defined(posix_euidaccess_USE_STUB)))
#define posix_faccessat_USE_access
#else /* ... */
#define posix_faccessat_USE_STUB
#endif /* !... */






/************************************************************************/
/* access()                                                             */
/************************************************************************/

#if defined(posix_access_USE_waccess) || defined(__DEEMON__)
/*[[[deemon import("rt.gen.dexutils").gw("access", "filename:c:wchar_t[],how:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_access_f_impl(Dee_wchar_t const *filename, int how);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ACCESS_DEF { "access", (DeeObject *)&posix_access, MODSYM_FREADONLY, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define POSIX_ACCESS_DEF_DOC(doc) { "access", (DeeObject *)&posix_access, MODSYM_FREADONLY, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_access, &posix_access_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__filename_how
#define DEFINED_kwlist__filename_how
PRIVATE DEFINE_KWLIST(kwlist__filename_how, { KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("how", 0xdb5b834b, 0x1f0671eb224f8869), KEND });
#endif /* !DEFINED_kwlist__filename_how */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		Dee_wchar_t const *filename;
		int how;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__filename_how, "lsd:access", &args))
		goto err;
	return posix_access_f_impl(args.filename, args.how);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_access_f_impl(Dee_wchar_t const *filename, int how)
/*[[[end]]]*/
#endif /* posix_access_USE_waccess */
#if !defined(posix_access_USE_waccess) || defined(__DEEMON__)
/*[[[deemon import("rt.gen.dexutils").gw("access", "filename:c:char[],how:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_access_f_impl(char const *filename, int how);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ACCESS_DEF { "access", (DeeObject *)&posix_access, MODSYM_FREADONLY, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define POSIX_ACCESS_DEF_DOC(doc) { "access", (DeeObject *)&posix_access, MODSYM_FREADONLY, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_access, &posix_access_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__filename_how
#define DEFINED_kwlist__filename_how
PRIVATE DEFINE_KWLIST(kwlist__filename_how, { KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("how", 0xdb5b834b, 0x1f0671eb224f8869), KEND });
#endif /* !DEFINED_kwlist__filename_how */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		char const *filename;
		int how;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__filename_how, "Usd:access", &args))
		goto err;
	return posix_access_f_impl(args.filename, args.how);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_access_f_impl(char const *filename, int how)
/*[[[end]]]*/
#endif /* !posix_access_USE_waccess */
{
#if defined(posix_access_USE_waccess) || defined(posix_access_USE_access)
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_access_USE_waccess
#define ACCESS_PRINTF_FILENAME "%lq"
	result = waccess((wchar_t *)filename, how);
#else /* posix_access_USE_waccess */
#define ACCESS_PRINTF_FILENAME "%q"
	result = access((char *)filename, how);
#endif /* !posix_access_USE_waccess */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		EINTR_HANDLE(result, again, err)
#ifdef EACCES
		DeeSystem_IF_E2(result, EACCES, EINVAL, return_false);
		HANDLE_ENOSYS(result, err, "access")
		HANDLE_EINVAL(result, err, "Invalid access mode %d", how)
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to check access to " ACCESS_PRINTF_FILENAME, filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory " ACCESS_PRINTF_FILENAME " could not be found", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file " ACCESS_PRINTF_FILENAME, filename)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, result,
		                          "Failed to check access to " ACCESS_PRINTF_FILENAME,
		                          filename);
		goto err;
#else /* EACCES */
		return_false;
#endif /* !EACCES */
	}
	return_true;
#undef ACCESS_PRINTF_FILENAME
#if defined(EACCES) || defined(EINTR)
err:
	return NULL;
#endif /* EACCES || EINTR */
#endif /* posix_access_USE_waccess || posix_access_USE_access */

#ifdef posix_access_USE_STUB
#define NEED_posix_err_unsupported
	(void)filename;
	(void)how;
	posix_err_unsupported("access");
	return NULL;
#endif /* posix_access_USE_STUB */
}






/************************************************************************/
/* euidaccess()                                                         */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("euidaccess", "filename:c:char[],how:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_euidaccess_f_impl(char const *filename, int how);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_euidaccess_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_EUIDACCESS_DEF { "euidaccess", (DeeObject *)&posix_euidaccess, MODSYM_FREADONLY, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define POSIX_EUIDACCESS_DEF_DOC(doc) { "euidaccess", (DeeObject *)&posix_euidaccess, MODSYM_FREADONLY, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_euidaccess, &posix_euidaccess_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__filename_how
#define DEFINED_kwlist__filename_how
PRIVATE DEFINE_KWLIST(kwlist__filename_how, { KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("how", 0xdb5b834b, 0x1f0671eb224f8869), KEND });
#endif /* !DEFINED_kwlist__filename_how */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_euidaccess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		char const *filename;
		int how;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__filename_how, "Usd:euidaccess", &args))
		goto err;
	return posix_euidaccess_f_impl(args.filename, args.how);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_euidaccess_f_impl(char const *filename, int how)
/*[[[end]]]*/
{
#ifdef posix_euidaccess_USE_euidaccess
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = euidaccess((char *)filename, how);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		EINTR_HANDLE(result, again, err)
#ifdef EACCES
		DeeSystem_IF_E2(result, EACCES, EINVAL, return_false);
		HANDLE_ENOSYS(result, err, "euidaccess")
		HANDLE_EINVAL(result, err, "Invalid euidaccess mode %d", how)
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to check euidaccess to %s", filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory %s could not be found", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file %s", filename)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, result,
		                          "Failed to check euidaccess to %s",
		                          filename);
		goto err;
#else /* EACCES */
		return_false;
#endif /* !EACCES */
	}
	return_true;
#if defined(EACCES) || defined(EINTR)
err:
	return NULL;
#endif /* EACCES || EINTR */
#endif /* posix_euidaccess_USE_euidaccess */

#ifdef posix_euidaccess_USE_STUB
#define NEED_posix_err_unsupported
	(void)filename;
	(void)how;
	posix_err_unsupported("euidaccess");
	return NULL;
#endif /* posix_euidaccess_USE_STUB */
}




/*[[[deemon import("rt.gen.dexutils").gw("faccessat", "dfd:d,filename:c:char[],how:d,atflags:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL posix_faccessat_f_impl(int dfd, char const *filename, int how, int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_faccessat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FACCESSAT_DEF { "faccessat", (DeeObject *)&posix_faccessat, MODSYM_FREADONLY, DOC("(dfd:?Dint,filename:?Dstring,how:?Dint,atflags:?Dint)->?Dbool") },
#define POSIX_FACCESSAT_DEF_DOC(doc) { "faccessat", (DeeObject *)&posix_faccessat, MODSYM_FREADONLY, DOC("(dfd:?Dint,filename:?Dstring,how:?Dint,atflags:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_faccessat, &posix_faccessat_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__dfd_filename_how_atflags
#define DEFINED_kwlist__dfd_filename_how_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_filename_how_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("how", 0xdb5b834b, 0x1f0671eb224f8869), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_filename_how_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_faccessat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		int dfd;
		char const *filename;
		int how;
		int atflags;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_filename_how_atflags, "dUsdd:faccessat", &args))
		goto err;
	return posix_faccessat_f_impl(args.dfd, args.filename, args.how, args.atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((2))DREF DeeObject *DCALL posix_faccessat_f_impl(int dfd, char const *filename, int how, int atflags)
/*[[[end]]]*/
{
	/* TODO: Re-write this function */
#ifdef posix_faccessat_USE_faccessat
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = faccessat(dfd, (char *)filename, how, atflags);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		EINTR_HANDLE(result, again, err)
#ifdef EACCES
		DeeSystem_IF_E2(result, EACCES, EINVAL, return_false);
		HANDLE_ENOSYS(result, err, "faccessat")
		HANDLE_EINVAL(result, err, "Invalid access-mode (%#x) or at-flags (%#x)", how, atflags)
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to check access to %d:%s", dfd, filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory %d:%s could not be found", dfd, filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file %d:%s", dfd, filename)
		HANDLE_EBADF(result, err, "Invalid handle %d", dfd)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, result,
		                          "Failed to check access to %d:%s",
		                          dfd, filename);
		goto err;
#else /* EACCES */
		return_false;
#endif /* !EACCES */
	}
	return_true;
#if defined(EACCES) || defined(EINTR)
err:
	return NULL;
#endif /* EACCES || EINTR */
#endif /* posix_faccessat_USE_faccessat */

#ifdef posix_faccessat_USE_access
	DREF DeeObject *result;
	DREF DeeObject *fullname;
#define NEED_libposix_get_dfd_filename 1
	fullname = libposix_get_dfd_filename(dfd, filename, atflags);
	if unlikely(!fullname)
		goto err;
#ifdef AT_EACCESS
	if (atflags & AT_EACCESS) {
		char const *ufullname;
		ufullname = DeeString_AsUtf8(fullname);
		if unlikely(!ufullname) {
			result = NULL;
		} else {
			result = posix_euidaccess_f_impl(ufullname, how);
		}
	} else
#endif /* AT_EACCESS */
	{
#ifdef posix_access_USE_waccess
		Dee_wchar_t const *str_fullname = DeeString_AsWide(fullname);
#else /* posix_access_USE_waccess */
		char const *str_fullname = DeeString_AsUtf8(fullname);
#endif /* !posix_access_USE_waccess */
		if unlikely(!str_fullname) {
			result = NULL;
		} else {
			result = posix_access_f_impl(str_fullname, how);
		}
	}
	Dee_Decref(fullname);
err:
	return NULL;
#endif /* posix_faccessat_USE_access */

#ifdef posix_faccessat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)filename;
	(void)how;
	(void)atflags;
	posix_err_unsupported("faccessat");
	return NULL;
#endif /* posix_faccessat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_ACCESS_C_INL */
