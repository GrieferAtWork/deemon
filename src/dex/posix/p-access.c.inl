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
#ifndef GUARD_DEX_POSIX_P_ACCESS_C_INL
#define GUARD_DEX_POSIX_P_ACCESS_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN
/*[[[deemon
import * from deemon;
import * from _dexutils;
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
for (local x: allDecls) {
	print "\tPOSIX_",;
	print x,;
	print "_DEF \\";
}
print "/" "**" "/";

]]]*/
#include "p-access-constants.def"
#define POSIX_ACCESS_DEFS \
	POSIX_R_OK_DEF \
	POSIX_W_OK_DEF \
	POSIX_X_OK_DEF \
	POSIX_F_OK_DEF \
/**/
//[[[end]]]


/* TODO: Reqire to use the foo_USE_XXX notational system. */


#if defined(CONFIG_HAVE_waccess) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("access", "filename:c:wchar_t[],how:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_access_f_impl(dwchar_t const *filename, int how);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_ACCESS_DEF { "access", (DeeObject *)&posix_access, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define POSIX_ACCESS_DEF_DOC(doc) { "access", (DeeObject *)&posix_access, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_access, posix_access_f);
#ifndef POSIX_KWDS_FILENAME_HOW_DEFINED
#define POSIX_KWDS_FILENAME_HOW_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_how, { K(filename), K(how), KEND });
#endif /* !POSIX_KWDS_FILENAME_HOW_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	int how;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_how, "od:access", &filename, &how))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_access_f_impl(filename_str, how);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_access_f_impl(dwchar_t const *filename, int how)
//[[[end]]]
#endif /* CONFIG_HAVE_waccess */
#if !defined(CONFIG_HAVE_waccess) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("access", "filename:c:char[],how:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_access_f_impl(/*utf-8*/ char const *filename, int how);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_ACCESS_DEF { "access", (DeeObject *)&posix_access, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define POSIX_ACCESS_DEF_DOC(doc) { "access", (DeeObject *)&posix_access, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_access, posix_access_f);
#ifndef POSIX_KWDS_FILENAME_HOW_DEFINED
#define POSIX_KWDS_FILENAME_HOW_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_how, { K(filename), K(how), KEND });
#endif /* !POSIX_KWDS_FILENAME_HOW_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_access_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	int how;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_how, "od:access", &filename, &how))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_access_f_impl(filename_str, how);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_access_f_impl(/*utf-8*/ char const *filename, int how)
//[[[end]]]
#endif /* !CONFIG_HAVE_waccess */
{
#ifdef CONFIG_HAVE_access
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_waccess
#define ACCESS_PRINTF_FILENAME "%ls"
	result = waccess(filename, how);
#else /* CONFIG_HAVE_waccess */
#define ACCESS_PRINTF_FILENAME "%s"
	result = access(filename, how);
#endif /* !CONFIG_HAVE_waccess */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
#ifdef EACCES
		if (result == EACCES)
			return_false;
		if (result == EINVAL)
			return_false;
		HANDLE_ENOSYS(result, err, "access")
		HANDLE_EINVAL(result, err, "Invalid access mode %d", how)
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to check access to " ACCESS_PRINTF_FILENAME, filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory " ACCESS_PRINTF_FILENAME " could not be found", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file " ACCESS_PRINTF_FILENAME, filename)
		DeeError_SysThrowf(&DeeError_SystemError, result,
		                   "Failed to check access to " ACCESS_PRINTF_FILENAME,
		                   filename);
		goto err;
#else /* EACCES */
		return_false;
#endif /* !EACCES */
	}
	return_true;
err:
#else /* CONFIG_HAVE_access */
#define NEED_ERR_UNSUPPORTED 1
	(void)filename;
	(void)how;
	posix_err_unsupported("access");
#endif /* !CONFIG_HAVE_access */
	return NULL;
}


