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
#ifndef GUARD_DEEMON_SYSTEM_SYSTEM_C
#define GUARD_DEEMON_SYSTEM_SYSTEM_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/mapfile.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-error.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <hybrid/host.h>
#include <hybrid/overflow.h>
#include <hybrid/typecore.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HAVE_SYS_PARAM_H
#include <sys/param.h> /* EXEC_PAGESIZE */
#endif /* CONFIG_HAVE_SYS_PARAM_H */

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

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifdef Dee_fd_t_IS_int
#define Dee_PRIpSYSFD "d"
#else /* Dee_fd_t_IS_int */
#define Dee_PRIpSYSFD "p"
#endif /* !Dee_fd_t_IS_int */

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */


/* Figure out how to implement `DeeSystem_PrintPwd()' */
#undef DeeSystem_PrintPwd_USE_GetCurrentDirectoryW
#undef DeeSystem_PrintPwd_USE_wgetcwd
#undef DeeSystem_PrintPwd_USE_getcwd
#undef DeeSystem_PrintPwd_USE_getenv
#undef DeeSystem_PrintPwd_USE_DOT
#if defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_PrintPwd_USE_GetCurrentDirectoryW
#elif defined(CONFIG_HAVE_wgetcwd) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeSystem_PrintPwd_USE_wgetcwd
#elif defined(CONFIG_HAVE_getcwd)
#define DeeSystem_PrintPwd_USE_getcwd
#elif defined(CONFIG_HAVE_wgetcwd)
#define DeeSystem_PrintPwd_USE_wgetcwd
#elif defined(CONFIG_HAVE_getenv)
#define DeeSystem_PrintPwd_USE_getenv
#elif /* ... */
#define DeeSystem_PrintPwd_USE_DOT
#endif /* !... */

#ifdef DeeSystem_PrintPwd_USE_wgetcwd
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* DeeSystem_PrintPwd_USE_wgetcwd */

#ifdef DeeSystem_PrintPwd_USE_GetCurrentDirectoryW
PRIVATE ATTR_COLD int DCALL nt_err_getcwd(DWORD dwError) {
	if (DeeNTSystem_IsAccessDeniedError(dwError)) {
		return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
		                               "Permission to read a part of the current "
		                               "working directory's path was denied");
	} else if (DeeNTSystem_IsFileNotFoundError(dwError)) {
		return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
		                               "The current working directory has been unlinked");
	}
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to retrieve the current working directory");
}
#endif /* DeeSystem_PrintPwd_USE_GetCurrentDirectoryW */



/* Ensure that the given `filename' describes an absolute path. */
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeSystem_MakeAbsolute(/*String*/ DeeObject *__restrict filename) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	char *iter, *begin, *end, *flush_start, *flush_end, ch;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	begin = DeeString_AsUtf8(filename);
	if unlikely(!begin)
		goto err;
	end = begin + WSTR_LENGTH(begin);
#ifdef CONFIG_HOST_WINDOWS
	/* Don't modify special filenames such as `CON' or `NUL' */
	switch ((size_t)(end - begin)) {
#ifdef DEE_SYSTEM_FS_ICASE
#define eqfscase(a, b) ((a) == (b) || (a) == ((b) - ('A' - 'a')))
#else /* DEE_SYSTEM_FS_ICASE */
#define eqfscase(a, b) ((a) == (b))
#endif /* !DEE_SYSTEM_FS_ICASE */

	case 3:
		if (eqfscase(begin[0], 'N') && eqfscase(begin[1], 'U') && eqfscase(begin[2], 'L'))
			goto return_unmodified; /* NUL */
		if (eqfscase(begin[0], 'C') && eqfscase(begin[1], 'O') && eqfscase(begin[2], 'N'))
			goto return_unmodified; /* CON */
		if (eqfscase(begin[0], 'P') && eqfscase(begin[1], 'R') && eqfscase(begin[2], 'N'))
			goto return_unmodified; /* PRN */
		if (eqfscase(begin[0], 'A') && eqfscase(begin[1], 'U') && eqfscase(begin[2], 'X'))
			goto return_unmodified; /* AUX */
		break;

	case 4:
		/* COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9,
		 * LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, LPT9 */
		if (begin[3] >= '1' && begin[3] <= '9' &&
		    ((eqfscase(begin[0], 'C') && eqfscase(begin[1], 'O') && eqfscase(begin[1], 'M')) ||
		     (eqfscase(begin[0], 'L') && eqfscase(begin[1], 'P') && eqfscase(begin[1], 'T'))))
			goto return_unmodified;
		break;

	case 6:
		if (eqfscase(begin[3], 'I') && eqfscase(begin[4], 'N') && begin[5] == '$') {
			if (eqfscase(begin[0], 'C') && eqfscase(begin[1], 'O') && eqfscase(begin[2], 'N'))
				goto return_unmodified; /* CONIN$ */
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
			if (eqfscase(begin[0], 'S') && eqfscase(begin[1], 'T') && eqfscase(begin[2], 'D'))
				goto return_unmodified; /* STDIN$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		}
		break;

	case 7:
		if (eqfscase(begin[3], 'O') && eqfscase(begin[4], 'U') &&
		    eqfscase(begin[5], 'T') && begin[6] == '$') {
			if (eqfscase(begin[0], 'C') && eqfscase(begin[1], 'O') && eqfscase(begin[2], 'N'))
				goto return_unmodified; /* CONOUT$ */
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
			if (eqfscase(begin[0], 'S') && eqfscase(begin[1], 'T') && eqfscase(begin[2], 'D'))
				goto return_unmodified; /* STDOUT$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		}
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
		if (eqfscase(begin[0], 'S') && eqfscase(begin[1], 'T') && eqfscase(begin[2], 'D') &&
		    eqfscase(begin[3], 'E') && eqfscase(begin[4], 'R') && eqfscase(begin[5], 'R') &&
		    begin[6] == '$')
			goto return_unmodified; /* STDERR$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		break;

#undef eqfscase
	default:
		break;
	}
#endif /* CONFIG_HOST_WINDOWS */

	/* Strip leading space. */
	begin = unicode_skipspaceutf8_n(begin, end);
	if (!DeeSystem_IsAbs(begin)) {
		/* Print the current working directory when the given path isn't absolute. */
		if unlikely(DeeSystem_PrintPwd(&printer, true) < 0)
			goto err;

		/* Handle drive-relative paths. */
#ifdef DEE_SYSTEM_FS_DRIVES
		if (DeeSystem_IsSep(begin[0]) && UNICODE_PRINTER_LENGTH(&printer)) {
			size_t index = 0;

			/* This sep must exist because it was printed by `DeeSystem_PrintPwd()' */
			while ((++index, UNICODE_PRINTER_GETCHAR(&printer, index - 1) != DeeSystem_SEP))
				;
			unicode_printer_truncate(&printer, index);

			/* Strip leading slashes. */
			for (;;) {
				begin = unicode_skipspaceutf8_n(begin, end);
				if (begin >= end)
					break;
				if (!DeeSystem_IsSep(*begin))
					break;
				++begin;
			}
		}
#endif /* DEE_SYSTEM_FS_DRIVES */
	}
	iter = flush_start = begin;
	ASSERTF(*end == '\0',
	        "path = %r\n"
	        "end = %p(%q)\n",
	        filename, end, end);
next:
	ch = *iter++;
	switch (ch) {

	/* NOTE: The following part has been mirrored in `posix_path_normalpath_f'
	 * If a bug is found in this code, it should be fixed here, as well as
	 * within the DEX source file. */
#if defined(DeeSystem_ALTSEP) && DeeSystem_ALTSEP != DeeSystem_SEP
	case DeeSystem_ALTSEP:
#endif /* DeeSystem_ALTSEP && DeeSystem_ALTSEP != DeeSystem_SEP */
	case DeeSystem_SEP:
	case '\0': {
		char const *sep_loc;
		bool did_print_sep;
		sep_loc = flush_end = iter - 1;

		/* Skip multiple slashes and whitespace following a path separator. */
		for (;;) {
			iter = unicode_skipspaceutf8_n(iter, end);
			if (iter >= end)
				break;
			if (!DeeSystem_IsSep(*iter))
				break;
			++iter;
		}
		flush_end = unicode_skipspaceutf8_rev_n(flush_end, flush_start);

		/* Analyze the last path portion for being a special name (`.' or `..') */
		if (flush_end[-1] == '.') {
			if (flush_end[-2] == '.' && flush_end - 2 == flush_start) {
				size_t new_end;
				size_t printer_length;
				/* Parent-directory-reference. */

				/* Delete the last directory that was written. */
				if (!printer.up_buffer)
					goto do_flush_after_sep;
				printer_length = printer.up_length;
				if (!printer_length)
					goto do_flush_after_sep;
				if (UNICODE_PRINTER_GETCHAR(&printer, printer_length - 1) == DeeSystem_SEP)
					--printer_length;
				new_end = unicode_printer_memrchr(&printer, DeeSystem_SEP, 0, printer_length);
				if (new_end == (size_t)-1) {
					/* Special handling to trim a path segment at the very start of the given path. */
					if (printer_length == 0)
						goto do_flush_after_sep;
					new_end = 0;
				} else {
					++new_end;
				}

				/* Truncate the valid length of the printer to after the previous slash. */
				printer.up_length = new_end;
				unicode_printer_truncate(&printer, new_end);
				goto done_flush;
			} else if (flush_end[-3] == DeeSystem_SEP && flush_end - 3 >= flush_start) {
				/* Parent-directory-reference. */
				char *new_end;
				new_end = (char *)memrchr(flush_start, DeeSystem_SEP,
				                          (size_t)((flush_end - 3) - flush_start));
				if (!new_end)
					goto done_flush;
				flush_end = new_end + 1; /* Include the previous sep in this flush. */
				if (unicode_printer_print(&printer, flush_start,
				                          (size_t)(flush_end - flush_start)) < 0)
					goto err;
				goto done_flush;
			} else if (flush_end - 1 == flush_start) {
				/* Self-directory-reference. */
done_flush:
				flush_start = iter;
				goto done_flush_nostart;
			} else if (flush_end[-2] == DeeSystem_SEP &&
			           flush_end - 2 >= flush_start) {
				/* Self-directory-reference. */
				flush_end -= 2;
			}
		}
do_flush_after_sep:
		/* Check if we need to fix anything */
		if (flush_end == iter - 1
#ifdef DeeSystem_ALTSEP
		    && (*sep_loc == DeeSystem_SEP || iter == end + 1)
#endif /* DeeSystem_ALTSEP */
		    ) {
			goto done_flush_nostart;
		}
		/* If we can already include a slash in this part, do so. */
		did_print_sep = false;
		if (sep_loc == flush_end
#ifdef DeeSystem_ALTSEP
		    && (*sep_loc == DeeSystem_SEP)
#endif /* !DeeSystem_ALTSEP */
		    ) {
			++flush_end;
			did_print_sep = true;
		}
		/* Flush everything prior to the path. */
		ASSERT(flush_end >= flush_start);
		if (unicode_printer_print(&printer, flush_start,
		                          (size_t)(flush_end - flush_start)) < 0)
			goto err;
		flush_start = iter;
		if (did_print_sep) {
			/* The slash has already been been printed: `foo/ bar' */
		} else if (sep_loc == iter - 1
#ifdef DeeSystem_ALTSEP
		         && (!*sep_loc || *sep_loc == DeeSystem_SEP)
#endif /* !DeeSystem_ALTSEP */
		         ) {
			--flush_start; /* The slash will be printed as part of the next flush: `foo /bar' */
		} else {
			/* The slash must be printed explicitly: `foo / bar' */
			if (unicode_printer_putascii(&printer, DeeSystem_SEP) < 0)
				goto err;
		}
done_flush_nostart:
		if (iter == end + 1)
			goto done;
		goto next;
	}
	default: goto next;
	}
done:
	--iter;
	/* Print the remainder. */
	if (iter > flush_start) {

		/* Check for special case: The printer was never used.
		 * If this is the case, we can simply re-return the given path. */
		if (UNICODE_PRINTER_ISEMPTY(&printer)) {
#ifdef CONFIG_UNICODE_PRINTER_MUSTFINI_IF_EMPTY
			unicode_printer_fini(&printer);
#endif /* CONFIG_UNICODE_PRINTER_MUSTFINI_IF_EMPTY */
			unicode_printer_fini(&printer);
#ifdef CONFIG_HOST_WINDOWS
return_unmodified:
#endif /* CONFIG_HOST_WINDOWS */
			return_reference_(filename);
		}

		/* Actually print the remainder. */
		if (unicode_printer_print(&printer, flush_start,
		                          (size_t)(iter - flush_start)) < 0)
			goto err;
	}
	/* Pack everything together. */
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


/* Print the current working directory to the given `printer'
 * @param: include_trailing_sep: A trailing / or \\-character is also printed.
 * @return:  0: Success.
 * @return: -1: An error occurred (s.a. `DeeError_*'). */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSystem_PrintPwd)(struct Dee_unicode_printer *__restrict printer,
                           bool include_trailing_sep) {
#ifdef DeeSystem_PrintPwd_USE_GetCurrentDirectoryW
	LPWSTR buffer;
	DWORD new_bufsize, bufsize = PATH_MAX;
	buffer = unicode_printer_alloc_wchar(printer, bufsize);
	if unlikely(!buffer)
		goto err;
again:
	if (DeeThread_CheckInterrupt())
		goto err_release;
	DBG_ALIGNMENT_DISABLE();
	new_bufsize = GetCurrentDirectoryW(bufsize + 1, buffer);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!new_bufsize) {
DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
		} else if (DeeNTSystem_IsIntr(dwError)) {
			goto again;
		} else {
			nt_err_getcwd(dwError);
		}
		goto err_release;
	}
	if (new_bufsize > bufsize) {
		LPWSTR new_buffer;
		/* Increase the buffer and try again. */
		new_buffer = unicode_printer_resize_wchar(printer, buffer, new_bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		bufsize = new_bufsize;
		goto again;
	}
	if (!include_trailing_sep) {
		while (new_bufsize && DeeSystem_IsSep(buffer[new_bufsize - 1]))
			--new_bufsize;
	}
	if unlikely(!new_bufsize) {
		new_bufsize = 1;
		buffer[0] = '.';
	}
	if (include_trailing_sep && new_bufsize < bufsize &&
	    !DeeSystem_IsSep(buffer[new_bufsize - 1])) {
		buffer[new_bufsize + 0] = DeeSystem_SEP;
		buffer[new_bufsize + 1] = 0;
		++new_bufsize;
		include_trailing_sep = false;
	}
	if unlikely(unicode_printer_commit_wchar(printer, buffer, new_bufsize) < 0)
		goto err;
	ASSERT(printer->up_length != 0);
	if (include_trailing_sep) {
		uint32_t ch;
		ch = UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1);
		if (!DeeSystem_IsSep(ch)) {
			if (unicode_printer_putascii(printer, DeeSystem_SEP))
				goto err;
		}
	}
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err:
	return -1;
