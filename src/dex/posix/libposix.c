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
#ifndef GUARD_DEX_POSIX_LIBPOSIX_C
#define GUARD_DEX_POSIX_LIBPOSIX_C 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#ifndef __INTELLISENSE__
#include "p-access.c.inl"
#include "p-cpucount.c.inl"
#include "p-environ.c.inl"
#include "p-errno.c.inl"
#include "p-exit.c.inl"
#include "p-fd.c.inl"
#include "p-fs.c.inl"
#include "p-open.c.inl"
#include "p-opendir.c.inl"
#include "p-pipe.c.inl"
#include "p-readlink.c.inl"
#include "p-readwrite.c.inl"
#include "p-realpath.c.inl"
#include "p-rename.c.inl"
#include "p-sched.c.inl"
#include "p-stat.c.inl"
#include "p-sync.c.inl"
#include "p-truncate.c.inl"

/**/
#include "p-pwd.c.inl" /* This one has to come after "p-environ.c.inl" */

/* Include p-ondemand.c.inl last, since it defined functions on-demand */
#include "p-ondemand.c.inl"

/* Include p-fs-deprecated.c.inl last, since this once #undef's a
 * bunch of stuff that may be needed by other components. */
#include "p-fs-deprecated.c.inl"
#endif /* !__INTELLISENSE__ */

#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/seq.h>
#include <deemon/string.h>


DECL_BEGIN


#if defined(NEED_libposix_get_dfd_filename) || defined(__INTELLISENSE__)
#undef NEED_libposix_get_dfd_filename
INTERN WUNUSED DREF /*String*/ DeeObject *DCALL
libposix_get_dfd_filename(int dfd, /*utf-8*/ char const *filename, int atflags) {
	(void)dfd;
	(void)filename;
	(void)atflags;
	/* TODO: `joinpath(frealpath(dfd), filename)' */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}
#endif /* NEED_libposix_get_dfd_filename */

#ifndef CONFIG_HAVE_errno
#define posix_errno_USE_STUB
#endif /* !CONFIG_HAVE_errno */

#undef posix_lstat_USE_STUB
#ifndef posix_stat_HAVE_lstat
#define posix_lstat_USE_STUB
#endif /* !posix_stat_HAVE_lstat */

