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
#ifndef GUARD_DEX_POSIX_P_ONDEMAND_C_INL
#define GUARD_DEX_POSIX_P_ONDEMAND_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

#ifdef __INTELLISENSE__
#include "p-path.c.inl"
#include "p-stat.c.inl"

#define NEED_err_unix_chdir
#define NEED_err_unix_remove
#define NEED_err_unix_unlink
#define NEED_err_nt_unlink
#define NEED_err_unix_rmdir
#define NEED_err_unix_mkdir
#define NEED_err_nt_rmdir
#define NEED_err_nt_mkdir
#define NEED_err_unix_rename
#define NEED_err_nt_rename
#define NEED_err_unix_link
#define NEED_err_unix_symlink
#define NEED_err_nt_link
#define NEED_err_nt_symlink
#define NEED_err_unix_truncate
#define NEED_err_unix_ftruncate
#define NEED_err_unix_remove_unsupported
#define NEED_err_unix_unlink_unsupported
#define NEED_err_nt_unlink_unsupported
#define NEED_err_unix_rmdir_unsupported
#define NEED_err_unix_mkdir_unsupported
#define NEED_err_nt_rmdir_unsupported
#define NEED_err_nt_mkdir_unsupported
#define NEED_err_unix_rename_unsupported
#define NEED_err_nt_rename_unsupported
#define NEED_err_unix_link_unsupported
#define NEED_err_unix_symlink_unsupported
#define NEED_err_nt_link_unsupported
#define NEED_err_nt_symlink_unsupported
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
#define NEED_err_unix_file_closed
#define NEED_err_unix_ftruncate_fbig
#define NEED_err_unix_truncate_fbig
#define NEED_err_unix_ftruncate_isdir
#define NEED_err_unix_truncate_isdir
#define NEED_err_unix_ftruncate_txtbusy
#define NEED_err_unix_truncate_txtbusy
#define NEED_err_nt_path_not_link
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
#define NEED_nt_CreateSymbolicLinkAuto
#define NEED_nt_CreateSymbolicLink
#define NEED_posix_dfd_makepath
#define NEED_posix_fd_makepath
#define NEED_posix_fd_makepath_fd
#define NEED_posix_fd_openfile
#define NEED_posix_copyfile_fileio
#define NEED_err_bad_atflags
#define NEED_err_bad_copyfile_bufsize_is_zero
#define NEED_posix_err_unsupported
#endif /* __INTELLISENSE__ */

#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/format.h>
#include <deemon/mapfile.h>
#include <deemon/system-features.h>

#include <hybrid/typecore.h>

#ifdef CONFIG_HAVE_sendfile
#ifdef CONFIG_HAVE_SYS_SENDFILE_H
#include <sys/sendfile.h>
#endif /* CONFIG_HAVE_SYS_SENDFILE_H */
#endif /* CONFIG_HAVE_sendfile */


DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

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
 * @param: path: Only used for error messages
 * @return: * :        Symlink contents
 * @return: NULL:      Error
 * @return: ITER_DONE: Not a symbolic link (only if `throw_error_if_not_a_link == false') */
INTERN WUNUSED NONNULL((2)) DREF DeeObject *DCALL
nt_FReadLink(HANDLE hLinkFile, DeeObject *__restrict path,
             bool throw_error_if_not_a_link) {
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
							/* Text is encoded as a wide-string */
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

			/* Not a symbolic link! */
			if (!throw_error_if_not_a_link) {
				Dee_Free(buffer);
				return ITER_DONE;
			}
			err_nt_path_not_link(dwError, path);
#define NEED_err_nt_path_not_link
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
		linkstr_begin = (LPWSTR)((PBYTE)buffer->SymbolicLinkReparseBuffer.PathBuffer +
		                         /*  */ buffer->SymbolicLinkReparseBuffer.SubstituteNameOffset);
		linkstr_end = (LPWSTR)((PBYTE)linkstr_begin + buffer->SymbolicLinkReparseBuffer.SubstituteNameLength);
		break;

	case IO_REPARSE_TAG_MOUNT_POINT:
		linkstr_begin = (LPWSTR)((PBYTE)buffer->MountPointReparseBuffer.PathBuffer +
		                         /*  */ buffer->MountPointReparseBuffer.SubstituteNameOffset);
		linkstr_end = (LPWSTR)((PBYTE)linkstr_begin + buffer->MountPointReparseBuffer.SubstituteNameLength);
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
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_chdir(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES)
		return err_unix_path_no_access(errno_value, path);
#define NEED_err_unix_path_no_access
#endif /* EACCES */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir(errno_value, path);
#define NEED_err_unix_path_not_dir
#endif /* ENOTDIR */
#ifdef ENOENT
	if (errno_value == ENOENT)
		return err_unix_path_not_found(errno_value, path);
#define NEED_err_unix_path_not_found
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
	if (errno_value == EACCES)
		return err_unix_path_not_writable(errno_value, path);
#define NEED_err_unix_path_not_writable
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
	if (errno_value == EROFS)
		return err_unix_path_readonly(errno_value, path);
#define NEED_err_unix_path_readonly
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
	if (errno_value == EACCES)
		return err_unix_path_not_writable(errno_value, path);
#define NEED_err_unix_path_not_writable
#endif /* EACCES */
#ifdef EBUSY
	if (errno_value == EBUSY)
		return err_unix_path_busy(errno_value, path);
#define NEED_err_unix_path_busy
#endif /* EBUSY */
#ifdef EISDIR
	if (errno_value == EISDIR)
		return err_unix_path_is_dir(errno_value, path);
#define NEED_err_unix_path_is_dir
#endif /* EISDIR */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir(errno_value, path);
#define NEED_err_unix_path_not_dir
#endif /* ENOTDIR */
#ifdef ENOENT
	if (errno_value == ENOENT)
		return err_unix_path_not_found(errno_value, path);
#define NEED_err_unix_path_not_found
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
INTERN ATTR_COLD NONNULL((2)) int DCALL
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
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_rmdir(int errno_value, DeeObject *__restrict path) {
#ifdef EACCES
	if (errno_value == EACCES)
		return err_unix_path_not_writable(errno_value, path);
#define NEED_err_unix_path_not_writable
#endif /* EACCES */
#if defined(EBUSY) || defined(EINVAL)
	DeeSystem_IF_E2(errno_value, EBUSY, EINVAL, {
		return err_unix_path_busy(errno_value, path);
	});
#define NEED_err_unix_path_busy
#endif /* EBUSY || EINVAL */
#ifdef ENOENT
	if (errno_value == ENOENT)
		return err_unix_path_not_found(errno_value, path);
#define NEED_err_unix_path_not_found
#endif /* ENOENT */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir(errno_value, path);
#define NEED_err_unix_path_not_dir
#endif /* ENOTDIR */
#ifdef ENOTEMPTY
	if (errno_value == ENOTEMPTY)
		return err_unix_path_not_empty(errno_value, path);
#define NEED_err_unix_path_not_empty
#endif /* ENOTEMPTY */
#ifdef EROFS
	if (errno_value == EROFS)
		return err_unix_path_readonly(errno_value, path);
#define NEED_err_unix_path_readonly
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
		return err_unix_rmdir_unsupported(errno_value, path);
#define NEED_err_unix_rmdir_unsupported
	}
#endif /* EPERM */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to remove directory %r",
	                                 path);
}
#endif /* NEED_err_unix_rmdir */

#ifdef NEED_err_unix_mkdir
#undef NEED_err_unix_mkdir
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_mkdir(int errno_value, DeeObject *__restrict path, unsigned int mode) {
#ifdef EACCES
	if (errno_value == EACCES)
		return err_unix_path_not_writable(errno_value, path);
#define NEED_err_unix_path_not_writable
#endif /* EACCES */
#ifdef EEXIST
	if (errno_value == EEXIST)
		return err_unix_path_exists(errno_value, path);
#define NEED_err_unix_path_exists
#endif /* EEXIST */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir(errno_value, path);
#define NEED_err_unix_path_not_dir
#endif /* ENOTDIR */
#ifdef EROFS
	if (errno_value == EROFS)
		return err_unix_path_readonly(errno_value, path);
#define NEED_err_unix_path_readonly
#endif /* EROFS */
#ifdef EPERM
	if (errno_value == EPERM)
		return err_unix_mkdir_unsupported(errno_value, path, mode);
#define NEED_err_unix_mkdir_unsupported
#endif /* EPERM */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to create directory %r with mode %#x",
	                                 path, mode);
}
#endif /* NEED_err_unix_mkdir */

#ifdef NEED_err_nt_rmdir
#undef NEED_err_nt_rmdir
INTERN ATTR_COLD NONNULL((2)) int DCALL
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

#ifdef NEED_err_nt_mkdir
#undef NEED_err_nt_mkdir
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_mkdir(DWORD dwError, DeeObject *__restrict path, unsigned int mode) {
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return err_nt_path_not_writable(dwError, path);
#define NEED_err_nt_path_not_writable
	if (DeeNTSystem_IsExists(dwError))
		return err_nt_path_exists(dwError, path);
#define NEED_err_nt_path_exists
	if (DeeNTSystem_IsNotDir(dwError))
		return err_nt_path_not_dir(dwError, path);
#define NEED_err_nt_path_not_dir
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return err_nt_mkdir_unsupported(dwError, path, mode);
#define NEED_err_nt_mkdir_unsupported
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to create directory %r with mode %#x",
	                               path, mode);
}
#endif /* NEED_err_nt_mkdir */

#ifdef NEED_err_unix_rename
#undef NEED_err_unix_rename
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_rename(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
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
			tnewpath_copy = (posix_stat_TCHAR *)Dee_Mallocc(tnewpath_lastsep + 1,
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
	if (errno_value == EROFS)
		return err_unix_path_readonly2(errno_value, existing_path, new_path);
#define NEED_err_unix_path_readonly2
#endif /* EROFS */
#ifdef EXDEV
	if (errno_value == EXDEV)
		return err_unix_path_cross_dev2(errno_value, existing_path, new_path);
#define NEED_err_unix_path_cross_dev2
#endif /* EXDEV */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to rename %r to %r",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_rename */

#ifdef NEED_err_nt_rename
#undef NEED_err_nt_rename
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_rename(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
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

#ifdef NEED_err_unix_link
#undef NEED_err_unix_link
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_link(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
#ifdef EACCES
#define NEED_err_unix_path_readonly2
	if (errno_value == EACCES)
		return err_unix_path_readonly2(errno_value, existing_path, new_path);
#endif /* EACCES */
#ifdef EEXIST
#define NEED_err_unix_path_exists
	if (errno_value == EEXIST)
		return err_unix_path_exists(errno_value, new_path);
#endif /* EEXIST */
#ifdef ENOENT
#define NEED_err_unix_path_not_found2
	if (errno_value == ENOENT)
		return err_unix_path_not_found2(errno_value, existing_path, new_path);
#endif /* ENOENT */
#ifdef EPERM
#define NEED_err_unix_link_unsupported
	if (errno_value == EPERM)
		return err_unix_link_unsupported(errno_value, existing_path, new_path);
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
	                                 "Failed to create hard-link for %r at %r",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_link */

#ifdef NEED_err_nt_link
#undef NEED_err_nt_link
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_link(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
#define NEED_err_nt_path_no_access2
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return err_nt_path_no_access2(dwError, existing_path, new_path);
#define NEED_err_nt_path_exists
	if (DeeNTSystem_IsExists(dwError))
		return err_nt_path_exists(dwError, new_path);
#define NEED_err_nt_path_not_found2
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return err_nt_path_not_found2(dwError, existing_path, new_path);
#define NEED_err_nt_path_cross_dev2
	if (DeeNTSystem_IsXDev(dwError))
		return err_nt_path_cross_dev2(dwError, existing_path, new_path);
#define NEED_err_nt_link_unsupported
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return err_nt_link_unsupported(dwError, existing_path, new_path);
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to create hard-link for %r at %r",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_link */

#ifdef NEED_err_unix_symlink
#undef NEED_err_unix_symlink
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_symlink(int errno_value, DeeObject *text, DeeObject *path) {
#ifdef EACCES
	if (errno_value == EACCES)
		return err_unix_path_readonly(errno_value, path);
#define NEED_err_unix_path_readonly
#endif /* EACCES */
#ifdef EEXIST
	if (errno_value == EEXIST)
		return err_unix_path_exists(errno_value, path);
#define NEED_err_unix_path_exists
#endif /* EEXIST */
#ifdef ENOENT
	if (errno_value == ENOENT)
		return err_unix_path_not_found(errno_value, path);
#define NEED_err_unix_path_not_found
#endif /* ENOENT */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir(errno_value, path);
#define NEED_err_unix_path_not_dir
#endif /* ENOTDIR */
#ifdef EPERM
	if (errno_value == EPERM)
		return err_unix_symlink_unsupported(errno_value, text, path);
#define NEED_err_unix_symlink_unsupported
#endif /* EPERM */
#ifdef EROFS
	if (errno_value == EROFS)
		return err_unix_path_readonly(errno_value, path);
#define NEED_err_unix_path_readonly
#endif /* EROFS */
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError, errno_value,
	                                 "Failed to create a symbolic link %r at %r",
	                                 text, path);
}
#endif /* NEED_err_unix_symlink */

#ifdef NEED_err_nt_symlink
#undef NEED_err_nt_symlink
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_symlink(DWORD dwError, DeeObject *text, DeeObject *path) {
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return err_nt_path_readonly(dwError, path);
#define NEED_err_nt_path_readonly
	if (DeeNTSystem_IsExists(dwError))
		return err_nt_path_exists(dwError, path);
#define NEED_err_nt_path_exists
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return err_nt_path_not_found(dwError, path);
#define NEED_err_nt_path_not_found
	if (DeeNTSystem_IsNotDir(dwError))
		return err_nt_path_not_dir(dwError, path);
#define NEED_err_nt_path_not_dir
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return err_nt_symlink_unsupported(dwError, text, path);
#define NEED_err_nt_symlink_unsupported
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to create a symbolic link %r at %r",
	                               text, path);
}
#endif /* NEED_err_nt_symlink */

#ifdef NEED_err_unix_truncate
#undef NEED_err_unix_truncate
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_truncate(int errno_value, DeeObject *path, DeeObject *length) {
#ifdef ENOENT
	if (errno_value == ENOENT)
		return err_unix_path_not_found(errno_value, path);
#define NEED_err_unix_path_not_found
#endif /* ENOENT */
#ifdef ENOTDIR
	if (errno_value == ENOTDIR)
		return err_unix_path_not_dir(errno_value, path);
#define NEED_err_unix_path_not_dir
#endif /* ENOTDIR */
#ifdef EACCES
	if (errno_value == EACCES)
		return err_unix_path_no_access(errno_value, path);
#define NEED_err_unix_path_no_access
#endif /* EACCES */
#if defined(EFBIG) || defined(EINVAL)
	DeeSystem_IF_E2(errno_value, EFBIG, EINVAL, {
		return err_unix_truncate_fbig(errno_value, path, length);
	});
#define NEED_err_unix_truncate_fbig
#endif /* EFBIG || EINVAL */
#if defined(ENXIO) || defined(EISDIR)
	DeeSystem_IF_E2(errno_value, ENXIO, EISDIR, {
		return err_unix_truncate_isdir(errno_value, path);
	});
#define NEED_err_unix_truncate_isdir
#endif /* ENXIO || EISDIR */
#ifdef ETXTBSY
	if (errno_value == ETXTBSY)
		return err_unix_truncate_txtbusy(errno_value, path);
#define NEED_err_unix_truncate_txtbusy
#endif /* ETXTBSY */
#ifdef EROFS
	if (errno_value == EROFS)
		return err_unix_path_readonly(errno_value, path);
#define NEED_err_unix_path_readonly
#endif /* EROFS */
	return DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, errno_value,
	                                 "Failed to truncate path %r to length %r",
	                                 path, length);
}
#endif /* NEED_err_unix_truncate */

#ifdef NEED_err_unix_ftruncate
#undef NEED_err_unix_ftruncate
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_ftruncate(int errno_value, int fd, DeeObject *length) {
#if defined(EFBIG) || defined(EINVAL)
	DeeSystem_IF_E2(errno_value, EFBIG, EINVAL, {
		return err_unix_ftruncate_fbig(errno_value, fd, length);
	});
#define NEED_err_unix_ftruncate_fbig
#endif /* EFBIG || EINVAL */
#if defined(ENXIO) || defined(EISDIR)
	DeeSystem_IF_E2(errno_value, ENXIO, EISDIR, {
		return err_unix_ftruncate_isdir(errno_value, fd);
	});
#define NEED_err_unix_ftruncate_isdir
#endif /* ENXIO || EISDIR */
#ifdef ETXTBSY
	if (errno_value == ETXTBSY)
		return err_unix_ftruncate_txtbusy(errno_value, fd);
#define NEED_err_unix_ftruncate_txtbusy
#endif /* ETXTBSY */
#ifdef EBADF
	if (errno_value == EBADF)
		return err_unix_file_closed(errno_value, fd);
#define NEED_err_unix_file_closed
#endif /* EBADF */
	return DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, errno_value,
	                                 "Failed to truncate fd %d to length %r",
	                                 fd, length);
}
#endif /* NEED_err_unix_ftruncate */

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
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_rmdir_unsupported(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the path %r does "
	                                 "not support the removal of directories",
	                                 path);
}
#endif /* NEED_err_unix_rmdir_unsupported */

#ifdef NEED_err_unix_mkdir_unsupported
#undef NEED_err_unix_mkdir_unsupported
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_mkdir_unsupported(int errno_value, DeeObject *__restrict path, unsigned int mode) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the path %r does not "
	                                 "support the creation of directories with mode %#x",
	                                 path, mode);
}
#endif /* NEED_err_unix_mkdir_unsupported */

#ifdef NEED_err_nt_rmdir_unsupported
#undef NEED_err_nt_rmdir_unsupported
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_rmdir_unsupported(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
	                               "The filesystem hosting the path %r does "
	                               "not support the removal of directories",
	                               path);
}
#endif /* NEED_err_nt_rmdir_unsupported */

#ifdef NEED_err_nt_mkdir_unsupported
#undef NEED_err_nt_mkdir_unsupported
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_mkdir_unsupported(DWORD dwError, DeeObject *__restrict path, unsigned int mode) {
	return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
	                               "The filesystem hosting the path %r does not "
	                               "support the creation of directories with mode %#x",
	                               path, mode);
}
#endif /* NEED_err_nt_mkdir_unsupported */

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

#ifdef NEED_err_unix_link_unsupported
#undef NEED_err_unix_link_unsupported
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_link_unsupported(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the paths %r and %r "
	                                 "does not support creation of hardlinks",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_link_unsupported */

#ifdef NEED_err_unix_symlink_unsupported
#undef NEED_err_unix_symlink_unsupported
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_symlink_unsupported(int errno_value, DeeObject *text, DeeObject *path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_UnsupportedAPI, errno_value,
	                                 "The filesystem hosting the path %r does not "
	                                 "support creation of symbolic link with text %r",
	                                 path, text);
}
#endif /* NEED_err_unix_symlink_unsupported */

#ifdef NEED_err_nt_link_unsupported
#undef NEED_err_nt_link_unsupported
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_link_unsupported(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
	                               "The filesystem hosting the paths %r and %r "
	                               "does not support creation of hardlinks",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_link_unsupported */

#ifdef NEED_err_nt_symlink_unsupported
#undef NEED_err_nt_symlink_unsupported
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_symlink_unsupported(DWORD dwError, DeeObject *text, DeeObject *path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
	                               "The filesystem hosting the path %r does not "
	                               "support creation of symbolic link with text %r",
	                               path, text);
}
#endif /* NEED_err_nt_symlink_unsupported */

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
err_unix_path_not_found2(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, errno_value,
	                                 "Path %r or %r could not be found",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_not_found2 */

#ifdef NEED_err_nt_path_not_found2
#undef NEED_err_nt_path_not_found2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_not_found2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
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
err_unix_path_no_access2(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, errno_value,
	                                 "Access to %r or %r has not been granted",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_no_access2 */

#ifdef NEED_err_nt_path_no_access2
#undef NEED_err_nt_path_no_access2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_no_access2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
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
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_path_readonly2(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
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
err_unix_path_cross_dev2(int errno_value, DeeObject *existing_path, DeeObject *new_path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_CrossDeviceLink, errno_value,
	                                 "Paths %r and %r are not apart of the same filesystem",
	                                 existing_path, new_path);
}
#endif /* NEED_err_unix_path_cross_dev2 */

#ifdef NEED_err_nt_path_cross_dev2
#undef NEED_err_nt_path_cross_dev2
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_nt_path_cross_dev2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_CrossDeviceLink, dwError,
	                               "Paths %r and %r are not apart of the same filesystem",
	                               existing_path, new_path);
}
#endif /* NEED_err_nt_path_cross_dev2 */

#ifdef NEED_err_nt_path_not_link
#undef NEED_err_nt_path_not_link
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_nt_path_not_link(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_NoSymlink, dwError,
	                               "Path %r is not a symbolic link",
	                               path);
}
#endif /* NEED_err_nt_path_not_link */

#ifdef NEED_err_unix_ftruncate_fbig
#undef NEED_err_unix_ftruncate_fbig
INTERN ATTR_COLD NONNULL((3)) int DCALL
err_unix_ftruncate_fbig(int errno_value, int fd, DeeObject *length) {
	int result = -1;
	DREF DeeObject *fd_path;
	fd_path = posix_fd_makepath_fd(fd);
#define NEED_posix_fd_makepath_fd
	if likely(fd_path) {
		result = err_unix_truncate_fbig(errno_value, fd_path, length);
#define NEED_err_unix_truncate_fbig
		Dee_Decref(fd_path);
	}
	return result;
}
#endif /* NEED_err_unix_ftruncate_fbig */

#ifdef NEED_err_unix_truncate_fbig
#undef NEED_err_unix_truncate_fbig
INTERN ATTR_COLD NONNULL((2, 3)) int DCALL
err_unix_truncate_fbig(int errno_value, DeeObject *path, DeeObject *length) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_IntegerOverflow, errno_value,
	                                 "Cannot truncate path %r: length %r is too large",
	                                 path, length);
}
#endif /* NEED_err_unix_truncate_fbig */

#ifdef NEED_err_unix_ftruncate_isdir
#undef NEED_err_unix_ftruncate_isdir
INTERN ATTR_COLD int DCALL
err_unix_ftruncate_isdir(int errno_value, int fd) {
	int result = -1;
	DREF DeeObject *fd_path;
	fd_path = posix_fd_makepath_fd(fd);
#define NEED_posix_fd_makepath_fd
	if likely(fd_path) {
		result = err_unix_truncate_isdir(errno_value, fd_path);
#define NEED_err_unix_truncate_isdir
		Dee_Decref(fd_path);
	}
	return result;
}
#endif /* NEED_err_unix_ftruncate_isdir */

#ifdef NEED_err_unix_truncate_isdir
#undef NEED_err_unix_truncate_isdir
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_truncate_isdir(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_IntegerOverflow, errno_value,
	                                 "Cannot truncate directory %r",
	                                 path);
}
#endif /* NEED_err_unix_truncate_isdir */

#ifdef NEED_err_unix_ftruncate_txtbusy
#undef NEED_err_unix_ftruncate_txtbusy
INTERN ATTR_COLD int DCALL
err_unix_ftruncate_txtbusy(int errno_value, int fd) {
	int result = -1;
	DREF DeeObject *fd_path;
	fd_path = posix_fd_makepath_fd(fd);
#define NEED_posix_fd_makepath_fd
	if likely(fd_path) {
		result = err_unix_truncate_txtbusy(errno_value, fd_path);
#define NEED_err_unix_truncate_txtbusy
		Dee_Decref(fd_path);
	}
	return result;
}
#endif /* NEED_err_unix_ftruncate_txtbusy */

#ifdef NEED_err_unix_truncate_txtbusy
#undef NEED_err_unix_truncate_txtbusy
INTERN ATTR_COLD NONNULL((2)) int DCALL
err_unix_truncate_txtbusy(int errno_value, DeeObject *__restrict path) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, errno_value,
	                                 "Cannot truncate %r because that file is currently being executed",
	                                 path);
}
#endif /* NEED_err_unix_truncate_txtbusy */

#ifdef NEED_err_unix_file_closed
#undef NEED_err_unix_file_closed
INTERN ATTR_COLD int DCALL
err_unix_file_closed(int errno_value, int fd) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_FileClosed, errno_value,
	                                 "Invalid fd %d", fd);
}
#endif /* NEED_err_unix_file_closed */



#ifdef NEED_nt_GetTempPath
#undef NEED_nt_GetTempPath
INTERN WUNUSED DREF DeeObject *DCALL
nt_GetTempPath(void) {
	LPWSTR lpwBuffer, lpwNewBuffer;
	DWORD dwBufsize = 256, dwError;
	lpwBuffer = DeeString_NewWideBuffer(dwBufsize);
	if unlikely(!lpwBuffer)
		goto err;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetTempPathW(dwBufsize + 1, lpwBuffer);
		if (!dwError) {
			/* Error. */
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to lookup the path for tmp");
			goto err_result;
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwError <= dwBufsize)
			break;
		/* Resize to fit. */
		lpwNewBuffer = DeeString_ResizeWideBuffer(lpwBuffer, dwError);
		if unlikely(!lpwNewBuffer)
			goto err_result;
		lpwBuffer  = lpwNewBuffer;
		dwBufsize = dwError;
	}
	lpwBuffer = DeeString_TruncateWideBuffer(lpwBuffer, dwError);
	return DeeString_PackWideBuffer(lpwBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpwBuffer);
err:
	return NULL;
}
#endif /* NEED_nt_GetTempPath */


#ifdef NEED_nt_GetComputerName
#undef NEED_nt_GetComputerName
INTERN WUNUSED DREF DeeObject *DCALL
nt_GetComputerName(void) {
	DWORD dwBufsize = MAX_COMPUTERNAME_LENGTH + 1;
	LPWSTR lpwBuffer, lpwNewBuffer;
	if (DeeThread_CheckInterrupt())
		goto err;
	lpwBuffer = DeeString_NewWideBuffer(dwBufsize - 1);
	if unlikely(!lpwBuffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	if (!GetComputerNameW(lpwBuffer, &dwBufsize)) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError == ERROR_BUFFER_OVERFLOW && dwBufsize &&
		    dwBufsize - 1 > WSTR_LENGTH(lpwBuffer)) {
			lpwNewBuffer = DeeString_ResizeWideBuffer(lpwBuffer, dwBufsize - 1);
			if unlikely(!lpwNewBuffer)
				goto err_result;
			lpwBuffer = lpwNewBuffer;
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
	lpwBuffer = DeeString_TruncateWideBuffer(lpwBuffer, dwBufsize);
	return DeeString_PackWideBuffer(lpwBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpwBuffer);
err:
	return NULL;
}
#endif /* NEED_nt_GetComputerName */


#ifdef NEED_nt_SetCurrentDirectory
#undef NEED_nt_SetCurrentDirectory
/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_SetCurrentDirectory(DeeObject *__restrict lpPathName) {
	LPWSTR lpwName;
	BOOL bOK;
	DWORD dwError;
	lpwName = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!lpwName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = SetCurrentDirectoryW(lpwName);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = DeeNTSystem_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		lpwName = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!lpwName) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		bOK     = SetCurrentDirectoryW(lpwName);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
err:
	return -1;
}
#endif /* NEED_nt_SetCurrentDirectory */


#ifdef NEED_nt_CreateSymbolicLinkAuto
#undef NEED_nt_CreateSymbolicLinkAuto

#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x1
#endif /* !SYMBOLIC_LINK_FLAG_DIRECTORY */

#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
#endif /* !SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE */

PRIVATE DWORD nt_symlink_dwSymlinkAdditionalFlags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
PRIVATE BOOL nt_symlink_bHoldingSymlinkPriv       = FALSE;

/* Used to determined `SYMBOLIC_LINK_FLAG_DIRECTORY':
 * >> local abs_lpTargetFileName = lpTargetFileName;
 * >> if (!posix.isabs(lpTargetFileName))
 * >>     abs_lpTargetFileName = posix.abspath(lpTargetFileName, posix.headof(lpSymlinkFileName));
 * >> if (posix.stat.isdir(abs_lpTargetFileName))
 * >>     dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
 * @return: 1 : Is a directory
 * @return: 0 : Not a directory (or file doesn't exist)
 * @return: -1: Error */
PRIVATE NONNULL((1, 2)) int DCALL
nt_CreateSymbolicLinkAuto_isdir(DeeObject *lpSymlinkFileName,
                                DeeObject *lpTargetFileName) {
	int attribute_error;
	DWORD dwTargetAttributes;
	if (!DeeString_IsAbsPath(lpTargetFileName)) {
		DREF DeeObject *filename_path, *target_abspath;
		filename_path = posix_path_headof_f(lpSymlinkFileName);
		if unlikely(!filename_path)
			goto err;
		target_abspath = posix_path_abspath_f(lpTargetFileName, filename_path);
		Dee_Decref(filename_path);
		if unlikely(!target_abspath)
			goto err;
		attribute_error = nt_GetFileAttributes(target_abspath, &dwTargetAttributes);
		Dee_Decref(target_abspath);
	} else {
		attribute_error = nt_GetFileAttributes(lpTargetFileName, &dwTargetAttributes);
	}
#define NEED_nt_GetFileAttributes
	if unlikely(attribute_error < 0)
		goto err;
	if (attribute_error == 0 && (dwTargetAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 1; /* It's a directory, all right! */
	return 0; /* Default: not a directory */
err:
	return -1;
}

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
		goto fail;
	error = GetLastError();
	SetLastError(0);
	return unlikely(error == ERROR_NOT_ALL_ASSIGNED) ? 0 : 1;
fail:
	return FALSE;
}

/* Same as `nt_CreateSymbolicLink()', but automatically determine proper `dwFlags'
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN NONNULL((1, 2)) int DCALL
nt_CreateSymbolicLinkAuto(DeeObject *lpSymlinkFileName,
                          DeeObject *lpTargetFileName) {
	int error;
	DWORD dwFlags;
	dwFlags = atomic_read(&nt_symlink_dwSymlinkAdditionalFlags);

	/* Figure out of the target is a directory. */
	error = nt_CreateSymbolicLinkAuto_isdir(lpSymlinkFileName, lpTargetFileName);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
again:
	error = nt_CreateSymbolicLink(lpSymlinkFileName, lpTargetFileName, dwFlags);
#define NEED_nt_CreateSymbolicLink
	if (error > 1) {
		/* Check if we might be able to fix this error. */
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (error == ERROR_INVALID_PARAMETER && (dwFlags & SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) {
			/* Older versions of windows didn't accept this flag. */
			atomic_and(&nt_symlink_dwSymlinkAdditionalFlags, ~SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE);
			dwFlags &= ~SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
			goto again;
		}

		/* Try to acquire the ~privilege~ to create symbolic links. */
		if (error == ERROR_PRIVILEGE_NOT_HELD) {
			if (!nt_symlink_bHoldingSymlinkPriv) {
				DBG_ALIGNMENT_DISABLE();
				if (nt_AcquirePrivilege(str_SeCreateSymbolicLinkPrivilege)) {
					DBG_ALIGNMENT_ENABLE();
					nt_symlink_bHoldingSymlinkPriv = TRUE;
					goto again;
				}
				DBG_ALIGNMENT_ENABLE();
			}

			/* May as well not exist at all... */
			return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, dwError,
			                               "The operating system has restricted "
			                               "access to symlink functionality");
		}
	}
	return error;
err:
	return -1;
}
#endif /* NEED_nt_CreateSymbolicLinkAuto */


#ifdef NEED_nt_GetFileAttributesEx
#undef NEED_nt_GetFileAttributesEx
/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation) {
	LPWSTR lpwName;
	BOOL bOK;
	DWORD dwError;
	lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!lpwName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = GetFileAttributesExW(lpwName, fInfoLevelId, lpFileInformation);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!lpwName) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		bOK     = GetFileAttributesExW(lpwName, fInfoLevelId, lpFileInformation);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
err:
	return -1;
}
#endif /* NEED_nt_GetFileAttributesEx */

