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
#ifndef GUARD_DEX_POSIX_P_ONDEMAND_C_INL
#define GUARD_DEX_POSIX_P_ONDEMAND_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

#ifdef __INTELLISENSE__
#define NEED_err_unix_path_not_dir
#define NEED_err_nt_path_not_dir
#define NEED_err_unix_path_not_found
#define NEED_err_nt_path_not_found
#define NEED_err_unix_path_not_found2
#define NEED_err_nt_path_not_found2
#define NEED_err_unix_path_no_access
#define NEED_err_nt_path_no_access
#define NEED_err_unix_path_no_access2
#define NEED_err_nt_path_no_access2
#define NEED_err_unix_chattr_no_access
#define NEED_err_nt_chattr_no_access
#define NEED_err_unix_handle_closed
#define NEED_err_nt_handle_closed
#define NEED_err_unix_path_exists
#define NEED_err_nt_path_exists
#define NEED_err_unix_path_is_dir
#define NEED_err_nt_path_is_dir
#define NEED_err_unix_path_read_only
#define NEED_err_nt_path_readonly
#define NEED_err_unix_file_not_found
#define NEED_err_nt_file_not_found
#define NEED_err_unix_file_not_writable
#define NEED_err_nt_file_not_writable
#define NEED_err_unix_path_not_writable
#define NEED_err_nt_path_not_writable
#define NEED_err_unix_path_busy
#define NEED_err_nt_path_busy
#define NEED_err_unix_path_not_empty
#define NEED_err_nt_path_not_empty
#define NEED_err_unix_chtime_no_access
#define NEED_err_nt_chtime_no_access
#define NEED_err_unix_path_cross_dev2
#define NEED_err_nt_path_cross_dev2
#define NEED_nt_GetTempPath
#define NEED_nt_GetComputerName
#define NEED_nt_SetCurrentDirectory
#define NEED_nt_GetFileAttributesEx
#define NEED_nt_GetFileAttributes
#define NEED_nt_SetFileAttributes
#define NEED_nt_CreateDirectory
#define NEED_nt_RemoveDirectory
#define NEED_nt_DeleteFile
#define NEED_nt_MoveFile
#define NEED_nt_CreateHardLink
#define NEED_nt_CreateSymbolicLink
#define NEED_posix_dfd_abspath
#define NEED_posix_fd_abspath
#define NEED_err_bad_atflags
#define NEED_posix_err_unsupported
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#ifdef NEED_err_unix_path_not_dir
#undef NEED_err_unix_path_not_dir
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_not_dir(int error, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_NoDirectory, error,
	                                 "Some part of the path %r is not a directory",
	                                 path);
}
#endif /* NEED_err_unix_path_not_dir */

#ifdef NEED_err_nt_path_not_dir
#undef NEED_err_nt_path_not_dir
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_not_dir(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_NoDirectory, dwError,
	                               "Some part of the path %r is not a directory",
	                               path);
}
#endif /* NEED_err_nt_path_not_dir */

#ifdef NEED_err_unix_path_not_found
#undef NEED_err_unix_path_not_found
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_not_found(int error, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error,
	                                 "Path %r could not be found",
	                                 path);
}
#endif /* NEED_err_unix_path_not_found */

#ifdef NEED_err_nt_path_not_found
#undef NEED_err_nt_path_not_found
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_not_found(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
	                               "Path %r could not be found",
	                               path);
}
#endif /* NEED_err_nt_path_not_found */

#ifdef NEED_err_unix_path_not_found2
#undef NEED_err_unix_path_not_found2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_path_not_found2(int errno_value,
                         DeeObject *__restrict existing_path,
                         DeeObject *__restrict new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "Path %r or %r could not be found",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_not_found2 */

#ifdef NEED_err_nt_path_not_found2
#undef NEED_err_nt_path_not_found2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_not_found2(DWORD dwError,
                       DeeObject *__restrict existing_path,
                       DeeObject *__restrict new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
	                               "Path %r or %r could not be found",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_path_not_found2 */

