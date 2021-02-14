/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FS_OSFS_C
#define GUARD_DEX_FS_OSFS_C 1
#define DEE_SOURCE 1

#include "libfs.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system-error.h>
#include <deemon/system.h>
#include <deemon/tuple.h>

#include "_res.h"

#include <deemon/util/rwlock.h>
#include <hybrid/atomic.h>
#include <hybrid/unaligned.h>

#include "../time/libtime.h"

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */


#ifdef CONFIG_HOST_WINDOWS
#ifndef FILETIME_GET64
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FILETIME_GET64(x) (((x) << 32) | ((x) >> 32))
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#define FILETIME_GET64(x)   (x)
#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */
#endif /* !FILETIME_GET64 */

#ifndef FILETIME_PER_SECONDS
#define FILETIME_PER_SECONDS 10000000 /* 100 nanoseconds / 0.1 microseconds. */
#endif /* !FILETIME_PER_SECONDS */
#endif /* CONFIG_HOST_WINDOWS */


#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else
#   define PATH_MAX 260
#endif
#endif /* !PATH_MAX */


#ifdef CONFIG_HAVE_stat64
#define STRUCT_STAT struct stat64
#define STAT        stat64
#ifdef CONFIG_HAVE_lstat64
#define LSTAT       lstat64
#endif /* CONFIG_HAVE_lstat64 */
#ifdef CONFIG_HAVE_fstat64
#define FSTAT       fstat64
#endif /* CONFIG_HAVE_fstat64 */
#ifdef CONFIG_HAVE_wstat64
#define WSTAT       wstat64
#endif /* CONFIG_HAVE_wstat64 */
#ifdef CONFIG_HAVE_wlstat64
#define WLSTAT      wlstat64
#endif /* CONFIG_HAVE_wlstat64 */
#else /* __USE_LARGEFILE64 */
#define STRUCT_STAT  struct stat
/* Work-around for buggy MS headers. (NO! IT'S NOT A FEATURE
 * TO BLATANTLY DISREGARD POSIX AND CALL IT NON-STDC!) */
#ifdef _MSC_VER
#ifdef _USE_32BIT_TIME_T
#define STRUCT_WSTAT struct _stat32
#else /* _USE_32BIT_TIME_T */
#define STRUCT_WSTAT struct _stat64i32
#endif /* !_USE_32BIT_TIME_T */
#else /* _MSC_VER */
#define STRUCT_WSTAT struct stat
#endif /* !_MSC_VER */
#ifdef CONFIG_HAVE_stat
#define STAT        stat
#endif /* CONFIG_HAVE_stat */
#ifdef CONFIG_HAVE_lstat
#define LSTAT       lstat
#endif /* CONFIG_HAVE_lstat */
#ifdef CONFIG_HAVE_fstat
#define FSTAT       fstat
#endif /* CONFIG_HAVE_fstat */
#ifdef CONFIG_HAVE_wstat
#define WSTAT       wstat
#endif /* CONFIG_HAVE_wstat */
#ifdef CONFIG_HAVE_wlstat
#define WLSTAT      wlstat
#endif /* CONFIG_HAVE_wlstat */
#endif /* !__USE_LARGEFILE64 */

#ifndef STRUCT_WSTAT
#define STRUCT_WSTAT STRUCT_STAT
#endif /* !STRUCT_WSTAT */

#if !defined(WLSTAT) && defined(WSTAT)
#define WLSTAT WSTAT
#endif /* !WLSTAT && WSTAT */

#if !defined(LSTAT) && defined(STAT)
#define LSTAT STAT
#endif /* !LSTAT && STAT */


DECL_BEGIN

#undef WANT_ERR_UNSUPPORTED
INTDEF ATTR_COLD NONNULL((1)) int DCALL errUnsupported(char const *__restrict name);

