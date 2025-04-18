/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_STAT_C_INL
#define GUARD_DEX_POSIX_P_STAT_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/abi/time.h>
#include <deemon/seq.h>
#include <deemon/util/atomic.h>

#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/unaligned.h>
/**/

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Figure out how to implement `stat()' */
#undef posix_stat_USE_WINDOWS
#undef posix_stat_USE_stat64
#undef posix_stat_USE_stat
#undef posix_stat_USE_fopen
#undef posix_stat_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define posix_stat_USE_WINDOWS
#elif (defined(CONFIG_HAVE_wstat64) || defined(CONFIG_HAVE_fstat64) || defined(CONFIG_HAVE_wfstatat64)) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_stat_USE_wstat64
#elif (defined(CONFIG_HAVE_wstat) || defined(CONFIG_HAVE_fstat) || defined(CONFIG_HAVE_wfstatat)) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_stat_USE_wstat
#elif (defined(CONFIG_HAVE_stat64) || defined(CONFIG_HAVE_fstat64) || defined(CONFIG_HAVE_fstatat64))
#define posix_stat_USE_stat64
#elif (defined(CONFIG_HAVE_stat) || defined(CONFIG_HAVE_fstat) || defined(CONFIG_HAVE_fstatat))
#define posix_stat_USE_stat
#ifdef CONFIG_HAVE_fstatat
#endif /* CONFIG_HAVE_fstatat */
#elif (defined(CONFIG_HAVE_wstat64) || defined(CONFIG_HAVE_fstat64) || defined(CONFIG_HAVE_wfstatat64))
#define posix_stat_USE_wstat64
#elif (defined(CONFIG_HAVE_wstat) || defined(CONFIG_HAVE_fstat) || defined(CONFIG_HAVE_wfstatat))
#define posix_stat_USE_wstat
#elif defined(CONFIG_HAVE_fopen) || defined(CONFIG_HAVE_fopen64)
#define posix_stat_USE_fopen
#else /* ... */
#define posix_stat_USE_STUB
#endif /* !... */


#undef posix_stat_CANNOT_HANDLE_DFD
#ifdef posix_stat_USE_WINDOWS
#define posix_stat_CANNOT_HANDLE_DFD
#elif defined(posix_stat_USE_stat64)
#ifndef CONFIG_HAVE_fstatat64
#define posix_stat_CANNOT_HANDLE_DFD
#endif /* !CONFIG_HAVE_fstatat64 */
#elif defined(posix_stat_USE_stat)
#ifndef CONFIG_HAVE_fstatat
#define posix_stat_CANNOT_HANDLE_DFD
#endif /* !CONFIG_HAVE_fstatat */
#endif /* posix_stat_USE_WINDOWS */


#undef posix_stat_HAVE_lstat
#undef posix_stat_USED_STRUCT_STAT
#undef posix_stat_USED_stat
#undef posix_stat_USED_lstat
#undef posix_stat_USED_fstat
#undef posix_stat_USED_fstatat

#ifdef posix_stat_USE_WINDOWS
#define posix_stat_NO_NATIVE_DFD
#define posix_stat_USES_WCHAR
#define posix_stat_HAVE_lstat
#elif defined(posix_stat_USE_wstat64)
#define posix_stat_USES_WCHAR
#define posix_stat_USES_STAT64
#define posix_stat_USED_STRUCT_STAT struct stat64
#define posix_stat_USED_stat(f, s)  wstat64((wchar_t *)(f), s)
#ifdef CONFIG_HAVE_wlstat64
#define posix_stat_USED_lstat(f, s) wlstat64((wchar_t *)(f), s)
#define posix_stat_HAVE_lstat
#endif /* CONFIG_HAVE_wlstat64 */
#ifdef CONFIG_HAVE_fstat64
#define posix_stat_USED_fstat fstat64
#endif /* CONFIG_HAVE_fstat64 */
#ifdef CONFIG_HAVE_wfstatat64
#define posix_stat_USED_fstatat wfstatat64
#endif /* CONFIG_HAVE_wfstatat64 */
#elif defined(posix_stat_USE_wstat)
#define posix_stat_USES_WCHAR
#define posix_stat_USES_STAT32
#define posix_stat_USED_STRUCT_STAT struct stat
#define posix_stat_USED_stat(f, s)  wstat((wchar_t *)(f), s)
#ifdef CONFIG_HAVE_wlstat
#define posix_stat_USED_lstat(f, s) wlstat((wchar_t *)(f), s)
#define posix_stat_HAVE_lstat
#endif /* CONFIG_HAVE_wlstat */
#ifdef CONFIG_HAVE_fstat
#define posix_stat_USED_fstat fstat
#endif /* CONFIG_HAVE_fstat */
#ifdef CONFIG_HAVE_wfstatat
#define posix_stat_USED_fstatat wfstatat
#endif /* CONFIG_HAVE_wfstatat */
#elif defined(posix_stat_USE_stat64)
#define posix_stat_USED_STRUCT_STAT struct stat64
#define posix_stat_USED_stat(f, s)  stat64((char *)(f), s)
#define posix_stat_USES_STAT64
#ifdef CONFIG_HAVE_lstat64
#define posix_stat_USED_lstat(f, s) lstat64((char *)(f), s)
#define posix_stat_HAVE_lstat
#endif /* CONFIG_HAVE_lstat64 */
#ifdef CONFIG_HAVE_fstat64
#define posix_stat_USED_fstat fstat64
#endif /* CONFIG_HAVE_fstat64 */
#ifdef CONFIG_HAVE_fstatat64
#define posix_stat_USED_fstatat fstatat64
#endif /* CONFIG_HAVE_fstatat64 */
#elif defined(posix_stat_USE_stat)
#define posix_stat_USED_STRUCT_STAT struct stat
#define posix_stat_USED_stat(f, s)  stat((char *)(f), s)
#define posix_stat_USES_STAT32
#ifdef CONFIG_HAVE_lstat
#define posix_stat_USED_lstat(f, s) lstat((char *)(f), s)
#define posix_stat_HAVE_lstat
#endif /* CONFIG_HAVE_lstat */
#ifdef CONFIG_HAVE_fstat
#define posix_stat_USED_fstat fstat
#endif /* CONFIG_HAVE_fstat */
#ifdef CONFIG_HAVE_fstatat
#define posix_stat_USED_fstatat fstatat
#endif /* CONFIG_HAVE_fstatat */
#elif defined(posix_stat_USE_fopen)
#define posix_stat_NO_NATIVE_DFD
#endif /* ... */

#ifdef posix_stat_USED_STRUCT_STAT
#ifdef posix_stat_USES_WCHAR
#define posix_stat_TCHAR             Dee_wchar_t
#define posix_stat_DeeString_AsTChar DeeString_AsWide
#else /* posix_stat_USES_WCHAR */
#define posix_stat_TCHAR             char
#define posix_stat_DeeString_AsTChar DeeString_AsUtf8
#endif /* !posix_stat_USES_WCHAR */
#ifndef posix_stat_USED_fstatat
#define posix_stat_NO_NATIVE_DFD
#endif /* !posix_stat_USED_fstatat */

#undef posix_stat_HAVE_st_dev
#undef posix_stat_HAVE_st_ino
#undef posix_stat_HAVE_st_mode
#undef posix_stat_HAVE_st_nlink
#undef posix_stat_HAVE_st_uid
#undef posix_stat_HAVE_st_gid
#undef posix_stat_HAVE_st_rdev
#undef posix_stat_HAVE_st_size
#undef posix_stat_HAVE_st_blksize
#undef posix_stat_HAVE_st_blocks
#undef posix_stat_HAVE_st_atime
#undef posix_stat_HAVE_st_atim
#undef posix_stat_HAVE_st_atimespec
#undef posix_stat_HAVE_st_atimensec
#undef posix_stat_HAVE_st_mtime
#undef posix_stat_HAVE_st_mtim
#undef posix_stat_HAVE_st_mtimespec
#undef posix_stat_HAVE_st_mtimensec
#undef posix_stat_HAVE_st_ctime
#undef posix_stat_HAVE_st_ctim
#undef posix_stat_HAVE_st_ctimespec
#undef posix_stat_HAVE_st_ctimensec
#undef posix_stat_HAVE_st_btime
#undef posix_stat_HAVE_st_btim
#undef posix_stat_HAVE_st_btimespec
#undef posix_stat_HAVE_st_btimensec
#undef posix_stat_HAVE_st_birthtime
#undef posix_stat_HAVE_st_birthtim
#undef posix_stat_HAVE_st_birthtimespec
#undef posix_stat_HAVE_st_birthtimensec
#ifdef posix_stat_USES_STAT32
#ifdef CONFIG_HAVE_struct_stat_st_dev
#define posix_stat_HAVE_st_dev
#endif /* CONFIG_HAVE_struct_stat_st_dev */
#ifdef CONFIG_HAVE_struct_stat_st_ino
#define posix_stat_HAVE_st_ino
#endif /* CONFIG_HAVE_struct_stat_st_ino */
#ifdef CONFIG_HAVE_struct_stat_st_mode
#define posix_stat_HAVE_st_mode
#endif /* CONFIG_HAVE_struct_stat_st_mode */
#ifdef CONFIG_HAVE_struct_stat_st_nlink
#define posix_stat_HAVE_st_nlink
#endif /* CONFIG_HAVE_struct_stat_st_nlink */
#ifdef CONFIG_HAVE_struct_stat_st_uid
#define posix_stat_HAVE_st_uid
#endif /* CONFIG_HAVE_struct_stat_st_uid */
#ifdef CONFIG_HAVE_struct_stat_st_gid
#define posix_stat_HAVE_st_gid
#endif /* CONFIG_HAVE_struct_stat_st_gid */
#ifdef CONFIG_HAVE_struct_stat_st_rdev
#define posix_stat_HAVE_st_rdev
#endif /* CONFIG_HAVE_struct_stat_st_rdev */
#ifdef CONFIG_HAVE_struct_stat_st_size
#define posix_stat_HAVE_st_size
#endif /* CONFIG_HAVE_struct_stat_st_size */
#ifdef CONFIG_HAVE_struct_stat_st_blksize
#define posix_stat_HAVE_st_blksize
#endif /* CONFIG_HAVE_struct_stat_st_blksize */
#ifdef CONFIG_HAVE_struct_stat_st_blocks
#define posix_stat_HAVE_st_blocks
#endif /* CONFIG_HAVE_struct_stat_st_blocks */
#ifdef CONFIG_HAVE_struct_stat_st_atime
#define posix_stat_HAVE_st_atime
#endif /* CONFIG_HAVE_struct_stat_st_atime */
#ifdef CONFIG_HAVE_struct_stat_st_atim
#define posix_stat_HAVE_st_atim
#endif /* CONFIG_HAVE_struct_stat_st_atim */
#ifdef CONFIG_HAVE_struct_stat_st_atimespec
#define posix_stat_HAVE_st_atimespec
#endif /* CONFIG_HAVE_struct_stat_st_atimespec */
#ifdef CONFIG_HAVE_struct_stat_st_atimensec
#define posix_stat_HAVE_st_atimensec
#endif /* CONFIG_HAVE_struct_stat_st_atimensec */
#ifdef CONFIG_HAVE_struct_stat_st_mtime
#define posix_stat_HAVE_st_mtime
#endif /* CONFIG_HAVE_struct_stat_st_mtime */
#ifdef CONFIG_HAVE_struct_stat_st_mtim
#define posix_stat_HAVE_st_mtim
#endif /* CONFIG_HAVE_struct_stat_st_mtim */
#ifdef CONFIG_HAVE_struct_stat_st_mtimespec
#define posix_stat_HAVE_st_mtimespec
#endif /* CONFIG_HAVE_struct_stat_st_mtimespec */
#ifdef CONFIG_HAVE_struct_stat_st_mtimensec
#define posix_stat_HAVE_st_mtimensec
#endif /* CONFIG_HAVE_struct_stat_st_mtimensec */
#ifdef CONFIG_HAVE_struct_stat_st_ctime
#define posix_stat_HAVE_st_ctime
#endif /* CONFIG_HAVE_struct_stat_st_ctime */
#ifdef CONFIG_HAVE_struct_stat_st_ctim
#define posix_stat_HAVE_st_ctim
#endif /* CONFIG_HAVE_struct_stat_st_ctim */
#ifdef CONFIG_HAVE_struct_stat_st_ctimespec
#define posix_stat_HAVE_st_ctimespec
#endif /* CONFIG_HAVE_struct_stat_st_ctimespec */
#ifdef CONFIG_HAVE_struct_stat_st_ctimensec
#define posix_stat_HAVE_st_ctimensec
#endif /* CONFIG_HAVE_struct_stat_st_ctimensec */
#ifdef CONFIG_HAVE_struct_stat_st_btime
#define posix_stat_HAVE_st_btime
#endif /* CONFIG_HAVE_struct_stat_st_btime */
#ifdef CONFIG_HAVE_struct_stat_st_btim
#define posix_stat_HAVE_st_btim
#endif /* CONFIG_HAVE_struct_stat_st_btim */
#ifdef CONFIG_HAVE_struct_stat_st_btimespec
#define posix_stat_HAVE_st_btimespec
#endif /* CONFIG_HAVE_struct_stat_st_btimespec */
#ifdef CONFIG_HAVE_struct_stat_st_btimensec
#define posix_stat_HAVE_st_btimensec
#endif /* CONFIG_HAVE_struct_stat_st_btimensec */
#ifdef CONFIG_HAVE_struct_stat_st_birthtime
#define posix_stat_HAVE_st_birthtime
#endif /* CONFIG_HAVE_struct_stat_st_birthtime */
#ifdef CONFIG_HAVE_struct_stat_st_birthtim
#define posix_stat_HAVE_st_birthtim
#endif /* CONFIG_HAVE_struct_stat_st_birthtim */
#ifdef CONFIG_HAVE_struct_stat_st_birthtimespec
#define posix_stat_HAVE_st_birthtimespec
#endif /* CONFIG_HAVE_struct_stat_st_birthtimespec */
#ifdef CONFIG_HAVE_struct_stat_st_birthtimensec
#define posix_stat_HAVE_st_birthtimensec
#endif /* CONFIG_HAVE_struct_stat_st_birthtimensec */
#elif defined(posix_stat_USES_STAT64)
#ifdef CONFIG_HAVE_struct_stat64_st_dev
#define posix_stat_HAVE_st_dev
#endif /* CONFIG_HAVE_struct_stat64_st_dev */
#ifdef CONFIG_HAVE_struct_stat64_st_ino
#define posix_stat_HAVE_st_ino
#endif /* CONFIG_HAVE_struct_stat64_st_ino */
#ifdef CONFIG_HAVE_struct_stat64_st_mode
#define posix_stat_HAVE_st_mode
#endif /* CONFIG_HAVE_struct_stat64_st_mode */
#ifdef CONFIG_HAVE_struct_stat64_st_nlink
#define posix_stat_HAVE_st_nlink
#endif /* CONFIG_HAVE_struct_stat64_st_nlink */
#ifdef CONFIG_HAVE_struct_stat64_st_uid
#define posix_stat_HAVE_st_uid
#endif /* CONFIG_HAVE_struct_stat64_st_uid */
#ifdef CONFIG_HAVE_struct_stat64_st_gid
#define posix_stat_HAVE_st_gid
#endif /* CONFIG_HAVE_struct_stat64_st_gid */
#ifdef CONFIG_HAVE_struct_stat64_st_rdev
#define posix_stat_HAVE_st_rdev
#endif /* CONFIG_HAVE_struct_stat64_st_rdev */
#ifdef CONFIG_HAVE_struct_stat64_st_size
#define posix_stat_HAVE_st_size
#endif /* CONFIG_HAVE_struct_stat64_st_size */
#ifdef CONFIG_HAVE_struct_stat64_st_blksize
#define posix_stat_HAVE_st_blksize
#endif /* CONFIG_HAVE_struct_stat64_st_blksize */
#ifdef CONFIG_HAVE_struct_stat64_st_blocks
#define posix_stat_HAVE_st_blocks
#endif /* CONFIG_HAVE_struct_stat64_st_blocks */
#ifdef CONFIG_HAVE_struct_stat64_st_atime
#define posix_stat_HAVE_st_atime
#endif /* CONFIG_HAVE_struct_stat64_st_atime */
#ifdef CONFIG_HAVE_struct_stat64_st_atim
#define posix_stat_HAVE_st_atim
#endif /* CONFIG_HAVE_struct_stat64_st_atim */
#ifdef CONFIG_HAVE_struct_stat64_st_atimespec
#define posix_stat_HAVE_st_atimespec
#endif /* CONFIG_HAVE_struct_stat64_st_atimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_atimensec
#define posix_stat_HAVE_st_atimensec
#endif /* CONFIG_HAVE_struct_stat64_st_atimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_mtime
#define posix_stat_HAVE_st_mtime
#endif /* CONFIG_HAVE_struct_stat64_st_mtime */
#ifdef CONFIG_HAVE_struct_stat64_st_mtim
#define posix_stat_HAVE_st_mtim
#endif /* CONFIG_HAVE_struct_stat64_st_mtim */
#ifdef CONFIG_HAVE_struct_stat64_st_mtimespec
#define posix_stat_HAVE_st_mtimespec
#endif /* CONFIG_HAVE_struct_stat64_st_mtimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_mtimensec
#define posix_stat_HAVE_st_mtimensec
#endif /* CONFIG_HAVE_struct_stat64_st_mtimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_ctime
#define posix_stat_HAVE_st_ctime
#endif /* CONFIG_HAVE_struct_stat64_st_ctime */
#ifdef CONFIG_HAVE_struct_stat64_st_ctim
#define posix_stat_HAVE_st_ctim
#endif /* CONFIG_HAVE_struct_stat64_st_ctim */
#ifdef CONFIG_HAVE_struct_stat64_st_ctimespec
#define posix_stat_HAVE_st_ctimespec
#endif /* CONFIG_HAVE_struct_stat64_st_ctimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_ctimensec
#define posix_stat_HAVE_st_ctimensec
#endif /* CONFIG_HAVE_struct_stat64_st_ctimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_btime
#define posix_stat_HAVE_st_btime
#endif /* CONFIG_HAVE_struct_stat64_st_btime */
#ifdef CONFIG_HAVE_struct_stat64_st_btim
#define posix_stat_HAVE_st_btim
#endif /* CONFIG_HAVE_struct_stat64_st_btim */
#ifdef CONFIG_HAVE_struct_stat64_st_btimespec
#define posix_stat_HAVE_st_btimespec
#endif /* CONFIG_HAVE_struct_stat64_st_btimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_btimensec
#define posix_stat_HAVE_st_btimensec
#endif /* CONFIG_HAVE_struct_stat64_st_btimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtime
#define posix_stat_HAVE_st_birthtime
#endif /* CONFIG_HAVE_struct_stat64_st_birthtime */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtim
#define posix_stat_HAVE_st_birthtim
#endif /* CONFIG_HAVE_struct_stat64_st_birthtim */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtimespec
#define posix_stat_HAVE_st_birthtimespec
#endif /* CONFIG_HAVE_struct_stat64_st_birthtimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtimensec
#define posix_stat_HAVE_st_birthtimensec
#endif /* CONFIG_HAVE_struct_stat64_st_birthtimensec */
#endif /* ... */

/* Helper macros to accessing timestamp data */
#undef posix_stat_GET_RAW_ATIME_SEC
#undef posix_stat_GET_RAW_ATIME_NSEC
#undef posix_stat_GET_RAW_MTIME_SEC
#undef posix_stat_GET_RAW_MTIME_NSEC
#undef posix_stat_GET_RAW_CTIME_SEC
#undef posix_stat_GET_RAW_CTIME_NSEC
#undef posix_stat_GET_RAW_BTIME_SEC
#undef posix_stat_GET_RAW_BTIME_NSEC
#ifdef posix_stat_HAVE_st_atimespec
#define posix_stat_GET_RAW_ATIME_SEC(st)  (st)->st_atimespec.tv_sec
#define posix_stat_GET_RAW_ATIME_NSEC(st) (st)->st_atimespec.tv_nsec
#elif defined(posix_stat_HAVE_st_atim)
#define posix_stat_GET_RAW_ATIME_SEC(st)  (st)->st_atim.tv_sec
#define posix_stat_GET_RAW_ATIME_NSEC(st) (st)->st_atim.tv_nsec
#else /* ... */
#ifdef posix_stat_HAVE_st_atime
#define posix_stat_GET_RAW_ATIME_SEC(st)  (st)->st_atime
#endif /* posix_stat_HAVE_st_atime */
#ifdef posix_stat_HAVE_st_atimensec
#define posix_stat_GET_RAW_ATIME_NSEC(st) (st)->st_atimensec
#endif /* posix_stat_HAVE_st_atimensec */
#endif /* !... */

#ifdef posix_stat_HAVE_st_mtimespec
#define posix_stat_GET_RAW_MTIME_SEC(st)  (st)->st_mtimespec.tv_sec
#define posix_stat_GET_RAW_MTIME_NSEC(st) (st)->st_mtimespec.tv_nsec
#elif defined(posix_stat_HAVE_st_mtim)
#define posix_stat_GET_RAW_MTIME_SEC(st)  (st)->st_mtim.tv_sec
#define posix_stat_GET_RAW_MTIME_NSEC(st) (st)->st_mtim.tv_nsec
#else /* ... */
#ifdef posix_stat_HAVE_st_mtime
#define posix_stat_GET_RAW_MTIME_SEC(st)  (st)->st_mtime
#endif /* posix_stat_HAVE_st_mtime */
#ifdef posix_stat_HAVE_st_mtimensec
#define posix_stat_GET_RAW_MTIME_NSEC(st) (st)->st_mtimensec
#endif /* posix_stat_HAVE_st_mtimensec */
#endif /* !... */

#ifdef posix_stat_HAVE_st_ctimespec
#define posix_stat_GET_RAW_CTIME_SEC(st)  (st)->st_ctimespec.tv_sec
#define posix_stat_GET_RAW_CTIME_NSEC(st) (st)->st_ctimespec.tv_nsec
#elif defined(posix_stat_HAVE_st_ctim)
#define posix_stat_GET_RAW_CTIME_SEC(st)  (st)->st_ctim.tv_sec
#define posix_stat_GET_RAW_CTIME_NSEC(st) (st)->st_ctim.tv_nsec
#else /* ... */
#ifdef posix_stat_HAVE_st_ctime
#define posix_stat_GET_RAW_CTIME_SEC(st)  (st)->st_ctime
#endif /* posix_stat_HAVE_st_ctime */
#ifdef posix_stat_HAVE_st_ctimensec
#define posix_stat_GET_RAW_CTIME_NSEC(st) (st)->st_ctimensec
#endif /* posix_stat_HAVE_st_ctimensec */
#endif /* !... */

#ifdef posix_stat_HAVE_st_btimespec
#define posix_stat_GET_RAW_BTIME_SEC(st)  (st)->st_btimespec.tv_sec
#define posix_stat_GET_RAW_BTIME_NSEC(st) (st)->st_btimespec.tv_nsec
#elif defined(posix_stat_HAVE_st_btim)
#define posix_stat_GET_RAW_BTIME_SEC(st)  (st)->st_btim.tv_sec
#define posix_stat_GET_RAW_BTIME_NSEC(st) (st)->st_btim.tv_nsec
#elif defined(posix_stat_HAVE_st_birthtimespec)
#define posix_stat_GET_RAW_BTIME_SEC(st)  (st)->st_birthtimespec.tv_sec
#define posix_stat_GET_RAW_BTIME_NSEC(st) (st)->st_birthtimespec.tv_nsec
#elif defined(posix_stat_HAVE_st_birthtim)
#define posix_stat_GET_RAW_BTIME_SEC(st)  (st)->st_birthtim.tv_sec
#define posix_stat_GET_RAW_BTIME_NSEC(st) (st)->st_birthtim.tv_nsec
#else /* ... */
#ifdef posix_stat_HAVE_st_btime
#define posix_stat_GET_RAW_BTIME_SEC(st)  (st)->st_btime
#elif defined(posix_stat_HAVE_st_birthtime)
#define posix_stat_GET_RAW_BTIME_SEC(st)  (st)->st_birthtime
#endif /* ... */
#ifdef posix_stat_HAVE_st_btimensec
#define posix_stat_GET_RAW_BTIME_NSEC(st) (st)->st_btimensec
#elif defined(posix_stat_HAVE_st_birthtimensec)
#define posix_stat_GET_RAW_BTIME_NSEC(st) (st)->st_birthtimensec
#endif /* ... */
#endif /* !... */

#undef posix_stat_GET_ATIME_SEC
#undef posix_stat_GET_ATIME_NSEC
#undef posix_stat_GET_MTIME_SEC
#undef posix_stat_GET_MTIME_NSEC
#undef posix_stat_GET_CTIME_SEC
#undef posix_stat_GET_CTIME_NSEC
#undef posix_stat_GET_BTIME_SEC
#undef posix_stat_GET_BTIME_NSEC
#ifdef posix_stat_GET_RAW_ATIME_SEC
#define posix_stat_GET_ATIME_SEC posix_stat_GET_RAW_ATIME_SEC
#endif /* posix_stat_GET_RAW_ATIME_SEC */
#ifdef posix_stat_GET_RAW_ATIME_NSEC
#define posix_stat_GET_ATIME_NSEC posix_stat_GET_RAW_ATIME_NSEC
#endif /* posix_stat_GET_RAW_ATIME_NSEC */

#ifdef posix_stat_GET_RAW_MTIME_SEC
#define posix_stat_GET_MTIME_SEC posix_stat_GET_RAW_MTIME_SEC
#endif /* posix_stat_GET_RAW_MTIME_SEC */
#ifdef posix_stat_GET_RAW_MTIME_NSEC
#define posix_stat_GET_MTIME_NSEC posix_stat_GET_RAW_MTIME_NSEC
#endif /* posix_stat_GET_RAW_MTIME_NSEC */

#ifdef CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME
#ifdef posix_stat_GET_RAW_CTIME_SEC
#define posix_stat_GET_BTIME_SEC posix_stat_GET_RAW_CTIME_SEC
#endif /* posix_stat_GET_RAW_CTIME_SEC */
#ifdef posix_stat_GET_RAW_CTIME_NSEC
#define posix_stat_GET_BTIME_NSEC posix_stat_GET_RAW_CTIME_NSEC
#endif /* posix_stat_GET_RAW_CTIME_NSEC */
#else /* CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME */
#ifdef posix_stat_GET_RAW_CTIME_SEC
#define posix_stat_GET_CTIME_SEC posix_stat_GET_RAW_CTIME_SEC
#endif /* posix_stat_GET_RAW_CTIME_SEC */
#ifdef posix_stat_GET_RAW_CTIME_NSEC
#define posix_stat_GET_CTIME_NSEC posix_stat_GET_RAW_CTIME_NSEC
#endif /* posix_stat_GET_RAW_CTIME_NSEC */

#ifdef posix_stat_GET_RAW_BTIME_SEC
#define posix_stat_GET_BTIME_SEC posix_stat_GET_RAW_BTIME_SEC
#endif /* posix_stat_GET_RAW_BTIME_SEC */
#ifdef posix_stat_GET_RAW_BTIME_NSEC
#define posix_stat_GET_BTIME_NSEC posix_stat_GET_RAW_BTIME_NSEC
#endif /* posix_stat_GET_RAW_BTIME_NSEC */
#endif /* !CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME */

#endif /* posix_stat_USED_STRUCT_STAT */


struct stat_object;
typedef struct stat_object DeeStatObject;
struct dee_stat {
#ifdef posix_stat_USE_WINDOWS
	BY_HANDLE_FILE_INFORMATION st_info;       /* [const] Windows-specific stat information. */
	DWORD                      st_ftype;      /* One of `FILE_TYPE_*' or `FILE_TYPE_UNKNOWN' when not determined. */