#ifdef NEED_err_unix_path_no_access
#undef NEED_err_unix_path_no_access
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_no_access(int error, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error,
	                                 "Search permissions are not granted for path %r",
	                                 path);
}
#endif /* NEED_err_unix_path_no_access */

#ifdef NEED_err_nt_path_no_access
#undef NEED_err_nt_path_no_access
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_no_access(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Search permissions are not granted for path %r",
	                               path);
}
#endif /* NEED_err_nt_path_no_access */

#ifdef NEED_err_unix_path_no_access2
#undef NEED_err_unix_path_no_access2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_path_no_access2(int errno_value,
                         DeeObject *__restrict existing_path,
                         DeeObject *__restrict new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Access to %r or %r has not been granted",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_no_access2 */

#ifdef NEED_err_nt_path_no_access2
#undef NEED_err_nt_path_no_access2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_no_access2(DWORD dwError,
                       DeeObject *__restrict existing_path,
                       DeeObject *__restrict new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Access to %r or %r has not been granted",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_path_no_access2 */

#ifdef NEED_err_unix_chattr_no_access
#undef NEED_err_unix_chattr_no_access
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_chattr_no_access(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Changes to the attributes of %r are not allowed",
	                                 path);
}
#endif /* NEED_err_unix_chattr_no_access */

#ifdef NEED_err_nt_chattr_no_access
#undef NEED_err_nt_chattr_no_access
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_chattr_no_access(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Changes to the attributes of %r are not allowed",
	                               path);
}
#endif /* NEED_err_nt_chattr_no_access */

#ifdef NEED_err_unix_handle_closed
#undef NEED_err_unix_handle_closed
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_handle_closed(int error, DeeObject *__restrict handle) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileClosed, error,
	                                 "The given handle %r has been closed",
	                                 handle);
}
#endif /* NEED_err_unix_handle_closed */

#ifdef NEED_err_nt_handle_closed
#undef NEED_err_nt_handle_closed
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_handle_closed(DWORD dwError, DeeObject *__restrict handle) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileClosed, dwError,
	                               "The given handle %r has been closed",
	                               handle);
}
#endif /* NEED_err_nt_handle_closed */

#ifdef NEED_err_unix_path_exists
#undef NEED_err_unix_path_exists
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_exists(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileExists, errno_value,
	                                 "Path %r already exists",
	                                 path);
}
#endif /* NEED_err_unix_path_exists */

#ifdef NEED_err_nt_path_exists
#undef NEED_err_nt_path_exists
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_exists(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileExists, dwError,
	                               "Path %r already exists",
	                               path);
}
#endif /* NEED_err_nt_path_exists */

#ifdef NEED_err_unix_path_is_dir
#undef NEED_err_unix_path_is_dir
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_is_dir(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_IsDirectory, errno_value,
	                                 "Path %r is a directory",
	                                 path);
}
#endif /* NEED_err_unix_path_is_dir */

#ifdef NEED_err_nt_path_is_dir
#undef NEED_err_nt_path_is_dir
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_is_dir(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_IsDirectory, dwError,
	                               "Path %r is a directory",
	                               path);
}
#endif /* NEED_err_nt_path_is_dir */

#ifdef NEED_err_unix_path_read_only
#undef NEED_err_unix_path_read_only
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_read_only(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, errno_value,
	                                 "Path %r is apart of a read-only filesystem",
	                                 path);
}
#endif /* NEED_err_unix_path_read_only */

#ifdef NEED_err_nt_path_readonly
#undef NEED_err_nt_path_readonly
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_readonly(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_ReadOnlyFile, dwError,
	                               "Path %r is apart of a read-only filesystem",
	                               path);
}
#endif /* NEED_err_nt_path_readonly */

#ifdef NEED_err_unix_file_not_found
#undef NEED_err_unix_file_not_found
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_file_not_found(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "File %r could not be found",
	                                 path);
}
#endif /* NEED_err_unix_file_not_found */