#ifdef CONFIG_HAVE_errno
#undef WANT_UNIX_ERRPATHEXISTS
#undef WANT_UNIX_ERRPATHNOTDIR
#undef WANT_UNIX_ERRPATHISDIR
#undef WANT_UNIX_ERRPATHREADONLY
#undef WANT_UNIX_ERRPATHNOTFOUND
#undef WANT_UNIX_ERRFILENOTFOUND
#undef WANT_UNIX_ERRFILENOTWRITABLE
#undef WANT_UNIX_ERRPATHNOTWRITABLE
#undef WANT_UNIX_ERRPATHBUSY
#undef WANT_UNIX_ERRPATHNOACCESS
#undef WANT_UNIX_ERRPATHNOTEMPTY
#undef WANT_UNIX_ERRCHTIMENOACCESS
#undef WANT_UNIX_ERRCHATTRNOACCESS
#undef WANT_UNIX_ERRHANDLECLOSED
#undef WANT_UNIX_ERRPATHREADONLY2
#undef WANT_UNIX_ERRPATHNOTFOUND2
#undef WANT_UNIX_ERRPATHCROSSDEV2
#undef WANT_UNIX_ERRRMDIR
#undef WANT_UNIX_ERRUNLINK
#undef WANT_UNIX_ERRREMOVE
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathExists(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathNotDir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathIsDir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathReadonly(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathNotFound(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrFileNotFound(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrFileNotWritable(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathNotWritable(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathBusy(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathNoAccess(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrPathNotEmpty(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrChtimeNoAccess(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrChattrNoAccess(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrHandleClosed(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL unix_ErrPathReadonly2(int errno_value, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL unix_ErrPathNotFound2(int errno_value, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL unix_ErrPathCrossDev2(int errno_value, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrRmDir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrUnlink(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL unix_ErrRemove(int errno_value, DeeObject *__restrict path);
#endif /* CONFIG_HAVE_errno */


#ifdef CONFIG_HOST_WINDOWS
#undef WANT_NT_ERRPATHEXISTS
#undef WANT_NT_ERRPATHNOTDIR
#undef WANT_NT_ERRPATHISDIR
#undef WANT_NT_ERRPATHREADONLY
#undef WANT_NT_ERRPATHNOTFOUND
#undef WANT_NT_ERRFILENOTFOUND
#undef WANT_NT_ERRFILENOWRITEACCESS
#undef WANT_NT_ERRPATHNOTWRITABLE
#undef WANT_NT_ERRPATHBUSY
#undef WANT_NT_ERRPATHNOACCESS
#undef WANT_NT_ERRPATHNOTEMPTY
#undef WANT_NT_ERRCHTIMENOACCESS
#undef WANT_NT_ERRCHATTRNOACCESS
#undef WANT_NT_ERRHANDLECLOSED
#undef WANT_NT_ERRPATHNOACCESS2
#undef WANT_NT_ERRPATHNOTFOUND2
#undef WANT_NT_ERRPATHCROSSDEV2
#undef WANT_NT_ERRRMDIR
#undef WANT_NT_ERRUNLINK
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathExists(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathNotDir(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathIsDir(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathReadonly(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathNotFound(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrFileNotFound(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrFileNotWritable(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathNotWritable(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathBusy(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathNoAccess(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrPathNotEmpty(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrChtimeNoAccess(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrChattrNoAccess(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrHandleClosed(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL nt_ErrPathNoAccess2(DWORD dwError, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL nt_ErrPathNotFound2(DWORD dwError, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL nt_ErrPathCrossDev2(DWORD dwError, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrRmDir(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL nt_ErrUnlink(DWORD dwError, DeeObject *__restrict path);

/* @return:  0: Successfully written the handle to `phandle'
 * @return:  1: Successfully written the handle to `phandle',
 *              but the caller must CloseHandle() it when they are done
 * @return: -1: An error occurred. */
#undef WANT_DEEOBJECT_ASPATHHANDLEWITHWRITEATTRIBUTES
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_AsPathHandleWithWriteAttributes(DeeObject *__restrict path,
                                          HANDLE *__restrict phandle);

/* Convert a given object into a windows FILETIME timestamp */
#undef WANT_DEEOBJECT_ASFILETIME
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_AsFileTime(DeeObject *__restrict lpTime,
                     FILETIME *__restrict lpFileTime);

/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_SETCURRENTDIRECTORY
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_SetCurrentDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_GETFILEATTRIBUTESEX
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_GETFILEATTRIBUTES
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict presult);

/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_SETFILEATTRIBUTES
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_CREATEDIRECTORY
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_REMOVEDIRECTORY
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_DELETEFILE
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_MOVEFILE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_MoveFile(DeeObject *__restrict lpExistingFileName,
            DeeObject *__restrict lpNewFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the hardlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_CREATEHARDLINK
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_CreateHardLink(DeeObject *__restrict lpFileName,
                  DeeObject *__restrict lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
#undef WANT_NT_CREATESYMBOLICLINK
INTDEF int DCALL
nt_CreateSymbolicLink(DeeObject *__restrict lpSymlinkFileName,
                      DeeObject *__restrict lpTargetFileName,
                      DWORD dwFlags);

#endif /* CONFIG_HOST_WINDOWS */



/************************************************************************/
/* fs_chtime()                                                          */
/************************************************************************/
#undef fs_chtime_USE_SETFILETIME
#undef fs_chtime_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_chtime_USE_SETFILETIME 1
#elif 0 /* TODO: utime() and friends. */
#else
#define fs_chtime_USE_STUB 1
#endif


/* Change timestamps for a given file. */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
fs_chtime(DeeObject *__restrict path, DeeObject *__restrict atime,
          DeeObject *__restrict mtime, DeeObject *__restrict ctime) {
#ifdef fs_chtime_USE_SETFILETIME
	int result;
	HANDLE hnd;
	BOOL error;
	FILETIME ftAtime, ftMtime, ftCtime;
	if (DeeThread_CheckInterrupt())
		goto err;
#define WANT_DEEOBJECT_ASPATHHANDLEWITHWRITEATTRIBUTES 1
again:
	result = DeeObject_AsPathHandleWithWriteAttributes(path, &hnd);
	if unlikely(result < 0)
		goto err;
#define WANT_DEEOBJECT_ASFILETIME 1
	if (!DeeNone_Check(atime) && unlikely(DeeObject_AsFileTime(atime, &ftAtime)))
		goto err;
	if (!DeeNone_Check(mtime) && unlikely(DeeObject_AsFileTime(mtime, &ftMtime)))
		goto err;
	if (!DeeNone_Check(ctime) && unlikely(DeeObject_AsFileTime(ctime, &ftCtime)))
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = SetFileTime(hnd,
	                    DeeNone_Check(ctime) ? NULL : &ftCtime,
	                    DeeNone_Check(atime) ? NULL : &ftAtime,
	                    DeeNone_Check(mtime) ? NULL : &ftMtime);
	if (result)
		CloseHandle(hnd);
	if unlikely(!error) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
		} else if (DeeNTSystem_IsBadF(dwError)) {
#define WANT_NT_ERRHANDLECLOSED 1
			nt_ErrHandleClosed(dwError, path);
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define WANT_NT_ERRCHTIMENOACCESS 1
			nt_ErrChtimeNoAccess(dwError, path);
		} else if (DeeNTSystem_IsUnsupportedError(dwError)) {
			DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, (DWORD)error,
			                        "The filesystem hosting the path %r does "
			                        "not support the changing of time stamps",
			                        path);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
			                        "Failed to change time properties of %r",
			                        path);
		}
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
#endif /* fs_chtime_USE_SETFILETIME */

#ifdef fs_chtime_USE_STUB
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("chtime");
#endif /* fs_chtime_USE_STUB */
}



/************************************************************************/
/* fs_chmod()                                                           */
/************************************************************************/
#undef fs_chmod_USE_SETFILEATTRIBUTES
#undef fs_chmod_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_chmod_USE_SETFILEATTRIBUTES 1
#elif 0 /* TODO: chmod() */
#else
#define fs_chmod_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_chmod(DeeObject *__restrict path,
         DeeObject *__restrict mode) {
#ifdef fs_chmod_USE_SETFILEATTRIBUTES
	uint16_t mask, flags;
	DWORD old_flags, new_flags;
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (!DeeString_Check(path)) {
		if (DeeInt_Check(path)) {
			HANDLE fd; /* Support for descriptor-based chmod() */
			if (DeeObject_AsUIntptr(path, (uintptr_t *)&fd))
				goto err;
			path = DeeNTSystem_GetFilenameOfHandle(fd);
		} else {
			path = DeeFile_Filename(path);
		}
		if unlikely(!path)
			goto err;
		error = fs_chmod(path, mode);
		Dee_Decref(path);
		return error;
	}
	if (fs_getchmod_mask(mode, &mask, &flags))
		goto err;
again:
#define WANT_NT_GETFILEATTRIBUTES 1
	error = nt_GetFileAttributes(path, &old_flags);
	if unlikely(error < 0)
		goto err;
	if unlikely(error)
		goto err_nt;
	new_flags = old_flags & ~FILE_ATTRIBUTE_READONLY;
	if (mask & 0222) /* Inherit old writability mode. */
		new_flags |= old_flags & FILE_ATTRIBUTE_READONLY;
	if (flags & 0222) {
		new_flags &= ~FILE_ATTRIBUTE_READONLY; /* Make writable. */
	} else {
		new_flags |= FILE_ATTRIBUTE_READONLY; /* Make readonly. */
	}
	if (new_flags != old_flags) {
		/* Set new flags. */
#define WANT_NT_SETFILEATTRIBUTES 1
		error = nt_SetFileAttributes(path, new_flags);
		if unlikely(error < 0)
			goto err;
		if unlikely(error)
			goto err_nt;
	}
	return 0;
err_nt:
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again;
	} else if (DeeNTSystem_IsAccessDeniedError((DWORD)error)) {
#define WANT_NT_ERRCHATTRNOACCESS 1
		nt_ErrChattrNoAccess((DWORD)error, path);
	} else if (DeeNTSystem_IsUnsupportedError((DWORD)error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, (DWORD)error,
		                        "The filesystem hosting the path %r does "
		                        "not support the changing of NT attributes",
		                        path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, (DWORD)error,
		                        "Failed to change attributes of %r",
		                        path);
	}
err:
	return -1;
#endif /* fs_chmod_USE_SETFILEATTRIBUTES */

#ifdef fs_chmod_USE_STUB
	(void)path;
	(void)mode;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("chmod");
#endif /* fs_chmod_USE_STUB */
}



/************************************************************************/
/* fs_lchmod()                                                          */
/************************************************************************/
#undef fs_lchmod_USE_CHMOD
#undef fs_lchmod_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_lchmod_USE_CHMOD 1
#elif 0 /* TODO: lchmod() */
#else
#define fs_lchmod_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_lchmod(DeeObject *__restrict path,
          DeeObject *__restrict mode) {
#ifdef fs_lchmod_USE_CHMOD
	return fs_chmod(path, mode);
#endif /* fs_lchmod_USE_CHMOD */

#ifdef fs_lchmod_USE_STUB
	(void)path;
	(void)mode;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("lchmod");
#endif /* fs_lchmod_USE_STUB */
}





/************************************************************************/
/* fs_chown()                                                           */
/************************************************************************/
#undef fs_chown_USE_STUB
#if 0 /* TODO: Windows implementation */
#elif 0 /* TODO: chown() */
#else
#define fs_chown_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fs_chown(DeeObject *__restrict path,
         DeeObject *__restrict user,
         DeeObject *__restrict group) {
#ifdef fs_chown_USE_STUB
	(void)path;
	(void)user;
	(void)group;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("chown");
#endif /* fs_chown_USE_STUB */
}



/************************************************************************/
/* fs_lchown()                                                          */
/************************************************************************/
#undef fs_lchown_USE_CHMOD
#undef fs_lchown_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_lchown_USE_CHMOD 1
#elif 0 /* lchown() */
#else
#define fs_lchown_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fs_lchown(DeeObject *__restrict path,
          DeeObject *__restrict user,
          DeeObject *__restrict group) {
#ifdef fs_lchown_USE_CHMOD
	return fs_chown(path, user, group);
#endif /* fs_lchown_USE_CHMOD */

#ifdef fs_lchown_USE_STUB
	(void)path;
	(void)user;
	(void)group;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("lchown");
#endif /* fs_lchown_USE_STUB */
}



/************************************************************************/
/* fs_mkdir()                                                           */
/************************************************************************/
#undef fs_mkdir_USE_CREATEDIRECTORY
#undef fs_mkdir_USE_WMKDIR
#undef fs_mkdir_USE_MKDIR
#undef fs_mkdir_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_mkdir_USE_CREATEDIRECTORY 1
#elif defined(CONFIG_HAVE_wmkdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_mkdir_USE_WMKDIR 1
#elif defined(CONFIG_HAVE_mkdir)
#define fs_mkdir_USE_MKDIR 1
#elif defined(CONFIG_HAVE_wmkdir)
#define fs_mkdir_USE_WMKDIR 1
#else
#define fs_mkdir_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_mkdir(DeeObject *__restrict path,
         DeeObject *__restrict perm) {
#ifdef fs_mkdir_USE_CREATEDIRECTORY
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
	if (!DeeNone_Check(perm)) {
		/* TODO: Initial security attributes. */
#define WANT_NT_CREATEDIRECTORY 1
		error = nt_CreateDirectory(path, NULL);
	} else {
		error = nt_CreateDirectory(path, NULL);
	}
	if unlikely(error > 0)
		goto err_nt;
	return error;
err_nt:
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again;
	} else if (DeeNTSystem_IsAccessDeniedError((DWORD)error)) {
#define WANT_NT_ERRPATHNOTWRITABLE 1
		nt_ErrPathNotWritable((DWORD)error, path);
	} else if (DeeNTSystem_IsExists((DWORD)error)) {
#define WANT_NT_ERRPATHEXISTS 1
		nt_ErrPathExists((DWORD)error, path);
	} else if (DeeNTSystem_IsNotDir((DWORD)error)) {
#define WANT_NT_ERRPATHNOTDIR 1
		nt_ErrPathNotDir((DWORD)error, path);
	} else if (DeeNTSystem_IsUnsupportedError((DWORD)error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, (DWORD)error,
		                        "The filesystem hosting the path %r does "
		                        "not support the creation of directories",
		                        path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, (DWORD)error,
		                        "Failed to create directory %r",
		                        path);
	}
err:
	return -1;
#endif /* fs_mkdir_USE_CREATEDIRECTORY */

#if defined(fs_mkdir_USE_WMKDIR) || defined(fs_mkdir_USE_MKDIR)
	int error;
	int creat_mode = 755;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	if (!DeeNone_Check(perm) &&
	    DeeObject_AsInt(perm, &creat_mode))
		goto err;
#ifdef fs_mkdir_USE_WMKDIR
	{
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = wmkdir(wpath, creat_mode);
		DBG_ALIGNMENT_ENABLE();
	}
#else /* fs_mkdir_USE_WMKDIR */
	{
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = mkdir(utf8, creat_mode);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* !fs_mkdir_USE_WMKDIR */
	if unlikely(error) {
		error = DeeSystem_GetErrno();
#ifdef EACCES
		if (error == EACCES) {
#define WANT_UNIX_ERRPATHNOTWRITABLE 1
			unix_ErrPathNotWritable(error, path);
		} else
#endif /* EACCES */
#ifdef EEXIST
		if (error == EEXIST) {
#define WANT_UNIX_ERRPATHEXISTS 1
			unix_ErrPathExists(error, path);
		} else
#endif /* EEXIST */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
#define WANT_UNIX_ERRPATHNOTDIR 1
			unix_ErrPathNotDir(error, path);
		} else
#endif /* ENOTDIR */
#ifdef EROFS
		if (error == EROFS) {
#define WANT_UNIX_ERRPATHREADONLY 1
			unix_ErrPathReadonly(error, path);
		} else
#endif /* EROFS */
#ifdef EPERM
		if (error == EPERM) {
			DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, error,
			                          "The filesystem hosting the path %r does "
			                          "not support the creation of directories",
			                          path);
		} else
#endif /* EPERM */
		{
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to create directory %r",
			                          path);
		}
		goto err;
	}
	return 0;
err:
	return -1;
#endif /* fs_mkdir_USE_WMKDIR || fs_mkdir_USE_MKDIR */

#ifdef fs_mkdir_USE_STUB
	(void)path;
	(void)perm;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("mkdir");
#endif /* fs_mkdir_USE_STUB */
}




/************************************************************************/
/* fs_rmdir()                                                           */
/************************************************************************/
#undef fs_rmdir_USE_REMOVEDIRECTORY
#undef fs_rmdir_USE_WRMDIR
#undef fs_rmdir_USE_RMDIR
#undef fs_rmdir_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_rmdir_USE_REMOVEDIRECTORY 1
#elif defined(CONFIG_HAVE_wrmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_rmdir_USE_WRMDIR 1
#elif defined(CONFIG_HAVE_rmdir)
#define fs_rmdir_USE_RMDIR 1
#elif defined(CONFIG_HAVE_wrmdir)
#define fs_rmdir_USE_WRMDIR 1
#else
#define fs_rmdir_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1)) int DCALL
fs_rmdir(DeeObject *__restrict path) {
#ifdef fs_rmdir_USE_REMOVEDIRECTORY
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
#define WANT_NT_REMOVEDIRECTORY 1
	error = nt_RemoveDirectory(path);
	if unlikely(error > 0)
		goto err_nt;
	return error;
err_nt:
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again;
	}
#define WANT_NT_ERRRMDIR 1
	nt_ErrRmDir((DWORD)error, path);
err:
	return -1;
#endif /* fs_rmdir_USE_REMOVEDIRECTORY */

#if defined(fs_rmdir_USE_WRMDIR) || defined(fs_rmdir_USE_RMDIR)
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef fs_rmdir_USE_WRMDIR
	{
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = wrmdir(wpath);
		DBG_ALIGNMENT_ENABLE();
	}
#else /* fs_rmdir_USE_WRMDIR */
	{
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = rmdir(utf8);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* !fs_rmdir_USE_WRMDIR */
	if unlikely(error) {
		error = DeeSystem_GetErrno();
#define WANT_UNIX_ERRRMDIR 1
		unix_ErrRmDir(error, path);
		goto err;
	}
	return 0;
err:
	return -1;
#endif /* fs_rmdir_USE_WRMDIR || fs_rmdir_USE_RMDIR */

#ifdef fs_rmdir_USE_STUB
	(void)path;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("rmdir");
#endif /* fs_rmdir_USE_STUB */
}




/************************************************************************/
/* fs_chdir()                                                           */
/************************************************************************/
#undef fs_chdir_USE_SETCURRENTDIRECTORY
#undef fs_chdir_USE_WCHDIR
#undef fs_chdir_USE_CHDIR
#undef fs_chdir_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_chdir_USE_SETCURRENTDIRECTORY 1
#elif defined(CONFIG_HAVE_wchdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_chdir_USE_WCHDIR 1
#elif defined(CONFIG_HAVE_chdir)
#define fs_chdir_USE_CHDIR 1
#elif defined(CONFIG_HAVE_wchdir)
#define fs_chdir_USE_WCHDIR 1
#else
#define fs_chdir_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1)) int DCALL
fs_chdir(DeeObject *__restrict path) {
#ifdef fs_chdir_USE_SETCURRENTDIRECTORY
	int result;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (!DeeString_Check(path)) {
		if (DeeInt_Check(path)) {
			HANDLE fd; /* Support for descriptor-based chdir() */
			if (DeeObject_AsUIntptr(path, (uintptr_t *)&fd))
				goto err;
			path = DeeNTSystem_GetFilenameOfHandle(fd);
			if unlikely(!path)
				goto err;
			result = fs_chdir(path);
			Dee_Decref(path);
			return result;
		}
		path = DeeFile_Filename(path);
		if unlikely(!path)
			goto err;
		result = fs_chdir(path);
		Dee_Decref(path);
		return result;
	}
again:
	/* Allow an empty path as an alias for `chdir(".")' (aka. no-op)
	 * Windows would normally set an `ERROR_INVALID_NAME' error for this case. */
	if (DeeString_WLEN(path) == 0)
		goto done;
#define WANT_NT_SETCURRENTDIRECTORY 1
	result = nt_SetCurrentDirectory(path);
	if unlikely(result != 0) {
		DWORD dwError;
		if unlikely(result < 0)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
		if (DeeNTSystem_IsNotDir(dwError))
			goto do_throw_not_dir;
		if (dwError == ERROR_ACCESS_DENIED) {
			DWORD dwAttributes;
			/* Check if the path is actually a directory. */
#define WANT_NT_GETFILEATTRIBUTES 1
			result = nt_GetFileAttributes(path, &dwAttributes);
			if (result < 0)
				goto err;
			if (result == 0 && !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
do_throw_not_dir:
#define WANT_NT_ERRPATHNOTDIR 1
				nt_ErrPathNotDir(dwError, path);
				goto err;
			}
		}
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
#define WANT_NT_ERRPATHNOTFOUND 1
			nt_ErrPathNotFound(dwError, path);
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define WANT_NT_ERRPATHNOACCESS 1
			nt_ErrPathNoAccess(dwError, path);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
			                        "Failed to change the current working directory to %r",
			                        path);
		}
		goto err;
	}
done:
	return 0;
err:
	return -1;
#endif /* fs_chdir_USE_SETCURRENTDIRECTORY */

#if defined(fs_chdir_USE_WCHDIR) || defined(fs_chdir_USE_CHDIR)
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (!DeeString_Check(path)) {
#ifdef CONFIG_HAVE_fchdir
		int fd;
		if (DeeInt_Check(path)) {
			if (DeeObject_AsInt(path, &fd))
				goto err;
		} else {
			fd = DeeUnixSystem_GetFD(path);
			if unlikely(fd == -1) {
				if (DeeError_Catch(&DeeError_AttributeError) ||
				    DeeError_Catch(&DeeError_NotImplemented))
					goto try_filename;
				goto err;
			}
		}
		DBG_ALIGNMENT_DISABLE();
		error = fchdir(fd);
		DBG_ALIGNMENT_ENABLE();
		goto check_error;
try_filename:
#endif /* CONFIG_HAVE_fchdir */
		/* Use the filename of a file stream. */
		path = DeeFile_Filename(path);
		if unlikely(!path)
			goto err;
		error = fs_chdir(path);
		Dee_Decref(path);
		return error;
	}
	/* Allow an empty path as an alias for `chdir(".")' (aka. no-op) */
	if (DeeString_WLEN(path) == 0)
		goto done;
#ifdef fs_chdir_USE_WCHDIR
	{
		wchar_t *wstr;
		wstr = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wstr)
			goto err;
		if (!*wstr && !Dee_WSTR_LENGTH(wstr))
			goto done;
		DBG_ALIGNMENT_DISABLE();
		error = wchdir(wstr);
		DBG_ALIGNMENT_ENABLE();
	}
#else /* fs_chdir_USE_WCHDIR */
	{
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		if (!*utf8 && !Dee_WSTR_LENGTH(utf8))
			goto done;
		DBG_ALIGNMENT_DISABLE();
		error = chdir(utf8);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* !fs_chdir_USE_WCHDIR */
#ifdef CONFIG_HAVE_fchdir
check_error:
#endif /* CONFIG_HAVE_fchdir */
	if unlikely(error) {
		error = DeeSystem_GetErrno();
#ifdef EACCES
		if (error == EACCES) {
#define WANT_UNIX_ERRPATHNOACCESS 1
			unix_ErrPathNoAccess(error, path);
		} else
#endif /* EACCES */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
#define WANT_UNIX_ERRPATHNOTDIR 1
			unix_ErrPathNotDir(error, path);
		} else
#endif /* ENOTDIR */
#ifdef ENOENT
		if (error == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND 1
			unix_ErrPathNotFound(error, path);
		} else
#endif /* ENOENT */
#ifdef CONFIG_HAVE_fchdir
#ifdef EBADF
		if (error == EBADF) {
#define WANT_UNIX_ERRHANDLECLOSED 1
			unix_ErrHandleClosed(error, path);
		} else
#endif /* EBADF */
#endif /* CONFIG_HAVE_fchdir */
		{
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to change the current working directory to %r",
			                          path);
		}
		goto err;
	}
done:
	return 0;
err:
	return -1;
#endif /* fs_chdir_USE_WCHDIR || fs_chdir_USE_CHDIR */

#ifdef fs_chdir_USE_STUB
	(void)path;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("chdir");
#endif /* fs_chdir_USE_STUB */
}




/************************************************************************/
/* fs_unlink()                                                          */
/************************************************************************/
#undef fs_unlink_USE_DELETEFILE
#undef fs_unlink_USE_WUNLINK
#undef fs_unlink_USE_UNLINK
#undef fs_unlink_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_unlink_USE_DELETEFILE 1
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_unlink_USE_WUNLINK 1
#elif defined(CONFIG_HAVE_unlink)
#define fs_unlink_USE_UNLINK 1
#elif defined(CONFIG_HAVE_wunlink)
#define fs_unlink_USE_WUNLINK 1
#else
#define fs_unlink_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1)) int DCALL
fs_unlink(DeeObject *__restrict path) {
#ifdef fs_unlink_USE_DELETEFILE
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again_deletefile:
#define WANT_NT_DELETEFILE 1
	error = nt_DeleteFile(path);
	if unlikely(error <= 0)
		return error;
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (error == ERROR_ACCESS_DENIED) {
		/* Check if we've failed to delete a symbolic
		 * directory-link (for which RemoveDirectory() must be used) */
		DWORD attr;
again_getattr:
#define WANT_NT_GETFILEATTRIBUTES 1
		error = nt_GetFileAttributes(path, &attr);
		if (error < 0)
			goto err;
		if (!error &&
		    (attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ==
		    (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) {
again_rmdir:
#define WANT_NT_REMOVEDIRECTORY 1
			error = nt_RemoveDirectory(path);
			if unlikely(error <= 0)
				return error;
			DBG_ALIGNMENT_DISABLE();
			error = (int)GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
				if (Dee_CollectMemory(1))
					goto again_rmdir;
				goto err;
			}
#define WANT_NT_ERRRMDIR 1
			nt_ErrRmDir((DWORD)error, path);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error = (int)GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
			if (Dee_CollectMemory(1))
				goto again_getattr;
			goto err;
		}
		error = ERROR_ACCESS_DENIED;
	}
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again_deletefile;
		goto err;
	}
#define WANT_NT_ERRUNLINK 1
	nt_ErrUnlink((DWORD)error, path);
err:
	return -1;
#endif /* fs_unlink_USE_DELETEFILE */

#if defined(fs_unlink_USE_WUNLINK) || defined(fs_unlink_USE_UNLINK)
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef fs_unlink_USE_WUNLINK
	{
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = wunlink(wpath);
		DBG_ALIGNMENT_ENABLE();
	}
#else /* fs_unlink_USE_WUNLINK */
	{
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = unlink(utf8);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* !fs_unlink_USE_WUNLINK */
	if unlikely(error) {
		error = DeeSystem_GetErrno();
#define WANT_UNIX_ERRUNLINK 1
		unix_ErrUnlink(error, path);
		goto err;
	}
	return 0;
err:
	return -1;
#endif /* fs_unlink_USE_WUNLINK || fs_unlink_USE_UNLINK */

#ifdef fs_unlink_USE_STUB
	(void)path;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("unlink");
#endif /* fs_unlink_USE_STUB */
}




/************************************************************************/
/* fs_remove()                                                          */
/************************************************************************/
#undef fs_remove_USE_DELETEFILE_REMOVEDIRECTORY
#undef fs_remove_USE_WREMOVE
#undef fs_remove_USE_REMOVE
#undef fs_remove_USE_WUNLINK_WRMDIR
#undef fs_remove_USE_WUNLINK_RMDIR
#undef fs_remove_USE_UNLINK_WRMDIR
#undef fs_remove_USE_UNLINK_RMDIR
#undef fs_remove_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_remove_USE_DELETEFILE_REMOVEDIRECTORY 1
#elif defined(CONFIG_HAVE_wremove) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_remove_USE_WREMOVE 1
#elif defined(CONFIG_HAVE_remove)
#define fs_remove_USE_REMOVE 1
#elif defined(CONFIG_HAVE_wremove)
#define fs_remove_USE_WREMOVE 1
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_wrmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_remove_USE_WUNLINK_WRMDIR 1
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_rmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_remove_USE_WUNLINK_RMDIR 1
#elif defined(CONFIG_HAVE_unlink) && defined(CONFIG_HAVE_wrmdir) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_remove_USE_UNLINK_WRMDIR 1
#elif defined(CONFIG_HAVE_unlink) && defined(CONFIG_HAVE_rmdir)
#define fs_remove_USE_UNLINK_RMDIR 1
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_wrmdir)
#define fs_remove_USE_WUNLINK_WRMDIR 1
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_HAVE_rmdir)
#define fs_remove_USE_WUNLINK_RMDIR 1
#elif defined(CONFIG_HAVE_unlink) && defined(CONFIG_HAVE_wrmdir)
#define fs_remove_USE_UNLINK_WRMDIR 1
#else
#define fs_remove_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1)) int DCALL
fs_remove(DeeObject *__restrict path) {
#ifdef fs_remove_USE_DELETEFILE_REMOVEDIRECTORY
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
again:
#define WANT_NT_DELETEFILE 1
	error = nt_DeleteFile(path);
	if likely(error <= 0)
		return error;
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	/* NOTE: DeleteFile() sets `ERROR_ACCESS_DENIED'
	 *       if the folder is actually a directory. */
	if (error == ERROR_ACCESS_DENIED) {
#define WANT_NT_REMOVEDIRECTORY 1
		error = nt_RemoveDirectory(path);
		if (error <= 0)
			return error;
		DBG_ALIGNMENT_DISABLE();
		error = (int)GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
#define WANT_NT_ERRRMDIR 1
		nt_ErrRmDir((DWORD)error, path);
		goto err;
	}
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}
#define WANT_NT_ERRUNLINK 1
	nt_ErrUnlink((DWORD)error, path);
err:
	return -1;
#endif /* fs_remove_USE_DELETEFILE_REMOVEDIRECTORY */

#if defined(fs_remove_USE_WREMOVE) || defined(fs_remove_USE_REMOVE)
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef fs_remove_USE_WREMOVE
	{
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = wremove(wpath);
		DBG_ALIGNMENT_ENABLE();
	}
#else /* fs_remove_USE_WREMOVE */
	{
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = remove(utf8);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* !fs_remove_USE_WREMOVE */
	if unlikely(error) {
		error = DeeSystem_GetErrno();
#define WANT_UNIX_ERRREMOVE 1
		unix_ErrRemove(error, path);
		goto err;
	}
	return 0;
err:
	return -1;
#endif /* fs_remove_USE_WREMOVE || fs_remove_USE_REMOVE */

#if (defined(fs_remove_USE_WUNLINK_WRMDIR) || \
     defined(fs_remove_USE_WUNLINK_RMDIR) ||  \
     defined(fs_remove_USE_UNLINK_WRMDIR) ||  \
     defined(fs_remove_USE_UNLINK_RMDIR))
	int error;
#if (defined(fs_remove_USE_WUNLINK_WRMDIR) || \
     defined(fs_remove_USE_WUNLINK_RMDIR) || \
     defined(fs_remove_USE_UNLINK_WRMDIR))
#define fs_remove_USE_WCHAR 1
	wchar_t *wpath;
#endif /* wchar */
#if (defined(fs_remove_USE_WUNLINK_RMDIR) || \
     defined(fs_remove_USE_UNLINK_WRMDIR) || \
     defined(fs_remove_USE_UNLINK_RMDIR))
#define fs_remove_USE_UTF8 1
	char *utf8;
#endif /* utf-8 */
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef fs_remove_USE_WCHAR
	wpath = (wchar_t *)DeeString_AsWide(path);
	if unlikely(!wpath)
		goto err;
#endif /* fs_remove_USE_WCHAR */
#ifdef fs_remove_USE_UTF8
	utf8 = DeeString_AsUtf8(path);
	if unlikely(!utf8)
		goto err;
#endif /* fs_remove_USE_UTF8 */

#ifdef ENOMEM
again:
#endif /* ENOMEM */
	DBG_ALIGNMENT_DISABLE();
#if defined(fs_remove_USE_WUNLINK_WRMDIR) || defined(fs_remove_USE_WUNLINK_RMDIR)
	error = wunlink(wpath);
#else /* fs_remove_USE_WUNLINK_WRMDIR || fs_remove_USE_WUNLINK_RMDIR */
	error = unlink(utf8);
#endif /* !fs_remove_USE_WUNLINK_WRMDIR && !fs_remove_USE_WUNLINK_RMDIR */
	DBG_ALIGNMENT_ENABLE();
	if (error) {
		error = DeeSystem_GetErrno();
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
#else
		if (error == EPERM)
#endif
		{
			/* Delete a directory. */
			DBG_ALIGNMENT_DISABLE();
#if defined(fs_remove_USE_WUNLINK_WRMDIR) || defined(fs_remove_USE_UNLINK_WRMDIR)
			error = wrmdir(wpath);
#else /* fs_remove_USE_WUNLINK_WRMDIR || fs_remove_USE_UNLINK_WRMDIR */
			error = rmdir(utf8);
#endif /* !fs_remove_USE_WUNLINK_WRMDIR && !fs_remove_USE_UNLINK_WRMDIR */
			DBG_ALIGNMENT_ENABLE();
			if likely(!error)
				goto done;
			error = DeeSystem_GetErrno();
			DeeSystem_IF_E1(error, ENOMEM, {
				if (Dee_CollectMemory(1))
					goto again;
				goto err;
			});
#define WANT_UNIX_ERRRMDIR 1
			unix_ErrRmDir(error, path);
		} else {
			/* Handle errors. */
#define WANT_UNIX_ERRUNLINK 1
			unix_ErrUnlink(error, path);
		}
#else /* EISDIR || EPERM */
		/* Delete a directory. */
		DBG_ALIGNMENT_DISABLE();
#if defined(fs_remove_USE_WUNLINK_WRMDIR) || defined(fs_remove_USE_UNLINK_WRMDIR)
		error = wrmdir(wpath);
#else /* fs_remove_USE_WUNLINK_WRMDIR || fs_remove_USE_UNLINK_WRMDIR */
		error = rmdir(utf8);
#endif /* !fs_remove_USE_WUNLINK_WRMDIR && !fs_remove_USE_UNLINK_WRMDIR */
		DBG_ALIGNMENT_ENABLE();
		if likely(!error)
			goto done;
		error = DeeSystem_GetErrno();
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		});
#define WANT_UNIX_ERRRMDIR 1
		unix_ErrRmDir(error, path);
#endif /* !EISDIR && !EPERM */
		goto err;
	}
done:
	return 0;
err:
	return -1;
#endif /* unlink() + rmdir() */

#ifdef fs_remove_USE_STUB
	(void)path;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("remove");
#endif /* fs_remove_USE_STUB */
}




/************************************************************************/
/* fs_rename()                                                          */
/************************************************************************/
#undef fs_rename_USE_MOVEFILE
#undef fs_rename_USE_WRENAME
#undef fs_rename_USE_RENAME
#undef fs_rename_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_rename_USE_MOVEFILE 1
#elif defined(CONFIG_HAVE_wrename) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_rename_USE_WRENAME 1
#elif defined(CONFIG_HAVE_rename)
#define fs_rename_USE_RENAME 1
#elif defined(CONFIG_HAVE_wrename)
#define fs_rename_USE_WRENAME 1
#else
#define fs_rename_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_rename(DeeObject *__restrict existing_path,
          DeeObject *__restrict new_path) {
#ifdef fs_rename_USE_MOVEFILE
	int error;
	if (!DeeString_Check(existing_path)) {
		if (DeeInt_Check(existing_path)) {
			HANDLE fd; /* Support for descriptor-based rename() */
			if (DeeObject_AsUIntptr(existing_path, (uintptr_t *)&fd))
				goto err;
			existing_path = DeeNTSystem_GetFilenameOfHandle(fd);
		} else {
			existing_path = DeeFile_Filename(existing_path);
		}
		if unlikely(!existing_path)
			goto err;
		error = fs_rename(existing_path, new_path);
		Dee_Decref(existing_path);
		return error;
	}
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(new_path, &DeeString_Type))
		goto err;
again_movefile:
#define WANT_NT_MOVEFILE 1
	error = nt_MoveFile(existing_path, new_path);
	if (error > 0)
		goto err_nt;
	return error;
err_nt:
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again_movefile;
	} else if (DeeNTSystem_IsAccessDeniedError((DWORD)error)) {
#define WANT_NT_ERRPATHNOACCESS2 1
		nt_ErrPathNoAccess2((DWORD)error, existing_path, new_path);
	} else if (DeeNTSystem_IsBusy((DWORD)error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_BusyFile, (DWORD)error,
		                        "Path %r or %r cannot be accessed because it is already in use",
		                        existing_path, new_path);
	} else if ((DWORD)error == ERROR_SHARING_VIOLATION) {
		DeeNTSystem_ThrowErrorf(&DeeError_ValueError, (DWORD)error,
		                        "Cannot rename path %r to %r which is a sub-directory of the old path",
		                        existing_path, new_path);
	} else if (DeeNTSystem_IsFileNotFoundError((DWORD)error)) {
#define WANT_NT_ERRPATHNOTFOUND2 1
		nt_ErrPathNotFound2((DWORD)error, existing_path, new_path);
	} else if (DeeNTSystem_IsNotEmpty((DWORD)error) || DeeNTSystem_IsExists((DWORD)error)) {
#define WANT_NT_ERRPATHEXISTS 1
		nt_ErrPathExists((DWORD)error, new_path);
	} else if (DeeNTSystem_IsNotDir((DWORD)error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_NoDirectory, (DWORD)error,
		                        "Some part of the path %r or %r is not a directory",
		                        existing_path, new_path);
	} else if (DeeNTSystem_IsUnsupportedError((DWORD)error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, (DWORD)error,
		                        "The filesystem hosting the paths %r and %r "
		                        "does not support renaming of files",
		                        existing_path, new_path);
	} else if (DeeNTSystem_IsXDev((DWORD)error)) {
#define WANT_NT_ERRPATHCROSSDEV2 1
		nt_ErrPathCrossDev2((DWORD)error, existing_path, new_path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, (DWORD)error,
		                        "Failed to rename %r to %r",
		                        existing_path, new_path);
	}
err:
	return -1;
#endif /* fs_rename_USE_MOVEFILE */

#if defined(fs_rename_USE_WRENAME) || defined(fs_rename_USE_RENAME)
	int error;
#ifdef fs_rename_USE_WRENAME
	wchar_t *woldpath, *wnewpath;
#else /* fs_rename_USE_WRENAME */
	char *uoldpath, *unewpath;
#endif /* !fs_rename_USE_WRENAME */
	if (!DeeString_Check(existing_path)) {
		existing_path = DeeFile_Filename(existing_path);
		if unlikely(!existing_path)
			goto err;
		error = fs_rename(existing_path, new_path);
		Dee_Decref(existing_path);
		return error;
	}
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(new_path, &DeeString_Type))
		goto err;
#ifdef fs_rename_USE_WRENAME
	woldpath = (wchar_t *)DeeString_AsWide(existing_path);
	if unlikely(!woldpath)
		goto err;
	wnewpath = (wchar_t *)DeeString_AsWide(new_path);
	if unlikely(!wnewpath)
		goto err;
#else /* fs_rename_USE_WRENAME */
	uoldpath = DeeString_AsUtf8(existing_path);
	if unlikely(!uoldpath)
		goto err;
	unewpath = DeeString_AsUtf8(new_path);
	if unlikely(!unewpath)
		goto err;
#endif /* !fs_rename_USE_WRENAME */
	DBG_ALIGNMENT_DISABLE();
#ifdef fs_rename_USE_WRENAME
	error = wrename(woldpath, wnewpath);
#else /* fs_rename_USE_WRENAME */
	error = rename(uoldpath, unewpath);
#endif /* !fs_rename_USE_WRENAME */
	DBG_ALIGNMENT_ENABLE();
	if likely(!error)
		return 0;
	error = DeeSystem_GetErrno();
#ifdef EACCES
	if (error == EACCES) {
#ifdef EPERM
err_access:
#endif /* EPERM */
		DBG_ALIGNMENT_ENABLE();
#define WANT_UNIX_ERRPATHREADONLY2 1
		unix_ErrPathReadonly2(error, existing_path, new_path);
	} else
#endif /* EACCES */
#ifdef EBUSY
	if (error == EBUSY) {
		DeeUnixSystem_ThrowErrorf(&DeeError_BusyFile, error,
		                          "Path %r or %r cannot be accessed because it is already in use",
		                          existing_path, new_path);
	} else
#endif /* EBUSY */
#ifdef EINVAL
	if (error == EINVAL) {
		DeeUnixSystem_ThrowErrorf(&DeeError_ValueError, error,
		                          "Cannot rename path %r to %r which is a sub-directory of the old path",
		                          existing_path, new_path);
	} else
#endif /* EINVAL */
#ifdef ENOENT
	if (error == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND2 1
		unix_ErrPathNotFound2(error, existing_path, new_path);
	} else
#endif /* ENOENT */
#if defined(ENOTEMPTY) || defined(EEXIST)
#if defined(ENOTEMPTY) && defined(EEXIST)
	if (error == ENOTEMPTY || error == EEXIST)
#elif defined(ENOTEMPTY)
	if (error == ENOTEMPTY)
#else
	if (error == EEXIST)
#endif
	{
#define WANT_UNIX_ERRPATHEXISTS 1
		unix_ErrPathExists(error, new_path);
	} else
#endif /* ENOTEMPTY || EEXIST */
#ifdef ENOTDIR
	if (error == ENOTDIR) {
		DeeUnixSystem_ThrowErrorf(&DeeError_NoDirectory, error,
		                          "Some part of the path %r or %r is not a directory",
		                          existing_path, new_path);
	} else
#endif /* ENOTDIR */
#ifdef EPERM
	if (error == EPERM) {
		/* The same deal concerning the sticky bit. */
#ifdef LSTAT
#ifdef WLSTAT
		STRUCT_WSTAT st;
#ifndef fs_rename_USE_WRENAME
		wchar_t *woldpath, *wnewpath;
		woldpath = (wchar_t *)DeeString_AsWide(existing_path);
		if unlikely(!woldpath)
			goto err;
		wnewpath = (wchar_t *)DeeString_AsWide(new_path);
		if unlikely(!wnewpath)
			goto err;
#endif /* !fs_rename_USE_WRENAME */
		DBG_ALIGNMENT_DISABLE();
		if ((!WLSTAT(woldpath, &st) && (st.st_mode & STAT_ISVTX)) ||
		    (!WLSTAT(wnewpath, &st) && (st.st_mode & STAT_ISVTX)))
#else /* WLSTAT */
		STRUCT_STAT st;
#ifdef fs_rename_USE_WRENAME
		char *uoldpath, *unewpath;
		uoldpath = DeeString_AsUtf8(existing_path);
		if unlikely(!uoldpath)
			goto err;
		unewpath = DeeString_AsUtf8(new_path);
		if unlikely(!unewpath)
			goto err;
#endif /* fs_rename_USE_WRENAME */
		DBG_ALIGNMENT_DISABLE();
		if ((!LSTAT(uoldpath, &st) && (st.st_mode & STAT_ISVTX)) ||
		    (!LSTAT(unewpath, &st) && (st.st_mode & STAT_ISVTX)))
#endif /* !WLSTAT */
		{
#ifdef EACCES
			goto err_access;
#else /* EACCES */
			DBG_ALIGNMENT_ENABLE();
#define WANT_UNIX_ERRPATHREADONLY2 1
			unix_ErrPathReadonly2(error, existing_path, new_path);
#endif /* !EACCES */
		}
		DBG_ALIGNMENT_ENABLE();
#endif /* LSTAT */
		DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, error,
		                          "The filesystem hosting the paths %r and %r "
		                          "does not support renaming of files",
		                          existing_path, new_path);
	} else
#endif /* EPERM */
#ifdef EROFS
	if (error == EROFS) {
#define WANT_UNIX_ERRPATHREADONLY2 1
		unix_ErrPathReadonly2(error, existing_path, new_path);
	} else
#endif /* EROFS */
#ifdef EXDEV
	if (error == EXDEV) {
#define WANT_UNIX_ERRPATHCROSSDEV2 1
		unix_ErrPathCrossDev2(error, existing_path, new_path);
	} else
#endif /* EXDEV */
	{
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to rename %r to %r",
		                          existing_path, new_path);
	}
err:
	return -1;
#endif /* fs_rename_USE_WRENAME || fs_rename_USE_RENAME */

#ifdef fs_rename_USE_STUB
	(void)existing_path;
	(void)new_path;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("rename");
#endif /* fs_rename_USE_STUB */
}




/************************************************************************/
/* fs_link()                                                           */
/************************************************************************/
#undef fs_link_USE_CREATEHARDLINK
#undef fs_link_USE_WLINK
#undef fs_link_USE_LINK
#undef fs_link_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_link_USE_CREATEHARDLINK 1
#elif defined(CONFIG_HAVE_wlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_link_USE_WLINK 1
#elif defined(CONFIG_HAVE_link)
#define fs_link_USE_LINK 1
#elif defined(CONFIG_HAVE_wlink)
#define fs_link_USE_WLINK 1
#else
#define fs_link_USE_STUB 1
#endif

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_link(DeeObject *__restrict existing_path,
        DeeObject *__restrict new_path) {
#ifdef fs_link_USE_CREATEHARDLINK
	int error;
	if (!DeeString_Check(existing_path)) {
		if (DeeInt_Check(existing_path)) {
			HANDLE fd; /* Support for descriptor-based link() */
			if (DeeObject_AsUIntptr(existing_path, (uintptr_t *)&fd))
				goto err;
			existing_path = DeeNTSystem_GetFilenameOfHandle(fd);
		} else {
			existing_path = DeeFile_Filename(existing_path);
		}
		if unlikely(!existing_path)
			goto err;
		error = fs_link(existing_path, new_path);
		Dee_Decref(existing_path);
		return error;
	}
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(new_path, &DeeString_Type))
		goto err;
again:
#define WANT_NT_CREATEHARDLINK 1
	error = nt_CreateHardLink(new_path, existing_path, NULL);
	if likely(error <= 0)
		return error;
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again;
	} else if (DeeNTSystem_IsAccessDeniedError((DWORD)error)) {
#define WANT_NT_ERRPATHNOACCESS2 1
		nt_ErrPathNoAccess2((DWORD)error, existing_path, new_path);
	} else if (DeeNTSystem_IsExists((DWORD)error)) {
#define WANT_NT_ERRPATHEXISTS 1
		nt_ErrPathExists((DWORD)error, new_path);
	} else if (DeeNTSystem_IsFileNotFoundError((DWORD)error)) {
#define WANT_NT_ERRPATHNOTFOUND2 1
		nt_ErrPathNotFound2((DWORD)error, existing_path, new_path);
	} else if (DeeNTSystem_IsXDev((DWORD)error)) {
#define WANT_NT_ERRPATHCROSSDEV2 1
		nt_ErrPathCrossDev2((DWORD)error, existing_path, new_path);
	} else if (DeeNTSystem_IsUnsupportedError((DWORD)error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, (DWORD)error,
		                        "The filesystem hosting the paths %r and %r "
		                        "does not support creation of hardlinks",
		                        existing_path, new_path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, (DWORD)error,
		                        "Failed to rename %r to %r",
		                        existing_path, new_path);
	}
err:
	return -1;
#endif /* fs_link_USE_CREATEHARDLINK */

#if defined(fs_link_USE_WLINK) || defined(fs_link_USE_LINK)
	int error;
#ifdef fs_link_USE_WLINK
	wchar_t *woldpath, *wnewpath;
#else /* fs_link_USE_WLINK */
	char *uoldpath, *unewpath;
#endif /* !fs_link_USE_WLINK */
	if (!DeeString_Check(existing_path)) {
		if (DeeInt_Check(existing_path)) {
			int fd; /* Support for descriptor-based link() */
			if (DeeObject_AsInt(existing_path, &fd))
				goto err;
			existing_path = DeeSystem_GetFilenameOfFD(fd);
		} else {
			existing_path = DeeFile_Filename(existing_path);
		}
		if unlikely(!existing_path)
			goto err;
		error = fs_link(existing_path, new_path);
		Dee_Decref(existing_path);
		return error;
	}
	if (!DeeString_Check(existing_path)) {
		existing_path = DeeFile_Filename(existing_path);
		if unlikely(!existing_path)
			goto err;
		error = fs_link(existing_path, new_path);
		Dee_Decref(existing_path);
		return error;
	}
	if (DeeObject_AssertTypeExact(new_path, &DeeString_Type))
		goto err;
#ifdef fs_link_USE_WLINK
	woldpath = (wchar_t *)DeeString_AsWide(existing_path);
	if unlikely(!woldpath)
		goto err;
	wnewpath = (wchar_t *)DeeString_AsWide(new_path);
	if unlikely(!wnewpath)
		goto err;
#else /* fs_link_USE_WLINK */
	uoldpath = DeeString_AsUtf8(existing_path);
	if unlikely(!uoldpath)
		goto err;
	unewpath = DeeString_AsUtf8(new_path);
	if unlikely(!unewpath)
		goto err;
#endif /* !fs_link_USE_WLINK */
	DBG_ALIGNMENT_DISABLE();
#ifdef fs_link_USE_WLINK
	error = wlink(woldpath, wnewpath);
#else /* fs_link_USE_WLINK */
	error = link(uoldpath, unewpath);
#endif /* !fs_link_USE_WLINK */
	DBG_ALIGNMENT_ENABLE();
	if likely(!error)
		return 0;
	error = DeeSystem_GetErrno();
#ifdef EACCES
	if (error == EACCES) {
#define WANT_UNIX_ERRPATHREADONLY2 1
		unix_ErrPathReadonly2(error, existing_path, new_path);
	} else
#endif /* EACCES */
#ifdef EEXIST
	if (error == EEXIST) {
#define WANT_UNIX_ERRPATHEXISTS 1
		unix_ErrPathExists(error, new_path);
	} else
#endif /* EEXIST */
#ifdef ENOENT
	if (error == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND2 1
		unix_ErrPathNotFound2(error, existing_path, new_path);
	} else
#endif /* ENOENT */
#ifdef EPERM
	if (error == EPERM) {
		DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, error,
		                          "The filesystem hosting the paths %r and %r "
		                          "does not support creation of hardlinks",
		                          existing_path, new_path);
	} else
#endif /* EPERM */
#ifdef EROFS
	if (error == EROFS) {
#define WANT_UNIX_ERRPATHREADONLY2 1
		unix_ErrPathReadonly2(error, existing_path, new_path);
	} else
#endif /* EROFS */
#ifdef EXDEV
	if (error == EXDEV) {
#define WANT_UNIX_ERRPATHCROSSDEV2 1
		unix_ErrPathCrossDev2(error, existing_path, new_path);
	} else
#endif /* EXDEV */
	{
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to rename %r to %r",
		                          existing_path, new_path);
	}
err:
	return -1;
#endif /* fs_link_USE_WLINK || fs_link_USE_LINK */

#ifdef fs_link_USE_STUB
	(void)existing_path;
	(void)new_path;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("link");
#endif /* fs_link_USE_STUB */
}




/************************************************************************/
/* fs_symlink()                                                         */
/************************************************************************/
#undef fs_symlink_USE_CREATESYMBOLICLINK
#undef fs_symlink_USE_WSYMLINK
#undef fs_symlink_USE_SYMLINK
#undef fs_symlink_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_symlink_USE_CREATESYMBOLICLINK 1
#elif defined(CONFIG_HAVE_wsymlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_symlink_USE_WSYMLINK 1
#elif defined(CONFIG_HAVE_symlink)
#define fs_symlink_USE_SYMLINK 1
#elif defined(CONFIG_HAVE_wsymlink)
#define fs_symlink_USE_WSYMLINK 1
#else
#define fs_symlink_USE_STUB 1
#endif

#ifdef fs_symlink_USE_CREATESYMBOLICLINK
PRIVATE WCHAR const str_SeCreateSymbolicLinkPrivilege[] = {
	'S', 'e', 'C', 'r', 'e', 'a', 't', 'e', 'S', 'y', 'm', 'b', 'o', 'l', 'i',
	'c', 'L', 'i', 'n', 'k', 'P', 'r', 'i', 'v', 'i', 'l', 'e', 'g', 'e', 0
};

PRIVATE BOOL DCALL nt_AcquirePrivilege(LPCWSTR lpName) {
	HANDLE tok, hProcess;
	LUID luid;
	TOKEN_PRIVILEGES tok_priv;
	DWORD error;
	hProcess = GetCurrentProcess();
	if unlikely(!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &tok))
		goto fail;
	if unlikely(!LookupPrivilegeValueW(NULL, lpName, &luid))
		goto fail;
	tok_priv.PrivilegeCount           = 1;
	tok_priv.Privileges[0].Luid       = luid;
	tok_priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if unlikely(!AdjustTokenPrivileges(tok, FALSE, &tok_priv, 0, NULL, NULL))
		return FALSE;
	error = GetLastError();
	SetLastError(0);
	return (unlikely(error == ERROR_NOT_ALL_ASSIGNED))
	       ? 0
	       : 1;
fail:
	return FALSE;
}
#endif /* fs_symlink_USE_CREATESYMBOLICLINK */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_symlink(DeeObject *__restrict target_text,
           DeeObject *__restrict link_path,
           bool format_target) {
#ifdef fs_symlink_USE_CREATESYMBOLICLINK
#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x1
#endif /* !SYMBOLIC_LINK_FLAG_DIRECTORY */
#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
#endif /* !SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE */
	PRIVATE DWORD dwSymlinkAdditionalFlags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
	PRIVATE BOOL bHoldingSymlinkPriv       = FALSE;
	int error;
	DWORD dwFlags;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(target_text, &DeeString_Type) ||
	    DeeObject_AssertTypeExact(link_path, &DeeString_Type))
		goto err;
	(void)format_target; /* TODO: Fix slashes. */
	dwFlags = dwSymlinkAdditionalFlags;
	/* TODO: `SYMBOLIC_LINK_FLAG_DIRECTORY'
	 * >> local abs_target = target_text;
	 * >> if (!fs.isabs(target_text))
	 * >>      abs_target = fs.abspath(target_text, fs.headof(link_path));
	 * >> if (fs.stat.isdir(abs_target))
	 * >>     flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
	 */
again:
	error = nt_CreateSymbolicLink(link_path, target_text, dwFlags);
	if unlikely(error > 0)
		goto err_nt;
	return error;
err_nt:
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (error == ERROR_INVALID_PARAMETER &&
	    (dwFlags & SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) {
		/* Older versions of windows didn't accept this flag. */
		ATOMIC_FETCHAND(dwSymlinkAdditionalFlags, ~SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE);
		dwFlags &= ~SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
		goto again;
	}
	/* Try to acquire the ~privilege~ to create symbolic links. */
	if (error == ERROR_PRIVILEGE_NOT_HELD) {
		if (!bHoldingSymlinkPriv) {
			DBG_ALIGNMENT_DISABLE();
			if (nt_AcquirePrivilege(str_SeCreateSymbolicLinkPrivilege)) {
				DBG_ALIGNMENT_ENABLE();
				bHoldingSymlinkPriv = TRUE;
				goto again;
			}
			DBG_ALIGNMENT_ENABLE();
		}
		/* May as well not exist at all... */
		DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, error,
		                        "The operating system has restricted "
		                        "access to symlink functionality");
	} else if (DeeNTSystem_IsAccessDeniedError(error)) {
#define WANT_NT_ERRPATHREADONLY 1
		nt_ErrPathReadonly(error, link_path);
	} else if (DeeNTSystem_IsExists(error)) {
#define WANT_NT_ERRPATHEXISTS 1
		nt_ErrPathExists(error, link_path);
	} else if (DeeNTSystem_IsFileNotFoundError(error)) {
#define WANT_NT_ERRPATHNOTFOUND 1
		nt_ErrPathNotFound(error, link_path);
	} else if (DeeNTSystem_IsNotDir(error)) {
#define WANT_NT_ERRPATHNOTDIR 1
		nt_ErrPathNotDir(error, link_path);
	} else if (DeeNTSystem_IsUnsupportedError(error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, error,
		                        "The filesystem hosting the path %r "
		                        "does not support creation of symbolic links",
		                        link_path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, error,
		                        "Failed to create a symbolic link %r with text %r",
		                        link_path, target_text);
	}
err:
	return -1;
#endif /* fs_symlink_USE_CREATESYMBOLICLINK */

#if defined(fs_symlink_USE_WSYMLINK) || defined(fs_symlink_USE_SYMLINK)
	int error;
#ifdef fs_symlink_USE_WSYMLINK
	wchar_t *wtarget_text, *wlink_path;
#else /* fs_symlink_USE_WSYMLINK */
	char *utarget_text, *ulink_path;
#endif /* !fs_symlink_USE_WSYMLINK */
	(void)format_target;
	if (DeeObject_AssertTypeExact(target_text, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(link_path, &DeeString_Type))
		goto err;
#ifdef fs_symlink_USE_WSYMLINK
	wtarget_text = (wchar_t *)DeeString_AsWide(target_text);
	if unlikely(!wtarget_text)
		goto err;
	wlink_path = (wchar_t *)DeeString_AsWide(link_path);
	if unlikely(!wlink_path)
		goto err;
#else /* fs_symlink_USE_WSYMLINK */
	utarget_text = DeeString_AsUtf8(target_text);
	if unlikely(!utarget_text)
		goto err;
	ulink_path = DeeString_AsUtf8(link_path);
	if unlikely(!ulink_path)
		goto err;
#endif /* !fs_symlink_USE_WSYMLINK */
	DBG_ALIGNMENT_DISABLE();
#ifdef fs_symlink_USE_WSYMLINK
	error = wsymlink(wtarget_text, wlink_path);
#else /* fs_symlink_USE_WSYMLINK */
	error = symlink(utarget_text, ulink_path);
#endif /* !fs_symlink_USE_WSYMLINK */
	DBG_ALIGNMENT_ENABLE();
	if unlikely(error) {
		DBG_ALIGNMENT_DISABLE();
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#ifdef EACCES
		if (error == EACCES) {
#define WANT_UNIX_ERRPATHREADONLY 1
			unix_ErrPathReadonly(error, link_path);
		} else
#endif /* EACCES */
#ifdef EEXIST
		if (error == EEXIST) {
#define WANT_UNIX_ERRPATHEXISTS 1
			unix_ErrPathExists(error, link_path);
		} else
#endif /* EEXIST */
#ifdef ENOENT
		if (error == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND 1
			unix_ErrPathNotFound(error, link_path);
		} else
#endif /* ENOENT */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
#define WANT_UNIX_ERRPATHNOTDIR 1
			unix_ErrPathNotDir(error, link_path);
		} else
#endif /* ENOTDIR */
#ifdef EPERM
		if (error == EPERM) {
			DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, error,
			                          "The filesystem hosting the path %r "
			                          "does not support creation of symbolic links",
			                          link_path);
		} else
#endif /* EPERM */
#ifdef EROFS
		if (error == EROFS) {
#define WANT_UNIX_ERRPATHREADONLY 1
			unix_ErrPathReadonly(error, link_path);
		} else
#endif /* EROFS */
		{
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to create a symbolic link %r with text %r",
			                          link_path, target_text);
		}
	}