#ifdef NEED_nt_GetFileAttributes
#undef NEED_nt_GetFileAttributes
/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict p_result) {
	LPWSTR lpwName;
	DWORD dwError;
	lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!lpwName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	*p_result = GetFileAttributesW(lpwName);
	if ((*p_result == INVALID_FILE_ATTRIBUTES) && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!lpwName) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		*p_result = GetFileAttributesW(lpwName);
		dwError  = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
	}
	DBG_ALIGNMENT_ENABLE();
	return *p_result == INVALID_FILE_ATTRIBUTES;
err:
	return -1;
}
#endif /* NEED_nt_GetFileAttributes */


#ifdef NEED_nt_SetFileAttributes
#undef NEED_nt_SetFileAttributes
/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes) {
	LPWSTR lpwName;
	BOOL bOK;
	DWORD dwError;
	lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!lpwName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = SetFileAttributesW(lpwName, dwFileAttributes);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!lpwName) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		bOK     = SetFileAttributesW(lpwName, dwFileAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
err:
	return -1;
}
#endif /* NEED_nt_SetFileAttributes */


#ifdef NEED_nt_CreateDirectory
#undef NEED_nt_CreateDirectory
/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	LPWSTR lpwName;
	BOOL bOK;
	DWORD dwError;
	lpwName = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!lpwName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = CreateDirectoryW(lpwName, lpSecurityAttributes);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = DeeNTSystem_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		lpwName = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!lpwName) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		bOK     = CreateDirectoryW(lpwName, lpSecurityAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
err:
	return -1;
}
#endif /* NEED_nt_CreateDirectory */

