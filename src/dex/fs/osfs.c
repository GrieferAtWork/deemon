/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FS_OSFS_C
#define GUARD_DEX_FS_OSFS_C 1

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

#ifndef CONFIG_HAVE_strend
#define CONFIG_HAVE_strend 1
#define strend(x) ((x) + strlen(x))
#endif /* !CONFIG_HAVE_strend */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#include <string.h>
#include <wchar.h>
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


DECL_BEGIN

#undef WANT_ERR_PATH_EXISTS
#undef WANT_ERR_PATH_NO_DIR
#undef WANT_ERR_PATH_IS_DIR
#undef WANT_ERR_PATH_READONLY
#undef WANT_ERR_PATH_NOT_FOUND
#undef WANT_ERR_FILE_NOT_FOUND
#undef WANT_ERR_FILE_NO_WRITE_ACCESS
#undef WANT_ERR_PATH_NO_WRITE_ACCESS
#undef WANT_ERR_PATH_BUSY
#undef WANT_ERR_PATH_NO_ACCESS
#undef WANT_ERR_PATH_NOT_EMPTY
#undef WANT_ERR_CHTIME_NO_ACCESS
#undef WANT_ERR_CHATTR_NO_ACCESS
#undef WANT_ERR_HANDLE_CLOSED
#undef WANT_ERR_PATH_NO_ACCESS2
#undef WANT_ERR_PATH_NOT_FOUND2
#undef WANT_ERR_PATH_CROSSDEV2
#undef WANT_ERR_UNSUPPORTED

INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_exists(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_no_dir(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_is_dir(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_readonly(int error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_not_found(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_file_not_found(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_file_no_write_access(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_no_write_access(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_busy(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_no_access(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_path_not_empty(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_chtime_no_access(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_chattr_no_access(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_handle_closed(Dee_syserrno_t error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_path_no_access2(Dee_syserrno_t error, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_path_not_found2(Dee_syserrno_t error, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_path_crossdev2(Dee_syserrno_t error, DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unsupported(char const *__restrict name);



#ifdef CONFIG_HOST_WINDOWS
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
#define WANT_ERR_HANDLE_CLOSED 1
			err_handle_closed(dwError, path);
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define WANT_ERR_CHTIME_NO_ACCESS 1
			err_chtime_no_access(dwError, path);
		} else if (DeeNTSystem_IsUnsupportedError(dwError)) {
			DeeError_SysThrowf(&DeeError_UnsupportedAPI, (DWORD)error,
			                   "The filesystem hosting the path %r does "
			                   "not support the changing of time stamps",
			                   path);
		} else {
			DeeError_SysThrowf(&DeeError_FSError, dwError,
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
	return err_unsupported("chtime");
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


/* Change timestamps for a given file. */
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
#define WANT_ERR_CHATTR_NO_ACCESS 1
		err_chattr_no_access((DWORD)error, path);
	} else if (DeeNTSystem_IsUnsupportedError((DWORD)error)) {
		DeeError_SysThrowf(&DeeError_UnsupportedAPI, (DWORD)error,
		                   "The filesystem hosting the path %r does "
		                   "not support the changing of NT attributes",
		                   path);
	} else {
		DeeError_SysThrowf(&DeeError_FSError, (DWORD)error,
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
	return err_unsupported("chmod");
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


/* Change timestamps for a given file. */
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
	return err_unsupported("lchmod");
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


/* Change timestamps for a given file. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fs_chown(DeeObject *__restrict path,
         DeeObject *__restrict user,
         DeeObject *__restrict group) {
#ifdef fs_chown_USE_STUB
	(void)path;
	(void)user;
	(void)group;
#define WANT_ERR_UNSUPPORTED 1
	return err_unsupported("chown");
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


/* Change timestamps for a given file. */
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
	return err_unsupported("lchown");
#endif /* fs_lchown_USE_STUB */
}



/************************************************************************/
/* fs_mkdir()                                                           */
/************************************************************************/
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


/* Change timestamps for a given file. */
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
#define WANT_ERR_PATH_NO_WRITE_ACCESS 1
		err_path_no_write_access((DWORD)error, path);
	} else if (DeeNTSystem_IsExists((DWORD)error)) {
#define WANT_ERR_PATH_EXISTS 1
		err_path_exists((DWORD)error, path);
	} else if (DeeNTSystem_IsNotDir((DWORD)error)) {
#define WANT_ERR_PATH_NO_DIR 1
		err_path_no_dir((DWORD)error, path);
	} else if (DeeNTSystem_IsUnsupportedError((DWORD)error)) {
		DeeError_SysThrowf(&DeeError_UnsupportedAPI, (DWORD)error,
		                   "The filesystem hosting the path %r does "
		                   "not support the creation of directories",
		                   path);
	} else {
		DeeError_SysThrowf(&DeeError_FSError, (DWORD)error,
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
#define WANT_ERR_PATH_NO_WRITE_ACCESS 1
			err_path_no_write_access(error, path);
		} else
#endif /* EACCES */
#ifdef EEXIST
		if (error == EEXIST) {
#define WANT_ERR_PATH_EXISTS 1
			err_path_exists(error, path);
		} else
#endif /* EEXIST */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
#define WANT_ERR_PATH_NO_DIR 1
			err_path_no_dir(error, path);
		} else
#endif /* ENOTDIR */
#ifdef EROFS
		if (error == EROFS) {
#define WANT_ERR_PATH_READONLY 1
			err_path_readonly(error, path);
		} else
#endif /* EROFS */
#ifdef EPERM
		if (error == EPERM) {
			DeeError_SysThrowf(&DeeError_UnsupportedAPI, error,
			                   "The filesystem hosting the path %r does "
			                   "not support the creation of directories",
			                   path);
		} else
#endif /* EPERM */
		{
			DeeError_SysThrowf(&DeeError_FSError, error,
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
	return err_unsupported("mkdir");
#endif /* fs_mkdir_USE_STUB */
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
#define WANT_ERR_PATH_NO_DIR 1
				err_path_no_dir(dwError, path);
			} else if (DeeNTSystem_IsFileNotFoundError(dwError)) {
#define WANT_ERR_FILE_NOT_FOUND 1
				err_file_not_found(dwError, path);
			} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define WANT_ERR_FILE_NO_WRITE_ACCESS 1
				err_file_no_write_access(dwError, path);
			} else {
				DeeError_SysThrowf(&DeeError_FSError, dwError,
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




#ifdef WANT_ERR_PATH_EXISTS
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_exists(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileExists, error,
	                          "Path %r already exists",
	                          path);
}
#endif /* WANT_ERR_PATH_EXISTS */

#ifdef WANT_ERR_PATH_NO_DIR
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_no_dir(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_NoDirectory, error,
	                          "Some part of the path %r is not a directory",
	                          path);
}
#endif /* WANT_ERR_PATH_NO_DIR */

#ifdef WANT_ERR_PATH_IS_DIR
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_is_dir(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_IsDirectory, error,
	                          "Path %r is a directory",
	                          path);
}
#endif /* WANT_ERR_PATH_IS_DIR */

#ifdef WANT_ERR_PATH_READONLY
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_readonly(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_ReadOnlyFile, error,
	                          "Path %r is apart of a read-only filesystem",
	                          path);
}
#endif /* WANT_ERR_PATH_READONLY */

#ifdef WANT_ERR_PATH_NOT_FOUND
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_not_found(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileNotFound, error,
	                          "Path %r could not be found",
	                          path);
}
#endif /* WANT_ERR_PATH_NOT_FOUND */

#ifdef WANT_ERR_FILE_NOT_FOUND
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_file_not_found(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileNotFound, error,
	                          "File %r could not be found",
	                          path);
}
#endif /* WANT_ERR_FILE_NOT_FOUND */

#ifdef WANT_ERR_FILE_NO_WRITE_ACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_file_no_write_access(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Write permissions have not been granted for file %r",
	                          path);
}
#endif /* WANT_ERR_FILE_NO_WRITE_ACCESS */

#ifdef WANT_ERR_PATH_NO_WRITE_ACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_no_write_access(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Write permissions have not been granted for path %r",
	                          path);
}
#endif /* WANT_ERR_PATH_NO_WRITE_ACCESS */

#ifdef WANT_ERR_PATH_BUSY
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_busy(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_BusyFile, error,
	                          "Path %r cannot be deleted because it is still in use",
	                          path);
}
#endif /* WANT_ERR_PATH_BUSY */

#ifdef WANT_ERR_PATH_NO_ACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_no_access(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Search permissions are not granted for path %r",
	                          path);
}
#endif /* WANT_ERR_PATH_NO_ACCESS */

#ifdef WANT_ERR_PATH_NOT_EMPTY
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_path_not_empty(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_NotEmpty, error,
	                          "The directory %r cannot be deleted because it is not empty",
	                          path);
}
#endif /* WANT_ERR_PATH_NOT_EMPTY */

#ifdef WANT_ERR_CHTIME_NO_ACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_chtime_no_access(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Changes to the selected timestamps of %r are not allowed",
	                          path);
}
#endif /* WANT_ERR_CHTIME_NO_ACCESS */

#ifdef WANT_ERR_CHATTR_NO_ACCESS
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_chattr_no_access(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Changes to the attributes of %r are not allowed",
	                          path);
}
#endif /* WANT_ERR_CHATTR_NO_ACCESS */

#ifdef WANT_ERR_HANDLE_CLOSED
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_handle_closed(Dee_syserrno_t error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileClosed, error,
	                          "The given handle %r has been closed",
	                          path);
}
#endif /* WANT_ERR_HANDLE_CLOSED */

#ifdef WANT_ERR_PATH_NO_ACCESS2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_path_no_access2(Dee_syserrno_t error,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Access to %r or %r has not been granted",
	                          existing_path, new_path);
}
#endif /* WANT_ERR_PATH_NO_ACCESS2 */

#ifdef WANT_ERR_PATH_NOT_FOUND2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_path_not_found2(Dee_syserrno_t error,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
	return DeeError_SysThrowf(&DeeError_FileNotFound, error,
	                          "Path %r or %r could not be found",
	                          existing_path, new_path);
}
#endif /* WANT_ERR_PATH_NOT_FOUND2 */

#ifdef WANT_ERR_PATH_CROSSDEV2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_path_crossdev2(Dee_syserrno_t error,
                   DeeObject *__restrict existing_path,
                   DeeObject *__restrict new_path) {
	return DeeError_SysThrowf(&DeeError_CrossDevice, error,
	                          "Paths %r and %r are not apart of the same filesystem",
	                          existing_path, new_path);
}
#endif /* WANT_ERR_PATH_CROSSDEV2 */

#ifdef WANT_ERR_UNSUPPORTED
INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unsupported(char const *__restrict name) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unsupported function: %s",
	                       name);
}
#endif /* WANT_ERR_UNSUPPORTED */



DECL_END

#endif /* !GUARD_DEX_FS_OSFS_C */
