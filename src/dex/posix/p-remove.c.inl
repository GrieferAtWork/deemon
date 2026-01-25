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
#ifndef GUARD_DEX_POSIX_P_REMOVE_C_INL
#define GUARD_DEX_POSIX_P_REMOVE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_ReleaseSystemMemory */
#include <deemon/arg.h>             /* DeeArg_UnpackStructKw */
#include <deemon/dex.h>             /* DEXSYM_READONLY, DEX_MEMBER_F */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>
#include <deemon/objmethod.h>       /*  */
#include <deemon/string.h>          /* DeeString*, Dee_wchar_t */
#include <deemon/system-features.h> /* AT_REMOVEDIR, AT_REMOVEREG, CONFIG_HAVE_*, CONFIG_PREFER_WCHAR_FUNCTIONS, DeeSystem_GetErrno, remove, rmdir, rmdirat, unlink, unlinkat, wremove, wrmdir, wunlink */
#include <deemon/system.h>          /* DeeNTSystem_IsBadAllocError, DeeNTSystem_IsIntr, DeeUnixSystem_GetFD */
#include <deemon/thread.h>          /* DeeThread_CheckInterrupt */

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */

#include <stddef.h> /* NULL, size_t, wchar_t */

DECL_BEGIN

/* Figure out how to implement `unlink()' */
#undef posix_unlink_USE_nt_DeleteFile
#undef posix_unlink_USE_unlink
#undef posix_unlink_USE_wunlink
#undef posix_unlink_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_unlink_USE_nt_DeleteFile
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_unlink_USE_wunlink
#elif defined(CONFIG_HAVE_unlink)
#define posix_unlink_USE_unlink
#elif defined(CONFIG_HAVE_wunlink)
#define posix_unlink_USE_wunlink
#else /* ... */
#define posix_unlink_USE_STUB
#endif /* !... */


/* Figure out how to implement `rmdir()' */
#undef posix_rmdir_USE_nt_RemoveDirectory
#undef posix_rmdir_USE_rmdir
#undef posix_rmdir_USE_wrmdir
#undef posix_rmdir_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_rmdir_USE_nt_RemoveDirectory
#elif defined(CONFIG_HAVE_wrmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_rmdir_USE_wrmdir
#elif defined(CONFIG_HAVE_rmdir)
#define posix_rmdir_USE_rmdir
#elif defined(CONFIG_HAVE_wrmdir)
#define posix_rmdir_USE_wrmdir
#else /* ... */
#define posix_rmdir_USE_STUB
#endif /* !... */


/* Figure out how to implement `remove()' */
#undef posix_remove_USE_nt_DeleteFile_AND_nt_RemoveDirectory
#undef posix_remove_USE_remove
#undef posix_remove_USE_wremove
#undef posix_remove_USE_wunlink_AND_wrmdir
#undef posix_remove_USE_wunlink_AND_rmdir
#undef posix_remove_USE_unlink_AND_wrmdir
#undef posix_remove_USE_unlink_AND_rmdir
#undef posix_remove_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_remove_USE_nt_DeleteFile_AND_nt_RemoveDirectory
#elif defined(CONFIG_HAVE_wremove) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_remove_USE_wremove
#elif defined(CONFIG_HAVE_remove)
#define posix_remove_USE_remove
#elif defined(CONFIG_HAVE_wremove)
#define posix_remove_USE_wremove
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_wrmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_remove_USE_wunlink_AND_wrmdir
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_rmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_remove_USE_wunlink_AND_rmdir
#elif defined(CONFIG_HAVE_unlink) && defined(CONFIG_HAVE_wrmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_remove_USE_unlink_AND_wrmdir
#elif defined(CONFIG_HAVE_unlink) && defined(CONFIG_HAVE_rmdir)
#define posix_remove_USE_unlink_AND_rmdir
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_wrmdir)
#define posix_remove_USE_wunlink_AND_wrmdir
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_rmdir)
#define posix_remove_USE_wunlink_AND_rmdir
#elif defined(CONFIG_HAVE_unlink) && defined(CONFIG_HAVE_wrmdir)
#define posix_remove_USE_unlink_AND_wrmdir
#else /* ... */
#define posix_remove_USE_STUB
#endif /* !... */


