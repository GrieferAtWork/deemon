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
#ifndef GUARD_DEX_POSIX_LIBPOSIX_C
#define GUARD_DEX_POSIX_LIBPOSIX_C 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#ifndef __INTELLISENSE__
#include "p-access.c.inl"
#include "p-chmod.c.inl"
#include "p-chown.c.inl"
#include "p-copyfile.c.inl"
#include "p-cpucount.c.inl"
#include "p-environ.c.inl"
#include "p-errno.c.inl"
#include "p-exit.c.inl"
#include "p-fd.c.inl"
#include "p-mkdir.c.inl"
#include "p-open.c.inl"
#include "p-opendir.c.inl"
#include "p-pipe.c.inl"
#include "p-readlink.c.inl"
#include "p-readwrite.c.inl"
#include "p-realpath.c.inl"
#include "p-remove.c.inl"
#include "p-rename.c.inl"
#include "p-sched.c.inl"
#include "p-stat.c.inl"
#include "p-symlink.c.inl"
#include "p-sync.c.inl"
#include "p-truncate.c.inl"
#include "p-utime.c.inl"

/**/
#include "p-pwd.c.inl" /* This one has to come after "p-environ.c.inl" */

/**/
#include "p-path.c.inl" /* This one has to come after "p-pwd.c.inl" */

/* Include p-ondemand.c.inl last, since it defined functions on-demand */
#include "p-ondemand.c.inl"
#endif /* !__INTELLISENSE__ */

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>

DECL_BEGIN


/* Timestamp creation */
#if defined(NEED_DeeTime_NewUnix) || defined(NEED_DeeTime_NewFILETIME)
PRIVATE DREF DeeObject *dee_time_module = NULL;
PRIVATE WUNUSED DREF DeeObject *DCALL get_time_module(void) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if (dee_time_module == NULL)
		dee_time_module = DeeModule_ImportString("time", 4, NULL, DeeModule_IMPORT_F_NORMAL);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if (dee_time_module == NULL)
		dee_time_module = DeeModule_OpenGlobalString("time", 4, NULL, true);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return dee_time_module;
}

#ifdef HAVE_posix_dex_fini
#error "More than one posix_dex_fini() function"
#endif /* HAVE_posix_dex_fini */
#define HAVE_posix_dex_fini
PRIVATE void DCALL posix_dex_fini(void) {
	Dee_XDecref(dee_time_module);
}

#ifdef NEED_DeeTime_NewUnix
#undef NEED_DeeTime_NewUnix
INTERN DEFINE_DeeTime_NewUnix(get_time_module);
#endif /* NEED_DeeTime_NewUnix */

#ifdef NEED_DeeTime_NewFILETIME
#undef NEED_DeeTime_NewFILETIME
INTERN DEFINE_DeeTime_NewFILETIME(get_time_module);
#endif /* NEED_DeeTime_NewFILETIME */
#endif /* NEED_DeeTime_NewUnix || NEED_DeeTime_NewFILETIME */