#endif /* DeeSystem_PrintPwd_USE_GetCurrentDirectoryW */

#if (defined(DeeSystem_PrintPwd_USE_wgetcwd) || \
     defined(DeeSystem_PrintPwd_USE_getcwd))
#ifdef DeeSystem_PrintPwd_USE_wgetcwd
#define IFELSE_WCHAR(wc, c) wc
#else /* DeeSystem_PrintPwd_USE_wgetcwd */
#define IFELSE_WCHAR(wc, c) c
#endif /* !DeeSystem_PrintPwd_USE_wgetcwd */
	IFELSE_WCHAR(dwchar_t, char) *buffer, *new_buffer;
	size_t bufsize = PATH_MAX, buflen;
	buffer = IFELSE_WCHAR(unicode_printer_alloc_wchar(printer, bufsize),
	                      unicode_printer_alloc_utf8(printer, bufsize));
	if unlikely(!buffer)
		goto err;
#ifdef EINTR
again:
#endif /* EINTR */
	if (DeeThread_CheckInterrupt())
		goto err_release;
	DBG_ALIGNMENT_DISABLE();
	while (!IFELSE_WCHAR(wgetcwd((wchar_t *)buffer, bufsize + 1),
	                     getcwd((char *)buffer, bufsize + 1))) {
		/* Increase the buffer and try again. */
#if defined(CONFIG_HAVE_errno) && defined(ERANGE)
		int error = DeeSystem_GetErrno();
		if (error != ERANGE) {
			DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
			if (error == EINTR)
				goto again;
#endif /* EINTR */
			DeeSystem_IF_E1(error, EACCES, {
				DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error,
				                          "Permission to read a part of the current "
				                          "working directory's path was denied");
				goto err_release;
			});
			DeeSystem_IF_E1(error, ENOENT, {
				DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error,
				                          "The current working directory has been unlinked");
				goto err_release;
			});
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to retrieve the current working directory");
			goto err_release;
		}
#endif /* CONFIG_HAVE_errno && ERANGE */
		DBG_ALIGNMENT_ENABLE();
		bufsize *= 2;
		new_buffer = IFELSE_WCHAR(unicode_printer_resize_wchar(printer, buffer, bufsize),
		                          unicode_printer_resize_utf8(printer, buffer, bufsize));
		if unlikely(!new_buffer)
			goto err_release;
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	buflen = IFELSE_WCHAR(wcslen(buffer),
	                      strlen(buffer));
	if (!include_trailing_sep) {
		while (buflen && DeeSystem_IsSep(buffer[buflen - 1]))
			--buflen;
	}
	if unlikely(!buflen) {
		buflen    = 1;
		buffer[0] = '.';
	}
	if (include_trailing_sep && buflen < bufsize &&
	    !DeeSystem_IsSep(buffer[buflen - 1])) {
		buffer[buflen + 0] = DeeSystem_SEP;
		buffer[buflen + 1] = 0;
		++buflen;
		include_trailing_sep = false;
	}
	if unlikely(IFELSE_WCHAR(unicode_printer_commit_wchar(printer, buffer, buflen),
	                         unicode_printer_commit_utf8(printer, buffer, buflen)) < 0)
		goto err;
	ASSERT(printer->up_length != 0);
	if (include_trailing_sep) {
		uint32_t ch;
		ch = UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1);
		if (!DeeSystem_IsSep(ch)) {
			if (unicode_printer_putascii(printer, DeeSystem_SEP))
				goto err;
		}
	}
	return 0;
err_release:
	IFELSE_WCHAR(unicode_printer_free_wchar(printer, buffer),
	             unicode_printer_free_utf8(printer, buffer));
err:
	return -1;
#undef IFELSE_WCHAR
#endif /* DeeSystem_PrintPwd_USE_wgetcwd || DeeSystem_PrintPwd_USE_getcwd */

#ifdef DeeSystem_PrintPwd_USE_getenv
	size_t pwdlen;
	char const *pwd;
	DBG_ALIGNMENT_DISABLE();
	pwd = getenv("PWD");
	DBG_ALIGNMENT_ENABLE();
	if (pwd == NULL)
		pwd = "";
	DBG_ALIGNMENT_DISABLE();
	pwdlen = strlen(pwd);
	DBG_ALIGNMENT_ENABLE();
	while (pwdlen && DeeSystem_IsSep(pwd[pwdlen - 1]))
		--pwdlen;
	if unlikely(!pwdlen) {
		pwd = "." DeeSystem_SEP_S;
		pwdlen = 2;
		if (!include_trailing_sep)
			pwdlen = 1;
	}
	if (unicode_printer_printutf8(printer, pwd, pwdlen) < 0)
		goto err;
	ASSERT(printer->up_length != 0);
	if (include_trailing_sep) {
		uint32_t ch;
		ch = UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1);
		if (!DeeSystem_IsSep(ch)) {
			if (unicode_printer_putascii(printer, DeeSystem_SEP))
				goto err;
		}
	}
	return 0;
err:
	return -1;
#endif /* DeeSystem_PrintPwd_USE_getenv */

#ifdef DeeSystem_PrintPwd_USE_DOT
	dssize_t error;
	error = unicode_printer_printutf8(printer,
	                                  "." DeeSystem_SEP_S,
	                                  include_trailing_sep ? 2 : 1);
	if likely(error > 0)
		error = 0;
	return (int)error;
#endif /* DeeSystem_PrintPwd_USE_DOT */
}





/************************************************************************/
/* MISC. UTILS                                                          */
/************************************************************************/

#if !defined(DeeSystem_ALTSEP) || (DeeSystem_ALTSEP == DeeSystem_SEP)
#ifndef CONFIG_HAVE_memrend
#define CONFIG_HAVE_memrend
#undef memrend
#define memrend dee_memrend
DeeSystem_DEFINE_memrend(dee_memrend)
#endif /* !CONFIG_HAVE_memrend */
#endif /* !DeeSystem_ALTSEP || (DeeSystem_ALTSEP == DeeSystem_SEP) */


/* Returns the filename-portion of a given `path'
 * >> print DeeSystem_BaseName(r"/foo/bar/file.txt");   // "file.txt"
 * >> print DeeSystem_BaseName(r"file.txt");            // "file.txt"
 * >> print DeeSystem_BaseName(r"E:\path\to\file.txt"); // "file.txt"  (Windows-only)
 * 
 * This function returns a pointer to 1 character past the
 * last `DeeSystem_SEP' or `DeeSystem_ALTSEP' in `path', or
 * just re-returns `path' when no such character exists. */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED ATTR_INS(1, 2) char const *
(FCALL DeeSystem_BaseName)(char const *__restrict path, size_t pathlen) {
#if defined(DeeSystem_ALTSEP) && (DeeSystem_ALTSEP != DeeSystem_SEP)
	char const *result = path + pathlen;
	while (result > path && !DeeSystem_IsSep(result[-1]))
		--result;
	return result;
#else /* DeeSystem_ALTSEP && (DeeSystem_ALTSEP != DeeSystem_SEP) */
	char const *result;
	result = (char *)((byte_t *)memrend(path, DeeSystem_SEP, pathlen) + 1);
	return result;
#endif /* !DeeSystem_ALTSEP || (DeeSystem_ALTSEP == DeeSystem_SEP) */
}





/************************************************************************/
/* DLFCN API                                                            */
/************************************************************************/
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */

#ifdef DeeSystem_DlOpen_USE_dlopen
#ifndef USED_DLOPEN_SCOPE
#if defined(CONFIG_HAVE_RTLD_LOCAL)
#define USED_DLOPEN_SCOPE RTLD_LOCAL
#elif defined(CONFIG_HAVE_RTLD_GLOBAL)
#define USED_DLOPEN_SCOPE RTLD_GLOBAL
#else /* ... */
#define USED_DLOPEN_SCOPE 0
#endif /* !... */
#endif /* !USED_DLOPEN_SCOPE */

#ifndef USED_DLOPEN_BIND
#if defined(CONFIG_HAVE_RTLD_LAZY)
#define USED_DLOPEN_BIND RTLD_LAZY
#elif defined(CONFIG_HAVE_RTLD_NOW)
#define USED_DLOPEN_BIND RTLD_NOW
#else /* ... */
#define USED_DLOPEN_BIND 0
#endif /* !... */
#endif /* !USED_DLOPEN_BIND */
#endif /* DeeSystem_DlOpen_USE_dlopen */

/* Open a shared library
 * @return: * :                      A handle for the shared library.
 * @return: NULL:                    A deemon callback failed and an error was thrown.
 * @return: DEESYSTEM_DLOPEN_FAILED: Failed to open the shared library. */