/* Figure out how to implement `unlinkat()' */
#undef posix_unlinkat_USE_posix_unlink
#undef posix_unlinkat_USE_unlinkat
#undef posix_unlinkat_USE_STUB
#ifdef CONFIG_HAVE_unlinkat
#define posix_unlinkat_USE_posix_unlink
#define posix_unlinkat_USE_unlinkat
#elif (!defined(posix_unlink_USE_STUB) || \
       !defined(posix_rmdir_USE_STUB) ||  \
       !defined(posix_remove_USE_STUB))
#define posix_unlinkat_USE_posix_unlink
#else /* !... */
#define posix_unlinkat_USE_STUB
#endif /* ... */


/* Figure out how to implement `rmdirat()' */
#undef posix_rmdirat_USE_posix_rmdir
#undef posix_rmdirat_USE_rmdirat
#undef posix_rmdirat_USE_STUB
#ifdef CONFIG_HAVE_rmdirat
#define posix_rmdirat_USE_posix_rmdir
#define posix_rmdirat_USE_rmdirat
#elif !defined(posix_rmdir_USE_STUB)
#define posix_rmdirat_USE_posix_rmdir
#else /* !... */
#define posix_rmdirat_USE_STUB
#endif /* ... */


/* Figure out how to implement `removeat()' */
#undef posix_removeat_USE_posix_remove
#undef posix_removeat_USE_removeat
#undef posix_removeat_USE_STUB
#ifdef CONFIG_HAVE_removeat
#define posix_removeat_USE_posix_remove
#define posix_removeat_USE_removeat
#elif !defined(posix_remove_USE_STUB)
#define posix_removeat_USE_posix_remove
#else /* !... */
#define posix_removeat_USE_STUB
#endif /* ... */


/*[[[deemon import("rt.gen.dexutils").gw("unlink", "file:?Dstring", libname: "posix");]]]*/
#define POSIX_UNLINK_DEF          DEX_MEMBER_F("unlink", &posix_unlink, DEXSYM_READONLY, "(file:?Dstring)"),
#define POSIX_UNLINK_DEF_DOC(doc) DEX_MEMBER_F("unlink", &posix_unlink, DEXSYM_READONLY, "(file:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_unlink_f_impl(DeeObject *file);
#ifndef DEFINED_kwlist__file
#define DEFINED_kwlist__file
PRIVATE DEFINE_KWLIST(kwlist__file, { KEX("file", 0x1a11b2b3, 0x612e37678ce7db5b), KEND });
#endif /* !DEFINED_kwlist__file */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *file;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__file, "o:unlink", &args))
		goto err;
	return posix_unlink_f_impl(args.file);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_unlink, &posix_unlink_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_unlink_f_impl(DeeObject *file)
/*[[[end]]]*/
{
#ifdef posix_unlink_USE_nt_DeleteFile
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(file, &DeeString_Type))
		goto err;
again_deletefile:
#define NEED_nt_DeleteFile
	error = nt_DeleteFile(file);
	if likely(error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (dwError == ERROR_ACCESS_DENIED) {
		/* Check if we've failed to delete a symbolic
		 * directory-link (for which RemoveDirectory() must be used) */
		DWORD attr;
again_getattr:
#define NEED_nt_GetFileAttributes
		dwError = nt_GetFileAttributes(file, &attr);
		if (dwError < 0)
			goto err;
		if (!dwError &&
		    (attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ==
		    (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) {
again_rmdir:
#define NEED_nt_RemoveDirectory
			dwError = nt_RemoveDirectory(file);
			if likely(dwError == 0)
				return_none;
			if unlikely(dwError < 0)
				goto err;
			DBG_ALIGNMENT_DISABLE();
			dwError = (int)GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_ReleaseSystemMemory())
					goto again_rmdir;
				goto err;
			}
#define NEED_err_nt_rmdir
			err_nt_rmdir(dwError, file);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		dwError = (int)GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_ReleaseSystemMemory())
				goto again_getattr;
			goto err;
		}
		dwError = ERROR_ACCESS_DENIED;
	}
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_ReleaseSystemMemory())
			goto again_deletefile;
		goto err;
	}