#ifdef NEED_nt_RemoveDirectory
#undef NEED_nt_RemoveDirectory
/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName) {
	LPWSTR lpwName;
	BOOL bOK;
	DWORD dwError;
	lpwName = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!lpwName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = RemoveDirectoryW(lpwName);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = DeeNTSystem_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		lpwName = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!lpwName) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		bOK     = RemoveDirectoryW(lpwName);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
err:
	return -1;
}
#endif /* NEED_nt_RemoveDirectory */

#ifdef NEED_nt_DeleteFile
#undef NEED_nt_DeleteFile
/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1)) int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName) {
	LPWSTR lpwName;
	BOOL bOK;
	DWORD dwError;
	lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!lpwName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = DeleteFileW(lpwName);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!lpwName) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		bOK     = DeleteFileW(lpwName);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
err:
	return -1;
}
#endif /* NEED_nt_DeleteFile */

#ifdef NEED_nt_MoveFileEx
#undef NEED_nt_MoveFileEx
/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
nt_MoveFileEx(DeeObject *lpExistingFileName,
              DeeObject *lpNewFileName,
              DWORD dwFlags) {
	LPWSTR lpwExistingFileName;
	LPWSTR lpwNewFileName;
	BOOL bOK;
	DWORD dwError;
	lpwExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
	if unlikely(!lpwExistingFileName)
		goto err;
	lpwNewFileName = (LPWSTR)DeeString_AsWide(lpNewFileName);
	if unlikely(!lpwNewFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = MoveFileExW(lpwExistingFileName, lpwNewFileName, dwFlags);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpExistingFileName = DeeNTSystem_FixUncPath(lpExistingFileName);
		if unlikely(!lpExistingFileName)
			goto err;
		lpNewFileName = DeeNTSystem_FixUncPath(lpNewFileName);
		if unlikely(!lpNewFileName)
			goto err_existing;
		lpwExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
		if unlikely(!lpwExistingFileName)
			goto err_new;
		lpwNewFileName = (LPWSTR)DeeString_AsWide(lpNewFileName);
		if unlikely(!lpwNewFileName)
			goto err_new;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		bOK     = MoveFileExW(lpwExistingFileName, lpwNewFileName, dwFlags);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpNewFileName);
		Dee_Decref(lpExistingFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
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
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
nt_CreateHardLink(DeeObject *lpFileName,
                  DeeObject *lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	LPWSTR lpwFileName, lpwExistingFileName;
	BOOL bOK;
	DWORD dwError;
	lpwFileName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!lpwFileName)
		goto err;
	lpwExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
	if unlikely(!lpwExistingFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = CreateHardLinkW(lpwFileName, lpwExistingFileName, lpSecurityAttributes);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = DeeNTSystem_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		lpExistingFileName = DeeNTSystem_FixUncPath(lpExistingFileName);
		if unlikely(!lpExistingFileName)
			goto err_filename;
		lpwFileName = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!lpwFileName)
			goto err_existing;
		lpwExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
		if unlikely(!lpwExistingFileName)
			goto err_existing;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		bOK     = CreateHardLinkW(lpwFileName, lpwExistingFileName, lpSecurityAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpExistingFileName);
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !bOK;
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
typedef BOOLEAN (APIENTRY *LPCREATESYMBOLICLINKW)(LPCWSTR lpSymlinkFileName,
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
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTERN NONNULL((1, 2)) int DCALL
nt_CreateSymbolicLink(DeeObject *lpSymlinkFileName,
                      DeeObject *lpTargetFileName,
                      DWORD dwFlags) {
	LPWSTR lpwSymlinkFileName;
	LPWSTR lpwTargetFileName;
	BOOLEAN bOK;
	DWORD dwError;
	LPCREATESYMBOLICLINKW lpCallback = pCreateSymbolicLinkW;
	if (!lpCallback) {
		DBG_ALIGNMENT_DISABLE();
		*(FARPROC *)&lpCallback = GetProcAddress(GetModuleHandleW(wKernel32),
		                                         "CreateSymbolicLinkW");
		DBG_ALIGNMENT_ENABLE();
		if (!lpCallback)
			*(void **)&lpCallback = (void *)(uintptr_t)-1;
		*(void **)&pCreateSymbolicLinkW = *(void **)&lpCallback;
	}
	if (*(void **)&lpCallback == (void *)(uintptr_t)-1) {
		DeeError_Throwf(&DeeError_UnsupportedAPI,
		                "The host operating system does "
		                "not implement symbolic links");
		goto err;
	}
	lpwSymlinkFileName = (LPWSTR)DeeString_AsWide(lpSymlinkFileName);
	if unlikely(!lpwSymlinkFileName)
		goto err;
	lpwTargetFileName = (LPWSTR)DeeString_AsWide(lpTargetFileName);
	if unlikely(!lpwTargetFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	bOK = (*lpCallback)(lpwSymlinkFileName, lpwTargetFileName, dwFlags);
	if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpSymlinkFileName = DeeNTSystem_FixUncPath(lpSymlinkFileName);
		if unlikely(!lpSymlinkFileName)
			goto err;
		lpwSymlinkFileName = (LPWSTR)DeeString_AsWide(lpSymlinkFileName);
		if unlikely(!lpwSymlinkFileName)
			goto err_filename;

		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		bOK     = (*lpCallback)(lpwSymlinkFileName, lpwTargetFileName, dwFlags);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();

		if (!bOK && DeeNTSystem_IsUncError(GetLastError())) {
			lpTargetFileName = DeeNTSystem_FixUncPath(lpTargetFileName);
			if unlikely(!lpTargetFileName)
				goto err_filename;
			lpwTargetFileName = (LPWSTR)DeeString_AsWide(lpTargetFileName);
			if unlikely(!lpwTargetFileName)
				goto err_existing;

			/* Invoke the system call once again. */
			DBG_ALIGNMENT_DISABLE();
			bOK     = (*lpCallback)(lpwSymlinkFileName, lpwTargetFileName, dwFlags);
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			Dee_Decref(lpTargetFileName);
		}
		Dee_Decref(lpSymlinkFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return bOK ? 0 : 1;
err_existing:
	Dee_Decref(lpTargetFileName);
err_filename:
	Dee_Decref(lpSymlinkFileName);
err:
	return -1;
}
#endif /* NEED_nt_CreateSymbolicLink */


#ifdef NEED_posix_fd_openfile
#undef NEED_posix_fd_openfile
/* Open a HANDLE/fd-compatible object as a `File' */
INTERN WUNUSED NONNULL((1)) /*File*/ DREF DeeObject *DCALL
posix_fd_openfile(DeeObject *__restrict fd, int oflags) {
	DREF DeeObject *result;
	if (DeeFile_Check(fd)) {
		result = fd;
		Dee_Incref(result);
	} else {
#if defined(CONFIG_HOST_WINDOWS) && defined(Dee_fd_t_IS_HANDLE)
		HANDLE hFile;
		hFile = DeeNTSystem_GetHandle(fd);
		if unlikely(hFile == INVALID_HANDLE_VALUE)
			goto err;
		result = DeeFile_OpenFd((Dee_fd_t)hFile, NULL, oflags, false);
#elif defined(Dee_fd_t_IS_int)
		int os_fd;
		os_fd = DeeUnixSystem_GetFD(fd);
		if unlikely(os_fd == -1)
			goto err;
		result = DeeFile_OpenFd((Dee_fd_t)os_fd, NULL, oflags, false);
#else /* ... */
		DREF DeeObject *fd_path;
		fd_path = posix_fd_makepath(fd);
#define NEED_posix_fd_makepath
		if unlikely(!fd_path)
			goto err;
		result = DeeFile_Open(fd_path, oflags, 0);
		if unlikely(result == ITER_DONE) {
			DeeError_Throwf(&DeeError_FileNotFound, "File %r could not be found", fd_path);
			Dee_Decref(fd_path);
			goto err_src_file;
		}
		Dee_Decref(fd_path);
#endif /* !... */
	}
	return result;
err:
	return NULL;
}
#endif /* NEED_posix_fd_openfile */


#ifdef NEED_posix_copyfile_fileio
#undef NEED_posix_copyfile_fileio

/* Figure out if we want to support `sendfile(2)' in `posix_copyfile_fileio()' */
#undef HAVE_posix_copyfile_fileio_sendfile
#if defined(CONFIG_HAVE_sendfile) && defined(Dee_fd_t_IS_int)
#define HAVE_posix_copyfile_fileio_sendfile
#endif /* CONFIG_HAVE_sendfile && Dee_fd_t_IS_int */

/* Figure out if we want to skip `sendfile(2)' if it ever indicatse ENOSYS */
#undef HAVE_posix_copyfile_fileio_sendfile_ENOSYS
#if defined(HAVE_posix_copyfile_fileio_sendfile) && defined(ENOSYS) && !defined(__OPTIMIZE_SIZE__)
#define HAVE_posix_copyfile_fileio_sendfile_ENOSYS
#endif /* HAVE_posix_copyfile_fileio_sendfile && ENOSYS && !__OPTIMIZE_SIZE__ */

#ifdef HAVE_posix_copyfile_fileio_sendfile_ENOSYS
/* When set to true, don't attempt to use `sendfile(2)' in `posix_copyfile_fileio()' */
PRIVATE bool host_sendfile_is_ENOSYS = false;
#endif /* HAVE_posix_copyfile_fileio_sendfile_ENOSYS */

/* The max buffer limit for sendfile(2), as documented here:
 * https://man7.org/linux/man-pages/man2/sendfile.2.html */
#ifndef LINUX_SENDFILE_MAXCOUNT
#define LINUX_SENDFILE_MAXCOUNT 0x7ffff000
#endif /* !LINUX_SENDFILE_MAXCOUNT */

/* Copy all data from `src' to `dst', both of with are deemon File objects.
 * @param: src_mmap_hints: Set of `0 | DEE_MAPFILE_F_ATSTART'
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
posix_copyfile_fileio(/*File*/ DeeObject *src,
                      /*File*/ DeeObject *dst,
                      DeeObject *progress,
                      DeeObject *bufsize,
                      unsigned int src_mmap_hints) {
	uint64_t transfer_total = 0;
	DREF DeeCopyFileProgressObject *progress_info = NULL;
	size_t used_bufsize;
	struct DeeMapFile mf;
	int mf_status;

	/* Figure out the intended buffer size. */
	used_bufsize = POSIX_COPYFILE_DEFAULT_IO_BUFSIZE;
	if (!DeeNone_Check(bufsize)) {
		if (DeeObject_AsSize(bufsize, &used_bufsize))
			goto err;
		if unlikely(used_bufsize == 0) {
err_bad_used_bufsize:
			err_bad_copyfile_bufsize_is_zero();
#define NEED_err_bad_copyfile_bufsize_is_zero
			goto err_progress_info;
		}
	}

	/* Try to use `sendfile()' */
#ifdef HAVE_posix_copyfile_fileio_sendfile
	if (DeeSystemFile_Check(src) && DeeSystemFile_Check(dst) &&
#ifdef HAVE_posix_copyfile_fileio_sendfile_ENOSYS
	    (!host_sendfile_is_ENOSYS) &&
#endif /* HAVE_posix_copyfile_fileio_sendfile_ENOSYS */
	    1) {
		Dee_fd_t src_fd = DeeSystemFile_GetHandle(src);
		Dee_fd_t dst_fd = DeeSystemFile_GetHandle(dst);
		Dee_ssize_t sendfile_status;
		size_t sendfile_iosize = used_bufsize;
		if (DeeNone_Check(bufsize))
			sendfile_iosize = POSIX_COPYFILE_DEFAULT_SENDFILE_BUFSIZE;
		if unlikely(sendfile_iosize > LINUX_SENDFILE_MAXCOUNT)
			sendfile_iosize = LINUX_SENDFILE_MAXCOUNT;
		sendfile_status = sendfile(dst_fd, src_fd, NULL, sendfile_iosize);
		if (sendfile_status < 0) {
#ifdef HAVE_posix_copyfile_fileio_sendfile_ENOSYS
			/* If `sendfile(2)' indicates that it isn't implemented
			 * by the kernel, then don't *ever* try to use it again. */
			if (DeeSystem_GetErrno() == ENOSYS)
				host_sendfile_is_ENOSYS = true;
#endif /* HAVE_posix_copyfile_fileio_sendfile_ENOSYS */
		} else {
			/* Keep copying data using `sendfile' until we're done. */
			transfer_total = (size_t)sendfile_status;
			if (sendfile_status == 0)
				goto done; /* Input file was empty. */
#define WANT_done
			for (;;) {
				if (DeeThread_CheckInterrupt())
					goto err_progress_info;

				/* If given (and not `none'), invoke the progress-callback with copy status information. */
				if (!DeeNone_Check(progress)) {
					DREF DeeObject *progress_status;
					if (progress_info == NULL) {
						progress_info = DeeObject_MALLOC(DeeCopyFileProgressObject);
						if unlikely(!progress_info)
							goto err_progress_info;
						Dee_Incref(src);
						Dee_Incref(dst);
						progress_info->cfp_srcfile = src; /* Inherit reference */
						progress_info->cfp_dstfile = dst; /* Inherit reference */
						progress_info->cfp_total   = (uint64_t)-1;
						progress_info->cfp_bufsize = sendfile_iosize;
						DeeObject_Init(progress_info, &DeeCopyFileProgress_Type);
					}

					/* Update the copied-bytes counter of the progress info object. */
					progress_info->cfp_copied = transfer_total;

					/* Invoke the progress-info callback. */
					progress_status = DeeObject_Call(progress, 1, (DeeObject *const *)&progress_info);
					if unlikely(!progress_status)
						goto err_progress_info;
					Dee_Decref(progress_status);
					sendfile_iosize = progress_info->cfp_bufsize;
					COMPILER_READ_BARRIER();
					if unlikely(sendfile_iosize == 0)
						goto err_bad_used_bufsize;
					if unlikely(sendfile_iosize > LINUX_SENDFILE_MAXCOUNT)
						sendfile_iosize = LINUX_SENDFILE_MAXCOUNT;
				} /* if (!DeeNone_Check(progress)) */

				/* Send the next chunk */
				sendfile_status = sendfile(dst_fd, src_fd, NULL, sendfile_iosize);
				if (sendfile_status == 0)
					goto done; /* Done copying data! */
				if (sendfile_status < 0)
					break; /* sendfile(2) reported an error -> try to copy the rest using file I/O */
				transfer_total += sendfile_status;
			}

			/* Since we're going to copy remaining data via file I/O, we need
			 * to switch the used buffer size down to the file I/O default, or
			 * inherit a new custom I/O size, as set by the caller, or the
			 * progress-callback. */
			if (progress_info != NULL) {
				if (DeeNone_Check(bufsize))
					progress_info->cfp_bufsize = used_bufsize; /* Restore original default buffer size */
				used_bufsize = progress_info->cfp_bufsize;     /* Inherit actively set buffer size. */
			}
		} /* if (sendfile_status >= 0) */
	}     /* if (DeeSystemFile_Check(src) && DeeSystemFile_Check(dst)) */
#endif /* HAVE_posix_copyfile_fileio_sendfile */

	/* Secondly: try to mmap the source file, so we can try to use kernel I/O buffers.
	 * -> There is a chance that the kernel will notice us directly copying the I/O
	 *    buffer of one file into that of another file, which might allow it to perform
	 *    the copy operations more quickly. */
#ifdef HAVE_posix_copyfile_fileio_sendfile
	if (src_mmap_hints & DEE_MAPFILE_F_ATSTART) {
		/* Special handling needed for when the caller originally indicated
		 * that the source-file was located at its beginning, yet due to us
		 * possibly having been able to copy *some* file data using `sendfile',
		 * that might no longer be the case.
		 * 
		 * Instead, in this situation we can assume that the source-file's
		 * file pointer is currently located at `transfer_total' bytes, so
		 * we only have to map all of the file's contents starting from that
		 * byte-offset. */
		mf_status = DeeMapFile_InitFile(&mf, src, transfer_total,
		                                0, (size_t)-1, 0,
		                                DEE_MAPFILE_F_MUSTMMAP |
		                                DEE_MAPFILE_F_TRYMMAP);
	} else
#endif /* HAVE_posix_copyfile_fileio_sendfile */
	{
		/* Map the remainder of the given `src' file into memory. */
		mf_status = DeeMapFile_InitFile(&mf, src, (Dee_pos_t)-1,
		                                0, (size_t)-1, 0,
		                                DEE_MAPFILE_F_MUSTMMAP |
		                                DEE_MAPFILE_F_TRYMMAP |
		                                src_mmap_hints);
	}
	if (mf_status <= 0) {
		byte_t const *iter, *end;
		if unlikely(mf_status < 0)
			goto err_progress_info;

		/* Was able to mmap() the source file -> now to write that mapping into the target file. */
		iter = (byte_t const *)DeeMapFile_GetBase(&mf);
		end  = iter + DeeMapFile_GetSize(&mf);
		while (iter < end) {
			size_t ok, remaining, chunk_size;
			size_t local_used_bufsize;
			if (DeeThread_CheckInterrupt())
				goto err_progress_info_mapfile;
			local_used_bufsize = used_bufsize;
			if (progress_info != NULL) {
				local_used_bufsize = progress_info->cfp_bufsize;
				COMPILER_READ_BARRIER();
				if unlikely(local_used_bufsize == 0)
					goto err_bad_used_bufsize;
			}
			remaining  = (size_t)(end - iter);
			chunk_size = local_used_bufsize;
			if (chunk_size > remaining)
				chunk_size = remaining;
			ok = DeeFile_WriteAll(dst, iter, chunk_size);
			if unlikely(ok == (size_t)-1)
				goto err_progress_info_mapfile;
			ASSERT(ok <= chunk_size);
			iter += ok;
			if (ok < chunk_size) {
				DeeError_Throwf(&DeeError_FileClosed,
				                "Target file %k indicates EOF after "
				                "%" PRFuSIZ " of %" PRFuSIZ " bytes",
				                dst,
				                (size_t)(iter - (byte_t const *)DeeMapFile_GetBase(&mf)),
				                DeeMapFile_GetSize(&mf));
err_progress_info_mapfile:
				DeeMapFile_Fini(&mf);
				goto err_progress_info;
			}

			/* If given (and not `none'), invoke the progress-callback with copy status information. */
			if (!DeeNone_Check(progress)) {
				DREF DeeObject *progress_status;
				if (iter >= end)
					break;
				if (progress_info == NULL) {
					progress_info = DeeObject_MALLOC(DeeCopyFileProgressObject);
					if unlikely(!progress_info)
						goto err_progress_info_mapfile;
					Dee_Incref(src);
					Dee_Incref(dst);
					progress_info->cfp_srcfile = src; /* Inherit reference */
					progress_info->cfp_dstfile = dst; /* Inherit reference */
					progress_info->cfp_total   = transfer_total + DeeMapFile_GetSize(&mf);
					progress_info->cfp_bufsize = used_bufsize;
					DeeObject_Init(progress_info, &DeeCopyFileProgress_Type);
				}

				/* Update the copied-bytes counter of the progress info object. */
				progress_info->cfp_copied = transfer_total +
				                            (size_t)(iter - (byte_t const *)DeeMapFile_GetBase(&mf));

				/* Invoke the progress-info callback. */
				progress_status = DeeObject_Call(progress, 1, (DeeObject *const *)&progress_info);
				if unlikely(!progress_status)
					goto err_progress_info_mapfile;
				Dee_Decref(progress_status);
			}
		}
		DeeMapFile_Fini(&mf);
	} else {
		byte_t *transfer_buffer;
		size_t transfer_buffer_size;

		/* Copy file via read+write */
		transfer_buffer_size = used_bufsize;
		transfer_buffer = (byte_t *)Dee_Malloc(transfer_buffer_size);
		if unlikely(!transfer_buffer)
			goto err_progress_info;
		for (;;) {
			size_t rd_size, wr_size;
			if (DeeThread_CheckInterrupt())
				goto err_progress_info_transfer_buffer;
			rd_size = DeeFile_Read(src, transfer_buffer, transfer_buffer_size);
			if unlikely(rd_size == (size_t)-1) {
err_progress_info_transfer_buffer:
				Dee_Free(transfer_buffer);
				goto err_progress_info;
			}
			if (rd_size == 0)
				break; /* Everything was read */
			wr_size = DeeFile_WriteAll(dst, transfer_buffer, rd_size);
			if unlikely(wr_size == (size_t)-1)
				goto err_progress_info_transfer_buffer;
			ASSERT(wr_size <= rd_size);
			transfer_total += wr_size;
			if (wr_size < rd_size) {
				DeeError_Throwf(&DeeError_FileClosed,
				                "Target file %k indicates EOF after %" PRFu64 " bytes",
				                dst, transfer_total);
				goto err_progress_info;
			}

			/* If given (and not `none'), invoke the progress-callback with copy status information. */
			if (!DeeNone_Check(progress)) {
				DREF DeeObject *progress_status;
				if (progress_info == NULL) {
					progress_info = DeeObject_MALLOC(DeeCopyFileProgressObject);
					if unlikely(!progress_info)
						goto err_progress_info;
					Dee_Incref(src);
					Dee_Incref(dst);
					progress_info->cfp_srcfile = src; /* Inherit reference */
					progress_info->cfp_dstfile = dst; /* Inherit reference */
					progress_info->cfp_total   = (uint64_t)-1;
					progress_info->cfp_bufsize = transfer_buffer_size;
					DeeObject_Init(progress_info, &DeeCopyFileProgress_Type);
				}

				/* Update the copied-bytes counter of the progress info object. */
				progress_info->cfp_copied = transfer_total;

				/* Invoke the progress-info callback. */
				progress_status = DeeObject_Call(progress, 1, (DeeObject *const *)&progress_info);
				if unlikely(!progress_status)
					goto err_progress_info;
				Dee_Decref(progress_status);
				if unlikely(progress_info->cfp_bufsize != transfer_buffer_size) {
					/* Change transfer buffer size. */
					byte_t *new_transfer_buffer;
					size_t new_transfer_buffer_size;
					new_transfer_buffer_size = progress_info->cfp_bufsize;
					COMPILER_READ_BARRIER();
					if unlikely(new_transfer_buffer_size == 0)
						goto err_bad_used_bufsize;
					new_transfer_buffer = (byte_t *)Dee_TryMalloc(new_transfer_buffer_size);
					if (new_transfer_buffer) {
						Dee_Free(transfer_buffer);
						transfer_buffer = new_transfer_buffer;
					} else {
						new_transfer_buffer = (byte_t *)Dee_Realloc(transfer_buffer, new_transfer_buffer_size);
						if unlikely(!new_transfer_buffer)
							goto err_progress_info_transfer_buffer;
						transfer_buffer      = new_transfer_buffer;
						transfer_buffer_size = new_transfer_buffer_size;
					}
				}
			} /* if (!DeeNone_Check(progress)) */
		}
		Dee_Free(transfer_buffer);
	}
#ifdef WANT_done
#undef WANT_done
done:
#endif /* WANT_done */
	if (progress_info != NULL)
		Dee_Decref_likely(progress_info);
	return 0;
err_progress_info:
	if (progress_info != NULL)
		Dee_Decref_likely(progress_info);
err:
	return -1;
}
#endif /* NEED_posix_copyfile_fileio */


#ifdef NEED_posix_dfd_makepath
#undef NEED_posix_dfd_makepath
/* Construct a path from `dfd:path'
 * @param: dfd:  Can be a `File', `int', `string', or [nt:`HANDLE']
 * @param: path: Must be a `string' */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
posix_dfd_makepath(DeeObject *dfd, DeeObject *path, unsigned int atflags) {
	struct unicode_printer printer;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	if unlikely(atflags & ~POSIX_DFD_MAKEPATH_ATFLAGS_MASK) {
#define NEED_err_bad_atflags
		err_bad_atflags(atflags);
		goto err;
	}

	/* Check if `path' is absolute. - If it is, then we must use _it_ */
	if (DeeString_IsAbsPath(path))
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
			if (DeeString_IsAbsPath(dfd_filename)) {
				if unlikely(unicode_printer_printstring(&printer, dfd_filename) < 0)
					goto err_printer;
				Dee_Decref(dfd_filename);
				goto got_dfd_path;
			}
			Dee_Decref(dfd_filename);
		}

#ifndef CONFIG_HAVE_AT_FDCWD
		/* OS doesn't support AT_FDCWD --> check if `dfd' is our custom replacement. */
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
#endif /* !CONFIG_HAVE_AT_FDCWD */

#ifdef CONFIG_HOST_WINDOWS
		{
			int error;
			HANDLE hDfd;
			hDfd = (HANDLE)DeeNTSystem_GetHandle(dfd);
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
#endif /* NEED_posix_dfd_makepath */

#ifdef NEED_posix_fd_makepath
#undef NEED_posix_fd_makepath

#ifdef __INTELLISENSE__ /* Defined in "p-pwd.c.inl" */
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getcwd_f_impl(void);
#endif /* __INTELLISENSE__ */

/* Construct a path that refers to the file described by `fd'
 * @param: fd: Can be a `File', `int', or [nt:`HANDLE'] */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_fd_makepath(DeeObject *__restrict fd) {
	if (DeeFile_Check(fd)) {
		DREF DeeObject *result;
		result = DeeFile_Filename(fd);
		if unlikely(!result)
			goto err;
		if (DeeString_IsAbsPath(result))
			return result;
		Dee_Decref(result);
	}

#ifndef CONFIG_HAVE_AT_FDCWD
	/* OS doesn't support AT_FDCWD --> check if `fd' is our custom replacement. */
	if (DeeInt_Check(fd)) {
		int fd_intval;
		if (DeeInt_TryAsInt(fd, &fd_intval)) {
			if (fd_intval == AT_FDCWD) {
				/* Caller explicitly wants the current working directory. */
				return posix_getcwd_f_impl();
			}
		}
	}
#endif /* !CONFIG_HAVE_AT_FDCWD */

	{
#ifdef CONFIG_HOST_WINDOWS
		HANDLE hFd = (HANDLE)DeeNTSystem_GetHandle(fd);
		if unlikely(hFd == INVALID_HANDLE_VALUE)
			goto err;
		return DeeNTSystem_GetFilenameOfHandle(hFd);
#else /* ... */
		int os_fd = DeeUnixSystem_GetFD(fd);
		if unlikely(os_fd == -1)
			goto err;
		return posix_fd_makepath_fd(os_fd);
#define NEED_posix_fd_makepath_fd
#endif /* !... */
	}
err:
	return NULL;
}
#endif /* NEED_posix_fd_makepath */


#ifdef NEED_posix_fd_makepath_fd
#undef NEED_posix_fd_makepath_fd
/* Construct a path that refers to the file described by `fd' */
INTERN WUNUSED DREF DeeObject *DCALL
posix_fd_makepath_fd(int fd) {
#ifndef CONFIG_HAVE_AT_FDCWD
	if (fd == AT_FDCWD)
		return posix_getcwd_f_impl();
#endif /* !CONFIG_HAVE_AT_FDCWD */
#ifdef CONFIG_HAVE_PROCFS
	return DeeString_Newf("/proc/self/fd/%d", fd);
#else /* CONFIG_HAVE_PROCFS */
	return DeeSystem_GetFilenameOfFD(fd);
#endif /* !CONFIG_HAVE_PROCFS */
}
#endif /* NEED_posix_fd_makepath_fd */


#ifdef NEED_err_bad_atflags
#undef NEED_err_bad_atflags
INTERN ATTR_COLD int DCALL
err_bad_atflags(unsigned int atflags) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid atflags %#x",
	                       atflags);
}
#endif /* NEED_err_bad_atflags */

#ifdef NEED_err_bad_copyfile_bufsize_is_zero
#undef NEED_err_bad_copyfile_bufsize_is_zero
INTERN ATTR_COLD int DCALL
err_bad_copyfile_bufsize_is_zero(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid argument: `bufsize' cannot be zero");
}
#endif /* NEED_err_bad_copyfile_bufsize_is_zero */

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