#define NT_STAT_FNORMAL        0x0000         /* Normal information. */
#define NT_STAT_FNOTIME        0x0001         /* Time stamps are unknown. */
#define NT_STAT_FNOVOLSERIAL   0x0002         /* `dwVolumeSerialNumber' is unknown. */
#define NT_STAT_FNOSIZE        0x0004         /* `nFileSize' is unknown. */
#define NT_STAT_FNONLINK       0x0008         /* `nNumberOfLinks' is unknown. */
#define NT_STAT_FNOFILEID      0x0010         /* `nFileIndex' is unknown. */
	uint32_t                   st_valid;      /* Set of `NT_STAT_F*' */
	HANDLE                     st_hand;       /* [0..1|NULL(INVALID_HANDLE_VALUE)]
	                                           * Optional handle that may be used to load
	                                           * additional information upon request. */
	DREF DeeObject            *st_hand_owner; /* [0..1][const] Owner of `st_hand' (if NULL, we own it) */
#define dee_stat_fini(self)                    \
	((self)->st_hand_owner                     \
	 ? Dee_Decref((self)->st_hand_owner)       \
	 : (self)->st_hand != INVALID_HANDLE_VALUE \
	   ? (void)CloseHandle((self)->st_hand)    \
	   : (void)0)
#endif /* posix_stat_USE_WINDOWS */