PUBLIC WUNUSED NONNULL((1)) void *DCALL
DeeSystem_DlOpen(/*String*/ DeeObject *__restrict filename) {
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
	HMODULE hResult;
	LPCWSTR lpFilename;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
again:
	lpFilename = DeeString_AsWide(filename);
	if unlikely(!lpFilename)
		goto err;
again_loadlib:
	DBG_ALIGNMENT_DISABLE();
	hResult = LoadLibraryW(lpFilename);
	DBG_ALIGNMENT_ENABLE();
	if (!hResult) {
		DWORD dwError;
		hResult = (HMODULE)DEESYSTEM_DLOPEN_FAILED;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again_loadlib;
		}
		if (DeeNTSystem_IsUncError(dwError)) {
			DREF DeeObject *unc_filename;
			/* Try to fix-up the filename. */
			unc_filename = DeeNTSystem_FixUncPath(filename);
			if unlikely(!unc_filename)
				goto err;
			lpFilename = DeeString_AsWide(unc_filename);
			if unlikely(!lpFilename) {
				Dee_Decref(unc_filename);
				goto err;
			}
			/* Try to load the library once again. */
			DBG_ALIGNMENT_DISABLE();
			hResult = LoadLibraryW(lpFilename);
			DBG_ALIGNMENT_ENABLE();
			Dee_Decref(unc_filename);
			if (!hResult) {
				DBG_ALIGNMENT_DISABLE();
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (DeeNTSystem_IsIntr(dwError)) {
					if (DeeThread_CheckInterrupt())
						goto err;
					goto again;
				}
				hResult = (HMODULE)DEESYSTEM_DLOPEN_FAILED;
			}
		}
	}
	return (void *)hResult;
err:
	return NULL;
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */

#if defined(DeeSystem_DlOpen_USE_dlopen)
	void *result;
	char *utf8_filename;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		goto err;
	result = DeeSystem_DlOpenString(utf8_filename);
	return result;
err:
	return NULL;
#endif /* DeeSystem_DlOpen_USE_dlopen */

#ifdef DeeSystem_DlOpen_USE_STUB
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	(void)filename;
	return DEESYSTEM_DLOPEN_FAILED;
#endif /* DeeSystem_DlOpen_USE_STUB */
}


PUBLIC WUNUSED NONNULL((1)) void *DCALL
DeeSystem_DlOpenString(/*utf-8*/ char const *filename) {
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
	HMODULE hResult;
	DBG_ALIGNMENT_DISABLE();
	hResult = LoadLibraryA(filename);
	DBG_ALIGNMENT_ENABLE();
	if (!hResult) {
		/* Try to convert `filename' into its wide-character form,
		 * then use `DeeSystem_DlOpen()' to load the library. */
		DREF DeeStringObject *filename_ob;
		filename_ob = (DREF DeeStringObject *)DeeString_NewUtf8(filename, strlen(filename),
		                                                        STRING_ERROR_FSTRICT);
		if unlikely(!filename_ob)
			goto done;
		hResult = (HMODULE)DeeSystem_DlOpen((DeeObject *)filename_ob);
		Dee_Decref(filename_ob);
	}
done:
	return (void *)hResult;
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */

#ifdef DeeSystem_DlOpen_USE_dlopen
	void *result;
	DBG_ALIGNMENT_DISABLE();
	result = dlopen(filename,
	                USED_DLOPEN_SCOPE |
	                USED_DLOPEN_BIND);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!result)
		result = DEESYSTEM_DLOPEN_FAILED;
	return result;
#endif /* DeeSystem_DlOpen_USE_dlopen */

#ifdef DeeSystem_DlOpen_USE_STUB
	(void)filename;
	return DEESYSTEM_DLOPEN_FAILED;
#endif /* DeeSystem_DlOpen_USE_STUB */
}

/* Try to get a human-readable description on what went wrong during a call
 * to `DeeSystem_DlOpen[String]()' that caused `DEESYSTEM_DLOPEN_FAILED' to
 * be returned, or `DeeSystem_DlSym()' to have caused `NULL' to be returned.
 * @return: * :        The human-readable error description
 * @return: NULL:      A deemon callback failed and an error was thrown.
 * @return: ITER_DONE: No description is available. */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL DeeSystem_DlError(void) {
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
	DREF /*String*/ DeeObject *result;
	DWORD dwError;
	dwError = GetLastError();
	if (dwError == 0)
		return ITER_DONE;
	result = DeeNTSystem_FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
	                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                                   NULL);
	return result;
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */

#ifdef DeeSystem_DlOpen_USE_dlopen
#ifdef CONFIG_HAVE_dlerror
	char *message;
	DREF /*String*/ DeeObject *result;
	DBG_ALIGNMENT_DISABLE();
	message = dlerror();
	DBG_ALIGNMENT_ENABLE();
	if (!message)
		return ITER_DONE;
	result = DeeString_NewUtf8(message, strlen(message),
	                           STRING_ERROR_FIGNORE);
	return result;
#else /* CONFIG_HAVE_dlerror */
	COMPILER_IMPURE();
	return ITER_DONE;
#endif /* !CONFIG_HAVE_dlerror */
#endif /* DeeSystem_DlOpen_USE_dlopen */

#ifdef DeeSystem_DlOpen_USE_STUB
	COMPILER_IMPURE();
	return ITER_DONE;
#endif /* DeeSystem_DlOpen_USE_STUB */
}



/* Lookup a symbol within a given shared library
 * Returns `NULL' if the symbol could not be found */
PUBLIC WUNUSED NONNULL((2)) void *DCALL
DeeSystem_DlSym(void *handle, char const *symbol_name) {
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
	FARPROC result;
	DBG_ALIGNMENT_DISABLE();
	result = GetProcAddress((HMODULE)handle, symbol_name);
	DBG_ALIGNMENT_ENABLE();
	return *(void **)&result;
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */

#ifdef DeeSystem_DlOpen_USE_dlopen
	void *result;
	DBG_ALIGNMENT_DISABLE();
	result = dlsym(handle, symbol_name);
	DBG_ALIGNMENT_ENABLE();
	return result;
#endif /* DeeSystem_DlOpen_USE_dlopen */

#ifdef DeeSystem_DlOpen_USE_STUB
	(void)handle;
	(void)symbol_name;
	COMPILER_IMPURE();
	return NULL;
#endif /* DeeSystem_DlOpen_USE_STUB */
}

/* Close a given shared library */
PUBLIC void DCALL DeeSystem_DlClose(void *handle) {
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
	DBG_ALIGNMENT_DISABLE();
	FreeLibrary((HMODULE)handle);
	DBG_ALIGNMENT_ENABLE();
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */

#ifdef DeeSystem_DlOpen_USE_dlopen
#ifdef CONFIG_HAVE_dlclose
	DBG_ALIGNMENT_DISABLE();
	dlclose(handle);
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_dlclose */
#endif /* DeeSystem_DlOpen_USE_dlopen */

#ifdef DeeSystem_DlOpen_USE_STUB
	(void)handle;
	COMPILER_IMPURE();
#endif /* DeeSystem_DlOpen_USE_STUB */
}






#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FILETIME_GET64(x) (((x) << 32)|((x) >> 32))
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#define FILETIME_GET64(x) (x)
#endif /* !__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */

/* A couple of helper macros taken from the libtime DEX. */
#define time_year2day(value) ((146097 * (value)) / 400) /* TODO: REMOVE ME */
#define MICROSECONDS_PER_MILLISECOND UINT64_C(1000)
#define MILLISECONDS_PER_SECOND      UINT64_C(1000)
#define SECONDS_PER_MINUTE           UINT64_C(60)
#define MINUTES_PER_HOUR             UINT64_C(60)
#define HOURS_PER_DAY                UINT64_C(24)
#define MICROSECONDS_PER_SECOND (MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND)
#define MICROSECONDS_PER_MINUTE (MICROSECONDS_PER_SECOND * SECONDS_PER_MINUTE)
#define MICROSECONDS_PER_HOUR   (MICROSECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define MICROSECONDS_PER_DAY    (MICROSECONDS_PER_HOUR * HOURS_PER_DAY)



/* Figure out how to implement `DeeSystem_GetLastModified()' */
#undef DeeSystem_GetLastModified_USE_GetFileAttributesExW
#undef DeeSystem_GetLastModified_USE_stat
#undef DeeSystem_GetLastModified_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define DeeSystem_GetLastModified_USE_GetFileAttributesExW
#elif defined(CONFIG_HAVE_stat) || defined(CONFIG_HAVE_stat64)
#define DeeSystem_GetLastModified_USE_stat
#else /* ... */
#define DeeSystem_GetLastModified_USE_STUB
#endif /* !... */


/* Figure out how to implement `DeeSystem_GetWalltime()' */
#undef DeeSystem_GetWalltime_USE_GetSystemTimePreciseAsFileTime_OR_GetSystemTimeAsFileTime
#undef DeeSystem_GetWalltime_USE_gettimeofday64
#undef DeeSystem_GetWalltime_USE_gettimeofday
#undef DeeSystem_GetWalltime_USE_clock_gettime64
#undef DeeSystem_GetWalltime_USE_clock_gettime
#undef DeeSystem_GetWalltime_USE_time64
#undef DeeSystem_GetWalltime_USE_time
#undef DeeSystem_GetWalltime_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define DeeSystem_GetWalltime_USE_GetSystemTimePreciseAsFileTime_OR_GetSystemTimeAsFileTime
#elif defined(CONFIG_HAVE_gettimeofday64)
#define DeeSystem_GetWalltime_USE_gettimeofday64
#elif defined(CONFIG_HAVE_gettimeofday)
#define DeeSystem_GetWalltime_USE_gettimeofday
#elif defined(CONFIG_HAVE_clock_gettime64) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define DeeSystem_GetWalltime_USE_clock_gettime64
#elif defined(CONFIG_HAVE_clock_gettime) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define DeeSystem_GetWalltime_USE_clock_gettime
#elif defined(CONFIG_HAVE_time64)
#define DeeSystem_GetWalltime_USE_time64
#elif defined(CONFIG_HAVE_time)
#define DeeSystem_GetWalltime_USE_time
#else /* ... */
#define DeeSystem_GetWalltime_USE_STUB
#endif /* !... */




#if (defined(DeeSystem_GetLastModified_USE_GetFileAttributesExW) || \
     defined(DeeSystem_GetWalltime_USE_GetSystemTimePreciseAsFileTime_OR_GetSystemTimeAsFileTime))
#define FILETIME_PER_SECONDS 10000000 /* 100 nanoseconds / 0.1 microseconds. */
PRIVATE uint64_t DCALL
nt_getunixfiletime(uint64_t filetime) {
	uint64_t result, local_filetime;
	/* Windows FILETIME is in local time (but we want UTC) */
	DBG_ALIGNMENT_DISABLE();
	LocalFileTimeToFileTime((LPFILETIME)&filetime, (LPFILETIME)&local_filetime);
	DBG_ALIGNMENT_ENABLE();
	result = FILETIME_GET64(filetime) / (FILETIME_PER_SECONDS / MICROSECONDS_PER_SECOND);
	/* Window's filetime started counting on 01-01-1601. */
	result -= (time_year2day(1970) - time_year2day(1601)) * MICROSECONDS_PER_DAY;
	return result;
}
#endif /* ... */


