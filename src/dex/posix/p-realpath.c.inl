/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_REALPATH_C_INL
#define GUARD_DEX_POSIX_P_REALPATH_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/system-features.h>
#include <deemon/system.h>

#include "p-path.c.inl"     /* For `DeeString_IsAbsPath()' */
#include "p-readlink.c.inl" /* For `posix_readlink()' */

DECL_BEGIN

#ifndef PATH_MAX
#ifdef PATHMAX
#define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#define PATH_MAX MAXPATH
#else /* ... */
#define PATH_MAX 260
#endif /* !... */
#endif /* !PATH_MAX */


/* Figure out how to implement `DeeSystem_GetFilenameOfFD()'
 * NOTE: This listing is duplicated in `src/dex/posix/p-realpath.c.inl' */
#undef DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle
#undef DeeSystem_PrintFilenameOfFD_USE_frealpath
#undef DeeSystem_PrintFilenameOfFD_USE_readlink_procfs
#undef DeeSystem_PrintFilenameOfFD_USE_STUB
#if defined(CONFIG_HOST_WINDOWS) && defined(CONFIG_HAVE_get_osfhandle)
#define DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle
#elif defined(CONFIG_HAVE_frealpath)
#define DeeSystem_PrintFilenameOfFD_USE_frealpath
#elif defined(CONFIG_HAVE_frealpath4)
#define frealpath(fd, resolved, buflen) frealpath4(fd, resolved, buflen, 0)
#define DeeSystem_PrintFilenameOfFD_USE_frealpath
#elif !defined(DeeUnixSystem_ReadLink_USE_STUB)
#define DeeSystem_PrintFilenameOfFD_USE_readlink_procfs
#else /* ... */
#define DeeSystem_PrintFilenameOfFD_USE_STUB
#endif /* !... */



/* >> lrealpath(path: string): string */
#undef posix_lrealpath_USE_lrealpath
#undef posix_lrealpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle
#undef posix_lrealpath_USE_posix_readlink
#undef posix_lrealpath_USE_STUB
#ifdef CONFIG_HAVE_lrealpath
#define posix_lrealpath_USE_lrealpath
#elif defined(CONFIG_HOST_WINDOWS)
#define posix_lrealpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle
#elif !defined(posix_readlink_USE_STUB)
#define posix_lrealpath_USE_posix_readlink
#else /* ... */
#define posix_lrealpath_USE_STUB
#endif /* !... */

/* >> frealpath(fd: int | File, atflags: int = 0): string */
#undef posix_frealpath_USE_frealpath4
#undef posix_frealpath_USE_DeeSystem_GetFilenameOfFD
#undef posix_frealpath_USE_STUB
#ifdef CONFIG_HAVE_frealpath4
#define posix_frealpath_USE_frealpath4
#elif !defined(DeeSystem_PrintFilenameOfFD_USE_STUB)
#define posix_frealpath_USE_DeeSystem_GetFilenameOfFD
#else /* ... */
#define posix_frealpath_USE_STUB
#endif /* !... */

/* >> realpath(path: string): string */
#undef posix_realpath_USE_realpath3
#undef posix_realpath_USE_canonicalize_file_name
#undef posix_realpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle
#undef posix_realpath_USE_STUB
#ifdef CONFIG_HAVE_realpath3
#define posix_realpath_USE_realpath3
#elif defined(CONFIG_HAVE_canonicalize_file_name)
#define posix_realpath_USE_canonicalize_file_name
#elif defined(CONFIG_HOST_WINDOWS)
#define posix_realpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle
#elif !defined(posix_readlink_USE_STUB)
#define posix_realpath_USE_posix_readlink
#else /* ... */
#define posix_realpath_USE_STUB
#endif /* !... */

/* >> realpathat(dfd: int | File | string, path: string, atflags: int = 0): string */
#undef posix_realpathat_USE_realpathat
#undef posix_realpathat_USE_posix_realpath
#undef posix_realpathat_USE_posix_lrealpath
#undef posix_realpathat_USE_posix_frealpath
#undef posix_realpathat_USE_STUB
#if !defined(posix_realpath_USE_STUB) || !defined(posix_lrealpath_USE_STUB)
#ifdef CONFIG_HAVE_realpathat
#define posix_realpathat_USE_realpathat
#endif /* CONFIG_HAVE_realpathat */
#ifndef posix_realpath_USE_STUB
#define posix_realpathat_USE_posix_realpath
#endif /* !posix_realpath_USE_STUB */
#ifndef posix_lrealpath_USE_STUB
#define posix_realpathat_USE_posix_lrealpath
#endif /* !posix_lrealpath_USE_STUB */
#ifndef posix_frealpath_USE_STUB
#define posix_realpathat_USE_posix_frealpath
#endif /* !posix_frealpath_USE_STUB */
#else /* !posix_realpath_USE_STUB */
#define posix_realpathat_USE_STUB
#endif /* posix_realpath_USE_STUB */


#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */


#ifndef posix_readlink_USE_STUB
/* Print `path' to `printer' whilst expanding embedded symlink references.
 * NOTE: The caller must ensure that `printer' is either empty, or contains
 *       a trailing `DeeSystem_SEP' before calling this function! */
#undef posix_print_resolved_path
#define HAVE_posix_print_resolved_path

#define posix_try_readlink_ENOENT ((DREF DeeObject *)-2)

/* Try to do a `readlink()' operation on the string represented by `printer'
 * @return: * :        The expansion of a symlink
 * @return: NULL:      An error was thrown
 * @return: ITER_DONE: Not a symlink
 * @return: posix_try_readlink_ENOENT: Failed to access file */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_try_readlink(DeeObject *__restrict file) {
	/*Dee_DPRINTF("posix_try_readlink(%r)\n", file);*/

#ifdef posix_readlink_USE_nt_FReadLink
	DREF DeeObject *result;
	HANDLE hLinkFile;
	hLinkFile = DeeNTSystem_CreateFileNoATime(file, FILE_READ_DATA | FILE_READ_ATTRIBUTES,
	                                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                                          NULL, OPEN_EXISTING,
	                                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
	                                          NULL);
	if unlikely(!hLinkFile)
		goto err;
	if unlikely(hLinkFile == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			/* Try to open the file one more time, only this
			 * time _only_ pass the READ_ATTRIBUTES capability. */
			hLinkFile = DeeNTSystem_CreateFileNoATime(file, FILE_READ_ATTRIBUTES,
			                                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			                                          NULL, OPEN_EXISTING,
			                                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
			                                          NULL);
			if unlikely(!hLinkFile)
				goto err;
			if unlikely(hLinkFile != INVALID_HANDLE_VALUE)
				goto ok_got_ownds_linkfd;
		}
		return posix_try_readlink_ENOENT;
	}
