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
#ifndef GUARD_DEX_POSIX_P_FS_C_INL
#define GUARD_DEX_POSIX_P_FS_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN

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


/*[[[deemon import("_dexutils").gw("unlink", "file:?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_unlink_f_impl(DeeObject *file);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_UNLINK_DEF { "unlink", (DeeObject *)&posix_unlink, MODSYM_FNORMAL, DOC("(file:?Dstring)") },
#define POSIX_UNLINK_DEF_DOC(doc) { "unlink", (DeeObject *)&posix_unlink, MODSYM_FNORMAL, DOC("(file:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_unlink, posix_unlink_f);
#ifndef POSIX_KWDS_FILE_DEFINED
#define POSIX_KWDS_FILE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_file, { K(file), KEND });
#endif /* !POSIX_KWDS_FILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *file;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_file, "o:unlink", &file))
		goto err;
	return posix_unlink_f_impl(file);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_unlink_f_impl(DeeObject *file)
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
				if (Dee_CollectMemory(1))
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
			if (Dee_CollectMemory(1))
				goto again_getattr;
			goto err;
		}
		dwError = ERROR_ACCESS_DENIED;
	}
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_CollectMemory(1))
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
	char *utf8_file;
	utf8_file = DeeString_AsUtf8(file);
	if unlikely(!utf8_file)
		goto err;
#elif defined(posix_unlink_USE_wunlink)
	dwchar_t *wide_file;
	wide_file = DeeString_AsWide(file);
	if unlikely(!wide_file)
		goto err;
#endif /* ... */
EINTR_LABEL(again)
#ifdef posix_unlink_USE_unlink
	if unlikely(unlink(utf8_file) != 0)
#elif defined(posix_unlink_USE_wunlink)
	if unlikely(wunlink(wide_file) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
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



/*[[[deemon import("_dexutils").gw("rmdir", "path:?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_rmdir_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rmdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RMDIR_DEF { "rmdir", (DeeObject *)&posix_rmdir, MODSYM_FNORMAL, DOC("(path:?Dstring)") },
#define POSIX_RMDIR_DEF_DOC(doc) { "rmdir", (DeeObject *)&posix_rmdir, MODSYM_FNORMAL, DOC("(path:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_rmdir, posix_rmdir_f);
#ifndef POSIX_KWDS_PATH_DEFINED
#define POSIX_KWDS_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path, { K(path), KEND });
#endif /* !POSIX_KWDS_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rmdir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path, "o:rmdir", &path))
		goto err;
	return posix_rmdir_f_impl(path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_rmdir_f_impl(DeeObject *path)
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
	dwError = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_CollectMemory(1))
			goto again;
	}
#define NEED_err_nt_rmdir
	err_nt_rmdir(dwError, path);
err:
	return NULL;
#endif /* posix_rmdir_USE_nt_RemoveDirectory */

#if defined(posix_rmdir_USE_rmdir) || defined(posix_rmdir_USE_wrmdir)
#ifdef posix_rmdir_USE_rmdir
	char *utf8_path;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#elif defined(posix_rmdir_USE_wrmdir)
	dwchar_t *wide_path;
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* ... */
EINTR_LABEL(again)
#ifdef posix_rmdir_USE_rmdir
	if unlikely(rmdir(utf8_path) != 0)
#elif defined(posix_rmdir_USE_wrmdir)
	if unlikely(wrmdir(wide_path) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
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



/*[[[deemon import("_dexutils").gw("remove", "path:?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_remove_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_remove_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_REMOVE_DEF { "remove", (DeeObject *)&posix_remove, MODSYM_FNORMAL, DOC("(path:?Dstring)") },
#define POSIX_REMOVE_DEF_DOC(doc) { "remove", (DeeObject *)&posix_remove, MODSYM_FNORMAL, DOC("(path:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_remove, posix_remove_f);
#ifndef POSIX_KWDS_PATH_DEFINED
#define POSIX_KWDS_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path, { K(path), KEND });
#endif /* !POSIX_KWDS_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_remove_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path, "o:remove", &path))
		goto err;
	return posix_remove_f_impl(path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_remove_f_impl(DeeObject *path)
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
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
#define NEED_err_nt_rmdir
		err_nt_rmdir(dwError, path);
		goto err;
	}
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
#define NEED_err_nt_unlink
	err_nt_unlink(dwError, path);
err:
	return NULL;
#endif /* posix_remove_USE_nt_DeleteFile_AND_nt_RemoveDirectory */

#if defined(posix_remove_USE_remove) || defined(posix_remove_USE_wremove)
#ifdef posix_remove_USE_remove
	char *utf8_path;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#elif defined(posix_remove_USE_wremove)
	dwchar_t *wide_path;
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* ... */
EINTR_LABEL(again)
#ifdef posix_remove_USE_remove
	if unlikely(remove(utf8_path) != 0)
#elif defined(posix_remove_USE_wremove)
	if unlikely(wremove(wide_path) != 0)
#endif /* ... */
	{
		int error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
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
	dwchar_t *wide_path;
#endif /* wchar */
#if (defined(posix_remove_USE_wunlink_AND_rmdir) || \
     defined(posix_remove_USE_unlink_AND_wrmdir) || \
     defined(posix_remove_USE_unlink_AND_rmdir))
#define posix_remove_COMBINATION_NEEDS_UTF8
	char *utf8_path;
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

#if defined(ENOMEM) || defined(EINTR)
again:
#endif /* ENOMEM || EINTR */
	DBG_ALIGNMENT_DISABLE();
#if defined(posix_remove_USE_wunlink_AND_wrmdir) || defined(posix_remove_USE_wunlink_AND_rmdir)
	error = wunlink(wide_path);
#else /* posix_remove_USE_wunlink_AND_wrmdir || posix_remove_USE_wunlink_AND_rmdir */
	error = unlink(utf8_path);
#endif /* !posix_remove_USE_wunlink_AND_wrmdir && !posix_remove_USE_wunlink_AND_rmdir */
	DBG_ALIGNMENT_ENABLE();
	if (error) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		});
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
			error = wrmdir(wide_path);
