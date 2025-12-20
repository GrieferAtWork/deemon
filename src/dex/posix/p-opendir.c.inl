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
#ifndef GUARD_DEX_POSIX_P_OPENDIR_C_INL
#define GUARD_DEX_POSIX_P_OPENDIR_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/abi/time.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintN_t */

/* Figure out how to implement `opendir()' */
#undef posix_opendir_USE_FindFirstFileExW
#undef posix_opendir_USE_opendir
#undef posix_opendir_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define posix_opendir_USE_FindFirstFileExW
/* TODO: Add another option to implement using `_findfirst()' */
#elif defined(CONFIG_HAVE_opendir) && (defined(CONFIG_HAVE_readdir) || defined(CONFIG_HAVE_readdir64))
#define posix_opendir_USE_opendir
#else /* ... */
#define posix_opendir_USE_STUB
#endif /* !... */

#if (!defined(CONFIG_HAVE_struct_dirent_d_ino) && defined(CONFIG_HAVE_struct_dirent_d_fileno))
#define CONFIG_HAVE_struct_dirent_d_ino
#define d_ino d_fileno
#endif /* d_ino = d_fileno... */


#undef posix_opendir_NEED_STAT_EXTENSION
#ifdef posix_opendir_USE_opendir
#undef DIR_dirent
#undef DIR_readdir
#ifdef CONFIG_HAVE_readdir64
#define DIR_dirent  dirent64
#define DIR_readdir readdir64
#else /* CONFIG_HAVE_readdir64 */
#define DIR_dirent  dirent
#define DIR_readdir readdir
#endif /* !CONFIG_HAVE_readdir64 */
#endif /* posix_opendir_USE_opendir */

#ifdef posix_opendir_USE_FindFirstFileExW
#define posix_opendir_NEED_STAT_EXTENSION
#endif /* posix_opendir_USE_FindFirstFileExW */

/* Figure out how we can best implement lstat() */
#ifdef posix_opendir_NEED_STAT_EXTENSION
#ifdef posix_opendir_USE_FindFirstFileExW
#define DIR_struct_stat BY_HANDLE_FILE_INFORMATION
#define DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION
#elif defined(CONFIG_HAVE_fstatat64)
#define DIR_lstatat(dfd, filename, info) fstatat64(dfd, filename, info, AT_SYMLINK_NOFOLLOW)
#define DIR_struct_stat                  struct stat64
#elif defined(CONFIG_HAVE_fstatat)
#define DIR_lstatat(dfd, filename, info) fstatat(dfd, filename, info, AT_SYMLINK_NOFOLLOW)
#define DIR_struct_stat                  struct stat
#elif defined(CONFIG_HAVE_lstat64)
#define DIR_lstat       lstat64
#define DIR_struct_stat struct stat64
#define DIR_struct_stat_IS_stat64
#elif defined(CONFIG_HAVE_lstat)
#define DIR_lstat       lstat
#define DIR_struct_stat struct stat
#define DIR_struct_stat_IS_stat
#elif defined(CONFIG_HAVE_stat64)
#define DIR_lstat       stat64
#define DIR_struct_stat struct stat64
#define DIR_struct_stat_IS_stat64
#elif defined(CONFIG_HAVE_stat)
#define DIR_lstat       stat
#define DIR_struct_stat struct stat
#define DIR_struct_stat_IS_stat
#else /* ... */
#undef posix_opendir_NEED_STAT_EXTENSION
#endif /* !... */
#endif /* posix_opendir_NEED_STAT_EXTENSION */

#if defined(DIR_lstat) && defined(CONFIG_HAVE_AT_FDCWD) && !defined(DIR_lstatat)
#define DIR_lstat(filename, info) DIR_lstatat(AT_FDCWD, filename, info)
#endif /* DIR_lstat && CONFIG_HAVE_AT_FDCWD && !DIR_lstatat */

#undef DIR_stat_HAVE_st_dev
#undef DIR_stat_HAVE_st_ino
#undef DIR_stat_HAVE_st_mode
#undef DIR_stat_HAVE_st_nlink
#undef DIR_stat_HAVE_st_uid
#undef DIR_stat_HAVE_st_gid
#undef DIR_stat_HAVE_st_rdev
#undef DIR_stat_HAVE_st_size
#undef DIR_stat_HAVE_st_blksize
#undef DIR_stat_HAVE_st_blocks
#undef DIR_stat_HAVE_st_atime
#undef DIR_stat_HAVE_st_atim
#undef DIR_stat_HAVE_st_atimespec
#undef DIR_stat_HAVE_st_atimensec
#undef DIR_stat_HAVE_st_mtime
#undef DIR_stat_HAVE_st_mtim
#undef DIR_stat_HAVE_st_mtimespec
#undef DIR_stat_HAVE_st_mtimensec
#undef DIR_stat_HAVE_st_ctime
#undef DIR_stat_HAVE_st_ctim
#undef DIR_stat_HAVE_st_ctimespec
#undef DIR_stat_HAVE_st_ctimensec
#undef DIR_stat_HAVE_st_btime
#undef DIR_stat_HAVE_st_btim
#undef DIR_stat_HAVE_st_btimespec
#undef DIR_stat_HAVE_st_btimensec
#undef DIR_stat_HAVE_st_birthtime
#undef DIR_stat_HAVE_st_birthtim
#undef DIR_stat_HAVE_st_birthtimespec
#undef DIR_stat_HAVE_st_birthtimensec
#ifdef DIR_struct_stat_IS_stat
#ifdef CONFIG_HAVE_struct_stat_st_dev
#define DIR_stat_HAVE_st_dev
#endif /* CONFIG_HAVE_struct_stat_st_dev */
#ifdef CONFIG_HAVE_struct_stat_st_ino
#define DIR_stat_HAVE_st_ino
#endif /* CONFIG_HAVE_struct_stat_st_ino */
#ifdef CONFIG_HAVE_struct_stat_st_mode
#define DIR_stat_HAVE_st_mode
#endif /* CONFIG_HAVE_struct_stat_st_mode */
#ifdef CONFIG_HAVE_struct_stat_st_nlink
#define DIR_stat_HAVE_st_nlink
#endif /* CONFIG_HAVE_struct_stat_st_nlink */
#ifdef CONFIG_HAVE_struct_stat_st_uid
#define DIR_stat_HAVE_st_uid
#endif /* CONFIG_HAVE_struct_stat_st_uid */
#ifdef CONFIG_HAVE_struct_stat_st_gid
#define DIR_stat_HAVE_st_gid
#endif /* CONFIG_HAVE_struct_stat_st_gid */
#ifdef CONFIG_HAVE_struct_stat_st_rdev
#define DIR_stat_HAVE_st_rdev
#endif /* CONFIG_HAVE_struct_stat_st_rdev */
#ifdef CONFIG_HAVE_struct_stat_st_size
#define DIR_stat_HAVE_st_size
#endif /* CONFIG_HAVE_struct_stat_st_size */
#ifdef CONFIG_HAVE_struct_stat_st_blksize
#define DIR_stat_HAVE_st_blksize
#endif /* CONFIG_HAVE_struct_stat_st_blksize */
#ifdef CONFIG_HAVE_struct_stat_st_blocks
#define DIR_stat_HAVE_st_blocks
#endif /* CONFIG_HAVE_struct_stat_st_blocks */
#ifdef CONFIG_HAVE_struct_stat_st_atime
#define DIR_stat_HAVE_st_atime
#endif /* CONFIG_HAVE_struct_stat_st_atime */
#ifdef CONFIG_HAVE_struct_stat_st_atim
#define DIR_stat_HAVE_st_atim
#endif /* CONFIG_HAVE_struct_stat_st_atim */
#ifdef CONFIG_HAVE_struct_stat_st_atimespec
#define DIR_stat_HAVE_st_atimespec
#endif /* CONFIG_HAVE_struct_stat_st_atimespec */
#ifdef CONFIG_HAVE_struct_stat_st_atimensec
#define DIR_stat_HAVE_st_atimensec
#endif /* CONFIG_HAVE_struct_stat_st_atimensec */
#ifdef CONFIG_HAVE_struct_stat_st_mtime
#define DIR_stat_HAVE_st_mtime
#endif /* CONFIG_HAVE_struct_stat_st_mtime */
#ifdef CONFIG_HAVE_struct_stat_st_mtim
#define DIR_stat_HAVE_st_mtim
#endif /* CONFIG_HAVE_struct_stat_st_mtim */
#ifdef CONFIG_HAVE_struct_stat_st_mtimespec
#define DIR_stat_HAVE_st_mtimespec
#endif /* CONFIG_HAVE_struct_stat_st_mtimespec */
#ifdef CONFIG_HAVE_struct_stat_st_mtimensec
#define DIR_stat_HAVE_st_mtimensec
#endif /* CONFIG_HAVE_struct_stat_st_mtimensec */
#ifdef CONFIG_HAVE_struct_stat_st_ctime
#define DIR_stat_HAVE_st_ctime
#endif /* CONFIG_HAVE_struct_stat_st_ctime */
#ifdef CONFIG_HAVE_struct_stat_st_ctim
#define DIR_stat_HAVE_st_ctim
#endif /* CONFIG_HAVE_struct_stat_st_ctim */
#ifdef CONFIG_HAVE_struct_stat_st_ctimespec
#define DIR_stat_HAVE_st_ctimespec
#endif /* CONFIG_HAVE_struct_stat_st_ctimespec */
#ifdef CONFIG_HAVE_struct_stat_st_ctimensec
#define DIR_stat_HAVE_st_ctimensec
#endif /* CONFIG_HAVE_struct_stat_st_ctimensec */
#ifdef CONFIG_HAVE_struct_stat_st_btime
#define DIR_stat_HAVE_st_btime
#endif /* CONFIG_HAVE_struct_stat_st_btime */
#ifdef CONFIG_HAVE_struct_stat_st_btim
#define DIR_stat_HAVE_st_btim
#endif /* CONFIG_HAVE_struct_stat_st_btim */
#ifdef CONFIG_HAVE_struct_stat_st_btimespec
#define DIR_stat_HAVE_st_btimespec
#endif /* CONFIG_HAVE_struct_stat_st_btimespec */
#ifdef CONFIG_HAVE_struct_stat_st_btimensec
#define DIR_stat_HAVE_st_btimensec
#endif /* CONFIG_HAVE_struct_stat_st_btimensec */
#ifdef CONFIG_HAVE_struct_stat_st_birthtime
#define DIR_stat_HAVE_st_birthtime
#endif /* CONFIG_HAVE_struct_stat_st_birthtime */
#ifdef CONFIG_HAVE_struct_stat_st_birthtim
#define DIR_stat_HAVE_st_birthtim
#endif /* CONFIG_HAVE_struct_stat_st_birthtim */
#ifdef CONFIG_HAVE_struct_stat_st_birthtimespec
#define DIR_stat_HAVE_st_birthtimespec
#endif /* CONFIG_HAVE_struct_stat_st_birthtimespec */
#ifdef CONFIG_HAVE_struct_stat_st_birthtimensec
#define DIR_stat_HAVE_st_birthtimensec
#endif /* CONFIG_HAVE_struct_stat_st_birthtimensec */
#endif /* DIR_struct_stat_IS_stat */
#ifdef DIR_struct_stat_IS_stat64
#ifdef CONFIG_HAVE_struct_stat64_st_dev
#define DIR_stat_HAVE_st_dev
#endif /* CONFIG_HAVE_struct_stat64_st_dev */
#ifdef CONFIG_HAVE_struct_stat64_st_ino
#define DIR_stat_HAVE_st_ino
#endif /* CONFIG_HAVE_struct_stat64_st_ino */
#ifdef CONFIG_HAVE_struct_stat64_st_mode
#define DIR_stat_HAVE_st_mode
#endif /* CONFIG_HAVE_struct_stat64_st_mode */
#ifdef CONFIG_HAVE_struct_stat64_st_nlink
#define DIR_stat_HAVE_st_nlink
#endif /* CONFIG_HAVE_struct_stat64_st_nlink */
#ifdef CONFIG_HAVE_struct_stat64_st_uid
#define DIR_stat_HAVE_st_uid
#endif /* CONFIG_HAVE_struct_stat64_st_uid */
#ifdef CONFIG_HAVE_struct_stat64_st_gid
#define DIR_stat_HAVE_st_gid
#endif /* CONFIG_HAVE_struct_stat64_st_gid */
#ifdef CONFIG_HAVE_struct_stat64_st_rdev
#define DIR_stat_HAVE_st_rdev
#endif /* CONFIG_HAVE_struct_stat64_st_rdev */
#ifdef CONFIG_HAVE_struct_stat64_st_size
#define DIR_stat_HAVE_st_size
#endif /* CONFIG_HAVE_struct_stat64_st_size */
#ifdef CONFIG_HAVE_struct_stat64_st_blksize
#define DIR_stat_HAVE_st_blksize
#endif /* CONFIG_HAVE_struct_stat64_st_blksize */
#ifdef CONFIG_HAVE_struct_stat64_st_blocks
#define DIR_stat_HAVE_st_blocks
#endif /* CONFIG_HAVE_struct_stat64_st_blocks */
#ifdef CONFIG_HAVE_struct_stat64_st_atime
#define DIR_stat_HAVE_st_atime
#endif /* CONFIG_HAVE_struct_stat64_st_atime */
#ifdef CONFIG_HAVE_struct_stat64_st_atim
#define DIR_stat_HAVE_st_atim
#endif /* CONFIG_HAVE_struct_stat64_st_atim */
#ifdef CONFIG_HAVE_struct_stat64_st_atimespec
#define DIR_stat_HAVE_st_atimespec
#endif /* CONFIG_HAVE_struct_stat64_st_atimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_atimensec
#define DIR_stat_HAVE_st_atimensec
#endif /* CONFIG_HAVE_struct_stat64_st_atimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_mtime
#define DIR_stat_HAVE_st_mtime
#endif /* CONFIG_HAVE_struct_stat64_st_mtime */
#ifdef CONFIG_HAVE_struct_stat64_st_mtim
#define DIR_stat_HAVE_st_mtim
#endif /* CONFIG_HAVE_struct_stat64_st_mtim */
#ifdef CONFIG_HAVE_struct_stat64_st_mtimespec
#define DIR_stat_HAVE_st_mtimespec
#endif /* CONFIG_HAVE_struct_stat64_st_mtimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_mtimensec
#define DIR_stat_HAVE_st_mtimensec
#endif /* CONFIG_HAVE_struct_stat64_st_mtimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_ctime
#define DIR_stat_HAVE_st_ctime
#endif /* CONFIG_HAVE_struct_stat64_st_ctime */
#ifdef CONFIG_HAVE_struct_stat64_st_ctim
#define DIR_stat_HAVE_st_ctim
#endif /* CONFIG_HAVE_struct_stat64_st_ctim */
#ifdef CONFIG_HAVE_struct_stat64_st_ctimespec
#define DIR_stat_HAVE_st_ctimespec
#endif /* CONFIG_HAVE_struct_stat64_st_ctimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_ctimensec
#define DIR_stat_HAVE_st_ctimensec
#endif /* CONFIG_HAVE_struct_stat64_st_ctimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_btime
#define DIR_stat_HAVE_st_btime
#endif /* CONFIG_HAVE_struct_stat64_st_btime */
#ifdef CONFIG_HAVE_struct_stat64_st_btim
#define DIR_stat_HAVE_st_btim
#endif /* CONFIG_HAVE_struct_stat64_st_btim */
#ifdef CONFIG_HAVE_struct_stat64_st_btimespec
#define DIR_stat_HAVE_st_btimespec
#endif /* CONFIG_HAVE_struct_stat64_st_btimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_btimensec
#define DIR_stat_HAVE_st_btimensec
#endif /* CONFIG_HAVE_struct_stat64_st_btimensec */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtime
#define DIR_stat_HAVE_st_birthtime
#endif /* CONFIG_HAVE_struct_stat64_st_birthtime */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtim
#define DIR_stat_HAVE_st_birthtim
#endif /* CONFIG_HAVE_struct_stat64_st_birthtim */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtimespec
#define DIR_stat_HAVE_st_birthtimespec
#endif /* CONFIG_HAVE_struct_stat64_st_birthtimespec */
#ifdef CONFIG_HAVE_struct_stat64_st_birthtimensec
#define DIR_stat_HAVE_st_birthtimensec
#endif /* CONFIG_HAVE_struct_stat64_st_birthtimensec */
#endif /* DIR_struct_stat_IS_stat64 */