/* Return the last modified timestamp of `filename'
 * > uses the same format as `DeeSystem_GetWalltime()'
 * @return: (uint64_t)-1: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) uint64_t DCALL
DeeSystem_GetLastModified(/*String*/ DeeObject *__restrict filename) {
#ifdef DeeSystem_GetLastModified_USE_GetFileAttributesExW
	WIN32_FILE_ATTRIBUTE_DATA attrib;
	LPWSTR wname;
	wname = (LPWSTR)DeeString_AsWide(filename);
	if unlikely(!wname)
		return (uint64_t)-1;
	DBG_ALIGNMENT_DISABLE();
	if (!GetFileAttributesExW(wname, GetFileExInfoStandard, &attrib)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return nt_getunixfiletime((uint64_t)attrib.ftLastWriteTime.dwLowDateTime |
	                          (uint64_t)attrib.ftLastWriteTime.dwHighDateTime << 32);
#endif /* DeeSystem_GetLastModified_USE_GetFileAttributesExW */

#ifdef DeeSystem_GetLastModified_USE_stat
	uint64_t result;
#ifdef CONFIG_HAVE_stat64
	struct stat64 st;
#else /* CONFIG_HAVE_stat64 */
	struct stat st;
#endif /* !CONFIG_HAVE_stat64 */
	char const *utf8_name;
	utf8_name = DeeString_AsUtf8(filename);
	if unlikely(!utf8_name)
		return (uint64_t)-1;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_stat64
	if (stat64(utf8_name, &st))
#else /* CONFIG_HAVE_stat64 */
	if (stat(utf8_name, &st))
#endif /* !CONFIG_HAVE_stat64 */
	{
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	result = (uint64_t)st.st_mtime * MICROSECONDS_PER_SECOND;
	/* Try to get more precision out of this */
#ifdef CONFIG_HAVE_struct_stat_st_timensec
	result += st.st_mtimensec / 1000;
#elif defined(CONFIG_HAVE_struct_stat_st_tim)
	result += st.st_mtim.tv_nsec / 1000;
#elif defined(CONFIG_HAVE_struct_stat_st_timespec)
	result += st.st_mtimespec.tv_nsec / 1000;
#endif /* ... */

	return result;
#endif /* DeeSystem_GetLastModified_USE_stat */

#ifdef DeeSystem_GetLastModified_USE_STUB
	(void)filename;
	return 0;
#endif /* DeeSystem_GetLastModified_USE_STUB */
}

#ifdef DeeSystem_GetWalltime_USE_GetSystemTimePreciseAsFileTime_OR_GetSystemTimeAsFileTime
typedef void (WINAPI *LPGETSYSTEMTIMEPRECISEASFILETIME)(LPFILETIME lpSystemTimeAsFileTime);
static LPGETSYSTEMTIMEPRECISEASFILETIME pdyn_GetSystemTimePreciseAsFileTime = NULL;
#define GetSystemTimePreciseAsFileTime (*pdyn_GetSystemTimePreciseAsFileTime)

#ifndef DEFINED_GET_KERNEL32_HANDLE
#define DEFINED_GET_KERNEL32_HANDLE 1
PRIVATE WCHAR const wKernel32[]    = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
PRIVATE WCHAR const wKernel32Dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };
PRIVATE HMODULE DCALL GetKernel32Handle(void) {
	HMODULE hKernel32;
	hKernel32 = GetModuleHandleW(wKernel32);
	if (!hKernel32)
		hKernel32 = LoadLibraryW(wKernel32Dll);
	return hKernel32;
}
#endif /* !DEFINED_GET_KERNEL32_HANDLE */
#endif /* DeeSystem_GetWalltime_USE_GetSystemTimePreciseAsFileTime_OR_GetSystemTimeAsFileTime */


/* Return the current UTC realtime in microseconds since 01-01-1970T00:00:00+00:00 */
PUBLIC WUNUSED uint64_t DCALL DeeSystem_GetWalltime(void) {
#ifdef DeeSystem_GetWalltime_USE_GetSystemTimePreciseAsFileTime_OR_GetSystemTimeAsFileTime
	uint64_t filetime;
	DBG_ALIGNMENT_DISABLE();
	if (pdyn_GetSystemTimePreciseAsFileTime == NULL) {
		HMODULE hKernel32 = GetKernel32Handle();
		if (!hKernel32) {
			atomic_write((void **)&pdyn_GetSystemTimePreciseAsFileTime, (void *)(uintptr_t)-1);
		} else {
			LPGETSYSTEMTIMEPRECISEASFILETIME func;
			func = (LPGETSYSTEMTIMEPRECISEASFILETIME)GetProcAddress(hKernel32, "GetSystemTimePreciseAsFileTime");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			atomic_write(&pdyn_GetSystemTimePreciseAsFileTime, func);
		}
	}
	if (pdyn_GetSystemTimePreciseAsFileTime != (LPGETSYSTEMTIMEPRECISEASFILETIME)-1) {
		GetSystemTimePreciseAsFileTime((LPFILETIME)&filetime);
	} else {
		GetSystemTimeAsFileTime((LPFILETIME)&filetime);
	}
	DBG_ALIGNMENT_ENABLE();
	return nt_getunixfiletime(filetime);
#endif /* DeeSystem_GetWalltime_USE_GetSystemTimePreciseAsFileTime_OR_GetSystemTimeAsFileTime */

#ifdef DeeSystem_GetWalltime_USE_gettimeofday64
	struct timeval64 tv;
	DBG_ALIGNMENT_DISABLE();
	if (gettimeofday64(&tv, NULL)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + tv.tv_usec;
#endif /* DeeSystem_GetWalltime_USE_gettimeofday64 */

#ifdef DeeSystem_GetWalltime_USE_gettimeofday
	struct timeval tv;
	DBG_ALIGNMENT_DISABLE();
	if (gettimeofday(&tv, NULL)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + tv.tv_usec;
#endif /* DeeSystem_GetWalltime_USE_gettimeofday */

#ifdef DeeSystem_GetWalltime_USE_clock_gettime64
	struct timespec64 ts;
	DBG_ALIGNMENT_DISABLE();
	if (clock_gettime64(CLOCK_REALTIME, &ts)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + (tv.tv_nsec / 1000);
#endif /* DeeSystem_GetWalltime_USE_clock_gettime64 */

#ifdef DeeSystem_GetWalltime_USE_clock_gettime
	struct timespec ts;
	DBG_ALIGNMENT_DISABLE();
	if (clock_gettime(CLOCK_REALTIME, &ts)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + (tv.tv_nsec / 1000);
#endif /* DeeSystem_GetWalltime_USE_clock_gettime */

#ifdef DeeSystem_GetWalltime_USE_time64
	time64_t now;
	DBG_ALIGNMENT_DISABLE();
	now = time64(NULL);
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)now * MICROSECONDS_PER_SECOND;
#endif /* DeeSystem_GetWalltime_USE_time64 */

#ifdef DeeSystem_GetWalltime_USE_time
	time_t now;
	DBG_ALIGNMENT_DISABLE();
	now = time(NULL);
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)now * MICROSECONDS_PER_SECOND;
#endif /* DeeSystem_GetWalltime_USE_time */

#ifdef DeeSystem_GetWalltime_USE_STUB
	return 0;
#endif /* DeeSystem_GetWalltime_USE_STUB */
}




/* Figure out how to implement `DeeSystem_Unlink()' */
#undef DeeSystem_Unlink_USE_DeleteFileW
#undef DeeSystem_Unlink_USE_wunlink
#undef DeeSystem_Unlink_USE_unlink
#undef DeeSystem_Unlink_USE_wremove
#undef DeeSystem_Unlink_USE_remove
#undef DeeSystem_Unlink_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_Unlink_USE_DeleteFileW
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeSystem_Unlink_USE_wunlink
#elif defined(CONFIG_HAVE_unlink)
#define DeeSystem_Unlink_USE_unlink
#elif defined(CONFIG_HAVE_wunlink)
#define DeeSystem_Unlink_USE_wunlink
#elif defined(CONFIG_HAVE_wremove) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeSystem_Unlink_USE_wremove
#elif defined(CONFIG_HAVE_remove)
#define DeeSystem_Unlink_USE_remove
#elif defined(CONFIG_HAVE_wremove)
#define DeeSystem_Unlink_USE_wremove
#else /* ... */
#define DeeSystem_Unlink_STUB
#endif /* !... */



/* Try to unlink() the given `filename'
 * WARNING: When `filename' is an empty directory, it is system-specific if
 *          that directory will be removed or not (basically, this function
 *          may be implemented using the STD-C `remove()' function)
 * NOTE:    Even upon error, there exists a chance that the file was deleted.
 * @return: 1 : The unlink() operation failed (only returned when `throw_exception_on_error' is `false')
 * @return: 0 : The unlink() operation was successful
 * @return: -1: An error occurred (may still be returned, even when `throw_exception_on_error' is `false') */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSystem_Unlink(/*String*/ DeeObject *__restrict filename,
                 bool throw_exception_on_error) {
#ifdef DeeSystem_Unlink_USE_DeleteFileW
	DWORD dwError;
	LPWSTR wname;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	wname = (LPWSTR)DeeString_AsWide(filename);
	if unlikely(!wname) {
		/* Since unlink() is a cleanup operation,
		 * try hard to comply, even after an error! */
		if (DeleteFileA(DeeString_STR(filename))) {
			if (DeeError_Handled(ERROR_HANDLED_NORMAL))
				return 0;
		}
		goto err;
	}
again_deletefile:
	if (DeleteFileW(wname))
		return 0;
	dwError = GetLastError();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt()) {
			/* Try hard to delete the file... (even after an interrupt) */
			DeleteFileW(wname);
			goto err;
		}
		goto again_deletefile;
	}
	if (DeeNTSystem_IsUncError(dwError)) {
		DREF DeeStringObject *unc_filename;
		/* Try again with a UNC filename. */
		unc_filename = (DREF DeeStringObject *)DeeNTSystem_FixUncPath(filename);
		if unlikely(!unc_filename)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(filename);
		if unlikely(!wname) {
			/* No point in trying to use DeleteFileA() here.
			 * It doesn't work for UNC paths to begin with... */
			Dee_Decref(unc_filename);
			goto err;
		}
again_deletefile2:
		if (DeleteFileW(wname)) {
			Dee_Decref(unc_filename);
			return 0; /* Success! */
		}
		dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt()) {
				/* Try hard to delete the file... (even after an interrupt) */
				DeleteFileW(wname);
				Dee_Decref(unc_filename);
				goto err;
			}
			goto again_deletefile2;
		}
		Dee_Decref(unc_filename);
	}
	/* Can't delete the file, no matter what I try...  :( */
	if (!throw_exception_on_error)
		return 1;
	DeeNTSystem_ThrowErrorf(NULL, dwError,
	                        "Failed to delete file %r",
	                        filename);
err:
	return -1;
#endif /* DeeSystem_Unlink_USE_DeleteFileW */

#if (defined(DeeSystem_Unlink_USE_wunlink) || defined(DeeSystem_Unlink_USE_wremove) || \
     defined(DeeSystem_Unlink_USE_unlink) || defined(DeeSystem_Unlink_USE_remove))
#if defined(DeeSystem_Unlink_USE_wunlink) || defined(DeeSystem_Unlink_USE_wremove)
	dwchar_t *wname;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	wname = DeeString_AsWide(filename);
	if unlikely(!wname) {
#if defined(CONFIG_HAVE_unlink) || defined(CONFIG_HAVE_remove)
		/* Since unlink() is a cleanup operation,
		 * try hard to comply, even after an error! */
#ifdef CONFIG_HAVE_unlink
		if (unlink(DeeString_STR(filename)))
#else /* CONFIG_HAVE_unlink */
		if (remove(DeeString_STR(filename)))
#endif /* !CONFIG_HAVE_unlink */
		{
			if (DeeError_Handled(ERROR_HANDLED_NORMAL))
				return 0;
		}
#endif /* CONFIG_HAVE_unlink || CONFIG_HAVE_remove */
		return -1;
	}
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
again_deletefile:
#endif /* CONFIG_HAVE_errno && EINTR */
#ifdef DeeSystem_Unlink_USE_wunlink
	if (wunlink(wname))
		return 0;
#else /* DeeSystem_Unlink_USE_wunlink */
	if (wremove(wname))
		return 0;
#endif /* !DeeSystem_Unlink_USE_wunlink */
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
	if (DeeSystem_GetErrno() == EINTR) {
		if (DeeThread_CheckInterrupt()) {
			/* Try hard to delete the file... (even after an interrupt) */
#ifdef DeeSystem_Unlink_USE_wunlink
			wunlink(wname);
#else /* DeeSystem_Unlink_USE_wunlink */
			wremove(wname);
#endif /* !DeeSystem_Unlink_USE_wunlink */
			return -1;
		}
		goto again_deletefile;
	}