/*[[[deemon
local ALL_STUBS = {
	("posix_truncate_USE_STUB",     { "truncate" }),
	("posix_ftruncate_USE_STUB",    { "ftruncate" }),
	("posix_pipe_USE_STUB",         { "pipe" }),
	("posix_pipe2_USE_STUB",        { "pipe2" }),
	("posix_strerror_USE_STUB",     { "strerror" }),
	("posix_strerrorname_USE_STUB", { "strerrorname" }),
	("posix_open_USE_STUB",         { "open", "_open" }),
	("posix_creat_USE_STUB",        { "creat", "_creat" }),
	("posix_close_USE_STUB",        { "close" }),
	("posix_isatty_USE_STUB",       { "isatty" }),
	("posix_umask_USE_STUB",        { "umask" }),
	("posix_dup_USE_STUB",          { "dup" }),
	("posix_dup2_USE_STUB",         { "dup2" }),
	("posix_dup3_USE_STUB",         { "dup3" }),
	("posix_errno_USE_STUB",        { "errno" }),
	("posix_fsync_USE_STUB",        { "fsync" }),
	("posix_fdatasync_USE_STUB",    { "fdatasync" }),
	("posix_system_USE_STUB",       { "system" }),
	("posix_getpid_USE_STUB",       { "getpid" }),
	("posix_read_USE_STUB",         { "read" }),
	("posix_lseek_USE_STUB",        { "lseek" }),
	("posix_write_USE_STUB",        { "write" }),
	("posix_pread_USE_STUB",        { "pread" }),
	("posix_pwrite_USE_STUB",       { "pwrite" }),
	("posix_access_USE_STUB",       { "access" }),
	("posix_euidaccess_USE_STUB",   { "euidaccess" }),
	("posix_cpu_count_USE_STUB",    { "cpu_count" }),
	("posix_opendir_USE_STUB",      { "dirent", "DIR", "opendir", "fdopendir" }),
	("posix_getenv_USE_STUB",       { "getenv" }),
	("posix_setenv_USE_STUB",       { "setenv", "putenv" }),
	("posix_unsetenv_USE_STUB",     { "unsetenv" }),
	("posix_clearenv_USE_STUB",     { "clearenv" }),
	("posix_enumenv_USE_STUB",      { "environ" }),
	("posix_stat_USE_STUB",         { "stat" }),
	("posix_lstat_USE_STUB",        { "lstat" }),
	("posix_stat_get_dev_IS_STUB", { "stat.st_dev" }),
	("posix_stat_get_ino_IS_STUB", { "stat.st_ino" }),
	("posix_stat_get_mode_IS_STUB", { "stat.st_mode" }),
	("posix_stat_get_nlink_IS_STUB", { "stat.st_nlink" }),
	("posix_stat_get_uid_IS_STUB", { "stat.st_uid" }),
	("posix_stat_get_gid_IS_STUB", { "stat.st_gid" }),
	("posix_stat_get_rdev_IS_STUB", { "stat.st_rdev" }),
	("posix_stat_get_size_IS_STUB", { "stat.st_size" }),
	("posix_stat_get_atime_IS_STUB", { "stat.st_atime" }),
	("posix_stat_get_mtime_IS_STUB", { "stat.st_mtime" }),
	("posix_stat_get_ctime_IS_STUB", { "stat.st_ctime" }),
	("posix_stat_get_birthtime_IS_STUB", { "stat.st_birthtime" }),
	("posix_stat_isdir_IS_STUB", { "stat.isdir" }),
	("posix_stat_ischr_IS_STUB", { "stat.ischr" }),
	("posix_stat_isblk_IS_STUB", { "stat.isblk" }),
	("posix_stat_isdev_IS_STUB", { "stat.isdev" }),
	("posix_stat_isreg_IS_STUB", { "stat.isreg" }),
	("posix_stat_isfifo_IS_STUB", { "stat.isfifo" }),
	("posix_stat_islnk_IS_STUB", { "stat.islnk" }),
	("posix_stat_issock_IS_STUB", { "stat.issock" }),
	("stat_class_isexe_IS_STUB", { "stat.isexe" }),
	("stat_class_ishidden_IS_STUB", { "stat.ishidden" }),
	("posix_gethostname_USE_STUB", { "gethostname" }),
	("posix_chdir_USE_STUB", { "chdir" }),
	("posix_fchdir_USE_STUB", { "fchdir" }),
	("posix_fchdirat_USE_STUB", { "fchdirat" }),
	("posix_unlink_USE_STUB", { "unlink" }),
	("posix_rmdir_USE_STUB", { "rmdir" }),
	("posix_remove_USE_STUB", { "remove" }),
	("posix_unlinkat_USE_STUB", { "unlinkat" }),
	("posix_rmdirat_USE_STUB", { "rmdirat" }),
	("posix_removeat_USE_STUB", { "removeat" }),
	("posix_readlink_USE_STUB", { "readlink" }),
	("posix_freadlink_USE_STUB", { "freadlink" }),
	("posix_readlinkat_USE_STUB", { "readlinkat" }),
	("posix_rename_USE_STUB", { "rename" }),
	("posix_frename_USE_STUB", { "frename" }),
	("posix_renameat_USE_STUB", { "renameat" }),
	("posix_renameat2_USE_STUB", { "renameat2" }),
	("posix_link_USE_STUB", { "link" }),
	("posix_flink_USE_STUB", { "flink" }),
	("posix_linkat_USE_STUB", { "linkat" }),
}.sorted();
for (local test, functions: ALL_STUBS) {
	functions = "\0".join(functions) + "\0";
	print("#ifdef ", test);
	print("#define len_", test, " +", #functions);
	print("#define str_", test, " '", "', '".join(for (local c: functions) c.encode("c-escape")), "',");
	print("#else /" "* ", test, " *" "/");
	print("#define len_", test, " /" "* nothing *" "/");
	print("#define str_", test, " /" "* nothing *" "/");
	print("#endif /" "* !", test, " *" "/");
}
print("#define POSIX_STUBS_TOTLEN 0 \\");
for (local test, none: ALL_STUBS)
	print("\tlen_", test, " \\");
print("/" "**" "/");
print("#if POSIX_STUBS_TOTLEN != 0");
print("PRIVATE struct {");
print("	OBJECT_HEAD");
print("	struct string_utf *s_data;");
print("	dhash_t            s_hash;");
print("	size_t             s_len;");
print("	char               s_str[POSIX_STUBS_TOTLEN];");
print("	struct string_utf  s_utf;");
print("} posix_missing_features = {");
print("	OBJECT_HEAD_INIT(&DeeString_Type),");
print("	&posix_missing_features.s_utf,");
print("	DEE_STRING_HASH_UNSET,");
print("	POSIX_STUBS_TOTLEN - 1, {");
for (local test, none: ALL_STUBS)
	print("\t\tstr_", test);
print("	},");
print("	{ Dee_STRING_WIDTH_1BYTE,");
print("	  Dee_STRING_UTF_FASCII,");
print("	  { (size_t *)posix_missing_features.s_str },");
print("	  posix_missing_features.s_str }");
print("};");
print("#else /" "* POSIX_STUBS_TOTLEN != 0 *" "/");
print("#undef POSIX_STUBS_TOTLEN");
print("#define POSIX_STUBS_TOTLEN 0");
print("#endif /" "* POSIX_STUBS_TOTLEN == 0 *" "/");
]]]*/
#ifdef posix_access_USE_STUB
#define len_posix_access_USE_STUB +7
#define str_posix_access_USE_STUB 'a', 'c', 'c', 'e', 's', 's', '\0',
#else /* posix_access_USE_STUB */
#define len_posix_access_USE_STUB /* nothing */
#define str_posix_access_USE_STUB /* nothing */
#endif /* !posix_access_USE_STUB */
#ifdef posix_chdir_USE_STUB
#define len_posix_chdir_USE_STUB +6
#define str_posix_chdir_USE_STUB 'c', 'h', 'd', 'i', 'r', '\0',
#else /* posix_chdir_USE_STUB */
#define len_posix_chdir_USE_STUB /* nothing */
#define str_posix_chdir_USE_STUB /* nothing */
#endif /* !posix_chdir_USE_STUB */
#ifdef posix_clearenv_USE_STUB
#define len_posix_clearenv_USE_STUB +9
#define str_posix_clearenv_USE_STUB 'c', 'l', 'e', 'a', 'r', 'e', 'n', 'v', '\0',
#else /* posix_clearenv_USE_STUB */
#define len_posix_clearenv_USE_STUB /* nothing */
#define str_posix_clearenv_USE_STUB /* nothing */
#endif /* !posix_clearenv_USE_STUB */
#ifdef posix_close_USE_STUB
#define len_posix_close_USE_STUB +6
#define str_posix_close_USE_STUB 'c', 'l', 'o', 's', 'e', '\0',
#else /* posix_close_USE_STUB */
#define len_posix_close_USE_STUB /* nothing */
#define str_posix_close_USE_STUB /* nothing */
#endif /* !posix_close_USE_STUB */
#ifdef posix_cpu_count_USE_STUB
#define len_posix_cpu_count_USE_STUB +10
#define str_posix_cpu_count_USE_STUB 'c', 'p', 'u', '_', 'c', 'o', 'u', 'n', 't', '\0',
#else /* posix_cpu_count_USE_STUB */
#define len_posix_cpu_count_USE_STUB /* nothing */
#define str_posix_cpu_count_USE_STUB /* nothing */
#endif /* !posix_cpu_count_USE_STUB */
#ifdef posix_creat_USE_STUB
#define len_posix_creat_USE_STUB +13
#define str_posix_creat_USE_STUB 'c', 'r', 'e', 'a', 't', '\0', '_', 'c', 'r', 'e', 'a', 't', '\0',
#else /* posix_creat_USE_STUB */
#define len_posix_creat_USE_STUB /* nothing */
#define str_posix_creat_USE_STUB /* nothing */
#endif /* !posix_creat_USE_STUB */
#ifdef posix_dup2_USE_STUB
#define len_posix_dup2_USE_STUB +5
#define str_posix_dup2_USE_STUB 'd', 'u', 'p', '2', '\0',
#else /* posix_dup2_USE_STUB */
#define len_posix_dup2_USE_STUB /* nothing */
#define str_posix_dup2_USE_STUB /* nothing */
#endif /* !posix_dup2_USE_STUB */
#ifdef posix_dup3_USE_STUB
#define len_posix_dup3_USE_STUB +5
#define str_posix_dup3_USE_STUB 'd', 'u', 'p', '3', '\0',
#else /* posix_dup3_USE_STUB */
#define len_posix_dup3_USE_STUB /* nothing */
#define str_posix_dup3_USE_STUB /* nothing */
#endif /* !posix_dup3_USE_STUB */
#ifdef posix_dup_USE_STUB
#define len_posix_dup_USE_STUB +4
#define str_posix_dup_USE_STUB 'd', 'u', 'p', '\0',
#else /* posix_dup_USE_STUB */
#define len_posix_dup_USE_STUB /* nothing */
#define str_posix_dup_USE_STUB /* nothing */
#endif /* !posix_dup_USE_STUB */
#ifdef posix_enumenv_USE_STUB
#define len_posix_enumenv_USE_STUB +8
#define str_posix_enumenv_USE_STUB 'e', 'n', 'v', 'i', 'r', 'o', 'n', '\0',
#else /* posix_enumenv_USE_STUB */
#define len_posix_enumenv_USE_STUB /* nothing */
#define str_posix_enumenv_USE_STUB /* nothing */
#endif /* !posix_enumenv_USE_STUB */
#ifdef posix_errno_USE_STUB
#define len_posix_errno_USE_STUB +6
#define str_posix_errno_USE_STUB 'e', 'r', 'r', 'n', 'o', '\0',
#else /* posix_errno_USE_STUB */
#define len_posix_errno_USE_STUB /* nothing */
#define str_posix_errno_USE_STUB /* nothing */
#endif /* !posix_errno_USE_STUB */
#ifdef posix_euidaccess_USE_STUB
#define len_posix_euidaccess_USE_STUB +11
#define str_posix_euidaccess_USE_STUB 'e', 'u', 'i', 'd', 'a', 'c', 'c', 'e', 's', 's', '\0',
#else /* posix_euidaccess_USE_STUB */
#define len_posix_euidaccess_USE_STUB /* nothing */
#define str_posix_euidaccess_USE_STUB /* nothing */
#endif /* !posix_euidaccess_USE_STUB */
#ifdef posix_fchdir_USE_STUB
#define len_posix_fchdir_USE_STUB +7
#define str_posix_fchdir_USE_STUB 'f', 'c', 'h', 'd', 'i', 'r', '\0',
#else /* posix_fchdir_USE_STUB */
#define len_posix_fchdir_USE_STUB /* nothing */
#define str_posix_fchdir_USE_STUB /* nothing */
#endif /* !posix_fchdir_USE_STUB */
#ifdef posix_fchdirat_USE_STUB
#define len_posix_fchdirat_USE_STUB +9
#define str_posix_fchdirat_USE_STUB 'f', 'c', 'h', 'd', 'i', 'r', 'a', 't', '\0',
#else /* posix_fchdirat_USE_STUB */
#define len_posix_fchdirat_USE_STUB /* nothing */
#define str_posix_fchdirat_USE_STUB /* nothing */
#endif /* !posix_fchdirat_USE_STUB */
#ifdef posix_fdatasync_USE_STUB
#define len_posix_fdatasync_USE_STUB +10
#define str_posix_fdatasync_USE_STUB 'f', 'd', 'a', 't', 'a', 's', 'y', 'n', 'c', '\0',
#else /* posix_fdatasync_USE_STUB */
#define len_posix_fdatasync_USE_STUB /* nothing */
#define str_posix_fdatasync_USE_STUB /* nothing */
#endif /* !posix_fdatasync_USE_STUB */
#ifdef posix_freadlink_USE_STUB
#define len_posix_freadlink_USE_STUB +10
#define str_posix_freadlink_USE_STUB 'f', 'r', 'e', 'a', 'd', 'l', 'i', 'n', 'k', '\0',
#else /* posix_freadlink_USE_STUB */
#define len_posix_freadlink_USE_STUB /* nothing */
#define str_posix_freadlink_USE_STUB /* nothing */
#endif /* !posix_freadlink_USE_STUB */
#ifdef posix_frename_USE_STUB
#define len_posix_frename_USE_STUB +8
#define str_posix_frename_USE_STUB 'f', 'r', 'e', 'n', 'a', 'm', 'e', '\0',
#else /* posix_frename_USE_STUB */
#define len_posix_frename_USE_STUB /* nothing */
#define str_posix_frename_USE_STUB /* nothing */
#endif /* !posix_frename_USE_STUB */
#ifdef posix_fsync_USE_STUB
#define len_posix_fsync_USE_STUB +6
#define str_posix_fsync_USE_STUB 'f', 's', 'y', 'n', 'c', '\0',
#else /* posix_fsync_USE_STUB */
#define len_posix_fsync_USE_STUB /* nothing */
#define str_posix_fsync_USE_STUB /* nothing */
#endif /* !posix_fsync_USE_STUB */
#ifdef posix_ftruncate_USE_STUB
#define len_posix_ftruncate_USE_STUB +10
#define str_posix_ftruncate_USE_STUB 'f', 't', 'r', 'u', 'n', 'c', 'a', 't', 'e', '\0',
#else /* posix_ftruncate_USE_STUB */
#define len_posix_ftruncate_USE_STUB /* nothing */
#define str_posix_ftruncate_USE_STUB /* nothing */
#endif /* !posix_ftruncate_USE_STUB */
#ifdef posix_getenv_USE_STUB
#define len_posix_getenv_USE_STUB +7
#define str_posix_getenv_USE_STUB 'g', 'e', 't', 'e', 'n', 'v', '\0',
#else /* posix_getenv_USE_STUB */
#define len_posix_getenv_USE_STUB /* nothing */
#define str_posix_getenv_USE_STUB /* nothing */
#endif /* !posix_getenv_USE_STUB */
#ifdef posix_gethostname_USE_STUB
#define len_posix_gethostname_USE_STUB +12
#define str_posix_gethostname_USE_STUB 'g', 'e', 't', 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', '\0',
#else /* posix_gethostname_USE_STUB */
#define len_posix_gethostname_USE_STUB /* nothing */
#define str_posix_gethostname_USE_STUB /* nothing */
#endif /* !posix_gethostname_USE_STUB */
#ifdef posix_getpid_USE_STUB
#define len_posix_getpid_USE_STUB +7
#define str_posix_getpid_USE_STUB 'g', 'e', 't', 'p', 'i', 'd', '\0',
#else /* posix_getpid_USE_STUB */
#define len_posix_getpid_USE_STUB /* nothing */
#define str_posix_getpid_USE_STUB /* nothing */
#endif /* !posix_getpid_USE_STUB */
#ifdef posix_isatty_USE_STUB
#define len_posix_isatty_USE_STUB +7
#define str_posix_isatty_USE_STUB 'i', 's', 'a', 't', 't', 'y', '\0',
#else /* posix_isatty_USE_STUB */
#define len_posix_isatty_USE_STUB /* nothing */
#define str_posix_isatty_USE_STUB /* nothing */
#endif /* !posix_isatty_USE_STUB */
#ifdef posix_lseek_USE_STUB
#define len_posix_lseek_USE_STUB +6
#define str_posix_lseek_USE_STUB 'l', 's', 'e', 'e', 'k', '\0',
#else /* posix_lseek_USE_STUB */
#define len_posix_lseek_USE_STUB /* nothing */
#define str_posix_lseek_USE_STUB /* nothing */
#endif /* !posix_lseek_USE_STUB */
#ifdef posix_lstat_USE_STUB
#define len_posix_lstat_USE_STUB +6
#define str_posix_lstat_USE_STUB 'l', 's', 't', 'a', 't', '\0',
#else /* posix_lstat_USE_STUB */
#define len_posix_lstat_USE_STUB /* nothing */
#define str_posix_lstat_USE_STUB /* nothing */
#endif /* !posix_lstat_USE_STUB */
#ifdef posix_open_USE_STUB
#define len_posix_open_USE_STUB +11
#define str_posix_open_USE_STUB 'o', 'p', 'e', 'n', '\0', '_', 'o', 'p', 'e', 'n', '\0',
#else /* posix_open_USE_STUB */
#define len_posix_open_USE_STUB /* nothing */
#define str_posix_open_USE_STUB /* nothing */
#endif /* !posix_open_USE_STUB */
#ifdef posix_opendir_USE_STUB
#define len_posix_opendir_USE_STUB +29
#define str_posix_opendir_USE_STUB 'd', 'i', 'r', 'e', 'n', 't', '\0', 'D', 'I', 'R', '\0', 'o', 'p', 'e', 'n', 'd', 'i', 'r', '\0', 'f', 'd', 'o', 'p', 'e', 'n', 'd', 'i', 'r', '\0',
#else /* posix_opendir_USE_STUB */
#define len_posix_opendir_USE_STUB /* nothing */
#define str_posix_opendir_USE_STUB /* nothing */
#endif /* !posix_opendir_USE_STUB */
#ifdef posix_pipe2_USE_STUB
#define len_posix_pipe2_USE_STUB +6
#define str_posix_pipe2_USE_STUB 'p', 'i', 'p', 'e', '2', '\0',
#else /* posix_pipe2_USE_STUB */
#define len_posix_pipe2_USE_STUB /* nothing */
#define str_posix_pipe2_USE_STUB /* nothing */
#endif /* !posix_pipe2_USE_STUB */
#ifdef posix_pipe_USE_STUB
#define len_posix_pipe_USE_STUB +5
#define str_posix_pipe_USE_STUB 'p', 'i', 'p', 'e', '\0',
#else /* posix_pipe_USE_STUB */
#define len_posix_pipe_USE_STUB /* nothing */
#define str_posix_pipe_USE_STUB /* nothing */
#endif /* !posix_pipe_USE_STUB */
#ifdef posix_pread_USE_STUB
#define len_posix_pread_USE_STUB +6
#define str_posix_pread_USE_STUB 'p', 'r', 'e', 'a', 'd', '\0',
#else /* posix_pread_USE_STUB */
#define len_posix_pread_USE_STUB /* nothing */
#define str_posix_pread_USE_STUB /* nothing */
#endif /* !posix_pread_USE_STUB */
#ifdef posix_pwrite_USE_STUB
#define len_posix_pwrite_USE_STUB +7
#define str_posix_pwrite_USE_STUB 'p', 'w', 'r', 'i', 't', 'e', '\0',
#else /* posix_pwrite_USE_STUB */
#define len_posix_pwrite_USE_STUB /* nothing */
#define str_posix_pwrite_USE_STUB /* nothing */
#endif /* !posix_pwrite_USE_STUB */
#ifdef posix_read_USE_STUB
#define len_posix_read_USE_STUB +5
#define str_posix_read_USE_STUB 'r', 'e', 'a', 'd', '\0',
#else /* posix_read_USE_STUB */
#define len_posix_read_USE_STUB /* nothing */
#define str_posix_read_USE_STUB /* nothing */
#endif /* !posix_read_USE_STUB */
#ifdef posix_readlink_USE_STUB
#define len_posix_readlink_USE_STUB +9
#define str_posix_readlink_USE_STUB 'r', 'e', 'a', 'd', 'l', 'i', 'n', 'k', '\0',
#else /* posix_readlink_USE_STUB */
#define len_posix_readlink_USE_STUB /* nothing */
#define str_posix_readlink_USE_STUB /* nothing */
#endif /* !posix_readlink_USE_STUB */
#ifdef posix_readlinkat_USE_STUB
#define len_posix_readlinkat_USE_STUB +11
#define str_posix_readlinkat_USE_STUB 'r', 'e', 'a', 'd', 'l', 'i', 'n', 'k', 'a', 't', '\0',
#else /* posix_readlinkat_USE_STUB */
#define len_posix_readlinkat_USE_STUB /* nothing */
#define str_posix_readlinkat_USE_STUB /* nothing */
#endif /* !posix_readlinkat_USE_STUB */
#ifdef posix_remove_USE_STUB
#define len_posix_remove_USE_STUB +7
#define str_posix_remove_USE_STUB 'r', 'e', 'm', 'o', 'v', 'e', '\0',
#else /* posix_remove_USE_STUB */
#define len_posix_remove_USE_STUB /* nothing */
#define str_posix_remove_USE_STUB /* nothing */
#endif /* !posix_remove_USE_STUB */
#ifdef posix_removeat_USE_STUB
#define len_posix_removeat_USE_STUB +9
#define str_posix_removeat_USE_STUB 'r', 'e', 'm', 'o', 'v', 'e', 'a', 't', '\0',
#else /* posix_removeat_USE_STUB */
#define len_posix_removeat_USE_STUB /* nothing */
#define str_posix_removeat_USE_STUB /* nothing */
#endif /* !posix_removeat_USE_STUB */
#ifdef posix_rename_USE_STUB
#define len_posix_rename_USE_STUB +7
#define str_posix_rename_USE_STUB 'r', 'e', 'n', 'a', 'm', 'e', '\0',
#else /* posix_rename_USE_STUB */
#define len_posix_rename_USE_STUB /* nothing */
#define str_posix_rename_USE_STUB /* nothing */
#endif /* !posix_rename_USE_STUB */
#ifdef posix_renameat2_USE_STUB
#define len_posix_renameat2_USE_STUB +10
#define str_posix_renameat2_USE_STUB 'r', 'e', 'n', 'a', 'm', 'e', 'a', 't', '2', '\0',
#else /* posix_renameat2_USE_STUB */
#define len_posix_renameat2_USE_STUB /* nothing */
#define str_posix_renameat2_USE_STUB /* nothing */
#endif /* !posix_renameat2_USE_STUB */
#ifdef posix_renameat_USE_STUB
#define len_posix_renameat_USE_STUB +9
#define str_posix_renameat_USE_STUB 'r', 'e', 'n', 'a', 'm', 'e', 'a', 't', '\0',
#else /* posix_renameat_USE_STUB */
#define len_posix_renameat_USE_STUB /* nothing */
#define str_posix_renameat_USE_STUB /* nothing */
#endif /* !posix_renameat_USE_STUB */
#ifdef posix_rmdir_USE_STUB
#define len_posix_rmdir_USE_STUB +6
#define str_posix_rmdir_USE_STUB 'r', 'm', 'd', 'i', 'r', '\0',
#else /* posix_rmdir_USE_STUB */
#define len_posix_rmdir_USE_STUB /* nothing */
#define str_posix_rmdir_USE_STUB /* nothing */
#endif /* !posix_rmdir_USE_STUB */
#ifdef posix_rmdirat_USE_STUB
#define len_posix_rmdirat_USE_STUB +8
#define str_posix_rmdirat_USE_STUB 'r', 'm', 'd', 'i', 'r', 'a', 't', '\0',
#else /* posix_rmdirat_USE_STUB */
#define len_posix_rmdirat_USE_STUB /* nothing */
#define str_posix_rmdirat_USE_STUB /* nothing */
#endif /* !posix_rmdirat_USE_STUB */
#ifdef posix_setenv_USE_STUB
#define len_posix_setenv_USE_STUB +14
#define str_posix_setenv_USE_STUB 's', 'e', 't', 'e', 'n', 'v', '\0', 'p', 'u', 't', 'e', 'n', 'v', '\0',
#else /* posix_setenv_USE_STUB */
#define len_posix_setenv_USE_STUB /* nothing */
#define str_posix_setenv_USE_STUB /* nothing */
#endif /* !posix_setenv_USE_STUB */
#ifdef posix_stat_USE_STUB
#define len_posix_stat_USE_STUB +5
#define str_posix_stat_USE_STUB 's', 't', 'a', 't', '\0',
#else /* posix_stat_USE_STUB */
#define len_posix_stat_USE_STUB /* nothing */
#define str_posix_stat_USE_STUB /* nothing */
#endif /* !posix_stat_USE_STUB */
#ifdef posix_stat_get_atime_IS_STUB
#define len_posix_stat_get_atime_IS_STUB +14
#define str_posix_stat_get_atime_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'a', 't', 'i', 'm', 'e', '\0',
#else /* posix_stat_get_atime_IS_STUB */
#define len_posix_stat_get_atime_IS_STUB /* nothing */
#define str_posix_stat_get_atime_IS_STUB /* nothing */
#endif /* !posix_stat_get_atime_IS_STUB */
#ifdef posix_stat_get_birthtime_IS_STUB
#define len_posix_stat_get_birthtime_IS_STUB +18
#define str_posix_stat_get_birthtime_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'b', 'i', 'r', 't', 'h', 't', 'i', 'm', 'e', '\0',
#else /* posix_stat_get_birthtime_IS_STUB */
#define len_posix_stat_get_birthtime_IS_STUB /* nothing */
#define str_posix_stat_get_birthtime_IS_STUB /* nothing */
#endif /* !posix_stat_get_birthtime_IS_STUB */
#ifdef posix_stat_get_ctime_IS_STUB
#define len_posix_stat_get_ctime_IS_STUB +14
#define str_posix_stat_get_ctime_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'c', 't', 'i', 'm', 'e', '\0',
#else /* posix_stat_get_ctime_IS_STUB */
#define len_posix_stat_get_ctime_IS_STUB /* nothing */
#define str_posix_stat_get_ctime_IS_STUB /* nothing */
#endif /* !posix_stat_get_ctime_IS_STUB */
#ifdef posix_stat_get_dev_IS_STUB
#define len_posix_stat_get_dev_IS_STUB +12
#define str_posix_stat_get_dev_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'd', 'e', 'v', '\0',
#else /* posix_stat_get_dev_IS_STUB */
#define len_posix_stat_get_dev_IS_STUB /* nothing */
#define str_posix_stat_get_dev_IS_STUB /* nothing */
#endif /* !posix_stat_get_dev_IS_STUB */
#ifdef posix_stat_get_gid_IS_STUB
#define len_posix_stat_get_gid_IS_STUB +12
#define str_posix_stat_get_gid_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'g', 'i', 'd', '\0',
#else /* posix_stat_get_gid_IS_STUB */
#define len_posix_stat_get_gid_IS_STUB /* nothing */
#define str_posix_stat_get_gid_IS_STUB /* nothing */
#endif /* !posix_stat_get_gid_IS_STUB */
#ifdef posix_stat_get_ino_IS_STUB
#define len_posix_stat_get_ino_IS_STUB +12
#define str_posix_stat_get_ino_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'i', 'n', 'o', '\0',
#else /* posix_stat_get_ino_IS_STUB */
#define len_posix_stat_get_ino_IS_STUB /* nothing */
#define str_posix_stat_get_ino_IS_STUB /* nothing */
#endif /* !posix_stat_get_ino_IS_STUB */
#ifdef posix_stat_get_mode_IS_STUB
#define len_posix_stat_get_mode_IS_STUB +13
#define str_posix_stat_get_mode_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'm', 'o', 'd', 'e', '\0',
#else /* posix_stat_get_mode_IS_STUB */
#define len_posix_stat_get_mode_IS_STUB /* nothing */
#define str_posix_stat_get_mode_IS_STUB /* nothing */
#endif /* !posix_stat_get_mode_IS_STUB */
#ifdef posix_stat_get_mtime_IS_STUB
#define len_posix_stat_get_mtime_IS_STUB +14
#define str_posix_stat_get_mtime_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'm', 't', 'i', 'm', 'e', '\0',
#else /* posix_stat_get_mtime_IS_STUB */
#define len_posix_stat_get_mtime_IS_STUB /* nothing */
#define str_posix_stat_get_mtime_IS_STUB /* nothing */
#endif /* !posix_stat_get_mtime_IS_STUB */
#ifdef posix_stat_get_nlink_IS_STUB
#define len_posix_stat_get_nlink_IS_STUB +14
#define str_posix_stat_get_nlink_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'n', 'l', 'i', 'n', 'k', '\0',
#else /* posix_stat_get_nlink_IS_STUB */
#define len_posix_stat_get_nlink_IS_STUB /* nothing */
#define str_posix_stat_get_nlink_IS_STUB /* nothing */
#endif /* !posix_stat_get_nlink_IS_STUB */
#ifdef posix_stat_get_rdev_IS_STUB
#define len_posix_stat_get_rdev_IS_STUB +13
#define str_posix_stat_get_rdev_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'r', 'd', 'e', 'v', '\0',
#else /* posix_stat_get_rdev_IS_STUB */
#define len_posix_stat_get_rdev_IS_STUB /* nothing */
#define str_posix_stat_get_rdev_IS_STUB /* nothing */
#endif /* !posix_stat_get_rdev_IS_STUB */
#ifdef posix_stat_get_size_IS_STUB
#define len_posix_stat_get_size_IS_STUB +13
#define str_posix_stat_get_size_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 's', 'i', 'z', 'e', '\0',
#else /* posix_stat_get_size_IS_STUB */
#define len_posix_stat_get_size_IS_STUB /* nothing */
#define str_posix_stat_get_size_IS_STUB /* nothing */
#endif /* !posix_stat_get_size_IS_STUB */
#ifdef posix_stat_get_uid_IS_STUB
#define len_posix_stat_get_uid_IS_STUB +12
#define str_posix_stat_get_uid_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'u', 'i', 'd', '\0',
#else /* posix_stat_get_uid_IS_STUB */
#define len_posix_stat_get_uid_IS_STUB /* nothing */
#define str_posix_stat_get_uid_IS_STUB /* nothing */
#endif /* !posix_stat_get_uid_IS_STUB */
#ifdef posix_stat_isblk_IS_STUB
#define len_posix_stat_isblk_IS_STUB +11
#define str_posix_stat_isblk_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'b', 'l', 'k', '\0',
#else /* posix_stat_isblk_IS_STUB */
#define len_posix_stat_isblk_IS_STUB /* nothing */
#define str_posix_stat_isblk_IS_STUB /* nothing */
#endif /* !posix_stat_isblk_IS_STUB */
#ifdef posix_stat_ischr_IS_STUB
#define len_posix_stat_ischr_IS_STUB +11
#define str_posix_stat_ischr_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'c', 'h', 'r', '\0',
#else /* posix_stat_ischr_IS_STUB */
#define len_posix_stat_ischr_IS_STUB /* nothing */
#define str_posix_stat_ischr_IS_STUB /* nothing */
#endif /* !posix_stat_ischr_IS_STUB */
#ifdef posix_stat_isdev_IS_STUB
#define len_posix_stat_isdev_IS_STUB +11
#define str_posix_stat_isdev_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'd', 'e', 'v', '\0',
#else /* posix_stat_isdev_IS_STUB */
#define len_posix_stat_isdev_IS_STUB /* nothing */
#define str_posix_stat_isdev_IS_STUB /* nothing */
#endif /* !posix_stat_isdev_IS_STUB */
#ifdef posix_stat_isdir_IS_STUB
#define len_posix_stat_isdir_IS_STUB +11
#define str_posix_stat_isdir_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'd', 'i', 'r', '\0',
#else /* posix_stat_isdir_IS_STUB */
#define len_posix_stat_isdir_IS_STUB /* nothing */
#define str_posix_stat_isdir_IS_STUB /* nothing */
#endif /* !posix_stat_isdir_IS_STUB */
#ifdef posix_stat_isfifo_IS_STUB
#define len_posix_stat_isfifo_IS_STUB +12
#define str_posix_stat_isfifo_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'f', 'i', 'f', 'o', '\0',
#else /* posix_stat_isfifo_IS_STUB */
#define len_posix_stat_isfifo_IS_STUB /* nothing */
#define str_posix_stat_isfifo_IS_STUB /* nothing */
#endif /* !posix_stat_isfifo_IS_STUB */
#ifdef posix_stat_islnk_IS_STUB
#define len_posix_stat_islnk_IS_STUB +11
#define str_posix_stat_islnk_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'l', 'n', 'k', '\0',
#else /* posix_stat_islnk_IS_STUB */
#define len_posix_stat_islnk_IS_STUB /* nothing */
#define str_posix_stat_islnk_IS_STUB /* nothing */
#endif /* !posix_stat_islnk_IS_STUB */
#ifdef posix_stat_isreg_IS_STUB
#define len_posix_stat_isreg_IS_STUB +11
#define str_posix_stat_isreg_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'r', 'e', 'g', '\0',
#else /* posix_stat_isreg_IS_STUB */
#define len_posix_stat_isreg_IS_STUB /* nothing */
#define str_posix_stat_isreg_IS_STUB /* nothing */
#endif /* !posix_stat_isreg_IS_STUB */
#ifdef posix_stat_issock_IS_STUB
#define len_posix_stat_issock_IS_STUB +12
#define str_posix_stat_issock_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 's', 'o', 'c', 'k', '\0',
#else /* posix_stat_issock_IS_STUB */
#define len_posix_stat_issock_IS_STUB /* nothing */
#define str_posix_stat_issock_IS_STUB /* nothing */
#endif /* !posix_stat_issock_IS_STUB */
#ifdef posix_strerror_USE_STUB
#define len_posix_strerror_USE_STUB +9
#define str_posix_strerror_USE_STUB 's', 't', 'r', 'e', 'r', 'r', 'o', 'r', '\0',
#else /* posix_strerror_USE_STUB */
#define len_posix_strerror_USE_STUB /* nothing */
#define str_posix_strerror_USE_STUB /* nothing */
#endif /* !posix_strerror_USE_STUB */
#ifdef posix_strerrorname_USE_STUB
#define len_posix_strerrorname_USE_STUB +13
#define str_posix_strerrorname_USE_STUB 's', 't', 'r', 'e', 'r', 'r', 'o', 'r', 'n', 'a', 'm', 'e', '\0',
#else /* posix_strerrorname_USE_STUB */
#define len_posix_strerrorname_USE_STUB /* nothing */
#define str_posix_strerrorname_USE_STUB /* nothing */
#endif /* !posix_strerrorname_USE_STUB */
#ifdef posix_system_USE_STUB
#define len_posix_system_USE_STUB +7
#define str_posix_system_USE_STUB 's', 'y', 's', 't', 'e', 'm', '\0',
#else /* posix_system_USE_STUB */
#define len_posix_system_USE_STUB /* nothing */
#define str_posix_system_USE_STUB /* nothing */
#endif /* !posix_system_USE_STUB */
#ifdef posix_truncate_USE_STUB
#define len_posix_truncate_USE_STUB +9
#define str_posix_truncate_USE_STUB 't', 'r', 'u', 'n', 'c', 'a', 't', 'e', '\0',
#else /* posix_truncate_USE_STUB */
#define len_posix_truncate_USE_STUB /* nothing */
#define str_posix_truncate_USE_STUB /* nothing */
#endif /* !posix_truncate_USE_STUB */
#ifdef posix_umask_USE_STUB
#define len_posix_umask_USE_STUB +6
#define str_posix_umask_USE_STUB 'u', 'm', 'a', 's', 'k', '\0',
#else /* posix_umask_USE_STUB */
#define len_posix_umask_USE_STUB /* nothing */
#define str_posix_umask_USE_STUB /* nothing */
#endif /* !posix_umask_USE_STUB */
#ifdef posix_unlink_USE_STUB
#define len_posix_unlink_USE_STUB +7
#define str_posix_unlink_USE_STUB 'u', 'n', 'l', 'i', 'n', 'k', '\0',
#else /* posix_unlink_USE_STUB */
#define len_posix_unlink_USE_STUB /* nothing */
#define str_posix_unlink_USE_STUB /* nothing */
#endif /* !posix_unlink_USE_STUB */
#ifdef posix_unlinkat_USE_STUB
#define len_posix_unlinkat_USE_STUB +9
#define str_posix_unlinkat_USE_STUB 'u', 'n', 'l', 'i', 'n', 'k', 'a', 't', '\0',
#else /* posix_unlinkat_USE_STUB */
#define len_posix_unlinkat_USE_STUB /* nothing */
#define str_posix_unlinkat_USE_STUB /* nothing */
#endif /* !posix_unlinkat_USE_STUB */
#ifdef posix_unsetenv_USE_STUB
#define len_posix_unsetenv_USE_STUB +9
#define str_posix_unsetenv_USE_STUB 'u', 'n', 's', 'e', 't', 'e', 'n', 'v', '\0',
#else /* posix_unsetenv_USE_STUB */
#define len_posix_unsetenv_USE_STUB /* nothing */
#define str_posix_unsetenv_USE_STUB /* nothing */
#endif /* !posix_unsetenv_USE_STUB */
#ifdef posix_write_USE_STUB
#define len_posix_write_USE_STUB +6
#define str_posix_write_USE_STUB 'w', 'r', 'i', 't', 'e', '\0',
#else /* posix_write_USE_STUB */
#define len_posix_write_USE_STUB /* nothing */
#define str_posix_write_USE_STUB /* nothing */
#endif /* !posix_write_USE_STUB */
#ifdef stat_class_isexe_IS_STUB
#define len_stat_class_isexe_IS_STUB +11
#define str_stat_class_isexe_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'e', 'x', 'e', '\0',
#else /* stat_class_isexe_IS_STUB */
#define len_stat_class_isexe_IS_STUB /* nothing */
#define str_stat_class_isexe_IS_STUB /* nothing */
#endif /* !stat_class_isexe_IS_STUB */
#ifdef stat_class_ishidden_IS_STUB
#define len_stat_class_ishidden_IS_STUB +14
#define str_stat_class_ishidden_IS_STUB 's', 't', 'a', 't', '.', 'i', 's', 'h', 'i', 'd', 'd', 'e', 'n', '\0',
#else /* stat_class_ishidden_IS_STUB */
#define len_stat_class_ishidden_IS_STUB /* nothing */
#define str_stat_class_ishidden_IS_STUB /* nothing */
#endif /* !stat_class_ishidden_IS_STUB */
#define POSIX_STUBS_TOTLEN 0 \
	len_posix_access_USE_STUB \
	len_posix_chdir_USE_STUB \
	len_posix_clearenv_USE_STUB \
	len_posix_close_USE_STUB \
	len_posix_cpu_count_USE_STUB \
	len_posix_creat_USE_STUB \
	len_posix_dup2_USE_STUB \
	len_posix_dup3_USE_STUB \
	len_posix_dup_USE_STUB \
	len_posix_enumenv_USE_STUB \
	len_posix_errno_USE_STUB \
	len_posix_euidaccess_USE_STUB \
	len_posix_fchdir_USE_STUB \
	len_posix_fchdirat_USE_STUB \
	len_posix_fdatasync_USE_STUB \
	len_posix_freadlink_USE_STUB \
	len_posix_frename_USE_STUB \
	len_posix_fsync_USE_STUB \
	len_posix_ftruncate_USE_STUB \
	len_posix_getenv_USE_STUB \
	len_posix_gethostname_USE_STUB \
	len_posix_getpid_USE_STUB \
	len_posix_isatty_USE_STUB \
	len_posix_lseek_USE_STUB \
	len_posix_lstat_USE_STUB \
	len_posix_open_USE_STUB \
	len_posix_opendir_USE_STUB \
	len_posix_pipe2_USE_STUB \
	len_posix_pipe_USE_STUB \
	len_posix_pread_USE_STUB \
	len_posix_pwrite_USE_STUB \
	len_posix_read_USE_STUB \
	len_posix_readlink_USE_STUB \
	len_posix_readlinkat_USE_STUB \
	len_posix_remove_USE_STUB \
	len_posix_removeat_USE_STUB \
	len_posix_rename_USE_STUB \
	len_posix_renameat2_USE_STUB \
	len_posix_renameat_USE_STUB \
	len_posix_rmdir_USE_STUB \
	len_posix_rmdirat_USE_STUB \
	len_posix_setenv_USE_STUB \
	len_posix_stat_USE_STUB \
	len_posix_stat_get_atime_IS_STUB \
	len_posix_stat_get_birthtime_IS_STUB \
	len_posix_stat_get_ctime_IS_STUB \
	len_posix_stat_get_dev_IS_STUB \
	len_posix_stat_get_gid_IS_STUB \
	len_posix_stat_get_ino_IS_STUB \
	len_posix_stat_get_mode_IS_STUB \
	len_posix_stat_get_mtime_IS_STUB \
	len_posix_stat_get_nlink_IS_STUB \
	len_posix_stat_get_rdev_IS_STUB \
	len_posix_stat_get_size_IS_STUB \
	len_posix_stat_get_uid_IS_STUB \
	len_posix_stat_isblk_IS_STUB \
	len_posix_stat_ischr_IS_STUB \
	len_posix_stat_isdev_IS_STUB \
	len_posix_stat_isdir_IS_STUB \
	len_posix_stat_isfifo_IS_STUB \
	len_posix_stat_islnk_IS_STUB \
	len_posix_stat_isreg_IS_STUB \
	len_posix_stat_issock_IS_STUB \
	len_posix_strerror_USE_STUB \
	len_posix_strerrorname_USE_STUB \
	len_posix_system_USE_STUB \
	len_posix_truncate_USE_STUB \
	len_posix_umask_USE_STUB \
	len_posix_unlink_USE_STUB \
	len_posix_unlinkat_USE_STUB \
	len_posix_unsetenv_USE_STUB \
	len_posix_write_USE_STUB \
	len_stat_class_isexe_IS_STUB \
	len_stat_class_ishidden_IS_STUB \