#define NEED_err_nt_unlink
	err_nt_unlink(dwError, file);
err:
	return NULL;
#endif /* posix_unlink_USE_nt_DeleteFile */

#if defined(posix_unlink_USE_unlink) || defined(posix_unlink_USE_wunlink)
#ifdef posix_unlink_USE_unlink
	char const *utf8_file;
	utf8_file = DeeString_AsUtf8(file);
	if unlikely(!utf8_file)
		goto err;
#elif defined(posix_unlink_USE_wunlink)
	Dee_wchar_t const *wide_file;
	wide_file = DeeString_AsWide(file);
	if unlikely(!wide_file)
		goto err;
#endif /* ... */
EINTR_ENOMEM_LABEL(again)
#ifdef posix_unlink_USE_unlink
	if unlikely(unlink((char *)utf8_file) != 0)
#elif defined(posix_unlink_USE_wunlink)
	if unlikely(wunlink((wchar_t *)wide_file) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
#define NEED_err_unix_unlink
		err_unix_unlink(error, file);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_unlink_USE_unlink || posix_unlink_USE_wunlink */

#ifdef posix_unlink_USE_STUB
#define NEED_posix_err_unsupported
	(void)file;
	posix_err_unsupported("unlink");
	return NULL;
#endif /* posix_unlink_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("rmdir", "path:?Dstring", libname: "posix");]]]*/
#define POSIX_RMDIR_DEF          DEX_MEMBER_F("rmdir", &posix_rmdir, DEXSYM_READONLY, "(path:?Dstring)"),
#define POSIX_RMDIR_DEF_DOC(doc) DEX_MEMBER_F("rmdir", &posix_rmdir, DEXSYM_READONLY, "(path:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_rmdir_f_impl(DeeObject *path);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rmdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:rmdir", &args))
		goto err;
	return posix_rmdir_f_impl(args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_rmdir, &posix_rmdir_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_rmdir_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_rmdir_USE_nt_RemoveDirectory
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
#define NEED_nt_RemoveDirectory
	error = nt_RemoveDirectory(path);
	if likely(error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt() == 0)
			goto again;
		goto err;
	} else if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_ReleaseSystemMemory())
			goto again;
		goto err;
	}
#define NEED_err_nt_rmdir
	err_nt_rmdir(dwError, path);
err:
	return NULL;
#endif /* posix_rmdir_USE_nt_RemoveDirectory */

#if defined(posix_rmdir_USE_rmdir) || defined(posix_rmdir_USE_wrmdir)
#ifdef posix_rmdir_USE_rmdir
	char const *utf8_path;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#elif defined(posix_rmdir_USE_wrmdir)
	Dee_wchar_t const *wide_path;
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* ... */
EINTR_ENOMEM_LABEL(again)
#ifdef posix_rmdir_USE_rmdir
	if unlikely(rmdir((char *)utf8_path) != 0)
