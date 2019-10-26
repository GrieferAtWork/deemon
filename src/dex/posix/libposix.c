/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_LIBPOSIX_C
#define GUARD_DEX_POSIX_LIBPOSIX_C 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

#ifndef __INTELLISENSE__
#include "p-access.c.inl"
#include "p-errno.c.inl"
#include "p-exit.c.inl"
#include "p-fd.c.inl"
#include "p-open.c.inl"
#include "p-pipe.c.inl"
#include "p-readwrite.c.inl"
#include "p-sched.c.inl"
#include "p-sync.c.inl"
#include "p-truncate.c.inl"

/* Include fs.c.inl last, since this once #undef's a
 * bunch of stuff that may be needed by other components. */
#include "p-fs.c.inl"
#endif /* !__INTELLISENSE__ */


DECL_BEGIN



#ifdef NEED_ERR_UNSUPPORTED
#undef NEED_ERR_UNSUPPORTED
PRIVATE ATTR_NOINLINE ATTR_UNUSED ATTR_COLD int DCALL
posix_err_unsupported(char const *__restrict name) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unsupported function `%s'",
	                       name);
}
#endif /* NEED_ERR_UNSUPPORTED */

#ifdef NEED_GET_DFD_FILENAME
#undef NEED_GET_DFD_FILENAME
PRIVATE WUNUSED DREF /*String*/ DeeObject *DCALL
libposix_get_dfd_filename(int dfd, /*utf-8*/ char const *filename, int atflags) {
	(void)dfd;
	(void)filename;
	(void)atflags;
	/* TODO: `joinpath(frealpath(dfd), filename)' */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}
#endif /* NEED_GET_DFD_FILENAME */



PRIVATE char const *missing_features[] = {
#ifdef p_truncate_USE_STUB
	"truncate",
#endif /* p_truncate_USE_STUB */
#ifdef p_ftruncate_USE_STUB
	"ftruncate",
#endif /* p_ftruncate_USE_STUB */
#ifdef p_pipe_USE_STUB
	"pipe",
#endif /* p_pipe_USE_STUB */
#ifdef p_pipe2_USE_STUB
	"pipe2",
#endif /* p_pipe2_USE_STUB */
#ifdef p_strerror_USE_STUB
	"strerror",
#endif /*p_strerror_USE_STUB */
#ifdef p_strerrorname_USE_STUB
	"strerrorname",
#endif /*p_strerrorname_USE_STUB */
#ifdef posix_open_USE_STUB
	"open",
#endif /* posix_open_USE_STUB */
#ifdef posix_creat_USE_STUB
	"creat",
#endif /* posix_creat_USE_STUB */
#ifdef posix_close_USE_STUB
	"close",
#endif /* posix_close_USE_STUB */
#ifdef posix_dup_USE_STUB
	"dup",
#endif /* posix_dup_USE_STUB */
#ifdef posix_dup2_USE_STUB
	"dup2",
#endif /* posix_dup2_USE_STUB */
#ifdef posix_dup3_USE_STUB
	"dup3",
#endif /* posix_dup3_USE_STUB */
#ifndef CONFIG_HAVE_errno
	"errno",
#endif /* CONFIG_HAVE_errno */
	NULL
};


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