/* Helper macros to accessing timestamp data */
#undef DIR_stat_GET_RAW_ATIME_SEC
#undef DIR_stat_GET_RAW_ATIME_NSEC
#undef DIR_stat_GET_RAW_MTIME_SEC
#undef DIR_stat_GET_RAW_MTIME_NSEC
#undef DIR_stat_GET_RAW_CTIME_SEC
#undef DIR_stat_GET_RAW_CTIME_NSEC
#undef DIR_stat_GET_RAW_BTIME_SEC
#undef DIR_stat_GET_RAW_BTIME_NSEC
#ifdef DIR_stat_HAVE_st_atimespec
#define DIR_stat_GET_RAW_ATIME_SEC(st)  (st)->st_atimespec.tv_sec
#define DIR_stat_GET_RAW_ATIME_NSEC(st) (st)->st_atimespec.tv_nsec
#elif defined(DIR_stat_HAVE_st_atim)
#define DIR_stat_GET_RAW_ATIME_SEC(st)  (st)->st_atim.tv_sec
#define DIR_stat_GET_RAW_ATIME_NSEC(st) (st)->st_atim.tv_nsec
#else /* ... */
#ifdef DIR_stat_HAVE_st_atime
#define DIR_stat_GET_RAW_ATIME_SEC(st)  (st)->st_atime
#endif /* DIR_stat_HAVE_st_atime */
#ifdef DIR_stat_HAVE_st_atimensec
#define DIR_stat_GET_RAW_ATIME_NSEC(st) (st)->st_atimensec
#endif /* DIR_stat_HAVE_st_atimensec */
#endif /* !... */

#ifdef DIR_stat_HAVE_st_mtimespec
#define DIR_stat_GET_RAW_MTIME_SEC(st)  (st)->st_mtimespec.tv_sec
#define DIR_stat_GET_RAW_MTIME_NSEC(st) (st)->st_mtimespec.tv_nsec
#elif defined(DIR_stat_HAVE_st_mtim)
#define DIR_stat_GET_RAW_MTIME_SEC(st)  (st)->st_mtim.tv_sec
#define DIR_stat_GET_RAW_MTIME_NSEC(st) (st)->st_mtim.tv_nsec
#else /* ... */
#ifdef DIR_stat_HAVE_st_mtime
#define DIR_stat_GET_RAW_MTIME_SEC(st)  (st)->st_mtime
#endif /* DIR_stat_HAVE_st_mtime */
#ifdef DIR_stat_HAVE_st_mtimensec
#define DIR_stat_GET_RAW_MTIME_NSEC(st) (st)->st_mtimensec
#endif /* DIR_stat_HAVE_st_mtimensec */
#endif /* !... */

#ifdef DIR_stat_HAVE_st_ctimespec
#define DIR_stat_GET_RAW_CTIME_SEC(st)  (st)->st_ctimespec.tv_sec
#define DIR_stat_GET_RAW_CTIME_NSEC(st) (st)->st_ctimespec.tv_nsec
#elif defined(DIR_stat_HAVE_st_ctim)
#define DIR_stat_GET_RAW_CTIME_SEC(st)  (st)->st_ctim.tv_sec
#define DIR_stat_GET_RAW_CTIME_NSEC(st) (st)->st_ctim.tv_nsec
#else /* ... */
#ifdef DIR_stat_HAVE_st_ctime
#define DIR_stat_GET_RAW_CTIME_SEC(st)  (st)->st_ctime
#endif /* DIR_stat_HAVE_st_ctime */
#ifdef DIR_stat_HAVE_st_ctimensec
#define DIR_stat_GET_RAW_CTIME_NSEC(st) (st)->st_ctimensec
#endif /* DIR_stat_HAVE_st_ctimensec */
#endif /* !... */

#ifdef DIR_stat_HAVE_st_btimespec
#define DIR_stat_GET_RAW_BTIME_SEC(st)  (st)->st_btimespec.tv_sec
#define DIR_stat_GET_RAW_BTIME_NSEC(st) (st)->st_btimespec.tv_nsec
#elif defined(DIR_stat_HAVE_st_btim)
#define DIR_stat_GET_RAW_BTIME_SEC(st)  (st)->st_btim.tv_sec
#define DIR_stat_GET_RAW_BTIME_NSEC(st) (st)->st_btim.tv_nsec
#elif defined(DIR_stat_HAVE_st_birthtimespec)
#define DIR_stat_GET_RAW_BTIME_SEC(st)  (st)->st_birthtimespec.tv_sec
#define DIR_stat_GET_RAW_BTIME_NSEC(st) (st)->st_birthtimespec.tv_nsec
#elif defined(DIR_stat_HAVE_st_birthtim)
#define DIR_stat_GET_RAW_BTIME_SEC(st)  (st)->st_birthtim.tv_sec
#define DIR_stat_GET_RAW_BTIME_NSEC(st) (st)->st_birthtim.tv_nsec
#else /* ... */
#ifdef DIR_stat_HAVE_st_btime
#define DIR_stat_GET_RAW_BTIME_SEC(st)  (st)->st_btime
#elif defined(DIR_stat_HAVE_st_birthtime)
#define DIR_stat_GET_RAW_BTIME_SEC(st)  (st)->st_birthtime
#endif /* ... */
#ifdef DIR_stat_HAVE_st_btimensec
#define DIR_stat_GET_RAW_BTIME_NSEC(st) (st)->st_btimensec
#elif defined(DIR_stat_HAVE_st_birthtimensec)
#define DIR_stat_GET_RAW_BTIME_NSEC(st) (st)->st_birthtimensec
#endif /* ... */
#endif /* !... */