ok_got_ownds_linkfd:
#define NEED_nt_FReadLink
	result = nt_FReadLink(hLinkFile, file, false);
	DBG_ALIGNMENT_DISABLE();
	(void)CloseHandle(hLinkFile);
	DBG_ALIGNMENT_ENABLE();
	return result;
err:
	return NULL;
#endif /* posix_readlink_USE_nt_FReadLink */

#if (defined(posix_readlink_USE_freadlinkat) || \
     defined(posix_readlink_USE_wreadlink) ||   \
     defined(posix_readlink_USE_readlink))
#ifdef posix_readlink_USE_wreadlink
	dwchar_t *wide_file, *buffer, *new_buffer;
#else /* posix_readlink_USE_wreadlink */
	char *utf8_file, *buffer, *new_buffer;
#endif /* !posix_readlink_USE_wreadlink */
	int error;
	size_t bufsize, new_size;
	dssize_t req_size;
	if (DeeObject_AssertTypeExact(file, &DeeString_Type))
		goto err;
#ifdef posix_readlink_USE_wreadlink
	wide_file = DeeString_AsWide(file);
	if unlikely(!wide_file)
		goto err;
#else /* posix_readlink_USE_wreadlink */
	utf8_file = DeeString_AsUtf8(file);
	if unlikely(!utf8_file)
		goto err;
#endif /* !posix_readlink_USE_wreadlink */
#ifdef PATH_MAX
	bufsize = PATH_MAX;
#else /* PATH_MAX */
	bufsize = 256;
#endif /* !PATH_MAX */
#ifdef posix_readlink_USE_wreadlink
	buffer = DeeString_NewWideBuffer(bufsize);
#else /* posix_readlink_USE_wreadlink */
	buffer = (char *)DeeString_New1ByteBuffer(bufsize);
#endif /* !posix_readlink_USE_wreadlink */
	if unlikely(!buffer)
		goto err;
	for (;;) {
EINTR_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
#ifdef posix_readlink_USE_freadlinkat
		req_size = freadlinkat(AT_FDCWD, utf8_file, buffer, bufsize + 1, AT_READLINK_REQSIZE);
#elif defined(posix_readlink_USE_wreadlink)
		req_size = wreadlink(wide_file, buffer, bufsize + 1);
#else /* ... */
		req_size = readlink(utf8_file, buffer, bufsize + 1);
#endif /* !... */
		if likely(req_size < 0) {
			error = DeeSystem_GetErrno();
			DBG_ALIGNMENT_ENABLE();
			EINTR_HANDLE(error, again, err_buffer);
#ifdef posix_readlink_USE_wreadlink
			DeeString_FreeWideBuffer(buffer);
#else /* posix_readlink_USE_wreadlink */
			DeeString_Free1ByteBuffer((uint8_t *)buffer);
#endif /* !posix_readlink_USE_wreadlink */
#ifdef EINVAL
			if likely(error == EINVAL)
				return ITER_DONE; /* Not a symbolic link */
#endif /* EINVAL */
			return posix_try_readlink_ENOENT;
		}
		DBG_ALIGNMENT_ENABLE();
#ifdef posix_readlink_USE_freadlinkat
		if ((size_t)req_size <= (bufsize + 1)) {
			--req_size; /* Account for trailing NUL */
			break;
		}
		new_size = (size_t)req_size - 1;
#else /* posix_readlink_USE_freadlinkat */
		if ((size_t)req_size <= bufsize)
			break;
		new_size = bufsize * 2;
#endif /* !posix_readlink_USE_freadlinkat */
#ifdef posix_readlink_USE_wreadlink
		new_buffer = DeeString_ResizeWideBuffer(buffer, bufsize);
#else /* posix_readlink_USE_wreadlink */
		new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, bufsize);
#endif /* !posix_readlink_USE_wreadlink */
		if unlikely(!new_buffer)
			goto err_buffer;
		buffer  = new_buffer;
		bufsize = new_size;
	}

	/* Release unused data. */
#ifdef posix_readlink_USE_wreadlink
	buffer = DeeString_TruncateWideBuffer(buffer, (size_t)req_size);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
#else /* posix_readlink_USE_wreadlink */
	buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, (size_t)req_size);
	return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
#endif /* !posix_readlink_USE_wreadlink */
err_buffer:
#ifdef posix_readlink_USE_wreadlink
	DeeString_FreeWideBuffer(buffer);
#else /* posix_readlink_USE_wreadlink */
	DeeString_Free1ByteBuffer((uint8_t *)buffer);
#endif /* !posix_readlink_USE_wreadlink */
err:
	return NULL;
#endif /* posix_readlink_USE_wreadlink || posix_readlink_USE_readlink || posix_readlink_USE_freadlinkat */
}

/* Try to do a `readlink()' operation on the string represented by `printer'
 * @return: * :        The expansion of a symlink
 * @return: NULL:      An error was thrown
 * @return: ITER_DONE: Not a symlink
 * @return: posix_try_readlink_ENOENT: Failed to access file */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_try_readlink_unicode_printer(struct unicode_printer *__restrict printer) {
	DREF DeeObject *path, *result;
	path = unicode_printer_pack(printer);
	if unlikely(!path)
		goto err;
	result = posix_try_readlink(path);
	unicode_printer_init_string(printer, path);
	return result;
err:
	unicode_printer_init(printer);
	return NULL;
}

/* Max # of recursive symbolic links */
#define POSIX_PRINT_RESOLVED_PATH_MAX_LINK 64