err:
	return -1;
#endif /* fs_symlink_USE_WSYMLINK || fs_symlink_USE_SYMLINK */

#ifdef fs_symlink_USE_STUB
	(void)target_text;
	(void)link_path;
	(void)format_target;
#define WANT_ERR_UNSUPPORTED 1
	return errUnsupported("symlink");
#endif /* fs_symlink_USE_STUB */
}




/************************************************************************/
/* fs_readlink()                                                        */
/************************************************************************/
#undef fs_readlink_USE_DEVICEIOCONTROL
#undef fs_readlink_USE_WREADLINK
#undef fs_readlink_USE_READLINK
#undef fs_readlink_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define fs_readlink_USE_DEVICEIOCONTROL 1
#elif defined(CONFIG_HAVE_wreadlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define fs_readlink_USE_WREADLINK 1
#elif defined(CONFIG_HAVE_readlink)
#define fs_readlink_USE_READLINK 1
#elif defined(CONFIG_HAVE_wreadlink)
#define fs_readlink_USE_WREADLINK 1
#else
#define fs_readlink_USE_STUB 1
#endif

#ifdef fs_readlink_USE_DEVICEIOCONTROL
#ifndef IO_REPARSE_TAG_LX_SYMLINK
#define IO_REPARSE_TAG_LX_SYMLINK 0xA000001D
#endif /* !IO_REPARSE_TAG_LX_SYMLINK */