#undef DIR_stat_GET_ATIME_SEC
#undef DIR_stat_GET_ATIME_NSEC
#undef DIR_stat_GET_MTIME_SEC
#undef DIR_stat_GET_MTIME_NSEC
#undef DIR_stat_GET_CTIME_SEC
#undef DIR_stat_GET_CTIME_NSEC
#undef DIR_stat_GET_BTIME_SEC
#undef DIR_stat_GET_BTIME_NSEC
#ifdef DIR_stat_GET_RAW_ATIME_SEC
#define DIR_stat_GET_ATIME_SEC DIR_stat_GET_RAW_ATIME_SEC
#endif /* DIR_stat_GET_RAW_ATIME_SEC */
#ifdef DIR_stat_GET_RAW_ATIME_NSEC
#define DIR_stat_GET_ATIME_NSEC DIR_stat_GET_RAW_ATIME_NSEC
#endif /* DIR_stat_GET_RAW_ATIME_NSEC */

#ifdef DIR_stat_GET_RAW_MTIME_SEC
#define DIR_stat_GET_MTIME_SEC DIR_stat_GET_RAW_MTIME_SEC
#endif /* DIR_stat_GET_RAW_MTIME_SEC */
#ifdef DIR_stat_GET_RAW_MTIME_NSEC
#define DIR_stat_GET_MTIME_NSEC DIR_stat_GET_RAW_MTIME_NSEC
#endif /* DIR_stat_GET_RAW_MTIME_NSEC */

#ifdef CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME
#ifdef DIR_stat_GET_RAW_CTIME_SEC
#define DIR_stat_GET_BTIME_SEC DIR_stat_GET_RAW_CTIME_SEC
#endif /* DIR_stat_GET_RAW_CTIME_SEC */
#ifdef DIR_stat_GET_RAW_CTIME_NSEC
#define DIR_stat_GET_BTIME_NSEC DIR_stat_GET_RAW_CTIME_NSEC
#endif /* DIR_stat_GET_RAW_CTIME_NSEC */
#else /* CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME */
#ifdef DIR_stat_GET_RAW_CTIME_SEC
#define DIR_stat_GET_CTIME_SEC DIR_stat_GET_RAW_CTIME_SEC
#endif /* DIR_stat_GET_RAW_CTIME_SEC */
#ifdef DIR_stat_GET_RAW_CTIME_NSEC
#define DIR_stat_GET_CTIME_NSEC DIR_stat_GET_RAW_CTIME_NSEC
#endif /* DIR_stat_GET_RAW_CTIME_NSEC */

#ifdef DIR_stat_GET_RAW_BTIME_SEC
#define DIR_stat_GET_BTIME_SEC DIR_stat_GET_RAW_BTIME_SEC
#endif /* DIR_stat_GET_RAW_BTIME_SEC */
#ifdef DIR_stat_GET_RAW_BTIME_NSEC
#define DIR_stat_GET_BTIME_NSEC DIR_stat_GET_RAW_BTIME_NSEC
#endif /* DIR_stat_GET_RAW_BTIME_NSEC */
#endif /* !CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME */



#ifdef posix_opendir_USE_opendir
#ifdef CONFIG_HAVE_struct_dirent_d_namlen
#define dirent_namelen(x) (x)->d_namlen
#elif defined(_D_EXACT_NAMLEN)
#define dirent_namelen(x) _D_EXACT_NAMLEN(x)
#else /* ... */
#define dirent_namelen(x) strlen((x)->d_name)
#endif /* !... */
#endif /* posix_opendir_USE_opendir */


/* Always need stat info for file size/timestamp fields. */
#ifdef posix_opendir_USE_opendir
#if ((!defined(CONFIG_HAVE_struct_dirent_d_type) && defined(DIR_stat_HAVE_st_mode)) || \
     (!defined(CONFIG_HAVE_struct_dirent_d_ino) && defined(DIR_stat_HAVE_st_ino)))
#define posix_opendir_NEED_STAT_EXTENSION
#endif /* !... */
#if (defined(DIR_stat_HAVE_st_dev) ||       /* d_dev */     \
     defined(DIR_stat_HAVE_st_mode) ||      /* d_mode */    \
     defined(DIR_stat_HAVE_st_nlink) ||     /* d_nlink */   \
     defined(DIR_stat_HAVE_st_uid) ||       /* d_uid */     \
     defined(DIR_stat_HAVE_st_gid) ||       /* d_gid */     \
     defined(DIR_stat_HAVE_st_rdev) ||      /* d_rdev */    \
     defined(DIR_stat_HAVE_st_size) ||      /* d_size */    \
     defined(DIR_stat_HAVE_st_blksize) ||   /* d_blksize */ \
     defined(DIR_stat_HAVE_st_blocks) ||    /* d_blocks */  \
     defined(DIR_stat_GET_RAW_ATIME_SEC) || /* d_atime */   \
     defined(DIR_stat_GET_RAW_MTIME_SEC) || /* d_mtime */   \
     defined(DIR_stat_GET_RAW_CTIME_SEC) || /* d_ctime */   \
     defined(DIR_stat_GET_RAW_BTIME_SEC) /* d_birthtime */)
#define posix_opendir_NEED_STAT_EXTENSION
#endif /* ... */
#endif /* posix_opendir_USE_opendir */


DECL_BEGIN

/* NOTE: For performance, the fields of some given `dirent' will only remain valid
 *       as long as the next entry has yet to be read from the directory stream.
 * As such, you must take special care and not do something like `List(opendir(...))',
 * which will produce a list of dirent objects, where  */
typedef struct dir_iterator_object DeeDirIteratorObject;
typedef struct dir_object DeeDirObject;


struct dir_iterator_object {
	OBJECT_HEAD
	DREF DeeObject       *odi_path;     /* [1..1][const] String, File, or int */
	DREF DeeStringObject *odi_pathstr;  /* [0..1][lock(WRITE_ONCE)] String */
	bool                  odi_skipdots; /* [const] When true, skip '.' and '..' entries. */

#ifdef posix_opendir_USE_FindFirstFileExW
	bool                  odi_first;    /* [lock(odi_lock)] When true, we're at the first entry. */
	HANDLE                odi_hnd;      /* [0..1|NULL(INVALID_HANDLE_VALUE)][lock(odi_lock)]
	                                     * The iteration handle or INVALID_HANDLE_VALUE when exhausted. */
	WIN32_FIND_DATAW      odi_data;     /* [lock(odi_lock)][valid_if(odi_hnd != INVALID_HANDLE_VALUE)]
	                                     * The file data for the next matching entry. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t   odi_lock;     /* Lock for the above fields. */
#define dir_iterator_object_HAVE_odi_lock
#endif /* !CONFIG_NO_THREADS */
#endif /* posix_opendir_USE_FindFirstFileExW */

#ifdef posix_opendir_USE_opendir
	struct DIR_dirent    *odi_ent;      /* [0..1] Last-read directory entry (or `NULL' for end-of-directory) */
	DIR                  *odi_dir;      /* [1..1][const] The directory access stream. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t   odi_lock;     /* Lock for the above fields. */
#define dir_iterator_object_HAVE_odi_lock
#endif /* !CONFIG_NO_THREADS */
#endif /* ... */

#ifdef posix_opendir_NEED_STAT_EXTENSION
#ifdef DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION
	HANDLE                odi_stHandle; /* [lock(odi_lock)] Handle used to fill `odi_st', or `INVALID_HANDLE_VALUE'. */
#define DeeDirIterator_IsStatValid(self) ((self)->odi_stHandle != INVALID_HANDLE_VALUE)
#else /* DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
	bool                  odi_stvalid;  /* [lock(odi_lock)] Set to true if `odi_st' has been loaded. */
#define DeeDirIterator_IsStatValid(self) ((self)->odi_stvalid)
#endif /* !DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
	DIR_struct_stat       odi_st;       /* [lock(odi_lock)] Additional stat information (lazily loaded). */
#endif /* posix_opendir_NEED_STAT_EXTENSION */
};

#ifdef dir_iterator_object_HAVE_odi_lock
#define DeeDirIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->odi_lock)
#define DeeDirIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->odi_lock)
#define DeeDirIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->odi_lock)
#define DeeDirIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->odi_lock)
#define DeeDirIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->odi_lock)
#define DeeDirIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->odi_lock)
#define DeeDirIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->odi_lock)
#define DeeDirIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->odi_lock)
#define DeeDirIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->odi_lock)
#define DeeDirIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->odi_lock)
#define DeeDirIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->odi_lock)
#define DeeDirIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->odi_lock)
#define DeeDirIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->odi_lock)
#define DeeDirIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->odi_lock)
#define DeeDirIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->odi_lock)
#define DeeDirIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->odi_lock)
#else /* dir_iterator_object_HAVE_odi_lock */
#define DeeDirIterator_LockReading(self)    1
#define DeeDirIterator_LockWriting(self)    1
#define DeeDirIterator_LockTryRead(self)    1
#define DeeDirIterator_LockTryWrite(self)   1
#define DeeDirIterator_LockCanRead(self)    1
#define DeeDirIterator_LockCanWrite(self)   1
#define DeeDirIterator_LockWaitRead(self)   (void)0
#define DeeDirIterator_LockWaitWrite(self)  (void)0
#define DeeDirIterator_LockRead(self)       (void)0
#define DeeDirIterator_LockWrite(self)      (void)0
#define DeeDirIterator_LockTryUpgrade(self) 1
#define DeeDirIterator_LockUpgrade(self)    1
#define DeeDirIterator_LockDowngrade(self)  (void)0
#define DeeDirIterator_LockEndWrite(self)   (void)0
#define DeeDirIterator_LockEndRead(self)    (void)0
#define DeeDirIterator_LockEnd(self)        (void)0
#endif /* !dir_iterator_object_HAVE_odi_lock */

struct dir_object {
	OBJECT_HEAD
	DREF DeeObject *d_path;      /* [1..1][const] String, File, or int */
	bool            d_skipdots;  /* [const] When true, skip '.' and '..' entries. */
	bool            d_inheritfd; /* [const] When true, creating an iterator when `d_path' is
	                              * something other than a string (i.e. a HANDLE or fd_t),
	                              * then the iterator inherits that handle (iow: closes it
	                              * at some point during its lifetime).
	                              * When this dir_object was created by `fdopendir()', then
	                              * this option is enabled by default. */
};