/**/
#if POSIX_STUBS_TOTLEN != 0
PRIVATE struct {
	OBJECT_HEAD
	struct string_utf *s_data;
	dhash_t            s_hash;
	size_t             s_len;
	char               s_str[POSIX_STUBS_TOTLEN];
	struct string_utf  s_utf;
} posix_missing_features = {
	OBJECT_HEAD_INIT(&DeeString_Type),
	&posix_missing_features.s_utf,
	DEE_STRING_HASH_UNSET,
	POSIX_STUBS_TOTLEN - 1, {
		str_posix_access_USE_STUB
		str_posix_chdir_USE_STUB
		str_posix_clearenv_USE_STUB
		str_posix_close_USE_STUB
		str_posix_cpu_count_USE_STUB
		str_posix_creat_USE_STUB
		str_posix_dup2_USE_STUB
		str_posix_dup3_USE_STUB
		str_posix_dup_USE_STUB
		str_posix_enumenv_USE_STUB
		str_posix_errno_USE_STUB
		str_posix_euidaccess_USE_STUB
		str_posix_fchdir_USE_STUB
		str_posix_fchdirat_USE_STUB
		str_posix_fdatasync_USE_STUB
		str_posix_freadlink_USE_STUB
		str_posix_frename_USE_STUB
		str_posix_fsync_USE_STUB
		str_posix_ftruncate_USE_STUB
		str_posix_getenv_USE_STUB
		str_posix_gethostname_USE_STUB
		str_posix_getpid_USE_STUB
		str_posix_isatty_USE_STUB
		str_posix_lseek_USE_STUB
		str_posix_lstat_USE_STUB
		str_posix_open_USE_STUB
		str_posix_opendir_USE_STUB
		str_posix_pipe2_USE_STUB
		str_posix_pipe_USE_STUB
		str_posix_pread_USE_STUB
		str_posix_pwrite_USE_STUB
		str_posix_read_USE_STUB
		str_posix_readlink_USE_STUB
		str_posix_readlinkat_USE_STUB
		str_posix_remove_USE_STUB
		str_posix_removeat_USE_STUB
		str_posix_rename_USE_STUB
		str_posix_renameat2_USE_STUB
		str_posix_renameat_USE_STUB
		str_posix_rmdir_USE_STUB
		str_posix_rmdirat_USE_STUB
		str_posix_setenv_USE_STUB
		str_posix_stat_USE_STUB
		str_posix_stat_get_atime_IS_STUB
		str_posix_stat_get_birthtime_IS_STUB
		str_posix_stat_get_ctime_IS_STUB
		str_posix_stat_get_dev_IS_STUB
		str_posix_stat_get_gid_IS_STUB
		str_posix_stat_get_ino_IS_STUB
		str_posix_stat_get_mode_IS_STUB
		str_posix_stat_get_mtime_IS_STUB
		str_posix_stat_get_nlink_IS_STUB
		str_posix_stat_get_rdev_IS_STUB
		str_posix_stat_get_size_IS_STUB
		str_posix_stat_get_uid_IS_STUB
		str_posix_stat_isblk_IS_STUB
		str_posix_stat_ischr_IS_STUB
		str_posix_stat_isdev_IS_STUB
		str_posix_stat_isdir_IS_STUB
		str_posix_stat_isfifo_IS_STUB
		str_posix_stat_islnk_IS_STUB
		str_posix_stat_isreg_IS_STUB
		str_posix_stat_issock_IS_STUB
		str_posix_strerror_USE_STUB
		str_posix_strerrorname_USE_STUB
		str_posix_system_USE_STUB
		str_posix_truncate_USE_STUB
		str_posix_umask_USE_STUB
		str_posix_unlink_USE_STUB
		str_posix_unlinkat_USE_STUB
		str_posix_unsetenv_USE_STUB
		str_posix_write_USE_STUB
		str_stat_class_isexe_IS_STUB
		str_stat_class_ishidden_IS_STUB
	},
	{ Dee_STRING_WIDTH_1BYTE,
	  Dee_STRING_UTF_FASCII,
	  { (size_t *)posix_missing_features.s_str },
	  posix_missing_features.s_str }
};
#else /* POSIX_STUBS_TOTLEN != 0 */
#undef POSIX_STUBS_TOTLEN
#define POSIX_STUBS_TOTLEN 0
#endif /* POSIX_STUBS_TOTLEN == 0 */
/*[[[end]]]*/