PRIVATE struct dex_symbol symbols[] = {

	/* File control */
	D(POSIX_OPEN_DEF)
	D(POSIX_CREAT_DEF)
	D(POSIX_READ_DEF_DOC("Read up to @count bytes into @buf\n"
	                     "When @buf is given, return the actual number of read bytes. "
	                     "Otherwise read into a new :Bytes object that is then returned"))
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

	/* Environ control */
	D(POSIX_GETENV_DEF)
	D(POSIX_SETENV_DEF)
	D(POSIX_PUTENV_DEF)
	D(POSIX_UNSETENV_DEF)
	D(POSIX_CLEARENV_DEF)

	/* Python-like helper functions */
	/* TODO: cpu_count() */
	/* TODO: get_inheritable() */
	/* TODO: set_inheritable() */

	/* Higher-level wrapper functions */
	/* TODO: popen() */
	/* TODO: fdopen() (Basically just a wrapper around `DeeFile_OpenFd') */

	/* Forward-aliases to `libfs' */
#define DEFINE_LIBFS_ALIAS_ALT(altname, name, libfs_name, proto)                           \
	D({ altname, (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY | MODSYM_FREADONLY, \
	    DOC(proto "Alias for :fs." libfs_name) }, )
#define DEFINE_LIBFS_ALIAS_S_ALT(altname, name, proto)                            \
	D({ altname,                                                                  \
	    (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY | MODSYM_FREADONLY, \
	    DOC(proto "Alias for :fs." #name) }, )
#define DEFINE_LIBFS_ALIAS(name, libfs_name, proto) \
	DEFINE_LIBFS_ALIAS_ALT(#name, name, libfs_name, proto)
#define DEFINE_LIBFS_ALIAS_S(name, proto) \
	DEFINE_LIBFS_ALIAS_S_ALT(DeeString_STR(&libposix_libfs_name_##name), name, proto)
	DEFINE_LIBFS_ALIAS(opendir, "dir", "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(environ, "->?S?T2?Dstring?Dstring\n")
	DEFINE_LIBFS_ALIAS_S(stat, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(lstat, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(getcwd, "->?Dstring\n")
	DEFINE_LIBFS_ALIAS_S(gethostname, "->?Dstring\n")
	DEFINE_LIBFS_ALIAS_S(chdir, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(chmod, "(path:?Dstring,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(lchmod, "(path:?Dstring,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(chown, "(path:?Dstring,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(lchown, "(path:?Dstring,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(mkdir, "(path:?Dstring,permissions:?X2?Dstring?Dint=!N)\n")
	DEFINE_LIBFS_ALIAS_S(rmdir, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(unlink, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(remove, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(rename, "(existing_path:?Dstring,new_path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(link, "(existing_path:?X3?Dstring?DFile?Dint,new_path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(symlink, "(target_text:?Dstring,link_path:?Dstring,format_target=!t)\n")
	DEFINE_LIBFS_ALIAS_S(readlink, "(path:?Dstring)->?Dstring\n(fp:?DFile)->?Dstring\n(fd:?Dint)->?Dstring\n")
	DEFINE_LIBFS_ALIAS_ALT("fopendir", opendir, "dir", "(fp:?DFile)\n(fd:?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fstat", stat, "(fp:?DFile)\n(fd:?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchdir", chdir, "(fp:?DFile)\n(fd:?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchmod", chmod, "(fp:?DFile,mode:?X2?Dstring?Dint)\n"
	                                          "(fd:?Dint,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchown", chown, "(fp:?DFile,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n"
	                                          "(fd:?Dint,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n")

	/* stat.st_mode bits. */
	DEFINE_LIBFS_ALIAS_S(S_IFMT, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFDIR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFCHR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFBLK, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFREG, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFIFO, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFLNK, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFSOCK, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_ISUID, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_ISGID, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_ISVTX, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IRUSR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IWUSR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IXUSR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IRGRP, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IWGRP, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IXGRP, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IROTH, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IWOTH, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IXOTH, "->?Dint\n")

	/* stat.st_mode helper functions. */
	DEFINE_LIBFS_ALIAS_S(S_ISDIR, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISCHR, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISBLK, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISREG, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISFIFO, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISLNK, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISSOCK, "(mode:?Dint)->?Dbool\n")

#undef DEFINE_LIBFS_ALIAS_S
#undef DEFINE_LIBFS_ALIAS
#undef DEFINE_LIBFS_ALIAS_S_ALT
#undef DEFINE_LIBFS_ALIAS_ALT

	/* Application exit control */
	D(POSIX_ATEXIT_DEF_DOC("Register a callback to-be invoked before #exit (Same as :deemon:Error.AppExit.atexit)"))
	D(POSIX_EXIT_DEF_DOC("Terminate execution of deemon after invoking #atexit callbacks\n"
	                     "Termination is done using the C $exit or $_Exit functions, if available. However if these "
	                     "functions are not provided by the host, an :AppExit error is thrown instead\n"
	                     "When no @exitcode is given, the host's default default value of #EXIT_FAILURE, or $1 is used\n"
	                     "This function never returns normally"))
	D(POSIX__EXIT_DEF_DOC("Terminate execution of deemon without invoking #atexit callbacks (s.a. #exit)"))
	D(POSIX_ABORT_DEF_DOC("Same as #_Exit when passing #EXIT_FAILURE"))

	/* *_FILENO values */
	D(POSIX_FD_DEFS)

	/* O_* and AT_* values */
	D(POSIX_OPEN_DEFS)

	/* SEEK_* values */
	D(POSIX_READWRITE_DEFS)

	/* EXIT_* values */
	D(POSIX_EXIT_DEFS)

	/* E* errno codes */
	D(POSIX_ERRNO_DEFS)

	/* *_OK codes for `access()' and friends */
	D(POSIX_ACCESS_DEFS)

	D({ "errno", (DeeObject *)&posix_errno_get, MODSYM_FPROPERTY,
	    DOC("->?Dint\n"
	        "Read/write the C errno thread-local variable") }, )
	D({ NULL, (DeeObject *)&posix_errno_del, MODSYM_FNORMAL }, )
	D({ NULL, (DeeObject *)&posix_errno_set, MODSYM_FNORMAL }, )
	D(POSIX_STRERROR_DEF_DOC("Return the name of a given @errnum (which defaults to #errno), "
	                         "or return :none if the error doesn't have an associated name"))
	D(POSIX_STRERRORNAME_DEF_DOC("Similar to #strerror, but instead of returning the message "
	                             "associated with a given @errnum (which defaults to #errno), "
	                             "return the name (e.g. $\"ENOENT\") of the error as a string\n"
	                             "If the given error number is not recognized, return :none instead"))

	{ NULL }
};
#undef D

PUBLIC struct dex DEX = {
	/* .d_symbols      = */ symbols,
	/* .d_init         = */ NULL,
	/* .d_fini         = */ NULL,
	/* .d_import_names = */ { import_table }
};

DECL_END

#endif /* !GUARD_DEX_WIN32_LIBWIN32_C */