#ifdef posix_opendir_USE_FindFirstFileExW
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* posix_opendir_USE_FindFirstFileExW */


#ifdef posix_opendir_USE_FindFirstFileExW
#undef CONFIG_HAVE_DT_FIFO
#undef CONFIG_HAVE_DT_CHR
#undef CONFIG_HAVE_DT_DIR
#undef CONFIG_HAVE_DT_BLK
#undef CONFIG_HAVE_DT_REG
#undef CONFIG_HAVE_DT_LNK
#undef CONFIG_HAVE_DT_SOCK
#endif /* posix_opendir_USE_FindFirstFileExW */

#define USED_DT_UNKNOWN DT_UNKNOWN
#define USED_DT_FIFO    DT_FIFO
#define USED_DT_CHR     DT_CHR
#define USED_DT_DIR     DT_DIR
#define USED_DT_BLK     DT_BLK
#define USED_DT_REG     DT_REG
#define USED_DT_LNK     DT_LNK
#define USED_DT_SOCK    DT_SOCK
#define USED_DT_WHT     DT_WHT

#if (!defined(CONFIG_HAVE_DT_FIFO) || !defined(CONFIG_HAVE_DT_CHR) || !defined(CONFIG_HAVE_DT_DIR) || \
     !defined(CONFIG_HAVE_DT_BLK) || !defined(CONFIG_HAVE_DT_REG) || !defined(CONFIG_HAVE_DT_LNK) ||  \
     !defined(CONFIG_HAVE_DT_SOCK))
#undef USED_DT_FIFO
#undef USED_DT_CHR
#undef USED_DT_DIR
#undef USED_DT_BLK
#undef USED_DT_REG
#undef USED_DT_LNK
#undef USED_DT_SOCK
#define USED_DT_FIFO 1
#define USED_DT_CHR  2
#define USED_DT_DIR  4
#define USED_DT_BLK  6
#define USED_DT_REG  8
#define USED_DT_LNK  10
#define USED_DT_SOCK 12
#undef CONFIG_HAVE_DT_UNKNOWN
#undef CONFIG_HAVE_DT_WHT
#define posix_opendir_USE_GENERIC_DT_CONSTANTS
#endif /* !CONFIG_HAVE_... */

#ifndef CONFIG_HAVE_DT_UNKNOWN
#undef USED_DT_UNKNOWN
#define USED_DT_UNKNOWN 0
#endif /* !CONFIG_HAVE_DT_UNKNOWN */
#ifndef CONFIG_HAVE_DT_WHT
#undef USED_DT_WHT
#define USED_DT_WHT 14
#endif /* !CONFIG_HAVE_DT_WHT */


#ifdef CONFIG_HAVE_struct_dirent_d_type_size_1
#define TYPEOF_struct_dirent_d_type uint8_t
#define UNP_struct_dirent_d_type    UNPu8
#define DEFINE_D_TYPE_CONSTANT      DEFINE_INT15
#define DeeInt_New_D_TYPE           DeeInt_NewUInt8
#elif defined(CONFIG_HAVE_struct_dirent_d_type_size_2)
#define TYPEOF_struct_dirent_d_type uint16_t
#define UNP_struct_dirent_d_type    UNPu16
#define DEFINE_D_TYPE_CONSTANT      DEFINE_INT16
#define DeeInt_New_D_TYPE           DeeInt_NewUInt16
#elif defined(CONFIG_HAVE_struct_dirent_d_type_size_4)
#define TYPEOF_struct_dirent_d_type uint32_t
#define UNP_struct_dirent_d_type    UNPu32
#define DEFINE_D_TYPE_CONSTANT      DEFINE_INT32
#define DeeInt_New_D_TYPE           DeeInt_NewUInt32
#else /* ... */
#define TYPEOF_struct_dirent_d_type uint64_t
#define UNP_struct_dirent_d_type    UNPu64
#define DEFINE_D_TYPE_CONSTANT      DEFINE_INT64
#define DeeInt_New_D_TYPE           DeeInt_NewUInt64
#endif /* !... */

/* Define the DT_* constants for export. */
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_UNKNOWN, USED_DT_UNKNOWN);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_FIFO, USED_DT_FIFO);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_CHR, USED_DT_CHR);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_DIR, USED_DT_DIR);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_BLK, USED_DT_BLK);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_REG, USED_DT_REG);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_LNK, USED_DT_LNK);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_SOCK, USED_DT_SOCK);
PRIVATE DEFINE_D_TYPE_CONSTANT(posix_DT_WHT, USED_DT_WHT);

#ifdef posix_opendir_USE_GENERIC_DT_CONSTANTS
#define USED_IFTODT(mode)    (((mode)&0170000) >> 12)
#define USED_DTTOIF(dirtype) ((dirtype) << 12)
#ifdef posix_opendir_USE_opendir
#define NATIVE_DT_TO_USED_DT dee_NATIVE_DT_TO_USED_DT
LOCAL TYPEOF_struct_dirent_d_type dee_NATIVE_DT_TO_USED_DT(unsigned int dt) {
	switch (dt) {
#ifdef CONFIG_HAVE_DT_DIR
	case DT_DIR:  return USED_DT_DIR;
#endif /* CONFIG_HAVE_DT_DIR */
#ifdef CONFIG_HAVE_DT_CHR
	case DT_CHR:  return USED_DT_CHR;
#endif /* CONFIG_HAVE_DT_CHR */
#ifdef CONFIG_HAVE_DT_BLK
	case DT_BLK:  return USED_DT_BLK;
#endif /* CONFIG_HAVE_DT_BLK */
#ifdef CONFIG_HAVE_DT_REG
	case DT_REG:  return USED_DT_REG;
#endif /* CONFIG_HAVE_DT_REG */
#ifdef CONFIG_HAVE_DT_FIFO
	case DT_FIFO: return USED_DT_FIFO;
#endif /* CONFIG_HAVE_DT_FIFO */
#ifdef CONFIG_HAVE_DT_LNK
	case DT_LNK:  return USED_DT_LNK;
#endif /* CONFIG_HAVE_DT_LNK */
#ifdef CONFIG_HAVE_DT_SOCK
	case DT_SOCK: return USED_DT_SOCK;
#endif /* CONFIG_HAVE_DT_SOCK */
	default:      return USED_DT_UNKNOWN;
	}
}
#endif /* posix_opendir_USE_opendir */
#else /* posix_opendir_USE_GENERIC_DT_CONSTANTS */
#define NATIVE_DT_TO_USED_DT(dt) dt
#ifdef CONFIG_HAVE_IFTODT
#define USED_IFTODT IFTODT
#else /* CONFIG_HAVE_IFTODT */
#define USED_IFTODT dee_IFTODT
LOCAL TYPEOF_struct_dirent_d_type dee_IFTODT(unsigned int if_) {
	switch (if_ & STAT_IFMT) {
	case STAT_IFDIR:  return USED_DT_DIR;
	case STAT_IFCHR:  return USED_DT_CHR;
	case STAT_IFBLK:  return USED_DT_BLK;
	case STAT_IFREG:  return USED_DT_REG;
	case STAT_IFIFO:  return USED_DT_FIFO;
	case STAT_IFLNK:  return USED_DT_LNK;
	case STAT_IFSOCK: return USED_DT_SOCK;
	default:          return USED_DT_UNKNOWN;
	}
}
#endif /* !CONFIG_HAVE_IFTODT */
#ifdef CONFIG_HAVE_DTTOIF
#define USED_DTTOIF DTTOIF
#else /* CONFIG_HAVE_DTTOIF */
#define USED_DTTOIF dee_DTTOIF
LOCAL unsigned int dee_DTTOIF(TYPEOF_struct_dirent_d_type dt) {
	switch (dt) {
	case USED_DT_DIR:  return STAT_IFDIR;
	case USED_DT_CHR:  return STAT_IFCHR;
	case USED_DT_BLK:  return STAT_IFBLK;
	case USED_DT_REG:  return STAT_IFREG;
	case USED_DT_FIFO: return STAT_IFIFO;
	case USED_DT_LNK:  return STAT_IFLNK;
	case USED_DT_SOCK: return STAT_IFSOCK;
	default:           return 0;
	}
}
#endif /* !CONFIG_HAVE_DTTOIF */
#endif /* !posix_opendir_USE_GENERIC_DT_CONSTANTS */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_DTTOIF_f(DeeObject *__restrict dt_arg) {
	TYPEOF_struct_dirent_d_type dt;
	if (DeeObject_AsUIntX(dt_arg, &dt))
		goto err;
	return DeeInt_NewUInt(USED_DTTOIF(dt));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_IFTODT_f(DeeObject *__restrict if_arg) {
	unsigned int if_;
	if (DeeObject_AsUInt(if_arg, &if_))
		goto err;
	return DeeInt_New_D_TYPE(USED_IFTODT(if_));
err:
	return NULL;
}

INTERN DEFINE_CMETHOD1(posix_DTTOIF, &posix_DTTOIF_f, METHOD_FNORMAL);
INTERN DEFINE_CMETHOD1(posix_IFTODT, &posix_IFTODT_f, METHOD_FNORMAL);

/* High-level wrappers around `struct dirent' and `DIR'
 * Note that `DIR' has a constructor that behaves just like `opendir(3)',
 * which is also why `posix.opendir' is exported as an alias for `posix.DIR' */
INTDEF DeeTypeObject DeeDirIterator_Type;
INTDEF DeeTypeObject DeeDir_Type;