#if POSIX_STUBS_TOTLEN != 0

PRIVATE WUNUSED DREF DeeObject *DCALL pst_ctor(void);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pst_Iterator_get(DeeObject *__restrict UNUSED(self)) {
	return DeeModule_GetExternString("rt", "StringSplitIterator");
}

/*[[[deemon (PRIVATE_DEFINE_STRING from rt.gen.string)("str_nul", "\0");]]]*/
PRIVATE DEFINE_STRING_EX(str_nul, "\0", 0x514e28b7, 0x0);
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pst_iter_self(DeeObject *__restrict UNUSED(self)) {
	DeeObject *argv[] = { (DeeObject *)&str_nul };
	DREF DeeObject *seq, *iter;
	seq = DeeObject_CallAttrString((DeeObject *)&posix_missing_features,
	                               "split", 1, argv);
	if unlikely(!seq)
		goto err;
	iter = DeeObject_IterSelf(seq);
	Dee_Decref(seq);
	return iter;
err:
	return NULL;
}

#ifndef CONFIG_HAVE_memmem
#define memmem dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pst_contains(DeeObject *__restrict UNUSED(self),
             DeeObject *__restrict function_name) {
	void *p;
	if unlikely(!DeeString_Check(function_name))
		return_false;
	p = memmem(posix_missing_features.s_str,
	           sizeof(posix_missing_features.s_str),
	           DeeString_STR(function_name),
	           (DeeString_SIZE(function_name) + 1) * sizeof(char));
	return_bool_(p != NULL);
}