#if defined(NEED_libposix_get_dfd_filename) || defined(__INTELLISENSE__)
#undef NEED_libposix_get_dfd_filename
INTERN WUNUSED DREF /*String*/ DeeObject *DCALL /* TODO: REMOVE ME */
libposix_get_dfd_filename(int dfd, /*utf-8*/ char const *filename, int atflags) {
	(void)dfd;
	(void)filename;
	(void)atflags;
	/* TODO: Don't try to use this function (it's gonna get removed) -- use `posix_dfd_makepath()' instead! */
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
	("posix_ftruncateat_USE_STUB",  { "ftruncateat" }),
	("posix_truncateat_USE_STUB",   { "truncateat" }),
	("posix_chmod_USE_STUB",        { "chmod" }),
	("posix_lchmod_USE_STUB",       { "lchmod" }),
	("posix_fchmod_USE_STUB",       { "fchmod" }),
	("posix_fchmodat_USE_STUB",     { "fchmodat" }),
	("posix_utime_USE_STUB",        { "utime" }),
	("posix_lutime_USE_STUB",       { "lutime" }),
	("posix_futime_USE_STUB",       { "futime" }),
	("posix_utimeat_USE_STUB",      { "utimeat" }),
	("posix_chown_USE_STUB",        { "chown" }),
	("posix_lchown_USE_STUB",       { "lchown" }),
	("posix_fchown_USE_STUB",       { "fchown" }),
	("posix_fchownat_USE_STUB",     { "fchownat" }),
	("posix_pipe_USE_STUB",         { "pipe" }),
	("posix_pipe2_USE_STUB",        { "pipe2" }),
	("posix_lrealpath_USE_STUB",    { "lrealpath" }),
	("posix_frealpath_USE_STUB",    { "frealpath" }),
	("posix_realpath_USE_STUB",     { "realpath" }),
	("posix_realpathat_USE_STUB",   { "realpathat" }),
	("posix_lresolvepath_USE_STUB", { "lresolvepath" }),
	("posix_resolvepath_USE_STUB",  { "resolvepath" }),
	("posix_resolvepathat_USE_STUB",{ "resolvepathat" }),
	("posix_strerror_USE_STUB",     { "strerror" }),
	("posix_strerrorname_USE_STUB", { "strerrorname" }),
	("posix_open_USE_STUB",         { "open", "_open" }),
	("posix_creat_USE_STUB",        { "creat", "_creat" }),
	("posix_openat_USE_STUB",       { "openat", "_openat" }),
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
	("diriter_get_d_name_IS_STUB",  { "dirent.d_name" }),
	("diriter_get_d_fullname_IS_STUB", { "dirent.d_fullname" }),
	("diriter_get_d_type_IS_STUB",  { "dirent.d_type" }),
	("diriter_get_d_ino_IS_STUB",   { "dirent.d_ino" }),
	("diriter_get_d_namlen_IS_STUB", { "dirent.d_namlen" }),
	("diriter_get_d_reclen_IS_STUB", { "dirent.d_reclen" }),
	("diriter_get_d_off_IS_STUB",   { "dirent.d_off" }),
	("diriter_get_d_dev_IS_STUB",   { "dirent.d_dev" }),
	("diriter_get_d_mode_IS_STUB",  { "dirent.d_mode" }),
	("diriter_get_d_nlink_IS_STUB", { "dirent.d_nlink" }),
	("diriter_get_d_uid_IS_STUB",   { "dirent.d_uid" }),
	("diriter_get_d_gid_IS_STUB",   { "dirent.d_gid" }),
	("diriter_get_d_rdev_IS_STUB",  { "dirent.d_rdev" }),
	("diriter_get_d_size_IS_STUB",  { "dirent.d_size" }),
	("diriter_get_d_blocks_IS_STUB", { "dirent.d_blocks" }),
	("diriter_get_d_blksize_IS_STUB", { "dirent.d_blksize" }),
	("diriter_get_d_atime_IS_STUB", { "dirent.d_atime" }),
	("diriter_get_d_mtime_IS_STUB", { "dirent.d_mtime" }),
	("diriter_get_d_ctime_IS_STUB", { "dirent.d_ctime" }),
	("diriter_get_d_birthtime_IS_STUB", { "dirent.st_birthtime" }),
	("posix_stat_get_dev_IS_STUB",  { "stat.st_dev" }),
	("posix_stat_get_ino_IS_STUB",  { "stat.st_ino" }),
	("posix_stat_get_mode_IS_STUB", { "stat.st_mode" }),
	("posix_stat_get_nlink_IS_STUB", { "stat.st_nlink" }),
	("posix_stat_get_uid_IS_STUB",  { "stat.st_uid" }),
	("posix_stat_get_gid_IS_STUB",  { "stat.st_gid" }),
	("posix_stat_get_rdev_IS_STUB", { "stat.st_rdev" }),
	("posix_stat_get_size_IS_STUB", { "stat.st_size" }),
	("posix_stat_get_blocks_IS_STUB", { "stat.st_blocks" }),
	("posix_stat_get_blksize_IS_STUB", { "stat.st_blksize" }),
	("posix_stat_get_atime_IS_STUB", { "stat.st_atime" }),
	("posix_stat_get_mtime_IS_STUB", { "stat.st_mtime" }),
	("posix_stat_get_ctime_IS_STUB", { "stat.st_ctime" }),
	("posix_stat_get_birthtime_IS_STUB", { "stat.st_birthtime" }),
	("posix_stat_isdir_IS_STUB",    { "stat.isdir" }),
	("posix_stat_ischr_IS_STUB",    { "stat.ischr" }),
	("posix_stat_isblk_IS_STUB",    { "stat.isblk" }),
	("posix_stat_isdev_IS_STUB",    { "stat.isdev" }),
	("posix_stat_isreg_IS_STUB",    { "stat.isreg" }),
	("posix_stat_isfifo_IS_STUB",   { "stat.isfifo" }),
	("posix_stat_islnk_IS_STUB",    { "stat.islnk" }),
	("posix_stat_issock_IS_STUB",   { "stat.issock" }),
	("stat_class_isexe_IS_STUB",    { "stat.isexe" }),
	("stat_class_ishidden_IS_STUB", { "stat.ishidden" }),
	("posix_gethostname_USE_STUB",  { "gethostname" }),
	("posix_chdir_USE_STUB",        { "chdir" }),
	("posix_fchdir_USE_STUB",       { "fchdir" }),
	("posix_fchdirat_USE_STUB",     { "fchdirat" }),
	("posix_unlink_USE_STUB",       { "unlink" }),
	("posix_rmdir_USE_STUB",        { "rmdir" }),
	("posix_remove_USE_STUB",       { "remove" }),
	("posix_unlinkat_USE_STUB",     { "unlinkat" }),
	("posix_rmdirat_USE_STUB",      { "rmdirat" }),
	("posix_mkdir_USE_STUB",        { "mkdir" }),
	("posix_mkdirat_USE_STUB",      { "mkdirat" }),
	("posix_fmkdirat_USE_STUB",     { "fmkdirat" }),
	("posix_symlink_USE_STUB",      { "symlink", "_symlink" }),
	("posix_symlinkat_USE_STUB",    { "symlinkat", "_symlinkat" }),
	("posix_fsymlinkat_USE_STUB",   { "fsymlinkat", "_fsymlinkat" }),
	("posix_removeat_USE_STUB",     { "removeat" }),
	("posix_readlink_USE_STUB",     { "readlink" }),
	("posix_freadlink_USE_STUB",    { "freadlink" }),
	("posix_readlinkat_USE_STUB",   { "readlinkat" }),
	("posix_rename_USE_STUB",       { "rename" }),
	("posix_frename_USE_STUB",      { "frename" }),
	("posix_renameat_USE_STUB",     { "renameat" }),
	("posix_renameat2_USE_STUB",    { "renameat2" }),
	("posix_link_USE_STUB",         { "link" }),
	("posix_flink_USE_STUB",        { "flink" }),
	("posix_linkat_USE_STUB",       { "linkat" }),
	("posix_copyfile_USE_STUB",     { "copyfile" }),
	("posix_lcopyfile_USE_STUB",    { "lcopyfile" }),
	("posix_fcopyfile_USE_STUB",    { "fcopyfile" }),
	("posix_copyfileat_USE_STUB",   { "copyfileat" }),
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
print("	Dee_hash_t         s_hash;");
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
#ifdef diriter_get_d_atime_IS_STUB
#define len_diriter_get_d_atime_IS_STUB +15
#define str_diriter_get_d_atime_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'a', 't', 'i', 'm', 'e', '\0',
#else /* diriter_get_d_atime_IS_STUB */
#define len_diriter_get_d_atime_IS_STUB /* nothing */
#define str_diriter_get_d_atime_IS_STUB /* nothing */
#endif /* !diriter_get_d_atime_IS_STUB */
#ifdef diriter_get_d_birthtime_IS_STUB
#define len_diriter_get_d_birthtime_IS_STUB +20
#define str_diriter_get_d_birthtime_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 's', 't', '_', 'b', 'i', 'r', 't', 'h', 't', 'i', 'm', 'e', '\0',
#else /* diriter_get_d_birthtime_IS_STUB */
#define len_diriter_get_d_birthtime_IS_STUB /* nothing */
#define str_diriter_get_d_birthtime_IS_STUB /* nothing */
#endif /* !diriter_get_d_birthtime_IS_STUB */
#ifdef diriter_get_d_blksize_IS_STUB
#define len_diriter_get_d_blksize_IS_STUB +17
#define str_diriter_get_d_blksize_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'b', 'l', 'k', 's', 'i', 'z', 'e', '\0',
#else /* diriter_get_d_blksize_IS_STUB */
#define len_diriter_get_d_blksize_IS_STUB /* nothing */
#define str_diriter_get_d_blksize_IS_STUB /* nothing */
#endif /* !diriter_get_d_blksize_IS_STUB */
#ifdef diriter_get_d_blocks_IS_STUB
#define len_diriter_get_d_blocks_IS_STUB +16
#define str_diriter_get_d_blocks_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'b', 'l', 'o', 'c', 'k', 's', '\0',
#else /* diriter_get_d_blocks_IS_STUB */
#define len_diriter_get_d_blocks_IS_STUB /* nothing */
#define str_diriter_get_d_blocks_IS_STUB /* nothing */
#endif /* !diriter_get_d_blocks_IS_STUB */
#ifdef diriter_get_d_ctime_IS_STUB
#define len_diriter_get_d_ctime_IS_STUB +15
#define str_diriter_get_d_ctime_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'c', 't', 'i', 'm', 'e', '\0',
#else /* diriter_get_d_ctime_IS_STUB */
#define len_diriter_get_d_ctime_IS_STUB /* nothing */
#define str_diriter_get_d_ctime_IS_STUB /* nothing */
#endif /* !diriter_get_d_ctime_IS_STUB */
#ifdef diriter_get_d_dev_IS_STUB
#define len_diriter_get_d_dev_IS_STUB +13
#define str_diriter_get_d_dev_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'd', 'e', 'v', '\0',
#else /* diriter_get_d_dev_IS_STUB */
#define len_diriter_get_d_dev_IS_STUB /* nothing */
#define str_diriter_get_d_dev_IS_STUB /* nothing */
#endif /* !diriter_get_d_dev_IS_STUB */
#ifdef diriter_get_d_fullname_IS_STUB
#define len_diriter_get_d_fullname_IS_STUB +18
#define str_diriter_get_d_fullname_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'f', 'u', 'l', 'l', 'n', 'a', 'm', 'e', '\0',
#else /* diriter_get_d_fullname_IS_STUB */
#define len_diriter_get_d_fullname_IS_STUB /* nothing */
#define str_diriter_get_d_fullname_IS_STUB /* nothing */
#endif /* !diriter_get_d_fullname_IS_STUB */
#ifdef diriter_get_d_gid_IS_STUB
#define len_diriter_get_d_gid_IS_STUB +13
#define str_diriter_get_d_gid_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'g', 'i', 'd', '\0',
#else /* diriter_get_d_gid_IS_STUB */
#define len_diriter_get_d_gid_IS_STUB /* nothing */
#define str_diriter_get_d_gid_IS_STUB /* nothing */
#endif /* !diriter_get_d_gid_IS_STUB */
#ifdef diriter_get_d_ino_IS_STUB
#define len_diriter_get_d_ino_IS_STUB +13
#define str_diriter_get_d_ino_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'i', 'n', 'o', '\0',
#else /* diriter_get_d_ino_IS_STUB */
#define len_diriter_get_d_ino_IS_STUB /* nothing */
#define str_diriter_get_d_ino_IS_STUB /* nothing */
#endif /* !diriter_get_d_ino_IS_STUB */
#ifdef diriter_get_d_mode_IS_STUB
#define len_diriter_get_d_mode_IS_STUB +14
#define str_diriter_get_d_mode_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'm', 'o', 'd', 'e', '\0',
#else /* diriter_get_d_mode_IS_STUB */
#define len_diriter_get_d_mode_IS_STUB /* nothing */
#define str_diriter_get_d_mode_IS_STUB /* nothing */
#endif /* !diriter_get_d_mode_IS_STUB */
#ifdef diriter_get_d_mtime_IS_STUB
#define len_diriter_get_d_mtime_IS_STUB +15
#define str_diriter_get_d_mtime_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'm', 't', 'i', 'm', 'e', '\0',
#else /* diriter_get_d_mtime_IS_STUB */
#define len_diriter_get_d_mtime_IS_STUB /* nothing */
#define str_diriter_get_d_mtime_IS_STUB /* nothing */
#endif /* !diriter_get_d_mtime_IS_STUB */
#ifdef diriter_get_d_name_IS_STUB
#define len_diriter_get_d_name_IS_STUB +14
#define str_diriter_get_d_name_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'n', 'a', 'm', 'e', '\0',
#else /* diriter_get_d_name_IS_STUB */
#define len_diriter_get_d_name_IS_STUB /* nothing */
#define str_diriter_get_d_name_IS_STUB /* nothing */
#endif /* !diriter_get_d_name_IS_STUB */
#ifdef diriter_get_d_namlen_IS_STUB
#define len_diriter_get_d_namlen_IS_STUB +16
#define str_diriter_get_d_namlen_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'n', 'a', 'm', 'l', 'e', 'n', '\0',
#else /* diriter_get_d_namlen_IS_STUB */
#define len_diriter_get_d_namlen_IS_STUB /* nothing */
#define str_diriter_get_d_namlen_IS_STUB /* nothing */
#endif /* !diriter_get_d_namlen_IS_STUB */
#ifdef diriter_get_d_nlink_IS_STUB
#define len_diriter_get_d_nlink_IS_STUB +15
#define str_diriter_get_d_nlink_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'n', 'l', 'i', 'n', 'k', '\0',
#else /* diriter_get_d_nlink_IS_STUB */
#define len_diriter_get_d_nlink_IS_STUB /* nothing */
#define str_diriter_get_d_nlink_IS_STUB /* nothing */
#endif /* !diriter_get_d_nlink_IS_STUB */
#ifdef diriter_get_d_off_IS_STUB
#define len_diriter_get_d_off_IS_STUB +13
#define str_diriter_get_d_off_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'o', 'f', 'f', '\0',
#else /* diriter_get_d_off_IS_STUB */
#define len_diriter_get_d_off_IS_STUB /* nothing */
#define str_diriter_get_d_off_IS_STUB /* nothing */
#endif /* !diriter_get_d_off_IS_STUB */
#ifdef diriter_get_d_rdev_IS_STUB
#define len_diriter_get_d_rdev_IS_STUB +14
#define str_diriter_get_d_rdev_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'r', 'd', 'e', 'v', '\0',
#else /* diriter_get_d_rdev_IS_STUB */
#define len_diriter_get_d_rdev_IS_STUB /* nothing */
#define str_diriter_get_d_rdev_IS_STUB /* nothing */
#endif /* !diriter_get_d_rdev_IS_STUB */
#ifdef diriter_get_d_reclen_IS_STUB
#define len_diriter_get_d_reclen_IS_STUB +16
#define str_diriter_get_d_reclen_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'r', 'e', 'c', 'l', 'e', 'n', '\0',
#else /* diriter_get_d_reclen_IS_STUB */
#define len_diriter_get_d_reclen_IS_STUB /* nothing */
#define str_diriter_get_d_reclen_IS_STUB /* nothing */
#endif /* !diriter_get_d_reclen_IS_STUB */
#ifdef diriter_get_d_size_IS_STUB
#define len_diriter_get_d_size_IS_STUB +14
#define str_diriter_get_d_size_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 's', 'i', 'z', 'e', '\0',
#else /* diriter_get_d_size_IS_STUB */
#define len_diriter_get_d_size_IS_STUB /* nothing */
#define str_diriter_get_d_size_IS_STUB /* nothing */
#endif /* !diriter_get_d_size_IS_STUB */
#ifdef diriter_get_d_type_IS_STUB
#define len_diriter_get_d_type_IS_STUB +14
#define str_diriter_get_d_type_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 't', 'y', 'p', 'e', '\0',
#else /* diriter_get_d_type_IS_STUB */
#define len_diriter_get_d_type_IS_STUB /* nothing */
#define str_diriter_get_d_type_IS_STUB /* nothing */
#endif /* !diriter_get_d_type_IS_STUB */
#ifdef diriter_get_d_uid_IS_STUB
#define len_diriter_get_d_uid_IS_STUB +13
#define str_diriter_get_d_uid_IS_STUB 'd', 'i', 'r', 'e', 'n', 't', '.', 'd', '_', 'u', 'i', 'd', '\0',
#else /* diriter_get_d_uid_IS_STUB */
#define len_diriter_get_d_uid_IS_STUB /* nothing */
#define str_diriter_get_d_uid_IS_STUB /* nothing */
#endif /* !diriter_get_d_uid_IS_STUB */
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
#ifdef posix_chmod_USE_STUB
#define len_posix_chmod_USE_STUB +6
#define str_posix_chmod_USE_STUB 'c', 'h', 'm', 'o', 'd', '\0',
#else /* posix_chmod_USE_STUB */
#define len_posix_chmod_USE_STUB /* nothing */
#define str_posix_chmod_USE_STUB /* nothing */
#endif /* !posix_chmod_USE_STUB */
#ifdef posix_chown_USE_STUB
#define len_posix_chown_USE_STUB +6
#define str_posix_chown_USE_STUB 'c', 'h', 'o', 'w', 'n', '\0',
#else /* posix_chown_USE_STUB */
#define len_posix_chown_USE_STUB /* nothing */
#define str_posix_chown_USE_STUB /* nothing */
#endif /* !posix_chown_USE_STUB */
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
#ifdef posix_copyfile_USE_STUB
#define len_posix_copyfile_USE_STUB +9
#define str_posix_copyfile_USE_STUB 'c', 'o', 'p', 'y', 'f', 'i', 'l', 'e', '\0',
#else /* posix_copyfile_USE_STUB */
#define len_posix_copyfile_USE_STUB /* nothing */
#define str_posix_copyfile_USE_STUB /* nothing */
#endif /* !posix_copyfile_USE_STUB */
#ifdef posix_copyfileat_USE_STUB
#define len_posix_copyfileat_USE_STUB +11
#define str_posix_copyfileat_USE_STUB 'c', 'o', 'p', 'y', 'f', 'i', 'l', 'e', 'a', 't', '\0',
#else /* posix_copyfileat_USE_STUB */
#define len_posix_copyfileat_USE_STUB /* nothing */
#define str_posix_copyfileat_USE_STUB /* nothing */
#endif /* !posix_copyfileat_USE_STUB */
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
#ifdef posix_fchmod_USE_STUB
#define len_posix_fchmod_USE_STUB +7
#define str_posix_fchmod_USE_STUB 'f', 'c', 'h', 'm', 'o', 'd', '\0',
#else /* posix_fchmod_USE_STUB */
#define len_posix_fchmod_USE_STUB /* nothing */
#define str_posix_fchmod_USE_STUB /* nothing */
#endif /* !posix_fchmod_USE_STUB */
#ifdef posix_fchmodat_USE_STUB
#define len_posix_fchmodat_USE_STUB +9
#define str_posix_fchmodat_USE_STUB 'f', 'c', 'h', 'm', 'o', 'd', 'a', 't', '\0',
#else /* posix_fchmodat_USE_STUB */
#define len_posix_fchmodat_USE_STUB /* nothing */
#define str_posix_fchmodat_USE_STUB /* nothing */
#endif /* !posix_fchmodat_USE_STUB */
#ifdef posix_fchown_USE_STUB
#define len_posix_fchown_USE_STUB +7
#define str_posix_fchown_USE_STUB 'f', 'c', 'h', 'o', 'w', 'n', '\0',
#else /* posix_fchown_USE_STUB */
#define len_posix_fchown_USE_STUB /* nothing */
#define str_posix_fchown_USE_STUB /* nothing */
#endif /* !posix_fchown_USE_STUB */
#ifdef posix_fchownat_USE_STUB
#define len_posix_fchownat_USE_STUB +9
#define str_posix_fchownat_USE_STUB 'f', 'c', 'h', 'o', 'w', 'n', 'a', 't', '\0',
#else /* posix_fchownat_USE_STUB */
#define len_posix_fchownat_USE_STUB /* nothing */
#define str_posix_fchownat_USE_STUB /* nothing */
#endif /* !posix_fchownat_USE_STUB */
#ifdef posix_fcopyfile_USE_STUB
#define len_posix_fcopyfile_USE_STUB +10
#define str_posix_fcopyfile_USE_STUB 'f', 'c', 'o', 'p', 'y', 'f', 'i', 'l', 'e', '\0',
#else /* posix_fcopyfile_USE_STUB */
#define len_posix_fcopyfile_USE_STUB /* nothing */
#define str_posix_fcopyfile_USE_STUB /* nothing */
#endif /* !posix_fcopyfile_USE_STUB */
#ifdef posix_fdatasync_USE_STUB
#define len_posix_fdatasync_USE_STUB +10
#define str_posix_fdatasync_USE_STUB 'f', 'd', 'a', 't', 'a', 's', 'y', 'n', 'c', '\0',
#else /* posix_fdatasync_USE_STUB */
#define len_posix_fdatasync_USE_STUB /* nothing */
#define str_posix_fdatasync_USE_STUB /* nothing */
#endif /* !posix_fdatasync_USE_STUB */
#ifdef posix_flink_USE_STUB
#define len_posix_flink_USE_STUB +6
#define str_posix_flink_USE_STUB 'f', 'l', 'i', 'n', 'k', '\0',
#else /* posix_flink_USE_STUB */
#define len_posix_flink_USE_STUB /* nothing */
#define str_posix_flink_USE_STUB /* nothing */
#endif /* !posix_flink_USE_STUB */
#ifdef posix_fmkdirat_USE_STUB
#define len_posix_fmkdirat_USE_STUB +9
#define str_posix_fmkdirat_USE_STUB 'f', 'm', 'k', 'd', 'i', 'r', 'a', 't', '\0',
#else /* posix_fmkdirat_USE_STUB */
#define len_posix_fmkdirat_USE_STUB /* nothing */
#define str_posix_fmkdirat_USE_STUB /* nothing */
#endif /* !posix_fmkdirat_USE_STUB */
#ifdef posix_freadlink_USE_STUB
#define len_posix_freadlink_USE_STUB +10
#define str_posix_freadlink_USE_STUB 'f', 'r', 'e', 'a', 'd', 'l', 'i', 'n', 'k', '\0',
#else /* posix_freadlink_USE_STUB */
#define len_posix_freadlink_USE_STUB /* nothing */
#define str_posix_freadlink_USE_STUB /* nothing */
#endif /* !posix_freadlink_USE_STUB */
#ifdef posix_frealpath_USE_STUB
#define len_posix_frealpath_USE_STUB +10
#define str_posix_frealpath_USE_STUB 'f', 'r', 'e', 'a', 'l', 'p', 'a', 't', 'h', '\0',
#else /* posix_frealpath_USE_STUB */
#define len_posix_frealpath_USE_STUB /* nothing */
#define str_posix_frealpath_USE_STUB /* nothing */
#endif /* !posix_frealpath_USE_STUB */
#ifdef posix_frename_USE_STUB
#define len_posix_frename_USE_STUB +8
#define str_posix_frename_USE_STUB 'f', 'r', 'e', 'n', 'a', 'm', 'e', '\0',
#else /* posix_frename_USE_STUB */
#define len_posix_frename_USE_STUB /* nothing */
#define str_posix_frename_USE_STUB /* nothing */
#endif /* !posix_frename_USE_STUB */
#ifdef posix_fsymlinkat_USE_STUB
#define len_posix_fsymlinkat_USE_STUB +23
#define str_posix_fsymlinkat_USE_STUB 'f', 's', 'y', 'm', 'l', 'i', 'n', 'k', 'a', 't', '\0', '_', 'f', 's', 'y', 'm', 'l', 'i', 'n', 'k', 'a', 't', '\0',
#else /* posix_fsymlinkat_USE_STUB */
#define len_posix_fsymlinkat_USE_STUB /* nothing */
#define str_posix_fsymlinkat_USE_STUB /* nothing */
#endif /* !posix_fsymlinkat_USE_STUB */
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
#ifdef posix_ftruncateat_USE_STUB
#define len_posix_ftruncateat_USE_STUB +12
#define str_posix_ftruncateat_USE_STUB 'f', 't', 'r', 'u', 'n', 'c', 'a', 't', 'e', 'a', 't', '\0',
#else /* posix_ftruncateat_USE_STUB */
#define len_posix_ftruncateat_USE_STUB /* nothing */
#define str_posix_ftruncateat_USE_STUB /* nothing */
#endif /* !posix_ftruncateat_USE_STUB */
#ifdef posix_futime_USE_STUB
#define len_posix_futime_USE_STUB +7
#define str_posix_futime_USE_STUB 'f', 'u', 't', 'i', 'm', 'e', '\0',
#else /* posix_futime_USE_STUB */
#define len_posix_futime_USE_STUB /* nothing */
#define str_posix_futime_USE_STUB /* nothing */
#endif /* !posix_futime_USE_STUB */
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
#ifdef posix_lchmod_USE_STUB
#define len_posix_lchmod_USE_STUB +7
#define str_posix_lchmod_USE_STUB 'l', 'c', 'h', 'm', 'o', 'd', '\0',
#else /* posix_lchmod_USE_STUB */
#define len_posix_lchmod_USE_STUB /* nothing */
#define str_posix_lchmod_USE_STUB /* nothing */
#endif /* !posix_lchmod_USE_STUB */
#ifdef posix_lchown_USE_STUB
#define len_posix_lchown_USE_STUB +7
#define str_posix_lchown_USE_STUB 'l', 'c', 'h', 'o', 'w', 'n', '\0',
#else /* posix_lchown_USE_STUB */
#define len_posix_lchown_USE_STUB /* nothing */
#define str_posix_lchown_USE_STUB /* nothing */
#endif /* !posix_lchown_USE_STUB */
#ifdef posix_lcopyfile_USE_STUB
#define len_posix_lcopyfile_USE_STUB +10
#define str_posix_lcopyfile_USE_STUB 'l', 'c', 'o', 'p', 'y', 'f', 'i', 'l', 'e', '\0',
#else /* posix_lcopyfile_USE_STUB */
#define len_posix_lcopyfile_USE_STUB /* nothing */
#define str_posix_lcopyfile_USE_STUB /* nothing */
#endif /* !posix_lcopyfile_USE_STUB */
#ifdef posix_link_USE_STUB
#define len_posix_link_USE_STUB +5
#define str_posix_link_USE_STUB 'l', 'i', 'n', 'k', '\0',
#else /* posix_link_USE_STUB */
#define len_posix_link_USE_STUB /* nothing */
#define str_posix_link_USE_STUB /* nothing */
#endif /* !posix_link_USE_STUB */
#ifdef posix_linkat_USE_STUB
#define len_posix_linkat_USE_STUB +7
#define str_posix_linkat_USE_STUB 'l', 'i', 'n', 'k', 'a', 't', '\0',
#else /* posix_linkat_USE_STUB */
#define len_posix_linkat_USE_STUB /* nothing */
#define str_posix_linkat_USE_STUB /* nothing */
#endif /* !posix_linkat_USE_STUB */
#ifdef posix_lrealpath_USE_STUB
#define len_posix_lrealpath_USE_STUB +10
#define str_posix_lrealpath_USE_STUB 'l', 'r', 'e', 'a', 'l', 'p', 'a', 't', 'h', '\0',
#else /* posix_lrealpath_USE_STUB */
#define len_posix_lrealpath_USE_STUB /* nothing */
#define str_posix_lrealpath_USE_STUB /* nothing */
#endif /* !posix_lrealpath_USE_STUB */
#ifdef posix_lresolvepath_USE_STUB
#define len_posix_lresolvepath_USE_STUB +13
#define str_posix_lresolvepath_USE_STUB 'l', 'r', 'e', 's', 'o', 'l', 'v', 'e', 'p', 'a', 't', 'h', '\0',
#else /* posix_lresolvepath_USE_STUB */
#define len_posix_lresolvepath_USE_STUB /* nothing */
#define str_posix_lresolvepath_USE_STUB /* nothing */
#endif /* !posix_lresolvepath_USE_STUB */
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
#ifdef posix_lutime_USE_STUB
#define len_posix_lutime_USE_STUB +7
#define str_posix_lutime_USE_STUB 'l', 'u', 't', 'i', 'm', 'e', '\0',
#else /* posix_lutime_USE_STUB */
#define len_posix_lutime_USE_STUB /* nothing */
#define str_posix_lutime_USE_STUB /* nothing */
#endif /* !posix_lutime_USE_STUB */
#ifdef posix_mkdir_USE_STUB
#define len_posix_mkdir_USE_STUB +6
#define str_posix_mkdir_USE_STUB 'm', 'k', 'd', 'i', 'r', '\0',
#else /* posix_mkdir_USE_STUB */
#define len_posix_mkdir_USE_STUB /* nothing */
#define str_posix_mkdir_USE_STUB /* nothing */
#endif /* !posix_mkdir_USE_STUB */
#ifdef posix_mkdirat_USE_STUB
#define len_posix_mkdirat_USE_STUB +8
#define str_posix_mkdirat_USE_STUB 'm', 'k', 'd', 'i', 'r', 'a', 't', '\0',
#else /* posix_mkdirat_USE_STUB */
#define len_posix_mkdirat_USE_STUB /* nothing */
#define str_posix_mkdirat_USE_STUB /* nothing */
#endif /* !posix_mkdirat_USE_STUB */
#ifdef posix_open_USE_STUB
#define len_posix_open_USE_STUB +11
#define str_posix_open_USE_STUB 'o', 'p', 'e', 'n', '\0', '_', 'o', 'p', 'e', 'n', '\0',
#else /* posix_open_USE_STUB */
#define len_posix_open_USE_STUB /* nothing */
#define str_posix_open_USE_STUB /* nothing */
#endif /* !posix_open_USE_STUB */
#ifdef posix_openat_USE_STUB
#define len_posix_openat_USE_STUB +15
#define str_posix_openat_USE_STUB 'o', 'p', 'e', 'n', 'a', 't', '\0', '_', 'o', 'p', 'e', 'n', 'a', 't', '\0',
#else /* posix_openat_USE_STUB */
#define len_posix_openat_USE_STUB /* nothing */
#define str_posix_openat_USE_STUB /* nothing */
#endif /* !posix_openat_USE_STUB */
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
#ifdef posix_realpath_USE_STUB
#define len_posix_realpath_USE_STUB +9
#define str_posix_realpath_USE_STUB 'r', 'e', 'a', 'l', 'p', 'a', 't', 'h', '\0',
#else /* posix_realpath_USE_STUB */
#define len_posix_realpath_USE_STUB /* nothing */
#define str_posix_realpath_USE_STUB /* nothing */
#endif /* !posix_realpath_USE_STUB */
#ifdef posix_realpathat_USE_STUB
#define len_posix_realpathat_USE_STUB +11
#define str_posix_realpathat_USE_STUB 'r', 'e', 'a', 'l', 'p', 'a', 't', 'h', 'a', 't', '\0',
#else /* posix_realpathat_USE_STUB */
#define len_posix_realpathat_USE_STUB /* nothing */
#define str_posix_realpathat_USE_STUB /* nothing */
#endif /* !posix_realpathat_USE_STUB */
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
#ifdef posix_resolvepath_USE_STUB
#define len_posix_resolvepath_USE_STUB +12
#define str_posix_resolvepath_USE_STUB 'r', 'e', 's', 'o', 'l', 'v', 'e', 'p', 'a', 't', 'h', '\0',
#else /* posix_resolvepath_USE_STUB */
#define len_posix_resolvepath_USE_STUB /* nothing */
#define str_posix_resolvepath_USE_STUB /* nothing */
#endif /* !posix_resolvepath_USE_STUB */
#ifdef posix_resolvepathat_USE_STUB
#define len_posix_resolvepathat_USE_STUB +14
#define str_posix_resolvepathat_USE_STUB 'r', 'e', 's', 'o', 'l', 'v', 'e', 'p', 'a', 't', 'h', 'a', 't', '\0',
#else /* posix_resolvepathat_USE_STUB */
#define len_posix_resolvepathat_USE_STUB /* nothing */
#define str_posix_resolvepathat_USE_STUB /* nothing */
#endif /* !posix_resolvepathat_USE_STUB */
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
#ifdef posix_stat_get_blksize_IS_STUB
#define len_posix_stat_get_blksize_IS_STUB +16
#define str_posix_stat_get_blksize_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'b', 'l', 'k', 's', 'i', 'z', 'e', '\0',
#else /* posix_stat_get_blksize_IS_STUB */
#define len_posix_stat_get_blksize_IS_STUB /* nothing */
#define str_posix_stat_get_blksize_IS_STUB /* nothing */
#endif /* !posix_stat_get_blksize_IS_STUB */
#ifdef posix_stat_get_blocks_IS_STUB
#define len_posix_stat_get_blocks_IS_STUB +15
#define str_posix_stat_get_blocks_IS_STUB 's', 't', 'a', 't', '.', 's', 't', '_', 'b', 'l', 'o', 'c', 'k', 's', '\0',
#else /* posix_stat_get_blocks_IS_STUB */
#define len_posix_stat_get_blocks_IS_STUB /* nothing */
#define str_posix_stat_get_blocks_IS_STUB /* nothing */
#endif /* !posix_stat_get_blocks_IS_STUB */
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
#ifdef posix_symlink_USE_STUB
#define len_posix_symlink_USE_STUB +17
#define str_posix_symlink_USE_STUB 's', 'y', 'm', 'l', 'i', 'n', 'k', '\0', '_', 's', 'y', 'm', 'l', 'i', 'n', 'k', '\0',
#else /* posix_symlink_USE_STUB */
#define len_posix_symlink_USE_STUB /* nothing */
#define str_posix_symlink_USE_STUB /* nothing */
#endif /* !posix_symlink_USE_STUB */
#ifdef posix_symlinkat_USE_STUB
#define len_posix_symlinkat_USE_STUB +21
#define str_posix_symlinkat_USE_STUB 's', 'y', 'm', 'l', 'i', 'n', 'k', 'a', 't', '\0', '_', 's', 'y', 'm', 'l', 'i', 'n', 'k', 'a', 't', '\0',
#else /* posix_symlinkat_USE_STUB */
#define len_posix_symlinkat_USE_STUB /* nothing */
#define str_posix_symlinkat_USE_STUB /* nothing */
#endif /* !posix_symlinkat_USE_STUB */
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
#ifdef posix_truncateat_USE_STUB
#define len_posix_truncateat_USE_STUB +11
#define str_posix_truncateat_USE_STUB 't', 'r', 'u', 'n', 'c', 'a', 't', 'e', 'a', 't', '\0',
#else /* posix_truncateat_USE_STUB */
#define len_posix_truncateat_USE_STUB /* nothing */
#define str_posix_truncateat_USE_STUB /* nothing */
#endif /* !posix_truncateat_USE_STUB */
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
#ifdef posix_utime_USE_STUB
#define len_posix_utime_USE_STUB +6
#define str_posix_utime_USE_STUB 'u', 't', 'i', 'm', 'e', '\0',
#else /* posix_utime_USE_STUB */
#define len_posix_utime_USE_STUB /* nothing */
#define str_posix_utime_USE_STUB /* nothing */
#endif /* !posix_utime_USE_STUB */
#ifdef posix_utimeat_USE_STUB
#define len_posix_utimeat_USE_STUB +8
#define str_posix_utimeat_USE_STUB 'u', 't', 'i', 'm', 'e', 'a', 't', '\0',
#else /* posix_utimeat_USE_STUB */
#define len_posix_utimeat_USE_STUB /* nothing */
#define str_posix_utimeat_USE_STUB /* nothing */
#endif /* !posix_utimeat_USE_STUB */
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
	len_diriter_get_d_atime_IS_STUB \
	len_diriter_get_d_birthtime_IS_STUB \
	len_diriter_get_d_blksize_IS_STUB \
	len_diriter_get_d_blocks_IS_STUB \
	len_diriter_get_d_ctime_IS_STUB \
	len_diriter_get_d_dev_IS_STUB \
	len_diriter_get_d_fullname_IS_STUB \
	len_diriter_get_d_gid_IS_STUB \
	len_diriter_get_d_ino_IS_STUB \
	len_diriter_get_d_mode_IS_STUB \
	len_diriter_get_d_mtime_IS_STUB \
	len_diriter_get_d_name_IS_STUB \
	len_diriter_get_d_namlen_IS_STUB \
	len_diriter_get_d_nlink_IS_STUB \
	len_diriter_get_d_off_IS_STUB \
	len_diriter_get_d_rdev_IS_STUB \
	len_diriter_get_d_reclen_IS_STUB \
	len_diriter_get_d_size_IS_STUB \
	len_diriter_get_d_type_IS_STUB \
	len_diriter_get_d_uid_IS_STUB \
	len_posix_access_USE_STUB \
	len_posix_chdir_USE_STUB \
	len_posix_chmod_USE_STUB \
	len_posix_chown_USE_STUB \
	len_posix_clearenv_USE_STUB \
	len_posix_close_USE_STUB \
	len_posix_copyfile_USE_STUB \
	len_posix_copyfileat_USE_STUB \
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
	len_posix_fchmod_USE_STUB \
	len_posix_fchmodat_USE_STUB \
	len_posix_fchown_USE_STUB \
	len_posix_fchownat_USE_STUB \
	len_posix_fcopyfile_USE_STUB \
	len_posix_fdatasync_USE_STUB \
	len_posix_flink_USE_STUB \
	len_posix_fmkdirat_USE_STUB \
	len_posix_freadlink_USE_STUB \
	len_posix_frealpath_USE_STUB \
	len_posix_frename_USE_STUB \
	len_posix_fsymlinkat_USE_STUB \
	len_posix_fsync_USE_STUB \
	len_posix_ftruncate_USE_STUB \
	len_posix_ftruncateat_USE_STUB \
	len_posix_futime_USE_STUB \
	len_posix_getenv_USE_STUB \
	len_posix_gethostname_USE_STUB \
	len_posix_getpid_USE_STUB \
	len_posix_isatty_USE_STUB \
	len_posix_lchmod_USE_STUB \
	len_posix_lchown_USE_STUB \
	len_posix_lcopyfile_USE_STUB \
	len_posix_link_USE_STUB \
	len_posix_linkat_USE_STUB \
	len_posix_lrealpath_USE_STUB \
	len_posix_lresolvepath_USE_STUB \
	len_posix_lseek_USE_STUB \
	len_posix_lstat_USE_STUB \
	len_posix_lutime_USE_STUB \
	len_posix_mkdir_USE_STUB \
	len_posix_mkdirat_USE_STUB \
	len_posix_open_USE_STUB \
	len_posix_openat_USE_STUB \
	len_posix_opendir_USE_STUB \
	len_posix_pipe2_USE_STUB \
	len_posix_pipe_USE_STUB \
	len_posix_pread_USE_STUB \
	len_posix_pwrite_USE_STUB \
	len_posix_read_USE_STUB \
	len_posix_readlink_USE_STUB \
	len_posix_readlinkat_USE_STUB \
	len_posix_realpath_USE_STUB \
	len_posix_realpathat_USE_STUB \
	len_posix_remove_USE_STUB \
	len_posix_removeat_USE_STUB \
	len_posix_rename_USE_STUB \
	len_posix_renameat2_USE_STUB \
	len_posix_renameat_USE_STUB \
	len_posix_resolvepath_USE_STUB \
	len_posix_resolvepathat_USE_STUB \
	len_posix_rmdir_USE_STUB \
	len_posix_rmdirat_USE_STUB \
	len_posix_setenv_USE_STUB \
	len_posix_stat_USE_STUB \
	len_posix_stat_get_atime_IS_STUB \
	len_posix_stat_get_birthtime_IS_STUB \
	len_posix_stat_get_blksize_IS_STUB \
	len_posix_stat_get_blocks_IS_STUB \
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
	len_posix_symlink_USE_STUB \
	len_posix_symlinkat_USE_STUB \
	len_posix_system_USE_STUB \
	len_posix_truncate_USE_STUB \
	len_posix_truncateat_USE_STUB \
	len_posix_umask_USE_STUB \
	len_posix_unlink_USE_STUB \
	len_posix_unlinkat_USE_STUB \
	len_posix_unsetenv_USE_STUB \
	len_posix_utime_USE_STUB \
	len_posix_utimeat_USE_STUB \
	len_posix_write_USE_STUB \
	len_stat_class_isexe_IS_STUB \
	len_stat_class_ishidden_IS_STUB \
/**/
#if POSIX_STUBS_TOTLEN != 0
PRIVATE struct {
	OBJECT_HEAD
	struct string_utf *s_data;
	Dee_hash_t         s_hash;
	size_t             s_len;
	char               s_str[POSIX_STUBS_TOTLEN];
	struct string_utf  s_utf;
} posix_missing_features = {
	OBJECT_HEAD_INIT(&DeeString_Type),
	&posix_missing_features.s_utf,
	DEE_STRING_HASH_UNSET,
	POSIX_STUBS_TOTLEN - 1, {
		str_diriter_get_d_atime_IS_STUB
		str_diriter_get_d_birthtime_IS_STUB
		str_diriter_get_d_blksize_IS_STUB
		str_diriter_get_d_blocks_IS_STUB
		str_diriter_get_d_ctime_IS_STUB
		str_diriter_get_d_dev_IS_STUB
		str_diriter_get_d_fullname_IS_STUB
		str_diriter_get_d_gid_IS_STUB
		str_diriter_get_d_ino_IS_STUB
		str_diriter_get_d_mode_IS_STUB
		str_diriter_get_d_mtime_IS_STUB
		str_diriter_get_d_name_IS_STUB
		str_diriter_get_d_namlen_IS_STUB
		str_diriter_get_d_nlink_IS_STUB
		str_diriter_get_d_off_IS_STUB
		str_diriter_get_d_rdev_IS_STUB
		str_diriter_get_d_reclen_IS_STUB
		str_diriter_get_d_size_IS_STUB
		str_diriter_get_d_type_IS_STUB
		str_diriter_get_d_uid_IS_STUB
		str_posix_access_USE_STUB
		str_posix_chdir_USE_STUB
		str_posix_chmod_USE_STUB
		str_posix_chown_USE_STUB
		str_posix_clearenv_USE_STUB
		str_posix_close_USE_STUB
		str_posix_copyfile_USE_STUB
		str_posix_copyfileat_USE_STUB
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
		str_posix_fchmod_USE_STUB
		str_posix_fchmodat_USE_STUB
		str_posix_fchown_USE_STUB
		str_posix_fchownat_USE_STUB
		str_posix_fcopyfile_USE_STUB
		str_posix_fdatasync_USE_STUB
		str_posix_flink_USE_STUB
		str_posix_fmkdirat_USE_STUB
		str_posix_freadlink_USE_STUB
		str_posix_frealpath_USE_STUB
		str_posix_frename_USE_STUB
		str_posix_fsymlinkat_USE_STUB
		str_posix_fsync_USE_STUB
		str_posix_ftruncate_USE_STUB
		str_posix_ftruncateat_USE_STUB
		str_posix_futime_USE_STUB
		str_posix_getenv_USE_STUB
		str_posix_gethostname_USE_STUB
		str_posix_getpid_USE_STUB
		str_posix_isatty_USE_STUB
		str_posix_lchmod_USE_STUB
		str_posix_lchown_USE_STUB
		str_posix_lcopyfile_USE_STUB
		str_posix_link_USE_STUB
		str_posix_linkat_USE_STUB
		str_posix_lrealpath_USE_STUB
		str_posix_lresolvepath_USE_STUB
		str_posix_lseek_USE_STUB
		str_posix_lstat_USE_STUB
		str_posix_lutime_USE_STUB
		str_posix_mkdir_USE_STUB
		str_posix_mkdirat_USE_STUB
		str_posix_open_USE_STUB
		str_posix_openat_USE_STUB
		str_posix_opendir_USE_STUB
		str_posix_pipe2_USE_STUB
		str_posix_pipe_USE_STUB
		str_posix_pread_USE_STUB
		str_posix_pwrite_USE_STUB
		str_posix_read_USE_STUB
		str_posix_readlink_USE_STUB
		str_posix_readlinkat_USE_STUB
		str_posix_realpath_USE_STUB
		str_posix_realpathat_USE_STUB
		str_posix_remove_USE_STUB
		str_posix_removeat_USE_STUB
		str_posix_rename_USE_STUB
		str_posix_renameat2_USE_STUB
		str_posix_renameat_USE_STUB
		str_posix_resolvepath_USE_STUB
		str_posix_resolvepathat_USE_STUB
		str_posix_rmdir_USE_STUB
		str_posix_rmdirat_USE_STUB
		str_posix_setenv_USE_STUB
		str_posix_stat_USE_STUB
		str_posix_stat_get_atime_IS_STUB
		str_posix_stat_get_birthtime_IS_STUB
		str_posix_stat_get_blksize_IS_STUB
		str_posix_stat_get_blocks_IS_STUB
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
		str_posix_symlink_USE_STUB
		str_posix_symlinkat_USE_STUB
		str_posix_system_USE_STUB
		str_posix_truncate_USE_STUB
		str_posix_truncateat_USE_STUB
		str_posix_umask_USE_STUB
		str_posix_unlink_USE_STUB
		str_posix_unlinkat_USE_STUB
		str_posix_unsetenv_USE_STUB
		str_posix_utime_USE_STUB
		str_posix_utimeat_USE_STUB
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
	iter = DeeObject_Iter(seq);
	Dee_Decref(seq);
	return iter;
err:
	return NULL;
}

#ifndef CONFIG_HAVE_memmem
#define CONFIG_HAVE_memmem
#undef memmem
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
	return_bool(p != NULL);
}

PRIVATE struct type_seq pst_seq = {
	/* .tp_iter     = */ &pst_iter_self,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ &pst_contains
};

PRIVATE struct type_getset tpconst pst_class_getsets[] = {
	TYPE_GETTER_NODOC("Iterator", &pst_Iterator_get),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst pst_class_members[] = {
	TYPE_MEMBER_CONST("KeyType", &DeeString_Type),
	TYPE_MEMBER_END
};


PRIVATE DeeTypeObject PosixStubsList_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_PosixStubsList",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &pst_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL, /* Static singleton, so no serial needed */
			/* tp_free:        */ NULL
		),
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
	/* .tp_seq           = */ &pst_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ pst_class_getsets,
	/* .tp_class_members = */ pst_class_members
};