#elif defined(posix_rmdir_USE_wrmdir)
	if unlikely(wrmdir((wchar_t *)wide_path) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
#define NEED_err_unix_rmdir
		err_unix_rmdir(error, path);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_rmdir_USE_rmdir || posix_rmdir_USE_wrmdir */

#ifdef posix_rmdir_USE_STUB
#define NEED_posix_err_unsupported
	(void)path;
	posix_err_unsupported("rmdir");
	return NULL;
#endif /* posix_rmdir_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("remove", "path:?Dstring", libname: "posix");]]]*/
#define POSIX_REMOVE_DEF          DEX_MEMBER_F("remove", &posix_remove, DEXSYM_READONLY, "(path:?Dstring)"),
#define POSIX_REMOVE_DEF_DOC(doc) DEX_MEMBER_F("remove", &posix_remove, DEXSYM_READONLY, "(path:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_remove_f_impl(DeeObject *path);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_remove_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:remove", &args))
		goto err;
	return posix_remove_f_impl(args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_remove, &posix_remove_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_remove_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_remove_USE_nt_DeleteFile_AND_nt_RemoveDirectory
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
#define NEED_nt_DeleteFile
	error = nt_DeleteFile(path);
	if (error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	/* NOTE: DeleteFile() sets `ERROR_ACCESS_DENIED'
	 *       if the folder is actually a directory. */
	if (dwError == ERROR_ACCESS_DENIED) {
#define NEED_nt_RemoveDirectory
		error = nt_RemoveDirectory(path);
		if likely(error == 0)
			return_none;
		if unlikely(error < 0)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt() == 0)
				goto again;
			goto err;
		} else if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_ReleaseSystemMemory())
				goto again;
			goto err;
		}
#define NEED_err_nt_rmdir
		err_nt_rmdir(dwError, path);
		goto err;
	}
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt() == 0)
			goto again;
		goto err;
	} else if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_ReleaseSystemMemory())
			goto again;
		goto err;
	}
#define NEED_err_nt_unlink
	err_nt_unlink(dwError, path);
err:
	return NULL;
#endif /* posix_remove_USE_nt_DeleteFile_AND_nt_RemoveDirectory */

#if defined(posix_remove_USE_remove) || defined(posix_remove_USE_wremove)
#ifdef posix_remove_USE_remove
	char const *utf8_path;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#elif defined(posix_remove_USE_wremove)
	Dee_wchar_t const *wide_path;
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* ... */
EINTR_ENOMEM_LABEL(again)
#ifdef posix_remove_USE_remove
	if unlikely(remove((char *)utf8_path) != 0)
#elif defined(posix_remove_USE_wremove)
	if unlikely(wremove((wchar_t *)wide_path) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
#define NEED_err_unix_remove
		err_unix_remove(error, path);
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_remove_USE_remove || posix_remove_USE_wremove */

#if (defined(posix_remove_USE_wunlink_AND_wrmdir) || \
     defined(posix_remove_USE_wunlink_AND_rmdir) ||  \
     defined(posix_remove_USE_unlink_AND_wrmdir) ||  \
     defined(posix_remove_USE_unlink_AND_rmdir))
	int error;
#if (defined(posix_remove_USE_wunlink_AND_wrmdir) || \
     defined(posix_remove_USE_wunlink_AND_rmdir) || \
     defined(posix_remove_USE_unlink_AND_wrmdir))
#define posix_remove_COMBINATION_NEEDS_WCHAR
	Dee_wchar_t const *wide_path;
#endif /* wchar */
#if (defined(posix_remove_USE_wunlink_AND_rmdir) || \
     defined(posix_remove_USE_unlink_AND_wrmdir) || \
     defined(posix_remove_USE_unlink_AND_rmdir))
#define posix_remove_COMBINATION_NEEDS_UTF8
	char const *utf8_path;
#endif /* utf-8 */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_remove_COMBINATION_NEEDS_WCHAR
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_remove_COMBINATION_NEEDS_WCHAR */
#ifdef posix_remove_COMBINATION_NEEDS_UTF8
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_remove_COMBINATION_NEEDS_UTF8 */

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#if defined(posix_remove_USE_wunlink_AND_wrmdir) || defined(posix_remove_USE_wunlink_AND_rmdir)
	error = wunlink((wchar_t *)wide_path);
#else /* posix_remove_USE_wunlink_AND_wrmdir || posix_remove_USE_wunlink_AND_rmdir */
	error = unlink((char *)utf8_path);