typedef struct _DEE_REPARSE_DATA_BUFFER {
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer; /* IO_REPARSE_TAG_SYMLINK */

		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR PathBuffer[1];
		} MountPointReparseBuffer; /* IO_REPARSE_TAG_MOUNT_POINT */

		/* BEGIN: Taken from cygwin */
		struct {
			DWORD FileType;     /* Take member name with a grain of salt.  Value is
			                     * apparently always 2 for symlinks. */
			char PathBuffer[1]; /* POSIX path as given to symlink(2).
			                     * Path is not \0 terminated.
			                     * Length is ReparseDataLength - sizeof (FileType).
			                     * Always UTF-8.
			                     * Chars given in incompatible codesets, e. g. umlauts
			                     * in ISO-8859-x, are converted to the Unicode
			                     * REPLACEMENT CHARACTER 0xfffd == \xef\xbf\bd */
		} LxSymlinkReparseBuffer; /* IO_REPARSE_TAG_LX_SYMLINK */
		/* END: Taken from cygwin */

		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define SymbolicLinkReparseBuffer _dee_aunion.SymbolicLinkReparseBuffer
#define MountPointReparseBuffer   _dee_aunion.MountPointReparseBuffer
#define LxSymlinkReparseBuffer    _dee_aunion.LxSymlinkReparseBuffer
#define GenericReparseBuffer      _dee_aunion.GenericReparseBuffer
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
} DEE_REPARSE_DATA_BUFFER, *DEE_PREPARSE_DATA_BUFFER;
#endif /* fs_readlink_USE_DEVICEIOCONTROL */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fs_readlink(DeeObject *__restrict path) {
#ifdef fs_readlink_USE_DEVICEIOCONTROL
#define READLINK_INITIAL_BUFFER 300
	DEE_PREPARSE_DATA_BUFFER buffer;
	HANDLE hLink;
	DREF DeeObject *result;
	DWORD bufsiz, buflen, error;
	LPWSTR linkstr_begin, linkstr_end;
	bool owns_linkfd;
	if (DeeString_Check(path)) {
again_createfile:
		hLink = DeeNTSystem_CreateFile(path,
		                               FILE_READ_DATA | FILE_READ_ATTRIBUTES,
		                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                               NULL,
		                               OPEN_EXISTING,
		                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
		                               NULL);
		if unlikely(!hLink)
			goto err;
		if unlikely(hLink == INVALID_HANDLE_VALUE)
			goto err_nt_createfile;
ok_got_ownds_linkfd:
		owns_linkfd = true;
	} else {
		hLink = (HANDLE)DeeNTSystem_GetHandle(path);
		if (hLink == INVALID_HANDLE_VALUE) {
			if (!DeeError_Catch(&DeeError_AttributeError) &&
			    !DeeError_Catch(&DeeError_NotImplemented) &&
			    !DeeError_Catch(&DeeError_FileClosed))
				goto err;
			/* Use the filename of a file. */
			path = DeeFile_Filename(path);
			if unlikely(!path)
				goto err;
			result = fs_readlink(path);
			Dee_Decref(path);
			return result;
		}
		owns_linkfd = false;
	}
	bufsiz = READLINK_INITIAL_BUFFER;
	buffer = (DEE_PREPARSE_DATA_BUFFER)Dee_Malloc(bufsiz);
	if unlikely(!buffer)
		goto err_fd;
	/* Read symbolic link data. */
	DBG_ALIGNMENT_DISABLE();
	while (!DeviceIoControl(hLink, FSCTL_GET_REPARSE_POINT,
	                        NULL, 0, buffer, bufsiz, &buflen, NULL)) {
		error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBufferTooSmall(error)) {
			DEE_PREPARSE_DATA_BUFFER new_buffer;
			bufsiz *= 2;
			new_buffer = (DEE_PREPARSE_DATA_BUFFER)Dee_Realloc(buffer, bufsiz);
			if unlikely(!new_buffer)
				goto err_buffer;
			DBG_ALIGNMENT_DISABLE();
			continue;
		}
		if (DeeNTSystem_IsAccessDeniedError(error)) {
#define WANT_NT_ERRPATHNOACCESS 1
			nt_ErrPathNoAccess(error, path);
		} else if (DeeNTSystem_IsNoLink(error)) {
			/* Special handling for cygwin's symbolic links. */
			BY_HANDLE_FILE_INFORMATION hfInfo;
			if (GetFileInformationByHandle(hLink, &hfInfo) &&
			    /* First check: Cygwin's symbolic links always have the SYSTEM flag set. */
			    (hfInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0 &&
			    /* Second check: Let's impose a limit on how long a symlink can be.
			     * The `4 * 65536' used here can also be found within cygwin's source code. */
			    (hfInfo.nFileSizeHigh == 0 && hfInfo.nFileSizeLow <= 4 * 65536)) {
				/* Try to load the file into memory. */
				void *pFileBuffer = buffer;
				OVERLAPPED oOffsetInfo;
				DWORD dwBytesRead;
				static BYTE const cygSymlinkCookie[] = { '!', '<', 's', 'y', 'm', 'l', 'i', 'n', 'k', '>' };

				if (hfInfo.nFileSizeLow > bufsiz) {
					pFileBuffer = Dee_Realloc(buffer, hfInfo.nFileSizeLow);
					if unlikely(!pFileBuffer)
						goto err_buffer;
					buffer = (DEE_PREPARSE_DATA_BUFFER)pFileBuffer; /* For cleanup... */
				}
				bzero(&oOffsetInfo, sizeof(oOffsetInfo));
				if (ReadFile(hLink, pFileBuffer, hfInfo.nFileSizeLow, &dwBytesRead, &oOffsetInfo) &&
				    dwBytesRead > sizeof(cygSymlinkCookie) &&
				    memcmp(pFileBuffer, cygSymlinkCookie, sizeof(cygSymlinkCookie)) == 0) {
					/* Yes! It is a cygwin symlink! -> Now to decode it. */
					char const *symlink_text;
					size_t symlink_size;
					symlink_text = (char const *)((BYTE *)pFileBuffer + sizeof(cygSymlinkCookie));
					symlink_size = dwBytesRead - sizeof(cygSymlinkCookie);
					if (symlink_size > 2) {
						BYTE b0, b1;
						b0 = ((BYTE *)symlink_text)[0];
						b1 = ((BYTE *)symlink_text)[1];
						if ((b0 == 0xff && b1 == 0xfe) || (b0 == 0xfe && b1 == 0xff)) {
							/* Text is encoded as a little-endian wide-string */
							Dee_wchar_t const *wcs_start;
							size_t wcs_length;
							wcs_start  = (Dee_wchar_t const *)(symlink_text + 2);
							wcs_length = (symlink_size - 2) / 2;
							/* Trim trailing NUL-characters */
							while (wcs_length && !wcs_start[wcs_length - 1])
								--wcs_length;
							result = b0 == 0xff
							         ? DeeString_NewWideLe(wcs_start, wcs_length, STRING_ERROR_FIGNORE)
							         : DeeString_NewWideBe(wcs_start, wcs_length, STRING_ERROR_FIGNORE);
						} else {
							goto cygwin_symlink_utf8;
						}
					} else {
cygwin_symlink_utf8:
						/* Trim trailing NUL-characters */
						while (symlink_size && !symlink_text[symlink_size - 1])
							--symlink_size;
						/* Text is encoded as a utf-8 string */
						result = DeeString_NewUtf8(symlink_text,
						                           symlink_size,
						                           STRING_ERROR_FIGNORE);
					}
					goto free_buffer_and_return_result;
				}
			}
			DeeNTSystem_ThrowErrorf(&DeeError_NoLink, error,
			                        "Path %r is not a symbolic link",
			                        path);
		} else if (DeeNTSystem_IsUnsupportedError(error)) {
			DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, error,
			                        "The filesystem hosting the path %r does "
			                        "not support reading symbolic links",
			                        path);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, error,
			                        "Failed to read symbolic link %r",
			                        path);
		}
		goto err_buffer;
	}
	if (owns_linkfd)
		CloseHandle(hLink);
	DBG_ALIGNMENT_ENABLE();
	if (buffer->ReparseDataLength > (USHORT)(buflen - offsetof(DEE_REPARSE_DATA_BUFFER, GenericReparseBuffer)))
		buffer->ReparseDataLength = (USHORT)(buflen - offsetof(DEE_REPARSE_DATA_BUFFER, GenericReparseBuffer));
	/* Interpret the read data. */
	switch (buffer->ReparseTag) {

	case IO_REPARSE_TAG_SYMLINK:
		linkstr_begin = buffer->SymbolicLinkReparseBuffer.PathBuffer;
		linkstr_end = (linkstr_begin + (buffer->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR))) +
		              (buffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR));
		break;

	case IO_REPARSE_TAG_MOUNT_POINT:
		linkstr_begin = buffer->MountPointReparseBuffer.PathBuffer;
		linkstr_end = (linkstr_begin + (buffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR))) +
		              (buffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR));
		break;

	case IO_REPARSE_TAG_LX_SYMLINK: {
		/* I couldn't find any official documentation on the actual format of this symlink type.
		 * However, cygwin supports it as well, and with one of its newer versions, has also
		 * started using it as its method of implementing symbolic links... */
#define OFFSETOF_PATHBUFFER                                                 \
	(offsetof(DEE_REPARSE_DATA_BUFFER, LxSymlinkReparseBuffer.PathBuffer) - \
	 offsetof(DEE_REPARSE_DATA_BUFFER, LxSymlinkReparseBuffer))
		if (buffer->ReparseDataLength <= OFFSETOF_PATHBUFFER)
			goto bad_link_type;
		result = DeeString_NewUtf8(buffer->LxSymlinkReparseBuffer.PathBuffer,
		                           buffer->ReparseDataLength - OFFSETOF_PATHBUFFER,
		                           STRING_ERROR_FIGNORE);
		goto free_buffer_and_return_result;
#undef OFFSETOF_PATHBUFFER
	}	break;

	default:
bad_link_type:
		DeeError_Throwf(&DeeError_UnsupportedAPI,
		                "Unsupported link type %lu in file %r",
		                (unsigned long)buffer->ReparseTag, path);
		goto err_buffer;
	}
	/* Get rid of that annoying '\??\' prefix */
	if (linkstr_begin + 4 <= linkstr_end &&
	    linkstr_begin[0] == '\\' && linkstr_begin[1] == '?' &&
	    linkstr_begin[2] == '?' && linkstr_begin[3] == '\\')
		linkstr_begin += 4;
	/* Create the resulting string. */
	result = DeeString_NewWide(linkstr_begin,
	                           (size_t)(linkstr_end - linkstr_begin),
	                           Dee_STRING_ERROR_FREPLAC);
	/* Free our buffer. */
free_buffer_and_return_result:
	Dee_Free(buffer);
	if (owns_linkfd) {
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(hLink);
		DBG_ALIGNMENT_ENABLE();
	}
	return result;
err_nt_createfile:
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again_createfile;
	} else if (DeeNTSystem_IsNotDir((DWORD)error)) {
#define WANT_NT_ERRPATHNOTDIR 1
		nt_ErrPathNotDir((DWORD)error, path);
	} else if (DeeNTSystem_IsAccessDeniedError((DWORD)error)) {
		/* Try to open the file one more time, only this
		 * time _only_ pass the READ_ATTRIBUTES capability. */
		hLink = DeeNTSystem_CreateFile(path,
		                               FILE_READ_ATTRIBUTES,
		                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                               NULL,
		                               OPEN_EXISTING,
		                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
		                               NULL);
		if unlikely(!hLink)
			goto err;
		if unlikely(hLink != INVALID_HANDLE_VALUE)
			goto ok_got_ownds_linkfd;
#define WANT_NT_ERRPATHNOACCESS 1
		nt_ErrPathNoAccess((DWORD)error, path);
	} else if (DeeNTSystem_IsFileNotFoundError((DWORD)error)) {
#define WANT_NT_ERRPATHNOTFOUND 1
		nt_ErrPathNotFound((DWORD)error, path);
	} else if (DeeNTSystem_IsBadF((DWORD)error)) {
#define WANT_NT_ERRHANDLECLOSED 1
		nt_ErrHandleClosed((DWORD)error, path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, (DWORD)error,
		                        "Failed to read link %r",
		                        path);
	}