PRIVATE DeeObject PosixStubsList_Singleton = { OBJECT_HEAD_INIT(&PosixStubsList_Type) };
PRIVATE WUNUSED DREF DeeObject *DCALL pst_ctor(void) {
	return_reference_(&PosixStubsList_Singleton);
}
#else /* POSIX_STUBS_TOTLEN != 0 */
/* Special case: There aren't _any_ stubs! */
#define PosixStubsList_Singleton Dee_EmptySet
#endif /* POSIX_STUBS_TOTLEN == 0 */


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
/*[[[end]]]*/



/* Stat helper functions. */
PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISDIR(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISDIR", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISDIR_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISDIR", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISDIR(args.mode));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISCHR(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISCHR", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISCHR_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISCHR", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISCHR(args.mode));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISBLK(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISBLK", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISBLK_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISBLK", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISBLK(args.mode));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISDEV(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISDEV", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISDEV_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISDEV", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISDEV(args.mode));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISREG(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISREG", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISREG_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISREG", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISREG(args.mode));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISFIFO(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISFIFO", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISFIFO_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISFIFO", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISFIFO(args.mode));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISLNK(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISLNK", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISLNK_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISLNK", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISLNK(args.mode));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_S_ISSOCK(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("S_ISSOCK", params: """
	unsigned int mode;
""", docStringPrefix: "posix");]]]*/
#define posix_S_ISSOCK_params "mode:?Dint"
	struct {
		unsigned int mode;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "S_ISSOCK", &args.mode, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	return_bool(STAT_ISSOCK(args.mode));
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(libposix_S_ISDIR, &posix_S_ISDIR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(libposix_S_ISCHR, &posix_S_ISCHR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(libposix_S_ISBLK, &posix_S_ISBLK, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(libposix_S_ISDEV, &posix_S_ISDEV, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(libposix_S_ISREG, &posix_S_ISREG, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(libposix_S_ISFIFO, &posix_S_ISFIFO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(libposix_S_ISLNK, &posix_S_ISLNK, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(libposix_S_ISSOCK, &posix_S_ISSOCK, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);



#ifdef __INTELLISENSE__
#define D(...) /* nothing */
#else /* __INTELLISENSE__ */
#define D(...) __VA_ARGS__
#endif /* !__INTELLISENSE__ */


DEX_BEGIN

/* E* errno codes */
D(POSIX_ERRNO_DEFS)
/* IMPORTANT: errno codes must come first! */

DEX_MEMBER_F("stubs", &PosixStubsList_Singleton, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
             "->?S?Dstring\n"
             "Set of the names of all of the functions that are implemented as stubs\n"
             "The names contained within this set are identical to the names of the "
             /**/ "resp. symbols exported by this module. e.g. ${'open' in posix.stubs} would "
             /**/ "mean that ?Gopen will unconditionally throw :{UnsupportedAPI}. Other functions "
             /**/ "may behave differently or always behave as no-ops, but the general meaning is "
             /**/ "that the function will simply be implemented as a stub"),

/* File control */
D(POSIX_OPEN_DEF_DOC("Open a given @filename using @oflags (a set of ${O_*} flags), and @mode (describing "
                     /**/ "the posix permissions to apply to a newly created file when ?GO_CREAT is given)"))
D(POSIX__OPEN_DEF_DOC("Same as ?Gopen, but whereas ?Gopen will automatically set the ?GO_OBTAIN_DIR and "
                      /**/ "?GO_BINARY flags on platforms that define them in order to better standartize "
                      /**/ "behavior of that function on those system, this function (?G_open) will not "
                      /**/ "make any changes to the given @oflags"))
D(POSIX_CREAT_DEF_DOC("Create a new file (same as ${open(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)})"))
D(POSIX__CREAT_DEF_DOC("Same as ?Gcreat, but on systems that define ?GO_BINARY, that flag is also passed "
                       /**/ "via the internal @oflags list eventually passed to ?Gopen (or rather ?G_open)"))
D(POSIX_OPENAT_DEF_DOC("Same as ?Gopen, but allows the path to be specified as @dfd:@filename"))
D(POSIX__OPENAT_DEF_DOC("Same as ?G_open, but allows the path to be specified as @dfd:@filename"))
D(POSIX_READ_DEF_DOC("Read up to @count bytes into @buf\n"
                     "When @buf is given, return the actual number of read bytes. "
                     /**/ "Otherwise, read into a new ?GBytes object that is then returned"))
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
/* TODO: fcntl commands */
/* TODO: ioctl commands */

/* Filesystem control */
D(POSIX_TRUNCATE_DEF)
D(POSIX_FTRUNCATE_DEF)
D(POSIX_TRUNCATEAT_DEF)
D(POSIX_FTRUNCATEAT_DEF)
D(POSIX_ACCESS_DEF)
D(POSIX_EUIDACCESS_DEF)
D(POSIX_FACCESSAT_DEF)
D(POSIX_FCHOWNAT_DEF)
/* TODO: chflags() */
/* TODO: lchflags() */
/* TODO: chroot() */
/* TODO: mkfifo() */
/* TODO: mknod() */
/* TODO: major() */
/* TODO: minor() */
/* TODO: mkdev() */
D(POSIX_SYNC_DEF)
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
/* TODO: fremovexattr() */
/* TODO: listxattr() */
/* TODO: llistxattr() */
/* TODO: flistxattr() */

/* Path normalization */
D(POSIX_LREALPATH_DEF_DOC("#tFileAccessError{Permissions to access @path were denied (consider using ?Glresolvepath)}"
                          "#tFileNotFound{Some part of @path does not exist (consider using ?Glresolvepath)}"
                          "#tNoDirectory{Some part of @path isn't a directory (consider using ?Glresolvepath)}"
                          "#tSystemError{Failed to resolve the realpath for some reason}"
                          "Return the absolute, fully resolved, canonical name of @path.\n"
                          "This function also ensures that @path actually exists. If "
                          /**/ "this is not a requirement, consider using ?Glresolvepath instead.\n"
                          "If @path is a symbolic link, don't resolve that link but return the path to the link itself"))
D(POSIX_REALPATH_DEF_DOC("#tFileAccessError{Permissions to access @path were denied (consider using ?Gresolvepath)}"
                         "#tFileNotFound{Some part of @path does not exist (consider using ?Gresolvepath)}"
                         "#tNoDirectory{Some part of @path isn't a directory (consider using ?Gresolvepath)}"
                         "#tSystemError{Failed to resolve the realpath for some reason}"
                         "Return the absolute, fully resolved, canonical name of @path.\n"
                         "This function also ensures that @path actually exists. If "
                         /**/ "this is not a requirement, consider using ?Gresolvepath instead"))
D(POSIX_FREALPATH_DEF_DOC("#tSystemError{Failed to resolve the realpath for some reason}"
                          "#patflags{Should always be $0, except on KOS where ?GAT_DOSPATH can also be passed}"
                          "Return the absolute, fully resolved, canonical name of @fd"))
D(POSIX_REALPATHAT_DEF_DOC("#tFileAccessError{Permissions to access @dfd:@path were denied (consider using ?Gresolvepathat)}"
                           "#tFileNotFound{Some part of @dfd:@path does not exist (consider using ?Gresolvepathat)}"
                           "#tNoDirectory{Some part of @dfd:@path isn't a directory (consider using ?Gresolvepathat)}"
                           "#tSystemError{Failed to resolve the realpath for some reason}"
                           "#patflags{Set of ?GAT_SYMLINK_NOFOLLOW, ?GAT_DOSPATH}"
                           "Return the absolute, fully resolved, canonical name of @dfd:@path.\n"
                           "This function also ensures that @dfd:@path actually exists. If "
                           /**/ "this is not a requirement, consider using ?Gresolvepathat instead"))
D(POSIX_LRESOLVEPATH_DEF_DOC("#tSystemError{Failed to resolve the path for some reason}"
                             "Same as ?Glrealpath, but don't force the returned path to be absolute (if @path is "
                             /**/ "relative, then so will the returned path be), and handle errors by simply not "
                             /**/ "expanding a relevant portion of the path"))
D(POSIX_RESOLVEPATH_DEF_DOC("#tSystemError{Failed to resolve the path for some reason}"
                            "Same as ?Grealpath, but don't force the returned path to be absolute (if @path is "
                            /**/ "relative, then so will the returned path be), and handle errors by simply not "
                            /**/ "expanding a relevant portion of the path"))
D(POSIX_RESOLVEPATHAT_DEF_DOC("#tSystemError{Failed to resolve the path for some reason}"
                              "Same as ?Grealpathat, but don't force the returned path to be absolute (if @dfd:@path "
                              /**/ "is relative, then so will the returned path be), and handle errors by simply not "
                              /**/ "expanding a relevant portion of the path"))

/* System information */
/* TODO: uname() */
/* TODO: sethostname() */
/* TODO: setdomainname() */
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
/* TODO: gettid() */
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
/* TODO: geteuid() */ /* nt: GetTokenInformation(OpenProcessToken(), TokenOwner) */
/* TODO: seteuid() */
/* TODO: getegid() */ /* nt: GetTokenInformation(OpenProcessToken(), TokenPrimaryGroup) */
/* TODO: setegid() */
/* TODO: getuid() */ /* nt: LookupAccountName(GetUserName()) */
/* TODO: setuid() */
/* TODO: getgid() */ /* nt: ??? */
/* TODO: setgid() */
/* TODO: setreuid() */
/* TODO: setregid() */
/* TODO: setresuid() */
/* TODO: setresgid() */
/* TODO: getresuid() */
/* TODO: getresgid() */
/* TODO: getgrouplist() */
/* TODO: getgroups() */ /* nt: GetTokenInformation(OpenProcessToken(), TokenGroups) */
/* TODO: setgroups() */
/* TODO: initgroups() */

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
D(DEX_MEMBER_F_NODOC("dirent", &DeeDirIterator_Type, DEXSYM_READONLY),)
D(DEX_MEMBER_F_NODOC("DIR", &DeeDir_Type, DEXSYM_READONLY),)
D(DEX_MEMBER_F("opendir", &DeeDir_Type, DEXSYM_READONLY,
               "(" posix_opendir_params ")->?GDIR\n"
               "Read the contents of a given directory. The returned "
               /**/ "object may be iterated to yield ?Gdirent objects.\n"
               "Additionally, you may specify @skipdots as ?f if you "
               /**/ "wish to include the special $'.' and $'..' entries."),)
D(DEX_MEMBER_F("fdopendir", &posix_fdopendir, DEXSYM_READONLY,
               "(" posix_fdopendir_params ")->?GDIR\n"
               "Same as ?Gopendir, but the default value of @inheritfd is ?t, "
               /**/ "mimicking the behavior of the native $fdopendir function"),)

/* File type constants. */
D(DEX_MEMBER_F_NODOC("DT_UNKNOWN", &posix_DT_UNKNOWN, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_FIFO", &posix_DT_FIFO, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_CHR", &posix_DT_CHR, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_DIR", &posix_DT_DIR, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_BLK", &posix_DT_BLK, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_REG", &posix_DT_REG, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_LNK", &posix_DT_LNK, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_SOCK", &posix_DT_SOCK, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F_NODOC("DT_WHT", &posix_DT_WHT, DEXSYM_READONLY | DEXSYM_CONSTEXPR),)
D(DEX_MEMBER_F("DTTOIF", &posix_DTTOIF, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "(dt:?Dint)->?Dint\n"
               "Convert a #C{DT_*} constant to #C{S_IF*}"),)
D(DEX_MEMBER_F("IFTODT", &posix_IFTODT, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "(if:?Dint)->?Dint\n"
               "Convert an #C{S_IF*} constant to #C{DT_*}"),)

/* Environ control */
D(POSIX_GETENV_DEF_DOC("#tKeyError{The given @varname wasn't found, and @defl wasn't given}"
                       "Same as ${environ[varname]}\n"))
D(POSIX_SETENV_DEF_DOC("Same as ${environ[varname] = value}. When @replace is ?f, only "
                       "add new variables, but don't override one that was already set\n"))
D(POSIX_PUTENV_DEF_DOC("If @envline constains $'=', same as ${local name, none, value = envline.partition(\"=\")...; setenv(name, value);}\n"
                       "Otherwise, same as ?#unsetenv"))
D(POSIX_UNSETENV_DEF_DOC("Returns ?t if @varname was deleted, and ?f if @varname didn't exist in ?Genviron"))
D(POSIX_CLEARENV_DEF_DOC("Clear ?Genviron"))
D(DEX_MEMBER_F("environ", &DeeEnviron_Singleton, DEXSYM_READONLY,
               "->?M?Dstring?Dstring\n"
               "A ?DMapping-style singleton instance that can be used to "
               /**/ "access and enumerate environment variables by name:\n"
               "${"
               /**/ "print environ[\"PATH\"]; /* \"/bin:/usr/bin:...\" */"
               "}\n"
               "Other mapping operations known from ?DMapping can be used "
               /**/ "to delete (${del environ[...]}), set (${environ[...] = ...}) and "
               /**/ "check for the existance of (${... in environ}) environment variables, "
               /**/ "as well as enumerating all variables (${for (key, item: environ) ...})"),)

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
DEX_MEMBER_F("S_ISDIR", &libposix_S_ISDIR, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISDIR_params ")->?Dbool"),
DEX_MEMBER_F("S_ISCHR", &libposix_S_ISCHR, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISCHR_params ")->?Dbool"),
DEX_MEMBER_F("S_ISBLK", &libposix_S_ISBLK, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISBLK_params ")->?Dbool"),
DEX_MEMBER_F("S_ISDEV", &libposix_S_ISDEV, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISDEV_params ")->?Dbool"),
DEX_MEMBER_F("S_ISREG", &libposix_S_ISREG, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISREG_params ")->?Dbool"),
DEX_MEMBER_F("S_ISFIFO", &libposix_S_ISFIFO, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISFIFO_params ")->?Dbool"),
DEX_MEMBER_F("S_ISLNK", &libposix_S_ISLNK, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISLNK_params ")->?Dbool"),
DEX_MEMBER_F("S_ISSOCK", &libposix_S_ISSOCK, DEXSYM_READONLY | DEXSYM_CONSTEXPR, "(" posix_S_ISSOCK_params ")->?Dbool"),

/* stat & friends */
D(DEX_MEMBER_F_NODOC("stat", &DeeStat_Type, DEXSYM_READONLY),)
D(DEX_MEMBER_F_NODOC("lstat", &DeeLStat_Type, DEXSYM_READONLY),)
D(POSIX_FSTAT_DEF_DOC("More restrictive alias for ?Gstat"))
D(POSIX_FSTATAT_DEF_DOC("More restrictive alias for ?Gstat"))

/* Process Environment */
D(POSIX_GETCWD_DEF_DOC("#t{:Interrupt}"
                       "#tFileAccessError{Permission to read a part of the current working directory's path was denied}"
                       "#tFileNotFound{The current working directory has been unlinked}"
                       "#tSystemError{Failed to retrieve the current working directory for some reason}"
                       "Return the absolute path of the current working directory"))
D(POSIX_GETTMP_DEF_DOC("#t{:Interrupt}"
                       "#tSystemError{Failed to retrieve a temporary path name for some reason}"
                       "Return the path to a folder that can be used as temporary storage of files and directories\n"
                       "If (in this order) one of these environment variables is defined, "
                       /**/ "it will be returned $'TMPDIR', $'TMP', $'TEMP', $'TEMPDIR'"))
D(POSIX_GETHOSTNAME_DEF_DOC("#t{:Interrupt}"
                            "#tSystemError{Failed to retrieve the name of the hosting machine for some reason}"
                            "Returns the user-assigned name of the hosting machine"))
D(POSIX_CHDIR_DEF_DOC("#t{:Interrupt}"
                      "#tFileNotFound{The given @path could not be found}"
                      "#tNoDirectory{The given @path is not a directory}"
                      "#tFileAccessError{The current user does not have permissions to enter @path}"
                      "#tSystemError{Failed to change the current working directory for some reason}"
                      "Change the current working directory to @path, which may be "
                      /**/ "a path relative to the old current working directory"))
D(POSIX_FCHDIR_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @fd could not be found}"
                       "#tNoDirectory{The given @fd is not a directory}"
                       "#tFileClosed{The given @fd has been closed or is invalid}"
                       "#tFileAccessError{The current user does not have permissions to enter @fd}"
                       "#tSystemError{Failed to change the current working directory for some reason}"
                       "Change the current working directory to @fd"))
D(POSIX_FCHDIRAT_DEF_DOC("#t{:Interrupt}"
                         "#tFileNotFound{The given @dfd:@path could not be found}"
                         "#tNoDirectory{The given @dfd:@path is not a directory}"
                         "#tFileClosed{The given @dfd has been closed or is invalid}"
                         "#tFileAccessError{The current user does not have permissions to enter @dfd:@path}"
                         "#tSystemError{Failed to change the current working directory for some reason}"
                         "#tValueError{Invalid set of flags specified by @atflags}"
                         "Change the current working directory to @dfd:@path"))

/************************************************************************/
/* Filesystem                                                           */
/************************************************************************/

/* mkdir(2) */
D(POSIX_MKDIR_DEF_DOC("#t{:Interrupt}"
                      "#tFileNotFound{One or more of @path's parents do not exist}"
                      "#tNoDirectory{A part of the given @path is not a directory}"
                      "#tFileExists{The given @path already exists}"
                      "#tValueError{The given @permissions are malformed or not recognized}"
                      "#tFileAccessError{The current user does not have permissions to "
                      /*                  */ "create a new directory within the folder of @path}"
                      "#tReadOnlyFile{The filesystem or device hosting the directory of "
                      /*               */ "@path is in read-only operations mode, preventing the "
                      /*               */ "creation of new directories}"
                      "#tSystemError{Failed to create a directory for some reason}"
                      "Create a new directory named @path"))
D(POSIX_MKDIRAT_DEF_DOC("#t{:Interrupt}"
                        "#tFileNotFound{One or more of @dfd:@path's parents do not exist}"
                        "#tNoDirectory{A part of the given @dfd:@path is not a directory}"
                        "#tFileExists{The given @dfd:@path already exists}"
                        "#tValueError{The given @permissions are malformed or not recognized}"
                        "#tFileAccessError{The current user does not have permissions to "
                        /*                  */ "create a new directory within the folder of @dfd:@path}"
                        "#tReadOnlyFile{The filesystem or device hosting the directory of "
                        /*               */ "@dfd:@path is in read-only operations mode, preventing the "
                        /*               */ "creation of new directories}"
                        "#tSystemError{Failed to create a directory for some reason}"
                        "#tFileClosed{The given @dfd was closed}"
                        "Create a new directory named @dfd:@path"))
D(POSIX_FMKDIRAT_DEF_DOC("#t{:Interrupt}"
                         "#tFileNotFound{One or more of @dfd:@path's parents do not exist}"
                         "#tNoDirectory{A part of the given @dfd:@path is not a directory}"
                         "#tFileExists{The given @dfd:@path already exists}"
                         "#tValueError{The given @permissions are malformed or not recognized}"
                         "#tFileAccessError{The current user does not have permissions to "
                         /*                  */ "create a new directory within the folder of @dfd:@path}"
                         "#tReadOnlyFile{The filesystem or device hosting the directory of "
                         /*               */ "@dfd:@path is in read-only operations mode, preventing the "
                         /*               */ "creation of new directories}"
                         "#tSystemError{Failed to create a directory for some reason}"
                         "#tFileClosed{The given @dfd was closed}"
                         "Create a new directory named @dfd:@path"))

/* TODO: mkdir_p() -- Same as `mkdir()', but:
	 * - Also create missing parent directories
	 * - Ignore directory-already-exists errors (but only if the already-existing thing *actually* is a directory) */

/* symlink(2) */
D(POSIX_SYMLINK_DEF_DOC("#t{:Interrupt}"
                        "#tFileExists{A file or directory named @path already exists}"
                        "#tFileNotFound{A parent directory of @path does not exist}"
                        "#tNoDirectory{A part of the given @path is not a directory}"
                        "#tUnsupportedAPI{The underlying filesystem does not support symbolic links}"
                        "#tFileAccessError{The current user does not have permissions to access the "
                        /*                  */ "directory containing the non-existent file @path for writing}"
                        "#tReadOnlyFile{The filesystem or device hosting the directory "
                        /*               */ "containing the non-existent object @path is "
                        /*               */ "in read-only operations mode, preventing the "
                        /*               */ "creation of new symbolic links}"
                        "#tSystemError{Failed to create a symbolic link under @path for some reason}"
                        "Symbolic links are filesystem redirection points which you can think of as "
                        /**/ "keyword-style macros that exist in directories. When addressed, simply imagine "
                        /**/ "their name being replaced with @text, at which point the resulting path "
                        /**/ "is then re-evaluated:\n"
                        "${"
                        /**/ "import symlink from fs;\n"
                        /**/ "import File from deemon;\n"
                        /**/ "symlink(\"../foo\", \"/path/to/link\");\n"
                        /**/ "/* \"/path/to/[link]/file.txt\" */\n"
                        /**/ "/* \"/path/to/[../foo]/file.txt\" */\n"
                        /**/ "/* \"/path/foo/file.txt\" */\n"
                        /**/ "File.open(\"/path/to/link/file.txt\");"
                        "}"))
D(POSIX_SYMLINKAT_DEF_DOC("#t{:Interrupt}"
                          "#tFileExists{A file or directory named @dfd:@path already exists}"
                          "#tFileNotFound{A parent directory of @dfd:@path does not exist}"
                          "#tNoDirectory{A part of the given @dfd:@path is not a directory}"
                          "#tUnsupportedAPI{The underlying filesystem does not support symbolic links}"
                          "#tFileAccessError{The current user does not have permissions to access the "
                          /*                  */ "directory containing the non-existent file @dfd:@path for writing}"
                          "#tReadOnlyFile{The filesystem or device hosting the directory "
                          /*               */ "containing the non-existent object @dfd:@path is "
                          /*               */ "in read-only operations mode, preventing the "
                          /*               */ "creation of new symbolic links}"
                          "#tSystemError{Failed to create a symbolic link under @dfd:@path for some reason}"
                          "#tFileClosed{The given @dfd was closed}"
                          "Symbolic links are filesystem redirection points which you can think of as "
                          /**/ "keyword-style macros that exist in directories. When addressed, simply imagine "
                          /**/ "their name being replaced with @text, at which point the resulting path "
                          /**/ "is then re-evaluated:\n"
                          "${"
                          /**/ "import symlink from fs;\n"
                          /**/ "import File from deemon;\n"
                          /**/ "symlink(\"../foo\", \"/path/to/link\");\n"
                          /**/ "/* \"/path/to/[link]/file.txt\" */\n"
                          /**/ "/* \"/path/to/[../foo]/file.txt\" */\n"
                          /**/ "/* \"/path/foo/file.txt\" */\n"
                          /**/ "File.open(\"/path/to/link/file.txt\");"
                          "}"))
D(POSIX_FSYMLINKAT_DEF_DOC("#t{:Interrupt}"
                           "#tFileExists{A file or directory named @dfd:@path already exists}"
                           "#tFileNotFound{A parent directory of @dfd:@path does not exist}"
                           "#tNoDirectory{A part of the given @dfd:@path is not a directory}"
                           "#tUnsupportedAPI{The underlying filesystem does not support symbolic links}"
                           "#tFileAccessError{The current user does not have permissions to access the "
                           /*                  */ "directory containing the non-existent file @dfd:@path for writing}"
                           "#tReadOnlyFile{The filesystem or device hosting the directory "
                           /*               */ "containing the non-existent object @dfd:@path is "
                           /*               */ "in read-only operations mode, preventing the "
                           /*               */ "creation of new symbolic links}"
                           "#tSystemError{Failed to create a symbolic link under @dfd:@path for some reason}"
                           "#tFileClosed{The given @dfd was closed}"
                           "Symbolic links are filesystem redirection points which you can think of as "
                           /**/ "keyword-style macros that exist in directories. When addressed, simply imagine "
                           /**/ "their name being replaced with @text, at which point the resulting path "
                           /**/ "is then re-evaluated:\n"
                           "${"
                           /**/ "import symlink from fs;\n"
                           /**/ "import File from deemon;\n"
                           /**/ "symlink(\"../foo\", \"/path/to/link\");\n"
                           /**/ "/* \"/path/to/[link]/file.txt\" */\n"
                           /**/ "/* \"/path/to/[../foo]/file.txt\" */\n"
                           /**/ "/* \"/path/foo/file.txt\" */\n"
                           /**/ "File.open(\"/path/to/link/file.txt\");"
                           "}"))
D(POSIX__SYMLINK_DEF_DOC("Same as ?Gsymlink, but don't perform any OS-specific transformations on @text"))
D(POSIX__SYMLINKAT_DEF_DOC("Same as ?Gsymlinkat, but don't perform any OS-specific transformations on @text"))
D(POSIX__FSYMLINKAT_DEF_DOC("Same as ?Gfsymlinkat, but don't perform any OS-specific transformations on @text"))

/* unlink(2) */
D(POSIX_UNLINK_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @file does not exist}"
                       "#tNoDirectory{A part of the given @file is not a directory}"
                       "#tIsDirectory{The given @file is a directory and ?Grmdir or ?Gremove must be used to delete it}"
                       "#tBusyFile{The given @file is currently being used and cannot be deleted}"
                       "#tFileAccessError{The current user does not have permissions to remove the file described by @file}"
                       "#tReadOnlyFile{The filesystem or device hosting the directory of @file is in read-only "
                       /*               */ "operations mode, preventing the deletion of existing files}"
                       "#tSystemError{Failed to unlink the given file @file for some reason}"
                       "Remove a non-directory filesystem object named @file"))
D(POSIX_UNLINKAT_DEF_DOC("#t{:Interrupt}"
                         "#tFileNotFound{The given @dfd:@file does not exist}"
                         "#tNoDirectory{A part of the given @dfd:@file is not a directory}"
                         "#tIsDirectory{The given @dfd:@file is a directory and ?Grmdir or ?Gremove must be used to delete it}"
                         "#tBusyFile{The given @dfd:@file is currently being used and cannot be deleted}"
                         "#tFileAccessError{The current user does not have permissions to remove the file described by @dfd:@file}"
                         "#tReadOnlyFile{The filesystem or device hosting the directory of @dfd:@file is in read-only "
                         /*               */ "operations mode, preventing the deletion of existing files}"
                         "#tFileClosed{The given @fd was closed}"
                         "#tSystemError{Failed to unlink the given file @dfd:@file for some reason}"
                         "#patflags{Set of ?GAT_REMOVEDIR, ?GAT_REMOVEREG}"
                         "Remove a filesystem object named @dfd:@file"))

/* rmdir(2) */
D(POSIX_RMDIR_DEF_DOC("#t{:Interrupt}"
                      "#tFileNotFound{The given @path does not exist}"
                      "#tNoDirectory{A part of the given @path is not a directory}"
                      "#tNoDirectory{The given @path isn't a directory and either ?Gunlink or ?Gremove must be used to "
                      /*              */ "delete it, or it is a mounting point if such functionality is supported by the host}"
                      "#tNotEmpty{The given directory @path isn't empty empty}"
                      "#tBusyFile{The given @path is currently being used and cannot be deleted}"
                      "#tFileAccessError{The current user does not have permissions to remove the directory described by @path}"
                      "#tReadOnlyFile{The filesystem or device hosting the directory of @path is in read-only "
                      /*               */ "operations mode, preventing the deletion of existing directories}"
                      "#tSystemError{Failed to delete the directory @path for some reason}"
                      "Remove a directory named @path"))
D(POSIX_RMDIRAT_DEF_DOC("#t{:Interrupt}"
                        "#tFileNotFound{The given @dfd:@path does not exist}"
                        "#tNoDirectory{A part of the given @dfd:@path is not a directory}"
                        "#tNoDirectory{The given @dfd:@path isn't a directory and either ?Gunlink or ?Gremove must be used to "
                        /*              */ "delete it, or it is a mounting point if such functionality is supported by the host}"
                        "#tNotEmpty{The given directory @dfd:@path isn't empty empty}"
                        "#tBusyFile{The given @dfd:@path is currently being used and cannot be deleted}"
                        "#tFileAccessError{The current user does not have permissions to remove the directory described by @dfd:@path}"
                        "#tReadOnlyFile{The filesystem or device hosting the directory of @dfd:@path is in read-only "
                        /*               */ "operations mode, preventing the deletion of existing directories}"
                        "#tFileClosed{The given @dfd was closed}"
                        "#tSystemError{Failed to delete the directory @dfd:@path for some reason}"
                        "Remove a directory named @dfd:@path"))

/* remove(3) */
D(POSIX_REMOVE_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @path does not exist}"
                       "#tNoDirectory{A part of the given @path is not a directory}"
                       "#tNotEmpty{The given @path is a directory that isn't empty empty}"
                       "#tBusyFile{The given @path is currently being used and cannot be deleted}"
                       "#tFileAccessError{The current user does not have permissions to remove the file or directory described by @path}"
                       "#tReadOnlyFile{The filesystem or device hosting the directory of @path is in read-only "
                       /*               */ "operations mode, preventing the deletion of existing files or directories}"
                       "#tSystemError{Failed to remove the given file @path for some reason}"
                       "Remove a file or an empty directory name @path"))
D(POSIX_REMOVEAT_DEF_DOC("#t{:Interrupt}"
                         "#tFileNotFound{The given @dfd:@path does not exist}"
                         "#tNoDirectory{A part of the given @dfd:@path is not a directory}"
                         "#tNotEmpty{The given @dfd:@path is a directory that isn't empty empty}"
                         "#tBusyFile{The given @dfd:@path is currently being used and cannot be deleted}"
                         "#tFileAccessError{The current user does not have permissions to remove the file or directory described by @dfd:@path}"
                         "#tReadOnlyFile{The filesystem or device hosting the directory of @dfd:@path is in read-only "
                         /*               */ "operations mode, preventing the deletion of existing files or directories}"
                         "#tFileClosed{The given @fd was closed}"
                         "#tSystemError{Failed to remove the given file @dfd:@path for some reason}"
                         "Remove a file or an empty directory name @dfd:@path"))

/* readlink(2) */
D(POSIX_READLINK_DEF_DOC("#t{:Interrupt}"
                         "#tFileNotFound{The given @file does not exist}"
                         "#tNoDirectory{A part of the given @file is not a directory}"
                         "#tNoLink{The given @file does not refer to a symbolic link}"
                         "#tValueError{The file described by @file is not a symlink}"
                         "#tUnsupportedAPI{The underlying filesystem does not support reading of symbolic links}"
                         "#tFileAccessError{The current user does not have permissions to access @file or one of the containing directories for reading}"
                         "#tSystemError{Failed to read the symbolic link under @file for some reason}"
                         "Read and return the targetText used to create a symbolic link (see ?Gsymlink)"))
D(POSIX_FREADLINK_DEF_DOC("#t{:Interrupt}"
                          "#tNoLink{The given @fd does not refer to a symbolic link}"
                          "#tValueError{The file described by @fd is not a symlink}"
                          "#tUnsupportedAPI{The underlying filesystem does not support reading of symbolic links}"
                          "#tFileAccessError{The current user does not have permissions to access @fd or one of the containing directories for reading}"
                          "#tSystemError{Failed to read the symbolic link under @fd for some reason}"
                          "#tFileClosed{The given @fd was closed}"
                          "Read and return the targetText used to create a symbolic link (see ?Gsymlink)"))
D(POSIX_READLINKAT_DEF_DOC("#t{:Interrupt}"
                           "#tFileNotFound{The given @dfd:@file does not exist}"
                           "#tNoDirectory{A part of the given @dfd:@file is not a directory}"
                           "#tNoLink{The given @dfd:@file does not refer to a symbolic link}"
                           "#tValueError{The file described by @dfd:@file is not a symlink}"
                           "#tUnsupportedAPI{The underlying filesystem does not support reading of symbolic links}"
                           "#tFileAccessError{The current user does not have permissions to access @dfd:@file or one of the containing directories for reading}"
                           "#tFileClosed{The given @dfd was closed}"
                           "#tSystemError{Failed to read the symbolic link under @dfd:@file for some reason}"
                           "Read and return the targetText used to create a symbolic link (see ?Gsymlink)"))

/* rename(2) */
D(POSIX_RENAME_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @oldpath could not be found, or a parent directory of @newpath does not exist}"
                       "#tNotEmpty{The given @newpath is a non-empty directory}"
                       "#tIsDirectory{The given @newpath is an existing directory, but @oldpath isn't "
                       /*              */ "one (directories can only be replaced by other directories)}"
                       "#tNoDirectory{A part of the given @oldpath or @newpath is not a directory, "
                       /*              */ "or @oldpath is a directory, but @newpath isn't}"
                       "#tCrossDevice{The given @oldpath and @newpath are not located on the same device}"
                       "#tValueError{Attempted to move a directory into itself}"
                       "#tFileAccessError{The current user does not have permissions to access the file "
                       /*                  */ "or directory @oldpath, or the directory containing @newpath}"
                       "#tReadOnlyFile{The filesystem or device hosting the given file or directory @oldpath or the "
                       /*               */ "directory containing the non-existent file @newpath is in read-only operations "
                       /*               */ "mode, preventing the modification of existing files or directories}"
                       "#tSystemError{Failed to rename the given @oldpath for some reason}"
                       "Renames or moves a given @oldpath to be referred to as @newpath from then on\n"
                       "When @newpath already exists, it is replaced by @oldpath"))
D(POSIX_FRENAME_DEF_DOC("#t{:Interrupt}"
                        "#tFileNotFound{A parent directory of @newpath does not exist}"
                        "#tNotEmpty{The given @newpath is a non-empty directory}"
                        "#tIsDirectory{The given @newpath is an existing directory, but @oldfd isn't "
                        /*              */ "one (directories can only be replaced by other directories)}"
                        "#tNoDirectory{A part of @newpath is not a directory, "
                        /*              */ "or @oldfd is a directory, but @newpath isn't}"
                        "#tCrossDevice{The given @oldfd and @newpath are not located on the same device}"
                        "#tValueError{Attempted to move a directory into itself}"
                        "#tFileAccessError{The current user does not have permissions to access the file "
                        /*                  */ "or directory @oldfd, or the directory containing @newpath}"
                        "#tReadOnlyFile{The filesystem or device hosting the given file or directory @oldfd or the "
                        /*               */ "directory containing the non-existent file @newpath is in read-only operations "
                        /*               */ "mode, preventing the modification of existing files or directories}"
                        "#tFileClosed{The given @oldfd has already been closed}"
                        "#tSystemError{Failed to rename the given @oldfd for some reason}"
                        "Renames or moves a given @oldfd to be referred to as @newpath from then on\n"
                        "When @newpath already exists, it is replaced by @oldfd"))
D(POSIX_RENAMEAT_DEF_DOC("#t{:Interrupt}"
                         "#tFileNotFound{The given @olddirfd:@oldpath could not be found, or a parent directory of @newdirfd:@newpath does not exist}"
                         "#tNotEmpty{The given @newdirfd:@newpath is a non-empty directory}"
                         "#tIsDirectory{The given @newdirfd:@newpath is an existing directory, but @olddirfd:@oldpath isn't "
                         /*              */ "one (directories can only be replaced by other directories)}"
                         "#tNoDirectory{A part of the given @olddirfd:@oldpath or @newdirfd:@newpath is not a directory, "
                         /*              */ "or @olddirfd:@oldpath is a directory, but @newdirfd:@newpath isn't}"
                         "#tCrossDevice{The given @olddirfd:@oldpath and @newdirfd:@newpath are not located on the same device}"
                         "#tValueError{Attempted to move a directory into itself}"
                         "#tFileAccessError{The current user does not have permissions to access the file "
                         /*                  */ "or directory @olddirfd:@oldpath, or the directory containing @newdirfd:@newpath}"
                         "#tReadOnlyFile{The filesystem or device hosting the given file or directory @olddirfd:@oldpath or the "
                         /*               */ "directory containing the non-existent file @newdirfd:@newpath is in read-only operations "
                         /*               */ "mode, preventing the modification of existing files or directories}"
                         "#tFileClosed{The given @olddirfd or @newdirfd has already been closed}"
                         "#tValueError{Invalid @atflags}"
                         "#tSystemError{Failed to rename the given @olddirfd:@oldpath for some reason}"
                         "Renames or moves a given @olddirfd:@oldpath to be referred to as @newdirfd:@newpath from then on\n"
                         "When @newpath already exists, it is replaced by @olddirfd:@oldpath"))
D(POSIX_RENAMEAT2_DEF_DOC("#t{:Interrupt}"
                          "#tFileNotFound{The given @olddirfd:@oldpath could not be found, or a parent directory of @newdirfd:@newpath does not exist}"
                          "#tNotEmpty{The given @newdirfd:@newpath is a non-empty directory}"
                          "#tIsDirectory{The given @newdirfd:@newpath is an existing directory, but @olddirfd:@oldpath isn't "
                          /*              */ "one (directories can only be replaced by other directories)}"
                          "#tNoDirectory{A part of the given @olddirfd:@oldpath or @newdirfd:@newpath is not a directory, "
                          /*              */ "or @olddirfd:@oldpath is a directory, but @newdirfd:@newpath isn't}"
                          "#tCrossDevice{The given @olddirfd:@oldpath and @newdirfd:@newpath are not located on the same device}"
                          "#tValueError{Attempted to move a directory into itself}"
                          "#tFileAccessError{The current user does not have permissions to access the file "
                          /*                  */ "or directory @olddirfd:@oldpath, or the directory containing @newdirfd:@newpath}"
                          "#tReadOnlyFile{The filesystem or device hosting the given file or directory @olddirfd:@oldpath or the "
                          /*               */ "directory containing the non-existent file @newdirfd:@newpath is in read-only operations "
                          /*               */ "mode, preventing the modification of existing files or directories}"
                          "#tFileExists{@flags include ?GRENAME_NOREPLACE, but @newdirfd:@newpath already exists}"
                          "#tFileClosed{The given @olddirfd or @newdirfd has already been closed}"
                          "#tValueError{Invalid @flags or @atflags}"
                          "#tSystemError{Failed to rename the given @olddirfd:@oldpath for some reason}"
                          "Renames or moves a given @olddirfd:@oldpath to be referred to as @newdirfd:@newpath from then on\n"
                          "When @newpath already exists, it is replaced by @olddirfd:@oldpath"))

/* link(2) */
D(POSIX_LINK_DEF_DOC("#t{:Interrupt}"
                     "#tFileNotFound{The given @oldpath could not be found, or a parent directory of @newpath does not exist}"
                     "#tFileExists{The given @newpath already exists}"
                     "#tNoDirectory{A part of the given @oldpath or @newpath is not a directory}"
                     "#tUnsupportedAPI{The underlying filesystem hosting @oldpath and @newpathdoes not support hard links}"
                     "#tCrossDevice{The given @oldpath and @newpath are not apart of the same drive}"
                     "#tFileAccessError{The current user does not have permissions to access the file or directory "
                     /*                  */ "@oldpath for reading, or the directory containing the non-existent "
                     /*                  */ "object @newpath for writing}"
                     "#tReadOnlyFile{The filesystem or device hosting the directory containing the non-existent "
                     /*               */ "object @newpath is in read-only operations mode, preventing the addition "
                     /*               */ "of file or directory links}"
                     "#tSystemError{Failed to create the link to @oldpath for some reason}"
                     "Create a new hard link pointing to @oldpath as a file named @newpath\n"
                     "Hard links are similar to symbolic links, yet cannot be used across multiple devices and are "
                     /**/ "unaffected by mount locations. A hard link simply create a new directory entry under "
                     /**/ "@newpath that points to the data block of an existing file @oldpath"))
D(POSIX_FLINK_DEF_DOC("#t{:Interrupt}"
                      "#tFileNotFound{A parent directory of @newpath does not exist}"
                      "#tFileExists{The given @newpath already exists}"
                      "#tNoDirectory{A part of the given @newpath is not a directory}"
                      "#tUnsupportedAPI{The underlying filesystem hosting @oldfd and @newpathdoes not support hard links}"
                      "#tCrossDevice{The given @oldfd and @newpath are not apart of the same drive}"
                      "#tFileAccessError{The current user does not have permissions to access the file or directory "
                      /*                  */ "@oldfd for reading, or the directory containing the non-existent "
                      /*                  */ "object @newpath for writing}"
                      "#tReadOnlyFile{The filesystem or device hosting the directory containing the non-existent "
                      /*               */ "object @newpath is in read-only operations mode, preventing the addition "
                      /*               */ "of file or directory links}"
                      "#tSystemError{Failed to create the link to @oldfd for some reason}"
                      "Create a new hard link pointing to @oldfd as a file named @newpath\n"
                      "Hard links are similar to symbolic links, yet cannot be used across multiple devices and are "
                      /**/ "unaffected by mount locations. A hard link simply create a new directory entry under "
                      /**/ "@newpath that points to the data block of an existing file @oldfd"))
D(POSIX_LINKAT_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @olddirfd:@oldpath could not be found, or a parent directory of @newdirfd:@newpath does not exist}"
                       "#tFileExists{The given @newdirfd:@newpath already exists}"
                       "#tNoDirectory{A part of the given @olddirfd:@oldpath or @newdirfd:@newpath is not a directory}"
                       "#tUnsupportedAPI{The underlying filesystem hosting @olddirfd:@oldpath and @newdirfd:@newpathdoes not support hard links}"
                       "#tCrossDevice{The given @olddirfd:@oldpath and @newdirfd:@newpath are not apart of the same drive}"
                       "#tFileAccessError{The current user does not have permissions to access the file or directory "
                       /*                  */ "@olddirfd:@oldpath for reading, or the directory containing the non-existent "
                       /*                  */ "object @newdirfd:@newpath for writing}"
                       "#tReadOnlyFile{The filesystem or device hosting the directory containing the non-existent "
                       /*               */ "object @newdirfd:@newpath is in read-only operations mode, preventing the addition "
                       /*               */ "of file or directory links}"
                       "#tSystemError{Failed to create the link to @olddirfd:@oldpath for some reason}"
                       "Create a new hard link pointing to @olddirfd:@oldpath as a file named @newdirfd:@newpath\n"
                       "Hard links are similar to symbolic links, yet cannot be used across multiple devices and are "
                       /**/ "unaffected by mount locations. A hard link simply create a new directory entry under "
                       /**/ "@newdirfd:@newpath that points to the data block of an existing file @olddirfd:@oldpath"))