#endif /* !posix_remove_USE_wunlink_AND_wrmdir && !posix_remove_USE_wunlink_AND_rmdir */
	DBG_ALIGNMENT_ENABLE();
	if (error) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
#if defined(EISDIR) || defined(EPERM)
#if defined(EISDIR) && defined(EPERM)
		if (error == EISDIR || error == EPERM)
#elif defined(EISDIR)
		if (error == EISDIR)
#else /* ... */
		if (error == EPERM)
#endif /* !... */
		{
			/* Delete a directory. */
			DBG_ALIGNMENT_DISABLE();
#if defined(posix_remove_USE_wunlink_AND_wrmdir) || defined(posix_remove_USE_unlink_AND_wrmdir)
			error = wrmdir(wchar_t *)(wide_path);
#else /* posix_remove_USE_wunlink_AND_wrmdir || posix_remove_USE_unlink_AND_wrmdir */
			error = rmdir((char *)utf8_path);
#endif /* !posix_remove_USE_wunlink_AND_wrmdir && !posix_remove_USE_unlink_AND_wrmdir */
			DBG_ALIGNMENT_ENABLE();
			if likely(!error)
				return_none;
			error = DeeSystem_GetErrno();
			EINTR_HANDLE(error, again, err);
			ENOMEM_HANDLE(error, again, err);
#define NEED_err_unix_rmdir
			err_unix_rmdir(error, path);
		} else {
			/* Handle errors. */
#define NEED_err_unix_unlink
			err_unix_unlink(error, path);
		}
#else /* EISDIR || EPERM */
		/* Delete a directory. */
		DBG_ALIGNMENT_DISABLE();
#if defined(posix_remove_USE_wunlink_AND_wrmdir) || defined(posix_remove_USE_unlink_AND_wrmdir)
		error = wrmdir((wchar_t *)wide_path);
#else /* posix_remove_USE_wunlink_AND_wrmdir || posix_remove_USE_unlink_AND_wrmdir */
		error = rmdir((char *)utf8_path);