/* @return: 1 : Success, but hit ENOENT
/* @return: 0 : Success
/* @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
posix_print_resolved_path(struct unicode_printer *__restrict printer,
                          DeeObject *__restrict path, uint32_t max_link,
                          bool do_AT_SYMLINK_NOFOLLOW
#if defined(posix_realpath_USE_posix_readlink) || defined(posix_realpath_USE_posix_readlink)
                          , bool ignore_ENOENT
#define LOCAL_ignore_ENOENT ignore_ENOENT
#else /* posix_realpath_USE_posix_readlink || posix_realpath_USE_posix_readlink */
#define posix_print_resolved_path(printer, path, max_link, do_AT_SYMLINK_NOFOLLOW, ignore_ENOENT) \
	(posix_print_resolved_path)(printer, path, max_link, do_AT_SYMLINK_NOFOLLOW)
#define LOCAL_ignore_ENOENT true
#endif /* !posix_realpath_USE_posix_readlink && !posix_realpath_USE_posix_readlink */
                          ) {
	char *path_utf8;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	if unlikely(!max_link)
		goto err_too_many_links;
	path_utf8 = DeeString_AsUtf8(path);
	if unlikely(!path_utf8)
		goto err;
	if (!*path_utf8)
		return 0; /* End-of-path */
	if (DeeSystem_IsAbs(path_utf8)) {
		/* Reset path resolver and print everything from `path' until we reach the relative part. */
		unicode_printer_clear(printer);
#ifdef DEE_SYSTEM_FS_DRIVES
		{
			char *drive_end = path_utf8;
			if (drive_end[1] == ':') {
				drive_end += 2;
				if (DeeSystem_IsSep(*drive_end))
					++drive_end;
			} else {
				/* Must be a '\\'-prefix */
				drive_end += 2;
			}
			if unlikely(unicode_printer_print(printer, path_utf8, (size_t)(drive_end - path_utf8)) < 0)
				goto err;
			path_utf8 = drive_end;
			while (DeeSystem_IsSep(*path_utf8))
				++path_utf8;
		}
#else /* DEE_SYSTEM_FS_DRIVES */
		if unlikely(unicode_printer_putascii(printer, DeeSystem_SEP))
			goto err;
		do {
			++path_utf8;
		} while (DeeSystem_IsSep(*path_utf8));
#endif /* !DEE_SYSTEM_FS_DRIVES */
	}

#ifdef DEE_SYSTEM_FS_DRIVES
	if (DeeSystem_IsSep(*path_utf8)) {
		/* Special case: drive-relative path (truncate the printer and dump leading slashes) */
		char *leading_seps_end;
		leading_seps_end = path_utf8;
		do {
			++leading_seps_end;
		} while (DeeSystem_IsSep(*leading_seps_end));
		unicode_printer_clear(printer);
		if unlikely(unicode_printer_print(printer, path_utf8, (size_t)(leading_seps_end - path_utf8)) < 0)
			goto err;
		path_utf8 = leading_seps_end;
	}
#endif /* DEE_SYSTEM_FS_DRIVES */

	while (*path_utf8) {
		char *segment_end = path_utf8;
		size_t segment_len;
		size_t segment_start_offset;
		DREF DeeObject *segment_link;
		int inner_error;
		ASSERT(!DeeSystem_IsSep(*segment_end));

		/* Figure out where this path-chunk ends. */
		while (*segment_end && !DeeSystem_IsSep(*segment_end))
			++segment_end;

		/* Check for special "." and ".." segments. */
		segment_len = (size_t)(segment_end - path_utf8);
		switch (segment_len) {

		case 1:
			if (path_utf8[0] == '.') {
skip_segment:
				/* Skip this segment. */
				path_utf8 = segment_end;
				while (DeeSystem_IsSep(*path_utf8))
					++path_utf8;
				continue;
			}
			break;

		case 2:
			if (path_utf8[0] == '.' && path_utf8[1] == '.') {
				/* Delete the last segment from `printer' (so-long as that segment isn't "..").
				 * If the printer is empty, print the ".." to the printer. */
				size_t printer_length, prev_segment_end;
				uint32_t lastch;
				printer_length = UNICODE_PRINTER_LENGTH(printer);
				if (!printer_length) {
print_dot_dot_segment:
					if unlikely(unicode_printer_print(printer, "..", 2) < 0)
						goto err;
					goto skip_segment;
				}

				/* Check if the preceding segment is ".." */
				prev_segment_end = printer_length;
				lastch = UNICODE_PRINTER_GETCHAR(printer, prev_segment_end - 1);
				if (DeeSystem_IsSep(lastch)) {
					--prev_segment_end;
					lastch = UNICODE_PRINTER_GETCHAR(printer, prev_segment_end - 1);
				}
				if (prev_segment_end >= 2 && lastch == '.' && UNICODE_PRINTER_GETCHAR(printer, prev_segment_end - 2) == '.' &&
				    (prev_segment_end == 2 || DeeSystem_IsSep(UNICODE_PRINTER_GETCHAR(printer, prev_segment_end - 3)))) {
					lastch = UNICODE_PRINTER_GETCHAR(printer, printer_length - 1);
					if (!DeeSystem_IsSep(lastch)) {
						if unlikely(unicode_printer_putascii(printer, DeeSystem_SEP))
							goto err;
					}
					goto print_dot_dot_segment;
				}

				/* Find the start of the preceding segment. */
				while (prev_segment_end) {
					lastch = UNICODE_PRINTER_GETCHAR(printer, prev_segment_end - 1);
					if (DeeSystem_IsSep(lastch))
						break;
					--prev_segment_end;
				}

#ifdef DEE_SYSTEM_FS_DRIVES
				/* Check for special case: don't delete a leading "C:" or "\\" prefix */
				if (prev_segment_end == 0 && printer_length >= 2) {
					uint32_t ch0 = UNICODE_PRINTER_GETCHAR(printer, 0);
					uint32_t ch1 = UNICODE_PRINTER_GETCHAR(printer, 1);
					if (ch1 == ':')
						goto skip_segment;
					if (ch0 == '\\' && ch1 == '\\')
						goto skip_segment;
				}
#endif /* DEE_SYSTEM_FS_DRIVES */

				/* Kill the preceding segment. */
				unicode_printer_truncate(printer, prev_segment_end);
				goto skip_segment;
			}
			break;

		default: break;
		}
	
		/* Print this path segment to `printer' */
		if (!UNICODE_PRINTER_ISEMPTY(printer)) {
			uint32_t lastch;
			lastch = UNICODE_PRINTER_GETCHAR(printer, UNICODE_PRINTER_LENGTH(printer) - 1);
			if (!DeeSystem_IsSep(lastch)) {
				if unlikely(unicode_printer_putascii(printer, DeeSystem_SEP))
					goto err;
			}
		}
		segment_start_offset = UNICODE_PRINTER_LENGTH(printer);
		if unlikely(unicode_printer_printutf8(printer, path_utf8, segment_len) < 0)
			goto err;

		/* Skip trailing slashes. */
		path_utf8 = segment_end;
		ASSERT(!*segment_end || DeeSystem_IsSep(*segment_end));
		while (DeeSystem_IsSep(*path_utf8))
			++path_utf8;
		
		/* Check for special case: don't follow the last symlink in AT_SYMLINK_NOFOLLOW-mode. */
		if (do_AT_SYMLINK_NOFOLLOW && !*path_utf8)
			break;

		/* Try to follow a symlink produced by this segment. */
		segment_link = posix_try_readlink_unicode_printer(printer);
		if likely(segment_link == ITER_DONE)
			continue;
		if unlikely(!segment_link)
			goto err;
		if unlikely(segment_link == posix_try_readlink_ENOENT) {
			int os_err;
			DREF DeeObject *printed_filename;
			if (LOCAL_ignore_ENOENT) {
				/* Just dump the remainder of the path to the printer. */
				if (*path_utf8) {
					ASSERT(DeeSystem_IsSep(path_utf8[-1]));
					if (!UNICODE_PRINTER_ISEMPTY(printer)) {
						uint32_t lastch;
						lastch = UNICODE_PRINTER_GETCHAR(printer, UNICODE_PRINTER_LENGTH(printer) - 1);
						if (!DeeSystem_IsSep(lastch))
							--path_utf8; /* Include the path separator. */
					}
					if unlikely(unicode_printer_printutf8(printer, path_utf8, strlen(path_utf8)) < 0)
						goto err;
				}
				return 1;
			}
			DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HOST_WINDOWS
			os_err = GetLastError();
#else /* CONFIG_HOST_WINDOWS */
			os_err = DeeSystem_GetErrno();
#endif /* !CONFIG_HOST_WINDOWS */
			DBG_ALIGNMENT_ENABLE();
			printed_filename = unicode_printer_pack(printer);
			unicode_printer_init(printer);
			if unlikely(!printed_filename)
				goto err;

#ifdef CONFIG_HOST_WINDOWS
			if (DeeNTSystem_IsNotDir(os_err)) {
#define NEED_err_nt_path_not_dir
				err_nt_path_not_dir(os_err, printed_filename);
			} else if (DeeNTSystem_IsAccessDeniedError(os_err)) {
#define NEED_err_nt_path_no_access
				err_nt_path_no_access(os_err, printed_filename);
			} else if (DeeNTSystem_IsFileNotFoundError(os_err)) {
#define NEED_err_nt_path_not_found
				err_nt_path_not_found(os_err, printed_filename);
			} else {
				DeeNTSystem_ThrowErrorf(&DeeError_FSError, os_err,
				                        "Failed to read symbolic link %r",
				                        printed_filename);
			}
#else /* CONFIG_HOST_WINDOWS */
#ifdef EACCES
			if (os_err == EACCES) {
#define NEED_err_unix_path_no_access
				err_unix_path_no_access(os_err, printed_filename);
			} else
#endif /* EACCES */
#ifdef ENOTDIR
			if (os_err == ENOTDIR) {
#define NEED_err_unix_path_not_dir
				err_unix_path_not_dir(os_err, printed_filename);
			} else
#endif /* ENOTDIR */
#ifdef ENOENT
			if (os_err == ENOENT) {
#define NEED_err_unix_path_not_found
				err_unix_path_not_found(os_err, printed_filename);
			} else
#endif /* ENOENT */
			{
				DeeUnixSystem_ThrowErrorf(&DeeError_FSError, os_err,
				                          "Failed to read symbolic link %r",
				                          printed_filename);
			}
#endif /* !CONFIG_HOST_WINDOWS */
			Dee_Decref_likely(printed_filename);
			goto err;
		}

		/* Actually got a symbolic link! -> Must expand recursively */
		unicode_printer_truncate(printer, segment_start_offset);
		inner_error = posix_print_resolved_path(printer, segment_link, max_link - 1,
		                                        false, LOCAL_ignore_ENOENT);
		Dee_Decref_likely(segment_link);
		if (inner_error != 0) {
			if unlikely(inner_error < 0)
				goto err;

			/* Encountered a file-not-found problem -> dump the remainder of the path. */
			if (!UNICODE_PRINTER_ISEMPTY(printer)) {
				uint32_t lastch;
				lastch = UNICODE_PRINTER_GETCHAR(printer, UNICODE_PRINTER_LENGTH(printer) - 1);
				if (!DeeSystem_IsSep(lastch)) {
					if unlikely(unicode_printer_putascii(printer, DeeSystem_SEP))
						goto err;
				}
			}
			if unlikely(unicode_printer_printutf8(printer, path_utf8, strlen(path_utf8)) < 0)
				goto err;
			return 1;
		}
	}

	return 0;
err_too_many_links:
	DeeError_Throwf(&DeeError_FSError, "Too many symbolic links");
err:
	return -1;
#undef LOCAL_ignore_ENOENT
}
#endif /* !posix_readlink_USE_STUB */


