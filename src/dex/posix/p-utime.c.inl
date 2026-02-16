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
#ifndef GUARD_DEX_POSIX_P_UTIME_C_INL
#define GUARD_DEX_POSIX_P_UTIME_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_UnpackStructKw */
#include <deemon/dex.h>             /* DEXSYM_READONLY, DEX_MEMBER_F */
#include <deemon/error.h>           /* DeeError_Catch, DeeError_NoSymlink */
#include <deemon/none.h>            /* DeeNone_Check, Dee_None, return_none */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_AssertTypeExact, Dee_AsObject, Dee_Decref */
#include <deemon/objmethod.h>       /*  */
#include <deemon/string.h>          /* DeeString*, Dee_wchar_t */
#include <deemon/system-features.h> /* AT_CHANGE_BTIME, AT_EMPTY_PATH, AT_FDCWD, AT_SYMLINK_NOFOLLOW, CONFIG_HAVE_*, CONFIG_PREFER_WCHAR_FUNCTIONS, DeeSystem_GetErrno, O_NOFOLLOW, O_RDWR, creat64, futime, futime32, futime64, futimens, futimens64, futimes, futimes64, lutimens, lutimens64, lutimes, lutimes64, openat64, open64, utimbuf, utimbuf32, utimbuf64, utimens, utimensat, utimensat64, utimens64, utimes, utimes64, wcreat64, wopenat64, wopen64 */
#include <deemon/system.h>          /* DeeNTSystem_CreateFileNoATime, DeeNTSystem_GetHandle, DeeNT_DWORD, DeeUnixSystem_GetFD, DeeUnixSystem_HandleGenericError */
#include <deemon/type.h>            /* METHOD_FNORMAL */

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */

#include "p-path.c.inl"     /* For `posix_utime_USE_posix_readlink__AND__posix_lutime()' */
#include "p-readlink.c.inl" /* For `posix_utime_USE_posix_readlink__AND__posix_lutime()' */
#include "p-stat.c.inl"     /* For `Dee_STAT_F_LSTAT', `stat_get_ctime_IS_stat_get_mtime' */

#include <stddef.h> /* NULL, size_t, wchar_t */

DECL_BEGIN

/* Re-link system functions to try and use 64-bit variants (if available) */
/*[[[deemon
local functions = {
	"open",
	"openat",
	"creat",
	"wopen",
	"wopenat",
	"wcreat",
};
for (local f: functions) {
	print("#ifdef CONFIG_HAVE_", f, "64");
	print("#undef CONFIG_HAVE_", f);
	print("#define CONFIG_HAVE_", f);
	print("#undef ", f);
	print("#define ", f, " ", f, "64");
	print("#endif /" "* CONFIG_HAVE_", f, "64 *" "/");
}
]]]*/
#ifdef CONFIG_HAVE_open64
#undef CONFIG_HAVE_open
#define CONFIG_HAVE_open
#undef open
#define open open64
#endif /* CONFIG_HAVE_open64 */
#ifdef CONFIG_HAVE_openat64
#undef CONFIG_HAVE_openat
#define CONFIG_HAVE_openat
#undef openat
#define openat openat64
#endif /* CONFIG_HAVE_openat64 */
#ifdef CONFIG_HAVE_creat64
#undef CONFIG_HAVE_creat
#define CONFIG_HAVE_creat
#undef creat
#define creat creat64
#endif /* CONFIG_HAVE_creat64 */
#ifdef CONFIG_HAVE_wopen64
#undef CONFIG_HAVE_wopen
#define CONFIG_HAVE_wopen
#undef wopen
#define wopen wopen64
#endif /* CONFIG_HAVE_wopen64 */
#ifdef CONFIG_HAVE_wopenat64
#undef CONFIG_HAVE_wopenat
#define CONFIG_HAVE_wopenat
#undef wopenat
#define wopenat wopenat64
#endif /* CONFIG_HAVE_wopenat64 */
#ifdef CONFIG_HAVE_wcreat64
#undef CONFIG_HAVE_wcreat
#define CONFIG_HAVE_wcreat
#undef wcreat
#define wcreat wcreat64
#endif /* CONFIG_HAVE_wcreat64 */
/*[[[end]]]*/


/* Check if we can use `AT_CHANGE_BTIME' to implement 3-channel utime() functions. */
#undef utimens_3
#undef futimens_3
#undef lutimens_3
#undef utimens64_3
#undef futimens64_3
#undef lutimens64_3
#ifdef CONFIG_HAVE_AT_CHANGE_BTIME
#ifdef CONFIG_HAVE_utimensat
#define futimens_3(fd, tsv)   utimensat(fd, NULL, tsv, AT_CHANGE_BTIME)
#ifdef CONFIG_HAVE_AT_FDCWD
#define utimens_3(path, tsv)  utimensat(AT_FDCWD, path, tsv, AT_CHANGE_BTIME)
#ifdef CONFIG_HAVE_AT_SYMLINK_NOFOLLOW
#define lutimens_3(path, tsv) utimensat(AT_FDCWD, path, tsv, AT_CHANGE_BTIME | AT_SYMLINK_NOFOLLOW)
#endif /* CONFIG_HAVE_AT_SYMLINK_NOFOLLOW */
#endif /* CONFIG_HAVE_AT_FDCWD */
#endif /* CONFIG_HAVE_utimensat */
#ifdef CONFIG_HAVE_utimensat64
#define futimens64_3(fd, tsv)   utimensat64(fd, NULL, tsv, AT_CHANGE_BTIME)
#ifdef CONFIG_HAVE_AT_FDCWD
#define utimens64_3(path, tsv)  utimensat64(AT_FDCWD, path, tsv, AT_CHANGE_BTIME)
#ifdef CONFIG_HAVE_AT_SYMLINK_NOFOLLOW
#define lutimens64_3(path, tsv) utimensat64(AT_FDCWD, path, tsv, AT_CHANGE_BTIME | AT_SYMLINK_NOFOLLOW)
#endif /* CONFIG_HAVE_AT_SYMLINK_NOFOLLOW */
#endif /* CONFIG_HAVE_AT_FDCWD */
#endif /* CONFIG_HAVE_utimensat64 */
#endif /* CONFIG_HAVE_AT_CHANGE_BTIME */