err:
	return NULL;
err_buffer:
	Dee_Free(buffer);
err_fd:
	if (owns_linkfd) {
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(hLink);
		DBG_ALIGNMENT_ENABLE();
	}
	goto err;
#endif /* fs_readlink_USE_DEVICEIOCONTROL */

#if defined(fs_readlink_USE_WREADLINK) || defined(fs_readlink_USE_READLINK)
	if (!DeeString_Check(path)) {
		DREF DeeObject *result;
		if (DeeInt_Check(path)) {
			int fd; /* Support for descriptor-based readlink() */
			if (DeeObject_AsInt(path, &fd))
				goto err;
			path = DeeSystem_GetFilenameOfFD(fd);
		} else {
			path = DeeFile_Filename(path);
		}
		if unlikely(!path)
			return NULL;
		result = fs_readlink(path);
		Dee_Decref(path);
		return result;
	} else {
#ifdef fs_readlink_USE_WREADLINK
		wchar_t *wpath, *buffer, *new_buffer;
#else /* fs_readlink_USE_WREADLINK */
		char *utf8, *buffer, *new_buffer;
#endif /* !fs_readlink_USE_WREADLINK */
		int error;
		size_t bufsize, new_size;
		Dee_ssize_t req_size;
		struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
#ifdef fs_readlink_USE_WREADLINK
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err_printer;
#else /* fs_readlink_USE_WREADLINK */
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err_printer;
#endif /* !fs_readlink_USE_WREADLINK */
		bufsize = PATH_MAX;
#ifdef fs_readlink_USE_WREADLINK
		buffer = (wchar_t *)Dee_unicode_printer_alloc_wchar(&printer, bufsize);
#else /* fs_readlink_USE_WREADLINK */
		buffer = Dee_unicode_printer_alloc_utf8(&printer, bufsize);
#endif /* !fs_readlink_USE_WREADLINK */
		if unlikely(!buffer)
			goto err_printer;
		for (;;) {
			DBG_ALIGNMENT_DISABLE();
#ifdef fs_readlink_USE_WREADLINK
			req_size = wreadlink(wpath, buffer, bufsize + 1);
#else /* fs_readlink_USE_WREADLINK */
			req_size = readlink(utf8, buffer, bufsize + 1);
#endif /* !fs_readlink_USE_WREADLINK */
			if unlikely(req_size < 0) {
handle_error:
				error = DeeSystem_GetErrno();
				DBG_ALIGNMENT_ENABLE();
#ifdef EACCES
				if (error == EACCES) {
#define WANT_UNIX_ERRPATHNOACCESS 1
					unix_ErrPathNoAccess(error, path);
				} else
#endif /* EACCES */
#ifdef ENOTDIR
				if (error == ENOTDIR) {
#define WANT_UNIX_ERRPATHNOTDIR 1
					unix_ErrPathNotDir(error, path);
				} else
#endif /* ENOTDIR */
#ifdef ENOENT
				if (error == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND 1
					unix_ErrPathNotFound(error, path);
				} else
#endif /* ENOENT */
#ifdef EINVAL
				if (error == EINVAL) {
no_link:
					DeeUnixSystem_ThrowErrorf(&DeeError_NoLink, error,
					                          "Path %r is not a symbolic link",
					                          path);
				} else
#endif /* EINVAL */
				{
					DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
					                          "Failed to read symbolic link %r",
					                          path);
				}
				goto err;
			}
			DBG_ALIGNMENT_ENABLE();
			if ((size_t)req_size <= bufsize)
				break;
#ifdef LSTAT
			{
#ifdef fs_readlink_USE_WREADLINK
#ifdef WLSTAT
				STRUCT_WSTAT st;
				DBG_ALIGNMENT_DISABLE();
				if (WLSTAT(wpath, &st))
					goto handle_error;
#else /* WLSTAT */
				char *utf8;
				STRUCT_STAT st;
				utf8 = DeeString_AsUtf8(path);
				if unlikely(!utf8)
					goto err;
				DBG_ALIGNMENT_DISABLE();
				if (LSTAT(utf8, &st))
					goto handle_error;
#endif /* !WLSTAT */
#else /* fs_readlink_USE_WREADLINK */
#ifdef WLSTAT
				STRUCT_WSTAT st;
				wchar_t *wpath;
				wpath = (wchar_t *)DeeString_AsWide(path);
				if unlikely(!wpath)
					goto err;
				DBG_ALIGNMENT_DISABLE();
				if (WLSTAT(wpath, &st))
					goto handle_error;
#else /* WLSTAT */
				STRUCT_STAT st;
				DBG_ALIGNMENT_DISABLE();
				if (LSTAT(utf8, &st))
					goto handle_error;
#endif /* !WLSTAT */
#endif /* !fs_readlink_USE_WREADLINK */
				DBG_ALIGNMENT_ENABLE();
				/* Ensure that this is still a symbolic link. */
				if (!STAT_ISLNK(st.st_mode)) {
#ifdef EINVAL
					error = EINVAL;
					goto no_link;
#else /* EINVAL */
					DeeError_Throwf(&DeeError_NoLink,
					                "Path %r is not a symbolic link",
					                path);
					goto err;
#endif /* !EINVAL */
				}
				new_size = (size_t)st.st_size;
			}
			if (new_size <= bufsize)
				break; /* Shouldn't happen, but might due to race conditions? */
#else /* LSTAT */
			new_size = bufsize * 2;
				break; /* Shouldn't happen, but might due to race conditions? */
#endif /* !LSTAT */
#ifdef fs_readlink_USE_WREADLINK
			new_buffer = (wchar_t *)Dee_unicode_printer_resize_wchar(&printer, buffer, new_size);
#else /* fs_readlink_USE_WREADLINK */
			new_buffer = Dee_unicode_printer_resize_utf8(&printer, buffer, new_size);
#endif /* !fs_readlink_USE_WREADLINK */
			if unlikely(!new_buffer)
				goto err;
			buffer  = new_buffer;
			bufsize = new_size;
		}
		/* Release unused data. */