/*[[[deemon import("rt.gen.dexutils").gw("lrealpath", "path:?Dstring->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lrealpath_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lrealpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_LREALPATH_DEF { "lrealpath", (DeeObject *)&posix_lrealpath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_LREALPATH_DEF_DOC(doc) { "lrealpath", (DeeObject *)&posix_lrealpath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lrealpath, &posix_lrealpath_f);
#ifndef POSIX_KWDS_PATH_DEFINED
#define POSIX_KWDS_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !POSIX_KWDS_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lrealpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path, "o:lrealpath", &path))
		goto err;
	return posix_lrealpath_f_impl(path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lrealpath_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_lrealpath_USE_lrealpath
	char *utf8_path, *buffer, *new_buffer;
	int error;
	size_t bufsize, new_size;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
	bufsize = PATH_MAX;
	buffer = (char *)DeeString_New1ByteBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		if (lrealpath(utf8_path, buffer, bufsize + 1)) {
			DBG_ALIGNMENT_ENABLE();
			break;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#ifdef ERANGE
		if (error == ERANGE) {
			new_size   = bufsize * 2;
			new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, bufsize);
			if unlikely(!new_buffer)
				goto err_buffer;
			buffer  = new_buffer;
			bufsize = new_size;
		}
#endif /* ERANGE */
		EINTR_HANDLE(error, again, err_buffer);
		ENOMEM_HANDLE(error, again, err_buffer);