/*[[[deemon import("_dexutils").gw("euidaccess", "filename:c:char[],how:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_euidaccess_f_impl(/*utf-8*/ char const *filename, int how);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_euidaccess_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_EUIDACCESS_DEF { "euidaccess", (DeeObject *)&posix_euidaccess, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define POSIX_EUIDACCESS_DEF_DOC(doc) { "euidaccess", (DeeObject *)&posix_euidaccess, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_euidaccess, posix_euidaccess_f);
#ifndef POSIX_KWDS_FILENAME_HOW_DEFINED
#define POSIX_KWDS_FILENAME_HOW_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_how, { K(filename), K(how), KEND });
#endif /* !POSIX_KWDS_FILENAME_HOW_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_euidaccess_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	int how;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_how, "od:euidaccess", &filename, &how))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_euidaccess_f_impl(filename_str, how);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_euidaccess_f_impl(/*utf-8*/ char const *filename, int how)
//[[[end]]]
{
#ifdef CONFIG_HAVE_euidaccess
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = euidaccess(filename, how);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
#ifdef EACCES
		if (result == EACCES)
			return_false;
		if (result == EINVAL)
			return_false;
		HANDLE_ENOSYS(result, err, "euidaccess")
		HANDLE_EINVAL(result, err, "Invalid euidaccess mode %d", how)
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to check euidaccess to %s", filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory %s could not be found", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file %s", filename)
		DeeError_SysThrowf(&DeeError_SystemError, result,
		                   "Failed to check euidaccess to %s",
		                   filename);
		goto err;
#else /* EACCES */
		return_false;
#endif /* !EACCES */
	}
	return_true;
err:
#else /* CONFIG_HAVE_euidaccess */
#define NEED_ERR_UNSUPPORTED 1
	(void)filename;
	(void)how;
	posix_err_unsupported("euidaccess");
#endif /* !CONFIG_HAVE_euidaccess */
	return NULL;
}




/*[[[deemon import("_dexutils").gw("faccessat", "dfd:d,filename:c:char[],how:d,atflags:d->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_faccessat_f_impl(int dfd, /*utf-8*/ char const *filename, int how, int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_faccessat_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_FACCESSAT_DEF { "faccessat", (DeeObject *)&posix_faccessat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,how:?Dint,atflags:?Dint)->?Dbool") },
#define POSIX_FACCESSAT_DEF_DOC(doc) { "faccessat", (DeeObject *)&posix_faccessat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,how:?Dint,atflags:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_faccessat, posix_faccessat_f);
#ifndef POSIX_KWDS_DFD_FILENAME_HOW_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_FILENAME_HOW_ATFLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_filename_how_atflags, { K(dfd), K(filename), K(how), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_FILENAME_HOW_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_faccessat_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int dfd;
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	int how;
	int atflags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_filename_how_atflags, "dodd:faccessat", &dfd, &filename, &how, &atflags))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_faccessat_f_impl(dfd, filename_str, how, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_faccessat_f_impl(int dfd, /*utf-8*/ char const *filename, int how, int atflags)
//[[[end]]]
{
#ifdef CONFIG_HAVE_faccessat
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = faccessat(dfd, filename, how, atflags);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
#ifdef EACCES
		if (result == EACCES)
			return_false;
		if (result == EINVAL)
			return_false;
		HANDLE_ENOSYS(result, err, "faccessat")
		HANDLE_EINVAL(result, err, "Invalid access-mode (%#x) or at-flags (%#x)", how, atflags)
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to check access to %d:%s", dfd, filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory %d:%s could not be found", dfd, filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file %d:%s", dfd, filename)
		HANDLE_EBADF(result, err, "Invalid handle %d", dfd)
		DeeError_SysThrowf(&DeeError_SystemError, result, "Failed to check access to %d:%s", dfd, filename);
		goto err;
#else /* EACCES */
		return_false;
#endif /* !EACCES */
	}
	return_true;
#else /* CONFIG_HAVE_faccessat */
	DREF DeeObject *result;
	DREF DeeObject *fullname;
#define NEED_GET_DFD_FILENAME 1
	fullname = libposix_get_dfd_filename(dfd, filename, atflags);
	if unlikely(!fullname)
		goto err;
#ifdef AT_EACCESS
	if (atflags & AT_EACCESS) {
		char *ufullname;
		ufullname = DeeString_AsUtf8(fullname);
		if unlikely(!ufullname)
			result = NULL;
		else {
			result = libposix_euidaccess_f_impl(ufullname, how);
		}
	} else
#endif /* AT_EACCESS */
	{
#ifdef CONFIG_HAVE_waccess
		dwchar_t *wfullname;
		wfullname = DeeString_AsWide(fullname);
		if unlikely(!wfullname)
			result = NULL;
		else {
			result = posix_access_f_impl(wfullname, how);
		}
#else /* CONFIG_HAVE_waccess */
		char *ufullname;
		ufullname = DeeString_AsUtf8(fullname);
		if unlikely(!ufullname)
			result = NULL;
		else {
			result = posix_access_f_impl(ufullname, how);
		}
#endif /* !CONFIG_HAVE_waccess */
	}
	Dee_Decref(fullname);
#endif /* !CONFIG_HAVE_faccessat */
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_ACCESS_C_INL */