/* Figure out how we want to implement `utime()' */
#undef posix_utime_USE_nt_SetFileTime
#undef posix_utime_USE_utimens64_3
#undef posix_utime_USE_utimens_3
#undef posix_utime_USE_utimens64
#undef posix_utime_USE_utimens
#undef posix_utime_USE_utimes64
#undef posix_utime_USE_utimes
#undef posix_utime_USE_wutime64
#undef posix_utime_USE_wutime32
#undef posix_utime_USE_wutime
#undef posix_utime_USE_utime64
#undef posix_utime_USE_utime32
#undef posix_utime_USE_utime
#undef posix_utime_USE_wopen_AND_futime64
#undef posix_utime_USE_wopen_AND_futime32
#undef posix_utime_USE_wopen_AND_futime
#undef posix_utime_USE_open_AND_futime64
#undef posix_utime_USE_open_AND_futime32
#undef posix_utime_USE_open_AND_futime
#undef posix_utime_USE_posix_readlink__AND__posix_lutime
#undef posix_utime_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_utime_USE_nt_SetFileTime
#elif defined(utimens64_3)
#define posix_utime_USE_utimens64_3
#elif defined(CONFIG_HAVE_utimens64)
#define posix_utime_USE_utimens64
#elif defined(CONFIG_HAVE_utimes64)
#define posix_utime_USE_utimes64
#elif defined(utimens_3)
#define posix_utime_USE_utimens_3
#elif defined(CONFIG_HAVE_utimens)
#define posix_utime_USE_utimens
#elif defined(CONFIG_HAVE_utimes)
#define posix_utime_USE_utimes
#elif defined(CONFIG_HAVE_wutime64) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_utime_USE_wutime64
#elif defined(CONFIG_HAVE_utime64)
#define posix_utime_USE_utime64
#elif defined(CONFIG_HAVE_wutime64)
#define posix_utime_USE_wutime64
#elif defined(CONFIG_HAVE_wutime) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_utime_USE_wutime
#elif defined(CONFIG_HAVE_utime)
#define posix_utime_USE_utime
#elif defined(CONFIG_HAVE_wutime)
#define posix_utime_USE_wutime
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime64) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_utime_USE_wopen_AND_futime64
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime64)
#define posix_utime_USE_open_AND_futime64
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime64)
#define posix_utime_USE_wopen_AND_futime64
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_utime_USE_wopen_AND_futime
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime)
#define posix_utime_USE_open_AND_futime
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime)
#define posix_utime_USE_wopen_AND_futime
#elif defined(CONFIG_HAVE_wutime32) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_utime_USE_wutime32
#elif defined(CONFIG_HAVE_utime32)
#define posix_utime_USE_utime32
#elif defined(CONFIG_HAVE_wutime32)
#define posix_utime_USE_wutime32
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime32) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_utime_USE_wopen_AND_futime32
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime32)
#define posix_utime_USE_open_AND_futime32
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_futime32)
#define posix_utime_USE_wopen_AND_futime32
#else /* ... */
#define posix_utime_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `lutime()' */
#undef posix_lutime_USE_nt_SetFileTime
#undef posix_lutime_USE_lutimens64_3
#undef posix_lutime_USE_lutimens_3
#undef posix_lutime_USE_lutimens64
#undef posix_lutime_USE_lutimens
#undef posix_lutime_USE_lutimes64
#undef posix_lutime_USE_lutimes
#undef posix_lutime_USE_wlutime64
#undef posix_lutime_USE_wlutime32
#undef posix_lutime_USE_wlutime
#undef posix_lutime_USE_lutime64
#undef posix_lutime_USE_lutime32
#undef posix_lutime_USE_lutime
#undef posix_lutime_USE_wopen_AND_futime64
#undef posix_lutime_USE_wopen_AND_futime32
#undef posix_lutime_USE_wopen_AND_futime
#undef posix_lutime_USE_open_AND_futime64
#undef posix_lutime_USE_open_AND_futime32
#undef posix_lutime_USE_open_AND_futime
#undef posix_lutime_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_lutime_USE_nt_SetFileTime
#elif defined(lutimens64_3)
#define posix_lutime_USE_lutimens64_3
#elif defined(CONFIG_HAVE_lutimens64)
#define posix_lutime_USE_lutimens64
#elif defined(CONFIG_HAVE_lutimes64)
#define posix_lutime_USE_lutimes64
#elif defined(lutimens_3)
#define posix_lutime_USE_lutimens_3
#elif defined(CONFIG_HAVE_lutimens)
#define posix_lutime_USE_lutimens
#elif defined(CONFIG_HAVE_lutimes)
#define posix_lutime_USE_lutimes
#elif defined(CONFIG_HAVE_wlutime64) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lutime_USE_wlutime64
#elif defined(CONFIG_HAVE_lutime64)
#define posix_lutime_USE_lutime64
#elif defined(CONFIG_HAVE_wlutime64)
#define posix_lutime_USE_wlutime64
#elif defined(CONFIG_HAVE_wlutime) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lutime_USE_wlutime
#elif defined(CONFIG_HAVE_lutime)
#define posix_lutime_USE_lutime
#elif defined(CONFIG_HAVE_wlutime)
#define posix_lutime_USE_wlutime
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime64) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lutime_USE_wopen_AND_futime64
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime64)
#define posix_lutime_USE_open_AND_futime64
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime64)
#define posix_lutime_USE_wopen_AND_futime64
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lutime_USE_wopen_AND_futime
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime)
#define posix_lutime_USE_open_AND_futime
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime)
#define posix_lutime_USE_wopen_AND_futime
#elif defined(CONFIG_HAVE_wlutime32) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lutime_USE_wlutime32
#elif defined(CONFIG_HAVE_lutime32)
#define posix_lutime_USE_lutime32
#elif defined(CONFIG_HAVE_wlutime32)
#define posix_lutime_USE_wlutime32
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime32) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lutime_USE_wopen_AND_futime32
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime32)
#define posix_lutime_USE_open_AND_futime32
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_futime32)
#define posix_lutime_USE_wopen_AND_futime32
#else /* ... */
#define posix_lutime_USE_STUB
#endif /* !... */

/* Check if we should emulate
 * >> utime(path, atime, mtime, ctime, birthtime);
 * As:
 * >> lutime(try joinpath(headof(path), readlink(path)) catch (NoSymlink) path, atime, mtime, ctime, birthtime); */
#if ((defined(posix_utime_USE_STUB) ||                \
      defined(posix_utime_USE_open_AND_futime) ||     \
      defined(posix_utime_USE_open_AND_futime32) ||   \
      defined(posix_utime_USE_open_AND_futime64) ||   \
      defined(posix_utime_USE_wopen_AND_futime) ||    \
      defined(posix_utime_USE_wopen_AND_futime32) ||  \
      defined(posix_utime_USE_wopen_AND_futime64)) && \
     (!defined(posix_lutime_USE_STUB) && !defined(posix_readlink_USE_STUB)))
#undef posix_utime_USE_STUB
#undef posix_utime_USE_open_AND_futime
#undef posix_utime_USE_open_AND_futime32
#undef posix_utime_USE_open_AND_futime64
#undef posix_utime_USE_wopen_AND_futime
#undef posix_utime_USE_wopen_AND_futime32
#undef posix_utime_USE_wopen_AND_futime64
#define posix_utime_USE_posix_readlink__AND__posix_lutime
#endif /* ... */



/* Figure out how we want to implement `futime()' */
#undef posix_futime_USE_nt_SetFileTime
#undef posix_futime_USE_futimens64_3
#undef posix_futime_USE_futimens_3
#undef posix_futime_USE_futimens64
#undef posix_futime_USE_futimens
#undef posix_futime_USE_futimes64
#undef posix_futime_USE_futimes
#undef posix_futime_USE_futime64
#undef posix_futime_USE_futime32
#undef posix_futime_USE_futime
#undef posix_futime_USE_posix_lutime
#undef posix_futime_USE_posix_utime
#undef posix_futime_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_futime_USE_nt_SetFileTime
#elif defined(futimens64_3)
#define posix_futime_USE_futimens64_3
#elif defined(CONFIG_HAVE_futimens64)
#define posix_futime_USE_futimens64
#elif defined(CONFIG_HAVE_futimes64)
#define posix_futime_USE_futimes64
#elif defined(futimens_3)
#define posix_futime_USE_futimens_3
#elif defined(CONFIG_HAVE_futimens)
#define posix_futime_USE_futimens
#elif defined(CONFIG_HAVE_futimes)
#define posix_futime_USE_futimes
#elif defined(CONFIG_HAVE_futime64)
#define posix_futime_USE_futime64
#elif defined(CONFIG_HAVE_futime)
#define posix_futime_USE_futime
#elif defined(CONFIG_HAVE_futime32)
#define posix_futime_USE_futime32
#elif !defined(posix_lutime_USE_STUB)
#define posix_futime_USE_posix_lutime
#elif !defined(posix_utime_USE_STUB)
#define posix_futime_USE_posix_utime
#else /* ... */
#define posix_futime_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `utimeat()' */
#undef posix_utimeat_USE_utimensat
#undef posix_utimeat_USE_utimensat64
#undef posix_utimeat_USE_posix_utime
#undef posix_utimeat_USE_posix_lutime
#undef posix_utimeat_USE_posix_futime
#undef posix_utimeat_USE_STUB
#if !defined(posix_utime_USE_STUB) || !defined(posix_lutime_USE_STUB)
#ifdef CONFIG_HAVE_utimensat64
#define posix_utimeat_USE_utimensat64
#elif defined(CONFIG_HAVE_utimensat)
#define posix_utimeat_USE_utimensat
#endif /* ... */
#ifndef posix_utime_USE_STUB
#define posix_utimeat_USE_posix_utime
#endif /* !posix_utime_USE_STUB */
#ifndef posix_lutime_USE_STUB
#define posix_utimeat_USE_posix_lutime
#endif /* !posix_lutime_USE_STUB */
#ifndef posix_futime_USE_STUB
#define posix_utimeat_USE_posix_futime
#endif /* !posix_futime_USE_STUB */
#else /* !posix_utime_USE_STUB */
#define posix_utimeat_USE_STUB
#endif /* posix_utime_USE_STUB */