#ifdef EACCES
		if (error == EACCES) {
#define NEED_err_unix_path_no_access
			err_unix_path_no_access(error, path);
			goto err_buffer;
		}
#endif /* EACCES */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
#define NEED_err_unix_path_not_dir
			err_unix_path_not_dir(error, path);
			goto err_buffer;
		}
#endif /* ENOTDIR */
#ifdef ENOENT
		if (error == ENOENT) {
#define NEED_err_unix_path_not_found
			err_unix_path_not_found(error, path);
			goto err_buffer;
		}
#endif /* ENOENT */
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to get lrealpath of %r",
		                          path);
		goto err_buffer;
	}

	/* Release unused data. */
	new_size = strnlen(buffer, bufsize);
	buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, (size_t)new_size);
	return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_Free1ByteBuffer((uint8_t *)buffer);
err:
	return NULL;
#endif /* posix_lrealpath_USE_lrealpath */

#ifdef posix_lrealpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle
	DREF DeeObject *result;
	HANDLE hFile;
	hFile = DeeNTSystem_CreateFileNoATime(path, FILE_READ_DATA | FILE_READ_ATTRIBUTES,
	                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                                      NULL, OPEN_EXISTING,
	                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
	                                      NULL);
	if unlikely(!hFile)
		goto err;
	if unlikely(hFile == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsNotDir(dwError)) {
#define NEED_err_nt_path_not_dir
			err_nt_path_not_dir(dwError, path);
			goto err;
		}
		if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			/* Try to open the file one more time, only this
			 * time _only_ pass the READ_ATTRIBUTES capability. */
			hFile = DeeNTSystem_CreateFileNoATime(path, FILE_READ_ATTRIBUTES,
			                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			                                      NULL, OPEN_EXISTING,
			                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
			                                      NULL);
			if unlikely(!hFile)
				goto err;
			if unlikely(hFile != INVALID_HANDLE_VALUE)
				goto ok_got_hFile;
#define NEED_err_nt_path_no_access
			err_nt_path_no_access(dwError, path);
			goto err;
		}
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
#define NEED_err_nt_path_not_found
			err_nt_path_not_found(dwError, path);
			goto err;
		}
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
		                        "Failed to get lrealpath of %r",
		                        path);
		goto err;
	}

	/* Load the absolute filename of this handle. */
ok_got_hFile:
	result = DeeNTSystem_GetFilenameOfHandle(hFile);
	DBG_ALIGNMENT_DISABLE();
	(void)CloseHandle(hFile);
	DBG_ALIGNMENT_ENABLE();
	return result;
err:
	return NULL;
#endif /* posix_lrealpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle */

#ifdef posix_lrealpath_USE_posix_readlink
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err_printer;
	if (!DeeString_IsAbsPath(path)) {
		if unlikely(DeeSystem_PrintPwd(&printer, true))
			goto err_printer;
	}
#ifdef CONFIG_HOST_WINDOWS
	if (posix_path_is_nt_special(DeeString_STR(path),
	                             DeeString_SIZE(path))) {
		unicode_printer_fini(&printer);
		return_reference_(path);
	}
#endif /* CONFIG_HOST_WINDOWS */
	if unlikely(posix_print_resolved_path(&printer, path, POSIX_PRINT_RESOLVED_PATH_MAX_LINK, true, false) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* posix_lrealpath_USE_posix_readlink */

#ifdef posix_lrealpath_USE_STUB
	(void)path;
	posix_err_unsupported("lrealpath");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_lrealpath_USE_STUB */
}




/*[[[deemon import("rt.gen.dexutils").gw("frealpath", "fd:?X2?DFile?Dint,atflags:u=0->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_frealpath_f_impl(DeeObject *fd, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_frealpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FREALPATH_DEF { "frealpath", (DeeObject *)&posix_frealpath, MODSYM_FREADONLY, DOC("(fd:?X2?DFile?Dint,atflags:?Dint=!0)->?Dstring") },
#define POSIX_FREALPATH_DEF_DOC(doc) { "frealpath", (DeeObject *)&posix_frealpath, MODSYM_FREADONLY, DOC("(fd:?X2?DFile?Dint,atflags:?Dint=!0)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_frealpath, &posix_frealpath_f);
#ifndef POSIX_KWDS_FD_ATFLAGS_DEFINED
#define POSIX_KWDS_FD_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd_atflags, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !POSIX_KWDS_FD_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_frealpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *fd;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_atflags, "o|u:frealpath", &fd, &atflags))
		goto err;
	return posix_frealpath_f_impl(fd, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_frealpath_f_impl(DeeObject *fd, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_frealpath_USE_frealpath4
	char *buffer, *new_buffer;
	int error;
	size_t bufsize, new_size;
	int os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		goto err;
	bufsize = PATH_MAX;
	buffer = (char *)DeeString_New1ByteBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		if (frealpath4(os_fd, buffer, bufsize + 1, atflags)) {
			DBG_ALIGNMENT_ENABLE();
			break;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#ifdef ERANGE
		if (error == ERANGE) {
			new_size   = bufsize * 2;
			new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, bufsize);
			if unlikely(!new_buffer)
				goto err_buffer;
			buffer  = new_buffer;
			bufsize = new_size;
		}
#endif /* ERANGE */
		EINTR_HANDLE(error, again, err_buffer);
		ENOMEM_HANDLE(error, again, err_buffer);
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to get realpath of %r",
		                          fd);
		goto err_buffer;
	}

	/* Release unused data. */
	new_size = strnlen(buffer, bufsize);
	buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, (size_t)new_size);
	return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_Free1ByteBuffer((uint8_t *)buffer);
