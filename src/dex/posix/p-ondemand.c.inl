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
#include "p-stat.c.inl"

#define NEED_err_unix_chdir
#define NEED_err_unix_remove
#define NEED_err_unix_unlink
#define NEED_err_nt_unlink
#define NEED_err_unix_rmdir
#define NEED_err_nt_rmdir
#define NEED_err_unix_rename
#define NEED_err_nt_rename
#define NEED_err_unix_remove_unsupported
#define NEED_err_unix_unlink_unsupported
#define NEED_err_nt_unlink_unsupported
#define NEED_err_unix_rmdir_unsupported
#define NEED_err_nt_rmdir_unsupported
#define NEED_err_unix_rename_unsupported
#define NEED_err_nt_rename_unsupported
#define NEED_err_unix_path_not_dir
#define NEED_err_nt_path_not_dir
#define NEED_err_unix_path_not_dir2
#define NEED_err_nt_path_not_dir2
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
#define NEED_err_unix_path_readonly2
#define NEED_err_unix_path_readonly
#define NEED_err_nt_path_readonly
#define NEED_err_unix_file_not_found
#define NEED_err_nt_file_not_found
#define NEED_err_unix_file_not_writable
#define NEED_err_nt_file_not_writable
#define NEED_err_unix_path_not_writable
#define NEED_err_nt_path_not_writable
#define NEED_err_unix_path_busy
#define NEED_err_nt_path_busy
#define NEED_err_unix_path_busy2
#define NEED_err_nt_path_busy2
#define NEED_err_unix_move_to_child
#define NEED_err_nt_move_to_child
#define NEED_err_unix_path_not_empty
#define NEED_err_nt_path_not_empty
#define NEED_err_unix_chtime_no_access
#define NEED_err_nt_chtime_no_access
#define NEED_err_unix_path_cross_dev2
#define NEED_err_nt_path_cross_dev2
#define NEED_nt_GetTempPath
#define NEED_nt_GetComputerName
#define NEED_nt_FReadLink
#define NEED_nt_SetCurrentDirectory
#define NEED_nt_GetFileAttributesEx
#define NEED_nt_GetFileAttributes
#define NEED_nt_SetFileAttributes
#define NEED_nt_CreateDirectory
#define NEED_nt_RemoveDirectory
#define NEED_nt_DeleteFile
#define NEED_nt_MoveFileEx
#define NEED_nt_CreateHardLink
#define NEED_nt_CreateSymbolicLink
#define NEED_posix_dfd_abspath
#define NEED_posix_fd_abspath
#define NEED_err_bad_atflags
#define NEED_posix_err_unsupported
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#ifdef NEED_nt_FReadLink
#undef NEED_nt_FReadLink

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
			COMPILER_FLEXIBLE_ARRAY(WCHAR, PathBuffer);
		} SymbolicLinkReparseBuffer; /* IO_REPARSE_TAG_SYMLINK */

		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			COMPILER_FLEXIBLE_ARRAY(WCHAR, PathBuffer);
		} MountPointReparseBuffer; /* IO_REPARSE_TAG_MOUNT_POINT */

		/* BEGIN: Taken from cygwin */
		struct {
			DWORD FileType;                            /* Take member name with a grain of salt.  Value is
			                                            * apparently always 2 for symlinks. */
			COMPILER_FLEXIBLE_ARRAY(char, PathBuffer); /* POSIX path as given to symlink(2).
			                                            * Path is not \0 terminated.
			                                            * Length is ReparseDataLength - sizeof (FileType).
			                                            * Always UTF-8.
			                                            * Chars given in incompatible codesets, e. g. umlauts
			                                            * in ISO-8859-x, are converted to the Unicode
			                                            * REPLACEMENT CHARACTER 0xfffd == \xef\xbf\bd */
		} LxSymlinkReparseBuffer; /* IO_REPARSE_TAG_LX_SYMLINK */
		/* END: Taken from cygwin */

		struct {
			COMPILER_FLEXIBLE_ARRAY(UCHAR, DataBuffer);
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

/* Read the contents of a symbolic link
 * @param: path: Only used for error messages */
INTDEF WUNUSED NONNULL((2)) DREF DeeObject *DCALL
nt_FReadLink(HANDLE hLinkFile, DeeObject *__restrict path) {
#define READLINK_INITIAL_BUFFER 300
	DEE_PREPARSE_DATA_BUFFER buffer;
	DREF DeeObject *result;
	DWORD bufsiz, buflen, dwError;
	LPWSTR linkstr_begin, linkstr_end;
	bufsiz = READLINK_INITIAL_BUFFER;
	buffer = (DEE_PREPARSE_DATA_BUFFER)Dee_Malloc(bufsiz);
	if unlikely(!buffer)
		goto err;
	/* Read symbolic link data. */
	DBG_ALIGNMENT_DISABLE();
	while (!DeviceIoControl(hLinkFile, FSCTL_GET_REPARSE_POINT,
	                        NULL, 0, buffer, bufsiz, &buflen, NULL)) {
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			DEE_PREPARSE_DATA_BUFFER new_buffer;
			bufsiz *= 2;
			new_buffer = (DEE_PREPARSE_DATA_BUFFER)Dee_Realloc(buffer, bufsiz);
			if unlikely(!new_buffer)
				goto err_buffer;
			DBG_ALIGNMENT_DISABLE();
			continue;
		}
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_buffer;
			DBG_ALIGNMENT_DISABLE();
			continue;
		}
		if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define NEED_err_nt_path_no_access
			err_nt_path_no_access(dwError, path);
			goto err_buffer;
		}
		if (DeeNTSystem_IsNoLink(dwError)) {
			/* Special handling for cygwin's symbolic links. */
			BY_HANDLE_FILE_INFORMATION hfInfo;
			if (GetFileInformationByHandle(hLinkFile, &hfInfo) &&
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
				if (ReadFile(hLinkFile, pFileBuffer, hfInfo.nFileSizeLow, &dwBytesRead, &oOffsetInfo) &&
				    dwBytesRead > sizeof(cygSymlinkCookie) &&
				    bcmp(pFileBuffer, cygSymlinkCookie, sizeof(cygSymlinkCookie)) == 0) {
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
			DeeNTSystem_ThrowErrorf(&DeeError_NoSymlink, dwError,
			                        "Path %r is not a symbolic link",
			                        path);
			goto err_buffer;
		}
		if (DeeNTSystem_IsUnsupportedError(dwError)) {
			DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
			                        "The filesystem hosting the path %r does "
			                        "not support reading symbolic links",
			                        path);
			goto err_buffer;
		}
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
		                        "Failed to read symbolic link %r",
		                        path);
		goto err_buffer;
	}
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
	return result;