/************************************************************************/
/* DIR                                                                  */
/************************************************************************/
PRIVATE NONNULL((1)) void DCALL
diriter_fini(DeeDirIteratorObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) int DCALL
directory_open(DeeDirIteratorObject *__restrict self,
               DeeObject *__restrict path,
               bool skipdots, bool inheritfd) {
#ifdef posix_opendir_USE_FindFirstFileExW
#define NEED_err
	LPWSTR wname, wpattern;
	size_t i, wname_length;
	if (!DeeString_Check(path)) {
		int result;
		HANDLE hPath;
		DREF DeeObject *sPath;
#ifdef CONFIG_HAVE_close
		int fd;
		hPath = DeeNTSystem_GetHandleEx(path, &fd);
#else /* CONFIG_HAVE_close */
		hPath = DeeNTSystem_GetHandle(path);
#endif /* !CONFIG_HAVE_close */
		if (hPath == INVALID_HANDLE_VALUE)
			goto err;
		sPath = DeeNTSystem_GetFilenameOfHandle(hPath);
		if unlikely(!sPath)
			goto err;
		result = directory_open(self, sPath, skipdots, false);
		Dee_Decref(sPath);
		if (likely(result == 0) && inheritfd) {
			if (DeeFile_Check(path)) {
				/* Inherit the handle by closing the associated file. */
				if unlikely(DeeFile_Close(path)) {
					diriter_fini(self);
					goto err;
#define NEED_err
				}
#ifdef CONFIG_HAVE_close
			} else if (fd != -1) {
				/* NOTE: If `DeeNTSystem_GetHandle' used get_osfhandle(), we must
				 *       close(fd) here instead (where `fd' is the fd that was
				 *       used by `DeeNTSystem_GetHandle')! */
				(void)close(fd); /* Inherited! */
#endif /* CONFIG_HAVE_close */
			} else {
				(void)CloseHandle(hPath); /* Inherited! */
			}
		}
		return result;
	}
	wname = (LPWSTR)DeeString_AsWide(path);
	if unlikely(!wname)
		goto err;

	/* Append the `\\*' to the given path and fix forward-slashes. */
	wname_length = WSTR_LENGTH(wname);
	wpattern     = (LPWSTR)Dee_Malloca(8 + wname_length * 2);
	if unlikely(!wpattern)
		goto err;
	for (i = 0; i < wname_length; ++i) {
		WCHAR ch = wname[i];
		/* FindFirstFile() actually fails when handed forward-slashes.
		 * That's something I didn't notice in the old deemon, which
		 * caused fs.dir() to (seemingly) fail at random. */
		if (ch == '/')
			ch = '\\';
		wpattern[i] = ch;
	}

	/* Use the current directory if the given name is empty. */
	if (!wname_length)
		wpattern[wname_length++] = '.';

	/* Append a trailing backslash if there isn't one already. */
	if (wpattern[wname_length - 1] != '\\')
		wpattern[wname_length++] = '\\';

	/* Append a match-all wildcard. */
	wpattern[wname_length++] = '*';
	wpattern[wname_length]   = 0;
	DBG_ALIGNMENT_DISABLE();
	self->odi_hnd = FindFirstFileExW(wpattern, FindExInfoBasic, &self->odi_data,
	                                 FindExSearchNameMatch, NULL, 0);
	DBG_ALIGNMENT_ENABLE();
	Dee_Freea(wpattern);
	self->odi_first = true;
	if unlikely(self->odi_hnd == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError != ERROR_NO_MORE_FILES) {
			if (DeeNTSystem_IsFileNotFoundError(dwError)) {
				DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
				                        "Path %r could not be found",
				                        path);
			} else if (DeeNTSystem_IsNotDir(dwError)) {
				DeeNTSystem_ThrowErrorf(&DeeError_NoDirectory, dwError,
				                        "Some part of the path %r is not a directory",
				                        path);
			} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
				DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
				                        "Search permissions are not granted for path %r",
				                        path);
			} else {
				DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
				                        "Failed to open directory %r",
				                        path);
			}
			goto err;
		}

		/* Empty directory? ok... */
		self->odi_first = false;
	} else if (skipdots) {
		while (self->odi_data.cFileName[0] == '.' &&
		       (self->odi_data.cFileName[1] == 0 ||
		        (self->odi_data.cFileName[1] == '.' &&
		         self->odi_data.cFileName[2] == 0))) {
			/* Skip this one... */
again_skipdots:
			DBG_ALIGNMENT_DISABLE();
			if (!FindNextFileW(self->odi_hnd, &self->odi_data)) {
				HANDLE hnd;
				DWORD dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if unlikely(dwError != ERROR_NO_MORE_FILES) {
					if (DeeNTSystem_IsBadAllocError(dwError)) {
						if (Dee_CollectMemory(1))
							goto again_skipdots;
					} else {
						DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
						                        "Failed to read entries from directory %r",
						                        path);
					}
					DBG_ALIGNMENT_DISABLE();
					(void)FindClose(self->odi_hnd);
					DBG_ALIGNMENT_ENABLE();
					goto err;
				}
				hnd          = self->odi_hnd;
				self->odi_hnd = INVALID_HANDLE_VALUE;
				DBG_ALIGNMENT_DISABLE();
				(void)FindClose(hnd);
				DBG_ALIGNMENT_ENABLE();
				/* Directory only contains "." and ".." */
				self->odi_first = false;
				break;
			}
			DBG_ALIGNMENT_ENABLE();
		}
	}
	Dee_atomic_rwlock_init(&self->odi_lock);
#elif defined(posix_opendir_USE_opendir)
	DIR *dir;
#define NEED_err
EINTR_LABEL(again)
	if (!DeeString_Check(path)) {
		int fd;
		fd = DeeUnixSystem_GetFD(path);
		if unlikely(fd == -1)
			goto err;
#if defined(CONFIG_HAVE_dup) && defined(CONFIG_HAVE_fdopendir)
		if (inheritfd) {
			dir = fdopendir(fd);
		} else {
			DBG_ALIGNMENT_DISABLE();
			fd = dup(fd);
			if unlikely(fd == -1) {
				dir = NULL;
			} else {
				dir = fdopendir(fd);
#ifdef CONFIG_HAVE_close
				if unlikely(!dir) {
					int saved_errno;
					saved_errno = DeeSystem_GetErrno();
					(void)close(fd);
					DeeSystem_SetErrno(saved_errno);
				}
			}
#endif /* CONFIG_HAVE_close */
		}
#else /* CONFIG_HAVE_dup && CONFIG_HAVE_fdopendir */
#ifdef CONFIG_HAVE_fdopendir
		if (inheritfd) {
			dir = fdopendir(fd);
		} else
#endif /* CONFIG_HAVE_fdopendir */
		{
			int result;
			DREF DeeObject *sPath;
			sPath = DeeSystem_GetFilenameOfFD(fd);
			if unlikely(!sPath)
				goto err;
			result = directory_open(self, sPath, skipdots, inheritfd);
			Dee_Decref(sPath);
			return result;
		}
#endif /* !CONFIG_HAVE_dup || !CONFIG_HAVE_fdopendir */
	} else {
		char const *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		dir = opendir((char *)utf8);
	}
	if unlikely(dir == NULL) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(error, again, err)
		HANDLE_ENOENT(error, err, "Path %r could not be found", path)
		HANDLE_ENOTDIR(error, err, "Path %r could not be found", path)
		HANDLE_EACCES(error, err, "Some part of the path %r is not a directory", path)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to open directory %r",
		                          path);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	self->odi_dir     = dir;
	self->odi_ent     = NULL;
	Dee_atomic_rwlock_init(&self->odi_lock);
#endif /* ... */
	self->odi_skipdots = skipdots;
	self->odi_path     = path;
	self->odi_pathstr  = NULL;
#ifdef posix_opendir_NEED_STAT_EXTENSION
#ifdef DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION
	self->odi_stHandle = INVALID_HANDLE_VALUE;
#else /* DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
	self->odi_stvalid = false;
#endif /* !DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
#endif /* posix_opendir_NEED_STAT_EXTENSION */
	Dee_Incref(path);
	return 0;
#ifdef NEED_err
#undef NEED_err
err:
	return -1;
#endif /* NEED_err */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeDirIteratorObject *DCALL
diriter_next(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
again:
	DeeDirIterator_LockWrite(self);
	if (!self->odi_first) {
		if (self->odi_hnd == INVALID_HANDLE_VALUE) {
			DeeDirIterator_LockEndWrite(self);
			return (DREF DeeDirIteratorObject *)ITER_DONE;
		}
		DBG_ALIGNMENT_DISABLE();
		if (!FindNextFileW(self->odi_hnd, &self->odi_data)) {
			HANDLE hnd;
			DWORD dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if unlikely(dwError != ERROR_NO_MORE_FILES) {
				DeeDirIterator_LockEndWrite(self);
				if (DeeNTSystem_IsBadAllocError(dwError)) {
					if (Dee_CollectMemory(1))
						goto again;
				} else {
					DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
					                        "Failed to read entries from directory %r",
					                        self->odi_path);
				}
				return NULL;
			}
			hnd           = self->odi_hnd;
			self->odi_hnd = INVALID_HANDLE_VALUE;
			DeeDirIterator_LockEndWrite(self);
			DBG_ALIGNMENT_DISABLE();
			(void)FindClose(hnd);
			DBG_ALIGNMENT_ENABLE();
			return (DREF DeeDirIteratorObject *)ITER_DONE;
		}
		DBG_ALIGNMENT_ENABLE();
	}
	self->odi_first = false;
	if (self->odi_skipdots) {
		if (self->odi_data.cFileName[0] == '.' &&
		    (self->odi_data.cFileName[1] == 0 ||
		     (self->odi_data.cFileName[1] == '.' &&
		      self->odi_data.cFileName[2] == 0)))
			goto again; /* Skip this one... */
	}
#ifdef posix_opendir_NEED_STAT_EXTENSION
#ifdef DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION
	if (self->odi_stHandle != INVALID_HANDLE_VALUE) {
		HANDLE hStHandle   = self->odi_stHandle;
		self->odi_stHandle = INVALID_HANDLE_VALUE;
		DeeDirIterator_LockEndWrite(self);
		DBG_ALIGNMENT_DISABLE();
		(void)CloseHandle(hStHandle);
		DBG_ALIGNMENT_ENABLE();
	} else {
		DeeDirIterator_LockEndWrite(self);
	}
#else /* DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
	self->odi_stvalid = false;
	DeeDirIterator_LockEndWrite(self);
#endif /* !DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
#else /* posix_opendir_NEED_STAT_EXTENSION */
	DeeDirIterator_LockEndWrite(self);
#endif /* !posix_opendir_NEED_STAT_EXTENSION */
	Dee_Incref(self);
	return self;
#elif defined(posix_opendir_USE_opendir)
	DeeDirIterator_LockWrite(self);
again:
	DBG_ALIGNMENT_DISABLE();
	DeeSystem_SetErrno(0);
	self->odi_ent = DIR_readdir(self->odi_dir);
	if (self->odi_ent == NULL) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeDirIterator_LockEndWrite(self);
		if (error == 0)
			return (DREF DeeDirIteratorObject *)ITER_DONE; /* End of directory. */
		EINTR_HANDLE(error, again, err)
		DeeUnixSystem_ThrowErrorf(NULL, error,
		                          "Failed to read entries from directory %r",
		                          self->odi_path);
err:
		return NULL;
	}
	DBG_ALIGNMENT_ENABLE();
	if (self->odi_skipdots) {
		char const *name = self->odi_ent->d_name;
		if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0)))
			goto again; /* Skip this one... */
	}
#ifdef posix_opendir_NEED_STAT_EXTENSION
#ifdef DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION
	if (self->odi_stHandle != INVALID_HANDLE_VALUE) {
		HANDLE hStHandle   = self->odi_stHandle;
		self->odi_stHandle = INVALID_HANDLE_VALUE;
		DeeDirIterator_LockEndWrite(self);
		DBG_ALIGNMENT_DISABLE();
		(void)CloseHandle(hStHandle);
		DBG_ALIGNMENT_ENABLE();
	} else {
		DeeDirIterator_LockEndWrite(self);
	}
#else /* DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
	self->odi_stvalid = false;
	DeeDirIterator_LockEndWrite(self);