err:
	return NULL;
#endif /* posix_frealpath_USE_frealpath4 */

#ifdef posix_frealpath_USE_DeeSystem_GetFilenameOfFD
	if unlikely(atflags != 0) {
		err_bad_atflags(atflags);
#define NEED_err_bad_atflags
		goto err;
	}

#ifdef CONFIG_HOST_WINDOWS
	{
		HANDLE hFile = (HANDLE)DeeNTSystem_GetHandle(fd);
		if unlikely(hFile == INVALID_HANDLE_VALUE)
			goto err;
		return DeeNTSystem_GetFilenameOfHandle(hFile);
	}
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_UNIX
	{
		int os_fd = DeeUnixSystem_GetFD(fd);
		if unlikely(os_fd == -1)
			goto err;
		return DeeSystem_GetFilenameOfFD(os_fd);
	}
#endif /* CONFIG_HOST_UNIX */
err:
	return NULL;
#endif /* posix_frealpath_USE_DeeSystem_GetFilenameOfFD */

#ifdef posix_frealpath_USE_STUB
	(void)fd;
	(void)atflags;
	posix_err_unsupported("frealpath");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_frealpath_USE_STUB */
}




/*[[[deemon import("rt.gen.dexutils").gw("realpath", "path:?Dstring->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_realpath_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_realpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_REALPATH_DEF { "realpath", (DeeObject *)&posix_realpath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_REALPATH_DEF_DOC(doc) { "realpath", (DeeObject *)&posix_realpath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_realpath, &posix_realpath_f);
#ifndef POSIX_KWDS_PATH_DEFINED
#define POSIX_KWDS_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !POSIX_KWDS_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_realpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path, "o:realpath", &path))
		goto err;
	return posix_realpath_f_impl(path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_realpath_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_realpath_USE_realpath3
	char *utf8_path, *buffer, *new_buffer;
	int error;
	size_t bufsize, new_size;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
	bufsize = PATH_MAX;
	buffer = (char *)DeeString_New1ByteBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		if (realpath3(utf8_path, buffer, bufsize + 1)) {
			DBG_ALIGNMENT_ENABLE();
			break;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#ifdef ERANGE
		if (error == ERANGE) {
			new_size   = bufsize * 2;
			new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, bufsize);
			if unlikely(!new_buffer)
				goto err_buffer;
			buffer  = new_buffer;
			bufsize = new_size;
		}
#endif /* ERANGE */
		EINTR_HANDLE(error, again, err_buffer);
		ENOMEM_HANDLE(error, again, err_buffer);
#ifdef EACCES
		if (error == EACCES) {
#define NEED_err_unix_path_no_access
			err_unix_path_no_access(error, path);
			goto err_buffer;
		}
#endif /* EACCES */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
#define NEED_err_unix_path_not_dir
			err_unix_path_not_dir(error, path);
			goto err_buffer;
		}
#endif /* ENOTDIR */
#ifdef ENOENT
		if (error == ENOENT) {
#define NEED_err_unix_path_not_found
			err_unix_path_not_found(error, path);
			goto err_buffer;
		}
#endif /* ENOENT */
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to get realpath of %r",
		                          path);
		goto err_buffer;
	}

	/* Release unused data. */
	new_size = strnlen(buffer, bufsize);
	buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, (size_t)new_size);
	return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_Free1ByteBuffer((uint8_t *)buffer);
err:
	return NULL;
#endif /* posix_realpath_USE_realpath3 */

#ifdef posix_realpath_USE_canonicalize_file_name
	DREF DeeObject *result;
	char *utf8_path, *buffer;
	int error;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	buffer = canonicalize_file_name(utf8_path);
	if (!buffer) {
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
#ifdef EACCES
		if (error == EACCES) {
#define NEED_err_unix_path_no_access
			err_unix_path_no_access(error, path);
			goto err;
		}
#endif /* EACCES */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
#define NEED_err_unix_path_not_dir
			err_unix_path_not_dir(error, path);
			goto err;
		}
#endif /* ENOTDIR */
#ifdef ENOENT
		if (error == ENOENT) {
#define NEED_err_unix_path_not_found
			err_unix_path_not_found(error, path);
			goto err;
		}
#endif /* ENOENT */
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to get realpath of %r",
		                          path);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();

	/* Pack into a deemon string object. */
	result = DeeString_NewUtf8(buffer, strlen(buffer), STRING_ERROR_FNORMAL);
	free(buffer);
	return result;
err:
	return NULL;
#endif /* posix_realpath_USE_canonicalize_file_name */

#ifdef posix_realpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle
	DREF DeeObject *result;
	HANDLE hFile;
	hFile = DeeNTSystem_CreateFileNoATime(path, FILE_READ_DATA | FILE_READ_ATTRIBUTES,
	                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                                      NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if unlikely(!hFile)
		goto err;
	if unlikely(hFile == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsNotDir(dwError)) {
#define NEED_err_nt_path_not_dir
			err_nt_path_not_dir(dwError, path);
			goto err;
		}
		if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			/* Try to open the file one more time, only this
			 * time _only_ pass the READ_ATTRIBUTES capability. */
			hFile = DeeNTSystem_CreateFileNoATime(path, FILE_READ_ATTRIBUTES,
			                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			                                      NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if unlikely(!hFile)
				goto err;
			if unlikely(hFile != INVALID_HANDLE_VALUE)
				goto ok_got_hFile;
#define NEED_err_nt_path_no_access
			err_nt_path_no_access(dwError, path);
			goto err;
		}
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
#define NEED_err_nt_path_not_found
			err_nt_path_not_found(dwError, path);
			goto err;
		}
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
		                        "Failed to get realpath of %r",
		                        path);
		goto err;
	}

	/* Load the absolute filename of this handle. */
ok_got_hFile:
	result = DeeNTSystem_GetFilenameOfHandle(hFile);
	DBG_ALIGNMENT_DISABLE();
	(void)CloseHandle(hFile);
	DBG_ALIGNMENT_ENABLE();
	return result;
err:
	return NULL;
#endif /* posix_realpath_USE_DeeNTSystem_CreateFileNoATime__AND__DeeNTSystem_GetFilenameOfHandle */