#ifdef fs_readlink_USE_WREADLINK
		if (Dee_unicode_printer_confirm_wchar(&printer, (Dee_wchar_t *)buffer, (size_t)req_size) < 0)
			goto err_printer;
#else /* fs_readlink_USE_WREADLINK */
		if (Dee_unicode_printer_confirm_utf8(&printer, buffer, (size_t)req_size) < 0)
			goto err_printer;
#endif /* !fs_readlink_USE_WREADLINK */
		return Dee_unicode_printer_pack(&printer);
err:
#ifdef fs_readlink_USE_WREADLINK
		Dee_unicode_printer_free_wchar(&printer, (Dee_wchar_t *)buffer);
#else /* fs_readlink_USE_WREADLINK */
		Dee_unicode_printer_free_utf8(&printer, buffer);
#endif /* !fs_readlink_USE_WREADLINK */
err_printer:
		Dee_unicode_printer_fini(&printer);
		return NULL;
	}
#endif /* fs_readlink_USE_WREADLINK || fs_readlink_USE_READLINK */

#ifdef fs_readlink_USE_STUB
	(void)path;
#define WANT_ERR_UNSUPPORTED 1
	errUnsupported("readlink");
	return NULL;
#endif /* fs_readlink_USE_STUB */
}



















/************************************************************************/
/* Optional dependencies                                                */
/************************************************************************/

#ifdef WANT_DEEOBJECT_ASPATHHANDLEWITHWRITEATTRIBUTES
/* @return:  0: Successfully written the handle to `phandle'
 * @return:  1: Successfully written the handle to `phandle',
 *              but the caller must CloseHandle() it when they are done
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_AsPathHandleWithWriteAttributes(DeeObject *__restrict path,
                                          HANDLE *__restrict phandle) {
	int result;
	if (DeeString_Check(path)) {
again:
		*phandle = DeeNTSystem_CreateFile(path,
		                                  FILE_WRITE_ATTRIBUTES,
		                                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                                  NULL,
		                                  OPEN_EXISTING,
		                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
		                                  NULL);
		if (*phandle == INVALID_HANDLE_VALUE) {
			DWORD dwError;
			DBG_ALIGNMENT_DISABLE();
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again;
			} else if (DeeNTSystem_IsNotDir(dwError)) {
#define WANT_NT_ERRPATHNOTDIR 1
				nt_ErrPathNotDir(dwError, path);
			} else if (DeeNTSystem_IsFileNotFoundError(dwError)) {
#define WANT_NT_ERRFILENOTFOUND 1
				nt_ErrFileNotFound(dwError, path);
			} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define WANT_NT_ERRFILENOWRITEACCESS 1
				nt_ErrFileNotWritable(dwError, path);
			} else {
				DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
				                        "Failed to obtain a writable handle for %r",
				                        path);
			}
			goto err;
		}
		if (*phandle == NULL)
			goto err;
		return 1;
	}
	/* Load the file number of a file stream. */
	*phandle = DeeNTSystem_GetHandle(path);
	if (*phandle != INVALID_HANDLE_VALUE)
		return 0;
	if (!DeeError_Catch(&DeeError_AttributeError) &&
	    !DeeError_Catch(&DeeError_NotImplemented))
		goto err;
	/* Use the filename of a file stream. */
	path = DeeFile_Filename(path);
	if unlikely(!path)
		goto err;
	result = DeeObject_AsPathHandleWithWriteAttributes(path, phandle);
	Dee_Decref(path);
	return result;
err:
	return -1;
}
#endif /* WANT_DEEOBJECT_ASPATHHANDLEWITHWRITEATTRIBUTES */

#ifdef WANT_DEEOBJECT_ASFILETIME
/* Convert a given object into a windows FILETIME timestamp */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_AsFileTime(DeeObject *__restrict lpTime,
                     FILETIME *__restrict lpFileTime) {
	uint64_t value;
	if (DeeObject_AsUInt64(lpTime, &value))
		goto err;
	if (value < time_yer2day(1601) * MICROSECONDS_PER_DAY) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid file timestamp %k (must be located after 01.01.1601)",
		                lpTime);
		goto err;
	}
	value -= time_yer2day(1601) * MICROSECONDS_PER_DAY;
	UNALIGNED_SET64((uint64_t *)lpFileTime,
	                FILETIME_GET64(value *
	                               (FILETIME_PER_SECONDS /
	                                MICROSECONDS_PER_SECOND)));
	return 0;
err:
	return -1;
}
#endif /* WANT_DEEOBJECT_ASFILETIME */


#ifdef WANT_NT_ERRRMDIR
/* Handle system errors indicated by `nt_RemoveDirectory()' */
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrRmDir(DWORD dwError, DeeObject *__restrict path) {
#define WANT_NT_ERRPATHNOTEMPTY 1
	if (DeeNTSystem_IsNotEmpty(dwError))
		return nt_ErrPathNotEmpty(dwError, path);
#define WANT_NT_ERRPATHNOTWRITABLE 1
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return nt_ErrPathNotWritable(dwError, path);
#define WANT_NT_ERRPATHBUSY 1
	if (DeeNTSystem_IsBusy(dwError))
		return nt_ErrPathBusy(dwError, path);
#define WANT_NT_ERRPATHNOTFOUND 1
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return nt_ErrPathNotFound(dwError, path);
#define WANT_NT_ERRPATHNOTDIR 1
	if (DeeNTSystem_IsNotDir(dwError))
		return nt_ErrPathNotDir(dwError, path);
	if (DeeNTSystem_IsUnsupportedError(dwError)) {
		return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
		                               "The filesystem hosting the path %r does "
		                               "not support the removal of directories",
		                               path);
	}
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to remove directory %r",
	                               path);
}
#endif /* WANT_NT_ERRRMDIR */

#ifdef WANT_NT_ERRUNLINK
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrUnlink(DWORD dwError, DeeObject *__restrict path) {
	if (dwError == ERROR_ACCESS_DENIED) {
		DWORD dwAttributes;
		int temp;
		/* Check if the path is actually a directory. */
		temp = nt_GetFileAttributes(path, &dwAttributes);
		if unlikely(temp < 0)
			return temp;
#define WANT_NT_ERRPATHISDIR 1
		if (temp == 0 && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return nt_ErrPathIsDir(dwError, path);
	}
#define WANT_NT_ERRPATHNOTWRITABLE 1
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return nt_ErrPathNotWritable(dwError, path);
#define WANT_NT_ERRPATHBUSY 1
	if (DeeNTSystem_IsBusy(dwError))
		return nt_ErrPathBusy(dwError, path);
#define WANT_NT_ERRPATHNOTDIR 1
	if (DeeNTSystem_IsNotDir(dwError))
		return nt_ErrPathNotDir(dwError, path);
#define WANT_NT_ERRPATHNOTFOUND 1
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return nt_ErrPathNotFound(dwError, path);
	if (DeeNTSystem_IsUnsupportedError(dwError)) {
		return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
		                               "The filesystem hosting the path %r "
		                               "does not support unlinking files",
		                               path);
	}
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to unlink file %r",
	                               path);
}
#endif /* WANT_NT_ERRUNLINK */


#ifdef WANT_UNIX_ERRRMDIR
INTDEF ATTR_COLD NONNULL((2)) int DCALL
unix_ErrRmDir(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES) {
#if defined(EPERM) && (defined(WLSTAT) || defined(LSTAT))
no_access:
#endif /* EPERM && (WLSTAT || LSTAT) */
		DBG_ALIGNMENT_ENABLE();
#define WANT_UNIX_ERRPATHNOTWRITABLE 1
		return unix_ErrPathNotWritable(errno_value, path);
	} else
#endif /* EACCES */
#if defined(EBUSY) || defined(EINVAL)
#if defined(EBUSY) && defined(EINVAL)
	if (errno_value == EBUSY || errno_value == EINVAL)
#elif defined(EBUSY)
	if (errno_value == EBUSY)
#else
	if (errno_value == EINVAL)
#endif
	{
#define WANT_UNIX_ERRPATHBUSY 1
		return unix_ErrPathBusy(errno_value, path);
	} else
#endif /* EBUSY || EINVAL */
#ifdef ENOENT
	if (errno_value == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND 1
		return unix_ErrPathNotFound(errno_value, path);
	} else
#endif /* ENOENT */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR) {
#define WANT_UNIX_ERRPATHNOTDIR 1
		return unix_ErrPathNotDir(errno_value, path);
	} else
#endif /* ENOTDIR */
#ifdef ENOTEMPTY
	if (errno_value == ENOTEMPTY) {
#define WANT_UNIX_ERRPATHNOTEMPTY 1
		return unix_ErrPathNotEmpty(errno_value, path);
	} else
#endif /* ENOTEMPTY */
#ifdef EROFS
	if (errno_value == EROFS) {
#define WANT_UNIX_ERRPATHREADONLY 1
		return unix_ErrPathReadonly(errno_value, path);
	} else
#endif /* EROFS */
#ifdef EPERM
	if (errno_value == EPERM) {
#if defined(WLSTAT) || defined(LSTAT)
		/* Posix states that `EPERM' may be returned because of the sticky bit.
		 * However in that event, we want to throw an access error, not an
		 * unsupported-api error. */
#ifdef WLSTAT
		STRUCT_WSTAT st;
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!WLSTAT(wpath, &st) && (st.st_mode & STAT_ISVTX))
#else /* WLSTAT */
		STRUCT_STAT st;
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!LSTAT(utf8, &st) && (st.st_mode & STAT_ISVTX))
#endif /* !WLSTAT */
		{
#ifdef EACCES
			goto no_access;
#else /* EACCES */
			DBG_ALIGNMENT_ENABLE();
#define WANT_UNIX_ERRPATHNOTWRITABLE 1
			return unix_ErrPathNotWritable(errno_value, path);
#endif /* !EACCES */
		}
		DBG_ALIGNMENT_ENABLE();
#endif /* WLSTAT || LSTAT */
		return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
		                                 "The filesystem hosting the path %r does "
		                                 "not support the removal of directories",
		                                 path);
	} else
#endif /* EPERM */
	{
		return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
		                                 "Failed to remove directory %r",
		                                 path);
	}
}
#endif /* WANT_UNIX_ERRRMDIR */


#ifdef WANT_UNIX_ERRUNLINK
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrUnlink(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES) {
#if defined(EPERM) && (defined(WLSTAT) || defined(LSTAT))
no_access:
#endif /* EPERM && (WLSTAT || LSTAT) */
#define WANT_UNIX_ERRPATHNOTWRITABLE 1
		return unix_ErrPathNotWritable(errno_value, path);
	} else
#endif /* EACCES */
#ifdef EBUSY
	if (errno_value == EBUSY) {
#define WANT_UNIX_ERRPATHBUSY 1
		return unix_ErrPathBusy(errno_value, path);
	} else
#endif /* EBUSY */
#ifdef EISDIR
	if (errno_value == EISDIR) {
#if defined(EPERM) && (defined(WLSTAT) || defined(LSTAT))
err_isdir:
#endif /* EPERM && (WLSTAT || LSTAT) */
#define WANT_UNIX_ERRPATHISDIR 1
		return unix_ErrPathIsDir(errno_value, path);
	} else