#else /* posix_remove_USE_wunlink_AND_wrmdir || posix_remove_USE_unlink_AND_wrmdir */
			error = rmdir(utf8_path);
#endif /* !posix_remove_USE_wunlink_AND_wrmdir && !posix_remove_USE_unlink_AND_wrmdir */
			DBG_ALIGNMENT_ENABLE();
			if likely(!error)
				return_none;
			error = DeeSystem_GetErrno();
			EINTR_HANDLE(error, again, err);
			DeeSystem_IF_E1(error, ENOMEM, {
				if (Dee_CollectMemory(1))
					goto again;
				goto err;
			});
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
		error = wrmdir(wide_path);
#else /* posix_remove_USE_wunlink_AND_wrmdir || posix_remove_USE_unlink_AND_wrmdir */
		error = rmdir(utf8_path);
#endif /* !posix_remove_USE_wunlink_AND_wrmdir && !posix_remove_USE_unlink_AND_wrmdir */
		DBG_ALIGNMENT_ENABLE();
		if likely(!error)
			return_none;
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		});
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


/*[[[deemon import("_dexutils").gw("unlinkat", "dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags:u=0", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_unlinkat_f_impl(DeeObject *dfd, DeeObject *file, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_UNLINKAT_DEF { "unlinkat", (DeeObject *)&posix_unlinkat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags:?Dint=!0)") },
#define POSIX_UNLINKAT_DEF_DOC(doc) { "unlinkat", (DeeObject *)&posix_unlinkat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_unlinkat, posix_unlinkat_f);
#ifndef POSIX_KWDS_DFD_FILE_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_FILE_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_file_atflags, { K(dfd), K(file), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_FILE_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *file;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_file_atflags, "oo|u:unlinkat", &dfd, &file, &atflags))
		goto err;
	return posix_unlinkat_f_impl(dfd, file, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_unlinkat_f_impl(DeeObject *dfd, DeeObject *file, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_unlinkat_USE_posix_unlink
	DREF DeeObject *absfile, *result;
#ifdef posix_unlinkat_USE_unlinkat
	if (!DeeString_Check(dfd) && DeeString_Check(file)) {
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char *utf8_file;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_file = DeeString_AsUtf8(file);
		if unlikely(!utf8_file)
			goto err;
EINTR_LABEL(again)
		if (unlinkat(os_fd, utf8_file, atflags) == 0)
			return_none;
		EINTR_HANDLE(DeeSystem_GetErrno(), again, err);
		/* fallthru to the fallback path below */
	}
#endif /* posix_unlinkat_USE_unlinkat */

#define NEED_posix_dfd_absfile
	absfile = posix_dfd_abspath(dfd, file, atflags & ~(AT_REMOVEDIR | AT_REMOVEREG));
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


/*[[[deemon import("_dexutils").gw("removeat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_removeat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_removeat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_REMOVEAT_DEF { "removeat", (DeeObject *)&posix_removeat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)") },
#define POSIX_REMOVEAT_DEF_DOC(doc) { "removeat", (DeeObject *)&posix_removeat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_removeat, posix_removeat_f);
#ifndef POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_atflags, { K(dfd), K(path), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_removeat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_atflags, "oo|u:removeat", &dfd, &path, &atflags))
		goto err;
	return posix_removeat_f_impl(dfd, path, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_removeat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_removeat_USE_posix_remove
	DREF DeeObject *abspath, *result;
#ifdef posix_removeat_USE_removeat
	if (!DeeString_Check(dfd) && DeeString_Check(path)) {
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char *utf8_path;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
EINTR_LABEL(again)
		if (removeat(os_fd, utf8_path, atflags) == 0)
			return_none;
		EINTR_HANDLE(DeeSystem_GetErrno(), again, err);
		/* fallthru to the fallback path below */
	}
#endif /* posix_removeat_USE_removeat */

#define NEED_posix_dfd_abspath
	abspath = posix_dfd_abspath(dfd, path, atflags);
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


/*[[[deemon import("_dexutils").gw("rmdirat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_rmdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rmdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RMDIRAT_DEF { "rmdirat", (DeeObject *)&posix_rmdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)") },
#define POSIX_RMDIRAT_DEF_DOC(doc) { "rmdirat", (DeeObject *)&posix_rmdirat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_rmdirat, posix_rmdirat_f);
#ifndef POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_atflags, { K(dfd), K(path), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rmdirat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_atflags, "oo|u:rmdirat", &dfd, &path, &atflags))
		goto err;
	return posix_rmdirat_f_impl(dfd, path, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_rmdirat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_rmdirat_USE_posix_rmdir
	DREF DeeObject *abspath, *result;
#ifdef posix_rmdirat_USE_rmdirat
	if (!DeeString_Check(dfd) && DeeString_Check(path)) {
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char *utf8_path;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
EINTR_LABEL(again)
		if (rmdirat(os_fd, utf8_path, atflags) == 0)
			return_none;
		EINTR_HANDLE(DeeSystem_GetErrno(), again, err);
		/* fallthru to the fallback path below */
	}
#endif /* posix_rmdirat_USE_rmdirat */

#define NEED_posix_dfd_abspath
	abspath = posix_dfd_abspath(dfd, path, atflags);
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

#endif /* !GUARD_DEX_POSIX_P_FS_C_INL */