#ifdef NEED_err_nt_file_not_found
#undef NEED_err_nt_file_not_found
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_file_not_found(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
	                               "File %r could not be found",
	                               path);
}
#endif /* NEED_err_nt_file_not_found */

#ifdef NEED_err_unix_file_not_writable
#undef NEED_err_unix_file_not_writable
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_file_not_writable(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Write permissions have not been granted for file %r",
	                                 path);
}
#endif /* NEED_err_unix_file_not_writable */

#ifdef NEED_err_nt_file_not_writable
#undef NEED_err_nt_file_not_writable
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_file_not_writable(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Write permissions have not been granted for file %r",
	                               path);
}
#endif /* NEED_err_nt_file_not_writable */

#ifdef NEED_err_unix_path_not_writable
#undef NEED_err_unix_path_not_writable
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_not_writable(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Write permissions have not been granted for path %r",
	                                 path);
}
#endif /* NEED_err_unix_path_not_writable */

#ifdef NEED_err_nt_path_not_writable
#undef NEED_err_nt_path_not_writable
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_not_writable(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Write permissions have not been granted for path %r",
	                               path);
}
#endif /* NEED_err_nt_path_not_writable */

#ifdef NEED_err_unix_path_busy
#undef NEED_err_unix_path_busy
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_busy(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_BusyFile, errno_value,
	                                 "Path %r cannot be deleted because it is still in use",
	                                 path);
}
#endif /* NEED_err_unix_path_busy */

#ifdef NEED_err_nt_path_busy
#undef NEED_err_nt_path_busy
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_busy(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_BusyFile, dwError,
	                               "Path %r cannot be deleted because it is still in use",
	                               path);
}
#endif /* NEED_err_nt_path_busy */

#ifdef NEED_err_unix_path_not_empty
#undef NEED_err_unix_path_not_empty
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_not_empty(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_DirectoryNotEmpty, errno_value,
	                                 "The directory %r cannot be deleted because it is not empty",
	                                 path);
}
#endif /* NEED_err_unix_path_not_empty */

#ifdef NEED_err_nt_path_not_empty
#undef NEED_err_nt_path_not_empty
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_not_empty(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_DirectoryNotEmpty, dwError,
	                               "The directory %r cannot be deleted because it is not empty",
	                               path);
}
#endif /* NEED_err_nt_path_not_empty */

#ifdef NEED_err_unix_chtime_no_access
#undef NEED_err_unix_chtime_no_access
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_chtime_no_access(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Changes to the selected timestamps of %r are not allowed",
	                                 path);
}
#endif /* NEED_err_unix_chtime_no_access */

#ifdef NEED_err_nt_chtime_no_access
#undef NEED_err_nt_chtime_no_access
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_chtime_no_access(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
	                               "Changes to the selected timestamps of %r are not allowed",
	                               path);
}
#endif /* NEED_err_nt_chtime_no_access */

#ifdef NEED_err_unix_path_cross_dev2
#undef NEED_err_unix_path_cross_dev2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_path_cross_dev2(int errno_value,
                         DeeObject *__restrict existing_path,
                         DeeObject *__restrict new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_CrossDeviceLink, errno_value,
	                                 "Paths %r and %r are not apart of the same filesystem",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_cross_dev2 */

#ifdef NEED_err_nt_path_cross_dev2
#undef NEED_err_nt_path_cross_dev2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_cross_dev2(DWORD dwError,
                       DeeObject *__restrict existing_path,
                       DeeObject *__restrict new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_CrossDeviceLink, dwError,
	                               "Paths %r and %r are not apart of the same filesystem",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_path_cross_dev2 */