#endif /* EISDIR */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR) {
#define WANT_UNIX_ERRPATHNOTDIR 1
		return unix_ErrPathNotDir(errno_value, path);
	} else
#endif /* ENOTDIR */
#ifdef ENOENT
	if (errno_value == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND 1
		return unix_ErrPathNotFound(errno_value, path);
	} else
#endif /* ENOENT */
#ifdef EPERM
	if (errno_value == EPERM) {
#if defined(WLSTAT) || defined(LSTAT)
		/* Posix states that this is the return value when the path is a directory,
		 * but also if the filesystem does not support unlinking files. */
#ifdef WLSTAT
		STRUCT_WSTAT st;
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!WLSTAT(wpath, &st))
#else /* WLSTAT */
		STRUCT_STAT st;
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!LSTAT(utf8, &st))
#endif /* !WLSTAT */
		{
			DBG_ALIGNMENT_ENABLE();
			if (STAT_ISDIR(st.st_mode)) {
#ifdef EISDIR
				goto err_isdir;
#else /* EISDIR */
#define WANT_UNIX_ERRPATHISDIR 1
				return unix_ErrPathIsDir(errno_value, path);
#endif /* !EISDIR */
			}
			/* Posix also states that the presence of the
			 * S_ISVTX bit may cause EPERM to be returned. */
			if (st.st_mode & STAT_ISVTX) {
#ifdef EACCES
				goto no_access;
#else /* EACCES */
#define WANT_UNIX_ERRPATHNOTWRITABLE 1
				return unix_ErrPathNotWritable(errno_value, path);
#endif /* !EACCES */
			}
		}
		DBG_ALIGNMENT_ENABLE();
#endif /* WLSTAT || LSTAT */
		return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
		                                 "The filesystem hosting the path %r "
		                                 "does not support unlinking files",
		                                 path);
	} else
#endif /* EPERM */
	{
		return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
		                                 "Failed to unlink file %r",
		                                 path);
	}
}
#endif /* WANT_UNIX_ERRUNLINK */


#ifdef WANT_UNIX_ERRREMOVE
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrRemove(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES) {
#if defined(EPERM) && (defined(WLSTAT) || defined(LSTAT))
no_access:
#endif /* EPERM && (WLSTAT || LSTAT) */
#define WANT_UNIX_ERRPATHNOTWRITABLE 1
		return unix_ErrPathNotWritable(errno_value, path);
	} else
#endif /* EACCES */
#if defined(EBUSY) || defined(EINVAL)
#if defined(EBUSY) && defined(EINVAL)
	if (errno_value == EBUSY || errno_value == EINVAL)
#elif defined(EBUSY)
	if (errno_value == EBUSY)
#else
	if (errno_value == EINVAL)
#endif
	{
#define WANT_UNIX_ERRPATHBUSY 1
		return unix_ErrPathBusy(errno_value, path);
	} else
#endif /* EBUSY || EINVAL */
#ifdef ENOENT
	if (errno_value == ENOENT) {
#define WANT_UNIX_ERRPATHNOTFOUND 1
		return unix_ErrPathNotFound(errno_value, path);
	} else
#endif /* ENOENT */
#ifdef ENOTEMPTY
	if (errno_value == ENOTEMPTY) {
#define WANT_UNIX_ERRPATHNOTEMPTY 1
		return unix_ErrPathNotEmpty(errno_value, path);
	} else
#endif /* ENOTEMPTY */
#ifdef EROFS
	if (errno_value == EROFS) {
#define WANT_UNIX_ERRPATHREADONLY 1
		return unix_ErrPathReadonly(errno_value, path);
	} else
#endif /* EROFS */
#ifdef EPERM
	if (errno_value == EPERM) {
#if defined(WLSTAT) || defined(LSTAT)
		/* Posix states that this is the return value when the path is a directory,
		 * but also if the filesystem does not support unlinking files. */
#ifdef WLSTAT
		STRUCT_WSTAT st;
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!WLSTAT(wpath, &st))
#else /* WLSTAT */
		STRUCT_STAT st;
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!LSTAT(utf8, &st))
#endif /* !WLSTAT */
		{
			DBG_ALIGNMENT_ENABLE();
			/* Posix also states that the presence of the
			 * S_ISVTX bit may cause EPERM to be returned. */
			if (st.st_mode & STAT_ISVTX) {
#ifdef EACCES
				goto no_access;
#else /* EACCES */
#define WANT_UNIX_ERRPATHNOTWRITABLE 1
				return unix_ErrPathNotWritable(errno_value, path);
#endif /* !EACCES */
			}
		}
		DBG_ALIGNMENT_ENABLE();
#endif /* WLSTAT || LSTAT */
		return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
		                                 "The filesystem hosting the path %r does "
		                                 "not support removing files or directories",
		                                 path);
	} else
#endif /* EPERM */
	{
		return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
		                                 "Failed to remove file or directory %r",
		                                 path);
	}
}
#endif /* WANT_UNIX_ERRREMOVE */


#ifdef WANT_NT_ERRPATHEXISTS
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathExists(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileExists, dwError,
	                               "Path %r already exists",
	                               path);
}
#endif /* WANT_NT_ERRPATHEXISTS */

#ifdef WANT_NT_ERRPATHNOTDIR
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathNotDir(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_NoDirectory, dwError,
	                               "Some part of the path %r is not a directory",
	                               path);
}
#endif /* WANT_NT_ERRPATHNOTDIR */

#ifdef WANT_NT_ERRPATHISDIR
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathIsDir(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_IsDirectory, dwError,
	                               "Path %r is a directory",
	                               path);
}
#endif /* WANT_NT_ERRPATHISDIR */

#ifdef WANT_NT_ERRPATHREADONLY
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathReadonly(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_ReadOnlyFile, dwError,
	                               "Path %r is apart of a read-only filesystem",
	                               path);
}
#endif /* WANT_NT_ERRPATHREADONLY */

#ifdef WANT_NT_ERRPATHNOTFOUND
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathNotFound(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
	                               "Path %r could not be found",
	                               path);
}
#endif /* WANT_NT_ERRPATHNOTFOUND */

#ifdef WANT_NT_ERRFILENOTFOUND
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrFileNotFound(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
	                               "File %r could not be found",
	                               path);
}
#endif /* WANT_NT_ERRFILENOTFOUND */

#ifdef WANT_NT_ERRFILENOWRITEACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrFileNotWritable(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Write permissions have not been granted for file %r",
	                               path);
}
#endif /* WANT_NT_ERRFILENOWRITEACCESS */

#ifdef WANT_NT_ERRPATHNOTWRITABLE
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathNotWritable(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Write permissions have not been granted for path %r",
	                               path);
}
#endif /* WANT_NT_ERRPATHNOTWRITABLE */

#ifdef WANT_NT_ERRPATHBUSY
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathBusy(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_BusyFile, dwError,
	                               "Path %r cannot be deleted because it is still in use",
	                               path);
}
#endif /* WANT_NT_ERRPATHBUSY */

#ifdef WANT_NT_ERRPATHNOACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathNoAccess(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Search permissions are not granted for path %r",
	                               path);
}
#endif /* WANT_NT_ERRPATHNOACCESS */

#ifdef WANT_NT_ERRPATHNOTEMPTY
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrPathNotEmpty(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_NotEmpty, dwError,
	                               "The directory %r cannot be deleted because it is not empty",
	                               path);
}
#endif /* WANT_NT_ERRPATHNOTEMPTY */

#ifdef WANT_NT_ERRCHTIMENOACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrChtimeNoAccess(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Changes to the selected timestamps of %r are not allowed",
	                               path);
}
#endif /* WANT_NT_ERRCHTIMENOACCESS */

#ifdef WANT_NT_ERRCHATTRNOACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrChattrNoAccess(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Changes to the attributes of %r are not allowed",
	                               path);
}
#endif /* WANT_NT_ERRCHATTRNOACCESS */

#ifdef WANT_NT_ERRHANDLECLOSED
INTERN ATTR_COLD NONNULL((2)) int DCALL
nt_ErrHandleClosed(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileClosed, dwError,
	                               "The given handle %r has been closed",
	                               path);
}
#endif /* WANT_NT_ERRHANDLECLOSED */

#ifdef WANT_NT_ERRPATHNOACCESS2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
nt_ErrPathNoAccess2(DWORD dwError,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Access to %r or %r has not been granted",
	                               existing_path, new_path);
}
#endif /* WANT_NT_ERRPATHNOACCESS2 */

#ifdef WANT_NT_ERRPATHNOTFOUND2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
nt_ErrPathNotFound2(DWORD dwError,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
	                               "Path %r or %r could not be found",
	                               existing_path, new_path);
}
#endif /* WANT_NT_ERRPATHNOTFOUND2 */

#ifdef WANT_NT_ERRPATHCROSSDEV2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
nt_ErrPathCrossDev2(DWORD dwError,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_CrossDevice, dwError,
	                               "Paths %r and %r are not apart of the same filesystem",
	                               existing_path, new_path);
}
#endif /* WANT_NT_ERRPATHCROSSDEV2 */

#ifdef WANT_UNIX_ERRPATHEXISTS
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathExists(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileExists, errno_value,
	                                 "Path %r already exists",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHEXISTS */

#ifdef WANT_UNIX_ERRPATHNOTDIR
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathNotDir(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_NoDirectory, errno_value,
	                                 "Some part of the path %r is not a directory",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHNOTDIR */

#ifdef WANT_UNIX_ERRPATHISDIR
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathIsDir(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_IsDirectory, errno_value,
	                                 "Path %r is a directory",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHISDIR */

#ifdef WANT_UNIX_ERRPATHREADONLY
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathReadonly(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, errno_value,
	                                 "Path %r is apart of a read-only filesystem",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHREADONLY */

#ifdef WANT_UNIX_ERRPATHNOTFOUND
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathNotFound(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "Path %r could not be found",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHNOTFOUND */

#ifdef WANT_UNIX_ERRFILENOTFOUND
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrFileNotFound(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "File %r could not be found",
	                                 path);
}
#endif /* WANT_UNIX_ERRFILENOTFOUND */

#ifdef WANT_UNIX_ERRFILENOWRITEACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrFileNotWritable(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Write permissions have not been granted for file %r",
	                                 path);
}
#endif /* WANT_UNIX_ERRFILENOWRITEACCESS */

#ifdef WANT_UNIX_ERRPATHNOTWRITABLE
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathNotWritable(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Write permissions have not been granted for path %r",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHNOTWRITABLE */

#ifdef WANT_UNIX_ERRPATHBUSY
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathBusy(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_BusyFile, errno_value,
	                                 "Path %r cannot be deleted because it is still in use",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHBUSY */

#ifdef WANT_UNIX_ERRPATHNOACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathNoAccess(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Search permissions are not granted for path %r",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHNOACCESS */

#ifdef WANT_UNIX_ERRPATHNOTEMPTY
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrPathNotEmpty(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_NotEmpty, errno_value,
	                                 "The directory %r cannot be deleted because it is not empty",
	                                 path);
}
#endif /* WANT_UNIX_ERRPATHNOTEMPTY */

#ifdef WANT_UNIX_ERRCHTIMENOACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrChtimeNoAccess(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Changes to the selected timestamps of %r are not allowed",
	                                 path);
}
#endif /* WANT_UNIX_ERRCHTIMENOACCESS */

#ifdef WANT_UNIX_ERRCHATTRNOACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrChtimeNoAccess(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Changes to the attributes of %r are not allowed",
	                                 path);
}
#endif /* WANT_UNIX_ERRCHATTRNOACCESS */

#ifdef WANT_UNIX_ERRHANDLECLOSED
INTERN ATTR_COLD NONNULL((2)) int DCALL
unix_ErrHandleClosed(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileClosed, errno_value,
	                                 "The given handle %r has been closed",
	                                 path);
}
#endif /* WANT_UNIX_ERRHANDLECLOSED */

#ifdef WANT_UNIX_ERRPATHREADONLY2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
unix_ErrPathReadonly2(int errno_value,
                      DeeObject *__restrict existing_path,
                      DeeObject *__restrict new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Access to %r or %r has not been granted",
	                                 existing_path, new_path);
}
#endif /* WANT_UNIX_ERRPATHREADONLY2 */

#ifdef WANT_UNIX_ERRPATHNOTFOUND2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
unix_ErrPathNotFound2(int errno_value,
                      DeeObject *__restrict existing_path,
                      DeeObject *__restrict new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "Path %r or %r could not be found",
	                                 existing_path, new_path);
}
#endif /* WANT_UNIX_ERRPATHNOTFOUND2 */

#ifdef WANT_UNIX_ERRPATHCROSSDEV2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
unix_ErrPathCrossDev2(int errno_value,
                      DeeObject *__restrict existing_path,
                      DeeObject *__restrict new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_CrossDevice, errno_value,
	                                 "Paths %r and %r are not apart of the same filesystem",
	                                 existing_path, new_path);
}
#endif /* WANT_UNIX_ERRPATHCROSSDEV2 */

#ifdef WANT_ERR_UNSUPPORTED
INTERN ATTR_COLD NONNULL((1)) int DCALL
errUnsupported(char const *__restrict name) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unsupported function: %s",
	                       name);
}
#endif /* WANT_ERR_UNSUPPORTED */



DECL_END

#endif /* !GUARD_DEX_FS_OSFS_C */