/* Implement the open+futime wrappers. */
#ifdef posix_utime_USE_wopen_AND_futime
#undef posix_utime_USE_wopen_AND_futime
#define posix_utime_USE_wutime
#undef wutime
#define wutime dee_wutime
PRIVATE int DCALL dee_wutime(wchar_t const *filename, struct utimbuf *file_times) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = futime(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_utime_USE_wopen_AND_futime */

#ifdef posix_utime_USE_open_AND_futime
#undef posix_utime_USE_open_AND_futime
#define posix_utime_USE_utime
#undef utime
#define utime dee_utime
PRIVATE int DCALL dee_utime(wchar_t const *filename, struct utimbuf *file_times) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = futime(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_utime_USE_open_AND_futime */

#ifdef posix_lutime_USE_wopen_AND_futime
#undef posix_lutime_USE_wopen_AND_futime
#define posix_lutime_USE_wlutime
#undef wlutime
#define wlutime dee_wlutime
PRIVATE int DCALL dee_wlutime(wchar_t const *filename, struct utimbuf *file_times) {
	int result;
	result = wopen(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = futime(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lutime_USE_wopen_AND_futime */

#ifdef posix_lutime_USE_open_AND_futime
#undef posix_lutime_USE_open_AND_futime
#define posix_lutime_USE_lutime
#undef lutime
#define lutime dee_lutime
PRIVATE int DCALL dee_lutime(wchar_t const *filename, struct utimbuf *file_times) {
	int result;
	result = open(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = futime(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lutime_USE_open_AND_futime */

#ifdef posix_utime_USE_wopen_AND_futime32
#undef posix_utime_USE_wopen_AND_futime32
#define posix_utime_USE_wutime32
#undef wutime32
#define wutime32 dee_wutime32
PRIVATE int DCALL dee_wutime32(wchar_t const *filename, struct utimbuf32 *file_times) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = futime32(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_utime_USE_wopen_AND_futime32 */

#ifdef posix_utime_USE_open_AND_futime32
#undef posix_utime_USE_open_AND_futime32
#define posix_utime_USE_utime32
#undef utime32
#define utime32 dee_utime32
PRIVATE int DCALL dee_utime32(wchar_t const *filename, struct utimbuf32 *file_times) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = futime32(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_utime_USE_open_AND_futime32 */

#ifdef posix_lutime_USE_wopen_AND_futime32
#undef posix_lutime_USE_wopen_AND_futime32
#define posix_lutime_USE_wlutime32
#undef wlutime32
#define wlutime32 dee_wlutime32
PRIVATE int DCALL dee_wlutime32(wchar_t const *filename, struct utimbuf32 *file_times) {
	int result;
	result = wopen(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = futime32(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lutime_USE_wopen_AND_futime32 */

#ifdef posix_lutime_USE_open_AND_futime32
#undef posix_lutime_USE_open_AND_futime32
#define posix_lutime_USE_lutime32
#undef lutime32
#define lutime32 dee_lutime32
PRIVATE int DCALL dee_lutime32(wchar_t const *filename, struct utimbuf32 *file_times) {
	int result;
	result = open(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = futime32(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lutime_USE_open_AND_futime32 */

#ifdef posix_utime_USE_wopen_AND_futime64
#undef posix_utime_USE_wopen_AND_futime64
#define posix_utime_USE_wutime64
#undef wutime64
#define wutime64 dee_wutime64
PRIVATE int DCALL dee_wutime64(wchar_t const *filename, struct utimbuf64 *file_times) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = futime64(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_utime_USE_wopen_AND_futime64 */

#ifdef posix_utime_USE_open_AND_futime64
#undef posix_utime_USE_open_AND_futime64
#define posix_utime_USE_utime64
#undef utime64
#define utime64 dee_utime64
PRIVATE int DCALL dee_utime64(wchar_t const *filename, struct utimbuf64 *file_times) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = futime64(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_utime_USE_open_AND_futime64 */

#ifdef posix_lutime_USE_wopen_AND_futime64
#undef posix_lutime_USE_wopen_AND_futime64
#define posix_lutime_USE_wlutime64
#undef wlutime64
#define wlutime64 dee_wlutime64
PRIVATE int DCALL dee_wlutime64(wchar_t const *filename, struct utimbuf64 *file_times) {
	int result;
	result = wopen(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = futime64(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lutime_USE_wopen_AND_futime64 */

#ifdef posix_lutime_USE_open_AND_futime64
#undef posix_lutime_USE_open_AND_futime64
#define posix_lutime_USE_lutime64
#undef lutime64
#define lutime64 dee_lutime64
PRIVATE int DCALL dee_lutime64(wchar_t const *filename, struct utimbuf64 *file_times) {
	int result;
	result = open(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = futime64(result, file_times);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lutime_USE_open_AND_futime64 */

#ifdef posix_utime_USE_posix_readlink__AND__posix_lutime
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lutime_f_impl(DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
#endif /* posix_utime_USE_posix_readlink__AND__posix_lutime */


/* Define helper macros for `posix_utime()' */
#undef posix_utime_USED_struct_utimbuf
#undef posix_utime_USED_struct_utimbuf_parse
#if defined(posix_utime_USE_utime) || defined(posix_utime_USE_wutime)
#define posix_utime_USED_struct_utimbuf       struct utimbuf
#define posix_utime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf
#define NEED_posix_utime_unix_parse_utimbuf
#elif defined(posix_utime_USE_utime32) || defined(posix_utime_USE_wutime32)
#define posix_utime_USED_struct_utimbuf       struct utimbuf32
#define posix_utime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf32
#define NEED_posix_utime_unix_parse_utimbuf32
#elif defined(posix_utime_USE_utime64) || defined(posix_utime_USE_wutime64)
#define posix_utime_USED_struct_utimbuf       struct utimbuf64
#define posix_utime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf64
#define NEED_posix_utime_unix_parse_utimbuf64
#endif /* ... */
#undef posix_utime_USED_charT
#undef posix_utime_USED_DeeString_AsCharT
#if defined(posix_utime_USE_wutime) || defined(posix_utime_USE_wutime32) || defined(posix_utime_USE_wutime64)
#define posix_utime_USED_charT             Dee_wchar_t
#define posix_utime_USED_DeeString_AsCharT DeeString_AsWide
#else /* ... */
#define posix_utime_USED_charT             char
#define posix_utime_USED_DeeString_AsCharT DeeString_AsUtf8
#endif /* !... */
#undef posix_utime_USED_utimens
#undef posix_utime_USED_struct_timespec
#undef posix_utime_USED_struct_timespec_COUNT
#undef posix_utime_USED_struct_timespec_parse
#undef posix_utime_USED_struct_timespec_IS_timeval
#ifdef posix_utime_USE_utimens64_3
#define posix_utime_USED_utimens(p, v)         utimens64_3((char *)(p), v)
#define posix_utime_USED_struct_timespec       struct timespec64
#define posix_utime_USED_struct_timespec_COUNT 3
#define posix_utime_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_3
#define NEED_posix_utime_unix_parse_timespec64_3
#elif defined(posix_utime_USE_utimens_3)
#define posix_utime_USED_utimens(p, v)         utimens_3((char *)(p), v)
#define posix_utime_USED_struct_timespec       struct timespec
#define posix_utime_USED_struct_timespec_COUNT 3
#define posix_utime_USED_struct_timespec_parse posix_utime_unix_parse_timespec_3
#define NEED_posix_utime_unix_parse_timespec_3
#elif defined(posix_utime_USE_utimens64)
#define posix_utime_USED_utimens(p, v)         utimens64((char *)(p), v)
#define posix_utime_USED_struct_timespec       struct timespec64
#define posix_utime_USED_struct_timespec_COUNT 2
#define posix_utime_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_2
#define NEED_posix_utime_unix_parse_timespec64_2
#elif defined(posix_utime_USE_utimens)
#define posix_utime_USED_utimens(p, v)         utimens((char *)(p), v)
#define posix_utime_USED_struct_timespec       struct timespec
#define posix_utime_USED_struct_timespec_COUNT 2
#define posix_utime_USED_struct_timespec_parse posix_utime_unix_parse_timespec_2
#define NEED_posix_utime_unix_parse_timespec_2
#elif defined(posix_utime_USE_utimes64)
#define posix_utime_USED_utimens(p, v)         utimes64((char *)(p), v)
#define posix_utime_USED_struct_timespec       struct timeval64
#define posix_utime_USED_struct_timespec_COUNT 2
#define posix_utime_USED_struct_timespec_parse posix_utime_unix_parse_timeval64
#define posix_utime_USED_struct_timespec_IS_timeval
#define NEED_posix_utime_unix_parse_timeval64
#elif defined(posix_utime_USE_utimes)
#define posix_utime_USED_utimens(p, v)         utimes((char *)(p), v)
#define posix_utime_USED_struct_timespec       struct timeval
#define posix_utime_USED_struct_timespec_COUNT 2
#define posix_utime_USED_struct_timespec_parse posix_utime_unix_parse_timeval
#define posix_utime_USED_struct_timespec_IS_timeval
#define NEED_posix_utime_unix_parse_timeval
#endif /* ... */


/* Define helper macros for `posix_lutime()' */
#undef posix_lutime_USED_struct_utimbuf
#undef posix_lutime_USED_struct_utimbuf_parse
#if defined(posix_lutime_USE_lutime) || defined(posix_lutime_USE_wlutime)
#define posix_lutime_USED_struct_utimbuf       struct utimbuf
#define posix_lutime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf
#define NEED_posix_utime_unix_parse_utimbuf
#elif defined(posix_lutime_USE_lutime32) || defined(posix_lutime_USE_wlutime32)
#define posix_lutime_USED_struct_utimbuf       struct utimbuf32
#define posix_lutime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf32
#define NEED_posix_utime_unix_parse_utimbuf32
#elif defined(posix_lutime_USE_lutime64) || defined(posix_lutime_USE_wlutime64)
#define posix_lutime_USED_struct_utimbuf       struct utimbuf64
#define posix_lutime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf64
#define NEED_posix_utime_unix_parse_utimbuf64
#endif /* ... */
#undef posix_lutime_USED_charT
#undef posix_lutime_USED_DeeString_AsCharT
#if defined(posix_lutime_USE_wlutime) || defined(posix_lutime_USE_wlutime32) || defined(posix_lutime_USE_wlutime64)
#define posix_lutime_USED_charT             Dee_wchar_t
#define posix_lutime_USED_DeeString_AsCharT DeeString_AsWide
#else /* ... */
#define posix_lutime_USED_charT             char
#define posix_lutime_USED_DeeString_AsCharT DeeString_AsUtf8
#endif /* !... */
#undef posix_lutime_USED_lutimens
#undef posix_lutime_USED_struct_timespec
#undef posix_lutime_USED_struct_timespec_COUNT
#undef posix_lutime_USED_struct_timespec_parse
#undef posix_lutime_USED_struct_timespec_IS_timeval
#ifdef posix_lutime_USE_lutimens64_3
#define posix_lutime_USED_lutimens(p, v)        lutimens64_3((char *)(p), v)
#define posix_lutime_USED_struct_timespec       struct timespec64
#define posix_lutime_USED_struct_timespec_COUNT 3
#define posix_lutime_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_3
#define NEED_posix_utime_unix_parse_timespec64_3
#elif defined(posix_lutime_USE_lutimens_3)
#define posix_lutime_USED_lutimens(p, v)        lutimens_3((char *)(p), v)
#define posix_lutime_USED_struct_timespec       struct timespec
#define posix_lutime_USED_struct_timespec_COUNT 3
#define posix_lutime_USED_struct_timespec_parse posix_utime_unix_parse_timespec_3
#define NEED_posix_utime_unix_parse_timespec_3
#elif defined(posix_lutime_USE_lutimens64)
#define posix_lutime_USED_lutimens(p, v)        lutimens64((char *)(p), v)
#define posix_lutime_USED_struct_timespec       struct timespec64
#define posix_lutime_USED_struct_timespec_COUNT 2
#define posix_lutime_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_2
#define NEED_posix_utime_unix_parse_timespec64_2
#elif defined(posix_lutime_USE_lutimens)
#define posix_lutime_USED_lutimens(p, v)        lutimens((char *)(p), v)
#define posix_lutime_USED_struct_timespec       struct timespec
#define posix_lutime_USED_struct_timespec_COUNT 2
#define posix_lutime_USED_struct_timespec_parse posix_utime_unix_parse_timespec_2
#define NEED_posix_utime_unix_parse_timespec_2
#elif defined(posix_lutime_USE_lutimes64)
#define posix_lutime_USED_lutimens(p, v)        lutimes64((char *)(p), v)
#define posix_lutime_USED_struct_timespec       struct timeval64
#define posix_lutime_USED_struct_timespec_COUNT 2
#define posix_lutime_USED_struct_timespec_parse posix_utime_unix_parse_timeval64
#define posix_lutime_USED_struct_timespec_IS_timeval
#define NEED_posix_utime_unix_parse_timeval64
#elif defined(posix_lutime_USE_lutimes)
#define posix_lutime_USED_lutimens(p, v)        lutimes((char *)(p), v)
#define posix_lutime_USED_struct_timespec       struct timeval
#define posix_lutime_USED_struct_timespec_COUNT 2
#define posix_lutime_USED_struct_timespec_parse posix_utime_unix_parse_timeval
#define posix_lutime_USED_struct_timespec_IS_timeval
#define NEED_posix_utime_unix_parse_timeval
#endif /* ... */


/* Define helper macros for `posix_futime()' */
#undef posix_futime_USED_struct_utimbuf
#undef posix_futime_USED_struct_utimbuf_parse
#if defined(posix_futime_USE_futime) || defined(posix_futime_USE_wfutime)
#define posix_futime_USED_struct_utimbuf       struct utimbuf
#define posix_futime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf
#define NEED_posix_utime_unix_parse_utimbuf
#elif defined(posix_futime_USE_futime32) || defined(posix_futime_USE_wfutime32)
#define posix_futime_USED_struct_utimbuf       struct utimbuf32
#define posix_futime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf32
#define NEED_posix_utime_unix_parse_utimbuf32
#elif defined(posix_futime_USE_futime64) || defined(posix_futime_USE_wfutime64)
#define posix_futime_USED_struct_utimbuf       struct utimbuf64
#define posix_futime_USED_struct_utimbuf_parse posix_utime_unix_parse_utimbuf64
#define NEED_posix_utime_unix_parse_utimbuf64
#endif /* ... */
#undef posix_futime_USED_futimens
#undef posix_futime_USED_struct_timespec
#undef posix_futime_USED_struct_timespec_COUNT
#undef posix_futime_USED_struct_timespec_parse
#undef posix_futime_USED_struct_timespec_IS_timeval
#ifdef posix_futime_USE_futimens64_3
#define posix_futime_USED_futimens(f, v)        futimens64_3(f, v)
#define posix_futime_USED_struct_timespec       struct timespec64
#define posix_futime_USED_struct_timespec_COUNT 3
#define posix_futime_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_3
#define NEED_posix_utime_unix_parse_timespec64_3
#elif defined(posix_futime_USE_futimens_3)
#define posix_futime_USED_futimens(f, v)        futimens_3(f, v)
#define posix_futime_USED_struct_timespec       struct timespec
#define posix_futime_USED_struct_timespec_COUNT 3
#define posix_futime_USED_struct_timespec_parse posix_utime_unix_parse_timespec_3
#define NEED_posix_utime_unix_parse_timespec_3
#elif defined(posix_futime_USE_futimens64)
#define posix_futime_USED_futimens(f, v)        futimens64(f, v)
#define posix_futime_USED_struct_timespec       struct timespec64
#define posix_futime_USED_struct_timespec_COUNT 2
#define posix_futime_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_2
#define NEED_posix_utime_unix_parse_timespec64_2
#elif defined(posix_futime_USE_futimens)
#define posix_futime_USED_futimens(f, v)        futimens(f, v)
#define posix_futime_USED_struct_timespec       struct timespec
#define posix_futime_USED_struct_timespec_COUNT 2
#define posix_futime_USED_struct_timespec_parse posix_utime_unix_parse_timespec_2
#define NEED_posix_utime_unix_parse_timespec_2
#elif defined(posix_futime_USE_futimes64)
#define posix_futime_USED_futimens(f, v)        futimes64(f, v)
#define posix_futime_USED_struct_timespec       struct timeval64
#define posix_futime_USED_struct_timespec_COUNT 2
#define posix_futime_USED_struct_timespec_parse posix_utime_unix_parse_timeval64
#define posix_futime_USED_struct_timespec_IS_timeval
#define NEED_posix_utime_unix_parse_timeval64
#elif defined(posix_futime_USE_futimes)
#define posix_futime_USED_futimens(f, v)        futimes(f, v)
#define posix_futime_USED_struct_timespec       struct timeval
#define posix_futime_USED_struct_timespec_COUNT 2
#define posix_futime_USED_struct_timespec_parse posix_utime_unix_parse_timeval
#define posix_futime_USED_struct_timespec_IS_timeval
#define NEED_posix_utime_unix_parse_timeval
#endif /* ... */




/* Define helper macros for `posix_utimeat()' */
#undef posix_utimeat_USED_utimensat
#undef posix_utimeat_USED_struct_timespec
#undef posix_utimeat_USED_struct_timespec_COUNT
#undef posix_utimeat_USED_struct_timespec_parse
#ifdef posix_utimeat_USE_utimensat64
#define posix_utimeat_USED_utimensat(d, p, v, f) utimensat64(d, (char *)(p), v, f)
#define posix_utimeat_USED_struct_timespec       struct timespec64
#define posix_utimeat_USED_struct_timespec_COUNT POSIX_UTIME_TIMESPEC_COUNT
#if posix_utimeat_USED_struct_timespec_COUNT == 2
#define posix_utimeat_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_2
#define NEED_posix_utime_unix_parse_timespec64_2
#else /* posix_utimeat_USED_struct_timespec_COUNT == 2 */
#define posix_utimeat_USED_struct_timespec_parse posix_utime_unix_parse_timespec64_3
#define NEED_posix_utime_unix_parse_timespec64_3
#endif /* posix_utimeat_USED_struct_timespec_COUNT != 2 */
#elif defined(posix_utimeat_USE_utimensat)
#define posix_utimeat_USED_utimensat(d, p, v, f) utimensat(d, (char *)(p), v, f)
#define posix_utimeat_USED_struct_timespec       struct timespec
#define posix_utimeat_USED_struct_timespec_COUNT POSIX_UTIME_TIMESPEC_COUNT
#if posix_utimeat_USED_struct_timespec_COUNT == 2
#define posix_utimeat_USED_struct_timespec_parse posix_utime_unix_parse_timespec_2
#define NEED_posix_utime_unix_parse_timespec_2
#else /* posix_utimeat_USED_struct_timespec_COUNT == 2 */
#define posix_utimeat_USED_struct_timespec_parse posix_utime_unix_parse_timespec_3
#define NEED_posix_utime_unix_parse_timespec_3
#endif /* posix_utimeat_USED_struct_timespec_COUNT != 2 */
#endif /* ... */





#if (defined(posix_utime_USE_nt_SetFileTime) || \
     defined(posix_lutime_USE_nt_SetFileTime))
PRIVATE WUNUSED DREF DeeObject *DCALL
posix_Xutime_impl_nt_SetFileTime(DeeObject *path,
                                 DeeObject *atime, DeeObject *mtime,
                                 DeeObject *ctime, DeeObject *birthtime,
                                 DeeNT_DWORD dwFlagsAndAttributes) {
	HANDLE hFile;
	int error;
	if unlikely(!DeeNone_Check(ctime)) {
#ifdef stat_get_ctime_IS_stat_get_mtime
		if (DeeNone_Check(mtime)) {
			mtime = ctime;
			/*ctime = Dee_None;*/
		} else
#endif /* stat_get_ctime_IS_stat_get_mtime */
		{
			goto err_canot_set_ctime;
		}
	}
	hFile = DeeNTSystem_CreateFileNoATime(path, FILE_WRITE_ATTRIBUTES,
	                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                                      NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
	if unlikely(!hFile)
		goto err;
	if unlikely(hFile == INVALID_HANDLE_VALUE) {
		error = 1;
	} else {
		error = nt_SetFileTime(hFile, atime, mtime, birthtime);
#define NEED_nt_SetFileTime
		(void)CloseHandle(hFile);
		if unlikely(error < 0)
			goto err;
	}
	if unlikely(error > 0) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		err_nt_utime(dwError, path, atime, mtime, birthtime);
#define NEED_err_nt_utime
		goto err;
	}
	return_none;
err_canot_set_ctime:
	err_utime_cannot_set_ctime(path, ctime);
#define NEED_err_utime_cannot_set_ctime
err:
	return NULL;
}
#endif /* ... */


/*[[[deemon import("rt.gen.dexutils").gw("utime", "path:?Dstring,atime:?Etime:Time=Dee_None,mtime:?Etime:Time=Dee_None,ctime:?Etime:Time=Dee_None,birthtime:?Etime:Time=Dee_None", libname: "posix"); ]]]*/
#define POSIX_UTIME_DEF          DEX_MEMBER_F("utime", &posix_utime, DEXSYM_READONLY, "(path:?Dstring,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N)"),
#define POSIX_UTIME_DEF_DOC(doc) DEX_MEMBER_F("utime", &posix_utime, DEXSYM_READONLY, "(path:?Dstring,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL posix_utime_f_impl(DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
#ifndef DEFINED_kwlist__path_atime_mtime_ctime_birthtime
#define DEFINED_kwlist__path_atime_mtime_ctime_birthtime
PRIVATE DEFINE_KWLIST(kwlist__path_atime_mtime_ctime_birthtime, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atime", 0xdc4358af, 0x2d543aa498d68399), KEX("mtime", 0xd2dc3dac, 0x25287308d8bca8fd), KEX("ctime", 0xd3c27b8d, 0x4582041ef6ec6f80), KEX("birthtime", 0x85a70ed7, 0x1f72b099e114440a), KEND });
#endif /* !DEFINED_kwlist__path_atime_mtime_ctime_birthtime */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_utime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
		DeeObject *atime;
		DeeObject *mtime;
		DeeObject *ctime;
		DeeObject *birthtime;
	} args;
	args.atime = Dee_None;
	args.mtime = Dee_None;
	args.ctime = Dee_None;
	args.birthtime = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_atime_mtime_ctime_birthtime, "o|oooo:utime", &args))
		goto err;
	return posix_utime_f_impl(args.path, args.atime, args.mtime, args.ctime, args.birthtime);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_utime, &posix_utime_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL posix_utime_f_impl(DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime)
/*[[[end]]]*/
{
#ifdef posix_utime_USE_nt_SetFileTime
	return posix_Xutime_impl_nt_SetFileTime(path, atime, mtime, ctime, birthtime,
	                                        FILE_ATTRIBUTE_NORMAL |
	                                        FILE_FLAG_BACKUP_SEMANTICS);
#endif /* posix_utime_USE_nt_SetFileTime */

#ifdef posix_utime_USE_posix_readlink__AND__posix_lutime
	DREF DeeObject *link_text;
	/* Try to readlink() the given `path' to see if it's a symbolic link. */
	link_text = posix_readlink_f_impl(path);
	if (link_text) {
		DREF DeeObject *full_path, *result;
		full_path = Dee_AsObject(posix_path_walklink_f((DeeStringObject *)link_text,
		                                               (DeeStringObject *)path));
		Dee_Decref(link_text);
		if unlikely(!full_path)
			goto err;
		result = posix_lutime_f_impl(full_path, atime, mtime, ctime, birthtime);
		Dee_Decref(full_path);
		return result;
	}
	if (!DeeError_Catch(&DeeError_NoSymlink))
		goto err;

	/* Not a symbolic link -> lutime() will work to do what we want! */
	return posix_lutime_f_impl(path, atime, mtime, ctime, birthtime);
err:
	return NULL;
#endif /* posix_utime_USE_posix_readlink__AND__posix_lutime */

#ifdef posix_utime_USED_utimens
	posix_utime_USED_struct_timespec tsv[posix_utime_USED_struct_timespec_COUNT];
	int error;
	posix_utime_USED_charT const *os_path;

	/* Check that we can satisfy all requested time channels. */
#ifdef stat_get_ctime_IS_stat_get_mtime
	if (!DeeNone_Check(ctime) && DeeNone_Check(mtime)) {
		mtime = ctime;
		ctime = Dee_None;
	}
#endif /* stat_get_ctime_IS_stat_get_mtime */
#if posix_utime_USED_struct_timespec_COUNT == 2
	if (!DeeNone_Check(ctime) || !DeeNone_Check(birthtime)) {
		err_utime_cannot_set_ctime_or_btime(path, ctime, birthtime);
#define NEED_err_utime_cannot_set_ctime_or_btime
		goto err;
	}
#else /* posix_utime_USED_struct_timespec_COUNT == 2 */
	if (!DeeNone_Check(ctime)) {
		err_utime_cannot_set_ctime(path, ctime);
#define NEED_err_utime_cannot_set_ctime
		goto err;
	}
#endif /* posix_utime_USED_struct_timespec_COUNT != 2 */

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	os_path = posix_utime_USED_DeeString_AsCharT(path);
	if unlikely(!os_path)
		goto err;
#ifdef posix_utime_USED_struct_timespec_IS_timeval
	if (posix_utime_USED_struct_timespec_parse(tsv, atime, mtime, path, 0))
#elif posix_utime_USED_struct_timespec_COUNT == 2
	if (posix_utime_USED_struct_timespec_parse(tsv, atime, mtime))
#else /* posix_utime_USED_struct_timespec_COUNT == 2 */
	if (posix_utime_USED_struct_timespec_parse(tsv, atime, mtime, birthtime))
#endif /* posix_utime_USED_struct_timespec_COUNT != 2 */
	{
		goto err;
	}

again:
	DBG_ALIGNMENT_DISABLE();
	error = posix_utime_USED_utimens(os_path, tsv);
	if unlikely(error != 0) {
		DBG_ALIGNMENT_DISABLE();
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeUnixSystem_HandleGenericError(error, err, again);
		err_unix_utime(error, path, atime, mtime, ctime, birthtime);
#define NEED_err_unix_utime
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_utime_USED_utimens */

#ifdef posix_utime_USED_struct_utimbuf
	int error;
	posix_utime_USED_struct_utimbuf file_times;
	posix_utime_USED_charT const *os_path;

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	os_path = posix_utime_USED_DeeString_AsCharT(path);
	if unlikely(!os_path)
		goto err;

	/* Load the utime-file_times argument. */
	if unlikely(posix_utime_USED_struct_utimbuf_parse(&file_times,
	                                                  atime, mtime,
	                                                  ctime, birthtime,
	                                                  path, 0))
		goto err;

again:
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_utime_USE_wutime
	error = wutime((wchar_t *)os_path, &file_times);
#endif /* posix_utime_USE_wutime */
#ifdef posix_utime_USE_wutime32
	error = wutime32((wchar_t *)os_path, &file_times);
#endif /* posix_utime_USE_wutime32 */
#ifdef posix_utime_USE_wutime64
	error = wutime64((wchar_t *)os_path, &file_times);
#endif /* posix_utime_USE_wutime64 */
#ifdef posix_utime_USE_utime
	error = utime((char *)os_path, &file_times);
#endif /* posix_utime_USE_utime */
#ifdef posix_utime_USE_utime32
	error = utime32((char *)os_path, &file_times);
#endif /* posix_utime_USE_utime32 */
#ifdef posix_utime_USE_utime64
	error = utime64((char *)os_path, &file_times);
#endif /* posix_utime_USE_utime64 */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error != 0) {
		error = DeeSystem_GetErrno();
		DeeUnixSystem_HandleGenericError(error, err, again);
		err_unix_utime(error, path, atime, mtime, ctime, birthtime);
#define NEED_err_unix_utime
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_utime_USED_struct_utimbuf */

#ifdef posix_utime_USE_STUB
	(void)path;
	(void)atime;
	(void)mtime;
	(void)ctime;
	(void)birthtime;
	posix_err_unsupported("utime");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_utime_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("lutime", "path:?Dstring,atime:?Etime:Time=Dee_None,mtime:?Etime:Time=Dee_None,ctime:?Etime:Time=Dee_None,birthtime:?Etime:Time=Dee_None", libname: "posix"); ]]]*/
#define POSIX_LUTIME_DEF          DEX_MEMBER_F("lutime", &posix_lutime, DEXSYM_READONLY, "(path:?Dstring,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N)"),
#define POSIX_LUTIME_DEF_DOC(doc) DEX_MEMBER_F("lutime", &posix_lutime, DEXSYM_READONLY, "(path:?Dstring,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL posix_lutime_f_impl(DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
#ifndef DEFINED_kwlist__path_atime_mtime_ctime_birthtime
#define DEFINED_kwlist__path_atime_mtime_ctime_birthtime
PRIVATE DEFINE_KWLIST(kwlist__path_atime_mtime_ctime_birthtime, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atime", 0xdc4358af, 0x2d543aa498d68399), KEX("mtime", 0xd2dc3dac, 0x25287308d8bca8fd), KEX("ctime", 0xd3c27b8d, 0x4582041ef6ec6f80), KEX("birthtime", 0x85a70ed7, 0x1f72b099e114440a), KEND });
#endif /* !DEFINED_kwlist__path_atime_mtime_ctime_birthtime */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lutime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
		DeeObject *atime;
		DeeObject *mtime;
		DeeObject *ctime;
		DeeObject *birthtime;
	} args;
	args.atime = Dee_None;
	args.mtime = Dee_None;
	args.ctime = Dee_None;
	args.birthtime = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_atime_mtime_ctime_birthtime, "o|oooo:lutime", &args))
		goto err;
	return posix_lutime_f_impl(args.path, args.atime, args.mtime, args.ctime, args.birthtime);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_lutime, &posix_lutime_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL posix_lutime_f_impl(DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime)
/*[[[end]]]*/
{
#ifdef posix_lutime_USE_nt_SetFileTime
	return posix_Xutime_impl_nt_SetFileTime(path, atime, mtime, ctime, birthtime,
	                                        FILE_ATTRIBUTE_NORMAL |
	                                        FILE_FLAG_BACKUP_SEMANTICS |
	                                        FILE_FLAG_OPEN_REPARSE_POINT);
#endif /* posix_lutime_USE_nt_SetFileTime */

#ifdef posix_lutime_USED_lutimens
	posix_lutime_USED_struct_timespec tsv[posix_lutime_USED_struct_timespec_COUNT];
	int error;
	posix_lutime_USED_charT const *os_path;

	/* Check that we can satisfy all requested time channels. */
#ifdef stat_get_ctime_IS_stat_get_mtime
	if (!DeeNone_Check(ctime) && DeeNone_Check(mtime)) {
		mtime = ctime;
		ctime = Dee_None;
	}
#endif /* stat_get_ctime_IS_stat_get_mtime */
#if posix_lutime_USED_struct_timespec_COUNT == 2
	if (!DeeNone_Check(ctime) || !DeeNone_Check(birthtime)) {
		err_utime_cannot_set_ctime_or_btime(path, ctime, birthtime);
#define NEED_err_utime_cannot_set_ctime_or_btime
		goto err;
	}
#else /* posix_lutime_USED_struct_timespec_COUNT == 2 */
	if (!DeeNone_Check(ctime)) {
		err_utime_cannot_set_ctime(path, ctime);
#define NEED_err_utime_cannot_set_ctime
		goto err;
	}
#endif /* posix_lutime_USED_struct_timespec_COUNT != 2 */

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	os_path = posix_lutime_USED_DeeString_AsCharT(path);
	if unlikely(!os_path)
		goto err;
#ifdef posix_lutime_USED_struct_timespec_IS_timeval
	if (posix_lutime_USED_struct_timespec_parse(tsv, atime, mtime, path, Dee_STAT_F_LSTAT))
#elif posix_lutime_USED_struct_timespec_COUNT == 2
	if (posix_lutime_USED_struct_timespec_parse(tsv, atime, mtime))
#else /* posix_lutime_USED_struct_timespec_COUNT == 2 */
	if (posix_lutime_USED_struct_timespec_parse(tsv, atime, mtime, birthtime))
#endif /* posix_lutime_USED_struct_timespec_COUNT != 2 */
	{
		goto err;
	}

again:
	DBG_ALIGNMENT_DISABLE();
	error = posix_lutime_USED_lutimens(os_path, tsv);
	if unlikely(error != 0) {
		DBG_ALIGNMENT_DISABLE();
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeUnixSystem_HandleGenericError(error, err, again);
		err_unix_lutime(error, path, atime, mtime, ctime, birthtime);
#define NEED_err_unix_lutime
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_lutime_USED_lutimens */

#ifdef posix_lutime_USED_struct_utimbuf
	int error;
	posix_lutime_USED_struct_utimbuf file_times;
	posix_lutime_USED_charT const *os_path;

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	os_path = posix_lutime_USED_DeeString_AsCharT(path);
	if unlikely(!os_path)
		goto err;

	/* Load the lutime-file_times argument. */
	if unlikely(posix_lutime_USED_struct_utimbuf_parse(&file_times,
	                                                   atime, mtime,
	                                                   ctime, birthtime,
	                                                   path, Dee_STAT_F_LSTAT))
		goto err;

again:
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_lutime_USE_wlutime
	error = wlutime((wchar_t *)os_path, &file_times);
#endif /* posix_lutime_USE_wlutime */
#ifdef posix_lutime_USE_wlutime32
	error = wlutime32((wchar_t *)os_path, &file_times);
#endif /* posix_lutime_USE_wlutime32 */
#ifdef posix_lutime_USE_wlutime64
	error = wlutime64((wchar_t *)os_path, &file_times);
#endif /* posix_lutime_USE_wlutime64 */
#ifdef posix_lutime_USE_lutime
	error = lutime((char *)os_path, &file_times);
#endif /* posix_lutime_USE_lutime */
#ifdef posix_lutime_USE_lutime32
	error = lutime32((char *)os_path, &file_times);
#endif /* posix_lutime_USE_lutime32 */
#ifdef posix_lutime_USE_lutime64
	error = lutime64((char *)os_path, &file_times);
#endif /* posix_lutime_USE_lutime64 */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error != 0) {
		error = DeeSystem_GetErrno();
		DeeUnixSystem_HandleGenericError(error, err, again);
		err_unix_lutime(error, path, atime, mtime, ctime, birthtime);
#define NEED_err_unix_lutime
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_lutime_USED_struct_utimbuf */

#ifdef posix_lutime_USE_STUB
	(void)path;
	(void)atime;
	(void)mtime;
	(void)ctime;
	(void)birthtime;
	posix_err_unsupported("lutime");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_lutime_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("futime", "fd:?X2?Dint?DFile,atime:?Etime:Time=Dee_None,mtime:?Etime:Time=Dee_None,ctime:?Etime:Time=Dee_None,birthtime:?Etime:Time=Dee_None", libname: "posix"); ]]]*/
#define POSIX_FUTIME_DEF          DEX_MEMBER_F("futime", &posix_futime, DEXSYM_READONLY, "(fd:?X2?Dint?DFile,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N)"),
#define POSIX_FUTIME_DEF_DOC(doc) DEX_MEMBER_F("futime", &posix_futime, DEXSYM_READONLY, "(fd:?X2?Dint?DFile,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL posix_futime_f_impl(DeeObject *fd, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
#ifndef DEFINED_kwlist__fd_atime_mtime_ctime_birthtime
#define DEFINED_kwlist__fd_atime_mtime_ctime_birthtime
PRIVATE DEFINE_KWLIST(kwlist__fd_atime_mtime_ctime_birthtime, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEX("atime", 0xdc4358af, 0x2d543aa498d68399), KEX("mtime", 0xd2dc3dac, 0x25287308d8bca8fd), KEX("ctime", 0xd3c27b8d, 0x4582041ef6ec6f80), KEX("birthtime", 0x85a70ed7, 0x1f72b099e114440a), KEND });
#endif /* !DEFINED_kwlist__fd_atime_mtime_ctime_birthtime */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_futime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *fd;
		DeeObject *atime;
		DeeObject *mtime;
		DeeObject *ctime;
		DeeObject *birthtime;
	} args;
	args.atime = Dee_None;
	args.mtime = Dee_None;
	args.ctime = Dee_None;
	args.birthtime = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fd_atime_mtime_ctime_birthtime, "o|oooo:futime", &args))
		goto err;
	return posix_futime_f_impl(args.fd, args.atime, args.mtime, args.ctime, args.birthtime);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_futime, &posix_futime_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL posix_futime_f_impl(DeeObject *fd, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime)
/*[[[end]]]*/
{
#ifdef posix_futime_USE_nt_SetFileTime
	HANDLE hFile;
	int error;
	if unlikely(!DeeNone_Check(ctime)) {
#ifdef stat_get_ctime_IS_stat_get_mtime
		if (DeeNone_Check(mtime)) {
			mtime = ctime;
			/*ctime = Dee_None;*/
		} else
#endif /* stat_get_ctime_IS_stat_get_mtime */
		{
			goto err_canot_set_ctime;
		}
	}
	hFile = DeeNTSystem_GetHandle(fd);
	if unlikely(hFile == INVALID_HANDLE_VALUE)
		goto err;
	error = nt_SetFileTime(hFile, atime, mtime, birthtime);
#define NEED_nt_SetFileTime
	if unlikely(error != 0) {
		DWORD dwError;
		if unlikely(error < 0)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		err_nt_futime(dwError, fd, atime, mtime, birthtime);
#define NEED_err_nt_futime
		goto err;
	}
	return_none;
err_canot_set_ctime:
	err_utime_cannot_set_ctime(fd, ctime);
#define NEED_err_utime_cannot_set_ctime
err:
	return NULL;
#endif /* posix_futime_USE_nt_SetFileTime */

#ifdef posix_futime_USED_futimens
	posix_futime_USED_struct_timespec tsv[posix_futime_USED_struct_timespec_COUNT];
	int error;
	int os_fd;

	/* Check that we can satisfy all requested time channels. */
#ifdef stat_get_ctime_IS_stat_get_mtime
	if (!DeeNone_Check(ctime) && DeeNone_Check(mtime)) {
		mtime = ctime;
		ctime = Dee_None;
	}
#endif /* stat_get_ctime_IS_stat_get_mtime */
#if posix_futime_USED_struct_timespec_COUNT == 2
	if (!DeeNone_Check(ctime) || !DeeNone_Check(birthtime)) {
		err_utime_cannot_set_ctime_or_btime(fd, ctime, birthtime);
#define NEED_err_utime_cannot_set_ctime_or_btime
		goto err;
	}
#else /* posix_futime_USED_struct_timespec_COUNT == 2 */
	if (!DeeNone_Check(ctime)) {
		err_utime_cannot_set_ctime(fd, ctime);
#define NEED_err_utime_cannot_set_ctime
		goto err;
	}
#endif /* posix_futime_USED_struct_timespec_COUNT != 2 */

	/* Decode arguments. */
	os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		goto err;
#ifdef posix_futime_USED_struct_timespec_IS_timeval
	if (posix_futime_USED_struct_timespec_parse(tsv, atime, mtime, fd, 0))
#elif posix_futime_USED_struct_timespec_COUNT == 2
	if (posix_futime_USED_struct_timespec_parse(tsv, atime, mtime))
#else /* posix_futime_USED_struct_timespec_COUNT == 2 */
	if (posix_futime_USED_struct_timespec_parse(tsv, atime, mtime, birthtime))
#endif /* posix_futime_USED_struct_timespec_COUNT != 2 */
	{
		goto err;
	}

again:
	DBG_ALIGNMENT_DISABLE();
	error = posix_futime_USED_futimens(os_fd, tsv);
	if unlikely(error != 0) {
		DBG_ALIGNMENT_DISABLE();
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeUnixSystem_HandleGenericError(error, err, again);
		err_unix_futime(error, fd, atime, mtime, ctime, birthtime);
#define NEED_err_unix_futime
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_futime_USED_futimens */

#ifdef posix_futime_USED_struct_utimbuf
	int error;
	int os_fd;
	posix_futime_USED_struct_utimbuf file_times;

	/* Decode arguments. */
	os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		goto err;

	/* Load the futime-file_times argument. */
	if unlikely(posix_futime_USED_struct_utimbuf_parse(&file_times,
	                                                   atime, mtime,
	                                                   ctime, birthtime,
	                                                   fd, 0))
		goto err;

again:
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_futime_USE_futime
	error = futime(os_fd, &file_times);
#endif /* posix_futime_USE_futime */
#ifdef posix_futime_USE_futime32
	error = futime32(os_fd, &file_times);
#endif /* posix_futime_USE_futime32 */
#ifdef posix_futime_USE_futime64
	error = futime64(os_fd, &file_times);
#endif /* posix_futime_USE_futime64 */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error != 0) {
		DBG_ALIGNMENT_DISABLE();
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeUnixSystem_HandleGenericError(error, err, again);
		err_unix_futime(error, fd, atime, mtime, ctime, birthtime);
#define NEED_err_unix_futime
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_futime_USED_struct_utimbuf */

#if defined(posix_futime_USE_posix_lutime) || defined(posix_futime_USE_posix_utime)
	DREF DeeObject *result;
	DREF DeeObject *abspath;
	abspath = posix_fd_makepath(fd);
#define NEED_posix_fd_makepath
	if unlikely(!abspath)
		goto err;
#ifdef posix_futime_USE_posix_lutime
	result = posix_lutime_f_impl(abspath, atime, mtime, ctime, birthtime);
#else /* posix_futime_USE_posix_lutime */
	result = posix_utime_f_impl(abspath, atime, mtime, ctime, birthtime);
#endif /* !posix_futime_USE_posix_lutime */
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_futime_USE_posix_lutime || posix_futime_USE_posix_utime */

#ifdef posix_futime_USE_STUB
	(void)fd;
	(void)atime;
	(void)mtime;
	(void)ctime;
	(void)birthtime;
	posix_err_unsupported("futime");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_futime_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("utimeat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atime:?Etime:Time=Dee_None,mtime:?Etime:Time=Dee_None,ctime:?Etime:Time=Dee_None,birthtime:?Etime:Time=Dee_None,atflags:u=0", libname: "posix"); ]]]*/
#define POSIX_UTIMEAT_DEF          DEX_MEMBER_F("utimeat", &posix_utimeat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N,atflags=!0)"),
#define POSIX_UTIMEAT_DEF_DOC(doc) DEX_MEMBER_F("utimeat", &posix_utimeat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N,birthtime:?Etime:Time=!N,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) DREF DeeObject *DCALL posix_utimeat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime, unsigned int atflags);
#ifndef DEFINED_kwlist__dfd_path_atime_mtime_ctime_birthtime_atflags
#define DEFINED_kwlist__dfd_path_atime_mtime_ctime_birthtime_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_path_atime_mtime_ctime_birthtime_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atime", 0xdc4358af, 0x2d543aa498d68399), KEX("mtime", 0xd2dc3dac, 0x25287308d8bca8fd), KEX("ctime", 0xd3c27b8d, 0x4582041ef6ec6f80), KEX("birthtime", 0x85a70ed7, 0x1f72b099e114440a), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_path_atime_mtime_ctime_birthtime_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_utimeat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *path;
		DeeObject *atime;
		DeeObject *mtime;
		DeeObject *ctime;
		DeeObject *birthtime;
		unsigned int atflags;
	} args;
	args.atime = Dee_None;
	args.mtime = Dee_None;
	args.ctime = Dee_None;
	args.birthtime = Dee_None;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_path_atime_mtime_ctime_birthtime_atflags, "oo|oooou:utimeat", &args))
		goto err;
	return posix_utimeat_f_impl(args.dfd, args.path, args.atime, args.mtime, args.ctime, args.birthtime, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_utimeat, &posix_utimeat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) DREF DeeObject *DCALL posix_utimeat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime, unsigned int atflags)
/*[[[end]]]*/
{
#if defined(posix_utimeat_USE_posix_utime) || defined(posix_utimeat_USE_posix_lutime)
	DREF DeeObject *result, *abspath;
#ifdef posix_utimeat_USE_posix_futime
	if (atflags & AT_EMPTY_PATH) {
		if (!DeeString_Check(dfd) &&
		    (DeeNone_Check(path) || (DeeString_Check(path) &&
		                             DeeString_IsEmpty(path))))
			return posix_futime_f_impl(dfd, atime, mtime, ctime, birthtime);
		atflags &= ~AT_EMPTY_PATH;
	}
#endif /* posix_utimeat_USE_posix_futime */

#ifdef posix_utimeat_USED_utimensat
#ifdef stat_get_ctime_IS_stat_get_mtime
	if (!DeeNone_Check(ctime) && DeeNone_Check(mtime)) {
		mtime = ctime;
		ctime = Dee_None;
	}
#endif /* stat_get_ctime_IS_stat_get_mtime */
	if (!DeeString_Check(dfd) &&
#if posix_utimeat_USED_struct_timespec_COUNT == 2
	    DeeNone_Check(birthtime) &&
#endif /* posix_utimeat_USED_struct_timespec_COUNT == 2 */
	    DeeNone_Check(ctime)) {
		posix_utimeat_USED_struct_timespec tsv[posix_utimeat_USED_struct_timespec_COUNT];
		int error;
		int os_dfd;
		char const *utf8_path;
		os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
		if (DeeObject_AssertTypeExact(path, &DeeString_Type))
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
#if posix_utimeat_USED_struct_timespec_COUNT == 2
		if (posix_utimeat_USED_struct_timespec_parse(tsv, atime, mtime))
#else /* posix_utimeat_USED_struct_timespec_COUNT == 2 */
		if (posix_utimeat_USED_struct_timespec_parse(tsv, atime, mtime, birthtime))
#endif /* posix_utimeat_USED_struct_timespec_COUNT != 2 */
		{
			goto err;
		}
#ifdef CONFIG_HAVE_AT_CHANGE_BTIME
		atflags |= AT_CHANGE_BTIME;
#endif /* CONFIG_HAVE_AT_CHANGE_BTIME */
again:
		DBG_ALIGNMENT_DISABLE();
		if (posix_utimeat_USED_utimensat(os_dfd, utf8_path, tsv, atflags) == 0) {
			DBG_ALIGNMENT_ENABLE();
			return_none;
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeUnixSystem_HandleGenericError(error, err, again);
#ifdef CONFIG_HAVE_AT_CHANGE_BTIME
		atflags &= ~AT_CHANGE_BTIME;
#endif /* CONFIG_HAVE_AT_CHANGE_BTIME */
		/* Fallthru to fallback path below */
	}
#endif /* posix_utimeat_USED_utimensat */

#ifdef posix_utimeat_USE_posix_lutime
	abspath = posix_dfd_makepath(dfd, path, atflags & ~AT_SYMLINK_NOFOLLOW);
#else /* posix_utimeat_USE_posix_lutime */
	abspath = posix_dfd_makepath(dfd, path, atflags);
#endif /* !posix_utimeat_USE_posix_lutime */
	if unlikely(!abspath)
		goto err;
#ifdef posix_utimeat_USE_posix_lutime
	if (atflags & AT_SYMLINK_NOFOLLOW) {
		result = posix_lutime_f_impl(abspath, atime, mtime, ctime, birthtime);
	} else
#endif /* posix_utimeat_USE_posix_lutime */
	{
#ifdef posix_utimeat_USE_posix_utime
		result = posix_utime_f_impl(abspath, atime, mtime, ctime, birthtime);
#else /* posix_utimeat_USE_posix_utime */
		Dee_Decref(abspath);
		err_bad_atflags(atflags);
#define NEED_err_bad_atflags
		goto err;
#endif /* !posix_utimeat_USE_posix_utime */
	}
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_utimeat_USE_posix_utime || posix_utimeat_USE_posix_lutime */

#ifdef posix_utimeat_USE_STUB
	(void)dfd;
	(void)path;
	(void)atime;
	(void)mtime;
	(void)ctime;
	(void)birthtime;
	(void)atflags;
	posix_err_unsupported("utimeat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_utimeat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_UTIME_C_INL */