#endif /* CONFIG_HAVE_errno && EINTR */
#endif /* DeeSystem_Unlink_USE_wunlink || DeeSystem_Unlink_USE_wremove */

#if defined(DeeSystem_Unlink_USE_unlink) || defined(DeeSystem_Unlink_USE_remove)
	char const *utf8_name;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	utf8_name = DeeString_AsUtf8(filename);
	if unlikely(!utf8_name) {
		/* Since unlink() is a cleanup operation,
		 * try hard to comply, even after an error! */
#ifdef DeeSystem_Unlink_USE_unlink
		if (unlink(DeeString_STR(filename)))
#else /* CONFIG_HAVE_unlink */
		if (remove(DeeString_STR(filename)))
#endif /* !CONFIG_HAVE_unlink */
		{
			if (DeeError_Handled(ERROR_HANDLED_NORMAL))
				return 0;
		}
		return -1;
	}
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
again_deletefile:
#endif /* CONFIG_HAVE_errno && EINTR */
#ifdef DeeSystem_Unlink_USE_unlink
	if (unlink(utf8_name))
		return 0;
#else /* DeeSystem_Unlink_USE_unlink */
	if (remove(utf8_name))
		return 0;
#endif /* !DeeSystem_Unlink_USE_unlink */
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
	if (DeeSystem_GetErrno() == EINTR) {
		if (DeeThread_CheckInterrupt()) {
			/* Try hard to delete the file... (even after an interrupt) */
#ifdef DeeSystem_Unlink_USE_unlink
			unlink(utf8_name);
#else /* DeeSystem_Unlink_USE_unlink */
			remove(utf8_name);
#endif /* !DeeSystem_Unlink_USE_unlink */
			return -1;
		}
		goto again_deletefile;
	}
#endif /* CONFIG_HAVE_errno && EINTR */
#endif /* DeeSystem_Unlink_USE_wunlink || DeeSystem_Unlink_USE_wremove */
	/* Can't delete the file, no matter what I try...  :( */
	if (!throw_exception_on_error)
		return 1;
	{
		int error = DeeSystem_GetErrno();
		DeeSystem_IF_E4(error, ENOENT, ENOTDIR, ENAMETOOLONG, ELOOP, {
			return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error,
			                                 "Cannot delete missing file %r",
			                                 filename);
		});
		DeeSystem_IF_E2(error, EPERM, EACCES, {
			return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error,
			                                 "Not allowed to delete file %r",
			                                 filename);
		});
		DeeSystem_IF_E1(error, EROFS, {
			return DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, error,
			                                 "Cannot delete file %r on read-only filesystem",
			                                 filename);
		});
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to delete file %r",
		                          filename);
	}
	return -1;
#endif /* DeeSystem_Unlink_USE_wunlink || DeeSystem_Unlink_USE_wremove || \
          DeeSystem_Unlink_USE_unlink || DeeSystem_Unlink_USE_remove */

#ifdef DeeSystem_Unlink_STUB
	if (!throw_exception_on_error)
		return 1;
	return DeeUnixSystem_ThrowErrorf(&DeeError_SystemError,
	                                 DeeSystem_GetErrno(),
	                                 "Failed to delete file %r",
	                                 filename);
#endif /* DeeSystem_Unlink_STUB */

}




#ifndef GETATTR_fileno
#if defined(Dee_fd_GETSET) && defined(Dee_fd_t_IS_FILE)
#define GETATTR_fileno(ob) DeeObject_GetAttr(ob, &str_getsysfd)
#else /* Dee_fd_GETSET && Dee_fd_t_IS_FILE */
#define GETATTR_fileno(ob) DeeObject_GetAttrString(ob, Dee_fd_fileno_GETSET)
#endif /* !Dee_fd_GETSET || !Dee_fd_t_IS_FILE */
#endif /* !GETATTR_fileno */


/* Retrieve the unix FD associated with a given object.
 * The translation is done by performing the following:
 * >> #ifdef Dee_fd_t_IS_int
 * >> if (DeeFile_Check(ob))
 * >>     return DeeFile_GetSysFD(ob);
 * >> #endif
 * >> if (DeeInt_Check(ob))
 * >>     return DeeInt_AsInt(ob);
 * >> try return DeeObject_AsInt(DeeObject_GetAttr(ob, Dee_fd_fileno_GETSET)); catch (AttributeError);
 * >> return DeeObject_AsInt(ob);
 * @return: * : Success (the actual handle value)
 * @return: -1: Error (handle translation failed)
 *              In case the actual handle value stored inside of `ob'
 *              was `-1', then an `DeeError_FileClosed' error is thrown. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeUnixSystem_GetFD(DeeObject *__restrict ob) {
	int error, result;
	DREF DeeObject *attr;
#ifdef Dee_fd_t_IS_int
	STATIC_ASSERT(Dee_fd_INVALID == -1);
	if (DeeFile_Check(ob))
		return (int)DeeFile_GetSysFD(ob);
#endif /* Dee_fd_t_IS_int */
	if (DeeInt_Check(ob)) {
		error = DeeInt_AsInt(ob, &result);
		if unlikely(error)
			goto err;
		if unlikely(result == -1)
			goto err_fd_minus_one;
		return result;
	}
	attr = GETATTR_fileno(ob);
	if unlikely(!attr)
		goto err;
	if (attr) {
		error = DeeObject_AsInt(attr, &result);
		Dee_Decref(attr);
	} else {
		if (!DeeError_Catch(&DeeError_AttributeError) &&
		    !DeeError_Catch(&DeeError_NotImplemented))
			goto err;
		/* Fallback: Convert an `int'-object into a unix file descriptor. */
		error = DeeObject_AsInt(ob, &result);
	}
	if unlikely(error)
		goto err;
	if unlikely(result == -1)
		goto err_fd_minus_one;
	return result;
err_fd_minus_one:
	DeeError_Throwf(&DeeError_FileClosed,
	                "Invalid file descriptor -1");
err:
	return -1;
}




/************************************************************************/
/* MMAP API                                                             */
/************************************************************************/

#ifdef __CYGWIN__
/* Cygwin's `getpagesize' is broken in that it returns the
 * allocation granularity instead of the actual page-size. */
#undef getpagesize
#define getpagesize() 4096
#elif !defined(CONFIG_HAVE_getpagesize)
#ifdef __ARCH_PAGESIZE
#define getpagesize() __ARCH_PAGESIZE
#elif defined(PAGESIZE)
#define getpagesize() PAGESIZE
#elif defined(PAGE_SIZE)
#define getpagesize() PAGE_SIZE
#elif defined(EXEC_PAGESIZE)
#define getpagesize() EXEC_PAGESIZE
#elif defined(NBPG) && defined(CLSIZE)
#define getpagesize() (NBPG * CLSIZE)
#elif defined(NBPG)
#define getpagesize() NBPG
#elif defined(_SC_PAGESIZE)
#define getpagesize() sysconf(_SC_PAGESIZE)
#elif defined(_SC_PAGE_SIZE)
#define getpagesize() sysconf(_SC_PAGE_SIZE)
#elif defined(CONFIG_HOST_WINDOWS)
#define getpagesize() dee_nt_getpagesize()
PRIVATE ATTR_CONST size_t DCALL dee_nt_getpagesize(void) {
	static size_t ps = 0;
	if (ps == 0) {
		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		ps = system_info.dwPageSize;
		if unlikely(ps == 0)
			ps = 1;
	}
	return ps;
}
#else /* ... */
#define getpagesize() 4096 /* Just guess... */
#endif /* !... */
#endif /* !CONFIG_HAVE_getpagesize */


#ifdef CONFIG_HOST_WINDOWS
#define STRUCT_STAT_FOR_SIZE_GETSIZE(x) ((Dee_pos_t)(x).QuadPart)
#define STRUCT_STAT_FOR_SIZE            LARGE_INTEGER
#define FSTAT_FOR_SIZE(fd, pst)         (GetFileSizeEx(fd, pst) ? 0 : -1)
#elif defined(CONFIG_HAVE_fstat64)
#define STRUCT_STAT_FOR_SIZE_GETSIZE(x) ((Dee_pos_t)(x).st_size)
#define STRUCT_STAT_FOR_SIZE            struct stat64
#define FSTAT_FOR_SIZE                  fstat64
#elif defined(CONFIG_HAVE_fstat)
#define STRUCT_STAT_FOR_SIZE_GETSIZE(x) ((Dee_pos_t)(x).st_size)
#define STRUCT_STAT_FOR_SIZE            struct stat
#define FSTAT_FOR_SIZE                  fstat
#endif /* ... */

#undef LSEEK
#ifdef CONFIG_HAVE_lseek64
#define LSEEK lseek64
#elif defined(CONFIG_HAVE_lseek)
#define LSEEK lseek
#endif /* ... */

#undef MMAP
#ifdef CONFIG_HAVE_mmap64
#define MMAP mmap64
#elif defined(CONFIG_HAVE_mmap)
#define MMAP mmap
#endif /* ... */

#undef PREAD
#ifdef CONFIG_HAVE_pread64
#define PREAD pread64
#elif defined(CONFIG_HAVE_pread)
#define PREAD pread
#endif /* ... */

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1l)
#endif /* !MAP_FAILED */


/* Configure `self' as using a heap buffer */
#ifdef DeeMapFile_IS_os_mapfile
#define DeeMapFile_SETHEAP(self) __mapfile_setheap(&(self)->dmf_map)
#elif defined(DeeMapFile_IS_CreateFileMapping)
#define DeeMapFile_SETHEAP(self) ((self)->_dmf_hmap = NULL)
#elif defined(DeeMapFile_IS_mmap)
#define DeeMapFile_SETHEAP(self) ((self)->_dmf_mapsize = 0)
#else /* ... */
#define DeeMapFile_SETHEAP(self) (void)0
#endif /* !... */

#ifdef DeeMapFile_IS_os_mapfile
#define DeeMapFile_SETADDR(self, p) (void)((self)->dmf_map.mf_addr = (unsigned char *)(p))
#define DeeMapFile_SETSIZE(self, s) (void)((self)->dmf_map.mf_size = (s))
#else /* DeeMapFile_IS_os_mapfile */
#define DeeMapFile_SETADDR(self, p) (void)((self)->dmf_addr = (void const *)(p))
#define DeeMapFile_SETSIZE(self, s) (void)((self)->dmf_size = (s))
#endif /* !DeeMapFile_IS_os_mapfile */



/* Finalize a given file map */
PUBLIC NONNULL((1)) void DCALL
DeeMapFile_Fini(struct DeeMapFile *__restrict self) {
#ifdef DeeMapFile_IS_os_mapfile
	(void)unmapfile(&self->dmf_map);
#elif defined(DeeMapFile_IS_CreateFileMapping)
	if (self->_dmf_hmap != NULL) {
		size_t psm = getpagesize() - 1;
		void *baseptr = (void *)((uintptr_t)self->dmf_addr & ~psm);
		if (self->_dmf_vfre) {
			void *vbas = (void *)(((uintptr_t)baseptr + self->dmf_size + psm) & ~psm);
			(void)VirtualFree(vbas, /*self->_dmf_vfre*/ 0, MEM_RELEASE);
		}
		(void)UnmapViewOfFile(baseptr);
		(void)CloseHandle(self->_dmf_hmap);
	} else {
		Dee_Free((void *)self->dmf_addr);
	}
#elif defined(DeeMapFile_IS_mmap)
	if (DeeMapFile_UsesMmap(self)) {
		size_t psm    = getpagesize() - 1;
		void *baseptr = (void *)((uintptr_t)self->dmf_addr & ~psm);
		(void)munmap(baseptr, self->_dmf_mapsize);
	} else {
		Dee_Free((void *)self->dmf_addr);
	}
#else /* ... */
	Dee_Free((void *)self->dmf_addr);
#endif /* !... */
}