err_buffer:
	Dee_Free(buffer);
err:
	return NULL;
}
#endif /* NEED_nt_FReadLink */


#ifdef NEED_err_unix_chdir
#undef NEED_err_unix_chdir
INTDEF ATTR_COLD NONNULL((2)) int DCALL
err_unix_chdir(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES) {
#define NEED_err_unix_path_no_access
		return err_unix_path_no_access(errno_value, path);
	}
#endif /* EACCES */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR) {
#define NEED_err_unix_path_not_dir
		return err_unix_path_not_dir(errno_value, path);
	}
#endif /* ENOTDIR */
#ifdef ENOENT
	if (errno_value == ENOENT) {
#define NEED_err_unix_path_not_found
		return err_unix_path_not_found(errno_value, path);
	}
#endif /* ENOENT */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to change the current working directory to %r",
	                                 path);
}
#endif /* NEED_err_unix_chdir */

#ifdef NEED_err_unix_remove
#undef NEED_err_unix_remove
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_remove(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES) {
#define NEED_err_unix_path_not_writable
		return err_unix_path_not_writable(errno_value, path);
	}
#endif /* EACCES */
#if defined(EBUSY) || defined(EINVAL)
#define NEED_err_unix_path_busy
	DeeSystem_IF_E2(errno_value, EBUSY, EINVAL, {
		return err_unix_path_busy(errno_value, path);
	});