#ifdef posix_stat_USED_STRUCT_STAT
	posix_stat_USED_STRUCT_STAT st_info; /* Unix stat information */
#endif /* posix_stat_USED_STRUCT_STAT */

#ifdef posix_stat_USE_fopen
	FILE *st_file; /* [1..1][const] Linked file */
#ifdef CONFIG_HAVE_fclose
#define dee_stat_fini(self) (void)fclose((self)->st_file)
#endif /* CONFIG_HAVE_fclose */
#endif /* posix_stat_USE_fopen */
};

#ifndef dee_stat_fini
#define dee_stat_fini_IS_NOOP
#define dee_stat_fini(self) (void)0
#endif /* !dee_stat_fini */


#ifdef posix_stat_USE_STUB
PRIVATE ATTR_COLD int DCALL err_stat_not_implemented(void) {
#define NEED_posix_err_unsupported
	return posix_err_unsupported("stat");
}
#endif /* !posix_stat_USE_STUB */

#ifndef posix_stat_HAVE_lstat
PRIVATE ATTR_COLD int DCALL err_lstat_not_implemented(void) {
#define NEED_posix_err_unsupported
	return posix_err_unsupported("lstat");
}
#endif /* !posix_stat_HAVE_lstat */


/* Bits for `dee_stat_init::atflags' */
#define DEE_STAT_F_NORMAL   0x00                       /* Normal flags */
#define DEE_STAT_F_TRY      (AT_SYMLINK_NOFOLLOW << 1) /* Return `1' on file-not-found, rather than throw an exception */
#define DEE_STAT_F_LSTAT    AT_SYMLINK_NOFOLLOW        /* Don't dereference a final symlink */

/* @return: 0 : Success
 * @return: -1: Error
 * @return: 1 : No such file, and `DEE_STAT_F_TRY' */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
dee_stat_init(struct dee_stat *__restrict self, DeeObject *dfd,
              DeeObject *path_or_file, unsigned int atflags) {
#ifdef posix_stat_NO_NATIVE_DFD
	if (dfd != NULL) {
		int result;
		DREF DeeObject *abs_path;
#define NEED_posix_dfd_makepath
		abs_path = posix_dfd_makepath(dfd, path_or_file,
		                              atflags & ~(DEE_STAT_F_TRY |
		                                          DEE_STAT_F_LSTAT));
		if unlikely(!abs_path)
			goto err;
#define NEED_err
		result = dee_stat_init(self, NULL, abs_path, atflags);
		Dee_Decref(abs_path);
		return result;
	}
#endif /* posix_stat_NO_NATIVE_DFD */

#ifdef posix_stat_USE_WINDOWS
	self->st_ftype = FILE_TYPE_UNKNOWN;
again:
	if (DeeString_Check(path_or_file)) {
		int error;
		HANDLE hFile;
		DWORD dwFlagsAndAttributes;
		if (atflags & DEE_STAT_F_LSTAT) {
			dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT;
		} else {
			dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS;
		}

		/* Try to open the file */
		hFile = DeeNTSystem_CreateFileNoATime(path_or_file, FILE_READ_ATTRIBUTES | READ_CONTROL,
		                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                                      NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			hFile = DeeNTSystem_CreateFileNoATime(path_or_file, FILE_READ_ATTRIBUTES,
			                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			                                      NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
		}
		if (hFile == NULL)
			goto err;
#define NEED_err
		self->st_hand_owner = NULL;
		if (hFile != INVALID_HANDLE_VALUE) {
			BOOL bOk;
			DBG_ALIGNMENT_DISABLE();
			bOk = GetFileInformationByHandle(hFile, &self->st_info);
			if (!bOk) {
				CloseHandle(hFile);
				DBG_ALIGNMENT_ENABLE();
				goto err_nt;
			}
			DBG_ALIGNMENT_ENABLE();
			self->st_valid = NT_STAT_FNORMAL;
			self->st_hand  = hFile; /* Inherit */
			return 0;
		}
		self->st_hand = INVALID_HANDLE_VALUE;

		/* CreateFile() failed. - Try a more direct approach. */
		bzero(&self->st_info, sizeof(BY_HANDLE_FILE_INFORMATION));
		{
			WIN32_FILE_ATTRIBUTE_DATA attrib;
			DBG_ALIGNMENT_DISABLE();
#define NEED_nt_GetFileAttributesEx
			error = nt_GetFileAttributesEx(path_or_file, GetFileExInfoStandard, &attrib);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(error < 0)
				goto err;
			if (!error) {
				/* It worked! */
				self->st_info.dwFileAttributes = attrib.dwFileAttributes;
				self->st_info.ftCreationTime   = attrib.ftCreationTime;
				self->st_info.ftLastAccessTime = attrib.ftLastAccessTime;
				self->st_info.ftLastWriteTime  = attrib.ftLastWriteTime;
				self->st_info.nFileSizeHigh    = attrib.nFileSizeHigh;
				self->st_info.nFileSizeLow     = attrib.nFileSizeLow;
				self->st_valid = (NT_STAT_FNOVOLSERIAL | NT_STAT_FNONLINK | NT_STAT_FNOFILEID);
				return 0;
			}
		}

		/* Nope. Still nothing...
		 * Try this one last thing. */
#define NEED_nt_GetFileAttributes
		DBG_ALIGNMENT_DISABLE();
		error = nt_GetFileAttributes(path_or_file, &self->st_info.dwFileAttributes);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(error < 0)
			goto err;
		if unlikely(error)
			goto err_nt;
		self->st_valid = (NT_STAT_FNOTIME | NT_STAT_FNOVOLSERIAL | NT_STAT_FNOSIZE |
		                  NT_STAT_FNONLINK | NT_STAT_FNOFILEID);
		return 0;
	} else {
		HANDLE hFile;
		hFile = DeeNTSystem_GetHandle(path_or_file);
		if (hFile == INVALID_HANDLE_VALUE) {
			int error;
			if (!DeeError_Catch(&DeeError_AttributeError) &&
			    !DeeError_Catch(&DeeError_NotImplemented) &&
			    !DeeError_Catch(&DeeError_FileClosed))
				goto err;
			/* Try to use the filename of the given object. */
			path_or_file = DeeFile_Filename(path_or_file);
			if unlikely(!path_or_file)
				goto err;
			error = dee_stat_init(self, NULL, path_or_file, atflags);
			Dee_Decref(path_or_file);
			return error;
		}

		/* Retrieve information by handle. */
		DBG_ALIGNMENT_DISABLE();
		if (GetFileInformationByHandle(hFile, &self->st_info)) {
			DBG_ALIGNMENT_ENABLE();
			self->st_valid      = NT_STAT_FNORMAL;
			self->st_hand       = hFile;
			self->st_hand_owner = path_or_file;
			Dee_Incref(path_or_file);
			return 0;
		}
		DBG_ALIGNMENT_ENABLE();
		/* Fallthru to `err_nt' */
	}

	{
		DWORD dwError;
err_nt:
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if ((atflags & DEE_STAT_F_TRY) && DeeNTSystem_IsFileNotFoundError(dwError))
			return 1; /* File not found. */

		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
		} else if (DeeNTSystem_IsNotDir(dwError)) {
			if (atflags & DEE_STAT_F_TRY)
				return 1;
#define NEED_err_nt_path_not_dir
			err_nt_path_not_dir(dwError, path_or_file);
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
#define NEED_err_nt_path_no_access
			err_nt_path_no_access(dwError, path_or_file);
		} else if (DeeNTSystem_IsFileNotFoundError(dwError)) {
			if (atflags & DEE_STAT_F_TRY)
				return 1;
#define NEED_err_nt_path_not_found
			err_nt_path_not_found(dwError, path_or_file);
		} else if (DeeNTSystem_IsBadF(dwError)) {
#define NEED_err_nt_handle_closed
			err_nt_handle_closed(dwError, path_or_file);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
			                        "Failed to open file %r",
			                        path_or_file);
		}
	}