#ifdef NEED_nt_GetTempPath
#undef NEED_nt_GetTempPath
INTERN WUNUSED DREF DeeObject *DCALL
nt_GetTempPath(void) {
	LPWSTR buffer, new_buffer;
	DWORD bufsize = 256, error;
	buffer = DeeString_NewWideBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		error = GetTempPathW(bufsize + 1, buffer);
		if (!error) {
			/* Error. */
			error = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, error,
			                        "Failed to lookup the path for tmp");
			goto err_result;
		}
		DBG_ALIGNMENT_ENABLE();
		if (error <= bufsize)
			break;
		/* Resize to fit. */
		new_buffer = DeeString_ResizeWideBuffer(buffer, error);
		if unlikely(!new_buffer)
			goto err_result;
		buffer  = new_buffer;
		bufsize = error;
	}
	buffer = DeeString_TruncateWideBuffer(buffer, error);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}
#endif /* NEED_nt_GetTempPath */


#ifdef NEED_nt_GetComputerName
#undef NEED_nt_GetComputerName
INTERN WUNUSED DREF DeeObject *DCALL
nt_GetComputerName(void) {
	DWORD bufsize = MAX_COMPUTERNAME_LENGTH + 1;
	LPWSTR buffer, new_buffer;
	if (DeeThread_CheckInterrupt())
		goto err;
	buffer = DeeString_NewWideBuffer(bufsize - 1);
	if unlikely(!buffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	if (!GetComputerNameW(buffer, &bufsize)) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError == ERROR_BUFFER_OVERFLOW && bufsize &&
		    bufsize - 1 > WSTR_LENGTH(buffer)) {
			new_buffer = DeeString_ResizeWideBuffer(buffer, bufsize - 1);
			if unlikely(!new_buffer)
				goto err_result;
			buffer = new_buffer;
			goto again;
		}
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err_result;
		}
		DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
		                        "Failed to retrieve the name of the hosting machine");
		goto err_result;
	}
	DBG_ALIGNMENT_ENABLE();
	/* Truncate the buffer and return it. */
	buffer = DeeString_TruncateWideBuffer(buffer, bufsize);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}
#endif /* NEED_nt_GetComputerName */


#ifdef NEED_nt_SetCurrentDirectory
#undef NEED_nt_SetCurrentDirectory
/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_SetCurrentDirectory(DeeObject *__restrict lpPathName) {
	LPWSTR wname;
	BOOL result;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	result = SetCurrentDirectoryW(wname);
	if (!result && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = DeeNTSystem_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!wname) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		result  = SetCurrentDirectoryW(wname);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !result;
err:
	return -1;
}
#endif /* NEED_nt_SetCurrentDirectory */