#endif /* EBUSY || EINVAL */
#ifdef ENOENT
#define NEED_err_unix_path_not_found
	if (errno_value == ENOENT)
		return err_unix_path_not_found(errno_value, path);
#endif /* ENOENT */
#ifdef ENOTEMPTY
#define NEED_err_unix_path_not_empty
	if (errno_value == ENOTEMPTY)
		return err_unix_path_not_empty(errno_value, path);
#endif /* ENOTEMPTY */
#ifdef EROFS
#define NEED_err_unix_path_readonly
	if (errno_value == EROFS)
		return err_unix_path_readonly(errno_value, path);
#endif /* EROFS */
#ifdef EPERM
	if (errno_value == EPERM) {
		/* Posix states that this is the return value when the path is a directory,
		 * but also if the filesystem does not support unlinking files. */
#ifdef posix_stat_USED_STRUCT_STAT
		if (DeeString_Check(path)) {
			struct posix_stat_USED_STRUCT_STAT st;
			posix_stat_TCHAR *tpath;
			tpath = posix_stat_DeeString_AsTChar(path);
			if unlikely(!tpath)
				return -1;
			DBG_ALIGNMENT_DISABLE();
#ifdef posix_stat_USED_lstat
			if (posix_stat_USED_lstat(tpath, &st) == 0)
#else /* posix_stat_USED_lstat */
			if (posix_stat_USED_stat(tpath, &st) == 0)
#endif /* !posix_stat_USED_lstat */
			{
				DBG_ALIGNMENT_ENABLE();
#define NEED_err_unix_path_is_dir
				if (STAT_ISDIR(st.st_mode))
					return err_unix_path_is_dir(errno_value, path);
	
				/* Posix also states that the presence of the
				 * S_ISVTX bit may cause EPERM to be returned. */
#define NEED_err_unix_path_not_writable
				if (st.st_mode & STAT_ISVTX)
					return err_unix_path_not_writable(errno_value, path);
			}
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* posix_stat_USED_STRUCT_STAT */
#define NEED_err_unix_remove_unsupported
		return err_unix_remove_unsupported(errno_value, path);
	}
#endif /* EPERM */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to remove file or directory %r",
	                                 path);
}
#endif /* NEED_err_unix_remove */


#ifdef NEED_err_unix_unlink
#undef NEED_err_unix_unlink
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_unlink(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES) {
#define NEED_err_unix_path_not_writable
		return err_unix_path_not_writable(errno_value, path);
	}
#endif /* EACCES */
#ifdef EBUSY
	if (errno_value == EBUSY) {
#define NEED_err_unix_path_busy
		return err_unix_path_busy(errno_value, path);
	}
#endif /* EBUSY */
#ifdef EISDIR
	if (errno_value == EISDIR) {
#define NEED_err_unix_path_is_dir
		return err_unix_path_is_dir(errno_value, path);
	}
#endif /* EISDIR */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR) {
#define NEED_err_unix_path_not_dir
		return err_unix_path_not_dir(errno_value, path);
	}
#endif /* ENOTDIR */
#ifdef ENOENT
	if (errno_value == ENOENT) {
#define NEED_err_unix_path_not_found
		return err_unix_path_not_found(errno_value, path);
	}
#endif /* ENOENT */
#ifdef EPERM
	if (errno_value == EPERM) {
		/* Posix states that this is the return value when the path is a directory,
		 * but also if the filesystem does not support unlinking files. */
#ifdef posix_stat_USED_STRUCT_STAT
		if (DeeString_Check(path)) {
			struct posix_stat_USED_STRUCT_STAT st;
			posix_stat_TCHAR *tpath;
			tpath = posix_stat_DeeString_AsTChar(path);
			if unlikely(!tpath)
				return -1;
			DBG_ALIGNMENT_DISABLE();
#ifdef posix_stat_USED_lstat
			if (posix_stat_USED_lstat(tpath, &st) == 0)
#else /* posix_stat_USED_lstat */
			if (posix_stat_USED_stat(tpath, &st) == 0)
#endif /* !posix_stat_USED_lstat */
			{
				DBG_ALIGNMENT_ENABLE();
#define NEED_err_unix_path_is_dir
				if (STAT_ISDIR(st.st_mode))
					return err_unix_path_is_dir(errno_value, path);
	
				/* Posix also states that the presence of the
				 * S_ISVTX bit may cause EPERM to be returned. */
#define NEED_err_unix_path_not_writable
				if (st.st_mode & STAT_ISVTX)
					return err_unix_path_not_writable(errno_value, path);
			}
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* posix_stat_USED_STRUCT_STAT */
#define NEED_err_unix_unlink_unsupported
		return err_unix_unlink_unsupported(errno_value, path);
	}
#endif /* EPERM */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to unlink file %r",
	                                 path);
}
#endif /* NEED_err_unix_unlink */