#endif /* !DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
#else /* posix_opendir_NEED_STAT_EXTENSION */
	DeeDirIterator_LockEndWrite(self);
#endif /* !posix_opendir_NEED_STAT_EXTENSION */
	Dee_Incref(self);
	return self;
#else /* ... */
	(void)self;
	return (DREF DeeDirIteratorObject *)ITER_DONE;
#endif /* !... */
}

PRIVATE NONNULL((1)) void DCALL
diriter_fini(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_NEED_STAT_EXTENSION
#ifdef DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION
	if (self->odi_stHandle != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		(void)CloseHandle(self->odi_stHandle);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
#endif /* posix_opendir_NEED_STAT_EXTENSION */
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		(void)FindClose(self->odi_hnd);
		DBG_ALIGNMENT_ENABLE();
	}
#elif defined(posix_opendir_USE_opendir)
	DBG_ALIGNMENT_DISABLE();
	(void)closedir(self->odi_dir);
	DBG_ALIGNMENT_ENABLE();
#endif /* ... */
	Dee_XDecref(self->odi_pathstr);
	Dee_Decref(self->odi_path);
}


PRIVATE NONNULL((1)) int DCALL
diriter_unbound_attr(char const *__restrict name) {
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%k.%s'",
	                       &DeeDirIterator_Type, name);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_makepathstr(DeeDirIteratorObject *__restrict self) {
	DREF DeeStringObject *result;
	result = (DREF DeeStringObject *)self->odi_path;
	if (DeeString_Check(result))
		return_reference_(result);
#ifdef posix_opendir_USE_FindFirstFileExW
	{
		HANDLE hPath;
		hPath = DeeNTSystem_GetHandle((DeeObject *)result);
		if (hPath == INVALID_HANDLE_VALUE)
			goto err;
		result = (DREF DeeStringObject *)DeeNTSystem_GetFilenameOfHandle(hPath);
	}
	return result;
err:
	return NULL;
#elif defined(posix_opendir_USE_opendir)
	{
		int fd;
		fd = DeeUnixSystem_GetFD((DeeObject *)result);
		if (fd == -1)
			goto err;
		result = (DREF DeeStringObject *)DeeSystem_GetFilenameOfFD(fd);
	}
	return result;
err:
	return NULL;
#else /* ... */
	diriter_unbound_attr("pathstr");
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_getpathstr(DeeDirIteratorObject *__restrict self) {
	DREF DeeStringObject *result;
again:
	result = self->odi_pathstr;
	if (result == NULL) {
		result = diriter_makepathstr(self);
		/* Remember the full path string. */
		if unlikely(!atomic_cmpxch_weak(&self->odi_pathstr, NULL, result)) {
			Dee_Decref_likely(result);
			goto again;
		}
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_get_d_fullname(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
#define HAVE_D_FULLNAME
	if (self->odi_hnd != INVALID_HANDLE_VALUE)
#elif defined(posix_opendir_USE_opendir)
#define HAVE_D_FULLNAME
	if (self->odi_ent != NULL)
#endif /* ... */
#ifdef HAVE_D_FULLNAME
#undef HAVE_D_FULLNAME
	{
		DREF DeeStringObject *path;
		path = diriter_getpathstr(self);
		if likely(path) {
			struct unicode_printer printer = UNICODE_PRINTER_INIT;
			if unlikely(unicode_printer_printstring(&printer, (DeeObject *)path) < 0) {
				Dee_Decref(path);
err_printer:
				unicode_printer_fini(&printer);
				return NULL;
			}
			Dee_Decref(path);
			if (UNICODE_PRINTER_LENGTH(&printer)) {
				uint32_t lastch;
				lastch = UNICODE_PRINTER_GETCHAR(&printer, UNICODE_PRINTER_LENGTH(&printer) - 1);
#ifdef posix_opendir_USE_FindFirstFileExW
				if (lastch != '/' && lastch != '\\') {
					if (unicode_printer_putc(&printer, '\\'))
						goto err_printer;
				}
#else /* posix_opendir_USE_FindFirstFileExW */
				if (lastch != DeeSystem_SEP) {
					if (unicode_printer_putc(&printer, DeeSystem_SEP))
						goto err_printer;
				}
#endif /* !posix_opendir_USE_FindFirstFileExW */
			}
#ifdef posix_opendir_USE_FindFirstFileExW
			if unlikely(unicode_printer_printwide(&printer, self->odi_data.cFileName,
			                                      wcslen(self->odi_data.cFileName)) < 0)
				goto err_printer;
#elif defined(posix_opendir_USE_opendir)
			if unlikely(unicode_printer_printutf8(&printer, self->odi_ent->d_name,
			                                      dirent_namelen(self->odi_ent)) < 0)
				goto err_printer;
#else /* ... */
			diriter_unbound_attr("d_fullname");
			goto err_printer;
#endif /* !... */
			path = (DREF DeeStringObject *)unicode_printer_pack(&printer);
		}
		return path;
	}
#else /* HAVE_D_FULLNAME */
#define diriter_get_d_fullname_IS_STUB
#endif /* !HAVE_D_FULLNAME */
	diriter_unbound_attr("d_fullname");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_name(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		return DeeString_NewWide(self->odi_data.cFileName,
		                         wcslen(self->odi_data.cFileName),
		                         STRING_ERROR_FREPLAC);
	}
#elif defined(posix_opendir_USE_opendir)
	if (self->odi_ent != NULL) {
		return DeeString_NewUtf8(self->odi_ent->d_name,
		                         dirent_namelen(self->odi_ent),
		                         STRING_ERROR_FREPLAC);
	}
#else /* ... */
#define diriter_get_d_name_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_name");
	return NULL;
}

#ifdef posix_opendir_NEED_STAT_EXTENSION
PRIVATE WUNUSED NONNULL((1)) int DCALL
diriter_loadstat(DeeDirIteratorObject *__restrict self) {
	if (!DeeDirIterator_IsStatValid(self)) {
#ifdef DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION
		DREF DeeStringObject *fullname;
		HANDLE hFile;
		fullname = diriter_get_d_fullname(self);
		if unlikely(!fullname)
			goto err;
		hFile = DeeNTSystem_CreateFileNoATime((DeeObject *)fullname, GENERIC_READ,
		                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
		                                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
		                                      NULL);
		if unlikely(hFile == NULL)
			goto err_fullname;
		if unlikely(hFile == INVALID_HANDLE_VALUE) {
			DeeNTSystem_ThrowLastErrorf(NULL, "Failed to stat-open %r in %k", fullname, self);
			goto err_fullname;
		}

		/* Actually retrieve stat information. */
		if (GetFileInformationByHandle(hFile, &self->odi_st)) {
			/* Don't leak handles if another thread allocated another handle in the mean time. */
			DeeDirIterator_LockWrite(self);
			if unlikely(self->odi_stHandle != INVALID_HANDLE_VALUE) {
				DeeDirIterator_LockEndWrite(self);
				(void)CloseHandle(hFile);
			} else {
				self->odi_stHandle = hFile; /* Inherit handle. */
				DeeDirIterator_LockEndWrite(self);
			}
			Dee_Decref(fullname);
			return 0;
		}
		DeeNTSystem_ThrowLastErrorf(NULL, "Failed to stat %r in %k", fullname, self);
		(void)CloseHandle(hFile);
err_fullname:
		Dee_Decref(fullname);
err:
		return -1;
#else /* DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
		int error;
again:
#if defined(DIR_lstatat) && defined(CONFIG_HAVE_dirfd)
		if (DIR_lstatat(dirfd(self->odi_dir), self->odi_ent->d_name, &self->odi_st) == 0) {
			self->odi_stvalid = true;
			return 0;
		}
		error = DeeSystem_GetErrno();
#else /* DIR_lstatat && CONFIG_HAVE_dirfd */
		DREF DeeStringObject *fullname;
		char const *utf8;
		fullname = diriter_get_d_fullname(self);
		if unlikely(!fullname)
			goto err;
		utf8 = DeeString_AsUtf8((DeeObject *)fullname);
		if unlikely(!utf8) {
			Dee_Decref(fullname);
			goto err;
		}
		if (DIR_lstat(utf8, &self->odi_st) == 0) {
			self->odi_stvalid = true;
			Dee_Decref(fullname);
			return 0;
		}
		error = DeeSystem_GetErrno();
		Dee_Decref(fullname);
#endif /* !DIR_lstatat || !CONFIG_HAVE_dirfd */
		EINTR_HANDLE(error, again, err)
		DeeUnixSystem_ThrowErrorf(NULL, error,
		                          "Failed to stat %R in %k",
		                          diriter_get_d_fullname(self),
		                          self);
		goto err;
err:
		return -1;
#endif /* !DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION */
	}
	return 0;
}
#endif /* posix_opendir_NEED_STAT_EXTENSION */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_type(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		DeeObject *result;
		if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
			result = (DeeObject *)&posix_DT_LNK;
		} else if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			result = (DeeObject *)&posix_DT_DIR;
		} else if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) {
			result = (DeeObject *)&posix_DT_CHR; /* TODO: Must determine the type of device */
		} else if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			/* TODO: Check if it's an old-style cygwin symlink! */
			result = (DeeObject *)&posix_DT_REG;
		} else {
			result = (DeeObject *)&posix_DT_REG;
		}
		return_reference_(result);
	}
#elif defined(posix_opendir_USE_opendir)
	if (self->odi_ent != NULL) {
#ifdef CONFIG_HAVE_struct_dirent_d_type
		return DeeInt_New_D_TYPE(NATIVE_DT_TO_USED_DT(self->odi_ent->d_type));
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_mode)
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_New_D_TYPE(USED_IFTODT(self->odi_st.st_mode));
#endif /* ... */
	}
#else /* ... */
#define diriter_get_d_type_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_type");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_ino(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		if unlikely(diriter_loadstat(self))
			return NULL;
		return DeeInt_NewUInt64(((uint64_t)self->odi_st.nFileIndexLow) |
		                     ((uint64_t)self->odi_st.nFileIndexHigh << 32));
	}
#elif (defined(posix_opendir_USE_opendir) &&        \
       (defined(CONFIG_HAVE_struct_dirent_d_ino) || \
        (defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_ino))))
	if (self->odi_ent != NULL) {
#ifdef CONFIG_HAVE_struct_dirent_d_ino
		return DeeInt_NEWU(self->odi_ent->d_ino);
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_ino)
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_ino);
#endif /* ... */
	}
#else /* ... */
#define diriter_get_d_ino_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_ino");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_namlen(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE)
		return DeeInt_NewSize(wcslen(self->odi_data.cFileName));