/* chmod(2) */
D(POSIX_CHMOD_DEF_DOC("#t{:Interrupt}"
                      "#tFileNotFound{The given @path could not be found}"
                      "#tNoDirectory{A part of the given @path is not a directory}"
                      "#tFileAccessError{The current user does not have permissions "
                      /*                  */ "to change the mode of the given file @path}"
                      "#tReadOnlyFile{The filesystem or device hosting the file found under "
                      /*               */ "@path is in read-only operations mode, preventing the "
                      /*               */ "file's mode from being changed}"
                      "#tSystemError{Failed to change permission for some reason}"
                      "#tValueError{The given @mode is malformed or not recognized}"
                      "Change the permissions associated with a given @path"))
D(POSIX_LCHMOD_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @path could not be found}"
                       "#tNoDirectory{A part of the given @path is not a directory}"
                       "#tFileAccessError{The current user does not have permissions "
                       /*                  */ "to change the mode of the given file @path}"
                       "#tReadOnlyFile{The filesystem or device hosting the file found under "
                       /*               */ "@path is in read-only operations mode, preventing the "
                       /*               */ "file's mode from being changed}"
                       "#tSystemError{Failed to change permission for some reason}"
                       "#tValueError{The given @mode is malformed or not recognized}"
                       "Change the permissions associated with a given @path\n"
                       "If @path refers to a symbolic link, change the permissions "
                       /**/ "of that link, rather than those of the pointed-to file"))