#ifdef NEED_err_nt_unlink
#undef NEED_err_nt_unlink
INTDEF ATTR_COLD NONNULL((2)) int DCALL
err_nt_unlink(DWORD dwError, DeeObject *__restrict path) {
	if (dwError == ERROR_ACCESS_DENIED) {
		DWORD dwAttributes;
		int temp;
		/* Check if the path is actually a directory. */
#define NEED_nt_GetFileAttributes
		temp = nt_GetFileAttributes(path, &dwAttributes);
		if unlikely(temp < 0)
			return temp;
#define NEED_err_nt_path_is_dir
		if (temp == 0 && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return err_nt_path_is_dir(dwError, path);
	}
#define NEED_err_nt_path_not_writable
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return err_nt_path_not_writable(dwError, path);
#define NEED_err_nt_path_busy
	if (DeeNTSystem_IsBusy(dwError))
		return err_nt_path_busy(dwError, path);
#define NEED_err_nt_path_not_dir
	if (DeeNTSystem_IsNotDir(dwError))
		return err_nt_path_not_dir(dwError, path);
#define NEED_err_nt_path_not_found
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return err_nt_path_not_found(dwError, path);
#define NEED_err_nt_unlink_unsupported
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return err_nt_unlink_unsupported(dwError, path);
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to unlink file %r",
	                               path);
}
#endif /* NEED_err_nt_unlink */

#ifdef NEED_err_unix_rmdir
#undef NEED_err_unix_rmdir
INTDEF ATTR_COLD NONNULL((2)) int DCALL
err_unix_rmdir(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
#define NEED_err_unix_path_not_writable
	if (errno_value == EACCES)
		return err_unix_path_not_writable(errno_value, path);
#endif /* EACCES */
#if defined(EBUSY) || defined(EINVAL)
#define NEED_err_unix_path_busy
	DeeSystem_IF_E2(errno_value, EBUSY, EINVAL, {
		return err_unix_path_busy(errno_value, path);
	});
#endif /* EBUSY || EINVAL */
#ifdef ENOENT
#define NEED_err_unix_path_not_found
	if (errno_value == ENOENT)
		return err_unix_path_not_found(errno_value, path);
#endif /* ENOENT */
#ifdef ENOTDIR
#define NEED_err_unix_path_not_dir
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir(errno_value, path);
#endif /* ENOTDIR */
#ifdef ENOTEMPTY
#define NEED_err_unix_path_not_empty
	if (errno_value == ENOTEMPTY)
		return err_unix_path_not_empty(errno_value, path);
#endif /* ENOTEMPTY */
#ifdef EROFS
#define NEED_err_unix_path_readonly
	if (errno_value == EROFS)
		return err_unix_path_readonly(errno_value, path);
#endif /* EROFS */
#ifdef EPERM
	if (errno_value == EPERM) {
		/* Posix states that `EPERM' may be returned because of the sticky bit.
		 * However in that event, we want to throw an access error, not an
		 * unsupported-api error. */
#ifdef posix_stat_USED_STRUCT_STAT
		if (DeeString_Check(path)) {
			struct posix_stat_USED_STRUCT_STAT st;
			posix_stat_TCHAR *tpath;
			tpath = posix_stat_DeeString_AsTChar(path);
			if unlikely(!tpath)
				return -1;
			DBG_ALIGNMENT_DISABLE();
#ifdef posix_stat_USED_lstat
			if (posix_stat_USED_lstat(tpath, &st) == 0)
#else /* posix_stat_USED_lstat */
			if (posix_stat_USED_stat(tpath, &st) == 0)
#endif /* !posix_stat_USED_lstat */
			{
				if (st.st_mode & STAT_ISVTX) {
					DBG_ALIGNMENT_ENABLE();
#define NEED_err_unix_path_not_writable
					return err_unix_path_not_writable(errno_value, path);
				}
			}
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* posix_stat_USED_STRUCT_STAT */
#define NEED_err_unix_rmdir_unsupported
		return err_unix_rmdir_unsupported(errno_value, path);
	}
#endif /* EPERM */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to remove directory %r",
	                                 path);
}
#endif /* NEED_err_unix_rmdir */