#elif defined(posix_opendir_USE_opendir)
	if (self->odi_ent != NULL) {
#ifdef CONFIG_HAVE_struct_dirent_d_namlen
		return DeeInt_NEWU(self->odi_ent->d_namlen);
#elif defined(_D_EXACT_NAMLEN)
		return DeeInt_NEWU(_D_EXACT_NAMLEN(self->odi_ent));
#else /* ... */
		return DeeInt_NewSize(strlen(self->odi_ent->d_name));
#endif /* !... */
	}
#else /* ... */
#define diriter_get_d_namlen_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_namlen");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_reclen(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		return DeeInt_NewSize((sizeof(self->odi_data) - sizeof(self->odi_data.cFileName)) +
		                      (wcslen(self->odi_data.cFileName) * sizeof(self->odi_data.cFileName[0])));
	}
#elif defined(posix_opendir_USE_opendir)
	if (self->odi_ent != NULL) {
#ifdef CONFIG_HAVE_struct_dirent_d_reclen
		return DeeInt_NEWU(self->odi_ent->d_reclen);
#else /* CONFIG_HAVE_struct_dirent_d_reclen */
		return DeeInt_NewSize((offsetof(struct DIR_dirent, d_name)) +
		                      (strlen(self->odi_ent->d_name) + 1) * sizeof(char));
#endif /* !CONFIG_HAVE_struct_dirent_d_reclen */
	}
#else /* ... */
#define diriter_get_d_reclen_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_reclen");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_off(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
#define diriter_get_d_off_IS_STUB
#elif defined(posix_opendir_USE_opendir) && defined(CONFIG_HAVE_struct_dirent_d_off)
	if (self->odi_ent != NULL) {
		return DeeInt_NEWU(self->odi_ent->d_off);
	}
#else /* ... */
#define diriter_get_d_off_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_off");
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_dev(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NewUInt32((uint32_t)self->odi_st.dwVolumeSerialNumber);
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_dev)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_dev);
	}
#else /* ... */
#define diriter_get_d_dev_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_dev");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_mode(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		uint32_t result = 0444 | 0111; /* XXX: executable should depend on extension. */
		if (!(self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
			result |= 0222;
		if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
			result |= STAT_IFLNK;
		} else if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			result |= STAT_IFDIR;
		} else if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) {
			result |= STAT_IFCHR; /* TODO: Must determine the type of device */
		} else if (self->odi_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			/* TODO: Check if it's an old-style cygwin symlink! */
			result |= STAT_IFREG;
		} else {
			result |= STAT_IFREG;
		}
		return DeeInt_NewUInt32(result);
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_mode)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_mode);
	}
#else /* ... */
#define diriter_get_d_mode_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_mode");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_nlink(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NewUInt32((uint32_t)self->odi_st.nNumberOfLinks);
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_nlink)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_nlink);
	}
#else /* ... */
#define diriter_get_d_nlink_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_nlink");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_uid(DeeDirIteratorObject *__restrict self) {
#if defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION)
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		if unlikely(diriter_loadstat(self))
			return NULL;
		return nt_GetSecurityInfoOwnerSid(self->odi_stHandle, SE_FILE_OBJECT);
#define NEED_nt_GetSecurityInfoOwnerSid
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_uid)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_uid);
	}
#else /* ... */
#define diriter_get_d_uid_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_uid");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_gid(DeeDirIteratorObject *__restrict self) {
#if defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_struct_stat_IS_BY_HANDLE_FILE_INFORMATION)
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		if unlikely(diriter_loadstat(self))
			return NULL;
		return nt_GetSecurityInfoGroupSid(self->odi_stHandle, SE_FILE_OBJECT);
#define NEED_nt_GetSecurityInfoGroupSid
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_gid)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_gid);
	}
#else /* ... */
#define diriter_get_d_gid_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_gid");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_rdev(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
#define diriter_get_d_rdev_IS_STUB
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_rdev)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_rdev);
	}
#else /* ... */
#define diriter_get_d_rdev_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_rdev");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_size(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		return DeeInt_NewUInt64(((uint64_t)self->odi_data.nFileSizeHigh << 32) |
		                     ((uint64_t)self->odi_data.nFileSizeLow));
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_HAVE_st_size)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_size);
	}
#else /* ... */
#define diriter_get_d_size_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_size");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_blocks(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		uint64_t result = ((uint64_t)self->odi_data.nFileSizeHigh << 32) |
		                  ((uint64_t)self->odi_data.nFileSizeLow);
		return DeeInt_NewUInt64(DEFAULT_BLOCKS_FROM_FILESIZE(result));
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && (defined(DIR_stat_HAVE_st_blocks) || defined(DIR_stat_HAVE_st_size))
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
#ifdef DIR_stat_HAVE_st_blocks
		return DeeInt_NEWU(self->odi_st.st_blocks);
#else /* DIR_stat_HAVE_st_blocks */
		return DeeInt_NEWU(DEFAULT_BLOCKS_FROM_FILESIZE(self->odi_st.st_size));
#endif /* !DIR_stat_HAVE_st_blocks */
	}
#else /* ... */
#define diriter_get_d_blocks_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_blocks");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_blksize(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
		return DeeInt_NewUInt16(DEFAULT_BLOCKSIZE);
	}
#elif (defined(posix_opendir_NEED_STAT_EXTENSION) && \
       (defined(DIR_stat_HAVE_st_blksize) || !defined(diriter_get_d_blocks_IS_STUB)))
	if (self->odi_ent != NULL) {
#ifdef DIR_stat_HAVE_st_blksize
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
		return DeeInt_NEWU(self->odi_st.st_blksize);
#else /* DIR_stat_HAVE_st_blksize */
		return DeeInt_NewUInt16(DEFAULT_BLOCKSIZE);
#endif /* !DIR_stat_HAVE_st_blksize */
	}
#else /* ... */
#define diriter_get_d_blksize_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_blksize");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}



#ifdef DECLARE_DeeTime_NewUnix
INTDEF DECLARE_DeeTime_NewUnix();
#undef DECLARE_DeeTime_NewUnix
#endif /* DECLARE_DeeTime_NewUnix */

#ifdef DECLARE_DeeTime_NewFILETIME
INTDEF DECLARE_DeeTime_NewFILETIME();
#undef DECLARE_DeeTime_NewFILETIME
#endif /* DECLARE_DeeTime_NewFILETIME */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_atime(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
#define NEED_DeeTime_NewFILETIME
		return DeeTime_NewFILETIME(&self->odi_data.ftLastAccessTime);
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_GET_RAW_ATIME_SEC)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
#define NEED_DeeTime_NewUnix
#ifdef DIR_stat_GET_RAW_ATIME_NSEC
		return DeeTime_NewUnix(DIR_stat_GET_RAW_ATIME_SEC(&self->odi_st),
		                       DIR_stat_GET_RAW_ATIME_NSEC(&self->odi_st));
#else /* DIR_stat_GET_RAW_ATIME_NSEC */
		return DeeTime_NewUnix(DIR_stat_GET_RAW_ATIME_SEC(&self->odi_st), 0);
#endif /* !DIR_stat_GET_RAW_ATIME_NSEC */
	}
#else /* ... */
#define diriter_get_d_atime_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_atime");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_mtime(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
#define NEED_DeeTime_NewFILETIME
		return DeeTime_NewFILETIME(&self->odi_data.ftLastWriteTime);
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_GET_RAW_MTIME_SEC)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
#define NEED_DeeTime_NewUnix
#ifdef DIR_stat_GET_RAW_MTIME_NSEC
		return DeeTime_NewUnix(DIR_stat_GET_RAW_MTIME_SEC(&self->odi_st),
		                       DIR_stat_GET_RAW_MTIME_NSEC(&self->odi_st));
#else /* DIR_stat_GET_RAW_MTIME_NSEC */
		return DeeTime_NewUnix(DIR_stat_GET_RAW_MTIME_SEC(&self->odi_st), 0);
#endif /* !DIR_stat_GET_RAW_MTIME_NSEC */
	}
#else /* ... */
#define diriter_get_d_mtime_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_mtime");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}

#ifdef posix_opendir_USE_FindFirstFileExW
/* If the OS doesn't have a dedicated st_ctime timestamp, re-use the d_mtime-one (if present) */
#define diriter_get_d_ctime diriter_get_d_mtime
#elif defined(posix_stat_GET_CTIME_SEC)
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_ctime(DeeDirIteratorObject *__restrict self) {
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_DeeTime_NewUnix
#ifdef DIR_stat_GET_RAW_CTIME_NSEC
		return DeeTime_NewUnix(DIR_stat_GET_RAW_CTIME_SEC(&self->odi_st),
		                       DIR_stat_GET_RAW_CTIME_NSEC(&self->odi_st));
#else /* DIR_stat_GET_RAW_CTIME_NSEC */
		return DeeTime_NewUnix(DIR_stat_GET_RAW_CTIME_SEC(&self->odi_st), 0);
#endif /* !DIR_stat_GET_RAW_CTIME_NSEC */
	}
	diriter_unbound_attr("d_ctime");
err:
	return NULL;
}
#elif !defined(diriter_get_d_mtime_IS_STUB)
/* If the OS doesn't have a dedicated st_ctime timestamp, re-use the d_mtime-one (if present) */
#define diriter_get_d_ctime diriter_get_d_mtime
#else /* ... */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_ctime(DeeDirIteratorObject *__restrict self) {
#define diriter_get_d_ctime_IS_STUB
	(void)self;
	diriter_unbound_attr("d_ctime");
	return NULL;
}
#endif /* !... */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_birthtime(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->odi_hnd != INVALID_HANDLE_VALUE) {
#define NEED_DeeTime_NewFILETIME
		return DeeTime_NewFILETIME(&self->odi_data.ftCreationTime);
	}
#elif defined(posix_opendir_NEED_STAT_EXTENSION) && defined(DIR_stat_GET_RAW_BTIME_SEC)
	if (self->odi_ent != NULL) {
		if unlikely(diriter_loadstat(self))
			goto err;
#define NEED_err
#define NEED_DeeTime_NewUnix
#ifdef DIR_stat_GET_RAW_BTIME_NSEC
		return DeeTime_NewUnix(DIR_stat_GET_RAW_BTIME_SEC(&self->odi_st),
		                       DIR_stat_GET_RAW_BTIME_NSEC(&self->odi_st));
#else /* DIR_stat_GET_RAW_BTIME_NSEC */
		return DeeTime_NewUnix(DIR_stat_GET_RAW_BTIME_SEC(&self->odi_st), 0);
#endif /* !DIR_stat_GET_RAW_BTIME_NSEC */
	}
#else /* ... */
#define diriter_get_d_birthtime_IS_STUB
#endif /* !... */
	(void)self;
	diriter_unbound_attr("d_birthtime");
#ifdef NEED_err
#undef NEED_err
err:
#endif /* NEED_err */
	return NULL;
}