/* Initialize a file mapping from a given system FD.
 * @param: fd:        The file that should be loaded into memory.
 * @param: self:      Filled with mapping information. This structure contains at least 2 fields:
 *                     - DeeMapFile_GetBase: Filled with the base address of a mapping of the file's contents
 *                     - DeeMapFile_GetSize: The actual number of mapped bytes (excluding `num_trailing_nulbytes')
 *                                           This will always be `>= min_bytes && <= max_bytes'.
 *                     - Other fields are implementation-specific
 *                    Note that the memory located at `DeeMapFile_GetBase' is writable, though changes to
 *                    it are guarantied not to be written back to `fd'. iow: it behaves like MAP_PRIVATE
 *                    mapped as PROT_READ|PROT_WRITE.
 * @param: offset:    File offset / number of leading bytes that should not be mapped
 *                    When set to `(Dee_pos_t)-1', use the fd's current file position.
 * @param: min_bytes: The  min number of bytes (excluding num_trailing_nulbytes) that should be mapped
 *                    starting  at `offset'. If the file is smaller than this, or indicates EOF before
 *                    this number of bytes has been reached,  nul bytes are mapped for its  remainder.
 *                    Note that this doesn't include `num_trailing_nulbytes', meaning that (e.g.) when
 *                    an entirely empty file is mapped you get a buffer like:
 *                    >> mf_addr = calloc(min_size + num_trailing_nulbytes);
 *                    >> mf_size = min_size;
 *                    This argument essentially acts as if `fd' was at least `min_bytes' bytes large
 *                    by filling the non-present address range with all zeroes.
 * @param: max_bytes: The max number of bytes (excluding num_trailing_nulbytes) that should be mapped
 *                    starting at `offset'. If the file is smaller than this, or indicates EOF before
 *                    this number of bytes has been reached, simply stop there. - The actual number of
 *                    mapped bytes (excluding `num_trailing_nulbytes') is `DeeMapFile_GetSize'.
 * @param: num_trailing_nulbytes: When non-zero, append this many trailing NUL-bytes at the end of
 *                    the mapping. More bytes than this may be appended if necessary, but at least
 *                    this many are guarantied to be. - Useful if you want to load a file as a
 *                    string, in which case you can specify `1' to always have a trailing '\0' be
 *                    appended:
 *                    >> bzero(DeeMapFile_GetBase + DeeMapFile_GetSize, num_trailing_nulbytes);
 * @param: flags:     Set of `DEE_MAPFILE_F_*'
 * @return:  1: Both `DEE_MAPFILE_F_MUSTMMAP' and `DEE_MAPFILE_F_TRYMMAP' were set, but mmap failed.
 * @return:  0: Success (`self' must be deleted using `DeeMapFile_Fini(3)')
 * @return: -1: Error (an exception was thrown) */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeMapFile_InitSysFd(struct DeeMapFile *__restrict self, Dee_fd_t fd,
                     Dee_pos_t offset, size_t min_bytes, size_t max_bytes,
                     size_t num_trailing_nulbytes, unsigned int flags) {
#ifdef DeeMapFile_IS_os_mapfile
	/* Special case: use `fmapfile(3)' */
	int result;
again:
	result = fmapfile(&self->dmf_map, fd,
	                  offset, min_bytes, max_bytes,
	                  num_trailing_nulbytes, flags);
	if (result != 0) {
		int error = DeeSystem_GetErrno();
		if (error == ENOMEM && Dee_CollectMemory(1)) {
			goto again;
		} else if (error == EINTR) {
			if (DeeThread_CheckInterrupt())
				goto done;
			goto again;
		} else if (error == EBADF) {
			result = DeeError_Throwf(&DeeError_FileClosed,
			                         "File descriptor %d was closed",
			                         fd);
		} else if (error == ENOTSUP) {
			if (flags & DEE_MAPFILE_F_TRYMMAP)
				return 1;
			return DeeError_Throwf(&DeeError_UnsupportedAPI,
			                       "File descriptor %" Dee_PRIpSYSFD " cannot be mmap'd",
			                       fd);
		} else {
			result = DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
			                                   "Failed to map file %d",
			                                   fd);
		}
	}
done:
	return result;
#elif defined(CONFIG_HOST_WINDOWS) || defined(CONFIG_HAVE_read)

	/* General implementation (stolen from KOS's `fmapfile(3)') */
#if defined(DeeMapFile_IS_CreateFileMapping) || defined(DeeMapFile_IS_mmap)
	STRUCT_STAT_FOR_SIZE st;
#endif /* DeeMapFile_IS_CreateFileMapping || DeeMapFile_IS_mmap */
	Dee_pos_t orig_offset = offset;
	unsigned char *buf;
	size_t bufsize;
	size_t bufused;
	size_t buffree;

	/* Try to use mmap(2) */
again:
#if defined(DeeMapFile_IS_CreateFileMapping) || defined(DeeMapFile_IS_mmap)
	if (FSTAT_FOR_SIZE(fd, &st) == 0) {
		Dee_pos_t map_offset = offset;
		size_t map_bytes     = max_bytes;
		if (map_offset == (Dee_pos_t)-1) {
			if unlikely(flags & DEE_MAPFILE_F_ATSTART) {
				map_offset = 0;
			} else {
				/* Use the file descriptors current offset. */
#ifdef DeeMapFile_IS_CreateFileMapping
				DWORD lo;
				LONG hi = 0;
				lo = SetFilePointer(fd, 0, &hi, FILE_CURRENT);
				if (lo == INVALID_SET_FILE_POINTER && GetLastError() != 0)
					goto after_mmap_attempt;
				map_offset = (Dee_pos_t)(((uint64_t)lo) |
				                         ((uint64_t)hi << 32));
#else /* DeeMapFile_IS_CreateFileMapping */
				map_offset = (Dee_pos_t)LSEEK(fd, 0, SEEK_CUR);
				if (map_offset == (Dee_pos_t)-1)
					goto after_mmap_attempt;
#endif /* !DeeMapFile_IS_CreateFileMapping */
			}
		}
		if (OVERFLOW_USUB(STRUCT_STAT_FOR_SIZE_GETSIZE(st), map_offset, &map_bytes)) {
			map_bytes = 0;
			if (STRUCT_STAT_FOR_SIZE_GETSIZE(st) > (Dee_pos_t)map_offset)
				map_bytes = (size_t)-1;
		}
		if (map_bytes >= max_bytes) {
			map_bytes = max_bytes;
#if __SIZEOF_SIZE_T__ < 8
			if (map_bytes == (size_t)-1) {
				/* Special case: caller wants to map the entire file, but it's too large. */
				Dee_pos_t true_size;
				true_size = STRUCT_STAT_FOR_SIZE_GETSIZE(st) - map_offset;
				if (true_size > (Dee_pos_t)(size_t)-1) {
					/* File is too large to be loaded into memory in its entirety. */
					if (flags & DEE_MAPFILE_F_MUSTMMAP)
						goto err_mmap_impossible;
#define WANT_err_mmap_impossible

					/* If we tried to allocate a buffer for this file in its entirety,
					 * we'd sooner or later end up OOM-ing, since the file is literally
					 * larger than what could fit into our address space.
					 *
					 * As such, just indicate a bad-allocation error with the max size. */
					return Dee_BadAlloc((size_t)-1);
				}
			}
#endif /* __SIZEOF_SIZE_T__ < 8 */
		}
		if (map_bytes) {
			/* Map file into memory. */
			size_t used_nulbytes;
			size_t psm = getpagesize() - 1;
			size_t addend, mapsize;
#ifdef DeeMapFile_IS_CreateFileMapping
			HANDLE hMap;
#endif /* DeeMapFile_IS_CreateFileMapping */
			addend = (size_t)(map_offset & psm);
			map_offset -= addend;
			mapsize = map_bytes + addend;
			used_nulbytes = num_trailing_nulbytes;
			if (min_bytes > map_bytes)
				used_nulbytes += min_bytes - map_bytes;
#ifdef DeeMapFile_IS_CreateFileMapping
			/* NOTE: This right here doesn't work in all cases, mainly due to
			 *       the fact that the `VirtualAlloc()' below fails if passed
			 *       a base address that isn't a multiple of 0x10000 (and we
			 *       need it to work if it's a multiple of 0x1000). Also, there
			 *       can be the case where `VirtualAlloc()' fails because the
			 *       relevant address is already mapped.
			 */
			hMap = CreateFileMappingA(fd, NULL,
			                          (flags & DEE_MAPFILE_F_MAPSHARED)
			                          ? PAGE_READWRITE
			                          : PAGE_WRITECOPY,
			                          0, 0, NULL);
			if (hMap == NULL || hMap == INVALID_HANDLE_VALUE)
				goto after_mmap_attempt;
			buf = (unsigned char *)MapViewOfFile(hMap,
			                                     (flags & DEE_MAPFILE_F_MAPSHARED)
			                                     ? (FILE_MAP_READ | FILE_MAP_WRITE)
			                                     : (FILE_MAP_COPY),
			                                     (DWORD)(map_offset >> 32),
			                                     (DWORD)(map_offset),
			                                     mapsize);
			if unlikely(buf == NULL) {
				(void)CloseHandle(hMap);
			} else
#else /* DeeMapFile_IS_CreateFileMapping */
			mapsize += used_nulbytes;
			mapsize = (mapsize + psm) & ~psm;
#ifndef CONFIG_HAVE_MAP_SHARED
			if unlikely(flags & DEE_MAPFILE_F_MAPSHARED) {
				return DeeError_Throwf(&DeeError_UnsupportedAPI,
				                       "MAP_SHARED isn't supported");
			}
#define LOCAL_USED_mmap_flags MAP_PRIVATE
#else /* !CONFIG_HAVE_MAP_SHARED */
#define LOCAL_USED_mmap_flags (flags & DEE_MAPFILE_F_MAPSHARED ? MAP_SHARED : MAP_PRIVATE)
#endif /* CONFIG_HAVE_MAP_SHARED */

#if defined(__CYGWIN__)
			/* Thanks to windows, cygwin has a (somewhat) broken mmap().
			 *
			 * Taken from `winsup/cygwin/mmap.cc:1108(mmap64)':
			 * """
			 * If the requested length is bigger than the file size, the remainder
			 * is created as anonymous mapping, as reserved pages which raise a SIGBUS
			 * when trying to access them.  This results in an allocation gap in the
			 * first 64K block the file ends in, but there's nothing at all we can do
			 * about that.
			 * """
			 *
			 * (It would have been really nice for cygwin to simply have mmap() return
			 * an error in this case, rather than literally creating a broken memory
			 * mapping, but oh well...)
			 */
			{
				size_t const psx = 0x10000 - 1;
				uint64_t true_filesize_psx, true_filesize_psm;
				true_filesize_psm = STRUCT_STAT_FOR_SIZE_GETSIZE(st);
				true_filesize_psm = (true_filesize_psm + psm) & ~psm; /* Hardware page-size-aligned true file size */
				true_filesize_psx = (true_filesize_psm + psx) & ~psx; /* Allocation granularity-aligned true file size (read: non-sense enforced by the windows kernel) */
				if ((map_offset + mapsize) > true_filesize_psm &&     /* Does the mapping include pages beyond the hardware end-of-file? */
				    true_filesize_psx != true_filesize_psm) {         /* Would there be an unmapped (SIGSEGV) hole after the true end-of-file? */
					buf = (unsigned char *)MAP_FAILED;
				} else {
					buf = (unsigned char *)MMAP(NULL, mapsize, PROT_READ | PROT_WRITE,
					                            LOCAL_USED_mmap_flags, fd, map_offset);
				}
			}
#else /* ... */
			buf = (unsigned char *)MMAP(NULL, mapsize, PROT_READ | PROT_WRITE,
			                            LOCAL_USED_mmap_flags, fd, map_offset);
#endif /* !... */

			/*Dee_DPRINTF("mmap(%Iu, %d, %I64d) -> %p [psm: %Iu, errno:%d:%s, true_filesize: %I64u]\n",
			            mapsize, fd, map_offset, buf, psm, errno, strerror(errno), (uint64_t)STRUCT_STAT_FOR_SIZE_GETSIZE(st));*/
#undef LOCAL_USED_mmap_flags
			if likely(buf != (unsigned char *)MAP_FAILED)
#endif /* !DeeMapFile_IS_CreateFileMapping */
			{
				/* Clear out the caller-required trailing NUL bytes.
				 * We do this in a kind-of special way that tries not
				 * to write-fault memory if it already contains NULs. */
				unsigned char *nul;
				buf += addend;
				nul = buf + map_bytes;
#ifdef DeeMapFile_IS_CreateFileMapping
				self->_dmf_vfre = 0;
#endif /* DeeMapFile_IS_CreateFileMapping */
				if (used_nulbytes) {
#ifdef DeeMapFile_IS_CreateFileMapping
					size_t bytes_in_page;
					bytes_in_page = (psm + 1) - ((uintptr_t)nul & psm);
					if (used_nulbytes > bytes_in_page) {
						void *tail_base;
						size_t tail_size;
						tail_base = (void *)(nul + bytes_in_page);
						tail_size = used_nulbytes - bytes_in_page;
						tail_size = (tail_size + psm) & ~psm;
						/* Must map bss memory at `tail_base...+=tail_size'
						 * If mapping memory here isn't possible, try to map
						 * the file once again, but at a different location.
						 *
						 * Do this a couple of times, and if we get overlap errors
						 * every single time, just give up eventually and jump to
						 * the mmap-not-supported handler. */
						if (!VirtualAlloc(tail_base, tail_size,
						                  MEM_COMMIT | MEM_RESERVE,
						                  PAGE_READWRITE)) {
							(void)UnmapViewOfFile(buf);
							(void)CloseHandle(hMap);
							goto after_mmap_attempt;
						}
						self->_dmf_vfre = tail_size;
					}
#endif /* DeeMapFile_IS_CreateFileMapping */
					do {
						if (*nul) {
							bzero(nul, used_nulbytes);
							break;
						}
						--used_nulbytes;
						++nul;
					} while (used_nulbytes);
				}
				DeeMapFile_SETADDR(self, buf);
				DeeMapFile_SETSIZE(self, map_bytes);
#ifdef DeeMapFile_IS_CreateFileMapping
				self->_dmf_hmap = (void *)hMap;
#else /* DeeMapFile_IS_CreateFileMapping */
				self->_dmf_mapsize = mapsize;
#endif /* !DeeMapFile_IS_CreateFileMapping */
				return 0;
			}
		} else {
			/* Special files from procfs indicate their size as `0', even
			 * though they aren't actually empty. - As such, we can't just
			 * use the normal approach of read(2)-ing the file.
			 *
			 * Only if at that point it still indicates being empty, are we
			 * actually allowed to believe that claim! */
		}
	}