D(POSIX_FCHMOD_DEF_DOC("#tFileClosed{The given file @fd was closed}"
                       "Same as ?Gchmod and ?Glchmod, but change permissions of @fd"))
D(POSIX_FCHMODAT_DEF_DOC("#patflags{Set of $0, ?GAT_SYMLINK_NOFOLLOW, ?GAT_EMPTY_PATH}"
                         "Combination of ?Gchmod, ?Glchmod, and ?Gfchmod, changing permissions of @dfd:@path"))

/* chown(2) */
D(POSIX_CHOWN_DEF_DOC("#t{:Interrupt}"
                      "#tFileNotFound{The given @path could not be found}"
                      "#tNoDirectory{A part of the given @path is not a directory}"
                      "#tFileAccessError{The current user does not have permissions "
                      /*                  */ "to change the ownership of the given file @path}"
                      "#tReadOnlyFile{The filesystem or device hosting the file found under "
                      /*               */ "@path is in read-only operations mode, preventing the "
                      /*               */ "file's ownership from being changed}"
                      "#tValueError{The given @user or @group could not be found}"
                      "#tSystemError{Failed to change ownership for some reason}"
                      "#puid{The new user-id for the file, or ?N to leave unchanged}"
                      "#pgid{The new group-id for the file, or ?N to leave unchanged}"
                      "Change the ownership of a given @path"))