#ifdef posix_realpath_USE_posix_readlink
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err_printer;
	if (!DeeString_IsAbsPath(path)) {
		if unlikely(DeeSystem_PrintPwd(&printer, true))
			goto err_printer;
	}
#ifdef CONFIG_HOST_WINDOWS
	if (posix_path_is_nt_special(DeeString_STR(path),
	                             DeeString_SIZE(path))) {
		unicode_printer_fini(&printer);
		return_reference_(path);
	}
#endif /* CONFIG_HOST_WINDOWS */
	if unlikely(posix_print_resolved_path(&printer, path, POSIX_PRINT_RESOLVED_PATH_MAX_LINK, false, false) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* posix_realpath_USE_posix_readlink */

#ifdef posix_realpath_USE_STUB
	(void)path;
	posix_err_unsupported("realpath");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_realpath_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("realpathat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_realpathat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_realpathat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_REALPATHAT_DEF { "realpathat", (DeeObject *)&posix_realpathat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)->?Dstring") },
#define POSIX_REALPATHAT_DEF_DOC(doc) { "realpathat", (DeeObject *)&posix_realpathat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_realpathat, &posix_realpathat_f);
#ifndef POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_realpathat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_atflags, "oo|u:realpathat", &dfd, &path, &atflags))
		goto err;
	return posix_realpathat_f_impl(dfd, path, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_realpathat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#if defined(posix_realpathat_USE_posix_realpath) || defined(posix_realpathat_USE_posix_lrealpath)
	DREF DeeObject *result, *abspath;
#ifdef posix_realpathat_USE_posix_realpath
	if (atflags & AT_EMPTY_PATH) {
		if (!DeeString_Check(dfd) &&
		    (DeeNone_Check(path) || (DeeString_Check(path) &&
		                             DeeString_IsEmpty(path))))
			return posix_realpath_f_impl(dfd);
		atflags &= ~AT_EMPTY_PATH;
	}
#endif /* posix_realpathat_USE_posix_realpath */

#ifdef posix_realpathat_USE_realpathat
	if (!DeeString_Check(dfd)) {
		int os_dfd, error;
		char const *utf8_path;
		char *buffer, *new_buffer;
		size_t bufsize, new_size;
		os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
		if (DeeObject_AssertTypeExact(path, &DeeString_Type))
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
		bufsize = PATH_MAX;
		buffer = (char *)DeeString_New1ByteBuffer(bufsize);
		if unlikely(!buffer)
			goto err;
#ifdef ERANGE
again:
#else /* ERANGE */
EINTR_ENOMEM_LABEL(again)
#endif /* !ERANGE */
		DBG_ALIGNMENT_DISABLE();
		if (realpathat(os_dfd, utf8_path, buffer, bufsize + 1, atflags)) {
			/* Release unused data. */
			new_size = strnlen(buffer, bufsize);
			buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, (size_t)new_size);
			return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
err_buffer:
			DeeString_Free1ByteBuffer((uint8_t *)buffer);
			goto err;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#ifdef ERANGE
		if (error == ERANGE) {
			new_size   = bufsize * 2;
			new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, bufsize);
			if unlikely(!new_buffer)
				goto err_buffer;
			buffer  = new_buffer;
			bufsize = new_size;
		}
#endif /* ERANGE */
		EINTR_HANDLE(error, again, err_buffer);
		ENOMEM_HANDLE(error, again, err_buffer);
		DeeString_Free1ByteBuffer((uint8_t *)buffer);
		/* Fallthru to fallback path below */
	}
#endif /* posix_realpathat_USE_realpathat */

#ifdef posix_realpathat_USE_posix_lrealpath
	abspath = posix_dfd_makepath(dfd, path, atflags & ~AT_SYMLINK_NOFOLLOW);
#else /* posix_realpathat_USE_posix_lrealpath */
	abspath = posix_dfd_makepath(dfd, path, atflags);
#endif /* !posix_realpathat_USE_posix_lrealpath */
	if unlikely(!abspath)
		goto err;
#ifdef posix_realpathat_USE_posix_lrealpath
	if (atflags & AT_SYMLINK_NOFOLLOW) {
		result = posix_lrealpath_f_impl(abspath);
	} else
#endif /* posix_realpathat_USE_posix_lrealpath */
	{
#ifdef posix_realpathat_USE_posix_realpath
		result = posix_realpath_f_impl(abspath);
#else /* posix_realpathat_USE_posix_realpath */
		Dee_Decref(abspath);
		err_bad_atflags(atflags);
#define NEED_err_bad_atflags
		goto err;
#endif /* !posix_realpathat_USE_posix_realpath */
	}
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_realpathat_USE_posix_realpath || posix_realpathat_USE_posix_lrealpath */

#ifdef posix_realpathat_USE_STUB
	(void)dfd;
	(void)path;
	(void)atflags;
	posix_err_unsupported("realpathat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_realpathat_USE_STUB */
}





/************************************************************************/
/* resolvepath()                                                        */
/************************************************************************/
/* - Same as realpath(), but don't error out when files are missing     */
/* - Also doesn't force the path to become absolute!                    */
/************************************************************************/

#undef posix_lresolvepath_USE_posix_print_resolved_path
#undef posix_lresolvepath_USE_STUB
#undef posix_resolvepath_USE_posix_print_resolved_path
#undef posix_resolvepath_USE_STUB
#undef posix_resolvepathat_USE_posix_print_resolved_path
#undef posix_resolvepathat_USE_STUB
#ifdef HAVE_posix_print_resolved_path
#define posix_lresolvepath_USE_posix_print_resolved_path
#define posix_resolvepath_USE_posix_print_resolved_path
#define posix_resolvepathat_USE_posix_print_resolved_path
#else /* HAVE_posix_print_resolved_path */
#define posix_lresolvepath_USE_STUB
#define posix_resolvepath_USE_STUB
#define posix_resolvepathat_USE_STUB
#endif /* !HAVE_posix_print_resolved_path */