after_mmap_attempt:
#endif /* DeeMapFile_IS_CreateFileMapping || DeeMapFile_IS_mmap */
	if (flags & DEE_MAPFILE_F_MUSTMMAP) {
#ifdef WANT_err_mmap_impossible
#undef WANT_err_mmap_impossible
err_mmap_impossible:
#endif /* WANT_err_mmap_impossible */
		if (flags & DEE_MAPFILE_F_TRYMMAP)
			return 1;
		return DeeError_Throwf(&DeeError_UnsupportedAPI,
		                       "File descriptor %" Dee_PRIpSYSFD " cannot be mmap'd",
		                       fd);
	}

	/* Allocate a heap buffer. */
	bufsize = max_bytes;
	if (bufsize > 0x10000)
		bufsize = 0x10000;
	if (bufsize < min_bytes)
		bufsize = min_bytes;
	buf = (unsigned char *)Dee_TryMalloc(bufsize + num_trailing_nulbytes);
	if unlikely(!buf) {
		bufsize = 1;
		if (bufsize < min_bytes)
			bufsize = min_bytes;
		buf = (unsigned char *)Dee_Malloc(bufsize + num_trailing_nulbytes);
		if unlikely(!buf)
			return -1;
	}
	bufused = 0;
	buffree = bufsize;

	if (offset != (Dee_pos_t)-1 && (offset != 0 || !(flags & DEE_MAPFILE_F_ATSTART))) {
		/* Try to use pread(2) */
#if defined(CONFIG_HOST_WINDOWS) || defined(PREAD)
		for (;;) {
#ifdef CONFIG_HOST_WINDOWS
			DWORD error;
			OVERLAPPED ol;
			size_t readsize = buffree;
#if __SIZEOF_SIZE_T__ > 4
			if (readsize > (size_t)(DWORD)-1)
				readsize = (size_t)(DWORD)-1;
#endif /* __SIZEOF_SIZE_T__ > 4 */
			bzero(&ol, sizeof(ol));
			ol.Offset     = (DWORD)(offset);
			ol.OffsetHigh = (DWORD)(offset >> 32);
			if (!ReadFile(fd, buf + bufused, (DWORD)readsize, &error, &ol)) {
				if (bufused == 0)
					break; /* File probably doesn't support `pread(2)'... */
				/* Read error */
				goto system_err_buf;
			}
			if (error == 0 || (!(flags & DEE_MAPFILE_F_READALL) && error < (DWORD)readsize)) {
				/* End-of-file! */
				unsigned char *newbuf;
				size_t used_nulbytes;
				bufused += (size_t)error;
				used_nulbytes = num_trailing_nulbytes;
				if (min_bytes > bufused)
					used_nulbytes += min_bytes - bufused;
				newbuf = (unsigned char *)Dee_TryRealloc(buf, bufused + used_nulbytes);
				if likely(newbuf)
					buf = newbuf;
				bzero(buf + bufused, used_nulbytes); /* Trailing NUL-bytes */
				DeeMapFile_SETADDR(self, buf);
				DeeMapFile_SETSIZE(self, bufused);
				DeeMapFile_SETHEAP(self);
				return 0;
			}
#else /* CONFIG_HOST_WINDOWS */
			Dee_ssize_t error;
			error = PREAD(fd, buf + bufused, buffree, offset);
			if (error <= 0 || (!(flags & DEE_MAPFILE_F_READALL) && (size_t)error < buffree)) {
				if ((size_t)error < buffree) {
					/* End-of-file! */
					unsigned char *newbuf;
					size_t used_nulbytes;
					bufused += (size_t)error;
					used_nulbytes = num_trailing_nulbytes;
					if (min_bytes > bufused)
						used_nulbytes += min_bytes - bufused;
					newbuf = (unsigned char *)Dee_TryRealloc(buf, bufused + used_nulbytes);
					if likely(newbuf)
						buf = newbuf;
					bzero(buf + bufused, used_nulbytes); /* Trailing NUL-bytes */
					DeeMapFile_SETADDR(self, buf);
					DeeMapFile_SETSIZE(self, bufused);
					DeeMapFile_SETHEAP(self);
					return 0;
				}
				if (bufused == 0)
					break; /* File probably doesn't support `pread(2)'... */
				/* Read error */
				goto system_err_buf;
			}
#endif /* !CONFIG_HOST_WINDOWS */
			offset  += (size_t)error;
			bufused += (size_t)error;
			buffree -= (size_t)error;
			if (buffree < 1024) {
				unsigned char *newbuf;
				size_t newsize = bufsize * 2;
				newbuf = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
				if (!newbuf) {
					newsize = bufsize + 1024;
					newbuf = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
					if (!newbuf) {
						if (!buffree) {
							newsize = bufsize + 1;
							newbuf  = (unsigned char *)Dee_Realloc(buf, newsize + num_trailing_nulbytes);
							if unlikely(!newbuf)
								goto err_buf;
						} else {
							newsize = bufsize;
							newbuf  = buf;
						}
					}
				}
				buffree += newsize - bufsize;
				bufsize = newsize;
				buf     = newbuf;
			}
		}
#endif /* CONFIG_HOST_WINDOWS || PREAD */

		/* For a custom offset, try to use lseek() (or read()) */
		{
#if defined(CONFIG_HOST_WINDOWS) || (defined(LSEEK) && defined(SEEK_SET))
#ifdef CONFIG_HOST_WINDOWS
			LONG offset_hi = (LONG)(DWORD)(offset >> 32);
			if (SetFilePointer(fd, (LONG)(DWORD)offset, &offset_hi,
			                   FILE_BEGIN) != INVALID_SET_FILE_POINTER ||
			    GetLastError() == 0)
#else /* CONFIG_HOST_WINDOWS */
			if (LSEEK(fd, offset, SEEK_SET) != -1)
#endif /* !CONFIG_HOST_WINDOWS */
			{
				/* Was able to lseek(2) */
			} else
#endif /* CONFIG_HOST_WINDOWS || (LSEEK && SEEK_SET) */
			{
				/* Try to use read(2) to skip leading data. */
				while (offset) {
					size_t skip = bufsize + num_trailing_nulbytes;
					if ((Dee_pos_t)skip > offset)
						skip = (size_t)offset;
					{
#ifdef CONFIG_HOST_WINDOWS
						DWORD error;
						size_t readsize = buffree;
#if __SIZEOF_SIZE_T__ > 4
						if (readsize > (size_t)(DWORD)-1)
							readsize = (size_t)(DWORD)-1;
#endif /* __SIZEOF_SIZE_T__ > 4 */
						if (!ReadFile(fd, buf, (DWORD)readsize, &error, NULL))
							goto system_err_buf;
						if (!error || (!(flags & DEE_MAPFILE_F_READALL) && error < (DWORD)readsize))
							goto empty_file; /* EOF reached before `offset' */
#else /* CONFIG_HOST_WINDOWS */
						Dee_ssize_t error;
						error = read(fd, buf, skip);
						if (error <= 0 || (!(flags & DEE_MAPFILE_F_READALL) && (size_t)error < skip)) {
							if (error < 0)
								goto system_err_buf;
							goto empty_file; /* EOF reached before `offset' */
						}
#endif /* !CONFIG_HOST_WINDOWS */
						offset -= (size_t)error;
					}
				}
			}
		}
	}

	/* Use read(2) as fallback */
	for (;;) {
#ifdef CONFIG_HOST_WINDOWS
		DWORD error;
		size_t readsize = buffree;
#if __SIZEOF_SIZE_T__ > 4
		if (readsize > (size_t)(DWORD)-1)
			readsize = (size_t)(DWORD)-1;
#endif /* __SIZEOF_SIZE_T__ > 4 */
		if (!ReadFile(fd, buf, (DWORD)readsize, &error, NULL))
			goto system_err_buf;
		if (!error || (!(flags & DEE_MAPFILE_F_READALL) && error < (DWORD)readsize)) {
			/* End-of-file! */
			unsigned char *newbuf;
			size_t used_nulbytes;
			bufused += (size_t)error;
			used_nulbytes = num_trailing_nulbytes;
			if (min_bytes > bufused)
				used_nulbytes += min_bytes - bufused;
			newbuf = (unsigned char *)Dee_TryRealloc(buf, bufused + used_nulbytes);
			if likely(newbuf)
				buf = newbuf;
			bzero(buf + bufused, used_nulbytes); /* Trailing NUL-bytes */
			DeeMapFile_SETADDR(self, buf);
			DeeMapFile_SETSIZE(self, bufused);
			DeeMapFile_SETHEAP(self);
			return 0;
		}
#else /* CONFIG_HOST_WINDOWS */
		Dee_ssize_t error;
		error = read(fd, buf + bufused, buffree);
		if (error <= 0 || (!(flags & DEE_MAPFILE_F_READALL) && (size_t)error < buffree)) {
			if (error >= 0) {
				/* End-of-file! */
				unsigned char *newbuf;
				size_t used_nulbytes;
				bufused += (size_t)error;
				used_nulbytes = num_trailing_nulbytes;
				if (min_bytes > bufused)
					used_nulbytes += min_bytes - bufused;
				newbuf = (unsigned char *)Dee_TryRealloc(buf, bufused + used_nulbytes);
				if likely(newbuf)
					buf = newbuf;
				bzero(buf + bufused, used_nulbytes); /* Trailing NUL-bytes */
				DeeMapFile_SETADDR(self, buf);
				DeeMapFile_SETSIZE(self, bufused);
				DeeMapFile_SETHEAP(self);
				return 0;
			}
			/* Read error */
			goto system_err_buf;
		}
#endif /* !CONFIG_HOST_WINDOWS */
		bufused += (size_t)error;
		buffree -= (size_t)error;
		if (buffree < 1024) {
			unsigned char *newbuf;
			size_t newsize = bufsize * 2;
			newbuf = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
			if (!newbuf) {
				newsize = bufsize + 1024;
				newbuf  = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
				if (!newbuf) {
					if (!buffree) {
						newsize = bufsize + 1;
						newbuf  = (unsigned char *)Dee_Realloc(buf, newsize + num_trailing_nulbytes);
						if unlikely(!newbuf)
							goto err_buf;
					} else {
						newsize = bufsize;
						newbuf  = buf;
					}
				}
			}
			buffree += newsize - bufsize;
			bufsize = newsize;
			buf     = newbuf;
		}
	}

	/*--------------------------------------------------------------------*/
	{
		unsigned char *newbuf;
		size_t used_nulbytes;
		/* Because of how large our original buffer was, and because at this
		 * point all we want to do is return a `num_trailing_nulbytes'-
		 * large buffer of all NUL-bytes, it's probably more efficient to
		 * allocate a new (small) buffer, than trying to realloc the old
		 * buffer. If we try to do realloc(), the heap might see that all
		 * we're trying to do is truncate the buffer, and so might choose
		 * not to alter its base address, which (if done repeatedly) might
		 * lead to memory becoming very badly fragmented. */
empty_file:
		used_nulbytes = min_bytes + num_trailing_nulbytes;
		newbuf = (unsigned char *)Dee_TryCalloc(used_nulbytes);
		if likely(newbuf) {
			Dee_Free(buf);
		} else {
			newbuf = (unsigned char *)Dee_TryRealloc(buf, used_nulbytes);
			if (!newbuf)
				newbuf = buf;
			bzero(newbuf, used_nulbytes);
		}
		DeeMapFile_SETADDR(self, newbuf);
		DeeMapFile_SETSIZE(self, 0);
		DeeMapFile_SETHEAP(self);
	}
	return 0;
	{
		/* Throw/handle system errors */
#ifdef CONFIG_HOST_WINDOWS
		DWORD error;
system_err_buf:
		error = GetLastError();
		if (DeeNTSystem_IsBadAllocError(error) && Dee_CollectMemory(1)) {
			offset = orig_offset;
			goto again;
		} else if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt() == 0) {
				offset = orig_offset;
				goto again;
			}
		} else if (DeeNTSystem_IsBadF(error)) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "File descriptor %d was closed",
			                fd);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, error,
			                        "Failed to map file %d", fd);
		}