#endif /* !posix_remove_USE_wunlink_AND_wrmdir && !posix_remove_USE_unlink_AND_wrmdir */
		if likely(error == 0) {
			DBG_ALIGNMENT_ENABLE();
			return_none;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
#define NEED_err_unix_rmdir
		err_unix_rmdir(error, path);
#endif /* !EISDIR && !EPERM */
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* [w]unlink() + [w]rmdir() */

#ifdef posix_remove_USE_STUB
#define NEED_posix_err_unsupported
	(void)path;
	posix_err_unsupported("remove");
	return NULL;
#endif /* posix_remove_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("unlinkat", "dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags:u=0", libname: "posix");]]]*/
#define POSIX_UNLINKAT_DEF          DEX_MEMBER_F("unlinkat", &posix_unlinkat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags=!0)"),
#define POSIX_UNLINKAT_DEF_DOC(doc) DEX_MEMBER_F("unlinkat", &posix_unlinkat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_unlinkat_f_impl(DeeObject *dfd, DeeObject *file, unsigned int atflags);
#ifndef DEFINED_kwlist__dfd_file_atflags
#define DEFINED_kwlist__dfd_file_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_file_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("file", 0x1a11b2b3, 0x612e37678ce7db5b), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_file_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *file;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_file_atflags, "oo|u:unlinkat", &args))
		goto err;
	return posix_unlinkat_f_impl(args.dfd, args.file, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_unlinkat, &posix_unlinkat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_unlinkat_f_impl(DeeObject *dfd, DeeObject *file, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_unlinkat_USE_posix_unlink
	DREF DeeObject *absfile, *result;
#ifdef posix_unlinkat_USE_unlinkat
	if (!DeeString_Check(dfd) && DeeString_Check(file)) {
		int error;
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char const *utf8_file;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_file = DeeString_AsUtf8(file);
		if unlikely(!utf8_file)
			goto err;
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		error = unlinkat(os_dfd, (char *)utf8_file, atflags);
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
#endif /* posix_unlinkat_USE_unlinkat */

#define NEED_posix_dfd_absfile
	absfile = posix_dfd_makepath(dfd, file, atflags & ~(AT_REMOVEDIR | AT_REMOVEREG));
	if unlikely(!absfile)
		goto err;
	if ((atflags & (AT_REMOVEDIR | AT_REMOVEREG)) == (AT_REMOVEDIR | AT_REMOVEREG)) {
		result = posix_remove_f_impl(absfile);
	} else if (atflags & AT_REMOVEDIR) {
		result = posix_rmdir_f_impl(absfile);
	} else {
		result = posix_unlink_f_impl(absfile);
	}
	Dee_Decref(absfile);
	return result;
err:
	return NULL;
#endif /* posix_unlinkat_USE_posix_chdir */

#ifdef posix_unlinkat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)file;
	(void)atflags;
	posix_err_unsupported("unlinkat");
	return NULL;
#endif /* posix_unlinkat_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("removeat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix");]]]*/
#define POSIX_REMOVEAT_DEF          DEX_MEMBER_F("removeat", &posix_removeat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)"),
#define POSIX_REMOVEAT_DEF_DOC(doc) DEX_MEMBER_F("removeat", &posix_removeat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_removeat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
#ifndef DEFINED_kwlist__dfd_path_atflags
#define DEFINED_kwlist__dfd_path_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_path_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_path_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_removeat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *path;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_path_atflags, "oo|u:removeat", &args))
		goto err;
	return posix_removeat_f_impl(args.dfd, args.path, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_removeat, &posix_removeat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_removeat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_removeat_USE_posix_remove
	DREF DeeObject *abspath, *result;
#ifdef posix_removeat_USE_removeat
	if (!DeeString_Check(dfd) && DeeString_Check(path)) {
		int error;
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char const *utf8_path;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		error = removeat(os_dfd, (char *)utf8_path, atflags);
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
#endif /* posix_removeat_USE_removeat */

#define NEED_posix_dfd_makepath
	abspath = posix_dfd_makepath(dfd, path, atflags);
	if unlikely(!abspath)
		goto err;
	result = posix_remove_f_impl(abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_removeat_USE_posix_chdir */

#ifdef posix_removeat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)file;
	(void)atflags;
	posix_err_unsupported("removeat");
	return NULL;
#endif /* posix_removeat_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("rmdirat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix");]]]*/
#define POSIX_RMDIRAT_DEF          DEX_MEMBER_F("rmdirat", &posix_rmdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)"),
#define POSIX_RMDIRAT_DEF_DOC(doc) DEX_MEMBER_F("rmdirat", &posix_rmdirat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_rmdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
#ifndef DEFINED_kwlist__dfd_path_atflags
#define DEFINED_kwlist__dfd_path_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_path_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_path_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rmdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *path;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_path_atflags, "oo|u:rmdirat", &args))
		goto err;
	return posix_rmdirat_f_impl(args.dfd, args.path, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_rmdirat, &posix_rmdirat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_rmdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_rmdirat_USE_posix_rmdir
	DREF DeeObject *abspath, *result;
#ifdef posix_rmdirat_USE_rmdirat
	if (!DeeString_Check(dfd) && DeeString_Check(path)) {
		int error;
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char const *utf8_path;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		error = rmdirat(os_dfd, (char *)utf8_path, atflags);
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
#endif /* posix_rmdirat_USE_rmdirat */

#define NEED_posix_dfd_makepath
	abspath = posix_dfd_makepath(dfd, path, atflags);
	if unlikely(!abspath)
		goto err;
	result = posix_rmdir_f_impl(abspath);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_rmdirat_USE_posix_chdir */

#ifdef posix_rmdirat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)file;
	(void)atflags;
	posix_err_unsupported("rmdirat");
	return NULL;
#endif /* posix_rmdirat_USE_STUB */
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_REMOVE_C_INL */