#ifdef NEED_err_nt_rmdir
#undef NEED_err_nt_rmdir
INTDEF ATTR_COLD NONNULL((2)) int DCALL
err_nt_rmdir(DWORD dwError, DeeObject *__restrict path) {
#define NEED_err_nt_path_not_empty
	if (DeeNTSystem_IsNotEmpty(dwError))
		return err_nt_path_not_empty(dwError, path);
#define NEED_err_nt_path_not_writable
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return err_nt_path_not_writable(dwError, path);
#define NEED_err_nt_path_busy
	if (DeeNTSystem_IsBusy(dwError))
		return err_nt_path_busy(dwError, path);
#define NEED_err_nt_path_not_found
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return err_nt_path_not_found(dwError, path);
#define NEED_err_nt_path_not_dir
	if (DeeNTSystem_IsNotDir(dwError))
		return err_nt_path_not_dir(dwError, path);
#define NEED_err_nt_rmdir_unsupported
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return err_nt_rmdir_unsupported(dwError, path);
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to remove directory %r",
	                               path);
}
#endif /* NEED_err_nt_rmdir */

#ifdef NEED_err_unix_rename
#undef NEED_err_unix_rename
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_rename(int errno_value,
                DeeObject *existing_path,
                DeeObject *new_path) {
#ifdef EACCES
#define NEED_err_unix_path_readonly2
	if (errno_value == EACCES)
		return err_unix_path_readonly2(errno_value, existing_path, new_path);
#endif /* EACCES */
#ifdef EBUSY
#define NEED_err_unix_path_busy2
	if (errno_value == EBUSY)
		return err_unix_path_busy2(errno_value, existing_path, new_path);
#endif /* EBUSY */
#ifdef EINVAL
#define NEED_err_unix_move_to_child
	if (errno_value == EINVAL)
		return err_unix_move_to_child(errno_value, existing_path, new_path);
#endif /* EINVAL */
#ifdef ENOENT
#define NEED_err_unix_path_not_found2
	if (errno_value == ENOENT)
		return err_unix_path_not_found2(errno_value, existing_path, new_path);
#endif /* ENOENT */
#ifdef EISDIR
#define NEED_err_unix_path_is_dir
	if (errno_value == EISDIR)
		return err_unix_path_is_dir(errno_value, new_path);
#endif /* EISDIR */
#if defined(ENOTEMPTY) || defined(EEXIST)
#define NEED_err_unix_path_not_empty
	DeeSystem_IF_E2(errno_value, ENOTEMPTY, EEXIST, {
		return err_unix_path_not_empty(errno_value, new_path);
	});
#endif /* ENOTEMPTY || EEXIST */
#ifdef ENOTDIR
#define NEED_err_unix_path_not_dir2
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir2(errno_value, existing_path, new_path);
#endif /* ENOTDIR */
#ifdef EPERM
	if (errno_value == EPERM) {
		/* The same deal concerning the sticky bit. */
#ifdef posix_stat_USED_STRUCT_STAT
		if (DeeString_Check(existing_path)) {
			struct posix_stat_USED_STRUCT_STAT oldst;
			posix_stat_TCHAR *toldpath;
			toldpath = posix_stat_DeeString_AsTChar(existing_path);
			if unlikely(!toldpath)
				return -1;
			DBG_ALIGNMENT_DISABLE();
#ifdef posix_stat_USED_lstat
			if (posix_stat_USED_lstat(toldpath, &oldst) == 0)
#else /* posix_stat_USED_lstat */
			if (posix_stat_USED_stat(toldpath, &oldst) == 0)
#endif /* !posix_stat_USED_lstat */
			{
				if (oldst.st_mode & STAT_ISVTX) {
					DBG_ALIGNMENT_ENABLE();
#define NEED_err_unix_path_readonly2
					return err_unix_path_readonly2(errno_value, existing_path, new_path);
				}
			}
			DBG_ALIGNMENT_ENABLE();
		}
		if (DeeString_Check(new_path)) {
			int stat_error;
			struct posix_stat_USED_STRUCT_STAT newst;
			posix_stat_TCHAR *tnewpath, *tnewpath_copy;
			size_t tnewpath_size, tnewpath_lastsep;
			tnewpath = posix_stat_DeeString_AsTChar(new_path);
			if unlikely(!tnewpath)
				return -1;
			tnewpath_lastsep = 1;
			for (tnewpath_size = 0; tnewpath[tnewpath_size]; ++tnewpath_size) {
				if (DeeSystem_IsSep(tnewpath[tnewpath_size]))
					tnewpath_lastsep = tnewpath_size + 1;
			}
			tnewpath_copy = (posix_stat_TCHAR *)Dee_Malloc((tnewpath_lastsep + 1) *
			                                               sizeof(posix_stat_TCHAR));
			if unlikely(!tnewpath_copy)
				return -1;
			if (tnewpath_lastsep == 1) {
				tnewpath_copy[0] = (posix_stat_TCHAR)'.';
				tnewpath_copy[1] = (posix_stat_TCHAR)'\0';
			} else {
				*(posix_stat_TCHAR *)mempcpyc(tnewpath_copy, tnewpath, tnewpath_lastsep,
				                              sizeof(posix_stat_TCHAR)) = (posix_stat_TCHAR)'\0';
			}
			DBG_ALIGNMENT_DISABLE();
#ifdef posix_stat_USED_lstat
			stat_error = posix_stat_USED_lstat(tnewpath_copy, &newst);
#else /* posix_stat_USED_lstat */
			stat_error = posix_stat_USED_stat(tnewpath_copy, &newst);
#endif /* !posix_stat_USED_lstat */
			Dee_Free(tnewpath_copy);
			if (stat_error == 0) {
				if (newst.st_mode & STAT_ISVTX) {
					DBG_ALIGNMENT_ENABLE();
#define NEED_err_unix_path_readonly2
					return err_unix_path_readonly2(errno_value, existing_path, new_path);
				}
			}
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* posix_stat_USED_STRUCT_STAT */
#define NEED_err_unix_rename_unsupported
		return err_unix_rename_unsupported(errno_value, existing_path, new_path);
	}
#endif /* EPERM */
#ifdef EROFS
#define NEED_err_unix_path_readonly2
	if (errno_value == EROFS)
		return err_unix_path_readonly2(errno_value, existing_path, new_path);
#endif /* EROFS */
#ifdef EXDEV
#define NEED_err_unix_path_cross_dev2
	if (errno_value == EXDEV)
		return err_unix_path_cross_dev2(errno_value, existing_path, new_path);
#endif /* EXDEV */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to rename %r to %r",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_rename */

#ifdef NEED_err_nt_rename
#undef NEED_err_nt_rename
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_rename(DWORD dwError,
              DeeObject *existing_path,
              DeeObject *new_path) {
#define NEED_err_nt_path_no_access2
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return err_nt_path_no_access2(dwError, existing_path, new_path);
#define NEED_err_nt_path_busy2
	if (DeeNTSystem_IsBusy(dwError))
		return err_nt_path_busy2(dwError, existing_path, new_path);
#define NEED_err_nt_move_to_child
	if (dwError == ERROR_SHARING_VIOLATION)
		return err_nt_move_to_child(dwError, existing_path, new_path);
#define NEED_err_nt_path_not_found2
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return err_nt_path_not_found2(dwError, existing_path, new_path);
#define NEED_err_nt_path_exists
	if (DeeNTSystem_IsNotEmpty(dwError) || DeeNTSystem_IsExists(dwError))
		return err_nt_path_exists(dwError, new_path);
#define NEED_err_nt_path_not_dir2
	if (DeeNTSystem_IsNotDir(dwError))
		return err_nt_path_not_dir2(dwError, existing_path, new_path);
#define NEED_err_nt_rename_unsupported
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return err_nt_rename_unsupported(dwError, existing_path, new_path);
#define NEED_err_nt_path_cross_dev2
	if (DeeNTSystem_IsXDev(dwError))
		return err_nt_path_cross_dev2(dwError, existing_path, new_path);
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to rename %r to %r",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_rename */

#ifdef NEED_err_unix_remove_unsupported
#undef NEED_err_unix_remove_unsupported
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_remove_unsupported(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the path %r does "
	                                 "not support removing files or directories",
	                                 path);
}
#endif /* NEED_err_unix_remove_unsupported */

#ifdef NEED_err_unix_unlink_unsupported
#undef NEED_err_unix_unlink_unsupported
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_unlink_unsupported(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the path %r "
	                                 "does not support unlinking files",
	                                 path);
}
#endif /* NEED_err_unix_unlink_unsupported */

#ifdef NEED_err_nt_unlink_unsupported
#undef NEED_err_nt_unlink_unsupported
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_unlink_unsupported(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
	                               "The filesystem hosting the path %r "
	                               "does not support unlinking files",
	                               path);
}
#endif /* NEED_err_nt_unlink_unsupported */