#else /* CONFIG_HOST_WINDOWS */
		int error;
system_err_buf:
		error = DeeSystem_GetErrno();
#ifdef ENOMEM
		if (error == ENOMEM) {
			if (Dee_CollectMemory(1)) {
				offset = orig_offset;
				goto again;
			}
			goto err_buf;
		}
#endif /* ENOMEM */
#ifdef EINTR
		if (error == EINTR) {
			if (DeeThread_CheckInterrupt())
				goto err_buf;
			offset = orig_offset;
			goto again;
		}
#endif /* EINTR */
#ifdef EBADF
		if (error == EBADF) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "File descriptor %d was closed",
			                fd);
			goto err_buf;
		}
#endif /* EBADF */
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to map file %d", fd);
#endif /* !CONFIG_HOST_WINDOWS */
	}
err_buf:
	Dee_Free(buf);
	return -1;
#else /* ... */
	return DeeError_Throwf(&DeeError_UnsupportedAPI, "File mappings aren't supported");
#endif /* !... */
}

/* Same as `DeeMapFile_InitSysFd()', but initialize from a deemon File object. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeMapFile_InitFile(struct DeeMapFile *__restrict self, DeeObject *__restrict file,
                    Dee_pos_t offset, size_t min_bytes, size_t max_bytes,
                    size_t num_trailing_nulbytes, unsigned int flags) {
	unsigned char *buf;
	size_t bufsize;
	size_t bufused;
	size_t buffree;

	if (DeeSystemFile_Check(file)) {
		Dee_fd_t sfd = DeeSystemFile_Fileno(file);
		if (sfd == Dee_fd_INVALID)
			return -1;
		return DeeMapFile_InitSysFd(self, sfd,
		                            offset, min_bytes, max_bytes,
		                            num_trailing_nulbytes, flags);
	}
	if (flags & DEE_MAPFILE_F_MUSTMMAP) {
		if (flags & DEE_MAPFILE_F_TRYMMAP)
			return 1;
		return DeeError_Throwf(&DeeError_UnsupportedAPI,
		                       "Cannot mmap objects of type %r",
		                       Dee_TYPE(file));
	}
	/* Same as the after-mmap code in `DeeMapFile_InitSysFd()',
	 * but using deemon's file API, rather than the system's! */

	/* Allocate a heap buffer. */
	bufsize = max_bytes;
	if (bufsize > 0x10000)
		bufsize = 0x10000;
	if (bufsize < min_bytes)
		bufsize = min_bytes;
	buf = (unsigned char *)Dee_TryMalloc(bufsize + num_trailing_nulbytes);
	if unlikely(!buf) {
		bufsize = 1;
		buf = (unsigned char *)Dee_Malloc(bufsize + num_trailing_nulbytes);
		if unlikely(!buf)
			return -1;
	}
	bufused = 0;
	buffree = bufsize;

	if (offset != (Dee_pos_t)-1 && (offset != 0 || !(flags & DEE_MAPFILE_F_ATSTART))) {
		/* Try to use pread(2) */
		for (;;) {
			size_t error;
			error = (flags & DEE_MAPFILE_F_READALL)
			        ? DeeFile_PReadAll(file, buf + bufused, buffree, offset)
			        : DeeFile_PRead(file, buf + bufused, buffree, offset);
			if unlikely(error == (size_t)-1) {
				if (DeeError_Catch(&DeeError_NotImplemented))
					break; /* Try to use seek+read instead */
				goto err_buf;
			}
			if (error < buffree) {
				/* End-of-file! */
				unsigned char *newbuf;
				size_t used_nulbytes;
				bufused += error;
				used_nulbytes = num_trailing_nulbytes;
				if (min_bytes > bufused)
					used_nulbytes += min_bytes - bufused;
				newbuf = (unsigned char *)Dee_TryRealloc(buf, bufused + used_nulbytes);
				if likely(newbuf)
					buf = newbuf;
				bzero(buf + bufused, used_nulbytes); /* Trailing NUL-bytes */
				DeeMapFile_SETADDR(self, buf);
				DeeMapFile_SETSIZE(self, bufused);
				DeeMapFile_SETHEAP(self);
				return 0;
			}
			offset  += error;
			bufused += error;
			buffree -= error;
			if (buffree < 1024) {
				unsigned char *newbuf;
				size_t newsize = bufsize * 2;
				newbuf = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
				if (!newbuf) {
					newsize = bufsize + 1024;
					newbuf = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
					if (!newbuf) {
						if (!buffree) {
							newsize = bufsize + 1;
							newbuf  = (unsigned char *)Dee_Realloc(buf, newsize + num_trailing_nulbytes);
							if unlikely(!newbuf)
								goto err_buf;
						} else {
							newsize = bufsize;
							newbuf  = buf;
						}
					}
				}
				buffree += newsize - bufsize;
				bufsize = newsize;
				buf     = newbuf;
			}
		}

		/* For a custom offset, try to use lseek() (or read()) */
		if (DeeFile_Seek(file, offset, SEEK_SET) != (Dee_pos_t)-1) {
			/* Was able to lseek(2) */
		} else {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL))
				goto err_buf;
			/* Try to use read(2) to skip leading data. */
			while (offset) {
				size_t error;
				size_t skip = bufsize + num_trailing_nulbytes;
				if ((Dee_pos_t)skip > offset)
					skip = (size_t)offset;
				error = (flags & DEE_MAPFILE_F_READALL)
				        ? DeeFile_ReadAll(file, buf, skip)
				        : DeeFile_Read(file, buf, skip);
				if unlikely(error == (size_t)-1)
					goto err_buf;
				if (error < skip)
					goto empty_file; /* EOF reached before `offset' */
				offset -= error;
			}
		}
	}

	/* Use read(2) as fallback */
	for (;;) {
		size_t error;
		error = (flags & DEE_MAPFILE_F_READALL)
		        ? DeeFile_ReadAll(file, buf + bufused, buffree)
		        : DeeFile_Read(file, buf + bufused, buffree);
		if unlikely(error == (size_t)-1)
			goto err_buf;
		if (error < buffree) {
			/* End-of-file! */
			unsigned char *newbuf;
			size_t used_nulbytes;
			bufused += error;
			used_nulbytes = num_trailing_nulbytes;
			if (min_bytes > bufused)
				used_nulbytes += min_bytes - bufused;
			newbuf = (unsigned char *)Dee_TryRealloc(buf, bufused + used_nulbytes);
			if likely(newbuf)
				buf = newbuf;
			bzero(buf + bufused, used_nulbytes); /* Trailing NUL-bytes */
			DeeMapFile_SETADDR(self, buf);
			DeeMapFile_SETSIZE(self, bufused);
			DeeMapFile_SETHEAP(self);
			return 0;
		}
		bufused += error;
		buffree -= error;
		if (buffree < 1024) {
			unsigned char *newbuf;
			size_t newsize = bufsize * 2;
			newbuf = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
			if (!newbuf) {
				newsize = bufsize + 1024;
				newbuf  = (unsigned char *)Dee_TryRealloc(buf, newsize + num_trailing_nulbytes);
				if (!newbuf) {
					if (!buffree) {
						newsize = bufsize + 1;
						newbuf  = (unsigned char *)Dee_Realloc(buf, newsize + num_trailing_nulbytes);
						if unlikely(!newbuf)
							goto err_buf;
					} else {
						newsize = bufsize;
						newbuf  = buf;
					}
				}
			}
			buffree += newsize - bufsize;
			bufsize = newsize;
			buf     = newbuf;
		}
	}

	/*--------------------------------------------------------------------*/
	{
		unsigned char *newbuf;
		size_t used_nulbytes;
		/* Because of how large our original buffer was, and because at this
		 * point all we want to do is return a `num_trailing_nulbytes'-
		 * large buffer of all NUL-bytes, it's probably more efficient to
		 * allocate a new (small) buffer, than trying to realloc the old
		 * buffer. If we try to do realloc(), the heap might see that all
		 * we're trying to do is truncate the buffer, and so might choose
		 * not to alter its base address, which (if done repeatedly) might
		 * lead to memory becoming very badly fragmented. */
empty_file:
		used_nulbytes = min_bytes + num_trailing_nulbytes;
		newbuf = (unsigned char *)Dee_TryCalloc(used_nulbytes);
		if likely(newbuf) {
			Dee_Free(buf);
		} else {
			newbuf = (unsigned char *)Dee_TryRealloc(buf, used_nulbytes);
			if (!newbuf)
				newbuf = buf;
			bzero(newbuf, used_nulbytes);
		}
		DeeMapFile_SETADDR(self, newbuf);
		DeeMapFile_SETSIZE(self, 0);
		DeeMapFile_SETHEAP(self);
	}
	return 0;
err_buf:
	Dee_Free(buf);
	return -1;
}

DECL_END

#ifndef __INTELLISENSE__
#ifdef CONFIG_HOST_WINDOWS
#include "system-nt.c.inl"
#endif /* CONFIG_HOST_WINDOWS */

#include "system-unix.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_SYSTEM_SYSTEM_C */