PRIVATE NONNULL((1, 2)) void DCALL
diriter_visit(DeeDirIteratorObject *__restrict self, Dee_visit_t proc, void *arg) {
	/* Not needed (and mustn't be enabled; `diriter_visit' is re-used as `dir_visit'!) */
	/*Dee_XVisit(self->odi_pathstr);*/
	Dee_Visit(self->odi_path);
}


/*[[[deemon (print_DEFINE_KWLIST from rt.gen.unpack)({ "path", "skipdots", "inheritfd" });]]]*/
#ifndef DEFINED_kwlist__path_skipdots_inheritfd
#define DEFINED_kwlist__path_skipdots_inheritfd
PRIVATE DEFINE_KWLIST(kwlist__path_skipdots_inheritfd, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("skipdots", 0x5f2962eb, 0x59ddb5f9342c7b50), KEX("inheritfd", 0x3b58e99c, 0x17889f39738f4141), KEND });
#endif /* !DEFINED_kwlist__path_skipdots_inheritfd */
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) int DCALL
diriter_init_kw(DeeDirIteratorObject *__restrict self,
                size_t argc, DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("_DirIterator", params: "
	DeeObject *path:?X4?GDIR?Dstring?DFile?Dint;
	bool skipdots  = true;
	bool inheritfd = false;
", docStringPrefix: "posix");]]]*/
#define posix__DirIterator_params "path:?X4?GDIR?Dstring?DFile?Dint,skipdots=!t,inheritfd=!f"
	struct {
		DeeObject *path;
		bool skipdots;
		bool inheritfd;
	} args;
	args.skipdots = true;
	args.inheritfd = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_skipdots_inheritfd, "o|bb:_DirIterator", &args))
		goto err;
/*[[[end]]]*/
	if (Dee_TYPE(args.path) == &DeeDir_Type) {
		DeeDirObject *dir = (DeeDirObject *)args.path;
		args.skipdots  = dir->d_skipdots;
		args.inheritfd = dir->d_inheritfd;
		args.path      = dir->d_path;
	}
	return directory_open(self, args.path, args.skipdots, args.inheritfd);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeDirObject *DCALL
diriter_getseq(DeeDirIteratorObject *__restrict self) {
	DREF DeeDirObject *result;
	result = DeeObject_MALLOC(DeeDirObject);
	if likely(result) {
		DeeObject_Init(result, &DeeDir_Type);
		result->d_path      = self->odi_path;
		result->d_skipdots  = self->odi_skipdots;
		result->d_inheritfd = false;
		Dee_Incref(result->d_path);
	}
	return result;
}

PRIVATE struct type_getset tpconst diriter_getsets[] = {
	TYPE_GETTER_F("seq", &diriter_getseq, METHOD_FNOREFESCAPE, "->?GDIR"),
	TYPE_GETTER_F("pathstr", &diriter_getpathstr, METHOD_FNOREFESCAPE, "->?Dstring"),
	TYPE_GETTER_F("d_name", &diriter_get_d_name, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "The name of the current file"),
	TYPE_GETTER_F("d_fullname", &diriter_get_d_fullname, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "The full (absolute) filename of the current file"),
	TYPE_GETTER_F("d_type", &diriter_get_d_type, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "The type of the current file (one of ${DT_*})"),
	TYPE_GETTER_F("d_ino", &diriter_get_d_ino, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "The inode number of the current file"),
	TYPE_GETTER_F("d_namlen", &diriter_get_d_namlen, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Length of ?#d_name (in characters)"),
	TYPE_GETTER_F("d_reclen", &diriter_get_d_reclen, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Size of the directory record (in bytes)"),
	TYPE_GETTER_F("d_off", &diriter_get_d_off, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Offset of the next directory entry (non-portable!)"),

	/* Additional (stat-like) fields (to take advantage of info that windows gives us) */
	TYPE_GETTER_F("d_dev", &diriter_get_d_dev, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_dev?Gstat"),
	TYPE_GETTER_F("d_mode", &diriter_get_d_mode, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_mode?Gstat"),
	TYPE_GETTER_F("d_nlink", &diriter_get_d_nlink, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_nlink?Gstat"),
	TYPE_GETTER_F("d_uid", &diriter_get_d_uid, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_uid?Gstat"),
	TYPE_GETTER_F("d_gid", &diriter_get_d_gid, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_gid?Gstat"),
	TYPE_GETTER_F("d_rdev", &diriter_get_d_rdev, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_rdev?Gstat"),
	TYPE_GETTER_F("d_size", &diriter_get_d_size, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_size?Gstat"),
	TYPE_GETTER_F("d_blocks", &diriter_get_d_blocks, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_blocks?Gstat"),
	TYPE_GETTER_F("d_blksize", &diriter_get_d_blksize, METHOD_FNOREFESCAPE, "->?Dint\ns.a. ?Ast_blksize?Gstat"),
	TYPE_GETTER_F("d_atime", &diriter_get_d_atime, METHOD_FNOREFESCAPE, "->?Etime:Time\ns.a. ?Ast_atime?Gstat"),
	TYPE_GETTER_F("d_mtime", &diriter_get_d_mtime, METHOD_FNOREFESCAPE, "->?Etime:Time\ns.a. ?Ast_mtime?Gstat"),
	TYPE_GETTER_F("d_ctime", &diriter_get_d_ctime, METHOD_FNOREFESCAPE, "->?Etime:Time\ns.a. ?Ast_ctime?Gstat"),
	TYPE_GETTER_F("d_birthtime", &diriter_get_d_birthtime, METHOD_FNOREFESCAPE, "->?Etime:Time\ns.a. ?Ast_birthtime?Gstat"),
	/* TODO: `stat->?Gstat' (returns the stat information for this directory entry) */

	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst diriter_members[] = {
	TYPE_MEMBER_FIELD_DOC("path", STRUCT_OBJECT, offsetof(DeeDirIteratorObject, odi_path),
	                      "->?X3?Dstring?DFile?Dint"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeDirIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DirIterator",
	/* .tp_doc      = */ DOC("(" posix__DirIterator_params ")\n"
	                         "\n"
	                         "next->?."),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeDirIteratorObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&diriter_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&diriter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&diriter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ diriter_getsets,
	/* .tp_members       = */ diriter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dir_copy(DeeDirObject *__restrict self,
         DeeDirObject *__restrict other) {
	self->d_path      = other->d_path;
	self->d_skipdots  = other->d_skipdots;
	self->d_inheritfd = false;
	Dee_Incref(self->d_path);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
dir_fini(DeeDirObject *__restrict self) {
	Dee_Decref(self->d_path);
}

STATIC_ASSERT(offsetof(DeeDirIteratorObject, odi_path) ==
              offsetof(DeeDirObject, d_path));
#define dir_visit   diriter_visit
#define dir_members diriter_members

PRIVATE WUNUSED NONNULL((1)) int DCALL
dir_init_kw(DeeDirObject *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
#if 1
	struct _layout {
		DeeObject *path;
		bool skipdots;
		bool inheritfd;
	};
#define RELBASE d_path
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeDirObject, d_path) - offsetof(DeeDirObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, path));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeDirObject, d_path) - offsetof(DeeDirObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, path));
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeDirObject, d_skipdots) - offsetof(DeeDirObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, skipdots));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeDirObject, d_skipdots) - offsetof(DeeDirObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, skipdots));
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeDirObject, d_inheritfd) - offsetof(DeeDirObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, inheritfd));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeDirObject, d_inheritfd) - offsetof(DeeDirObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, inheritfd));
	self->d_skipdots = true;
	self->d_inheritfd = false;
#define posix_opendir_params "path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!f"
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_skipdots_inheritfd, "o|bb:opendir", &self->RELBASE))
		goto err;
#undef RELBASE
#else
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("opendir", params: "
	DeeObject *path:?X3?Dstring?DFile?Dint;
	bool skipdots  = true;
	bool inheritfd = false;
", docStringPrefix: "posix");]]]*/
#define posix_opendir_params "path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!f"
	struct {
		DeeObject *path;
		bool skipdots;
		bool inheritfd;
	} args;
	args.skipdots = true;
	args.inheritfd = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_skipdots_inheritfd, "o|bb:opendir", &args))
		goto err;
/*[[[end]]]*/
	self->d_path      = args.path;
	self->d_skipdots  = args.skipdots;
	self->d_inheritfd = args.inheritfd;
#endif
	Dee_Incref(self->d_path);
	return 0;
err:
	return -1;
}

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("fdopendir", """
	DeeObject *path:?X3?Dstring?DFile?Dint;
	bool skipdots  = true;
	bool inheritfd = true;
""", libname: "posix");]]]*/
#define posix_fdopendir_params "path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!t"
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_fdopendir_f_impl(DeeObject *path, bool skipdots, bool inheritfd);
#ifndef DEFINED_kwlist__path_skipdots_inheritfd
#define DEFINED_kwlist__path_skipdots_inheritfd
PRIVATE DEFINE_KWLIST(kwlist__path_skipdots_inheritfd, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("skipdots", 0x5f2962eb, 0x59ddb5f9342c7b50), KEX("inheritfd", 0x3b58e99c, 0x17889f39738f4141), KEND });
#endif /* !DEFINED_kwlist__path_skipdots_inheritfd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fdopendir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
		bool skipdots;
		bool inheritfd;
	} args;
	args.skipdots = true;
	args.inheritfd = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_skipdots_inheritfd, "o|bb:fdopendir", &args))
		goto err;
	return posix_fdopendir_f_impl(args.path, args.skipdots, args.inheritfd);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fdopendir, &posix_fdopendir_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_fdopendir_f_impl(DeeObject *path, bool skipdots, bool inheritfd)
/*[[[end]]]*/
{
	DREF DeeDirObject *result;
	result = DeeObject_MALLOC(DeeDirObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeDir_Type);
	Dee_Incref(path);
	result->d_path      = path;
	result->d_skipdots  = skipdots;
	result->d_inheritfd = inheritfd;
	return (DREF DeeObject *)result;
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeDirIteratorObject *DCALL
dir_iter(DeeDirObject *__restrict self) {
	DREF DeeDirIteratorObject *result;
	result = DeeObject_MALLOC(DeeDirIteratorObject);
	if unlikely(!result)
		goto done;
	if unlikely(directory_open(result,
	                           self->d_path,
	                           self->d_skipdots,
	                           self->d_inheritfd)) {
		DeeObject_FREE(result);
		result = NULL;
		goto done;
	}
	DeeObject_Init(result, &DeeDirIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq dir_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dir_iter,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};


PRIVATE struct type_member tpconst dir_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeDirIterator_Type),
	TYPE_MEMBER_CONST("ItemType", &DeeDirIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeDir_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "DIR",
	/* .tp_doc      = */ DOC("(" posix_opendir_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)&dir_copy,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)&dir_copy,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeDirObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&dir_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dir_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dir_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dir_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dir_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dir_class_members
};

DECL_END

#endif /* !GUARD_DEX_POSIX_P_OPENDIR_C_INL */