/*[[[deemon import("rt.gen.dexutils").gw("lresolvepath", "path:?Dstring->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lresolvepath_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lresolvepath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_LRESOLVEPATH_DEF { "lresolvepath", (DeeObject *)&posix_lresolvepath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_LRESOLVEPATH_DEF_DOC(doc) { "lresolvepath", (DeeObject *)&posix_lresolvepath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lresolvepath, &posix_lresolvepath_f);
#ifndef POSIX_KWDS_PATH_DEFINED
#define POSIX_KWDS_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !POSIX_KWDS_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lresolvepath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path, "o:lresolvepath", &path))
		goto err;
	return posix_lresolvepath_f_impl(path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lresolvepath_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_lresolvepath_USE_posix_print_resolved_path
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err_printer;
	if unlikely(posix_print_resolved_path(&printer, path, POSIX_PRINT_RESOLVED_PATH_MAX_LINK, true, true) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* posix_lresolvepath_USE_posix_print_resolved_path */

#ifdef posix_lresolvepath_USE_STUB
	(void)path;
	posix_err_unsupported("lresolvepath");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_lresolvepath_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("resolvepath", "path:?Dstring->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_resolvepath_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_resolvepath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RESOLVEPATH_DEF { "resolvepath", (DeeObject *)&posix_resolvepath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_RESOLVEPATH_DEF_DOC(doc) { "resolvepath", (DeeObject *)&posix_resolvepath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_resolvepath, &posix_resolvepath_f);
#ifndef POSIX_KWDS_PATH_DEFINED
#define POSIX_KWDS_PATH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !POSIX_KWDS_PATH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_resolvepath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path, "o:resolvepath", &path))
		goto err;
	return posix_resolvepath_f_impl(path);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_resolvepath_f_impl(DeeObject *path)
/*[[[end]]]*/
{
#ifdef posix_resolvepath_USE_posix_print_resolved_path
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err_printer;
	if unlikely(posix_print_resolved_path(&printer, path, POSIX_PRINT_RESOLVED_PATH_MAX_LINK, false, true) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* posix_resolvepath_USE_posix_print_resolved_path */

#ifdef posix_resolvepath_USE_STUB
	(void)path;
	posix_err_unsupported("resolvepath");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_resolvepath_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("resolvepathat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:u=0->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_resolvepathat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_resolvepathat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RESOLVEPATHAT_DEF { "resolvepathat", (DeeObject *)&posix_resolvepathat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)->?Dstring") },
#define POSIX_RESOLVEPATHAT_DEF_DOC(doc) { "resolvepathat", (DeeObject *)&posix_resolvepathat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:?Dint=!0)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_resolvepathat, &posix_resolvepathat_f);
#ifndef POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_resolvepathat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_atflags, "oo|u:resolvepathat", &dfd, &path, &atflags))
		goto err;
	return posix_resolvepathat_f_impl(dfd, path, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_resolvepathat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_resolvepathat_USE_posix_print_resolved_path
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	bool do_AT_SYMLINK_NOFOLLOW = (atflags & AT_SYMLINK_NOFOLLOW) != 0;
	if unlikely(atflags & ~AT_SYMLINK_NOFOLLOW) {
		err_bad_atflags(atflags);
#define NEED_err_bad_atflags
		goto err_printer;
	}
	if unlikely(DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err_printer;
	if (DeeString_IsAbsPath(path)) {
do_print_resolve_path_as_is:
		if unlikely(posix_print_resolved_path(&printer, path, POSIX_PRINT_RESOLVED_PATH_MAX_LINK, do_AT_SYMLINK_NOFOLLOW, true) < 0)
			goto err_printer;
		return unicode_printer_pack(&printer);
	}
#ifdef CONFIG_HOST_WINDOWS
	if (posix_path_is_nt_special(DeeString_STR(path),
	                             DeeString_SIZE(path))) {
		unicode_printer_fini(&printer);
		return_reference_(path);
	}
#endif /* CONFIG_HOST_WINDOWS */

	if (DeeString_Check(dfd)) {
		uint32_t lastch;
		if unlikely(unicode_printer_printstring(&printer, dfd) < 0)
			goto err_printer;
got_dfd_path:
		if (!UNICODE_PRINTER_LENGTH(&printer))
			goto do_print_resolve_path_as_is;
		lastch = UNICODE_PRINTER_GETCHAR(&printer, UNICODE_PRINTER_LENGTH(&printer) - 1);
		if (!DeeSystem_IsSep(lastch)) {
			if unlikely(unicode_printer_putascii(&printer, DeeSystem_SEP))
				goto err_printer;
		}
		goto do_print_path_abs;
	}
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
			if (dfd_intval == AT_FDCWD)
				goto do_print_resolve_path_as_is; /* Empty path to use CWD */
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
		goto got_dfd_path;
	}
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_UNIX
	{
		int os_dfd, error;
		os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err_printer;
		error = DeeSystem_PrintFilenameOfFD(&printer, os_dfd);
		if unlikely(error != 0)
			goto err_printer;
		goto got_dfd_path;
	}
#endif /* CONFIG_HOST_UNIX */

do_print_path_abs:
	{
		DREF DeeObject *basedir, *abspath, *result;
		basedir = unicode_printer_pack(&printer);
		if unlikely(!basedir)
			goto err;
		unicode_printer_init(&printer);
		if unlikely(unicode_printer_printstring(&printer, basedir) < 0)
			goto err_printer_basedir;
		if unlikely(posix_print_resolved_path(&printer, path, POSIX_PRINT_RESOLVED_PATH_MAX_LINK, do_AT_SYMLINK_NOFOLLOW, true) < 0)
			goto err_printer_basedir;
		abspath = unicode_printer_pack(&printer);
		if unlikely(!abspath) {
			Dee_Decref_likely(basedir);
			goto err;
		}

		/* Return a path that is relative to the absolute path of the caller-given `dfd' */
		result = posix_path_relpath_f(abspath, basedir);
		Dee_Decref_likely(abspath);
		Dee_Decref_likely(basedir);
		return result;
err_printer_basedir:
		Dee_Decref_likely(basedir);
	}
err_printer:
	unicode_printer_fini(&printer);
err:
	return NULL;
#endif /* posix_resolvepathat_USE_posix_print_resolved_path */

#ifdef posix_resolvepathat_USE_STUB
	(void)dfd;
	(void)path;
	(void)atflags;
	posix_err_unsupported("resolvepathat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_resolvepathat_USE_STUB */
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_REALPATH_C_INL */