D(POSIX_LCHOWN_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @path could not be found}"
                       "#tNoDirectory{A part of the given @path is not a directory}"
                       "#tFileAccessError{The current user does not have permissions "
                       /*                  */ "to change the ownership of the given file @path}"
                       "#tReadOnlyFile{The filesystem or device hosting the file found under "
                       /*               */ "@path is in read-only operations mode, preventing the "
                       /*               */ "file's ownership from being changed}"
                       "#tValueError{The given @user or @group could not be found}"
                       "#tSystemError{Failed to change ownership for some reason}"
                       "#puid{The new user-id for the file, or ?N to leave unchanged}"
                       "#pgid{The new group-id for the file, or ?N to leave unchanged}"
                       "Change the ownership of a given @path\n"
                       "If @path refers to a symbolic link, change the ownership "
                       /**/ "of that link, rather than those of the pointed-to file"))
D(POSIX_FCHOWN_DEF_DOC("#tFileClosed{The given file @fd was closed}"
                       "Same as ?Glchown, but change ownership of @fd"))
D(POSIX_FCHOWNAT_DEF_DOC("#patflags{Set of $0, ?GAT_SYMLINK_NOFOLLOW, ?GAT_EMPTY_PATH}"
                         "Combination of ?Glchown, ?Glchown, and ?Gfchown, changing ownership of @dfd:@path"))