#ifdef NEED_nt_GetFileAttributesEx
#undef NEED_nt_GetFileAttributesEx
/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation) {
	LPWSTR wname;
	BOOL result;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	result = GetFileAttributesExW(wname, fInfoLevelId, lpFileInformation);
	if (!result && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		result  = GetFileAttributesExW(wname, fInfoLevelId, lpFileInformation);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !result;
err:
	return -1;
}
#endif /* NEED_nt_GetFileAttributesEx */

#ifdef NEED_nt_GetFileAttributes
#undef NEED_nt_GetFileAttributes
/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict presult) {
	LPWSTR wname;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	*presult = GetFileAttributesW(wname);
	if ((*presult == INVALID_FILE_ATTRIBUTES) && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		*presult = GetFileAttributesW(wname);
		dwError  = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return *presult == INVALID_FILE_ATTRIBUTES;
err:
	return -1;
}
#endif /* NEED_nt_GetFileAttributes */


#ifdef NEED_nt_SetFileAttributes
#undef NEED_nt_SetFileAttributes
/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = SetFileAttributesW(wname, dwFileAttributes);
	if (!error && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = SetFileAttributesW(wname, dwFileAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}
#endif /* NEED_nt_SetFileAttributes */


#ifdef NEED_nt_CreateDirectory
#undef NEED_nt_CreateDirectory
/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = CreateDirectoryW(wname, lpSecurityAttributes);
	if (!error && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = DeeNTSystem_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!wname) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = CreateDirectoryW(wname, lpSecurityAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}
#endif /* NEED_nt_CreateDirectory */

#ifdef NEED_nt_RemoveDirectory
#undef NEED_nt_RemoveDirectory
/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = RemoveDirectoryW(wname);
	if (!error && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = DeeNTSystem_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!wname) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = RemoveDirectoryW(wname);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}
#endif /* NEED_nt_RemoveDirectory */

#ifdef NEED_nt_DeleteFile
#undef NEED_nt_DeleteFile
/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = DeleteFileW(wname);
	if (!error && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = DeleteFileW(wname);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}
#endif /* NEED_nt_DeleteFile */

#ifdef NEED_nt_MoveFile
#undef NEED_nt_MoveFile
/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
nt_MoveFile(DeeObject *__restrict lpExistingFileName,
            DeeObject *__restrict lpNewFileName) {
	LPWSTR wExistingFileName, wNewFileName;
	BOOL error;
	DWORD dwError;
	wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
	if unlikely(!wExistingFileName)
		goto err;
	wNewFileName = (LPWSTR)DeeString_AsWide(lpNewFileName);
	if unlikely(!wNewFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = MoveFileW(wExistingFileName, wNewFileName);
	if (!error && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpExistingFileName = DeeNTSystem_FixUncPath(lpExistingFileName);
		if unlikely(!lpExistingFileName)
			goto err;
		lpNewFileName = DeeNTSystem_FixUncPath(lpNewFileName);
		if unlikely(!lpNewFileName)
			goto err_existing;
		wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
		if unlikely(!wExistingFileName)
			goto err_new;
		wNewFileName = (LPWSTR)DeeString_AsWide(lpNewFileName);
		if unlikely(!wNewFileName)
			goto err_new;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		error   = MoveFileW(wExistingFileName, wNewFileName);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpNewFileName);
		Dee_Decref(lpExistingFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err_new:
	Dee_Decref(lpNewFileName);
err_existing:
	Dee_Decref(lpExistingFileName);
err:
	return -1;
}
#endif /* NEED_nt_MoveFile */

#ifdef NEED_nt_CreateHardLink
#undef NEED_nt_CreateHardLink
/* Work around a problem with long path names.
 * @return:  0: Successfully created the hardlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
nt_CreateHardLink(DeeObject *__restrict lpFileName,
                  DeeObject *__restrict lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	LPWSTR wFileName, wExistingFileName;
	BOOL error;
	DWORD dwError;
	wFileName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wFileName)
		goto err;
	wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
	if unlikely(!wExistingFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = CreateHardLinkW(wFileName, wExistingFileName, lpSecurityAttributes);
	if (!error && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		lpExistingFileName = DeeNTSystem_FixUncPath(lpExistingFileName);
		if unlikely(!lpExistingFileName)
			goto err_filename;
		wFileName = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wFileName)
			goto err_existing;
		wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
		if unlikely(!wExistingFileName)
			goto err_existing;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		error   = CreateHardLinkW(wFileName, wExistingFileName, lpSecurityAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpExistingFileName);
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err_existing:
	Dee_Decref(lpExistingFileName);
err_filename:
	Dee_Decref(lpFileName);
err:
	return -1;
}
#endif /* NEED_nt_CreateHardLink */


#ifdef NEED_nt_CreateSymbolicLink
#undef NEED_nt_CreateSymbolicLink
typedef BOOLEAN(APIENTRY *LPCREATESYMBOLICLINKW)(LPCWSTR lpSymlinkFileName,
                                                 LPCWSTR lpTargetFileName,
                                                 DWORD dwFlags);
PRIVATE LPCREATESYMBOLICLINKW pCreateSymbolicLinkW = NULL;
PRIVATE WCHAR const wKernel32[] = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */

/* Work around a problem with long path names.
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN int DCALL
nt_CreateSymbolicLink(DeeObject *__restrict lpSymlinkFileName,
                      DeeObject *__restrict lpTargetFileName,
                      DWORD dwFlags) {
	LPWSTR wSymlinkFileName, wTargetFileName;
	BOOLEAN error;
	DWORD dwError;
	LPCREATESYMBOLICLINKW callback = pCreateSymbolicLinkW;
	if (!callback) {
		DBG_ALIGNMENT_DISABLE();
		*(FARPROC *)&callback = GetProcAddress(GetModuleHandleW(wKernel32),
		                                       "CreateSymbolicLinkW");
		DBG_ALIGNMENT_ENABLE();
		if (!callback)
			*(void **)&callback = (void *)(uintptr_t)-1;
		*(void **)&pCreateSymbolicLinkW = *(void **)&callback;
	}
	if (*(void **)&callback == (void *)(uintptr_t)-1) {
		DeeError_Throwf(&DeeError_UnsupportedAPI,
		                "The host operating system does "
		                "not implement symbolic links");
		goto err;
	}
	wSymlinkFileName = (LPWSTR)DeeString_AsWide(lpSymlinkFileName);
	if unlikely(!wSymlinkFileName)
		goto err;
	wTargetFileName = (LPWSTR)DeeString_AsWide(lpTargetFileName);
	if unlikely(!wTargetFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = (*callback)(wSymlinkFileName, wTargetFileName, dwFlags);
	if (!error && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpSymlinkFileName = DeeNTSystem_FixUncPath(lpSymlinkFileName);
		if unlikely(!lpSymlinkFileName)
			goto err;
		lpTargetFileName = DeeNTSystem_FixUncPath(lpTargetFileName);
		if unlikely(!lpTargetFileName)
			goto err_filename;
		wSymlinkFileName = (LPWSTR)DeeString_AsWide(lpSymlinkFileName);
		if unlikely(!wSymlinkFileName)
			goto err_existing;
		wTargetFileName = (LPWSTR)DeeString_AsWide(lpTargetFileName);
		if unlikely(!wTargetFileName)
			goto err_existing;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		error   = (*callback)(wSymlinkFileName, wTargetFileName, dwFlags);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpTargetFileName);
		Dee_Decref(lpSymlinkFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err_existing:
	Dee_Decref(lpTargetFileName);
err_filename:
	Dee_Decref(lpSymlinkFileName);
err:
	return -1;
}
#endif /* NEED_nt_CreateSymbolicLink */


#ifdef NEED_posix_dfd_abspath
#undef NEED_posix_dfd_abspath
/* Construct an absolute path from `dfd:path'
 * @param: dfd:  Can be a `File', `int', `string', or [nt:`HANDLE']
 * @param: path: Must be a `string' */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
posix_dfd_abspath(DeeObject *dfd, DeeObject *path, unsigned int atflags) {
	struct unicode_printer printer;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	if unlikely(atflags != 0) {
#define NEED_err_bad_atflags
		err_bad_atflags(atflags);
		goto err;
	}

	/* Check if `path' is absolute. - If it is, then we must use _it_ */
	if (DeeSystem_IsAbs(DeeString_STR(path)))
		return_reference_(path);

	/* Must combine `dfd' with `path' */
	unicode_printer_init(&printer);
	if (DeeString_Check(dfd)) {
		if unlikely(unicode_printer_printstring(&printer, dfd) < 0)
			goto err_printer;
	} else {
#ifdef CONFIG_HOST_WINDOWS
#define posix_dfd_abspath_MUST_NORMALIZE_SLASHES
		int error;
		HANDLE hDfd;
		if (DeeInt_Check(dfd)) {
			int dfd_intval;
			if (DeeInt_TryAsInt(dfd, &dfd_intval)) {
				if (dfd_intval == AT_FDCWD) {
					/* Caller made an explicit request for the path to be relative! */
					unicode_printer_fini(&printer);
					return_reference_(path);
				}
			}
		}
		hDfd = DeeNTSystem_GetHandle(dfd);
		if unlikely(hDfd == INVALID_HANDLE_VALUE)
			goto err_printer;
		error = DeeNTSystem_PrintFilenameOfHandle(&printer, hDfd);
		if unlikely(error != 0) {
			if (error > 0) {
				DeeNTSystem_ThrowLastErrorf(NULL,
				                            "Failed to print path of HANDLE %p",
				                            hDfd);
			}
			goto err_printer;
		}
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_UNIX
		int os_dfd;
		os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err_printer;
#ifdef CONFIG_HAVE_PROCFS
		if unlikely(unicode_printer_printf(&printer, "/proc/self/fd/%d/", os_dfd) < 0)
			goto err_printer;
#else /* CONFIG_HAVE_PROCFS */
#define posix_dfd_abspath_MUST_NORMALIZE_SLASHES
		{
			int error;
			error = DeeSystem_PrintFilenameOfFD(&printer, os_dfd);
			if unlikely(error != 0)
				goto err_printer;
		}
#endif /* !CONFIG_HAVE_PROCFS */
#endif /* CONFIG_HOST_UNIX */
	}

	/* Trim trailing slashes. */
#ifdef posix_dfd_abspath_MUST_NORMALIZE_SLASHES
#undef posix_dfd_abspath_MUST_NORMALIZE_SLASHES
	if (!UNICODE_PRINTER_ISEMPTY(&printer)) {
		size_t newlen = UNICODE_PRINTER_LENGTH(&printer);
		while (newlen && DeeSystem_IsSep(UNICODE_PRINTER_GETCHAR(&printer, newlen - 1)))
			--newlen;
		if (newlen >= UNICODE_PRINTER_LENGTH(&printer)) {
			/* Append trailing slash */
			if unlikely(unicode_printer_putascii(&printer, DeeSystem_SEP))
				goto err_printer;
		} else {
			/* Trailing slash is already present (but make sure that there's only 1 of them) */
			++newlen;
			unicode_printer_truncate(&printer, newlen);
		}
	}
#endif /* posix_dfd_abspath_MUST_NORMALIZE_SLASHES */

#ifndef DEE_SYSTEM_IS_ABS_CHECKS_LEADING_SLASHES
	if (DeeSystem_IsSep(DeeString_STR(path)[0])) {
		/* Must skip leading slashes in `path' */
		char *utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err_printer;
		while (DeeSystem_IsSep(*utf8_path))
			++utf8_path;
		if unlikely(unicode_printer_printutf8(&printer, utf8_path, strlen(utf8_path)) < 0)
			goto err_printer;
	} else
#endif /* !DEE_SYSTEM_IS_ABS_CHECKS_LEADING_SLASHES */
	{
		if unlikely(unicode_printer_printstring(&printer, path) < 0)
			goto err_printer;
	}

	/* Pack the resulting string together */
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
err:
	return NULL;
}
#endif /* NEED_posix_dfd_abspath */

#ifdef NEED_posix_fd_abspath
#undef NEED_posix_fd_abspath
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_fd_abspath(DeeObject *__restrict fd) {
#ifdef CONFIG_HOST_WINDOWS
	HANDLE hFd = DeeNTSystem_GetHandle(fd);
	if unlikely(hFd == INVALID_HANDLE_VALUE)
		return NULL;
	return DeeNTSystem_GetFilenameOfHandle(hFd);
#else /* CONFIG_HOST_WINDOWS */
	int os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		return NULL;
	return DeeSystem_GetFilenameOfFD(os_fd);
#endif /* !CONFIG_HOST_WINDOWS */
}
#endif /* NEED_posix_fd_abspath */


#ifdef NEED_err_bad_atflags
#undef NEED_err_bad_atflags
INTERN ATTR_COLD int DCALL
err_bad_atflags(unsigned int atflags) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid atflags %#x",
	                       atflags);
}
#endif /* NEED_err_bad_atflags */

#ifdef NEED_posix_err_unsupported
#undef NEED_posix_err_unsupported
INTERN ATTR_NOINLINE ATTR_UNUSED ATTR_COLD NONNULL((1)) int DCALL
posix_err_unsupported(char const *__restrict name) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unsupported function `%s'",
	                       name);
}
#endif /* NEED_posix_err_unsupported */

DECL_END

#endif /* !GUARD_DEX_POSIX_P_ONDEMAND_C_INL */