#ifdef NEED_err_unix_rmdir_unsupported
#undef NEED_err_unix_rmdir_unsupported
INTDEF ATTR_COLD NONNULL((2)) int DCALL
err_unix_rmdir_unsupported(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the path %r does "
	                                 "not support the removal of directories",
	                                 path);
}
#endif /* NEED_err_unix_rmdir_unsupported */

#ifdef NEED_err_nt_rmdir_unsupported
#undef NEED_err_nt_rmdir_unsupported
INTDEF ATTR_COLD NONNULL((2)) int DCALL
err_nt_rmdir_unsupported(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
	                               "The filesystem hosting the path %r does "
	                               "not support the removal of directories",
	                               path);
}
#endif /* NEED_err_nt_rmdir_unsupported */

#ifdef NEED_err_unix_rename_unsupported
#undef NEED_err_unix_rename_unsupported
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_rename_unsupported(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the paths %r and %r "
	                                 "does not support renaming of files",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_rename_unsupported */

#ifdef NEED_err_nt_rename_unsupported
#undef NEED_err_nt_rename_unsupported
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_rename_unsupported(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
	                               "The filesystem hosting the paths %r and %r "
	                               "does not support renaming of files",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_rename_unsupported */

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

#ifdef NEED_err_unix_path_not_dir2
#undef NEED_err_unix_path_not_dir2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_path_not_dir2(int error, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_NoDirectory, error,
	                                 "Some part of the path %r or %r is not a directory",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_not_dir2 */

#ifdef NEED_err_nt_path_not_dir2
#undef NEED_err_nt_path_not_dir2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_not_dir2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_NoDirectory, dwError,
	                               "Some part of the path %r or %r is not a directory",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_path_not_dir2 */

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
                         DeeObject *existing_path,
                         DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "Path %r or %r could not be found",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_not_found2 */

#ifdef NEED_err_nt_path_not_found2
#undef NEED_err_nt_path_not_found2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_not_found2(DWORD dwError,
                       DeeObject *existing_path,
                       DeeObject *new_path) {
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
                         DeeObject *existing_path,
                         DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Access to %r or %r has not been granted",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_no_access2 */

#ifdef NEED_err_nt_path_no_access2
#undef NEED_err_nt_path_no_access2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_no_access2(DWORD dwError,
                       DeeObject *existing_path,
                       DeeObject *new_path) {
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

#ifdef NEED_err_unix_path_readonly2
#undef NEED_err_unix_path_readonly2
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_path_readonly2(int errno_value,
                        DeeObject *existing_path,
                        DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "Path %r or %r could not be found",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_readonly2 */

#ifdef NEED_err_unix_path_readonly
#undef NEED_err_unix_path_readonly
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_path_readonly(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, errno_value,
	                                 "Path %r is apart of a read-only filesystem",
	                                 path);
}
#endif /* NEED_err_unix_path_readonly */

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

#ifdef NEED_err_unix_path_busy2
#undef NEED_err_unix_path_busy2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_path_busy2(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_BusyFile, errno_value,
	                                 "Path %r or %r cannot be accessed because it is already in use",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_busy2 */

#ifdef NEED_err_nt_path_busy2
#undef NEED_err_nt_path_busy2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_busy2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_BusyFile, dwError,
	                               "Path %r or %r cannot be accessed because it is already in use",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_path_busy2 */

#ifdef NEED_err_unix_move_to_child
#undef NEED_err_unix_move_to_child
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_move_to_child(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_ValueError, errno_value,
	                                 "Cannot rename path %r to %r which is a sub-directory of the old path",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_move_to_child */

#ifdef NEED_err_nt_move_to_child
#undef NEED_err_nt_move_to_child
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_move_to_child(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_ValueError, dwError,
	                               "Cannot rename path %r to %r which is a sub-directory of the old path",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_move_to_child */

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
                         DeeObject *existing_path,
                         DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_CrossDeviceLink, errno_value,
	                                 "Paths %r and %r are not apart of the same filesystem",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_cross_dev2 */

#ifdef NEED_err_nt_path_cross_dev2
#undef NEED_err_nt_path_cross_dev2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_cross_dev2(DWORD dwError,
                       DeeObject *existing_path,
                       DeeObject *new_path) {
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

#ifdef NEED_nt_MoveFileEx
#undef NEED_nt_MoveFileEx
/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
nt_MoveFileEx(DeeObject *__restrict lpExistingFileName,
              DeeObject *__restrict lpNewFileName,
              DWORD dwFlags) {
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
	error = MoveFileExW(wExistingFileName, wNewFileName, dwFlags);
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
		error   = MoveFileExW(wExistingFileName, wNewFileName, dwFlags);
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
#endif /* NEED_nt_MoveFileEx */

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
		/* Special handling for `deemon.File' */
		if (DeeFile_Check(dfd)) {
			DREF DeeObject *dfd_filename;
			dfd_filename = DeeFile_Filename(dfd);
			if unlikely(!dfd_filename)
				goto err_printer;
			if (DeeSystem_IsAbs(DeeString_STR(dfd_filename))) {
				if unlikely(unicode_printer_printstring(&printer, dfd_filename) < 0)
					goto err_printer;
				Dee_Decref(dfd_filename);
				goto got_dfd_path;
			}
			Dee_Decref(dfd_filename);
		}

#ifdef CONFIG_HOST_WINDOWS
		{
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
		}
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_UNIX
		{
			int os_dfd;
			os_dfd = DeeUnixSystem_GetFD(dfd);
			if unlikely(os_dfd == -1)
				goto err_printer;
#ifdef CONFIG_HAVE_PROCFS
			if unlikely(unicode_printer_printf(&printer, "/proc/self/fd/%d/", os_dfd) < 0)
				goto err_printer;
#else /* CONFIG_HAVE_PROCFS */
			{
				int error;
				error = DeeSystem_PrintFilenameOfFD(&printer, os_dfd);
				if unlikely(error != 0)
					goto err_printer;
			}
#endif /* !CONFIG_HAVE_PROCFS */
		}
#endif /* CONFIG_HOST_UNIX */
	}

	/* Trim trailing slashes. */
got_dfd_path:
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
	if (DeeFile_Check(fd)) {
		DREF DeeObject *result;
		result = DeeFile_Filename(fd);
		if (!result || DeeSystem_IsAbs(DeeString_STR(result)))
			return result;
		Dee_Decref(result);
	}

	{
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