#define NEED_err
	/* Fallthru to `err' */
#endif /* posix_stat_USE_WINDOWS */

#ifdef posix_stat_USED_STRUCT_STAT
#define NEED_err
#ifdef EINTR
again:
#endif /* EINTR */
	if (DeeThread_CheckInterrupt())
		goto err;
	{
		int error;
#ifdef posix_stat_USED_fstatat
		if (dfd) {
			posix_stat_TCHAR const *str_path;
			int os_dfd;
			os_dfd = DeeUnixSystem_GetFD(dfd);
			if (os_dfd == -1) {
				if (DeeError_Catch(&DeeError_AttributeError) ||
				    DeeError_Catch(&DeeError_NotImplemented)) {
					DREF DeeObject *abs_path;
#define NEED_posix_dfd_makepath
					abs_path = posix_dfd_makepath(dfd, path_or_file,
					                              atflags & ~(DEE_STAT_F_TRY |
					                                          DEE_STAT_F_LSTAT));
					if unlikely(!abs_path)
						goto err;
					error = dee_stat_init(self, NULL, abs_path, atflags);
					Dee_Decref(abs_path);
					return error;
				}
				goto err;
			}
			if (DeeObject_AssertTypeExact(path_or_file, &DeeString_Type))
				goto err;
			str_path = posix_stat_DeeString_AsTChar(path_or_file);
			if unlikely(!str_path)
				goto err;
			error = posix_stat_USED_fstatat(os_dfd, str_path, &self->st_info,
			                                atflags & ~DEE_STAT_F_TRY);
		} else
#endif /* !posix_stat_USED_fstatat */
		{
			if (DeeString_Check(path_or_file)) {
				posix_stat_TCHAR const *str_path;
				str_path = posix_stat_DeeString_AsTChar(path_or_file);
				if unlikely(!str_path)
					goto err;
#ifdef posix_stat_USED_lstat
				if (atflags & AT_SYMLINK_NOFOLLOW) {
					error = posix_stat_USED_lstat(str_path, &self->st_info);
				} else
#endif /* posix_stat_USED_lstat */
				{
					error = posix_stat_USED_stat(str_path, &self->st_info);
				}
			} else {
				int file_fd;
				file_fd = DeeUnixSystem_GetFD(path_or_file);
				if unlikely(file_fd == -1)
					goto err;
#ifdef posix_stat_USED_fstat
				error = posix_stat_USED_fstat(file_fd, &self->st_info);
#else /* posix_stat_USED_fstat */
				{
					char buf[COMPILER_STRLEN("/proc/self/fd/-2147483648") + 1];
					Dee_sprintf(buf, "/proc/self/fd/%d", file_fd);
#ifdef posix_stat_USES_WCHAR
					{ 
						Dee_wchar_t wbuf[COMPILER_LENOF(buf)];
						size_t i;
						for (i = 0; i < COMPILER_LENOF(buf); ++i)
							wbuf[i] = buf[i];
						error = posix_stat_USED_stat(wbuf, &self->st_info);
					}
#else /* posix_stat_USES_WCHAR */
					error = posix_stat_USED_stat(buf, &self->st_info);
#endif /* !posix_stat_USES_WCHAR */
				}
#endif /* !posix_stat_USED_fstat */
			}
		}
		if likely(error == 0)
			return 0;
		error = DeeSystem_GetErrno();
#ifdef EINTR
		if (error == EINTR)
			goto again;
#endif /* EINTR */
#ifdef EACCES
		if (error == EACCES)
			return err_unix_path_no_access(error, path_or_file);
#define NEED_err_unix_path_no_access
#endif /* EACCES */
#ifdef ENOTDIR
		if (error == ENOTDIR) {
			if (atflags & DEE_STAT_F_TRY)
				return 1;
			return err_unix_path_not_dir(error, path_or_file);
		}
#define NEED_err_unix_path_not_dir
#endif /* ENOTDIR */
#ifdef ENOENT
		if (error == ENOENT) {
			if (atflags & DEE_STAT_F_TRY)
				return 1;
			return err_unix_path_not_found(error, path_or_file);
		}
#define NEED_err_unix_path_not_found
#endif /* ENOENT */
#ifdef EBADF
		if (error == EBADF)
			return err_unix_handle_closed(error, path_or_file);
#define NEED_err_unix_handle_closed
#endif /* EBADF */
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to stat %r",
		                          path_or_file);
		goto err;
	}
#endif /* posix_stat_USED_STRUCT_STAT */

#ifdef posix_stat_USE_fopen
	(void)atflags;
	{
		char const *utf8_filename;
		utf8_filename = DeeString_AsUtf8(path_or_file);
		if unlikely(!utf8_filename)
			goto err;
#define NEED_err
#ifdef CONFIG_HAVE_fopen64
		self->st_file = fopen64((char *)utf8_filename, "r");
#else /* CONFIG_HAVE_fopen64 */
		self->st_file = fopen((char *)utf8_filename, "r");
#endif /* !CONFIG_HAVE_fopen64 */
		if unlikely(!self->st_file) {
			if (atflags & DEE_STAT_F_TRY)
				return 1;
			return DeeError_Throwf(&DeeError_FileNotFound,
			                       "Path %r could not be found",
			                       path_or_file);
		}
		return 0;
	}
#endif /* !posix_stat_USE_fopen */

#ifdef posix_stat_USE_STUB
	(void)self;
	(void)dfd;
	(void)path_or_file;
	(void)atflags;
	return err_stat_not_implemented();
#endif /* !posix_stat_USE_STUB */

#ifdef NEED_err
#undef NEED_err
err:
	return -1;
#endif /* NEED_err */
}


#ifdef DECLARE_DeeTime_NewUnix
INTDEF DECLARE_DeeTime_NewUnix();
#undef DECLARE_DeeTime_NewUnix
#endif /* DECLARE_DeeTime_NewUnix */

#ifdef DECLARE_DeeTime_NewFILETIME
INTDEF DECLARE_DeeTime_NewFILETIME();
#undef DECLARE_DeeTime_NewFILETIME
#endif /* DECLARE_DeeTime_NewFILETIME */


#ifdef posix_stat_USE_WINDOWS
/* Returns one of `FILE_TYPE_*' describing the type of the given file.
 * When the type cannot be determined, return FILE_TIME_UNKNOWN when
 * `try_get' is true, or throw an error when it is false. */
PRIVATE DWORD DCALL
stat_get_nttype(struct dee_stat *__restrict self, bool try_get) {
	DWORD new_type, result = self->st_ftype;
	if (result == FILE_TYPE_UNKNOWN) {
		if (self->st_hand == INVALID_HANDLE_VALUE)
			goto err_noinfo;
		if (DeeThread_CheckInterrupt())
			goto err;
		DBG_ALIGNMENT_DISABLE();
		result = GetFileType(self->st_hand);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(result == FILE_TYPE_UNKNOWN)
			goto err_noinfo;
		new_type = atomic_cmpxch_val(&self->st_ftype, FILE_TYPE_UNKNOWN, result);
		if (new_type != FILE_TYPE_UNKNOWN)
			result = new_type;
	}
	return result;
err_noinfo:
	if (!try_get)
		err_stat_no_nttype_info();
#define NEED_err_stat_no_nttype_info
err:
	return FILE_TYPE_UNKNOWN;
}
#endif /* posix_stat_USE_WINDOWS */



struct stat_object {
	OBJECT_HEAD
	struct dee_stat so_stat; /* Stat info */
};

INTDEF DeeTypeObject DeeStat_Type;
INTDEF DeeTypeObject DeeLStat_Type;