PRIVATE struct type_seq pst_seq = {
	/* .tp_iter_self = */ &pst_iter_self,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ &pst_contains
};

PRIVATE struct type_getset tpconst pst_class_getsets[] = {
	TYPE_GETTER_NODOC("Iterator", &pst_Iterator_get),
	TYPE_GETSET_END
};


PRIVATE DeeTypeObject PosixStubsList_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_PosixStubsList",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&pst_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &pst_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ pst_class_getsets,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeObject PosixStubsList_Singleton = { OBJECT_HEAD_INIT(&PosixStubsList_Type) };
PRIVATE WUNUSED DREF DeeObject *DCALL pst_ctor(void) {
	return_reference_(&PosixStubsList_Singleton);
}
#else /* POSIX_STUBS_TOTLEN != 0 */
/* Special case: There aren't _any_ stubs! */
#define PosixStubsList_Singleton DeeTuple_Empty
#endif /* POSIX_STUBS_TOTLEN == 0 */


PRIVATE char const *import_table[] = {
	/* NOTE: Indices in this table must match those used by `*_MODULE' macros! */
	"fs", /* #define FS_MODULE   DEX.d_imports[0] */
	NULL
};

#ifdef __INTELLISENSE__
#define D(...) /* nothing */
#else /* __INTELLISENSE__ */
#define D(...) __VA_ARGS__
#endif /* !__INTELLISENSE__ */


/*[[[deemon
local names = {
	"IFMT", "IFDIR", "IFCHR", "IFBLK", "IFREG",
	"IFIFO", "IFLNK", "IFSOCK", "ISUID", "ISGID",
	"ISVTX", "IRUSR", "IWUSR", "IXUSR", "IRGRP",
	"IWGRP", "IXGRP", "IROTH", "IWOTH", "IXOTH"
};
import * from rt.gen.dexutils;
include("p-stat-constants.def");
for (local x: names)
	gi("S_" + x, "STAT_" + x);
]]]*/
#include "p-stat-constants.def"
//[[[end]]]