/* utime(2) */
D(POSIX_UTIME_DEF_DOC("#t{:Interrupt}"
                      "#tFileNotFound{The given @path could not be found}"
                      "#tNoDirectory{A part of the given @path is not a directory}"
                      "#tFileAccessError{The current user does not have permissions "
                      /*                  */ "to change the mode of the given file @path}"
                      "#tReadOnlyFile{The filesystem or device hosting the file found under "
                      /*               */ "@path is in read-only operations mode, preventing the "
                      /*               */ "file's mode from being changed}"
                      "#tSystemError{Failed to change timestamps for some reason}"
                      "#tValueError{The given @mode is malformed or not recognized}"
                      "#patime{The new last-accessed timestamp to set, or ?N to leave unchanged (s.a. ?Ast_atime?Gstat)}"
                      "#pmtime{The new last-modified timestamp to set, or ?N to leave unchanged (s.a. ?Ast_mtime?Gstat)}"
                      "#pctime{The new last-attribute-changed timestamp to set, or ?N to leave unchanged (s.a. ?Ast_ctime?Gstat)}"
                      "#pbirthtime{The new birth timestamp to set, or ?N to leave unchanged (s.a. ?Ast_btime?Gstat)}"
                      "Change the timestamps associated with a given @path"))
D(POSIX_LUTIME_DEF_DOC("#t{:Interrupt}"
                       "#tFileNotFound{The given @path could not be found}"
                       "#tNoDirectory{A part of the given @path is not a directory}"
                       "#tFileAccessError{The current user does not have permissions "
                       /*                  */ "to change the mode of the given file @path}"
                       "#tReadOnlyFile{The filesystem or device hosting the file found under "
                       /*               */ "@path is in read-only operations mode, preventing the "
                       /*               */ "file's mode from being changed}"
                       "#tSystemError{Failed to change timestamps for some reason}"
                       "#tValueError{The given @mode is malformed or not recognized}"
                       "#patime{The new last-accessed timestamp to set, or ?N to leave unchanged (s.a. ?Ast_atime?Gstat)}"
                       "#pmtime{The new last-modified timestamp to set, or ?N to leave unchanged (s.a. ?Ast_mtime?Gstat)}"
                       "#pctime{The new last-attribute-changed timestamp to set, or ?N to leave unchanged (s.a. ?Ast_ctime?Gstat)}"
                       "#pbirthtime{The new birth timestamp to set, or ?N to leave unchanged (s.a. ?Ast_btime?Gstat)}"
                       "Change the timestamps associated with a given @path\n"
                       "If @path refers to a symbolic link, change the timestamps "
                       /**/ "of that link, rather than those of the pointed-to file"))