#define stat_unpack_args_format(name) "o|oo:" name
PRIVATE WUNUSED NONNULL((3, 4, 5, 6)) int DCALL
stat_unpack_args(size_t argc, DeeObject *const *argv,
                 char const *__restrict format,
                 /*out*/ DeeObject **__restrict p_dfd,
                 /*out*/ DeeObject **__restrict p_path_or_file,
                 /*out*/ unsigned int *__restrict p_used_atflags) {
	DeeObject *user_atflags = NULL;
	*p_path_or_file = NULL;
	if (DeeArg_Unpack(argc, argv, format, p_dfd, p_path_or_file, &user_atflags))
		goto err;
	/* Normalize arguments. */
	if (!user_atflags) {
		if (*p_path_or_file) {
			if (DeeInt_Check(*p_path_or_file)) {
				user_atflags    = *p_path_or_file;
				*p_path_or_file = *p_dfd;
				*p_dfd          = NULL;
			}
		} else {
			*p_path_or_file = *p_dfd;
			*p_dfd          = NULL;
		}
	}
	*p_used_atflags = DEE_STAT_F_NORMAL;
	if (user_atflags) {
		if (DeeInt_AsUInt(user_atflags, p_used_atflags))
			goto err;
#ifdef posix_stat_HAVE_lstat
		if (*p_used_atflags & ~AT_SYMLINK_NOFOLLOW)
#else /* posix_stat_HAVE_lstat */
		if (*p_used_atflags)
#endif /* !posix_stat_HAVE_lstat */
		{
#ifndef posix_stat_HAVE_lstat
			if (*p_used_atflags & AT_SYMLINK_NOFOLLOW) {
				err_lstat_not_implemented();
				goto err;
			}
#endif /* !posix_stat_HAVE_lstat */
#define NEED_err_bad_atflags
			err_bad_atflags(*p_used_atflags);
			goto err;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stat_init(DeeStatObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	unsigned int used_atflags;
	DeeObject *dfd, *path_or_file;
	if (stat_unpack_args(argc, argv, stat_unpack_args_format("stat"),
	                     &dfd, &path_or_file, &used_atflags))
		goto err;
	return dee_stat_init(&self->so_stat, dfd, path_or_file, used_atflags);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lstat_init(DeeStatObject *__restrict self,
           size_t argc, DeeObject *const *argv) {
#ifdef posix_stat_HAVE_lstat
	DeeObject *dfd, *path_or_file = NULL;
	_DeeArg_Unpack1Or2(err, argc, argv, "lstat", &dfd, &path_or_file);
	if (!path_or_file) {
		path_or_file = dfd;
		dfd          = NULL;
	}
	return dee_stat_init(&self->so_stat, dfd, path_or_file, DEE_STAT_F_LSTAT);
err:
	return -1;
#else /* posix_stat_HAVE_lstat */
	(void)self;
	(void)argc;
	(void)argv;
	return err_lstat_not_implemented();
#endif /* !posix_stat_HAVE_lstat */
}


/*[[[deemon import("rt.gen.dexutils").gw("fstat", "fd:?X2?DFile?Dint->?Gstat", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_fstat_f_impl(DeeObject *fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fstat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FSTAT_DEF { "fstat", (DeeObject *)&posix_fstat, MODSYM_FREADONLY, DOC("(fd:?X2?DFile?Dint)->?Gstat") },
#define POSIX_FSTAT_DEF_DOC(doc) { "fstat", (DeeObject *)&posix_fstat, MODSYM_FREADONLY, DOC("(fd:?X2?DFile?Dint)->?Gstat\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fstat, &posix_fstat_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__fd
#define DEFINED_kwlist__fd
PRIVATE DEFINE_KWLIST(kwlist__fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !DEFINED_kwlist__fd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fstat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *fd;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fd, "o:fstat", &args))
		goto err;
	return posix_fstat_f_impl(args.fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_fstat_f_impl(DeeObject *fd)
/*[[[end]]]*/
{
	DREF DeeStatObject *result;
	if unlikely(DeeString_Check(fd)) {
		DeeObject_TypeAssertFailed(fd, &DeeInt_Type);
		goto err;
	}
	result = DeeObject_MALLOC(DeeStatObject);
	if unlikely(!result)
		goto err;
	if unlikely(dee_stat_init(&result->so_stat, NULL, fd, DEE_STAT_F_NORMAL))
		goto err_r;
	DeeObject_Init(result, &DeeStat_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("fstatat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags:c:uint=0->?Gstat", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_fstatat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fstatat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FSTATAT_DEF { "fstatat", (DeeObject *)&posix_fstatat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Gstat") },
#define POSIX_FSTATAT_DEF_DOC(doc) { "fstatat", (DeeObject *)&posix_fstatat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Gstat\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fstatat, &posix_fstatat_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__dfd_path_atflags
#define DEFINED_kwlist__dfd_path_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_path_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_path_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fstatat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *path;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_path_atflags, "oo|u:fstatat", &args))
		goto err;
	return posix_fstatat_f_impl(args.dfd, args.path, args.atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1, 2))DREF DeeObject *DCALL posix_fstatat_f_impl(DeeObject *dfd, DeeObject *path, unsigned int atflags)
/*[[[end]]]*/
{
	DREF DeeStatObject *result;
#ifdef posix_stat_HAVE_lstat
	if (atflags & ~AT_SYMLINK_NOFOLLOW)
#else /* posix_stat_HAVE_lstat */
	if (atflags)
#endif /* !posix_stat_HAVE_lstat */
	{
#ifndef posix_stat_HAVE_lstat
		if (atflags & AT_SYMLINK_NOFOLLOW) {
			err_lstat_not_implemented();
			goto err;
		}
#endif /* !posix_stat_HAVE_lstat */
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid `atflags' argument: %#x",
		                atflags);
		goto err;
	}
	result = DeeObject_MALLOC(DeeStatObject);
	if unlikely(!result)
		goto err;
	if unlikely(dee_stat_init(&result->so_stat, dfd, path, atflags))
		goto err_r;
	DeeObject_Init(result, &DeeStat_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

#ifndef dee_stat_fini_IS_NOOP
#define HAVE_stat_fini
PRIVATE NONNULL((1)) void DCALL
stat_fini(DeeStatObject *__restrict self) {
	dee_stat_fini(&self->so_stat);
}
#endif /* dee_stat_fini_IS_NOOP */



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_dev(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if unlikely(self->so_stat.st_valid & NT_STAT_FNOVOLSERIAL) {
#define NEED_err_stat_no_dev_info
		err_stat_no_dev_info();
		return NULL;
	}
	return DeeInt_NewUInt32((uint32_t)self->so_stat.st_info.dwVolumeSerialNumber);
#elif defined(posix_stat_HAVE_st_dev)
	return DeeInt_NEWU(self->so_stat.st_info.st_dev);
#else /* ... */
#define posix_stat_get_dev_IS_STUB
	(void)self;
#define NEED_err_stat_no_dev_info
	err_stat_no_dev_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ino(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if unlikely(self->so_stat.st_valid & NT_STAT_FNOFILEID) {
#define NEED_err_stat_no_ino_info
		err_stat_no_ino_info();
		return NULL;
	}
	return DeeInt_NewUInt64(((uint64_t)self->so_stat.st_info.nFileIndexHigh << 32) |
	                     ((uint64_t)self->so_stat.st_info.nFileIndexLow));
#elif defined(posix_stat_HAVE_st_ino)
	return DeeInt_NEWU(self->so_stat.st_info.st_ino);
#else /* ... */
#define posix_stat_get_ino_IS_STUB
	(void)self;
#define NEED_err_stat_no_ino_info
	err_stat_no_ino_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_mode(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	uint32_t result = 0444 | 0111; /* XXX: executable should depend on extension. */
	if (!(self->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
		result |= 0222;
	switch (stat_get_nttype(&self->so_stat, true)) {

	case FILE_TYPE_CHAR:
		result |= STAT_IFCHR;
		break;

	case FILE_TYPE_PIPE:
		result |= STAT_IFIFO;
		break;

	case FILE_TYPE_REMOTE:
		result |= STAT_IFSOCK;
		break;

	case FILE_TYPE_DISK:
		/* Actually means a file on-disk when the device flag isn't set. */
		if (self->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) {
			result |= STAT_IFBLK;
			break;
		}
		ATTR_FALLTHROUGH
	default:
		if (self->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
			result |= STAT_IFLNK;
		} else if (self->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			result |= STAT_IFDIR;
		} else {
			result |= STAT_IFREG;
		}
		break;
	}
	return DeeInt_NewUInt32(result);
#elif defined(posix_stat_HAVE_st_mode)
	return DeeInt_NEWU(self->so_stat.st_info.st_mode);
#else /* ... */
#define posix_stat_get_mode_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_nlink(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if unlikely(self->so_stat.st_valid & NT_STAT_FNONLINK)
		goto err_nolink;
	return DeeInt_NewUInt32(self->so_stat.st_info.nNumberOfLinks);
err_nolink:
#define NEED_err_stat_no_link_info
	err_stat_no_link_info();
	return NULL;
#elif defined(posix_stat_HAVE_st_nlink)
	return DeeInt_NEWU(self->so_stat.st_info.st_nlink);
#else /* ... */
#define posix_stat_get_nlink_IS_STUB
	(void)self;
#define NEED_err_stat_no_link_info
	err_stat_no_link_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_uid(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if likely(self->so_stat.st_hand != INVALID_HANDLE_VALUE)
		return nt_GetSecurityInfoOwnerSid(self->so_stat.st_hand, SE_FILE_OBJECT);
#define NEED_nt_GetSecurityInfoOwnerSid
	err_stat_no_uid_info();
#define NEED_err_stat_no_uid_info
	return NULL;
#elif defined(posix_stat_HAVE_st_uid)
	return DeeInt_NEWU(self->so_stat.st_info.st_uid);
#else /* ... */
#define posix_stat_get_uid_IS_STUB
	(void)self;
#define NEED_err_stat_no_uid_info
	err_stat_no_uid_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_gid(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if likely(self->so_stat.st_hand != INVALID_HANDLE_VALUE)
		return nt_GetSecurityInfoGroupSid(self->so_stat.st_hand, SE_FILE_OBJECT);
#define NEED_nt_GetSecurityInfoGroupSid
	err_stat_no_gid_info();
#define NEED_err_stat_no_gid_info
	return NULL;
#elif defined(posix_stat_HAVE_st_gid)
	return DeeInt_NEWU(self->so_stat.st_info.st_gid);
#else /* ... */
#define posix_stat_get_gid_IS_STUB
	(void)self;
#define NEED_err_stat_no_gid_info
	err_stat_no_gid_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_get_rdev(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
#define posix_stat_get_rdev_IS_STUB
	(void)self;
#define NEED_err_stat_no_rdev_info
	err_stat_no_rdev_info();
	return NULL;
#elif defined(posix_stat_HAVE_st_rdev)
	return DeeInt_NEWU(self->so_stat.st_info.st_rdev);
#else /* ... */
#define posix_stat_get_rdev_IS_STUB
	(void)self;
#define NEED_err_stat_no_rdev_info
	err_stat_no_rdev_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_size(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if unlikely(self->so_stat.st_valid & NT_STAT_FNOSIZE)
		goto err_nosize;
	return DeeInt_NewUInt64(((uint64_t)self->so_stat.st_info.nFileSizeHigh << 32) |
	                     ((uint64_t)self->so_stat.st_info.nFileSizeLow));
err_nosize:
#define NEED_err_stat_no_size_info
	err_stat_no_size_info();
	return NULL;
#elif defined(posix_stat_HAVE_st_size)
	return DeeInt_NEWU(self->so_stat.st_info.st_size);
#elif (defined(posix_stat_USE_fopen) && defined(SEEK_END) &&                                           \
       (defined(CONFIG_HAVE_fseeko64) || defined(CONFIG_HAVE_fseeko) || defined(CONFIG_HAVE_fseek)) && \
       (defined(CONFIG_HAVE_ftello64) || defined(CONFIG_HAVE_ftello) || defined(CONFIG_HAVE_ftell)))
#ifdef CONFIG_HAVE_fseeko64
	if (fseeko64(self->so_stat.st_file, 0, SEEK_END) == -1)
#elif defined(CONFIG_HAVE_fseeko)
	if (fseeko(self->so_stat.st_file, 0, SEEK_END) == -1)
#else /* ... */
	if (fseek(self->so_stat.st_file, 0, SEEK_END) == -1)
#endif /* !... */
	{
#ifdef CONFIG_HAVE_ferror
#ifdef CONFIG_HAVE_clearerr
		clearerr(self->so_stat.st_file);
		if (fseeko64(self->so_stat.st_file, 0, SEEK_END) == -1)
#endif /* CONFIG_HAVE_clearerr */
		{
			if (ferror(self->so_stat.st_file))
				goto err_seek_error;
#define NEED_err_seek_error
		}
#endif /* CONFIG_HAVE_ferror */
	}
#ifdef CONFIG_HAVE_ftello64
	return DeeInt_NewUInt64((uint64_t)ftello64(self->so_stat.st_file));
#elif defined(CONFIG_HAVE_ftello)
	return DeeInt_NewUIntptr((uintptr_t)ftello(self->so_stat.st_file));
#else /* ... */
	return DeeInt_NewULong((unsigned long)ftell(self->so_stat.st_file));
#endif /* !... */
#ifdef NEED_err_seek_error
#undef NEED_err_seek_error
err_seek_error:
	DeeError_Throwf(&DeeError_FSError, "Unable to seek file");
	return NULL;
#endif /* NEED_err_seek_error */
#else /* ... */
#define posix_stat_get_size_IS_STUB
	(void)self;
#define NEED_err_stat_no_size_info
	err_stat_no_size_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_blocks(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	uint64_t size;
	if unlikely(self->so_stat.st_valid & NT_STAT_FNOSIZE)
		goto err_nosize;
	size = ((uint64_t)self->so_stat.st_info.nFileSizeHigh << 32) |
	       ((uint64_t)self->so_stat.st_info.nFileSizeLow);
	return DeeInt_NewUInt64(DEFAULT_BLOCKS_FROM_FILESIZE(size));
err_nosize:
#define NEED_err_stat_no_blocks_info
	err_stat_no_blocks_info();
	return NULL;
#elif defined(posix_stat_HAVE_st_blocks)
	return DeeInt_NEWU(self->so_stat.st_info.st_blocks);
#elif defined(posix_stat_HAVE_st_size)
	return DeeInt_NEWU(DEFAULT_BLOCKS_FROM_FILESIZE(self->so_stat.st_info.st_size));
#elif (defined(posix_stat_USE_fopen) && defined(SEEK_END) &&                                           \
       (defined(CONFIG_HAVE_fseeko64) || defined(CONFIG_HAVE_fseeko) || defined(CONFIG_HAVE_fseek)) && \
       (defined(CONFIG_HAVE_ftello64) || defined(CONFIG_HAVE_ftello) || defined(CONFIG_HAVE_ftell)))
#ifdef CONFIG_HAVE_fseeko64
	if (fseeko64(self->so_stat.st_file, 0, SEEK_END) == -1)
#elif defined(CONFIG_HAVE_fseeko)
	if (fseeko(self->so_stat.st_file, 0, SEEK_END) == -1)
#else /* ... */
	if (fseek(self->so_stat.st_file, 0, SEEK_END) == -1)
#endif /* !... */
	{
#ifdef CONFIG_HAVE_ferror
#ifdef CONFIG_HAVE_clearerr
		clearerr(self->so_stat.st_file);
		if (fseeko64(self->so_stat.st_file, 0, SEEK_END) == -1)
#endif /* CONFIG_HAVE_clearerr */
		{
			if (ferror(self->so_stat.st_file))
				goto err_seek_error;
#define NEED_err_seek_error
		}
#endif /* CONFIG_HAVE_ferror */
	}
#ifdef CONFIG_HAVE_ftello64
	return DeeInt_NewUInt64(DEFAULT_BLOCKS_FROM_FILESIZE((uint64_t)ftello64(self->so_stat.st_file)));
#elif defined(CONFIG_HAVE_ftello)
	return DeeInt_NewUIntptr(DEFAULT_BLOCKS_FROM_FILESIZE((uintptr_t)ftello(self->so_stat.st_file)));
#else /* ... */
	return DeeInt_NewULong(DEFAULT_BLOCKS_FROM_FILESIZE((unsigned long)ftell(self->so_stat.st_file)));
#endif /* !... */
#ifdef NEED_err_seek_error
#undef NEED_err_seek_error
err_seek_error:
	DeeError_Throwf(&DeeError_FSError, "Unable to seek file");
	return NULL;
#endif /* NEED_err_seek_error */
#else /* ... */
#define posix_stat_get_blocks_IS_STUB
	(void)self;
#define NEED_err_stat_no_blocks_info
	err_stat_no_blocks_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_blksize(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	(void)self;
	return DeeInt_NewUInt16(DEFAULT_BLOCKSIZE);
#elif defined(posix_stat_HAVE_st_blksize)
	return DeeInt_NEWU(self->so_stat.st_info.st_blksize);
#elif !defined(posix_stat_get_blocks_IS_STUB)
	(void)self;
	return DeeInt_NewUInt16(DEFAULT_BLOCKSIZE);
#else /* ... */
#define posix_stat_get_blksize_IS_STUB
	(void)self;
#define NEED_err_stat_no_blksize_info
	err_stat_no_blksize_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_atime(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if unlikely(self->so_stat.st_valid & NT_STAT_FNOTIME)
		goto err_notime;
#define NEED_DeeTime_NewFILETIME
	return DeeTime_NewFILETIME(&self->so_stat.st_info.ftLastAccessTime);
err_notime:
#define NEED_err_stat_no_atime_info
	err_stat_no_atime_info();
	return NULL;
#elif defined(posix_stat_GET_ATIME_SEC) && defined(posix_stat_GET_ATIME_NSEC)
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_ATIME_SEC(&self->so_stat.st_info),
	                       posix_stat_GET_ATIME_NSEC(&self->so_stat.st_info));
#elif defined(posix_stat_GET_ATIME_SEC)
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_ATIME_SEC(&self->so_stat.st_info), 0);
#else /* ... */
#define posix_stat_get_atime_IS_STUB
	(void)self;
#define NEED_err_stat_no_atime_info
	err_stat_no_atime_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_mtime(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if unlikely(self->so_stat.st_valid & NT_STAT_FNOTIME)
		goto err_notime;
#define NEED_DeeTime_NewFILETIME
	return DeeTime_NewFILETIME(&self->so_stat.st_info.ftLastWriteTime);
err_notime:
#define NEED_err_stat_no_mtime_info
	err_stat_no_mtime_info();
	return NULL;
#elif defined(posix_stat_GET_MTIME_SEC) && defined(posix_stat_GET_MTIME_NSEC)
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_MTIME_SEC(&self->so_stat.st_info),
	                       posix_stat_GET_MTIME_NSEC(&self->so_stat.st_info));
#elif defined(posix_stat_GET_MTIME_SEC)
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_MTIME_SEC(&self->so_stat.st_info), 0);
#else /* ... */
#define posix_stat_get_mtime_IS_STUB
	(void)self;
#define NEED_err_stat_no_mtime_info
	err_stat_no_mtime_info();
	return NULL;
#endif /* !... */
}

#undef stat_get_ctime_IS_stat_get_mtime
#ifdef posix_stat_USE_WINDOWS
/* If the OS doesn't have a dedicated st_ctime timestamp, re-use the st_mtime-one (if present) */
#define stat_get_ctime stat_get_mtime
#define stat_get_ctime_IS_stat_get_mtime
#elif defined(posix_stat_GET_CTIME_SEC) && defined(posix_stat_GET_CTIME_NSEC)
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ctime(DeeStatObject *__restrict self) {
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_CTIME_SEC(&self->so_stat.st_info),
	                       posix_stat_GET_CTIME_NSEC(&self->so_stat.st_info));
}
#elif defined(posix_stat_GET_CTIME_SEC)
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ctime(DeeStatObject *__restrict self) {
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_CTIME_SEC(&self->so_stat.st_info), 0);
}
#elif !defined(posix_stat_get_mtime_IS_STUB)
/* If the OS doesn't have a dedicated st_ctime timestamp, re-use the st_mtime-one (if present) */
#define stat_get_ctime stat_get_mtime
#define stat_get_ctime_IS_stat_get_mtime
#else /* ... */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ctime(DeeStatObject *__restrict self) {
#define posix_stat_get_ctime_IS_STUB
	(void)self;
#define NEED_err_stat_no_ctime_info
	err_stat_no_ctime_info();
	return NULL;
}
#endif /* !... */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_birthtime(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if unlikely(self->so_stat.st_valid & NT_STAT_FNOTIME)
		goto err_notime;
#define NEED_DeeTime_NewFILETIME
	return DeeTime_NewFILETIME(&self->so_stat.st_info.ftCreationTime);
err_notime:
#define NEED_err_stat_no_birthtime_info
	err_stat_no_birthtime_info();
	return NULL;
#elif defined(posix_stat_GET_BTIME_SEC) && defined(posix_stat_GET_BTIME_NSEC)
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_BTIME_SEC(&self->so_stat.st_info),
	                       posix_stat_GET_BTIME_NSEC(&self->so_stat.st_info));
#elif defined(posix_stat_GET_BTIME_SEC)
#define NEED_DeeTime_NewUnix
	return DeeTime_NewUnix(posix_stat_GET_BTIME_SEC(&self->so_stat.st_info), 0);
#else /* ... */
#define posix_stat_get_birthtime_IS_STUB
	(void)self;
#define NEED_err_stat_no_birthtime_info
	err_stat_no_birthtime_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isdir(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	return_bool((self->so_stat.st_info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ==
	/*                                                 */ (FILE_ATTRIBUTE_DIRECTORY));
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISDIR(self->so_stat.st_info.st_mode));
#elif defined(posix_stat_USE_fopen)
	(void)self;
	return_false;
#else /* ... */
#define posix_stat_isdir_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_ischr(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if (!(self->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
		return_false;
	return_bool(stat_get_nttype(&self->so_stat, true) == FILE_TYPE_CHAR);
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISCHR(self->so_stat.st_info.st_mode));
#else /* ... */
#define posix_stat_ischr_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isblk(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	if (!(self->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
		return_false;
	return_bool(stat_get_nttype(&self->so_stat, true) == FILE_TYPE_DISK);
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISBLK(self->so_stat.st_info.st_mode));
#else /* ... */
#define posix_stat_isblk_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isdev(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	DWORD dwType;
	if (!(self->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
		return_false;
	dwType = stat_get_nttype(&self->so_stat, true);
	return_bool(dwType == FILE_TYPE_CHAR ||
	            dwType == FILE_TYPE_DISK);
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISDEV(self->so_stat.st_info.st_mode));
#else /* ... */
#define posix_stat_isdev_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isreg(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	return_bool(!(self->so_stat.st_info.dwFileAttributes &
	              (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY |
	               FILE_ATTRIBUTE_REPARSE_POINT)));
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISREG(self->so_stat.st_info.st_mode));
#elif defined(posix_stat_USE_fopen)
	(void)self;
	return_true; /* ... probably ... */
#else /* ... */
#define posix_stat_isreg_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isfifo(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	return_bool(stat_get_nttype(&self->so_stat, true) == FILE_TYPE_PIPE);
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISFIFO(self->so_stat.st_info.st_mode));
#else /* ... */
#define posix_stat_isfifo_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_islnk(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	return_bool(stat_get_nttype(&self->so_stat, true) == FILE_TYPE_PIPE);
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISLNK(self->so_stat.st_info.st_mode));
#elif !defined(posix_stat_USE_STUB) && !defined(posix_stat_HAVE_lstat)
	/* Without lstat, anything that can be stat'd is guaranted to be regular */
	(void)self;
	return_false;
#else /* ... */
#define posix_stat_islnk_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_issock(DeeStatObject *__restrict self) {
#ifdef posix_stat_USE_WINDOWS
	return_bool(stat_get_nttype(&self->so_stat, true) == FILE_TYPE_REMOTE);
#elif defined(posix_stat_HAVE_st_mode)
	return_bool(STAT_ISSOCK(self->so_stat.st_info.st_mode));
#else /* ... */
#define posix_stat_issock_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}


#ifdef posix_stat_USE_WINDOWS
#define HAVE_stat_getntattr_np
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_getntattr_np(DeeStatObject *__restrict self) {
	return DeeInt_NewUInt32(self->so_stat.st_info.dwFileAttributes);
}

#define HAVE_stat_getnttype_np
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_getnttype_np(DeeStatObject *__restrict self) {
	DWORD result = stat_get_nttype(&self->so_stat, false);
	if unlikely(result == FILE_TYPE_UNKNOWN)
		goto err;
	return DeeInt_NewUInt32(result);
err:
	return NULL;
}
#endif /* posix_stat_USE_WINDOWS */




#define STAT_CLASS_ISLSTAT(self) \
	((self) == (DeeObject *)&DeeLStat_Type)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_exists(DeeObject *self, size_t argc, DeeObject *const *argv) {
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return_true;
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
#if !defined(__OPTIMIZE_SIZE__) && defined(posix_stat_USE_WINDOWS)
		if (argc == 1 && DeeString_Check(argv[0])) {
			DWORD dwAttr; /* Do a quick attribute query. */
			DBG_ALIGNMENT_DISABLE();
#define NEED_nt_GetFileAttributes
			error = nt_GetFileAttributes(argv[0], &dwAttr);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(error < 0)
				goto err;
			return_bool(error == 0);
		}
#endif /* !__OPTIMIZE_SIZE__ && posix_stat_USE_WINDOWS */
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("exists"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	dee_stat_fini(&st);
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isdir(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_isdir((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
#if !defined(__OPTIMIZE_SIZE__) && defined(posix_stat_USE_WINDOWS)
		if (argc == 1 && DeeString_Check(argv[0])) {
			DWORD dwAttr; /* Do a quick attribute query. */
			DBG_ALIGNMENT_DISABLE();
#define NEED_nt_GetFileAttributes
			error = nt_GetFileAttributes(argv[0], &dwAttr);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(error < 0)
				goto err;
			if (error == 0) {
				if (STAT_CLASS_ISLSTAT(self)) {
					/* If the caller used `posix.lstat.isdir()', then don't accept symlink-to-directory */
					return_bool((dwAttr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ==
					/*                 */ (FILE_ATTRIBUTE_DIRECTORY));
				}
				/* In this case, also accept symlink-to-directory */
				return_bool(dwAttr & FILE_ATTRIBUTE_DIRECTORY);
			}
		}
#endif /* !__OPTIMIZE_SIZE__ && posix_stat_USE_WINDOWS */
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("isdir"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_isdir(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_ischr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_ischr((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("ischr"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_ischr(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isblk(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_isblk((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("isblk"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_isblk(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isdev(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_isdev((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("isdev"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_isdev(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isreg(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_isreg((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
#if !defined(__OPTIMIZE_SIZE__) && defined(posix_stat_USE_WINDOWS)
		if (argc == 1 && DeeString_Check(argv[0])) {
			DWORD dwAttr; /* Do a quick attribute query. */
			DBG_ALIGNMENT_DISABLE();
#define NEED_nt_GetFileAttributes
			error = nt_GetFileAttributes(argv[0], &dwAttr);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(error < 0)
				goto err;
			if (error == 0 && (!(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) || STAT_CLASS_ISLSTAT(self))) {
				return_bool(!(dwAttr & (FILE_ATTRIBUTE_DEVICE |
				                        FILE_ATTRIBUTE_DIRECTORY |
				                        FILE_ATTRIBUTE_REPARSE_POINT)));
			}
		}
#endif /* !__OPTIMIZE_SIZE__ && posix_stat_USE_WINDOWS */
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("isreg"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_isreg(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isfifo(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_isfifo((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("isfifo"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_isfifo(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_islnk(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
#ifdef posix_stat_HAVE_lstat
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_islnk((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
#if !defined(__OPTIMIZE_SIZE__) && defined(posix_stat_USE_WINDOWS)
		if (argc == 1 && DeeString_Check(argv[0])) {
			DWORD dwAttr; /* Do a quick attribute query. */
			DBG_ALIGNMENT_DISABLE();
#define NEED_nt_GetFileAttributes
			error = nt_GetFileAttributes(argv[0], &dwAttr);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(error < 0)
				goto err;
			if (error == 0)
				return_bool(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT);
		}
#endif /* !__OPTIMIZE_SIZE__ && posix_stat_USE_WINDOWS */
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("islnk"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		used_atflags |= AT_SYMLINK_NOFOLLOW | DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_islnk(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
#else /* posix_stat_HAVE_lstat */
	(void)argc;
	(void)argv;
	err_lstat_not_implemented();
	return NULL;
#endif /* !posix_stat_HAVE_lstat */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_issock(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return stat_issock((DeeStatObject *)argv[0]);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("issock"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = DeeBool_NewFalse();
	} else {
		result = stat_issock(COMPILER_CONTAINER_OF(&st, DeeStatObject, so_stat));
	}
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSystem_GetFilenameOfHandleOrFdObject(DeeObject *__restrict handle_or_fd) {
#ifdef CONFIG_HOST_WINDOWS
	HANDLE hHandle;
	hHandle = DeeNTSystem_GetHandle(handle_or_fd);
	if unlikely(hHandle == INVALID_HANDLE_VALUE)
		return NULL;
	return DeeNTSystem_GetFilenameOfHandle(hHandle);
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_UNIX
	int os_fd;
	os_fd = DeeUnixSystem_GetFD(handle_or_fd);
	if unlikely(os_fd == -1)
		return NULL;
	return DeeSystem_GetFilenameOfFD(os_fd);
#endif /* CONFIG_HOST_UNIX */
}

#ifndef posix_stat_USE_WINDOWS
PRIVATE WUNUSED NONNULL((1)) bool DCALL
stat_is_unix_hidden_filename(DeeObject *__restrict path) {
	char const *path_str, *path_end;
	path_str = DeeString_STR(path);
	path_end = path_str + WSTR_LENGTH(path_str);
	while (path_end > path_str && !DeeSystem_IsSep(path_end[-1]))
		--path_end;
	return *path_end == '.';
}
#endif /* posix_stat_USE_WINDOWS */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_ishidden(DeeObject *self, size_t argc, DeeObject *const *argv) {
#ifdef posix_stat_USE_WINDOWS
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return_bool(((DeeStatObject *)argv[0])->so_stat.st_info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
#ifndef __OPTIMIZE_SIZE__
		if (argc == 1 && DeeString_Check(argv[0])) {
			DWORD dwAttr; /* Do a quick attribute query. */
			DBG_ALIGNMENT_DISABLE();
#define NEED_nt_GetFileAttributes
			error = nt_GetFileAttributes(argv[0], &dwAttr);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(error < 0)
				goto err;
			if (error == 0 && !(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) || STAT_CLASS_ISLSTAT(self))
				return_bool((dwAttr & FILE_ATTRIBUTE_HIDDEN) != 0);
		}
#endif /* !__OPTIMIZE_SIZE__ */
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("ishidden"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = Dee_False;
	} else {
		result = DeeBool_For(st.st_info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
	}
	Dee_Incref(result);
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
#else /* posix_stat_USE_WINDOWS */
	bool result;
	unsigned int used_atflags;
	DeeObject *dfd, *path_or_file;
	(void)self;
	if (stat_unpack_args(argc, argv, stat_unpack_args_format("ishidden"),
	                     &dfd, &path_or_file, &used_atflags))
		goto err;
	if unlikely(dfd) {
		DREF DeeObject *abs_path;
#define NEED_posix_dfd_makepath
		abs_path = posix_dfd_makepath(dfd, path_or_file, used_atflags);
		if unlikely(!abs_path)
			goto err;
		result = stat_is_unix_hidden_filename(abs_path);
		Dee_Decref(abs_path);
	} else if (DeeString_Check(path_or_file)) {
		result = stat_is_unix_hidden_filename(path_or_file);
	} else {
		DREF DeeObject *abs_path;
		abs_path = DeeSystem_GetFilenameOfHandleOrFdObject(path_or_file);
		if unlikely(!abs_path)
			goto err;
		result = stat_is_unix_hidden_filename(abs_path);
		Dee_Decref(abs_path);
	}
	return_bool(result);
err:
	return NULL;
#endif /* !posix_stat_USE_WINDOWS */
}

#ifdef posix_stat_USE_WINDOWS
#ifndef MEMCASEEQ
#ifdef CONFIG_HAVE_memcasecmp
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#else /* CONFIG_HAVE_memcasecmp */
#define MEMCASEEQ(a, b, s) dee_memcaseeq((uint8_t *)(a), (uint8_t *)(b), s)
LOCAL WUNUSED NONNULL((1, 2)) bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
	while (s--) {
		if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
			return false;
		++a;
		++b;
	}
	return true;
}
#endif /* !CONFIG_HAVE_memcasecmp */
#endif /* !MEMCASEEQ */

/*[[[deemon
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_PATHEXT", "PATHEXT");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_PATHEXT_default", ".COM;.EXE;.BAT;.CMD");
]]]*/
PRIVATE DEFINE_STRING_EX(str_PATHEXT, "PATHEXT", 0xe011e8eb, 0x16a0c5286fc39b1b);
PRIVATE DEFINE_STRING_EX(str_PATHEXT_default, ".COM;.EXE;.BAT;.CMD", 0x4fb60429, 0xd467178bef4dfb48);
/*[[[end]]]*/

/* @return:  1: yes
 * @return:  0: no
 * @return: -1: error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
stat_is_nt_exe_filename(DeeObject *__restrict path) {
	DREF DeeObject *pathext_ob;
	int result;
	char const *ext_begin, *ext_end, *pathext, *path_str;
	size_t ext_size;
	path_str = DeeString_AsUtf8(path);
	if unlikely(!path_str)
		goto err;
	ext_begin = ext_end = path_str + WSTR_LENGTH(path_str);
	for (;;) {
		if (ext_begin == path_str)
			return 0;
		--ext_begin;
		if (*ext_begin == '.')
			break;
		if (*ext_begin == '/' ||
		    *ext_begin == '\\')
			return 0;
	}
	ext_size = (size_t)(ext_end - ext_begin);

	/* Got the file path. */
	pathext_ob = posix_environ_trygetenv((DeeStringObject *)&str_PATHEXT);
	if unlikely(!pathext_ob)
		goto err;
	if (pathext_ob == ITER_DONE) {
		pathext_ob = (DeeObject *)&str_PATHEXT_default;
		Dee_Incref(pathext_ob);
	}
	pathext = DeeString_AsUtf8(pathext_ob);
	if unlikely(!pathext)
		goto err_pathext_ob;

	result = 0;
	while (*pathext) {
		char const *next = strchr(pathext, ';');
		if (!next)
			next = strend(pathext);
		/* Check if this is the extension we've been looking for. */
		if (ext_size == (size_t)(next - pathext) &&
		    MEMCASEEQ(pathext, ext_begin, ext_size * sizeof(char))) {
			result = 1;
			break;
		}
		pathext = next;
		if (*pathext)
			++pathext; /* Skip `;' */
	}
	Dee_Decref(pathext_ob);
	return result;
err_pathext_ob:
	Dee_Decref(pathext_ob);
err:
	return -1;
}
#endif /* posix_stat_USE_WINDOWS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isexe(DeeObject *self, size_t argc, DeeObject *const *argv) {
#ifdef posix_stat_USE_WINDOWS
	int result;
	unsigned int used_atflags;
	DeeObject *dfd, *path_or_file;
	(void)self;
	if (stat_unpack_args(argc, argv, stat_unpack_args_format("isexe"),
	                     &dfd, &path_or_file, &used_atflags))
		goto err;
	if unlikely(dfd) {
		DREF DeeObject *abs_path;
#define NEED_posix_dfd_makepath
		abs_path = posix_dfd_makepath(dfd, path_or_file, used_atflags);
		if unlikely(!abs_path)
			goto err;
		result = stat_is_nt_exe_filename(abs_path);
		Dee_Decref(abs_path);
	} else if (DeeString_Check(path_or_file)) {
		result = stat_is_nt_exe_filename(path_or_file);
	} else {
		DREF DeeObject *abs_path;
		abs_path = DeeSystem_GetFilenameOfHandleOrFdObject(path_or_file);
		if unlikely(!abs_path)
			goto err;
		result = stat_is_nt_exe_filename(abs_path);
		Dee_Decref(abs_path);
	}
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
#elif defined(posix_stat_USED_STRUCT_STAT)
	DREF DeeObject *result;
	struct dee_stat st;
	int error;
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeStat_Type)) {
		return_bool((((DeeStatObject *)argv[0])->so_stat.st_info.st_mode & 0111) != 0);
	} else {
		unsigned int used_atflags;
		DeeObject *dfd, *path_or_file;
		if (stat_unpack_args(argc, argv, stat_unpack_args_format("isexe"),
		                     &dfd, &path_or_file, &used_atflags))
			goto err;
		if (STAT_CLASS_ISLSTAT(self)) {
#ifdef posix_stat_HAVE_lstat
			used_atflags |= AT_SYMLINK_NOFOLLOW;
#else /* posix_stat_HAVE_lstat */
			err_lstat_not_implemented();
			goto err;
#endif /* !posix_stat_HAVE_lstat */
		}
		used_atflags |= DEE_STAT_F_TRY;
		error = dee_stat_init(&st, dfd, path_or_file, used_atflags);
	}
	if unlikely(error < 0)
		goto err;
	if (error > 0) {
		result = Dee_False;
	} else {
		result = DeeBool_For((st.st_info.st_mode & 0111) != 0);
	}
	Dee_Incref(result);
	dee_stat_fini(&st);
	return result;
err:
	return NULL;
#else /* ... */
#define stat_class_isexe_IS_STUB
	(void)self;
#define NEED_err_stat_no_mode_info
	err_stat_no_mode_info();
	return NULL;
#endif /* !... */
}





PRIVATE struct type_getset tpconst stat_getsets[] = {
	TYPE_GETTER_F("st_dev", &stat_get_dev, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{@this stat-file does not contain valid device information}"
	              "Return the device number of the storage device on which the stat-file is located"),
	TYPE_GETTER_F("st_ino", &stat_get_ino, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{@this stat-file does not contain valid inode information}"
	              "Returns the inode number or file-id of the stat-file"),
	TYPE_GETTER_F("st_mode", &stat_get_mode, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns a bitset describing the access permissions and mode of the stat-file. "
	              "For more information, see ?GS_IFMT"),
	TYPE_GETTER_F("st_nlink", &stat_get_nlink, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns the number of existing hard-links to this stat-file"),
	TYPE_GETTER_F("st_uid", &stat_get_uid, METHOD_FNOREFESCAPE,
	              "->?Guser\n"
	              "Returns a descriptor for the user owning this file"),
	TYPE_GETTER_F("st_gid", &stat_get_gid, METHOD_FNOREFESCAPE,
	              "->?Ggroup\n"
	              "Returns a descriptor for the group owning this file"),
	TYPE_GETTER_F("st_rdev", &stat_get_rdev, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{@this stat-file does not contain valid r-dev information}"
	              "Returns the device ID of the character/block device described by this stat-file"),
	TYPE_GETTER_F("st_size", &stat_get_size, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{@this stat-file does not contain valid size information}"
	              "Returns the size of the stat-file in bytes"),
	TYPE_GETTER_F("st_blocks", &stat_get_blocks, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{@this stat-file does not contain valid block-count information}"
	              "Returns the number of filesystem blocks used by the stat-file"),
	TYPE_GETTER_F("st_blksize", &stat_get_blksize, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{@this stat-file does not contain valid block-count information}"
	              "Returns the size of a filesystem blocks, as used by the stat-file"),
	TYPE_GETTER_F("st_atime", &stat_get_atime, METHOD_FNOREFESCAPE,
	              "->?Etime:Time\n"
	              "#tValueError{@this stat-file does not contain valid time information}"
	              "Return the last-accessed time of the stat-file"),
	TYPE_GETTER_F("st_mtime", &stat_get_mtime, METHOD_FNOREFESCAPE,
	              "->?Etime:Time\n"
	              "#tValueError{@this stat-file does not contain valid time information}"
	              "Return the last-modified (file content only) time of the stat-file"),
	TYPE_GETTER_F("st_ctime", &stat_get_ctime, METHOD_FNOREFESCAPE,
	              "->?Etime:Time\n"
	              "#tValueError{@this stat-file does not contain valid time information}"
	              "Return the lsat-changed (file content & file attributes) time of the stat-file"),
	TYPE_GETTER_F("st_birthtime", &stat_get_birthtime, METHOD_FNOREFESCAPE,
	              "->?Etime:Time\n"
	              "#tValueError{@this stat-file does not contain valid time information}"
	              "Return the creation (birth) time of the stat-file"),
	TYPE_GETTER_F("isdir", &stat_isdir, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a directory"),
	TYPE_GETTER_F("ischr", &stat_ischr, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a character-device"),
	TYPE_GETTER_F("isblk", &stat_isblk, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a block-device"),
	TYPE_GETTER_F("isdev", &stat_isdev, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a character- or block-device"),
	TYPE_GETTER_F("isreg", &stat_isreg, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a regular file"),
	TYPE_GETTER_F("isfifo", &stat_isfifo, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a pipe"),
	TYPE_GETTER_F("islnk", &stat_islnk, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a symbolic link"),
	TYPE_GETTER_F("issock", &stat_issock, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "Check if @this stat-file refers to a socket"),

	/* Non-portable NT extensions. */
#ifdef HAVE_stat_getntattr_np
	TYPE_GETTER_F("ntattr_np", &stat_getntattr_np, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Non-portable windows extension for retrieving the NT attributes of the stat-file, those "
	              /**/ "attributes being a set of the `FILE_ATTRIBUTE_*' constants found in windows system headers"),
#endif /* HAVE_stat_getntattr_np */
#ifdef HAVE_stat_getnttype_np
	TYPE_GETTER_F("nttype_np", &stat_getnttype_np, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{@this stat-file does not contain valid NT-type information}"
	              "Non-portable windows extension for retrieving the NT type of this stat-file, that "
	              /**/ "type being one of the `FILE_TYPE_*' constants found in windows system headers"),
#endif /* HAVE_stat_getnttype_np */
	TYPE_GETSET_END
};


PRIVATE struct type_method tpconst stat_class_methods[] = {
	TYPE_METHOD("exists", &stat_class_exists,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the referred file exists, or if the given "
	            /**/ "file described can be used with ?Gstat"),
	TYPE_METHOD("isdir", &stat_class_isdir,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing directory"),
	TYPE_METHOD("ischr", &stat_class_ischr,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing character-device"),
	TYPE_METHOD("isblk", &stat_class_isblk,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing block-device"),
	TYPE_METHOD("isdev", &stat_class_isdev,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing character- or block-device"),
	TYPE_METHOD("isreg", &stat_class_isreg,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing regular file"),
	TYPE_METHOD("isfifo", &stat_class_isfifo,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing pipe"),
	TYPE_METHOD("islnk", &stat_class_islnk,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing symbolic link"),
	TYPE_METHOD("issock", &stat_class_issock,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an existing socket"),
	TYPE_METHOD("ishidden", &stat_class_ishidden,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of "
	            /**/ "?Gstat, check if the passed parameters refer to a hidden file. "
	            /**/ "If the filesystem encodes the hidden-attribute as part of the "
	            /**/ "filename, this function always returns ?f if the path-string "
	            /**/ "of the file described by the passed arguments cannot be determined"),
	TYPE_METHOD("isexe", &stat_class_isexe,
	            "(st:?.)->?Dbool\n"
	            "(path:?Dstring,atflags=!0)->?Dbool\n"
	            "(fp:?DFile)->?Dbool\n"
	            "(fd:?Dint)->?Dbool\n"
	            "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)->?Dbool\n"
	            "#t{:Interrupt}"
	            "Taking the same arguments as the constructor of ?Gstat, "
	            /**/ "check if the passed parameters refer to an executable file. "
	            /**/ "If the filesystem encodes the executable-attribute as part of the "
	            /**/ "filename, this function always returns ?f if the path-string "
	            /**/ "of the file described by the passed arguments cannot be determined"),
	TYPE_METHOD_END
};


INTERN DeeTypeObject DeeStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "stat",
	/* .tp_doc      = */ DOC("(path:?Dstring,atflags=!0)\n"
	                         "(fp:?DFile)\n"
	                         "(fd:?Dint)\n"
	                         "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,atflags=!0)\n"
	                         "#t{:Interrupt}"
	                         "#tFileNotFound{The given @path or @fp could not be found}"
	                         "#tSystemError{Failed to query file information for some reason}"
	                         "#tValueError{Invalid @atflags argument}"
	                         "Query information on a given @path, file stream @fp "
	                         /**/ "or file descriptor @fd (if supported by the host)\n"
	                         "If you wish to test the existing and type of a type, "
	                         /**/ "consider using stat's class methods such as ?#{isdir}. "
	                         /**/ "Note however that stat instances also implement these "
	                         /**/ "methods as general purpose property checks that do not "
	                         /**/ "require calculation of ?#st_mode"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&stat_init,
				TYPE_FIXED_ALLOCATOR(DeeStatObject)
			}
		},
#ifdef HAVE_stat_fini
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&stat_fini,
#else /* HAVE_stat_fini */
		/* .tp_dtor        = */ NULL,
#endif /* !HAVE_stat_fini */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ stat_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ stat_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTERN DeeTypeObject DeeLStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "lstat",
	/* .tp_doc      = */ DOC("(path:?Dstring)\n"
	                         "(dfd:?X3?DFile?Dint?Dstring,path:?Dstring)\n"
	                         "#t{:Interrupt}"
	                         "#tFileNotFound{The given @path or @fp could not be found}"
	                         "#tSystemError{Failed to query file information for some reason}"
	                         "Same as its base type ?Gstat, but query information without "
	                         /**/ "dereferencing the final link"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeStat_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&lstat_init,
				TYPE_FIXED_ALLOCATOR(DeeStatObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_POSIX_P_STAT_C_INL */