/* Stat helper functions. */
INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISDIR(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISDIR", &arg))
		goto err;
	return_bool(STAT_ISDIR(arg));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISCHR(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISCHR", &arg))
		goto err;
	return_bool(STAT_ISCHR(arg));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISBLK(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISBLK", &arg))
		goto err;
	return_bool(STAT_ISBLK(arg));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISDEV(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISDEV", &arg))
		goto err;
	return_bool(STAT_ISDEV(arg));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISREG(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISREG", &arg))
		goto err;
	return_bool(STAT_ISREG(arg));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISFIFO(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISFIFO", &arg))
		goto err;
	return_bool(STAT_ISFIFO(arg));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISLNK(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISLNK", &arg))
		goto err;
	return_bool(STAT_ISLNK(arg));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
posix_S_ISSOCK(size_t argc, DeeObject *const *argv) {
	unsigned int arg;
	if (DeeArg_Unpack(argc, argv, "u:S_ISSOCK", &arg))
		goto err;
	return_bool(STAT_ISSOCK(arg));
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(libposix_S_ISDIR, &posix_S_ISDIR);
PRIVATE DEFINE_CMETHOD(libposix_S_ISCHR, &posix_S_ISCHR);
PRIVATE DEFINE_CMETHOD(libposix_S_ISBLK, &posix_S_ISBLK);
PRIVATE DEFINE_CMETHOD(libposix_S_ISDEV, &posix_S_ISDEV);
PRIVATE DEFINE_CMETHOD(libposix_S_ISREG, &posix_S_ISREG);
PRIVATE DEFINE_CMETHOD(libposix_S_ISFIFO, &posix_S_ISFIFO);
PRIVATE DEFINE_CMETHOD(libposix_S_ISLNK, &posix_S_ISLNK);
PRIVATE DEFINE_CMETHOD(libposix_S_ISSOCK, &posix_S_ISSOCK);




PRIVATE struct dex_symbol symbols[] = {
	/* E* errno codes */
	D(POSIX_ERRNO_DEFS)
	/* IMPORTANT: errno codes must come first! */

	{ "stubs", &PosixStubsList_Singleton, MODSYM_FNORMAL,
	  DOC("->?S?Dstring\n"
	      "List of the names of all of the functions that are implemented as stubs\n"
	      "The names contained within this list are identical to the names of the "
	      "resp. symbols exported by this module. e.g. ${'open' in posix.stubs} would "
	      "mean that ?Gopen will unconditionally throw :{UnsupportedAPI}. Other functions "
	      "may behave differently and always behave as no-ops, but the general behavior "
	      "is that the function will simply be implemented as a stub") },

	/* File control */
	D(POSIX_OPEN_DEF_DOC("Open a given @filename using @oflags (a set of ${O_*} flags), and @mode (describing "
	                     "the posix permissions to apply to a newly created file when ?GO_CREAT is given)"))
	D(POSIX__OPEN_DEF_DOC("Same as ?Gopen, but whereas ?Gopen will automatically set the ?GO_OBTAIN_DIR and ?GO_BINARY "
	                      "flags on platforms that define them in order to better standartize behavior of that "
	                      "function on those system, this function (?G_open) will not make any changes to the given @oflags"))
	D(POSIX_CREAT_DEF_DOC("Create a new file (same as ${open(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)})"))
	D(POSIX__CREAT_DEF_DOC("Same as ?Gcreat, but on systems that define ?GO_BINARY, that flag is also passed "
	                       "via the internal @oflags list eventually passed to ?Gopen (or rather ?G_open)"))
	D(POSIX_READ_DEF_DOC("Read up to @count bytes into @buf\n"
	                     "When @buf is given, return the actual number of read bytes. "
	                     "Otherwise, read into a new :Bytes object that is then returned"))
	D(POSIX_WRITE_DEF_DOC("Write up to @count bytes from @buf, returning the actual number of bytes written"))
	D(POSIX_LSEEK_DEF)
	D(POSIX_FSYNC_DEF)
	D(POSIX_FDATASYNC_DEF)
	D(POSIX_CLOSE_DEF)
	D(POSIX_UMASK_DEF)
	D(POSIX_DUP_DEF)
	D(POSIX_DUP2_DEF)
	D(POSIX_DUP3_DEF)
	/* TODO: lockf() */
	D(POSIX_PREAD_DEF)
	/* TODO: readv() */
	/* TODO: preadv() */
	D(POSIX_PWRITE_DEF)
	/* TODO: writev() */
	/* TODO: pwritev() */
	D(POSIX_ISATTY_DEF)
	D(POSIX_PIPE_DEF)
	D(POSIX_PIPE2_DEF)
	/* TODO: fcntl() */
	/* TODO: ioctl() */
	/* TODO: posix_fallocate() */
	/* TODO: posix_fadvise() */

	/* Filesystem control */
	D(POSIX_TRUNCATE_DEF)
	D(POSIX_FTRUNCATE_DEF)
	D(POSIX_ACCESS_DEF)
	D(POSIX_EUIDACCESS_DEF)
	D(POSIX_FACCESSAT_DEF)
	D(POSIX_FCHOWNAT_DEF)
	D(POSIX_FCHMODAT_DEF)
	/* TODO: chflags() */
	/* TODO: lchflags() */
	/* TODO: chroot() */
	/* TODO: mkfifo() */
	/* TODO: mknod() */
	/* TODO: major() */
	/* TODO: minor() */
	/* TODO: mkdev() */
	D(POSIX_SYNC_DEF)
	/* TODO: utime() */
	/* TODO: pathconf() */
	/* TODO: fpathconf() */
	/* TODO: statvfs() */
	/* TODO: fstatvfs() */
	/* TODO: getxattr() */
	/* TODO: lgetxattr() */
	/* TODO: fgetxattr() */
	/* TODO: setxattr() */
	/* TODO: lsetxattr() */
	/* TODO: fsetxattr() */
	/* TODO: removexattr() */
	/* TODO: lremovexattr() */
	/* TODO: flremovexattr() */
	/* TODO: listxattr() */
	/* TODO: llistxattr() */
	/* TODO: flistxattr() */

	/* Path normalization */
	D(POSIX_REALPATH_DEF)

	/* System information */
	/* TODO: uname() */
	/* TODO: confstr() */
	/* TODO: sysconf() */
	/* TODO: times() */
	/* TODO: getloadavg() */

	/* Terminal control */
	/* TODO: ttyname() */
	/* TODO: ctermid() */
	/* TODO: openpty() */
	/* TODO: forkpty() */
	/* TODO: getlogin() */
	/* TODO: tcgetpgrp() */
	/* TODO: tcsetpgrp() */

	/* Process control */
	D(POSIX_GETPID_DEF)
	D(POSIX_SYSTEM_DEF)
	/* TODO: execl() */
	/* TODO: execle() */
	/* TODO: execlp() */
	/* TODO: execlpe() */
	/* TODO: execv() */
	/* TODO: execve() */
	/* TODO: execvp() */
	/* TODO: execvpe() */
	/* TODO: fexecve() */
	/* TODO: cwait() */
	/* TODO: spawnl() */
	/* TODO: spawnle() */
	/* TODO: spawnlp() */
	/* TODO: spawnlpe() */
	/* TODO: spawnv() */
	/* TODO: spawnve() */
	/* TODO: spawnvp() */
	/* TODO: spawnvpe() */

	/* Scheduling control */
	D(POSIX_SCHED_YIELD_DEF)
	/* TODO: sched_get_priority_min() */
	/* TODO: sched_get_priority_max() */
	/* TODO: sched_getparam() */
	/* TODO: sched_setparam() */
	/* TODO: sched_getscheduler() */
	/* TODO: sched_setscheduler() */
	/* TODO: sched_rr_get_interval() */
	/* TODO: sched_getaffinity() */
	/* TODO: sched_getaffinity() */
	/* TODO: nice() */
	/* TODO: getpriority() */
	/* TODO: setpriority() */
	/* TODO: fork() */
	/* TODO: wait() */
	/* TODO: wait3() */
	/* TODO: wait4() */
	/* TODO: waitid() */
	/* TODO: waitpid() */
	/* TODO: kill() */
	/* TODO: killpg() */
	/* TODO: getppid() */
	/* TODO: getpgrp() */
	/* TODO: setpgrp() */
	/* TODO: getpgid() */
	/* TODO: setpgid() */
	/* TODO: getsid() */
	/* TODO: setsid() */
	/* TODO: WCOREDUMP() */
	/* TODO: WIFCONTINUED() */
	/* TODO: WIFSTOPPED() */
	/* TODO: WIFSIGNALED() */
	/* TODO: WIFEXITED() */
	/* TODO: WEXITSTATUS() */
	/* TODO: WTERMSIG() */
	/* TODO: WSTOPSIG() */

	/* User/Permission control */
	/* TODO: geteuid() */
	/* TODO: seteuid() */
	/* TODO: getegid() */
	/* TODO: setegid() */
	/* TODO: getgid() */
	/* TODO: setgid() */
	/* TODO: getuid() */
	/* TODO: setuid() */
	/* TODO: setreuid() */
	/* TODO: setregid() */
	/* TODO: getgrouplist() */
	/* TODO: getgroups() */
	/* TODO: setgroups() */
	/* TODO: initgroups() */
	/* TODO: setresuid() */
	/* TODO: setresgid() */
	/* TODO: getresuid() */
	/* TODO: getresgid() */

	/* Random number generation */
	/* TODO: urandom() */
	/* TODO: getrandom() */

	/* Python-like helper functions */
	D(POSIX_CPU_COUNT_DEF_DOC("Returns the ## of available processors on the host machine"))
	/* TODO: get_inheritable() */
	/* TODO: set_inheritable() */

	/* Higher-level wrapper functions */
	/* TODO: popen() */
	/* TODO: fdopen() (Basically just a wrapper around `DeeFile_OpenFd') */


	/* Directory access */
	D({ "dirent", (DeeObject *)&DeeDirIterator_Type, MODSYM_FNORMAL },)
	D({ "DIR", (DeeObject *)&DeeDir_Type, MODSYM_FNORMAL },)
	D({ "opendir", (DeeObject *)&DeeDir_Type, MODSYM_FNORMAL,
	    DOC("(path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!f)->?GDIR\n"
	        "Read the contents of a given directory. The returned "
	        /**/ "object may be iterated to yield ?Gdirent objects.\n"
	        "Additionally, you may specify @skipdots as !f if you "
	        /**/ "wish to include the special $'.' and $'..' entries.") },)
	D({ "fdopendir", (DeeObject *)&posix_fdopendir, MODSYM_FNORMAL,
	    DOC("(path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!t)->?GDIR\n"
	        "Same as ?Gopendir, but the default value of @inheritfd is !t, "
	        /**/ "mimicking the behavior of the native $fdopendir function") },)

	/* File type constants. */
	D({ "DT_UNKNOWN", (DeeObject *)&posix_DT_UNKNOWN, MODSYM_FNORMAL },)
	D({ "DT_FIFO", (DeeObject *)&posix_DT_FIFO, MODSYM_FNORMAL },)
	D({ "DT_CHR", (DeeObject *)&posix_DT_CHR, MODSYM_FNORMAL },)
	D({ "DT_DIR", (DeeObject *)&posix_DT_DIR, MODSYM_FNORMAL },)
	D({ "DT_BLK", (DeeObject *)&posix_DT_BLK, MODSYM_FNORMAL },)
	D({ "DT_REG", (DeeObject *)&posix_DT_REG, MODSYM_FNORMAL },)
	D({ "DT_LNK", (DeeObject *)&posix_DT_LNK, MODSYM_FNORMAL },)
	D({ "DT_SOCK", (DeeObject *)&posix_DT_SOCK, MODSYM_FNORMAL },)
	D({ "DT_WHT", (DeeObject *)&posix_DT_WHT, MODSYM_FNORMAL },)
	D({ "DTTOIF", (DeeObject *)&posix_DTTOIF, MODSYM_FNORMAL,
	    DOC("(dt:?Dint)->?Dint\n"
	        "Convert a ${DT_*} constant to ${S_IF*}") },)
	D({ "IFTODT", (DeeObject *)&posix_IFTODT, MODSYM_FNORMAL,
	    DOC("(if:?Dint)->?Dint\n"
	        "Convert an ${S_IF*} constant to ${DT_*}") },)

	/* Environ control */
	D(POSIX_GETENV_DEF_DOC("@throws KeyError The given @varname wasn't found, and @defl wasn't given\n"
	                       "Same as ${environ[varname]}\n"))
	D(POSIX_SETENV_DEF_DOC("Same as ${environ[varname] = value}. When @replace is !f, only "
	                       "add new variables, but don't override one that was already set\n"))
	D(POSIX_PUTENV_DEF_DOC("If @envline constains $'=', same as ${local name, none, value = envline.partition(\"=\")...; setenv(name, value);}\n"
	                       "Otherwise, same as ?#unsetenv"))
	D(POSIX_UNSETENV_DEF_DOC("Returns !t if @varname was deleted, and !f if @varname didn't exist in ?Genviron"))
	D(POSIX_CLEARENV_DEF_DOC("Clear ?Genviron"))
	D({ "environ", &DeeEnviron_Singleton, MODSYM_FREADONLY,
	    DOC("->?S?T2?Dstring?Dstring\n"
	        "A ?DMapping-style singleton instance that can be used to "
	        /**/ "access and enumerate environment variables by name:\n"
	        "${"
	        /**/ "print environ[\"PATH\"]; /* \"/bin:/usr/bin:...\" */"
	        "}\n"
	        "Other mapping operations known from ?DMapping can be used "
	        /**/ "to delete (${del environ[...]}), set (${environ[...] = ...}) and "
	        /**/ "check for the existance of (${... in environ}) environment variables, "
	        /**/ "as well as enumerating all variables (${for (key, item: environ) ...})")
	},)

	/* stat.st_mode bits. */
	LIBPOSIX_S_IFMT_DEF
	LIBPOSIX_S_IFDIR_DEF
	LIBPOSIX_S_IFCHR_DEF
	LIBPOSIX_S_IFBLK_DEF
	LIBPOSIX_S_IFREG_DEF
	LIBPOSIX_S_IFIFO_DEF
	LIBPOSIX_S_IFLNK_DEF
	LIBPOSIX_S_IFSOCK_DEF
	LIBPOSIX_S_ISUID_DEF
	LIBPOSIX_S_ISGID_DEF
	LIBPOSIX_S_ISVTX_DEF
	LIBPOSIX_S_IRUSR_DEF
	LIBPOSIX_S_IWUSR_DEF
	LIBPOSIX_S_IXUSR_DEF
	LIBPOSIX_S_IRGRP_DEF
	LIBPOSIX_S_IWGRP_DEF
	LIBPOSIX_S_IXGRP_DEF
	LIBPOSIX_S_IROTH_DEF
	LIBPOSIX_S_IWOTH_DEF
	LIBPOSIX_S_IXOTH_DEF

	/* stat.st_mode helper functions. */
	{ "S_ISDIR", (DeeObject *)&libposix_S_ISDIR, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
	{ "S_ISCHR", (DeeObject *)&libposix_S_ISCHR, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
	{ "S_ISBLK", (DeeObject *)&libposix_S_ISBLK, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
	{ "S_ISDEV", (DeeObject *)&libposix_S_ISDEV, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
	{ "S_ISREG", (DeeObject *)&libposix_S_ISREG, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
	{ "S_ISFIFO", (DeeObject *)&libposix_S_ISFIFO, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
	{ "S_ISLNK", (DeeObject *)&libposix_S_ISLNK, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
	{ "S_ISSOCK", (DeeObject *)&libposix_S_ISSOCK, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },

	/* stat & friends */
	D({ "stat", (DeeObject *)&DeeStat_Type, MODSYM_FNORMAL },)
	D({ "lstat", (DeeObject *)&DeeLStat_Type, MODSYM_FNORMAL },)
	D(POSIX_FSTAT_DEF_DOC("More restrictive alias for ?Gstat"))
	D(POSIX_FSTATAT_DEF_DOC("More restrictive alias for ?Gstat"))

	/* Process Environment */
	D(POSIX_GETCWD_DEF_DOC("@interrupt\n"
	                       "@throw FileAccessError Permission to read a part of the current working directory's path was denied\n"
	                       "@throw FileNotFound The current working directory has been unlinked\n"
	                       "@throw SystemError Failed to retrieve the current working directory for some reason\n"
	                       "Return the absolute path of the current working directory"))
	D(POSIX_GETTMP_DEF_DOC("@interrupt\n"
	                       "@throw SystemError Failed to retrieve a temporary path name for some reason\n"
	                       "Return the path to a folder that can be used as temporary storage of files and directories\n"
	                       "If (in this order) one of these environment variables is defined, "
	                       /**/ "it will be returned $'TMPDIR', $'TMP', $'TEMP', $'TEMPDIR'"))
	D(POSIX_GETHOSTNAME_DEF_DOC("@interrupt\n"
	                            "@throw SystemError Failed to retrieve the name of the hosting machine for some reason\n"
	                            "Returns the user-assigned name of the hosting machine"))
	D(POSIX_CHDIR_DEF_DOC("@interrupt\n"
	                      "@throw FileNotFound The given @path could not be found\n"
	                      "@throw NoDirectory The given @path is not a directory\n"
	                      "@throw FileAccessError The current user does not have permissions to enter @path\n"
	                      "@throw SystemError Failed to change the current working directory for some reason\n"
	                      "Change the current working directory to @path, which may be "
	                      /**/ "a path relative to the old current working directory"))
	D(POSIX_FCHDIR_DEF_DOC("@interrupt\n"
	                       "@throw FileNotFound The given @fd could not be found\n"
	                       "@throw NoDirectory The given @fd is not a directory\n"
	                       "@throw FileClosed The given @fd has been closed or is invalid\n"
	                       "@throw FileAccessError The current user does not have permissions to enter @fd\n"
	                       "@throw SystemError Failed to change the current working directory for some reason\n"
	                       "Change the current working directory to @fd"))
	D(POSIX_FCHDIRAT_DEF_DOC("@interrupt\n"
	                         "@throw FileNotFound The given @dfd:@path could not be found\n"
	                         "@throw NoDirectory The given @dfd:@path is not a directory\n"
	                         "@throw FileClosed The given @dfd has been closed or is invalid\n"
	                         "@throw FileAccessError The current user does not have permissions to enter @dfd:@path\n"
	                         "@throw SystemError Failed to change the current working directory for some reason\n"
	                         "@throw ValueError Invalid set of flags specified by @atflags\n"
	                         "Change the current working directory to @dfd:@path"))

	/* Filesystem */
	D(POSIX_UNLINK_DEF_DOC("@interrupt\n"
	                       "@throw FileNotFound The given @file does not exist\n"
	                       "@throw NoDirectory A part of the given @file is not a directory\n"
	                       "@throw IsDirectory The given @file is a directory and ?Grmdir or ?Gremove must be used to delete it\n"
	                       "@throw BusyFile The given @file is currently being used and cannot be deleted\n"
	                       "@throw FileAccessError The current user does not have permissions to remove the file described by @file\n"
	                       "@throw ReadOnlyFile The filesystem or device hosting the directory of @file is in read-only "
	                       /*               */ "operations mode, preventing the deletion of existing files\n"
	                       "@throw SystemError Failed to unlink the given file @file for some reason\n"
	                       "Remove a non-directory filesystem object named @file"))
	D(POSIX_RMDIR_DEF_DOC("@interrupt\n"
	                      "@throw FileNotFound The given @path does not exist\n"
	                      "@throw NoDirectory A part of the given @path is not a directory\n"
	                      "@throw NoDirectory The given @path isn't a directory and either ?Gunlink or ?Gremove must be used to "
	                      /*              */ "delete it, or it is a mounting point if such functionality is supported by the host\n"
	                      "@throw NotEmpty The given directory @path isn't empty empty\n"
	                      "@throw BusyFile The given @path is currently being used and cannot be deleted\n"
	                      "@throw FileAccessError The current user does not have permissions to remove the directory described by @path\n"
	                      "@throw ReadOnlyFile The filesystem or device hosting the directory of @path is in read-only "
	                      /*               */ "operations mode, preventing the deletion of existing directories\n"
	                      "@throw SystemError Failed to delete the directory @path for some reason\n"
	                      "Remove a directory named @path"))
	D(POSIX_REMOVE_DEF_DOC("@interrupt\n"
	                       "@throw FileNotFound The given @path does not exist\n"
	                       "@throw NoDirectory A part of the given @path is not a directory\n"
	                       "@throw NotEmpty The given @path is a directory that isn't empty empty\n"
	                       "@throw BusyFile The given @path is currently being used and cannot be deleted\n"
	                       "@throw FileAccessError The current user does not have permissions to remove the file or directory described by @path\n"
	                       "@throw ReadOnlyFile The filesystem or device hosting the directory of @path is in read-only "
	                       /*               */ "operations mode, preventing the deletion of existing files or directories\n"
	                       "@throw SystemError Failed to remove the given file @path for some reason\n"
	                       "Remove a file or an empty directory name @path"))
	D(POSIX_UNLINKAT_DEF_DOC("@interrupt\n"
	                         "@throw FileNotFound The given @dfd:@file does not exist\n"
	                         "@throw NoDirectory A part of the given @dfd:@file is not a directory\n"
	                         "@throw IsDirectory The given @dfd:@file is a directory and ?Grmdir or ?Gremove must be used to delete it\n"
	                         "@throw BusyFile The given @dfd:@file is currently being used and cannot be deleted\n"
	                         "@throw FileAccessError The current user does not have permissions to remove the file described by @dfd:@file\n"
	                         "@throw ReadOnlyFile The filesystem or device hosting the directory of @dfd:@file is in read-only "
	                         /*               */ "operations mode, preventing the deletion of existing files\n"
	                         "@throw FileClosed The given @fd was closed\n"
	                         "@throw SystemError Failed to unlink the given file @dfd:@file for some reason\n"
	                         "@param atflags Set of ?GAT_REMOVEDIR, ?GAT_REMOVEREG\n"
	                         "Remove a filesystem object named @dfd:@file"))
	D(POSIX_RMDIRAT_DEF_DOC("@interrupt\n"
	                        "@throw FileNotFound The given @dfd:@path does not exist\n"
	                        "@throw NoDirectory A part of the given @dfd:@path is not a directory\n"
	                        "@throw NoDirectory The given @dfd:@path isn't a directory and either ?Gunlink or ?Gremove must be used to "
	                        /*              */ "delete it, or it is a mounting point if such functionality is supported by the host\n"
	                        "@throw NotEmpty The given directory @dfd:@path isn't empty empty\n"
	                        "@throw BusyFile The given @dfd:@path is currently being used and cannot be deleted\n"
	                        "@throw FileAccessError The current user does not have permissions to remove the directory described by @dfd:@path\n"
	                        "@throw ReadOnlyFile The filesystem or device hosting the directory of @dfd:@path is in read-only "
	                        /*               */ "operations mode, preventing the deletion of existing directories\n"
	                        "@throw FileClosed The given @fd was closed\n"
	                        "@throw SystemError Failed to delete the directory @dfd:@path for some reason\n"
	                        "Remove a directory named @dfd:@path"))
	D(POSIX_REMOVEAT_DEF_DOC("@interrupt\n"
	                         "@throw FileNotFound The given @dfd:@path does not exist\n"
	                         "@throw NoDirectory A part of the given @dfd:@path is not a directory\n"
	                         "@throw NotEmpty The given @dfd:@path is a directory that isn't empty empty\n"
	                         "@throw BusyFile The given @dfd:@path is currently being used and cannot be deleted\n"
	                         "@throw FileAccessError The current user does not have permissions to remove the file or directory described by @dfd:@path\n"
	                         "@throw ReadOnlyFile The filesystem or device hosting the directory of @dfd:@path is in read-only "
	                         /*               */ "operations mode, preventing the deletion of existing files or directories\n"
	                         "@throw FileClosed The given @fd was closed\n"
	                         "@throw SystemError Failed to remove the given file @dfd:@path for some reason\n"
	                         "Remove a file or an empty directory name @dfd:@path"))
	D(POSIX_READLINK_DEF_DOC("@interrupt\n"
	                         "@throw FileNotFound The given @file does not exist\n"
	                         "@throw NoDirectory A part of the given @file is not a directory\n"
	                         "@throw NoLink The given @file does not refer to a symbolic link\n"
	                         "@throw ValueError The file described by @file is not a symlink\n"
	                         "@throw UnsupportedAPI The underlying filesystem does not support reading of symbolic links\n"
	                         "@throw FileAccessError The current user does not have permissions to access @file or one of the containing directories for reading\n"
	                         "@throw SystemError Failed to read the symbolic link under @file for some reason\n"
	                         "Read and return the targetText used to create a symbolic link (see ?Gsymlink)"))
	D(POSIX_FREADLINK_DEF_DOC("@interrupt\n"
	                          "@throw NoLink The given @fd does not refer to a symbolic link\n"
	                          "@throw ValueError The file described by @fd is not a symlink\n"
	                          "@throw UnsupportedAPI The underlying filesystem does not support reading of symbolic links\n"
	                          "@throw FileAccessError The current user does not have permissions to access @fd or one of the containing directories for reading\n"
	                          "@throw SystemError Failed to read the symbolic link under @fd for some reason\n"
	                          "@throw FileClosed The given @fd was closed\n"
	                          "Read and return the targetText used to create a symbolic link (see ?Gsymlink)"))
	D(POSIX_READLINKAT_DEF_DOC("@interrupt\n"
	                           "@throw FileNotFound The given @dfd:@file does not exist\n"
	                           "@throw NoDirectory A part of the given @dfd:@file is not a directory\n"
	                           "@throw NoLink The given @dfd:@file does not refer to a symbolic link\n"
	                           "@throw ValueError The file described by @dfd:@file is not a symlink\n"
	                           "@throw UnsupportedAPI The underlying filesystem does not support reading of symbolic links\n"
	                           "@throw FileAccessError The current user does not have permissions to access @dfd:@file or one of the containing directories for reading\n"
	                           "@throw FileClosed The given @dfd was closed\n"
	                           "@throw SystemError Failed to read the symbolic link under @dfd:@file for some reason\n"
	                           "Read and return the targetText used to create a symbolic link (see ?Gsymlink)"))
	D(POSIX_RENAME_DEF_DOC("@interrupt\n"
	                       "@throw FileNotFound The given @oldpath could not be found, or a parent directory of @newpath does not exist\n"
	                       "@throw NotEmpty The given @newpath is a non-empty directory\n"
	                       "@throw IsDirectory The given @newpath is an existing directory, but @oldpath isn't "
	                       /*              */ "one (directories can only be replaced by other directories)\n"
	                       "@throw NoDirectory A part of the given @oldpath or @newpath is not a directory, "
	                       /*              */ "or @oldpath is a directory, but @newpath isn't\n"
	                       "@throw CrossDevice The given @oldpath and @newpath are not located on the same device\n"
	                       "@throw ValueError Attempted to move a directory into itself\n"
	                       "@throw FileAccessError The current user does not have permissions to access the file "
	                       /*                  */ "or directory @oldpath, or the directory containing @newpath\n"
	                       "@throw ReadOnlyFile The filesystem or device hosting the given file or directory @oldpath or the "
	                       /*               */ "directory containing the non-existant file @newpath is in read-only operations "
	                       /*               */ "mode, preventing the modification of existing files or directories\n"
	                       "@throw SystemError Failed to rename the given @oldpath for some reason\n"
	                       "Renames or moves a given @oldpath to be referred to as @newpath from then on\n"
	                       "When @newpath already exists, it is replaced by @oldpath"))
	D(POSIX_FRENAME_DEF_DOC("@interrupt\n"
	                        "@throw FileNotFound A parent directory of @newpath does not exist\n"
	                        "@throw NotEmpty The given @newpath is a non-empty directory\n"
	                        "@throw IsDirectory The given @newpath is an existing directory, but @oldfd isn't "
	                        /*              */ "one (directories can only be replaced by other directories)\n"
	                        "@throw NoDirectory A part of @newpath is not a directory, "
	                        /*              */ "or @oldfd is a directory, but @newpath isn't\n"
	                        "@throw CrossDevice The given @oldfd and @newpath are not located on the same device\n"
	                        "@throw ValueError Attempted to move a directory into itself\n"
	                        "@throw FileAccessError The current user does not have permissions to access the file "
	                        /*                  */ "or directory @oldfd, or the directory containing @newpath\n"
	                        "@throw ReadOnlyFile The filesystem or device hosting the given file or directory @oldfd or the "
	                        /*               */ "directory containing the non-existant file @newpath is in read-only operations "
	                        /*               */ "mode, preventing the modification of existing files or directories\n"
	                        "@throw FileClosed The given @oldfd has already been closed\n"
	                        "@throw SystemError Failed to rename the given @oldfd for some reason\n"
	                        "Renames or moves a given @oldfd to be referred to as @newpath from then on\n"
	                        "When @newpath already exists, it is replaced by @oldfd"))
	D(POSIX_RENAMEAT_DEF_DOC("@interrupt\n"
	                         "@throw FileNotFound The given @olddirfd:@oldpath could not be found, or a parent directory of @newdirfd:@newpath does not exist\n"
	                         "@throw NotEmpty The given @newdirfd:@newpath is a non-empty directory\n"
	                         "@throw IsDirectory The given @newdirfd:@newpath is an existing directory, but @olddirfd:@oldpath isn't "
	                         /*              */ "one (directories can only be replaced by other directories)\n"
	                         "@throw NoDirectory A part of the given @olddirfd:@oldpath or @newdirfd:@newpath is not a directory, "
	                         /*              */ "or @olddirfd:@oldpath is a directory, but @newdirfd:@newpath isn't\n"
	                         "@throw CrossDevice The given @olddirfd:@oldpath and @newdirfd:@newpath are not located on the same device\n"
	                         "@throw ValueError Attempted to move a directory into itself\n"
	                         "@throw FileAccessError The current user does not have permissions to access the file "
	                         /*                  */ "or directory @olddirfd:@oldpath, or the directory containing @newdirfd:@newpath\n"
	                         "@throw ReadOnlyFile The filesystem or device hosting the given file or directory @olddirfd:@oldpath or the "
	                         /*               */ "directory containing the non-existant file @newdirfd:@newpath is in read-only operations "
	                         /*               */ "mode, preventing the modification of existing files or directories\n"
	                         "@throw FileClosed The given @olddirfd or @newdirfd has already been closed\n"
	                         "@throw ValueError Invalid @atflags\n"
	                         "@throw SystemError Failed to rename the given @olddirfd:@oldpath for some reason\n"
	                         "Renames or moves a given @olddirfd:@oldpath to be referred to as @newdirfd:@newpath from then on\n"
	                         "When @newpath already exists, it is replaced by @olddirfd:@oldpath"))
	D(POSIX_RENAMEAT2_DEF_DOC("@interrupt\n"
	                          "@throw FileNotFound The given @olddirfd:@oldpath could not be found, or a parent directory of @newdirfd:@newpath does not exist\n"
	                          "@throw NotEmpty The given @newdirfd:@newpath is a non-empty directory\n"
	                          "@throw IsDirectory The given @newdirfd:@newpath is an existing directory, but @olddirfd:@oldpath isn't "
	                          /*              */ "one (directories can only be replaced by other directories)\n"
	                          "@throw NoDirectory A part of the given @olddirfd:@oldpath or @newdirfd:@newpath is not a directory, "
	                          /*              */ "or @olddirfd:@oldpath is a directory, but @newdirfd:@newpath isn't\n"
	                          "@throw CrossDevice The given @olddirfd:@oldpath and @newdirfd:@newpath are not located on the same device\n"
	                          "@throw ValueError Attempted to move a directory into itself\n"
	                          "@throw FileAccessError The current user does not have permissions to access the file "
	                          /*                  */ "or directory @olddirfd:@oldpath, or the directory containing @newdirfd:@newpath\n"
	                          "@throw ReadOnlyFile The filesystem or device hosting the given file or directory @olddirfd:@oldpath or the "
	                          /*               */ "directory containing the non-existant file @newdirfd:@newpath is in read-only operations "
	                          /*               */ "mode, preventing the modification of existing files or directories\n"
	                          "@throw FileExists @flags include ?GRENAME_NOREPLACE, but @newdirfd:@newpath already exists\n"
	                          "@throw FileClosed The given @olddirfd or @newdirfd has already been closed\n"
	                          "@throw ValueError Invalid @flags or @atflags\n"
	                          "@throw SystemError Failed to rename the given @olddirfd:@oldpath for some reason\n"
	                          "Renames or moves a given @olddirfd:@oldpath to be referred to as @newdirfd:@newpath from then on\n"
	                          "When @newpath already exists, it is replaced by @olddirfd:@oldpath"))
	D(POSIX_LINK_DEF_DOC("@interrupt\n"
	                     "@throw FileNotFound The given @oldpath could not be found, or a parent directory of @newpath does not exist\n"
	                     "@throw FileExists The given @newpath already exists\n"
	                     "@throw NoDirectory A part of the given @oldpath or @newpath is not a directory\n"
	                     "@throw UnsupportedAPI The underlying filesystem hosting @oldpath and @newpathdoes not support hard links\n"
	                     "@throw CrossDevice The given @oldpath and @newpath are not apart of the same drive\n"
	                     "@throw FileAccessError The current user does not have permissions to access the file or directory "
	                     /*                  */ "@oldpath for reading, or the directory containing the non-existant "
	                     /*                  */ "object @newpath for writing\n"
	                     "@throw ReadOnlyFile The filesystem or device hosting the directory containing the non-existant "
	                     /*               */ "object @newpath is in read-only operations mode, preventing the addition "
	                     /*               */ "of file or directory links\n"
	                     "@throw SystemError Failed to create the link to @oldpath for some reason\n"
	                     "Create a new hard link pointing to @oldpath as a file named @newpath\n"
	                     "Hard links are similar to symbolic links, yet cannot be used across multiple devices and are "
	                     /**/ "unaffected by mount locations. A hard link simply create a new directory entry under "
	                     /**/ "@newpath that points to the data block of an existing file @oldpath"))
	D(POSIX_FLINK_DEF_DOC("@interrupt\n"
	                      "@throw FileNotFound A parent directory of @newpath does not exist\n"
	                      "@throw FileExists The given @newpath already exists\n"
	                      "@throw NoDirectory A part of the given @newpath is not a directory\n"
	                      "@throw UnsupportedAPI The underlying filesystem hosting @oldfd and @newpathdoes not support hard links\n"
	                      "@throw CrossDevice The given @oldfd and @newpath are not apart of the same drive\n"
	                      "@throw FileAccessError The current user does not have permissions to access the file or directory "
	                      /*                  */ "@oldfd for reading, or the directory containing the non-existant "
	                      /*                  */ "object @newpath for writing\n"
	                      "@throw ReadOnlyFile The filesystem or device hosting the directory containing the non-existant "
	                      /*               */ "object @newpath is in read-only operations mode, preventing the addition "
	                      /*               */ "of file or directory links\n"
	                      "@throw SystemError Failed to create the link to @oldfd for some reason\n"
	                      "Create a new hard link pointing to @oldfd as a file named @newpath\n"
	                      "Hard links are similar to symbolic links, yet cannot be used across multiple devices and are "
	                      /**/ "unaffected by mount locations. A hard link simply create a new directory entry under "
	                      /**/ "@newpath that points to the data block of an existing file @oldfd"))
	D(POSIX_LINKAT_DEF_DOC("@interrupt\n"
	                       "@throw FileNotFound The given @olddirfd:@oldpath could not be found, or a parent directory of @newdirfd:@newpath does not exist\n"
	                       "@throw FileExists The given @newdirfd:@newpath already exists\n"
	                       "@throw NoDirectory A part of the given @olddirfd:@oldpath or @newdirfd:@newpath is not a directory\n"
	                       "@throw UnsupportedAPI The underlying filesystem hosting @olddirfd:@oldpath and @newdirfd:@newpathdoes not support hard links\n"
	                       "@throw CrossDevice The given @olddirfd:@oldpath and @newdirfd:@newpath are not apart of the same drive\n"
	                       "@throw FileAccessError The current user does not have permissions to access the file or directory "
	                       /*                  */ "@olddirfd:@oldpath for reading, or the directory containing the non-existant "
	                       /*                  */ "object @newdirfd:@newpath for writing\n"
	                       "@throw ReadOnlyFile The filesystem or device hosting the directory containing the non-existant "
	                       /*               */ "object @newdirfd:@newpath is in read-only operations mode, preventing the addition "
	                       /*               */ "of file or directory links\n"
	                       "@throw SystemError Failed to create the link to @olddirfd:@oldpath for some reason\n"
	                       "Create a new hard link pointing to @olddirfd:@oldpath as a file named @newdirfd:@newpath\n"
	                       "Hard links are similar to symbolic links, yet cannot be used across multiple devices and are "
	                       /**/ "unaffected by mount locations. A hard link simply create a new directory entry under "
	                       /**/ "@newdirfd:@newpath that points to the data block of an existing file @olddirfd:@oldpath"))

	/* Forward-aliases to `libfs' */
#define DEFINE_LIBFS_ALIAS_ALT(altname, name, libfs_name, proto)                           \
	D({ altname, (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY | MODSYM_FREADONLY, \
	    DOC(proto "Alias for ?Efs:" libfs_name) }, )
#define DEFINE_LIBFS_ALIAS_S_ALT(altname, name, proto)                            \
	D({ altname,                                                                  \
	    (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY | MODSYM_FREADONLY, \
	    DOC(proto "Alias for ?Efs:" #name) }, )
#define DEFINE_LIBFS_ALIAS(name, libfs_name, proto) \
	DEFINE_LIBFS_ALIAS_ALT(#name, name, libfs_name, proto)
#define DEFINE_LIBFS_ALIAS_S(name, proto) \
	DEFINE_LIBFS_ALIAS_S_ALT(DeeString_STR(&libposix_libfs_name_##name), name, proto)
	DEFINE_LIBFS_ALIAS_S(chmod, "(path:?Dstring,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(lchmod, "(path:?Dstring,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(chown, "(path:?Dstring,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(lchown, "(path:?Dstring,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(mkdir, "(path:?Dstring,permissions:?X2?Dstring?Dint=!N)\n")
	DEFINE_LIBFS_ALIAS_S(symlink, "(linktext:?Dstring,linkname:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchmod", chmod, "(fp:?DFile,mode:?X2?Dstring?Dint)\n"
	                                          "(fd:?Dint,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchown", chown, "(fp:?DFile,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n"
	                                          "(fd:?Dint,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n")
#undef DEFINE_LIBFS_ALIAS_S
#undef DEFINE_LIBFS_ALIAS
#undef DEFINE_LIBFS_ALIAS_S_ALT
#undef DEFINE_LIBFS_ALIAS_ALT

	/* Application exit control */
	D(POSIX_ATEXIT_DEF_DOC("Register a callback to-be invoked before ?Gexit (Same as :AppExit.atexit)"))
	D(POSIX_EXIT_DEF_DOC("Terminate execution of deemon after invoking ?Gatexit callbacks\n"
	                     "Termination is done using the C $exit or $_Exit functions, if available. However if these "
	                     "functions are not provided by the host, an :AppExit error is thrown instead\n"
	                     "When no @exitcode is given, the host's default default value of ?GEXIT_FAILURE, or $1 is used\n"
	                     "This function never returns normally"))
	D(POSIX__EXIT_DEF_DOC("Terminate execution of deemon without invoking ?Gatexit callbacks (s.a. ?Gexit)"))
	D(POSIX_ABORT_DEF_DOC("Same as ?G_Exit when passing ?GEXIT_FAILURE"))

	/* *_FILENO values */
	D(POSIX_FD_DEFS)

	/* O_* and AT_* values */
	D(POSIX_OPEN_DEFS)

	/* SEEK_* values */
	D(POSIX_READWRITE_DEFS)

	/* EXIT_* values */
	D(POSIX_EXIT_DEFS)

	/* *_OK codes for `access()' and friends */
	D(POSIX_ACCESS_DEFS)

	D({ "errno", (DeeObject *)&posix_errno_get, MODSYM_FPROPERTY,
	    DOC("->?Dint\n"
	        "Read/write the C errno thread-local variable") }, )
	D({ NULL, (DeeObject *)&posix_errno_del, MODSYM_FNORMAL }, )
	D({ NULL, (DeeObject *)&posix_errno_set, MODSYM_FNORMAL }, )
	D(POSIX_STRERROR_DEF_DOC("Return the name of a given @errnum (which defaults to ?Gerrno), "
	                         /**/ "or return ?N if the error doesn't have an associated name"))
	D(POSIX_STRERRORNAME_DEF_DOC("Similar to ?Gstrerror, but instead of returning the message "
	                             /**/ "associated with a given @errnum (which defaults to ?Gerrno), "
	                             /**/ "return the name (e.g. $\"ENOENT\") of the error as a string\n"
	                             "If the given error number is not recognized, return ?N instead"))

	{ NULL }
};
#undef D

PUBLIC struct dex DEX = {
	/* .d_symbols      = */ symbols,
	/* .d_init         = */ NULL,
#ifdef HAVE_posix_dex_fini
	/* .d_fini         = */ &posix_dex_fini,
#else /* HAVE_posix_dex_fini */
	/* .d_fini         = */ NULL,
#endif /* !HAVE_posix_dex_fini */
	/* .d_import_names = */ { import_table }
};

DECL_END

#endif /* !GUARD_DEX_POSIX_LIBPOSIX_C */