D(POSIX_FUTIME_DEF_DOC("#tFileClosed{The given file @fd was closed}"
                       "Same as ?Gutime and ?Glutime, but change permissions of @fd"))
D(POSIX_UTIMEAT_DEF_DOC("#patflags{Set of $0, ?GAT_SYMLINK_NOFOLLOW, ?GAT_EMPTY_PATH}"
                        "Combination of ?Gutime, ?Glutime, and ?Gfutime, changing timestamps of @dfd:@path"))

/* copyfile */
D(POSIX_FCOPYFILE_DEF_DOC("#pflags{Set of $0, ?GRENAME_NOREPLACE}"
                          "#pprogress{Invoked every @bufsize bytes with an instance of ?GCopyFileProgress}"
                          "#pbufsize{How many bytes to copy at once (if not given, use a default)}"
                          "Copy all data from @oldfd to @newpath"))
D(POSIX_COPYFILE_DEF_DOC("#pflags{Set of $0, ?GRENAME_NOREPLACE}"
                         "#pprogress{Invoked every @bufsize bytes with an instance of ?GCopyFileProgress}"
                         "#pbufsize{How many bytes to copy at once (if not given, use a default)}"
                         "Copy all data from @oldpath to @newpath"))
D(POSIX_LCOPYFILE_DEF_DOC("#pflags{Set of $0, ?GRENAME_NOREPLACE}"
                          "#pprogress{Invoked every @bufsize bytes with an instance of ?GCopyFileProgress}"
                          "#pbufsize{How many bytes to copy at once (if not given, use a default)}"
                          "Copy all data from @oldpath to @newpath. When @oldpath is a symlink, rather than "
                          /**/ "copy the file pointed-to by the symlink, the symlink #Iitself is copied"))
D(POSIX_COPYFILEAT_DEF_DOC("#pflags{Set of $0, ?GRENAME_NOREPLACE}"
                           "#patflags{Set of $0, ?GAT_SYMLINK_NOFOLLOW, ?GAT_EMPTY_PATH}"
                           "#pprogress{Invoked every @bufsize bytes with an instance of ?GCopyFileProgress}"
                           "#pbufsize{How many bytes to copy at once (if not given, use a default)}"
                           "Copy all data from @olddirfd:@oldpath to @newdirfd:@newpath"))
D(DEX_MEMBER_F_NODOC("CopyFileProgress", &DeeCopyFileProgress_Type, DEXSYM_READONLY),)


/************************************************************************/
/* Application exit control                                             */
/************************************************************************/
D(POSIX_ATEXIT_DEF_DOC("Register a callback to-be invoked before ?Gexit (Same as ?Aatexit?AAppExit?DError)"))
D(POSIX_EXIT_DEF_DOC("Terminate execution of deemon after invoking ?Gatexit callbacks\n"
                     "Termination is done using the C $exit or $_Exit functions, if available. However if these "
                     /**/ "functions are not provided by the host, an ?AAppExit?DError error is thrown instead\n"
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

D(DEX_GETSET("errno", &posix_errno_get, &posix_errno_del, &posix_errno_set,
             "->?Dint\n"
             "Read/write the C errno thread-local variable"),)
D(POSIX_STRERROR_DEF_DOC("Return the name of a given @errnum (which defaults to ?Gerrno), "
                         /**/ "or return ?N if the error doesn't have an associated name"))
D(POSIX_STRERRORNAME_DEF_DOC("Similar to ?Gstrerror, but instead of returning the message "
                             /**/ "associated with a given @errnum (which defaults to ?Gerrno), "
                             /**/ "return the name (e.g. $\"ENOENT\") of the error as a string\n"
                             "If the given error number is not recognized, return ?N instead"))

/************************************************************************/
/* Path utilities                                                       */
/************************************************************************/
D(POSIX_HEADOF_DEF_DOC("#r{The head of a path, that is the directory without the filename}"
                       "${"
                       /**/ "import headof from posix;\n"
                       /**/ "print headof(\"bar.txt\");        /* \"\" */\n"
                       /**/ "print headof(\"/foo/bar.txt\");   /* \"/foo/\" */\n"
                       /**/ "print headof(\"C:/foo/bar.txt\"); /* \"C:/foo/\" */"
                       "}"))
D(POSIX_TAILOF_DEF_DOC("#r{The tail of a path, that is the filename + extension}"
                       "${"
                       /**/ "import tailof from posix;\n"
                       /**/ "print tailof(\"bar.txt\");        /* \"bar.txt\" */\n"
                       /**/ "print tailof(\"/foo/bar.txt\");   /* \"bar.txt\" */\n"
                       /**/ "print tailof(\"C:/foo/bar.txt\"); /* \"bar.txt\" */"
                       "}"))
D(POSIX_DRIVEOF_DEF_DOC("#r{The drive portion of an absolute path on windows, or $\"/\" on other platforms}"
                        "${"
                        /**/ "import driveof from posix;\n"
                        /**/ "print driveof(\"bar.txt\");        /* \"\" or \"/\" */\n"
                        /**/ "print driveof(\"/foo/bar.txt\");   /* \"\" or \"/\" */\n"
                        /**/ "print driveof(\"C:/foo/bar.txt\"); /* \"C:/\" or \"/\" */"
                        "}"))
D(POSIX_INCTRAIL_DEF_DOC("#r{The path with a trailing slash included}"
                         "${"
                         /**/ "import inctrail from posix;\n"
                         /**/ "print inctrail(\"/\");         /* \"/\" */\n"
                         /**/ "print inctrail(\"/foo/bar/\"); /* \"/foo/bar/\" */\n"
                         /**/ "print inctrail(\"/foo/bar\");  /* \"/foo/bar/\" */"
                         "}"))
D(POSIX_EXCTRAIL_DEF_DOC("#r{The path with a trailing slash excluded}"
                         "${"
                         /**/ "import exctrail from posix;\n"
                         /**/ "print exctrail(\"/foo/bar/\"); /* \"/foo/bar\" */\n"
                         /**/ "print exctrail(\"/foo/bar\");  /* \"/foo/bar\" */"
                         "}"))
D(POSIX_WALKLINK_DEF_DOC("Helper function that operates similar to ?Gabspath, but is specifically designed to "
                         /**/ "be faster and exclusively meant for expanding the contents of symbolic links.\n"
                         "This function should be used like:\n"
                         "${"
                         /**/ "import walklink from posix;\n"
                         /**/ "local linkName = \"dir/my-link\";\n"
                         /**/ "symlink(\"../foo/bar\", linkName);\n"
                         /**/ "print repr walklink(readlink(linkName), linkName); /* \"dir/../foo/bar\" */"
                         "}\n"
                         "This function is implemented as follows:\n"
                         "${"
                         /**/ "function walklink(linktext: string, linkname: string): string {\n"
                         /**/ "	if (posix.isabs(linktext))\n"
                         /**/ "		return linktext;\n"
                         /**/ "	#ifdef __WINDOWS__\n"
                         /**/ "	if (linktext in [\"CON\", \"NUL\", ...])\n"
                         /**/ "		return linktext; /* Special case for DOS device filenames. */\n"
                         /**/ "	#endif /* __WINDOWS__ */\n"
                         /**/ "	local linkdir = posix.headof(linkname);\n"
                         /**/ "	if (!linkdir)\n"
                         /**/ "		return linktext;\n"
                         /**/ "	while (#linktext >= 2 && linktext[0] == \".\" && posix.issep(linktext[1]))\n"
                         /**/ "		linktext = linktext[2:];\n"
                         /**/ "	return linkdir + linktext;\n"
                         /**/ "}"
                         "}"))
D(POSIX_ABSPATH_DEF_DOC("Makes @path an absolute path, using @pwd as the base point for the relative disposition\n"
                        "If @path was already relative to begin with, it is forced to become relative "
                        /**/ "as the result of calling ?Grelpath with it and the return value of ?Ggetcwd\n"
                        "If @pwd is relative, if will be forced to become absolute as the result of calling "
                        /**/ "?Gabspath with @pwd as first and the return value of ?Ggetcwd as second argument\n"
                        "${"
                        /**/ "import abspath from posix;\n"
                        /**/ "print abspath(\"../you/Downloads\", \"/home/me\"); /* \"/home/you/Downloads\" */"
                        "}"))
D(POSIX_RELPATH_DEF_DOC("Creates a relative path leading to @path and originating from @pwd\n"
                        "If @path was already relative to begin with, it is forced to become absolute "
                        /**/ "as the result of calling ?Gabspath with it and the return value of ?Ggetcwd\n"
                        "If @pwd is relative, if will be forced into an absolute path as the "
                        /**/ "result of calling ?Gabspath with it and the return value of ?Ggetcwd\n"
                        "When running on a windows host, in the event that @path is located on a "
                        /**/ "different ?Gdriveof than @pwd, @path will be re-returned as is"))
D(POSIX_NORMALPATH_DEF_DOC("Normalize the given @path. This removes spaces around slashes, replaces os-specific "
                           "?GFS_ALTSEP with ?GFS_SEP, as well as $\".\" and $\"..\" path segments (if possible)"))
D(POSIX_ISABS_DEF_DOC("Returns ?t if the given @path is considered to be absolute"))
D(POSIX_ISREL_DEF_DOC("Returns the inverse of ?Gisabs"))
D(POSIX_ISSEP_DEF_DOC("Returns ?t if the given @str is recognized as a path "
                      /**/ "separator (Usually $\"/\" and/or $\"\\\")\n"
                      "The host's primary and secondary separator "
                      /**/ "values can be read from ?GSEP and ?GALTSEP"))
D(POSIX_JOINPATH_DEF_DOC("Joins all @paths passed through varargs to generate a full path. "
                         /**/ "For this purpose, all path elements are joined with ?GSEP, "
                         /**/ "after removal of additional slashes and spaces surrounding the given @paths"))
D(DEX_MEMBER_F("FS_SEP", &posix_FS_SEP, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "The host's primary path separator. On windows that is "
               /**/ "$\"\\\" while on most other hosts it is $\"/\"\n"
               "If supported by the host, an alternative separator can be read from ?GALTSEP\n"
               "Additionally, a string can be testing for being a separator by calling ?Gissep"),)
D(DEX_MEMBER_F("FS_ALTSEP", &posix_FS_ALTSEP, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "The alternative path separator or an alias for ?GSEP "
               /**/ "if the host only supports a single type of separator"),)
D(DEX_MEMBER_F("FS_DELIM", &posix_FS_DELIM, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "A string used to delimit individual paths in path-listings often "
               /**/ "found in environment variables, most notably ${environ[\"PATH\"]}"),)
D(DEX_MEMBER_F("DEV_NULL", &posix_DEV_NULL, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "A special filename accepted by ?Gopen and ?Aopen?DFile to return a data sink"),)
D(DEX_MEMBER_F("DEV_TTY", &posix_DEV_TTY, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "A special filename accepted by ?Gopen and ?Aopen?DFile to return a "
               /**/ "handle to the calling process's controlling terminal"),)
D(DEX_MEMBER_F("DEV_STDIN", &posix_DEV_STDIN, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "A special filename accepted by ?Gopen and ?Aopen?DFile to return a handle to ?Astdin?DFile"),)
D(DEX_MEMBER_F("DEV_STDOUT", &posix_DEV_STDOUT, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "A special filename accepted by ?Gopen and ?Aopen?DFile to return a handle to ?Astdout?DFile"),)
D(DEX_MEMBER_F("DEV_STDERR", &posix_DEV_STDERR, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
               "->?Dstring\n"
               "A special filename accepted by ?Gopen and ?Aopen?DFile to return a handle to ?Astderr?DFile"),)

/* Allow user-code to dynamically determine if the host has a case-insensitive file-system.
	 * We mark this global variable a a CONSTEXPR (in user-code: `final'), so that the compiler
	 * is allowed to substitute it with a constant expression at compile-time. */
#ifdef DeeSystem_HAVE_FS_ICASE
#define OBJ_posix_FS_ICASE Dee_True
#else /* DeeSystem_HAVE_FS_ICASE */
#define OBJ_posix_FS_ICASE Dee_False
#endif /* !DeeSystem_HAVE_FS_ICASE */
DEX_MEMBER_F("FS_ICASE", OBJ_posix_FS_ICASE, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
             "Evaluations to true if the host has a case-insensitive file-system"),

/* Allow user-code to dynamically determine if the host has drives. */
#ifdef DeeSystem_HAVE_FS_DRIVES
#define OBJ_posix_FS_DRIVES Dee_True
#else /* DeeSystem_HAVE_FS_DRIVES */
#define OBJ_posix_FS_DRIVES Dee_False
#endif /* !DeeSystem_HAVE_FS_DRIVES */
DEX_MEMBER_F("FS_DRIVES", OBJ_posix_FS_DRIVES, DEXSYM_READONLY | DEXSYM_CONSTEXPR,
             "Evaluations to true if the host has DOS-like, drive-based file-system"),

#ifdef HAVE_posix_dex_fini
#define PTR_posix_dex_fini &posix_dex_fini
#else /* HAVE_posix_dex_fini */
#define PTR_posix_dex_fini NULL
#endif /* !HAVE_posix_dex_fini */

/* clang-format off */
DEX_END(
	/* init:  */ NULL,
	/* fini:  */ PTR_posix_dex_fini,
	/* clear: */ NULL
);
/* clang-format on */

DECL_END

#endif /* !GUARD_DEX_POSIX_LIBPOSIX_C */
