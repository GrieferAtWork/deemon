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
#ifndef GUARD_DEEMON_SYSTEM_FEATURES_H
#define GUARD_DEEMON_SYSTEM_FEATURES_H 1

#include "api.h"

#include <hybrid/typecore.h>

#include <stddef.h>

/*[[[deemon
function addparen(x) {
	for (;;) {
		local nx = x.strip();
		nx = nx.rsstrip("&& 1");
		nx = nx.rsstrip("&&1");
		nx = nx.lsstrip("1 &&");
		nx = nx.lsstrip("1&&");
		if (nx == x)
			break;
		x = nx;
	}
	if (("&&" in x) || ("||" in x)) {
		if (!x.startswith("(") || !x.endswith(")") ||
		    x.findmatch("(", ")", 1) != #x - 1)
			x = "({})".format({ x });
	}
	return x;
}
function feature(name, default_requirements) {
	print "#ifdef CONFIG_NO_",;
	print name;
	print "#undef CONFIG_HAVE_",;
	print name;
	if (default_requirements == "1") {
		print "#else";
	} else if (default_requirements in ["", "0"]) {
		print "#elif 0";
	} else {
		print "#elif !defined(CONFIG_HAVE_",;
		print name,;
		print ") && \\";
		print "      (",;
		default_requirements = default_requirements.strip();
		while (#default_requirements > 80) {
			local p = default_requirements.find(" ", 70);
			if (p < 0)
				break;
			while (default_requirements.isspace(p))
				++p;
			if (default_requirements.substr(p, p + 2) in ["||", "&&"])
				p += 2;
			print default_requirements[:p].rstrip(),;
			print " \\";
			print "       ",;
			default_requirements = default_requirements[p:].lstrip(),;
		}
		print default_requirements,;
		print ")";
	}
	print "#define CONFIG_HAVE_",;
	print name,;
	print " 1";
	print "#endif";
	print;
}

local known_headers = [];
function header_featnam(name) {
	return name
		.replace(".", "_").replace("-", "_")
		.replace("/", "_").upper();
}
function header_nostdinc(name, default_requirements = "") {
	default_requirements = addparen(default_requirements);
	if (default_requirements in ["", "0"]) {
		default_requirements = "__has_include(<{}>)".format({ name });
	} else {
		if (default_requirements != "1")
			default_requirements = "__has_include(<{}>) || (defined(__NO_has_include) && {})".format({ name, default_requirements });
	}
	local featnam = header_featnam(name);
	feature(featnam, default_requirements);
}
function header(name, default_requirements = "") {
	known_headers.append(name);
	header_nostdinc(name, default_requirements);
}

#define var      func
#define typ      func
#define constant func
function func(name, default_requirements = "", check_defined = true) {
	if (default_requirements != "1" && check_defined) {
		default_requirements = addparen(default_requirements);
		if (default_requirements !in ["", "0"])
			default_requirements = " || " + default_requirements;
		default_requirements = "defined({}) || defined(__{}_defined){}"
			.format({ name, name, default_requirements });
	}
	feature(name, default_requirements);
}
function isenabled(name) {
	return "(defined({}) && {}+0 != 0)".format({ name, name });
}
function include_known_headers() {
	for (local name: known_headers) {
		local featnam = header_featnam(name);
		print "#ifdef CONFIG_HAVE_",;
		print featnam;
		print "#include <",;
		print name,;
		print ">";
		print "#endif /" "* CONFIG_HAVE_",;
		print featnam, "*" "/";
		print;
	}
}


local linux = "defined(__linux__) || defined(__linux) || defined(linux)";
local unix = linux + " || defined(__unix__) || defined(__unix) || defined(unix)";
local kos = "defined(__KOS__)";
local msvc = "defined(_MSC_VER)";
local stdc = "1";

header("io.h", addparen(msvc) + " || " + addparen(kos));
header("direct.h", addparen(msvc) + " || " + addparen(kos));
header("process.h", addparen(msvc) + " || " + addparen(kos));
header("sys/stat.h", addparen(msvc) + " || " + addparen(unix));
header("fcntl.h", addparen(msvc) + " || " + addparen(unix));
header("sys/fcntl.h", addparen(linux) + " || " + addparen(kos));
header("sys/ioctl.h", unix);
header("ioctl.h");
header("unistd.h", unix);
header("errno.h", addparen(msvc) + " || " + addparen(unix));
header("stdio.h", stdc);
header("stdlib.h", stdc);
header("features.h", unix);
header("sched.h", unix);
header("signal.h", unix);
header("sys/signal.h", addparen(linux) + " || " + addparen(kos));
header("sys/syscall.h", addparen(linux) + " || " + addparen(kos));
header("pthread.h", unix);
header("sys/types.h", addparen(linux) + " || " + addparen(kos));
header("sys/select.h", addparen(linux) + " || " + addparen(kos));
header("semaphore.h", addparen(linux) + " || " + addparen(kos));
header("time.h", addparen(msvc) + " || " + addparen(unix));
header("sys/time.h", addparen(linux) + " || " + addparen(kos));
header("sys/mman.h", addparen(linux) + " || " + addparen(kos));
header("sys/wait.h", unix);
header("wait.h", addparen(linux) + " || " + addparen(kos));
header("sys/signalfd.h", addparen(linux) + " || " + addparen(kos));
header("ctype.h", stdc);
header("string.h", stdc);
header("wchar.h", stdc);

header_nostdinc("dlfcn.h", unix);
header_nostdinc("float.h", stdc);
header_nostdinc("limits.h", stdc);
header_nostdinc("link.h", stdc);

include_known_headers();

func("_Exit", "defined(__USE_ISOC99)");
func("_exit", addparen(msvc) + " || " + addparen(unix));
func("exit", stdc);
func("atexit", stdc);

func("execv", unix);
func("execve", unix);
func("execvp", unix);
func("execvpe", unix);
func("fexecve", "defined(__USE_XOPEN2K8)");
func("_execv", msvc);
func("_execve", msvc);
func("_execvp", msvc);
func("_execvpe", msvc);
func("_wexecv", msvc);
func("_wexecve", msvc);
func("_wexecvp", msvc);
func("_wexecvpe", msvc);
func("_spawnv", msvc);
func("_spawnve", msvc);
func("_spawnvp", msvc);
func("_spawnvpe", msvc);
func("_wspawnv", msvc);
func("_wspawnve", msvc);
func("_wspawnvp", msvc);
func("_wspawnvpe", msvc);

func("_cwait", msvc);
func("wait", unix);
func("waitpid", unix);
func("wait4", addparen(linux) + " || " + addparen(kos));
func("waitid", addparen(linux) + " || " + addparen(kos));
func("sigprocmask", unix);
func("detach", kos + " && defined(__USE_KOS) && __KOS_VERSION__ >= 300");

func("system", stdc);
func("_wsystem", msvc);

func("creat", unix);
func("_creat", msvc);
func("_wcreat", "defined(_WIO_DEFINED)");

func("open", unix);
func("_open", msvc);
func("_wopen", msvc);
func("open64", "defined(__USE_LARGEFILE64)");

func("read", unix);
func("_read", msvc);

func("write", unix);
func("_write", msvc);

func("lseek", unix);
func("lseek64", "defined(__USE_LARGEFILE64)");
func("_lseek", msvc);
func("_lseeki64", msvc);

func("chdir", unix);
func("_chdir", msvc);

func("readlink", "defined(CONFIG_HAVE_UNISTD_H) && (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))");
func("freadlinkat", "defined(CONFIG_HAVE_UNISTD_H) && defined(__USE_KOS) && defined(__CRT_HAVE_freadlinkat)");

func("stat", "defined(CONFIG_HAVE_SYS_STAT_H)");
func("fstat", "defined(CONFIG_HAVE_SYS_STAT_H)");
func("lstat", "defined(CONFIG_HAVE_SYS_STAT_H) && (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))");
func("stat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64)");
func("fstat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64)");
func("lstat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64) && (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))");
func("fstatat", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
func("fstatat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64) && defined(__USE_ATFILE)");
feature("STAT_HAVE_ST_NSEC", "defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_NSEC)"); // syscall_ulong_t st_[acm]timensec
feature("STAT_HAVE_ST_TIM", "defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_TIM)"); // struct timespec st_[acm]tim
feature("STAT_HAVE_ST_TIMESPEC", "defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_TIMESPEC)"); // struct timespec st_[acm]timespec

func("mkdir", "defined(CONFIG_HAVE_SYS_STAT_H) && " + addparen(unix));
func("_mkdir", msvc);
func("chmod", "defined(CONFIG_HAVE_SYS_STAT_H) && " + addparen(unix));
func("_chmod", msvc);
func("_wchmod", "defined(_WIO_DEFINED)");
func("mkfifo", "defined(CONFIG_HAVE_SYS_STAT_H) && " + addparen(unix));
func("lchmod", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_MISC)");
func("fchmodat", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
func("mkdirat", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
func("mkfifoat", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
func("fchmod", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_POSIX)");
func("mknod", "defined(CONFIG_HAVE_SYS_STAT_H) && (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED))");
func("mknodat", "defined(CONFIG_HAVE_SYS_STAT_H) && (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)) && defined(__USE_ATFILE)");
func("utimensat", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
func("utimensat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE) && defined(__USE_TIME64)");
func("futimens", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_XOPEN2K8)");
func("futimens64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_XOPEN2K8) && defined(__USE_TIME64)");

func("time", "defined(CONFIG_HAVE_TIME_H)");
func("time64", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_TIME64)");
func("clock_gettime", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309)");
func("clock_gettime64", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309) && defined(__USE_TIME64)");
constant("CLOCK_REALTIME", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309)");
func("gettimeofday", "defined(CONFIG_HAVE_SYS_TIME_H)");
func("gettimeofday64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_TIME64)");
func("utimes", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_MISC)");
func("utimes64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_MISC) && defined(__USE_TIME64)");
func("lutimes", "defined(CONFIG_HAVE_SYS_TIME_H)");
func("lutimes64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_TIME64)");
func("futimesat", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_GNU)");
func("futimesat64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_GNU) && defined(__USE_TIME64)");


print "#if",msvc;
print "#define F_OK     0";
print "#define X_OK     1 /* Not supported? *" "/";
print "#define W_OK     2";
print "#define R_OK     4";
print "#endif /" "*",msvc,"*" "/";
print;

func("access", "(defined(CONFIG_HAVE_UNISTD_H) || !" + addparen(msvc) + ") && defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK)");
func("euidaccess", "defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_GNU)");
func("eaccess", "defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_GNU)");
func("faccessat", "defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_ATFILE)");
func("_access", msvc);
func("_waccess", "defined(_WIO_DEFINED)");

func("fchownat", "defined(__USE_ATFILE)");

func("pread", "defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)");
func("pwrite", "defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)");
func("pread64", "defined(__USE_LARGEFILE64) && (defined(__USE_UNIX98) || defined(__USE_XOPEN2K8))");
func("pwrite64", "defined(__USE_LARGEFILE64) && (defined(__USE_UNIX98) || defined(__USE_XOPEN2K8))");

func("close", unix);
func("_close", msvc);

func("sync", unix);
func("fsync", isenabled("_POSIX_FSYNC") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + ")");
func("fdatasync", unix);
func("_commit", msvc);

func("getpid", unix);
func("_getpid", msvc);

func("umask", "defined(CONFIG_HAVE_SYS_STAT_H)");
func("_umask", msvc);

func("dup", unix);
func("_dup", msvc);

func("dup2", unix);
func("_dup2", msvc);

func("dup3", "defined(__USE_GNU)");

func("isatty", unix);
func("_isatty", msvc);

func("getcwd", unix);
func("_getcwd", msvc);
func("wgetcwd");
func("_wgetcwd", "defined(_WDIRECT_DEFINED)");

func("unlink", unix);
func("_unlink", "defined(_CRT_DIRECTORY_DEFINED)");
func("remove", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("_wunlink", "defined(_WIO_DEFINED)");
func("_wremove", "defined(_WSTDIO_DEFINED)");
func("rename", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("_wrename", "defined(_WIO_DEFINED)");

func("getenv", "defined(CONFIG_HAVE_STDLIB_H) && " + addparen(stdc));

func("wcslen", "defined(CONFIG_HAVE_WCHAR_H) && " + addparen(stdc));

func("qsort", "defined(CONFIG_HAVE_STDLIB_H) && " + addparen(stdc));

func("truncate", addparen(unix) + " || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)");
func("truncate64", "defined(__USE_LARGEFILE64) && (" + addparen(unix) + " || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8))");
func("ftruncate", addparen(unix) + " || defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)");
func("ftruncate64", "defined(__USE_LARGEFILE64) && (" + addparen(unix) + " || defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))");
func("_chsize", msvc);
func("_chsize_s", msvc);

func("getpgid", isenabled("_POSIX_JOB_CONTROL"));
func("setpgid", isenabled("_POSIX_JOB_CONTROL"));
func("setreuid", isenabled("_POSIX_SAVED_IDS") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + ")");
func("nice", "defined(__USE_MISC) || defined(__USE_XOPEN) || (" + isenabled("_POSIX_PRIORITY_SCHEDULING") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + "))");

func("mmap", isenabled("_POSIX_MAPPED_FILES") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + ")");
func("mmap64", "defined(__USE_LARGEFILE64) && (" + isenabled("_POSIX_MAPPED_FILES") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + "))");
func("munmap", "CONFIG_HAVE_mmap");
constant("MAP_ANONYMOUS");
constant("MAP_ANON");
constant("MAP_PRIVATE");
constant("MAP_GROWSUP");
constant("MAP_GROWSDOWN");
constant("MAP_FILE");
constant("MAP_STACK");
constant("MAP_UNINITIALIZED");
constant("PROT_READ");
constant("PROT_WRITE");

func("pipe", "defined(CONFIG_HAVE_UNISTD_H) || " + addparen(unix));
func("pipe2", "defined(__USE_GNU)");
func("_pipe", msvc);

func("usleep", "defined(CONFIG_HAVE_UNISTD_H) && ((defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || defined(__USE_MISC))");
typ("useconds_t", "defined(CONFIG_HAVE_UNISTD_H) && (defined(__USE_XOPEN) || defined(__USE_XOPEN2K))");
func("nanosleep", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309)");
func("nanosleep64", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309) && defined(__USE_TIME64)");

func("fork", unix);
func("vfork", "(defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || defined(__USE_MISC)");
func("fchown", "defined(CONFIG_HAVE_UNISTD_H) || " + addparen(unix));
func("fchdir", "defined(CONFIG_HAVE_UNISTD_H) || " + addparen(unix));

func("pause", unix);
func("select", "defined(CONFIG_HAVE_SYS_SELECT_H) && " + addparen(unix));

func("__iob_func", msvc + " || defined(____iob_func_defined)");

func("fseek", "defined(CONFIG_HAVE_STDIO_H)");
func("ftell", "defined(CONFIG_HAVE_STDIO_H)");
func("fseek64", "defined(CONFIG_HAVE_STDIO_H) && 0");
func("ftell64", "defined(CONFIG_HAVE_STDIO_H) && 0");
func("fseeko", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(unix));
func("ftello", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(unix));
func("fseeko64", "defined(CONFIG_HAVE_STDIO_H) && defined(__USE_LARGEFILE64)");
func("ftello64", "defined(CONFIG_HAVE_STDIO_H) && defined(__USE_LARGEFILE64)");
func("_fseeki64", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(msvc));
func("_ftelli64", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(msvc));
func("fflush", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("ferror", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fclose", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fileno", "defined(CONFIG_HAVE_STDIO_H) && " + "!" + addparen(msvc));
func("_fileno", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(msvc));
func("fftruncate", "defined(__USE_KOS)");
func("fftruncate64", "defined(__USE_KOS) && defined(__USE_LARGEFILE64)");
func("getc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fgetc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("putc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fputc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fread", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fwrite", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("ungetc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("setvbuf", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fopen", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("fopen64", "defined(CONFIG_HAVE_STDIO_H) && defined(__USE_LARGEFILE64)");
func("fprintf", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("stdin", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("stdout", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("stderr", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));

func("sem_init", "defined(CONFIG_HAVE_SEMAPHORE_H)");
func("sem_destroy", "defined(CONFIG_HAVE_SEMAPHORE_H)");
func("sem_wait", "defined(CONFIG_HAVE_SEMAPHORE_H)");
func("sem_trywait", "defined(CONFIG_HAVE_SEMAPHORE_H)");
func("sem_post", "defined(CONFIG_HAVE_SEMAPHORE_H)");
func("sem_timedwait", "defined(CONFIG_HAVE_SEMAPHORE_H) && defined(__USE_XOPEN2K)");
func("sem_timedwait64", "defined(CONFIG_HAVE_SEMAPHORE_H) && defined(__USE_XOPEN2K) && defined(__USE_TIME64)");

func("pthread_suspend", "defined(CONFIG_HAVE_PTHREAD_H) && 0");
func("pthread_continue", "defined(CONFIG_HAVE_PTHREAD_H) && 0");
func("pthread_suspend_np", "defined(CONFIG_HAVE_PTHREAD_H) && 0");
func("pthread_unsuspend_np", "defined(CONFIG_HAVE_PTHREAD_H) && 0");
func("pthread_setname", "defined(CONFIG_HAVE_PTHREAD_H) && 0");
func("pthread_setname_np", "defined(CONFIG_HAVE_PTHREAD_H) && defined(__USE_GNU)");

func("abort", "defined(CONFIG_HAVE_STDLIB_H) && " + addparen(stdc));
func("strerror", "defined(CONFIG_HAVE_STRING_H) && " + addparen(stdc));

func("dlopen", "defined(CONFIG_HAVE_DLFCN_H)");
func("dlclose", "defined(CONFIG_HAVE_DLFCN_H)");
func("dlsym", "defined(CONFIG_HAVE_DLFCN_H)");

func("_memicmp", msvc);
func("memcasecmp", "defined(__USE_KOS)");
func("memrchr", "defined(__USE_GNU)");
func("rawmemchr", "defined(__USE_GNU)");
func("strnlen", "defined(__USE_XOPEN2K8) || defined(__USE_DOS) || (defined(_MSC_VER) && !defined(__KOS_SYSTEM_HEADERS__))");

// NOTE: The GNU-variant of memmem() returns the start of the haystack
//       when `needle_length == 0', however for this case, deemon requires
//       that `NULL' be returned, as deemon considers an empty string not
//       to be contained ~in-between two other characters~
// KOS provides this behavior when given the `_MEMMEM_EMPTY_NEEDLE_SOURCE' option,
// which report back an ACK in the form of `__USE_MEMMEM_EMPTY_NEEDLE_NULL'
func("memmem", "defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL)", check_defined: false);
func("memrmem", "defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL)", check_defined: false);

func("rawmemrchr", "defined(__USE_KOS)");
func("memend", "defined(__USE_KOS)");
func("memrend", "defined(__USE_KOS)");
func("memlen", "defined(__USE_KOS)");
func("memrlen", "defined(__USE_KOS)");
func("rawmemlen", "defined(__USE_KOS)");
func("rawmemrlen", "defined(__USE_KOS)");
func("memcasemem", "defined(__USE_KOS)");
func("memrev", "defined(__USE_KOS)");
func("memcasermem", "", check_defined: false);
func("strend", "defined(__USE_KOS)");
func("strnend", "defined(__USE_KOS)");

func("memxend", "defined(__USE_STRING_XCHR)");
func("memxlen", "defined(__USE_STRING_XCHR)");
func("memxchr", "defined(__USE_STRING_XCHR)");
func("rawmemxchr", "defined(__USE_STRING_XCHR)");
func("rawmemxlen", "defined(__USE_STRING_XCHR)");
func("memxrchr");
func("memxrend");
func("memxrlen");
func("rawmemxrchr");
func("rawmemxrlen");

func("tolower", "defined(CONFIG_HAVE_CTYPE_H)");
func("toupper", "defined(CONFIG_HAVE_CTYPE_H)");

// CRT-specific functions
func("_dosmaperr", msvc + " || (defined(__CRT_DOS) && defined(__CRT_HAVE__dosmaperr))", check_defined: false);
func("errno_nt2kos", "defined(__CRT_HAVE_errno_nt2kos)", check_defined: false);
func("_get_osfhandle", msvc);
func("_open_osfhandle", msvc);
var("_doserrno", msvc);


// NOTE: Other config features used in deemon source files:
//    - CONFIG_NO_RTLD_LOCAL / CONFIG_HAVE_RTLD_LOCAL
//    - CONFIG_NO_RTLD_GLOBAL / CONFIG_HAVE_RTLD_GLOBAL
//    - CONFIG_NO_RTLD_LAZY / CONFIG_HAVE_RTLD_LAZY
//    - CONFIG_NO_RTLD_NOW / CONFIG_HAVE_RTLD_NOW

]]]*/
#ifdef CONFIG_NO_IO_H
#undef CONFIG_HAVE_IO_H
#elif !defined(CONFIG_HAVE_IO_H) && \
      (__has_include(<io.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       defined(__KOS__))))
#define CONFIG_HAVE_IO_H 1
#endif

#ifdef CONFIG_NO_DIRECT_H
#undef CONFIG_HAVE_DIRECT_H
#elif !defined(CONFIG_HAVE_DIRECT_H) && \
      (__has_include(<direct.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       defined(__KOS__))))
#define CONFIG_HAVE_DIRECT_H 1
#endif

#ifdef CONFIG_NO_PROCESS_H
#undef CONFIG_HAVE_PROCESS_H
#elif !defined(CONFIG_HAVE_PROCESS_H) && \
      (__has_include(<process.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       defined(__KOS__))))
#define CONFIG_HAVE_PROCESS_H 1
#endif

#ifdef CONFIG_NO_SYS_STAT_H
#undef CONFIG_HAVE_SYS_STAT_H
#elif !defined(CONFIG_HAVE_SYS_STAT_H) && \
      (__has_include(<sys/stat.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix)))))
#define CONFIG_HAVE_SYS_STAT_H 1
#endif

#ifdef CONFIG_NO_FCNTL_H
#undef CONFIG_HAVE_FCNTL_H
#elif !defined(CONFIG_HAVE_FCNTL_H) && \
      (__has_include(<fcntl.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix)))))
#define CONFIG_HAVE_FCNTL_H 1
#endif

#ifdef CONFIG_NO_SYS_FCNTL_H
#undef CONFIG_HAVE_SYS_FCNTL_H
#elif !defined(CONFIG_HAVE_SYS_FCNTL_H) && \
      (__has_include(<sys/fcntl.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_FCNTL_H 1
#endif

#ifdef CONFIG_NO_SYS_IOCTL_H
#undef CONFIG_HAVE_SYS_IOCTL_H
#elif !defined(CONFIG_HAVE_SYS_IOCTL_H) && \
      (__has_include(<sys/ioctl.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_SYS_IOCTL_H 1
#endif

#ifdef CONFIG_NO_IOCTL_H
#undef CONFIG_HAVE_IOCTL_H
#elif !defined(CONFIG_HAVE_IOCTL_H) && \
      (__has_include(<ioctl.h>))
#define CONFIG_HAVE_IOCTL_H 1
#endif

#ifdef CONFIG_NO_UNISTD_H
#undef CONFIG_HAVE_UNISTD_H
#elif !defined(CONFIG_HAVE_UNISTD_H) && \
      (__has_include(<unistd.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_UNISTD_H 1
#endif

#ifdef CONFIG_NO_ERRNO_H
#undef CONFIG_HAVE_ERRNO_H
#elif !defined(CONFIG_HAVE_ERRNO_H) && \
      (__has_include(<errno.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix)))))
#define CONFIG_HAVE_ERRNO_H 1
#endif

#ifdef CONFIG_NO_STDIO_H
#undef CONFIG_HAVE_STDIO_H
#else
#define CONFIG_HAVE_STDIO_H 1
#endif

#ifdef CONFIG_NO_STDLIB_H
#undef CONFIG_HAVE_STDLIB_H
#else
#define CONFIG_HAVE_STDLIB_H 1
#endif

#ifdef CONFIG_NO_FEATURES_H
#undef CONFIG_HAVE_FEATURES_H
#elif !defined(CONFIG_HAVE_FEATURES_H) && \
      (__has_include(<features.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_FEATURES_H 1
#endif

#ifdef CONFIG_NO_SCHED_H
#undef CONFIG_HAVE_SCHED_H
#elif !defined(CONFIG_HAVE_SCHED_H) && \
      (__has_include(<sched.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_SCHED_H 1
#endif

#ifdef CONFIG_NO_SIGNAL_H
#undef CONFIG_HAVE_SIGNAL_H
#elif !defined(CONFIG_HAVE_SIGNAL_H) && \
      (__has_include(<signal.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_SIGNAL_H 1
#endif

#ifdef CONFIG_NO_SYS_SIGNAL_H
#undef CONFIG_HAVE_SYS_SIGNAL_H
#elif !defined(CONFIG_HAVE_SYS_SIGNAL_H) && \
      (__has_include(<sys/signal.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_SIGNAL_H 1
#endif

#ifdef CONFIG_NO_SYS_SYSCALL_H
#undef CONFIG_HAVE_SYS_SYSCALL_H
#elif !defined(CONFIG_HAVE_SYS_SYSCALL_H) && \
      (__has_include(<sys/syscall.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_SYSCALL_H 1
#endif

#ifdef CONFIG_NO_PTHREAD_H
#undef CONFIG_HAVE_PTHREAD_H
#elif !defined(CONFIG_HAVE_PTHREAD_H) && \
      (__has_include(<pthread.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_PTHREAD_H 1
#endif

#ifdef CONFIG_NO_SYS_TYPES_H
#undef CONFIG_HAVE_SYS_TYPES_H
#elif !defined(CONFIG_HAVE_SYS_TYPES_H) && \
      (__has_include(<sys/types.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_TYPES_H 1
#endif

#ifdef CONFIG_NO_SYS_SELECT_H
#undef CONFIG_HAVE_SYS_SELECT_H
#elif !defined(CONFIG_HAVE_SYS_SELECT_H) && \
      (__has_include(<sys/select.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_SELECT_H 1
#endif

#ifdef CONFIG_NO_SEMAPHORE_H
#undef CONFIG_HAVE_SEMAPHORE_H
#elif !defined(CONFIG_HAVE_SEMAPHORE_H) && \
      (__has_include(<semaphore.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SEMAPHORE_H 1
#endif

#ifdef CONFIG_NO_TIME_H
#undef CONFIG_HAVE_TIME_H
#elif !defined(CONFIG_HAVE_TIME_H) && \
      (__has_include(<time.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix)))))
#define CONFIG_HAVE_TIME_H 1
#endif

#ifdef CONFIG_NO_SYS_TIME_H
#undef CONFIG_HAVE_SYS_TIME_H
#elif !defined(CONFIG_HAVE_SYS_TIME_H) && \
      (__has_include(<sys/time.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_TIME_H 1
#endif

#ifdef CONFIG_NO_SYS_MMAN_H
#undef CONFIG_HAVE_SYS_MMAN_H
#elif !defined(CONFIG_HAVE_SYS_MMAN_H) && \
      (__has_include(<sys/mman.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_MMAN_H 1
#endif

#ifdef CONFIG_NO_SYS_WAIT_H
#undef CONFIG_HAVE_SYS_WAIT_H
#elif !defined(CONFIG_HAVE_SYS_WAIT_H) && \
      (__has_include(<sys/wait.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_SYS_WAIT_H 1
#endif

#ifdef CONFIG_NO_WAIT_H
#undef CONFIG_HAVE_WAIT_H
#elif !defined(CONFIG_HAVE_WAIT_H) && \
      (__has_include(<wait.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_WAIT_H 1
#endif

#ifdef CONFIG_NO_SYS_SIGNALFD_H
#undef CONFIG_HAVE_SYS_SIGNALFD_H
#elif !defined(CONFIG_HAVE_SYS_SIGNALFD_H) && \
      (__has_include(<sys/signalfd.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_SIGNALFD_H 1
#endif

#ifdef CONFIG_NO_CTYPE_H
#undef CONFIG_HAVE_CTYPE_H
#else
#define CONFIG_HAVE_CTYPE_H 1
#endif

#ifdef CONFIG_NO_STRING_H
#undef CONFIG_HAVE_STRING_H
#else
#define CONFIG_HAVE_STRING_H 1
#endif

#ifdef CONFIG_NO_WCHAR_H
#undef CONFIG_HAVE_WCHAR_H
#else
#define CONFIG_HAVE_WCHAR_H 1
#endif

#ifdef CONFIG_NO_DLFCN_H
#undef CONFIG_HAVE_DLFCN_H
#elif !defined(CONFIG_HAVE_DLFCN_H) && \
      (__has_include(<dlfcn.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_DLFCN_H 1
#endif

#ifdef CONFIG_NO_FLOAT_H
#undef CONFIG_HAVE_FLOAT_H
#else
#define CONFIG_HAVE_FLOAT_H 1
#endif

#ifdef CONFIG_NO_LIMITS_H
#undef CONFIG_HAVE_LIMITS_H
#else
#define CONFIG_HAVE_LIMITS_H 1
#endif

#ifdef CONFIG_NO_LINK_H
#undef CONFIG_HAVE_LINK_H
#else
#define CONFIG_HAVE_LINK_H 1
#endif

#ifdef CONFIG_HAVE_IO_H
#include <io.h>
#endif /* CONFIG_HAVE_IO_H */

#ifdef CONFIG_HAVE_DIRECT_H
#include <direct.h>
#endif /* CONFIG_HAVE_DIRECT_H */

#ifdef CONFIG_HAVE_PROCESS_H
#include <process.h>
#endif /* CONFIG_HAVE_PROCESS_H */

#ifdef CONFIG_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* CONFIG_HAVE_SYS_STAT_H */

#ifdef CONFIG_HAVE_FCNTL_H
#include <fcntl.h>
#endif /* CONFIG_HAVE_FCNTL_H */

#ifdef CONFIG_HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif /* CONFIG_HAVE_SYS_FCNTL_H */

#ifdef CONFIG_HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* CONFIG_HAVE_SYS_IOCTL_H */

#ifdef CONFIG_HAVE_IOCTL_H
#include <ioctl.h>
#endif /* CONFIG_HAVE_IOCTL_H */

#ifdef CONFIG_HAVE_UNISTD_H
#include <unistd.h>
#endif /* CONFIG_HAVE_UNISTD_H */

#ifdef CONFIG_HAVE_ERRNO_H
#include <errno.h>
#endif /* CONFIG_HAVE_ERRNO_H */

#ifdef CONFIG_HAVE_STDIO_H
#include <stdio.h>
#endif /* CONFIG_HAVE_STDIO_H */

#ifdef CONFIG_HAVE_STDLIB_H
#include <stdlib.h>
#endif /* CONFIG_HAVE_STDLIB_H */

#ifdef CONFIG_HAVE_FEATURES_H
#include <features.h>
#endif /* CONFIG_HAVE_FEATURES_H */

#ifdef CONFIG_HAVE_SCHED_H
#include <sched.h>
#endif /* CONFIG_HAVE_SCHED_H */

#ifdef CONFIG_HAVE_SIGNAL_H
#include <signal.h>
#endif /* CONFIG_HAVE_SIGNAL_H */

#ifdef CONFIG_HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif /* CONFIG_HAVE_SYS_SIGNAL_H */

#ifdef CONFIG_HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif /* CONFIG_HAVE_SYS_SYSCALL_H */

#ifdef CONFIG_HAVE_PTHREAD_H
#include <pthread.h>
#endif /* CONFIG_HAVE_PTHREAD_H */

#ifdef CONFIG_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* CONFIG_HAVE_SYS_TYPES_H */

#ifdef CONFIG_HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* CONFIG_HAVE_SYS_SELECT_H */

#ifdef CONFIG_HAVE_SEMAPHORE_H
#include <semaphore.h>
#endif /* CONFIG_HAVE_SEMAPHORE_H */

#ifdef CONFIG_HAVE_TIME_H
#include <time.h>
#endif /* CONFIG_HAVE_TIME_H */

#ifdef CONFIG_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* CONFIG_HAVE_SYS_TIME_H */

#ifdef CONFIG_HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif /* CONFIG_HAVE_SYS_MMAN_H */

#ifdef CONFIG_HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* CONFIG_HAVE_SYS_WAIT_H */

#ifdef CONFIG_HAVE_WAIT_H
#include <wait.h>
#endif /* CONFIG_HAVE_WAIT_H */

#ifdef CONFIG_HAVE_SYS_SIGNALFD_H
#include <sys/signalfd.h>
#endif /* CONFIG_HAVE_SYS_SIGNALFD_H */

#ifdef CONFIG_HAVE_CTYPE_H
#include <ctype.h>
#endif /* CONFIG_HAVE_CTYPE_H */

#ifdef CONFIG_HAVE_STRING_H
#include <string.h>
#endif /* CONFIG_HAVE_STRING_H */

#ifdef CONFIG_HAVE_WCHAR_H
#include <wchar.h>
#endif /* CONFIG_HAVE_WCHAR_H */

#ifdef CONFIG_NO__Exit
#undef CONFIG_HAVE__Exit
#elif !defined(CONFIG_HAVE__Exit) && \
      (defined(_Exit) || defined(___Exit_defined) || defined(__USE_ISOC99))
#define CONFIG_HAVE__Exit 1
#endif

#ifdef CONFIG_NO__exit
#undef CONFIG_HAVE__exit
#elif !defined(CONFIG_HAVE__exit) && \
      (defined(_exit) || defined(___exit_defined) || (defined(_MSC_VER) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE__exit 1
#endif

#ifdef CONFIG_NO_exit
#undef CONFIG_HAVE_exit
#else
#define CONFIG_HAVE_exit 1
#endif

#ifdef CONFIG_NO_atexit
#undef CONFIG_HAVE_atexit
#else
#define CONFIG_HAVE_atexit 1
#endif

#ifdef CONFIG_NO_execv
#undef CONFIG_HAVE_execv
#elif !defined(CONFIG_HAVE_execv) && \
      (defined(execv) || defined(__execv_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_execv 1
#endif

#ifdef CONFIG_NO_execve
#undef CONFIG_HAVE_execve
#elif !defined(CONFIG_HAVE_execve) && \
      (defined(execve) || defined(__execve_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_execve 1
#endif

#ifdef CONFIG_NO_execvp
#undef CONFIG_HAVE_execvp
#elif !defined(CONFIG_HAVE_execvp) && \
      (defined(execvp) || defined(__execvp_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_execvp 1
#endif

#ifdef CONFIG_NO_execvpe
#undef CONFIG_HAVE_execvpe
#elif !defined(CONFIG_HAVE_execvpe) && \
      (defined(execvpe) || defined(__execvpe_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_execvpe 1
#endif

#ifdef CONFIG_NO_fexecve
#undef CONFIG_HAVE_fexecve
#elif !defined(CONFIG_HAVE_fexecve) && \
      (defined(fexecve) || defined(__fexecve_defined) || defined(__USE_XOPEN2K8))
#define CONFIG_HAVE_fexecve 1
#endif

#ifdef CONFIG_NO__execv
#undef CONFIG_HAVE__execv
#elif !defined(CONFIG_HAVE__execv) && \
      (defined(_execv) || defined(___execv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execv 1
#endif

#ifdef CONFIG_NO__execve
#undef CONFIG_HAVE__execve
#elif !defined(CONFIG_HAVE__execve) && \
      (defined(_execve) || defined(___execve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execve 1
#endif

#ifdef CONFIG_NO__execvp
#undef CONFIG_HAVE__execvp
#elif !defined(CONFIG_HAVE__execvp) && \
      (defined(_execvp) || defined(___execvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execvp 1
#endif

#ifdef CONFIG_NO__execvpe
#undef CONFIG_HAVE__execvpe
#elif !defined(CONFIG_HAVE__execvpe) && \
      (defined(_execvpe) || defined(___execvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execvpe 1
#endif

#ifdef CONFIG_NO__wexecv
#undef CONFIG_HAVE__wexecv
#elif !defined(CONFIG_HAVE__wexecv) && \
      (defined(_wexecv) || defined(___wexecv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecv 1
#endif

#ifdef CONFIG_NO__wexecve
#undef CONFIG_HAVE__wexecve
#elif !defined(CONFIG_HAVE__wexecve) && \
      (defined(_wexecve) || defined(___wexecve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecve 1
#endif

#ifdef CONFIG_NO__wexecvp
#undef CONFIG_HAVE__wexecvp
#elif !defined(CONFIG_HAVE__wexecvp) && \
      (defined(_wexecvp) || defined(___wexecvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecvp 1
#endif

#ifdef CONFIG_NO__wexecvpe
#undef CONFIG_HAVE__wexecvpe
#elif !defined(CONFIG_HAVE__wexecvpe) && \
      (defined(_wexecvpe) || defined(___wexecvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecvpe 1
#endif

#ifdef CONFIG_NO__spawnv
#undef CONFIG_HAVE__spawnv
#elif !defined(CONFIG_HAVE__spawnv) && \
      (defined(_spawnv) || defined(___spawnv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnv 1
#endif

#ifdef CONFIG_NO__spawnve
#undef CONFIG_HAVE__spawnve
#elif !defined(CONFIG_HAVE__spawnve) && \
      (defined(_spawnve) || defined(___spawnve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnve 1
#endif

#ifdef CONFIG_NO__spawnvp
#undef CONFIG_HAVE__spawnvp
#elif !defined(CONFIG_HAVE__spawnvp) && \
      (defined(_spawnvp) || defined(___spawnvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnvp 1
#endif

#ifdef CONFIG_NO__spawnvpe
#undef CONFIG_HAVE__spawnvpe
#elif !defined(CONFIG_HAVE__spawnvpe) && \
      (defined(_spawnvpe) || defined(___spawnvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnvpe 1
#endif

#ifdef CONFIG_NO__wspawnv
#undef CONFIG_HAVE__wspawnv
#elif !defined(CONFIG_HAVE__wspawnv) && \
      (defined(_wspawnv) || defined(___wspawnv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnv 1
#endif

#ifdef CONFIG_NO__wspawnve
#undef CONFIG_HAVE__wspawnve
#elif !defined(CONFIG_HAVE__wspawnve) && \
      (defined(_wspawnve) || defined(___wspawnve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnve 1
#endif

#ifdef CONFIG_NO__wspawnvp
#undef CONFIG_HAVE__wspawnvp
#elif !defined(CONFIG_HAVE__wspawnvp) && \
      (defined(_wspawnvp) || defined(___wspawnvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnvp 1
#endif

#ifdef CONFIG_NO__wspawnvpe
#undef CONFIG_HAVE__wspawnvpe
#elif !defined(CONFIG_HAVE__wspawnvpe) && \
      (defined(_wspawnvpe) || defined(___wspawnvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnvpe 1
#endif

#ifdef CONFIG_NO__cwait
#undef CONFIG_HAVE__cwait
#elif !defined(CONFIG_HAVE__cwait) && \
      (defined(_cwait) || defined(___cwait_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__cwait 1
#endif

#ifdef CONFIG_NO_wait
#undef CONFIG_HAVE_wait
#elif !defined(CONFIG_HAVE_wait) && \
      (defined(wait) || defined(__wait_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_wait 1
#endif

#ifdef CONFIG_NO_waitpid
#undef CONFIG_HAVE_waitpid
#elif !defined(CONFIG_HAVE_waitpid) && \
      (defined(waitpid) || defined(__waitpid_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_waitpid 1
#endif

#ifdef CONFIG_NO_wait4
#undef CONFIG_HAVE_wait4
#elif !defined(CONFIG_HAVE_wait4) && \
      (defined(wait4) || defined(__wait4_defined) || ((defined(__linux__) || defined(__linux) || \
       defined(linux)) || defined(__KOS__)))
#define CONFIG_HAVE_wait4 1
#endif

#ifdef CONFIG_NO_waitid
#undef CONFIG_HAVE_waitid
#elif !defined(CONFIG_HAVE_waitid) && \
      (defined(waitid) || defined(__waitid_defined) || ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__)))
#define CONFIG_HAVE_waitid 1
#endif

#ifdef CONFIG_NO_sigprocmask
#undef CONFIG_HAVE_sigprocmask
#elif !defined(CONFIG_HAVE_sigprocmask) && \
      (defined(sigprocmask) || defined(__sigprocmask_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_sigprocmask 1
#endif

#ifdef CONFIG_NO_detach
#undef CONFIG_HAVE_detach
#elif !defined(CONFIG_HAVE_detach) && \
      (defined(detach) || defined(__detach_defined) || (defined(__KOS__) && defined(__USE_KOS) && \
       __KOS_VERSION__ >= 300))
#define CONFIG_HAVE_detach 1
#endif

#ifdef CONFIG_NO_system
#undef CONFIG_HAVE_system
#else
#define CONFIG_HAVE_system 1
#endif

#ifdef CONFIG_NO__wsystem
#undef CONFIG_HAVE__wsystem
#elif !defined(CONFIG_HAVE__wsystem) && \
      (defined(_wsystem) || defined(___wsystem_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wsystem 1
#endif

#ifdef CONFIG_NO_creat
#undef CONFIG_HAVE_creat
#elif !defined(CONFIG_HAVE_creat) && \
      (defined(creat) || defined(__creat_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_creat 1
#endif

#ifdef CONFIG_NO__creat
#undef CONFIG_HAVE__creat
#elif !defined(CONFIG_HAVE__creat) && \
      (defined(_creat) || defined(___creat_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__creat 1
#endif

#ifdef CONFIG_NO__wcreat
#undef CONFIG_HAVE__wcreat
#elif !defined(CONFIG_HAVE__wcreat) && \
      (defined(_wcreat) || defined(___wcreat_defined) || defined(_WIO_DEFINED))
#define CONFIG_HAVE__wcreat 1
#endif

#ifdef CONFIG_NO_open
#undef CONFIG_HAVE_open
#elif !defined(CONFIG_HAVE_open) && \
      (defined(open) || defined(__open_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_open 1
#endif

#ifdef CONFIG_NO__open
#undef CONFIG_HAVE__open
#elif !defined(CONFIG_HAVE__open) && \
      (defined(_open) || defined(___open_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__open 1
#endif

#ifdef CONFIG_NO__wopen
#undef CONFIG_HAVE__wopen
#elif !defined(CONFIG_HAVE__wopen) && \
      (defined(_wopen) || defined(___wopen_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wopen 1
#endif

#ifdef CONFIG_NO_open64
#undef CONFIG_HAVE_open64
#elif !defined(CONFIG_HAVE_open64) && \
      (defined(open64) || defined(__open64_defined) || defined(__USE_LARGEFILE64))
#define CONFIG_HAVE_open64 1
#endif

#ifdef CONFIG_NO_read
#undef CONFIG_HAVE_read
#elif !defined(CONFIG_HAVE_read) && \
      (defined(read) || defined(__read_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_read 1
#endif

#ifdef CONFIG_NO__read
#undef CONFIG_HAVE__read
#elif !defined(CONFIG_HAVE__read) && \
      (defined(_read) || defined(___read_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__read 1
#endif

#ifdef CONFIG_NO_write
#undef CONFIG_HAVE_write
#elif !defined(CONFIG_HAVE_write) && \
      (defined(write) || defined(__write_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_write 1
#endif

#ifdef CONFIG_NO__write
#undef CONFIG_HAVE__write
#elif !defined(CONFIG_HAVE__write) && \
      (defined(_write) || defined(___write_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__write 1
#endif

#ifdef CONFIG_NO_lseek
#undef CONFIG_HAVE_lseek
#elif !defined(CONFIG_HAVE_lseek) && \
      (defined(lseek) || defined(__lseek_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_lseek 1
#endif

#ifdef CONFIG_NO_lseek64
#undef CONFIG_HAVE_lseek64
#elif !defined(CONFIG_HAVE_lseek64) && \
      (defined(lseek64) || defined(__lseek64_defined) || defined(__USE_LARGEFILE64))
#define CONFIG_HAVE_lseek64 1
#endif

#ifdef CONFIG_NO__lseek
#undef CONFIG_HAVE__lseek
#elif !defined(CONFIG_HAVE__lseek) && \
      (defined(_lseek) || defined(___lseek_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__lseek 1
#endif

#ifdef CONFIG_NO__lseeki64
#undef CONFIG_HAVE__lseeki64
#elif !defined(CONFIG_HAVE__lseeki64) && \
      (defined(_lseeki64) || defined(___lseeki64_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__lseeki64 1
#endif

#ifdef CONFIG_NO_chdir
#undef CONFIG_HAVE_chdir
#elif !defined(CONFIG_HAVE_chdir) && \
      (defined(chdir) || defined(__chdir_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_chdir 1
#endif

#ifdef CONFIG_NO__chdir
#undef CONFIG_HAVE__chdir
#elif !defined(CONFIG_HAVE__chdir) && \
      (defined(_chdir) || defined(___chdir_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__chdir 1
#endif

#ifdef CONFIG_NO_readlink
#undef CONFIG_HAVE_readlink
#elif !defined(CONFIG_HAVE_readlink) && \
      (defined(readlink) || defined(__readlink_defined) || (defined(CONFIG_HAVE_UNISTD_H) && \
       (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))))
#define CONFIG_HAVE_readlink 1
#endif

#ifdef CONFIG_NO_freadlinkat
#undef CONFIG_HAVE_freadlinkat
#elif !defined(CONFIG_HAVE_freadlinkat) && \
      (defined(freadlinkat) || defined(__freadlinkat_defined) || (defined(CONFIG_HAVE_UNISTD_H) && \
       defined(__USE_KOS) && defined(__CRT_HAVE_freadlinkat)))
#define CONFIG_HAVE_freadlinkat 1
#endif

#ifdef CONFIG_NO_stat
#undef CONFIG_HAVE_stat
#elif !defined(CONFIG_HAVE_stat) && \
      (defined(stat) || defined(__stat_defined) || defined(CONFIG_HAVE_SYS_STAT_H))
#define CONFIG_HAVE_stat 1
#endif

#ifdef CONFIG_NO_fstat
#undef CONFIG_HAVE_fstat
#elif !defined(CONFIG_HAVE_fstat) && \
      (defined(fstat) || defined(__fstat_defined) || defined(CONFIG_HAVE_SYS_STAT_H))
#define CONFIG_HAVE_fstat 1
#endif

#ifdef CONFIG_NO_lstat
#undef CONFIG_HAVE_lstat
#elif !defined(CONFIG_HAVE_lstat) && \
      (defined(lstat) || defined(__lstat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))))
#define CONFIG_HAVE_lstat 1
#endif

#ifdef CONFIG_NO_stat64
#undef CONFIG_HAVE_stat64
#elif !defined(CONFIG_HAVE_stat64) && \
      (defined(stat64) || defined(__stat64_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_LARGEFILE64)))
#define CONFIG_HAVE_stat64 1
#endif

#ifdef CONFIG_NO_fstat64
#undef CONFIG_HAVE_fstat64
#elif !defined(CONFIG_HAVE_fstat64) && \
      (defined(fstat64) || defined(__fstat64_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_LARGEFILE64)))
#define CONFIG_HAVE_fstat64 1
#endif

#ifdef CONFIG_NO_lstat64
#undef CONFIG_HAVE_lstat64
#elif !defined(CONFIG_HAVE_lstat64) && \
      (defined(lstat64) || defined(__lstat64_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_LARGEFILE64) && (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))))
#define CONFIG_HAVE_lstat64 1
#endif

#ifdef CONFIG_NO_fstatat
#undef CONFIG_HAVE_fstatat
#elif !defined(CONFIG_HAVE_fstatat) && \
      (defined(fstatat) || defined(__fstatat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_ATFILE)))
#define CONFIG_HAVE_fstatat 1
#endif

#ifdef CONFIG_NO_fstatat64
#undef CONFIG_HAVE_fstatat64
#elif !defined(CONFIG_HAVE_fstatat64) && \
      (defined(fstatat64) || defined(__fstatat64_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_LARGEFILE64) && defined(__USE_ATFILE)))
#define CONFIG_HAVE_fstatat64 1
#endif

#ifdef CONFIG_NO_STAT_HAVE_ST_NSEC
#undef CONFIG_HAVE_STAT_HAVE_ST_NSEC
#elif !defined(CONFIG_HAVE_STAT_HAVE_ST_NSEC) && \
      (defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_NSEC))
#define CONFIG_HAVE_STAT_HAVE_ST_NSEC 1
#endif

#ifdef CONFIG_NO_STAT_HAVE_ST_TIM
#undef CONFIG_HAVE_STAT_HAVE_ST_TIM
#elif !defined(CONFIG_HAVE_STAT_HAVE_ST_TIM) && \
      (defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_TIM))
#define CONFIG_HAVE_STAT_HAVE_ST_TIM 1
#endif

#ifdef CONFIG_NO_STAT_HAVE_ST_TIMESPEC
#undef CONFIG_HAVE_STAT_HAVE_ST_TIMESPEC
#elif !defined(CONFIG_HAVE_STAT_HAVE_ST_TIMESPEC) && \
      (defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_TIMESPEC))
#define CONFIG_HAVE_STAT_HAVE_ST_TIMESPEC 1
#endif

#ifdef CONFIG_NO_mkdir
#undef CONFIG_HAVE_mkdir
#elif !defined(CONFIG_HAVE_mkdir) && \
      (defined(mkdir) || defined(__mkdir_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_mkdir 1
#endif

#ifdef CONFIG_NO__mkdir
#undef CONFIG_HAVE__mkdir
#elif !defined(CONFIG_HAVE__mkdir) && \
      (defined(_mkdir) || defined(___mkdir_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__mkdir 1
#endif

#ifdef CONFIG_NO_chmod
#undef CONFIG_HAVE_chmod
#elif !defined(CONFIG_HAVE_chmod) && \
      (defined(chmod) || defined(__chmod_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_chmod 1
#endif

#ifdef CONFIG_NO__chmod
#undef CONFIG_HAVE__chmod
#elif !defined(CONFIG_HAVE__chmod) && \
      (defined(_chmod) || defined(___chmod_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__chmod 1
#endif

#ifdef CONFIG_NO__wchmod
#undef CONFIG_HAVE__wchmod
#elif !defined(CONFIG_HAVE__wchmod) && \
      (defined(_wchmod) || defined(___wchmod_defined) || defined(_WIO_DEFINED))
#define CONFIG_HAVE__wchmod 1
#endif

#ifdef CONFIG_NO_mkfifo
#undef CONFIG_HAVE_mkfifo
#elif !defined(CONFIG_HAVE_mkfifo) && \
      (defined(mkfifo) || defined(__mkfifo_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_mkfifo 1
#endif

#ifdef CONFIG_NO_lchmod
#undef CONFIG_HAVE_lchmod
#elif !defined(CONFIG_HAVE_lchmod) && \
      (defined(lchmod) || defined(__lchmod_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_MISC)))
#define CONFIG_HAVE_lchmod 1
#endif

#ifdef CONFIG_NO_fchmodat
#undef CONFIG_HAVE_fchmodat
#elif !defined(CONFIG_HAVE_fchmodat) && \
      (defined(fchmodat) || defined(__fchmodat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_ATFILE)))
#define CONFIG_HAVE_fchmodat 1
#endif

#ifdef CONFIG_NO_mkdirat
#undef CONFIG_HAVE_mkdirat
#elif !defined(CONFIG_HAVE_mkdirat) && \
      (defined(mkdirat) || defined(__mkdirat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_ATFILE)))
#define CONFIG_HAVE_mkdirat 1
#endif

#ifdef CONFIG_NO_mkfifoat
#undef CONFIG_HAVE_mkfifoat
#elif !defined(CONFIG_HAVE_mkfifoat) && \
      (defined(mkfifoat) || defined(__mkfifoat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_ATFILE)))
#define CONFIG_HAVE_mkfifoat 1
#endif

#ifdef CONFIG_NO_fchmod
#undef CONFIG_HAVE_fchmod
#elif !defined(CONFIG_HAVE_fchmod) && \
      (defined(fchmod) || defined(__fchmod_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_POSIX)))
#define CONFIG_HAVE_fchmod 1
#endif

#ifdef CONFIG_NO_mknod
#undef CONFIG_HAVE_mknod
#elif !defined(CONFIG_HAVE_mknod) && \
      (defined(mknod) || defined(__mknod_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED))))
#define CONFIG_HAVE_mknod 1
#endif

#ifdef CONFIG_NO_mknodat
#undef CONFIG_HAVE_mknodat
#elif !defined(CONFIG_HAVE_mknodat) && \
      (defined(mknodat) || defined(__mknodat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)) && defined(__USE_ATFILE)))
#define CONFIG_HAVE_mknodat 1
#endif

#ifdef CONFIG_NO_utimensat
#undef CONFIG_HAVE_utimensat
#elif !defined(CONFIG_HAVE_utimensat) && \
      (defined(utimensat) || defined(__utimensat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_ATFILE)))
#define CONFIG_HAVE_utimensat 1
#endif

#ifdef CONFIG_NO_utimensat64
#undef CONFIG_HAVE_utimensat64
#elif !defined(CONFIG_HAVE_utimensat64) && \
      (defined(utimensat64) || defined(__utimensat64_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_ATFILE) && defined(__USE_TIME64)))
#define CONFIG_HAVE_utimensat64 1
#endif

#ifdef CONFIG_NO_futimens
#undef CONFIG_HAVE_futimens
#elif !defined(CONFIG_HAVE_futimens) && \
      (defined(futimens) || defined(__futimens_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_XOPEN2K8)))
#define CONFIG_HAVE_futimens 1
#endif

#ifdef CONFIG_NO_futimens64
#undef CONFIG_HAVE_futimens64
#elif !defined(CONFIG_HAVE_futimens64) && \
      (defined(futimens64) || defined(__futimens64_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_XOPEN2K8) && defined(__USE_TIME64)))
#define CONFIG_HAVE_futimens64 1
#endif

#ifdef CONFIG_NO_time
#undef CONFIG_HAVE_time
#elif !defined(CONFIG_HAVE_time) && \
      (defined(time) || defined(__time_defined) || defined(CONFIG_HAVE_TIME_H))
#define CONFIG_HAVE_time 1
#endif

#ifdef CONFIG_NO_time64
#undef CONFIG_HAVE_time64
#elif !defined(CONFIG_HAVE_time64) && \
      (defined(time64) || defined(__time64_defined) || (defined(CONFIG_HAVE_TIME_H) && \
       defined(__USE_TIME64)))
#define CONFIG_HAVE_time64 1
#endif

#ifdef CONFIG_NO_clock_gettime
#undef CONFIG_HAVE_clock_gettime
#elif !defined(CONFIG_HAVE_clock_gettime) && \
      (defined(clock_gettime) || defined(__clock_gettime_defined) || (defined(CONFIG_HAVE_TIME_H) && \
       defined(__USE_POSIX199309)))
#define CONFIG_HAVE_clock_gettime 1
#endif

#ifdef CONFIG_NO_clock_gettime64
#undef CONFIG_HAVE_clock_gettime64
#elif !defined(CONFIG_HAVE_clock_gettime64) && \
      (defined(clock_gettime64) || defined(__clock_gettime64_defined) || (defined(CONFIG_HAVE_TIME_H) && \
       defined(__USE_POSIX199309) && defined(__USE_TIME64)))
#define CONFIG_HAVE_clock_gettime64 1
#endif

#ifdef CONFIG_NO_CLOCK_REALTIME
#undef CONFIG_HAVE_CLOCK_REALTIME
#elif !defined(CONFIG_HAVE_CLOCK_REALTIME) && \
      (defined(CLOCK_REALTIME) || defined(__CLOCK_REALTIME_defined) || (defined(CONFIG_HAVE_TIME_H) && \
       defined(__USE_POSIX199309)))
#define CONFIG_HAVE_CLOCK_REALTIME 1
#endif

#ifdef CONFIG_NO_gettimeofday
#undef CONFIG_HAVE_gettimeofday
#elif !defined(CONFIG_HAVE_gettimeofday) && \
      (defined(gettimeofday) || defined(__gettimeofday_defined) || defined(CONFIG_HAVE_SYS_TIME_H))
#define CONFIG_HAVE_gettimeofday 1
#endif

#ifdef CONFIG_NO_gettimeofday64
#undef CONFIG_HAVE_gettimeofday64
#elif !defined(CONFIG_HAVE_gettimeofday64) && \
      (defined(gettimeofday64) || defined(__gettimeofday64_defined) || (defined(CONFIG_HAVE_SYS_TIME_H) && \
       defined(__USE_TIME64)))
#define CONFIG_HAVE_gettimeofday64 1
#endif

#ifdef CONFIG_NO_utimes
#undef CONFIG_HAVE_utimes
#elif !defined(CONFIG_HAVE_utimes) && \
      (defined(utimes) || defined(__utimes_defined) || (defined(CONFIG_HAVE_SYS_TIME_H) && \
       defined(__USE_MISC)))
#define CONFIG_HAVE_utimes 1
#endif

#ifdef CONFIG_NO_utimes64
#undef CONFIG_HAVE_utimes64
#elif !defined(CONFIG_HAVE_utimes64) && \
      (defined(utimes64) || defined(__utimes64_defined) || (defined(CONFIG_HAVE_SYS_TIME_H) && \
       defined(__USE_MISC) && defined(__USE_TIME64)))
#define CONFIG_HAVE_utimes64 1
#endif

#ifdef CONFIG_NO_lutimes
#undef CONFIG_HAVE_lutimes
#elif !defined(CONFIG_HAVE_lutimes) && \
      (defined(lutimes) || defined(__lutimes_defined) || defined(CONFIG_HAVE_SYS_TIME_H))
#define CONFIG_HAVE_lutimes 1
#endif

#ifdef CONFIG_NO_lutimes64
#undef CONFIG_HAVE_lutimes64
#elif !defined(CONFIG_HAVE_lutimes64) && \
      (defined(lutimes64) || defined(__lutimes64_defined) || (defined(CONFIG_HAVE_SYS_TIME_H) && \
       defined(__USE_TIME64)))
#define CONFIG_HAVE_lutimes64 1
#endif

#ifdef CONFIG_NO_futimesat
#undef CONFIG_HAVE_futimesat
#elif !defined(CONFIG_HAVE_futimesat) && \
      (defined(futimesat) || defined(__futimesat_defined) || (defined(CONFIG_HAVE_SYS_TIME_H) && \
       defined(__USE_GNU)))
#define CONFIG_HAVE_futimesat 1
#endif

#ifdef CONFIG_NO_futimesat64
#undef CONFIG_HAVE_futimesat64
#elif !defined(CONFIG_HAVE_futimesat64) && \
      (defined(futimesat64) || defined(__futimesat64_defined) || (defined(CONFIG_HAVE_SYS_TIME_H) && \
       defined(__USE_GNU) && defined(__USE_TIME64)))
#define CONFIG_HAVE_futimesat64 1
#endif

#if defined(_MSC_VER)
#define F_OK     0
#define X_OK     1 /* Not supported? */
#define W_OK     2
#define R_OK     4
#endif /* defined(_MSC_VER) */

#ifdef CONFIG_NO_access
#undef CONFIG_HAVE_access
#elif !defined(CONFIG_HAVE_access) && \
      (defined(access) || defined(__access_defined) || ((defined(CONFIG_HAVE_UNISTD_H) || \
       !defined(_MSC_VER)) && defined(F_OK) && defined(X_OK) && defined(W_OK) && \
       defined(R_OK)))
#define CONFIG_HAVE_access 1
#endif

#ifdef CONFIG_NO_euidaccess
#undef CONFIG_HAVE_euidaccess
#elif !defined(CONFIG_HAVE_euidaccess) && \
      (defined(euidaccess) || defined(__euidaccess_defined) || (defined(F_OK) && \
       defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_GNU)))
#define CONFIG_HAVE_euidaccess 1
#endif

#ifdef CONFIG_NO_eaccess
#undef CONFIG_HAVE_eaccess
#elif !defined(CONFIG_HAVE_eaccess) && \
      (defined(eaccess) || defined(__eaccess_defined) || (defined(F_OK) && defined(X_OK) && \
       defined(W_OK) && defined(R_OK) && defined(__USE_GNU)))
#define CONFIG_HAVE_eaccess 1
#endif

#ifdef CONFIG_NO_faccessat
#undef CONFIG_HAVE_faccessat
#elif !defined(CONFIG_HAVE_faccessat) && \
      (defined(faccessat) || defined(__faccessat_defined) || (defined(F_OK) && \
       defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_ATFILE)))
#define CONFIG_HAVE_faccessat 1
#endif

#ifdef CONFIG_NO__access
#undef CONFIG_HAVE__access
#elif !defined(CONFIG_HAVE__access) && \
      (defined(_access) || defined(___access_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__access 1
#endif

#ifdef CONFIG_NO__waccess
#undef CONFIG_HAVE__waccess
#elif !defined(CONFIG_HAVE__waccess) && \
      (defined(_waccess) || defined(___waccess_defined) || defined(_WIO_DEFINED))
#define CONFIG_HAVE__waccess 1
#endif

#ifdef CONFIG_NO_fchownat
#undef CONFIG_HAVE_fchownat
#elif !defined(CONFIG_HAVE_fchownat) && \
      (defined(fchownat) || defined(__fchownat_defined) || defined(__USE_ATFILE))
#define CONFIG_HAVE_fchownat 1
#endif

#ifdef CONFIG_NO_pread
#undef CONFIG_HAVE_pread
#elif !defined(CONFIG_HAVE_pread) && \
      (defined(pread) || defined(__pread_defined) || (defined(__USE_UNIX98) || \
       defined(__USE_XOPEN2K8)))
#define CONFIG_HAVE_pread 1
#endif

#ifdef CONFIG_NO_pwrite
#undef CONFIG_HAVE_pwrite
#elif !defined(CONFIG_HAVE_pwrite) && \
      (defined(pwrite) || defined(__pwrite_defined) || (defined(__USE_UNIX98) || \
       defined(__USE_XOPEN2K8)))
#define CONFIG_HAVE_pwrite 1
#endif

#ifdef CONFIG_NO_pread64
#undef CONFIG_HAVE_pread64
#elif !defined(CONFIG_HAVE_pread64) && \
      (defined(pread64) || defined(__pread64_defined) || (defined(__USE_LARGEFILE64) && \
       (defined(__USE_UNIX98) || defined(__USE_XOPEN2K8))))
#define CONFIG_HAVE_pread64 1
#endif

#ifdef CONFIG_NO_pwrite64
#undef CONFIG_HAVE_pwrite64
#elif !defined(CONFIG_HAVE_pwrite64) && \
      (defined(pwrite64) || defined(__pwrite64_defined) || (defined(__USE_LARGEFILE64) && \
       (defined(__USE_UNIX98) || defined(__USE_XOPEN2K8))))
#define CONFIG_HAVE_pwrite64 1
#endif

#ifdef CONFIG_NO_close
#undef CONFIG_HAVE_close
#elif !defined(CONFIG_HAVE_close) && \
      (defined(close) || defined(__close_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_close 1
#endif

#ifdef CONFIG_NO__close
#undef CONFIG_HAVE__close
#elif !defined(CONFIG_HAVE__close) && \
      (defined(_close) || defined(___close_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__close 1
#endif

#ifdef CONFIG_NO_sync
#undef CONFIG_HAVE_sync
#elif !defined(CONFIG_HAVE_sync) && \
      (defined(sync) || defined(__sync_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_sync 1
#endif

#ifdef CONFIG_NO_fsync
#undef CONFIG_HAVE_fsync
#elif !defined(CONFIG_HAVE_fsync) && \
      (defined(fsync) || defined(__fsync_defined) || ((defined(_POSIX_FSYNC) && \
       _POSIX_FSYNC+0 != 0) || (!defined(CONFIG_HAVE_UNISTD_H) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))))
#define CONFIG_HAVE_fsync 1
#endif

#ifdef CONFIG_NO_fdatasync
#undef CONFIG_HAVE_fdatasync
#elif !defined(CONFIG_HAVE_fdatasync) && \
      (defined(fdatasync) || defined(__fdatasync_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_fdatasync 1
#endif

#ifdef CONFIG_NO__commit
#undef CONFIG_HAVE__commit
#elif !defined(CONFIG_HAVE__commit) && \
      (defined(_commit) || defined(___commit_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__commit 1
#endif

#ifdef CONFIG_NO_getpid
#undef CONFIG_HAVE_getpid
#elif !defined(CONFIG_HAVE_getpid) && \
      (defined(getpid) || defined(__getpid_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_getpid 1
#endif

#ifdef CONFIG_NO__getpid
#undef CONFIG_HAVE__getpid
#elif !defined(CONFIG_HAVE__getpid) && \
      (defined(_getpid) || defined(___getpid_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__getpid 1
#endif

#ifdef CONFIG_NO_umask
#undef CONFIG_HAVE_umask
#elif !defined(CONFIG_HAVE_umask) && \
      (defined(umask) || defined(__umask_defined) || defined(CONFIG_HAVE_SYS_STAT_H))
#define CONFIG_HAVE_umask 1
#endif

#ifdef CONFIG_NO__umask
#undef CONFIG_HAVE__umask
#elif !defined(CONFIG_HAVE__umask) && \
      (defined(_umask) || defined(___umask_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__umask 1
#endif

#ifdef CONFIG_NO_dup
#undef CONFIG_HAVE_dup
#elif !defined(CONFIG_HAVE_dup) && \
      (defined(dup) || defined(__dup_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_dup 1
#endif

#ifdef CONFIG_NO__dup
#undef CONFIG_HAVE__dup
#elif !defined(CONFIG_HAVE__dup) && \
      (defined(_dup) || defined(___dup_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__dup 1
#endif

#ifdef CONFIG_NO_dup2
#undef CONFIG_HAVE_dup2
#elif !defined(CONFIG_HAVE_dup2) && \
      (defined(dup2) || defined(__dup2_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_dup2 1
#endif

#ifdef CONFIG_NO__dup2
#undef CONFIG_HAVE__dup2
#elif !defined(CONFIG_HAVE__dup2) && \
      (defined(_dup2) || defined(___dup2_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__dup2 1
#endif

#ifdef CONFIG_NO_dup3
#undef CONFIG_HAVE_dup3
#elif !defined(CONFIG_HAVE_dup3) && \
      (defined(dup3) || defined(__dup3_defined) || defined(__USE_GNU))
#define CONFIG_HAVE_dup3 1
#endif

#ifdef CONFIG_NO_isatty
#undef CONFIG_HAVE_isatty
#elif !defined(CONFIG_HAVE_isatty) && \
      (defined(isatty) || defined(__isatty_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_isatty 1
#endif

#ifdef CONFIG_NO__isatty
#undef CONFIG_HAVE__isatty
#elif !defined(CONFIG_HAVE__isatty) && \
      (defined(_isatty) || defined(___isatty_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__isatty 1
#endif

#ifdef CONFIG_NO_getcwd
#undef CONFIG_HAVE_getcwd
#elif !defined(CONFIG_HAVE_getcwd) && \
      (defined(getcwd) || defined(__getcwd_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_getcwd 1
#endif

#ifdef CONFIG_NO__getcwd
#undef CONFIG_HAVE__getcwd
#elif !defined(CONFIG_HAVE__getcwd) && \
      (defined(_getcwd) || defined(___getcwd_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__getcwd 1
#endif

#ifdef CONFIG_NO_wgetcwd
#undef CONFIG_HAVE_wgetcwd
#elif !defined(CONFIG_HAVE_wgetcwd) && \
      (defined(wgetcwd) || defined(__wgetcwd_defined))
#define CONFIG_HAVE_wgetcwd 1
#endif

#ifdef CONFIG_NO__wgetcwd
#undef CONFIG_HAVE__wgetcwd
#elif !defined(CONFIG_HAVE__wgetcwd) && \
      (defined(_wgetcwd) || defined(___wgetcwd_defined) || defined(_WDIRECT_DEFINED))
#define CONFIG_HAVE__wgetcwd 1
#endif

#ifdef CONFIG_NO_unlink
#undef CONFIG_HAVE_unlink
#elif !defined(CONFIG_HAVE_unlink) && \
      (defined(unlink) || defined(__unlink_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_unlink 1
#endif

#ifdef CONFIG_NO__unlink
#undef CONFIG_HAVE__unlink
#elif !defined(CONFIG_HAVE__unlink) && \
      (defined(_unlink) || defined(___unlink_defined) || defined(_CRT_DIRECTORY_DEFINED))
#define CONFIG_HAVE__unlink 1
#endif

#ifdef CONFIG_NO_remove
#undef CONFIG_HAVE_remove
#elif !defined(CONFIG_HAVE_remove) && \
      (defined(remove) || defined(__remove_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_remove 1
#endif

#ifdef CONFIG_NO__wunlink
#undef CONFIG_HAVE__wunlink
#elif !defined(CONFIG_HAVE__wunlink) && \
      (defined(_wunlink) || defined(___wunlink_defined) || defined(_WIO_DEFINED))
#define CONFIG_HAVE__wunlink 1
#endif

#ifdef CONFIG_NO__wremove
#undef CONFIG_HAVE__wremove
#elif !defined(CONFIG_HAVE__wremove) && \
      (defined(_wremove) || defined(___wremove_defined) || defined(_WSTDIO_DEFINED))
#define CONFIG_HAVE__wremove 1
#endif

#ifdef CONFIG_NO_rename
#undef CONFIG_HAVE_rename
#elif !defined(CONFIG_HAVE_rename) && \
      (defined(rename) || defined(__rename_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_rename 1
#endif

#ifdef CONFIG_NO__wrename
#undef CONFIG_HAVE__wrename
#elif !defined(CONFIG_HAVE__wrename) && \
      (defined(_wrename) || defined(___wrename_defined) || defined(_WIO_DEFINED))
#define CONFIG_HAVE__wrename 1
#endif

#ifdef CONFIG_NO_getenv
#undef CONFIG_HAVE_getenv
#elif !defined(CONFIG_HAVE_getenv) && \
      (defined(getenv) || defined(__getenv_defined) || defined(CONFIG_HAVE_STDLIB_H))
#define CONFIG_HAVE_getenv 1
#endif

#ifdef CONFIG_NO_wcslen
#undef CONFIG_HAVE_wcslen
#elif !defined(CONFIG_HAVE_wcslen) && \
      (defined(wcslen) || defined(__wcslen_defined) || defined(CONFIG_HAVE_WCHAR_H))
#define CONFIG_HAVE_wcslen 1
#endif

#ifdef CONFIG_NO_qsort
#undef CONFIG_HAVE_qsort
#elif !defined(CONFIG_HAVE_qsort) && \
      (defined(qsort) || defined(__qsort_defined) || defined(CONFIG_HAVE_STDLIB_H))
#define CONFIG_HAVE_qsort 1
#endif

#ifdef CONFIG_NO_truncate
#undef CONFIG_HAVE_truncate
#elif !defined(CONFIG_HAVE_truncate) && \
      (defined(truncate) || defined(__truncate_defined) || ((defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)))
#define CONFIG_HAVE_truncate 1
#endif

#ifdef CONFIG_NO_truncate64
#undef CONFIG_HAVE_truncate64
#elif !defined(CONFIG_HAVE_truncate64) && \
      (defined(truncate64) || defined(__truncate64_defined) || (defined(__USE_LARGEFILE64) && \
       ((defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix)) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8))))
#define CONFIG_HAVE_truncate64 1
#endif

#ifdef CONFIG_NO_ftruncate
#undef CONFIG_HAVE_ftruncate
#elif !defined(CONFIG_HAVE_ftruncate) && \
      (defined(ftruncate) || defined(__ftruncate_defined) || ((defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)) || defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || \
       defined(__USE_XOPEN2K)))
#define CONFIG_HAVE_ftruncate 1
#endif

#ifdef CONFIG_NO_ftruncate64
#undef CONFIG_HAVE_ftruncate64
#elif !defined(CONFIG_HAVE_ftruncate64) && \
      (defined(ftruncate64) || defined(__ftruncate64_defined) || (defined(__USE_LARGEFILE64) && \
       ((defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix)) || defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || \
       defined(__USE_XOPEN2K))))
#define CONFIG_HAVE_ftruncate64 1
#endif

#ifdef CONFIG_NO__chsize
#undef CONFIG_HAVE__chsize
#elif !defined(CONFIG_HAVE__chsize) && \
      (defined(_chsize) || defined(___chsize_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__chsize 1
#endif

#ifdef CONFIG_NO__chsize_s
#undef CONFIG_HAVE__chsize_s
#elif !defined(CONFIG_HAVE__chsize_s) && \
      (defined(_chsize_s) || defined(___chsize_s_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__chsize_s 1
#endif

#ifdef CONFIG_NO_getpgid
#undef CONFIG_HAVE_getpgid
#elif !defined(CONFIG_HAVE_getpgid) && \
      (defined(getpgid) || defined(__getpgid_defined) || (defined(_POSIX_JOB_CONTROL) && \
       _POSIX_JOB_CONTROL+0 != 0))
#define CONFIG_HAVE_getpgid 1
#endif

#ifdef CONFIG_NO_setpgid
#undef CONFIG_HAVE_setpgid
#elif !defined(CONFIG_HAVE_setpgid) && \
      (defined(setpgid) || defined(__setpgid_defined) || (defined(_POSIX_JOB_CONTROL) && \
       _POSIX_JOB_CONTROL+0 != 0))
#define CONFIG_HAVE_setpgid 1
#endif

#ifdef CONFIG_NO_setreuid
#undef CONFIG_HAVE_setreuid
#elif !defined(CONFIG_HAVE_setreuid) && \
      (defined(setreuid) || defined(__setreuid_defined) || ((defined(_POSIX_SAVED_IDS) && \
       _POSIX_SAVED_IDS+0 != 0) || (!defined(CONFIG_HAVE_UNISTD_H) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))))
#define CONFIG_HAVE_setreuid 1
#endif

#ifdef CONFIG_NO_nice
#undef CONFIG_HAVE_nice
#elif !defined(CONFIG_HAVE_nice) && \
      (defined(nice) || defined(__nice_defined) || (defined(__USE_MISC) || defined(__USE_XOPEN) || \
       ((defined(_POSIX_PRIORITY_SCHEDULING) && _POSIX_PRIORITY_SCHEDULING+0 != \
       0) || (!defined(CONFIG_HAVE_UNISTD_H) && (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix))))))
#define CONFIG_HAVE_nice 1
#endif

#ifdef CONFIG_NO_mmap
#undef CONFIG_HAVE_mmap
#elif !defined(CONFIG_HAVE_mmap) && \
      (defined(mmap) || defined(__mmap_defined) || ((defined(_POSIX_MAPPED_FILES) && \
       _POSIX_MAPPED_FILES+0 != 0) || (!defined(CONFIG_HAVE_UNISTD_H) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))))
#define CONFIG_HAVE_mmap 1
#endif

#ifdef CONFIG_NO_mmap64
#undef CONFIG_HAVE_mmap64
#elif !defined(CONFIG_HAVE_mmap64) && \
      (defined(mmap64) || defined(__mmap64_defined) || (defined(__USE_LARGEFILE64) && \
       ((defined(_POSIX_MAPPED_FILES) && _POSIX_MAPPED_FILES+0 != 0) || (!defined(CONFIG_HAVE_UNISTD_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))))
#define CONFIG_HAVE_mmap64 1
#endif

#ifdef CONFIG_NO_munmap
#undef CONFIG_HAVE_munmap
#elif !defined(CONFIG_HAVE_munmap) && \
      (defined(munmap) || defined(__munmap_defined) || CONFIG_HAVE_mmap)
#define CONFIG_HAVE_munmap 1
#endif

#ifdef CONFIG_NO_MAP_ANONYMOUS
#undef CONFIG_HAVE_MAP_ANONYMOUS
#elif !defined(CONFIG_HAVE_MAP_ANONYMOUS) && \
      (defined(MAP_ANONYMOUS) || defined(__MAP_ANONYMOUS_defined))
#define CONFIG_HAVE_MAP_ANONYMOUS 1
#endif

#ifdef CONFIG_NO_MAP_ANON
#undef CONFIG_HAVE_MAP_ANON
#elif !defined(CONFIG_HAVE_MAP_ANON) && \
      (defined(MAP_ANON) || defined(__MAP_ANON_defined))
#define CONFIG_HAVE_MAP_ANON 1
#endif

#ifdef CONFIG_NO_MAP_PRIVATE
#undef CONFIG_HAVE_MAP_PRIVATE
#elif !defined(CONFIG_HAVE_MAP_PRIVATE) && \
      (defined(MAP_PRIVATE) || defined(__MAP_PRIVATE_defined))
#define CONFIG_HAVE_MAP_PRIVATE 1
#endif

#ifdef CONFIG_NO_MAP_GROWSUP
#undef CONFIG_HAVE_MAP_GROWSUP
#elif !defined(CONFIG_HAVE_MAP_GROWSUP) && \
      (defined(MAP_GROWSUP) || defined(__MAP_GROWSUP_defined))
#define CONFIG_HAVE_MAP_GROWSUP 1
#endif

#ifdef CONFIG_NO_MAP_GROWSDOWN
#undef CONFIG_HAVE_MAP_GROWSDOWN
#elif !defined(CONFIG_HAVE_MAP_GROWSDOWN) && \
      (defined(MAP_GROWSDOWN) || defined(__MAP_GROWSDOWN_defined))
#define CONFIG_HAVE_MAP_GROWSDOWN 1
#endif

#ifdef CONFIG_NO_MAP_FILE
#undef CONFIG_HAVE_MAP_FILE
#elif !defined(CONFIG_HAVE_MAP_FILE) && \
      (defined(MAP_FILE) || defined(__MAP_FILE_defined))
#define CONFIG_HAVE_MAP_FILE 1
#endif

#ifdef CONFIG_NO_MAP_STACK
#undef CONFIG_HAVE_MAP_STACK
#elif !defined(CONFIG_HAVE_MAP_STACK) && \
      (defined(MAP_STACK) || defined(__MAP_STACK_defined))
#define CONFIG_HAVE_MAP_STACK 1
#endif

#ifdef CONFIG_NO_MAP_UNINITIALIZED
#undef CONFIG_HAVE_MAP_UNINITIALIZED
#elif !defined(CONFIG_HAVE_MAP_UNINITIALIZED) && \
      (defined(MAP_UNINITIALIZED) || defined(__MAP_UNINITIALIZED_defined))
#define CONFIG_HAVE_MAP_UNINITIALIZED 1
#endif

#ifdef CONFIG_NO_PROT_READ
#undef CONFIG_HAVE_PROT_READ
#elif !defined(CONFIG_HAVE_PROT_READ) && \
      (defined(PROT_READ) || defined(__PROT_READ_defined))
#define CONFIG_HAVE_PROT_READ 1
#endif

#ifdef CONFIG_NO_PROT_WRITE
#undef CONFIG_HAVE_PROT_WRITE
#elif !defined(CONFIG_HAVE_PROT_WRITE) && \
      (defined(PROT_WRITE) || defined(__PROT_WRITE_defined))
#define CONFIG_HAVE_PROT_WRITE 1
#endif

#ifdef CONFIG_NO_pipe
#undef CONFIG_HAVE_pipe
#elif !defined(CONFIG_HAVE_pipe) && \
      (defined(pipe) || defined(__pipe_defined) || (defined(CONFIG_HAVE_UNISTD_H) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_pipe 1
#endif

#ifdef CONFIG_NO_pipe2
#undef CONFIG_HAVE_pipe2
#elif !defined(CONFIG_HAVE_pipe2) && \
      (defined(pipe2) || defined(__pipe2_defined) || defined(__USE_GNU))
#define CONFIG_HAVE_pipe2 1
#endif

#ifdef CONFIG_NO__pipe
#undef CONFIG_HAVE__pipe
#elif !defined(CONFIG_HAVE__pipe) && \
      (defined(_pipe) || defined(___pipe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__pipe 1
#endif

#ifdef CONFIG_NO_usleep
#undef CONFIG_HAVE_usleep
#elif !defined(CONFIG_HAVE_usleep) && \
      (defined(usleep) || defined(__usleep_defined) || (defined(CONFIG_HAVE_UNISTD_H) && \
       ((defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || defined(__USE_MISC))))
#define CONFIG_HAVE_usleep 1
#endif

#ifdef CONFIG_NO_useconds_t
#undef CONFIG_HAVE_useconds_t
#elif !defined(CONFIG_HAVE_useconds_t) && \
      (defined(useconds_t) || defined(__useconds_t_defined) || (defined(CONFIG_HAVE_UNISTD_H) && \
       (defined(__USE_XOPEN) || defined(__USE_XOPEN2K))))
#define CONFIG_HAVE_useconds_t 1
#endif

#ifdef CONFIG_NO_nanosleep
#undef CONFIG_HAVE_nanosleep
#elif !defined(CONFIG_HAVE_nanosleep) && \
      (defined(nanosleep) || defined(__nanosleep_defined) || (defined(CONFIG_HAVE_TIME_H) && \
       defined(__USE_POSIX199309)))
#define CONFIG_HAVE_nanosleep 1
#endif

#ifdef CONFIG_NO_nanosleep64
#undef CONFIG_HAVE_nanosleep64
#elif !defined(CONFIG_HAVE_nanosleep64) && \
      (defined(nanosleep64) || defined(__nanosleep64_defined) || (defined(CONFIG_HAVE_TIME_H) && \
       defined(__USE_POSIX199309) && defined(__USE_TIME64)))
#define CONFIG_HAVE_nanosleep64 1
#endif

#ifdef CONFIG_NO_fork
#undef CONFIG_HAVE_fork
#elif !defined(CONFIG_HAVE_fork) && \
      (defined(fork) || defined(__fork_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_fork 1
#endif

#ifdef CONFIG_NO_vfork
#undef CONFIG_HAVE_vfork
#elif !defined(CONFIG_HAVE_vfork) && \
      (defined(vfork) || defined(__vfork_defined) || ((defined(__USE_XOPEN_EXTENDED) && \
       !defined(__USE_XOPEN2K8)) || defined(__USE_MISC)))
#define CONFIG_HAVE_vfork 1
#endif

#ifdef CONFIG_NO_fchown
#undef CONFIG_HAVE_fchown
#elif !defined(CONFIG_HAVE_fchown) && \
      (defined(fchown) || defined(__fchown_defined) || (defined(CONFIG_HAVE_UNISTD_H) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_fchown 1
#endif

#ifdef CONFIG_NO_fchdir
#undef CONFIG_HAVE_fchdir
#elif !defined(CONFIG_HAVE_fchdir) && \
      (defined(fchdir) || defined(__fchdir_defined) || (defined(CONFIG_HAVE_UNISTD_H) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_fchdir 1
#endif

#ifdef CONFIG_NO_pause
#undef CONFIG_HAVE_pause
#elif !defined(CONFIG_HAVE_pause) && \
      (defined(pause) || defined(__pause_defined) || (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix)))
#define CONFIG_HAVE_pause 1
#endif

#ifdef CONFIG_NO_select
#undef CONFIG_HAVE_select
#elif !defined(CONFIG_HAVE_select) && \
      (defined(select) || defined(__select_defined) || (defined(CONFIG_HAVE_SYS_SELECT_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_select 1
#endif

#ifdef CONFIG_NO___iob_func
#undef CONFIG_HAVE___iob_func
#elif !defined(CONFIG_HAVE___iob_func) && \
      (defined(__iob_func) || defined(____iob_func_defined) || (defined(_MSC_VER) || \
       defined(____iob_func_defined)))
#define CONFIG_HAVE___iob_func 1
#endif

#ifdef CONFIG_NO_fseek
#undef CONFIG_HAVE_fseek
#elif !defined(CONFIG_HAVE_fseek) && \
      (defined(fseek) || defined(__fseek_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fseek 1
#endif

#ifdef CONFIG_NO_ftell
#undef CONFIG_HAVE_ftell
#elif !defined(CONFIG_HAVE_ftell) && \
      (defined(ftell) || defined(__ftell_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_ftell 1
#endif

#ifdef CONFIG_NO_fseek64
#undef CONFIG_HAVE_fseek64
#elif !defined(CONFIG_HAVE_fseek64) && \
      (defined(fseek64) || defined(__fseek64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       0))
#define CONFIG_HAVE_fseek64 1
#endif

#ifdef CONFIG_NO_ftell64
#undef CONFIG_HAVE_ftell64
#elif !defined(CONFIG_HAVE_ftell64) && \
      (defined(ftell64) || defined(__ftell64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       0))
#define CONFIG_HAVE_ftell64 1
#endif

#ifdef CONFIG_NO_fseeko
#undef CONFIG_HAVE_fseeko
#elif !defined(CONFIG_HAVE_fseeko) && \
      (defined(fseeko) || defined(__fseeko_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_fseeko 1
#endif

#ifdef CONFIG_NO_ftello
#undef CONFIG_HAVE_ftello
#elif !defined(CONFIG_HAVE_ftello) && \
      (defined(ftello) || defined(__ftello_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_ftello 1
#endif

#ifdef CONFIG_NO_fseeko64
#undef CONFIG_HAVE_fseeko64
#elif !defined(CONFIG_HAVE_fseeko64) && \
      (defined(fseeko64) || defined(__fseeko64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       defined(__USE_LARGEFILE64)))
#define CONFIG_HAVE_fseeko64 1
#endif

#ifdef CONFIG_NO_ftello64
#undef CONFIG_HAVE_ftello64
#elif !defined(CONFIG_HAVE_ftello64) && \
      (defined(ftello64) || defined(__ftello64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       defined(__USE_LARGEFILE64)))
#define CONFIG_HAVE_ftello64 1
#endif

#ifdef CONFIG_NO__fseeki64
#undef CONFIG_HAVE__fseeki64
#elif !defined(CONFIG_HAVE__fseeki64) && \
      (defined(_fseeki64) || defined(___fseeki64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       defined(_MSC_VER)))
#define CONFIG_HAVE__fseeki64 1
#endif

#ifdef CONFIG_NO__ftelli64
#undef CONFIG_HAVE__ftelli64
#elif !defined(CONFIG_HAVE__ftelli64) && \
      (defined(_ftelli64) || defined(___ftelli64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       defined(_MSC_VER)))
#define CONFIG_HAVE__ftelli64 1
#endif

#ifdef CONFIG_NO_fflush
#undef CONFIG_HAVE_fflush
#elif !defined(CONFIG_HAVE_fflush) && \
      (defined(fflush) || defined(__fflush_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fflush 1
#endif

#ifdef CONFIG_NO_ferror
#undef CONFIG_HAVE_ferror
#elif !defined(CONFIG_HAVE_ferror) && \
      (defined(ferror) || defined(__ferror_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_ferror 1
#endif

#ifdef CONFIG_NO_fclose
#undef CONFIG_HAVE_fclose
#elif !defined(CONFIG_HAVE_fclose) && \
      (defined(fclose) || defined(__fclose_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fclose 1
#endif

#ifdef CONFIG_NO_fileno
#undef CONFIG_HAVE_fileno
#elif !defined(CONFIG_HAVE_fileno) && \
      (defined(fileno) || defined(__fileno_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       !defined(_MSC_VER)))
#define CONFIG_HAVE_fileno 1
#endif

#ifdef CONFIG_NO__fileno
#undef CONFIG_HAVE__fileno
#elif !defined(CONFIG_HAVE__fileno) && \
      (defined(_fileno) || defined(___fileno_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       defined(_MSC_VER)))
#define CONFIG_HAVE__fileno 1
#endif

#ifdef CONFIG_NO_fftruncate
#undef CONFIG_HAVE_fftruncate
#elif !defined(CONFIG_HAVE_fftruncate) && \
      (defined(fftruncate) || defined(__fftruncate_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_fftruncate 1
#endif

#ifdef CONFIG_NO_fftruncate64
#undef CONFIG_HAVE_fftruncate64
#elif !defined(CONFIG_HAVE_fftruncate64) && \
      (defined(fftruncate64) || defined(__fftruncate64_defined) || (defined(__USE_KOS) && \
       defined(__USE_LARGEFILE64)))
#define CONFIG_HAVE_fftruncate64 1
#endif

#ifdef CONFIG_NO_getc
#undef CONFIG_HAVE_getc
#elif !defined(CONFIG_HAVE_getc) && \
      (defined(getc) || defined(__getc_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_getc 1
#endif

#ifdef CONFIG_NO_fgetc
#undef CONFIG_HAVE_fgetc
#elif !defined(CONFIG_HAVE_fgetc) && \
      (defined(fgetc) || defined(__fgetc_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fgetc 1
#endif

#ifdef CONFIG_NO_putc
#undef CONFIG_HAVE_putc
#elif !defined(CONFIG_HAVE_putc) && \
      (defined(putc) || defined(__putc_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_putc 1
#endif

#ifdef CONFIG_NO_fputc
#undef CONFIG_HAVE_fputc
#elif !defined(CONFIG_HAVE_fputc) && \
      (defined(fputc) || defined(__fputc_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fputc 1
#endif

#ifdef CONFIG_NO_fread
#undef CONFIG_HAVE_fread
#elif !defined(CONFIG_HAVE_fread) && \
      (defined(fread) || defined(__fread_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fread 1
#endif

#ifdef CONFIG_NO_fwrite
#undef CONFIG_HAVE_fwrite
#elif !defined(CONFIG_HAVE_fwrite) && \
      (defined(fwrite) || defined(__fwrite_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fwrite 1
#endif

#ifdef CONFIG_NO_ungetc
#undef CONFIG_HAVE_ungetc
#elif !defined(CONFIG_HAVE_ungetc) && \
      (defined(ungetc) || defined(__ungetc_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_ungetc 1
#endif

#ifdef CONFIG_NO_setvbuf
#undef CONFIG_HAVE_setvbuf
#elif !defined(CONFIG_HAVE_setvbuf) && \
      (defined(setvbuf) || defined(__setvbuf_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_setvbuf 1
#endif

#ifdef CONFIG_NO_fopen
#undef CONFIG_HAVE_fopen
#elif !defined(CONFIG_HAVE_fopen) && \
      (defined(fopen) || defined(__fopen_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fopen 1
#endif

#ifdef CONFIG_NO_fopen64
#undef CONFIG_HAVE_fopen64
#elif !defined(CONFIG_HAVE_fopen64) && \
      (defined(fopen64) || defined(__fopen64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       defined(__USE_LARGEFILE64)))
#define CONFIG_HAVE_fopen64 1
#endif

#ifdef CONFIG_NO_fprintf
#undef CONFIG_HAVE_fprintf
#elif !defined(CONFIG_HAVE_fprintf) && \
      (defined(fprintf) || defined(__fprintf_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_fprintf 1
#endif

#ifdef CONFIG_NO_stdin
#undef CONFIG_HAVE_stdin
#elif !defined(CONFIG_HAVE_stdin) && \
      (defined(stdin) || defined(__stdin_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_stdin 1
#endif

#ifdef CONFIG_NO_stdout
#undef CONFIG_HAVE_stdout
#elif !defined(CONFIG_HAVE_stdout) && \
      (defined(stdout) || defined(__stdout_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_stdout 1
#endif

#ifdef CONFIG_NO_stderr
#undef CONFIG_HAVE_stderr
#elif !defined(CONFIG_HAVE_stderr) && \
      (defined(stderr) || defined(__stderr_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_stderr 1
#endif

#ifdef CONFIG_NO_sem_init
#undef CONFIG_HAVE_sem_init
#elif !defined(CONFIG_HAVE_sem_init) && \
      (defined(sem_init) || defined(__sem_init_defined) || defined(CONFIG_HAVE_SEMAPHORE_H))
#define CONFIG_HAVE_sem_init 1
#endif

#ifdef CONFIG_NO_sem_destroy
#undef CONFIG_HAVE_sem_destroy
#elif !defined(CONFIG_HAVE_sem_destroy) && \
      (defined(sem_destroy) || defined(__sem_destroy_defined) || defined(CONFIG_HAVE_SEMAPHORE_H))
#define CONFIG_HAVE_sem_destroy 1
#endif

#ifdef CONFIG_NO_sem_wait
#undef CONFIG_HAVE_sem_wait
#elif !defined(CONFIG_HAVE_sem_wait) && \
      (defined(sem_wait) || defined(__sem_wait_defined) || defined(CONFIG_HAVE_SEMAPHORE_H))
#define CONFIG_HAVE_sem_wait 1
#endif

#ifdef CONFIG_NO_sem_trywait
#undef CONFIG_HAVE_sem_trywait
#elif !defined(CONFIG_HAVE_sem_trywait) && \
      (defined(sem_trywait) || defined(__sem_trywait_defined) || defined(CONFIG_HAVE_SEMAPHORE_H))
#define CONFIG_HAVE_sem_trywait 1
#endif

#ifdef CONFIG_NO_sem_post
#undef CONFIG_HAVE_sem_post
#elif !defined(CONFIG_HAVE_sem_post) && \
      (defined(sem_post) || defined(__sem_post_defined) || defined(CONFIG_HAVE_SEMAPHORE_H))
#define CONFIG_HAVE_sem_post 1
#endif

#ifdef CONFIG_NO_sem_timedwait
#undef CONFIG_HAVE_sem_timedwait
#elif !defined(CONFIG_HAVE_sem_timedwait) && \
      (defined(sem_timedwait) || defined(__sem_timedwait_defined) || (defined(CONFIG_HAVE_SEMAPHORE_H) && \
       defined(__USE_XOPEN2K)))
#define CONFIG_HAVE_sem_timedwait 1
#endif

#ifdef CONFIG_NO_sem_timedwait64
#undef CONFIG_HAVE_sem_timedwait64
#elif !defined(CONFIG_HAVE_sem_timedwait64) && \
      (defined(sem_timedwait64) || defined(__sem_timedwait64_defined) || (defined(CONFIG_HAVE_SEMAPHORE_H) && \
       defined(__USE_XOPEN2K) && defined(__USE_TIME64)))
#define CONFIG_HAVE_sem_timedwait64 1
#endif

#ifdef CONFIG_NO_pthread_suspend
#undef CONFIG_HAVE_pthread_suspend
#elif !defined(CONFIG_HAVE_pthread_suspend) && \
      (defined(pthread_suspend) || defined(__pthread_suspend_defined) || (defined(CONFIG_HAVE_PTHREAD_H) && \
       0))
#define CONFIG_HAVE_pthread_suspend 1
#endif

#ifdef CONFIG_NO_pthread_continue
#undef CONFIG_HAVE_pthread_continue
#elif !defined(CONFIG_HAVE_pthread_continue) && \
      (defined(pthread_continue) || defined(__pthread_continue_defined) || (defined(CONFIG_HAVE_PTHREAD_H) && \
       0))
#define CONFIG_HAVE_pthread_continue 1
#endif

#ifdef CONFIG_NO_pthread_suspend_np
#undef CONFIG_HAVE_pthread_suspend_np
#elif !defined(CONFIG_HAVE_pthread_suspend_np) && \
      (defined(pthread_suspend_np) || defined(__pthread_suspend_np_defined) || \
       (defined(CONFIG_HAVE_PTHREAD_H) && 0))
#define CONFIG_HAVE_pthread_suspend_np 1
#endif

#ifdef CONFIG_NO_pthread_unsuspend_np
#undef CONFIG_HAVE_pthread_unsuspend_np
#elif !defined(CONFIG_HAVE_pthread_unsuspend_np) && \
      (defined(pthread_unsuspend_np) || defined(__pthread_unsuspend_np_defined) || \
       (defined(CONFIG_HAVE_PTHREAD_H) && 0))
#define CONFIG_HAVE_pthread_unsuspend_np 1
#endif

#ifdef CONFIG_NO_pthread_setname
#undef CONFIG_HAVE_pthread_setname
#elif !defined(CONFIG_HAVE_pthread_setname) && \
      (defined(pthread_setname) || defined(__pthread_setname_defined) || (defined(CONFIG_HAVE_PTHREAD_H) && \
       0))
#define CONFIG_HAVE_pthread_setname 1
#endif

#ifdef CONFIG_NO_pthread_setname_np
#undef CONFIG_HAVE_pthread_setname_np
#elif !defined(CONFIG_HAVE_pthread_setname_np) && \
      (defined(pthread_setname_np) || defined(__pthread_setname_np_defined) || \
       (defined(CONFIG_HAVE_PTHREAD_H) && defined(__USE_GNU)))
#define CONFIG_HAVE_pthread_setname_np 1
#endif

#ifdef CONFIG_NO_abort
#undef CONFIG_HAVE_abort
#elif !defined(CONFIG_HAVE_abort) && \
      (defined(abort) || defined(__abort_defined) || defined(CONFIG_HAVE_STDLIB_H))
#define CONFIG_HAVE_abort 1
#endif

#ifdef CONFIG_NO_strerror
#undef CONFIG_HAVE_strerror
#elif !defined(CONFIG_HAVE_strerror) && \
      (defined(strerror) || defined(__strerror_defined) || defined(CONFIG_HAVE_STRING_H))
#define CONFIG_HAVE_strerror 1
#endif

#ifdef CONFIG_NO_dlopen
#undef CONFIG_HAVE_dlopen
#elif !defined(CONFIG_HAVE_dlopen) && \
      (defined(dlopen) || defined(__dlopen_defined) || defined(CONFIG_HAVE_DLFCN_H))
#define CONFIG_HAVE_dlopen 1
#endif

#ifdef CONFIG_NO_dlclose
#undef CONFIG_HAVE_dlclose
#elif !defined(CONFIG_HAVE_dlclose) && \
      (defined(dlclose) || defined(__dlclose_defined) || defined(CONFIG_HAVE_DLFCN_H))
#define CONFIG_HAVE_dlclose 1
#endif

#ifdef CONFIG_NO_dlsym
#undef CONFIG_HAVE_dlsym
#elif !defined(CONFIG_HAVE_dlsym) && \
      (defined(dlsym) || defined(__dlsym_defined) || defined(CONFIG_HAVE_DLFCN_H))
#define CONFIG_HAVE_dlsym 1
#endif

#ifdef CONFIG_NO__memicmp
#undef CONFIG_HAVE__memicmp
#elif !defined(CONFIG_HAVE__memicmp) && \
      (defined(_memicmp) || defined(___memicmp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__memicmp 1
#endif

#ifdef CONFIG_NO_memcasecmp
#undef CONFIG_HAVE_memcasecmp
#elif !defined(CONFIG_HAVE_memcasecmp) && \
      (defined(memcasecmp) || defined(__memcasecmp_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcasecmp 1
#endif

#ifdef CONFIG_NO_memrchr
#undef CONFIG_HAVE_memrchr
#elif !defined(CONFIG_HAVE_memrchr) && \
      (defined(memrchr) || defined(__memrchr_defined) || defined(__USE_GNU))
#define CONFIG_HAVE_memrchr 1
#endif

#ifdef CONFIG_NO_rawmemchr
#undef CONFIG_HAVE_rawmemchr
#elif !defined(CONFIG_HAVE_rawmemchr) && \
      (defined(rawmemchr) || defined(__rawmemchr_defined) || defined(__USE_GNU))
#define CONFIG_HAVE_rawmemchr 1
#endif

#ifdef CONFIG_NO_strnlen
#undef CONFIG_HAVE_strnlen
#elif !defined(CONFIG_HAVE_strnlen) && \
      (defined(strnlen) || defined(__strnlen_defined) || (defined(__USE_XOPEN2K8) || \
       defined(__USE_DOS) || (defined(_MSC_VER) && !defined(__KOS_SYSTEM_HEADERS__))))
#define CONFIG_HAVE_strnlen 1
#endif

#ifdef CONFIG_NO_memmem
#undef CONFIG_HAVE_memmem
#elif !defined(CONFIG_HAVE_memmem) && \
      (defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL))
#define CONFIG_HAVE_memmem 1
#endif

#ifdef CONFIG_NO_memrmem
#undef CONFIG_HAVE_memrmem
#elif !defined(CONFIG_HAVE_memrmem) && \
      (defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL))
#define CONFIG_HAVE_memrmem 1
#endif

#ifdef CONFIG_NO_rawmemrchr
#undef CONFIG_HAVE_rawmemrchr
#elif !defined(CONFIG_HAVE_rawmemrchr) && \
      (defined(rawmemrchr) || defined(__rawmemrchr_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_rawmemrchr 1
#endif

#ifdef CONFIG_NO_memend
#undef CONFIG_HAVE_memend
#elif !defined(CONFIG_HAVE_memend) && \
      (defined(memend) || defined(__memend_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memend 1
#endif

#ifdef CONFIG_NO_memrend
#undef CONFIG_HAVE_memrend
#elif !defined(CONFIG_HAVE_memrend) && \
      (defined(memrend) || defined(__memrend_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrend 1
#endif

#ifdef CONFIG_NO_memlen
#undef CONFIG_HAVE_memlen
#elif !defined(CONFIG_HAVE_memlen) && \
      (defined(memlen) || defined(__memlen_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memlen 1
#endif

#ifdef CONFIG_NO_memrlen
#undef CONFIG_HAVE_memrlen
#elif !defined(CONFIG_HAVE_memrlen) && \
      (defined(memrlen) || defined(__memrlen_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrlen 1
#endif

#ifdef CONFIG_NO_rawmemlen
#undef CONFIG_HAVE_rawmemlen
#elif !defined(CONFIG_HAVE_rawmemlen) && \
      (defined(rawmemlen) || defined(__rawmemlen_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_rawmemlen 1
#endif

#ifdef CONFIG_NO_rawmemrlen
#undef CONFIG_HAVE_rawmemrlen
#elif !defined(CONFIG_HAVE_rawmemrlen) && \
      (defined(rawmemrlen) || defined(__rawmemrlen_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_rawmemrlen 1
#endif

#ifdef CONFIG_NO_memcasemem
#undef CONFIG_HAVE_memcasemem
#elif !defined(CONFIG_HAVE_memcasemem) && \
      (defined(memcasemem) || defined(__memcasemem_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcasemem 1
#endif

#ifdef CONFIG_NO_memrev
#undef CONFIG_HAVE_memrev
#elif !defined(CONFIG_HAVE_memrev) && \
      (defined(memrev) || defined(__memrev_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrev 1
#endif

#ifdef CONFIG_NO_memcasermem
#undef CONFIG_HAVE_memcasermem
#elif 0
#define CONFIG_HAVE_memcasermem 1
#endif

#ifdef CONFIG_NO_strend
#undef CONFIG_HAVE_strend
#elif !defined(CONFIG_HAVE_strend) && \
      (defined(strend) || defined(__strend_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_strend 1
#endif

#ifdef CONFIG_NO_strnend
#undef CONFIG_HAVE_strnend
#elif !defined(CONFIG_HAVE_strnend) && \
      (defined(strnend) || defined(__strnend_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_strnend 1
#endif

#ifdef CONFIG_NO_memxend
#undef CONFIG_HAVE_memxend
#elif !defined(CONFIG_HAVE_memxend) && \
      (defined(memxend) || defined(__memxend_defined) || defined(__USE_STRING_XCHR))
#define CONFIG_HAVE_memxend 1
#endif

#ifdef CONFIG_NO_memxlen
#undef CONFIG_HAVE_memxlen
#elif !defined(CONFIG_HAVE_memxlen) && \
      (defined(memxlen) || defined(__memxlen_defined) || defined(__USE_STRING_XCHR))
#define CONFIG_HAVE_memxlen 1
#endif

#ifdef CONFIG_NO_memxchr
#undef CONFIG_HAVE_memxchr
#elif !defined(CONFIG_HAVE_memxchr) && \
      (defined(memxchr) || defined(__memxchr_defined) || defined(__USE_STRING_XCHR))
#define CONFIG_HAVE_memxchr 1
#endif

#ifdef CONFIG_NO_rawmemxchr
#undef CONFIG_HAVE_rawmemxchr
#elif !defined(CONFIG_HAVE_rawmemxchr) && \
      (defined(rawmemxchr) || defined(__rawmemxchr_defined) || defined(__USE_STRING_XCHR))
#define CONFIG_HAVE_rawmemxchr 1
#endif

#ifdef CONFIG_NO_rawmemxlen
#undef CONFIG_HAVE_rawmemxlen
#elif !defined(CONFIG_HAVE_rawmemxlen) && \
      (defined(rawmemxlen) || defined(__rawmemxlen_defined) || defined(__USE_STRING_XCHR))
#define CONFIG_HAVE_rawmemxlen 1
#endif

#ifdef CONFIG_NO_memxrchr
#undef CONFIG_HAVE_memxrchr
#elif !defined(CONFIG_HAVE_memxrchr) && \
      (defined(memxrchr) || defined(__memxrchr_defined))
#define CONFIG_HAVE_memxrchr 1
#endif

#ifdef CONFIG_NO_memxrend
#undef CONFIG_HAVE_memxrend
#elif !defined(CONFIG_HAVE_memxrend) && \
      (defined(memxrend) || defined(__memxrend_defined))
#define CONFIG_HAVE_memxrend 1
#endif

#ifdef CONFIG_NO_memxrlen
#undef CONFIG_HAVE_memxrlen
#elif !defined(CONFIG_HAVE_memxrlen) && \
      (defined(memxrlen) || defined(__memxrlen_defined))
#define CONFIG_HAVE_memxrlen 1
#endif

#ifdef CONFIG_NO_rawmemxrchr
#undef CONFIG_HAVE_rawmemxrchr
#elif !defined(CONFIG_HAVE_rawmemxrchr) && \
      (defined(rawmemxrchr) || defined(__rawmemxrchr_defined))
#define CONFIG_HAVE_rawmemxrchr 1
#endif

#ifdef CONFIG_NO_rawmemxrlen
#undef CONFIG_HAVE_rawmemxrlen
#elif !defined(CONFIG_HAVE_rawmemxrlen) && \
      (defined(rawmemxrlen) || defined(__rawmemxrlen_defined))
#define CONFIG_HAVE_rawmemxrlen 1
#endif

#ifdef CONFIG_NO_tolower
#undef CONFIG_HAVE_tolower
#elif !defined(CONFIG_HAVE_tolower) && \
      (defined(tolower) || defined(__tolower_defined) || defined(CONFIG_HAVE_CTYPE_H))
#define CONFIG_HAVE_tolower 1
#endif

#ifdef CONFIG_NO_toupper
#undef CONFIG_HAVE_toupper
#elif !defined(CONFIG_HAVE_toupper) && \
      (defined(toupper) || defined(__toupper_defined) || defined(CONFIG_HAVE_CTYPE_H))
#define CONFIG_HAVE_toupper 1
#endif

#ifdef CONFIG_NO__dosmaperr
#undef CONFIG_HAVE__dosmaperr
#elif !defined(CONFIG_HAVE__dosmaperr) && \
      (defined(_MSC_VER) || (defined(__CRT_DOS) && defined(__CRT_HAVE__dosmaperr)))
#define CONFIG_HAVE__dosmaperr 1
#endif

#ifdef CONFIG_NO_errno_nt2kos
#undef CONFIG_HAVE_errno_nt2kos
#elif !defined(CONFIG_HAVE_errno_nt2kos) && \
      (defined(__CRT_HAVE_errno_nt2kos))
#define CONFIG_HAVE_errno_nt2kos 1
#endif

#ifdef CONFIG_NO__get_osfhandle
#undef CONFIG_HAVE__get_osfhandle
#elif !defined(CONFIG_HAVE__get_osfhandle) && \
      (defined(_get_osfhandle) || defined(___get_osfhandle_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__get_osfhandle 1
#endif

#ifdef CONFIG_NO__open_osfhandle
#undef CONFIG_HAVE__open_osfhandle
#elif !defined(CONFIG_HAVE__open_osfhandle) && \
      (defined(_open_osfhandle) || defined(___open_osfhandle_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__open_osfhandle 1
#endif

#ifdef CONFIG_NO__doserrno
#undef CONFIG_HAVE__doserrno
#elif !defined(CONFIG_HAVE__doserrno) && \
      (defined(_doserrno) || defined(___doserrno_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__doserrno 1
#endif
//[[[end]]]



/* Substitute some known function aliases */
#if defined(CONFIG_HAVE__exit) && !defined(CONFIG_HAVE__Exit)
#define CONFIG_HAVE__Exit 1
#define _Exit(exit_code) _exit(exit_code)
#endif /* _Exit = _exit */

#if defined(CONFIG_HAVE__execv) && !defined(CONFIG_HAVE_execv)
#define CONFIG_HAVE_execv 1
#define execv _execv
#endif /* execv = _execv */

#if defined(CONFIG_HAVE__execve) && !defined(CONFIG_HAVE_execve)
#define CONFIG_HAVE_execve 1
#define execve _execve
#endif /* execve = _execve */

#if defined(CONFIG_HAVE__execvp) && !defined(CONFIG_HAVE_execvp)
#define CONFIG_HAVE_execvp 1
#define execvp _execvp
#endif /* execvp = _execvp */

#if defined(CONFIG_HAVE__execvpe) && !defined(CONFIG_HAVE_execvpe)
#define CONFIG_HAVE_execvpe 1
#define execvpe _execvpe
#endif /* execvpe = _execvpe */

#if defined(CONFIG_HAVE__wexecv) && !defined(CONFIG_HAVE_wexecv)
#define CONFIG_HAVE_wexecv 1
#define wexecv _wexecv
#endif /* wexecv = _wexecv */

#if defined(CONFIG_HAVE__wexecve) && !defined(CONFIG_HAVE_wexecve)
#define CONFIG_HAVE_wexecve 1
#define wexecve _wexecve
#endif /* wexecve = _wexecve */

#if defined(CONFIG_HAVE__wexecvp) && !defined(CONFIG_HAVE_wexecvp)
#define CONFIG_HAVE_wexecvp 1
#define wexecvp _wexecvp
#endif /* wexecvp = _wexecvp */

#if defined(CONFIG_HAVE__wexecvpe) && !defined(CONFIG_HAVE_wexecvpe)
#define CONFIG_HAVE_wexecvpe 1
#define wexecvpe _wexecvpe
#endif /* wexecvpe = _wexecvpe */

#if defined(CONFIG_HAVE__spawnv) && !defined(CONFIG_HAVE_spawnv)
#define CONFIG_HAVE_spawnv 1
#define spawnv _spawnv
#endif /* spawnv = _spawnv */

#if defined(CONFIG_HAVE__spawnve) && !defined(CONFIG_HAVE_spawnve)
#define CONFIG_HAVE_spawnve 1
#define spawnve _spawnve
#endif /* spawnve = _spawnve */

#if defined(CONFIG_HAVE__spawnvp) && !defined(CONFIG_HAVE_spawnvp)
#define CONFIG_HAVE_spawnvp 1
#define spawnvp _spawnvp
#endif /* spawnvp = _spawnvp */

#if defined(CONFIG_HAVE__spawnvpe) && !defined(CONFIG_HAVE_spawnvpe)
#define CONFIG_HAVE_spawnvpe 1
#define spawnvpe _spawnvpe
#endif /* spawnvpe = _spawnvpe */

#if defined(CONFIG_HAVE__wspawnv) && !defined(CONFIG_HAVE_wspawnv)
#define CONFIG_HAVE_wspawnv 1
#define wspawnv _wspawnv
#endif /* wspawnv = _wspawnv */

#if defined(CONFIG_HAVE__wspawnve) && !defined(CONFIG_HAVE_wspawnve)
#define CONFIG_HAVE_wspawnve 1
#define wspawnve _wspawnve
#endif /* wspawnve = _wspawnve */

#if defined(CONFIG_HAVE__wspawnvp) && !defined(CONFIG_HAVE_wspawnvp)
#define CONFIG_HAVE_wspawnvp 1
#define wspawnvp _wspawnvp
#endif /* wspawnvp = _wspawnvp */

#if defined(CONFIG_HAVE__wspawnvpe) && !defined(CONFIG_HAVE_wspawnvpe)
#define CONFIG_HAVE_wspawnvpe 1
#define wspawnvpe _wspawnvpe
#endif /* wspawnvpe = _wspawnvpe */

#if defined(CONFIG_HAVE__cwait) && !defined(CONFIG_HAVE_cwait)
#define CONFIG_HAVE_cwait 1
#define cwait _cwait
#endif /* cwait = _cwait */

#if defined(CONFIG_HAVE__wsystem) && !defined(CONFIG_HAVE_wsystem)
#define CONFIG_HAVE_wsystem 1
#define wsystem _wsystem
#endif /* wsystem = _wsystem */

#if defined(CONFIG_HAVE__open) && !defined(CONFIG_HAVE_open)
#define CONFIG_HAVE_open 1
#define open _open
#endif /* open = _open */

#if defined(CONFIG_HAVE__creat) && !defined(CONFIG_HAVE_creat)
#define CONFIG_HAVE_creat 1
#define creat _creat
#endif /* creat = _creat */

#if defined(CONFIG_HAVE__chmod) && !defined(CONFIG_HAVE_chmod)
#define CONFIG_HAVE_chmod 1
#define chmod _chmod
#endif /* chmod = _chmod */

#if defined(CONFIG_HAVE__unlink) && !defined(CONFIG_HAVE_unlink)
#define CONFIG_HAVE_unlink 1
#define unlink _unlink
#endif /* unlink = _unlink */

#if defined(CONFIG_HAVE__wunlink) && !defined(CONFIG_HAVE_wunlink)
#define CONFIG_HAVE_wunlink 1
#define wunlink _wunlink
#endif /* wunlink = _wunlink */

#if defined(CONFIG_HAVE__remove) && !defined(CONFIG_HAVE_remove)
#define CONFIG_HAVE_remove 1
#define remove _remove
#endif /* remove = _remove */

#if defined(CONFIG_HAVE__wremove) && !defined(CONFIG_HAVE_wremove)
#define CONFIG_HAVE_wremove 1
#define wremove _wremove
#endif /* wremove = _wremove */

#if defined(CONFIG_HAVE__rename) && !defined(CONFIG_HAVE_rename)
#define CONFIG_HAVE_rename 1
#define rename _rename
#endif /* rename = _rename */

#if defined(CONFIG_HAVE__wrename) && !defined(CONFIG_HAVE_wrename)
#define CONFIG_HAVE_wrename 1
#define wrename _wrename
#endif /* wrename = _wrename */

#if defined(CONFIG_HAVE__wopen) && !defined(CONFIG_HAVE_wopen)
#define CONFIG_HAVE_wopen 1
#define wopen _wopen
#endif /* wopen = _wopen */

#if defined(CONFIG_HAVE__read) && !defined(CONFIG_HAVE_read)
#define CONFIG_HAVE_read 1
#define read(fd, buf, bufsize) ((Dee_ssize_t)_read(fd, buf, (unsigned int)(bufsize)))
#endif /* read = _read */

#if defined(CONFIG_HAVE__write) && !defined(CONFIG_HAVE_write)
#define CONFIG_HAVE_write 1
#define write(fd, buf, bufsize) ((Dee_ssize_t)_write(fd, buf, (unsigned int)(bufsize)))
#endif /* write = _write */

#if defined(CONFIG_HAVE__lseek) && !defined(CONFIG_HAVE_lseek)
#define CONFIG_HAVE_lseek 1
#define lseek _lseek
#endif /* lseek = _lseek */

#if defined(CONFIG_HAVE__lseeki64) && !defined(CONFIG_HAVE_lseek64)
#define CONFIG_HAVE_lseek64 1
#define lseek64 _lseeki64
#endif /* lseek64 = _lseeki64 */

#if defined(CONFIG_HAVE__close) && !defined(CONFIG_HAVE_close)
#define CONFIG_HAVE_close 1
#define close _close
#endif /* close = _close */

#if defined(CONFIG_HAVE__commit) && !defined(CONFIG_HAVE_fsync)
#define CONFIG_HAVE_fsync 1
#define fsync _commit
#endif /* fsync = _commit */

#if defined(CONFIG_HAVE__commit) && !defined(CONFIG_HAVE_fdatasync)
#define CONFIG_HAVE_fdatasync 1
#define fdatasync _commit
#endif /* fdatasync = _commit */

#if defined(CONFIG_HAVE__chsize) && !defined(CONFIG_HAVE_ftruncate)
#define CONFIG_HAVE_ftruncate 1
#define ftruncate _chsize
#endif /* ftruncate = _chsize */

#if defined(CONFIG_HAVE__chsize_s) && !defined(CONFIG_HAVE_ftruncate64)
#define CONFIG_HAVE_ftruncate64 1
#define ftruncate64 _chsize_s
#endif /* ftruncate64 = _chsize_s */

#if defined(CONFIG_HAVE__chdir) && !defined(CONFIG_HAVE_chdir)
#define CONFIG_HAVE_chdir 1
#define chdir _chdir
#endif /* chdir = _chdir */

#if defined(CONFIG_HAVE__getpid) && !defined(CONFIG_HAVE_getpid)
#define CONFIG_HAVE_getpid 1
#define getpid _getpid
#endif /* getpid = _getpid */

#if defined(CONFIG_HAVE__umask) && !defined(CONFIG_HAVE_umask)
#define CONFIG_HAVE_umask 1
#define umask _umask
#endif /* umask = _umask */

#if defined(CONFIG_HAVE__dup) && !defined(CONFIG_HAVE_dup)
#define CONFIG_HAVE_dup 1
#define dup _dup
#endif /* dup = _dup */

#if defined(CONFIG_HAVE__dup2) && !defined(CONFIG_HAVE_dup2)
#define CONFIG_HAVE_dup2 1
#define dup2 _dup2
#endif /* dup2 = _dup2 */

#if defined(CONFIG_HAVE__isatty) && !defined(CONFIG_HAVE_isatty)
#define CONFIG_HAVE_isatty 1
#define isatty _isatty
#endif /* isatty = _isatty */

#if defined(CONFIG_HAVE__getcwd) && !defined(CONFIG_HAVE_getcwd)
#define CONFIG_HAVE_getcwd 1
#define getcwd _getcwd
#endif /* getcwd = _getcwd */

#if defined(CONFIG_HAVE__wgetcwd) && !defined(CONFIG_HAVE_wgetcwd)
#define CONFIG_HAVE_wgetcwd 1
#define wgetcwd _wgetcwd
#endif /* wgetcwd = _wgetcwd */

#if defined(CONFIG_HAVE__access) && !defined(CONFIG_HAVE_access)
#define CONFIG_HAVE_access 1
#define access _access
#endif /* access = _access */

#if defined(CONFIG_HAVE__waccess) && !defined(CONFIG_HAVE_waccess)
#define CONFIG_HAVE_waccess 1
#define waccess _waccess
#endif /* waccess = _waccess */

#if defined(CONFIG_HAVE_euidaccess) && !defined(CONFIG_HAVE_eaccess)
#define CONFIG_HAVE_eaccess 1
#define eaccess euidaccess
#endif /* eaccess = euidaccess */

#if defined(CONFIG_HAVE_eaccess) && !defined(CONFIG_HAVE_euidaccess)
#define CONFIG_HAVE_euidaccess 1
#define euidaccess eaccess
#endif /* euidaccess = eaccess */

#if defined(CONFIG_HAVE_vfork) && !defined(CONFIG_HAVE_fork)
#define CONFIG_HAVE_fork 1
#define fork vfork
#endif /* fork = vfork */

#if defined(CONFIG_HAVE__pipe) && !defined(CONFIG_HAVE_pipe)
#define CONFIG_HAVE_pipe 1
#define pipe(fds) _pipe(fds, 4096, O_BINARY)
#endif /* pipe = _pipe */

#if defined(CONFIG_HAVE__mkdir) && !defined(CONFIG_HAVE_mkdir)
#define CONFIG_HAVE_mkdir 1
#define mkdir(name, mode) _mkdir(name)
#endif /* mkdir = _mkdir */

#if defined(CONFIG_HAVE__fseeki64) && !defined(CONFIG_HAVE_fseeko64)
#define CONFIG_HAVE_fseeko64 1
#define fseeko64 _fseeki64
#endif /* fseeko64 = _fseeki64 */

#if defined(CONFIG_HAVE__ftelli64) && !defined(CONFIG_HAVE_ftello64)
#define CONFIG_HAVE_ftello64 1
#define ftello64 _ftelli64
#endif /* ftello64 = _ftelli64 */

#if defined(CONFIG_HAVE__ftelli64) && !defined(CONFIG_HAVE_ftello64)
#define CONFIG_HAVE_ftello64 1
#define ftello64 _ftelli64
#endif /* ftello64 = _ftelli64 */

/* Make sure that we've got both fseeko64() and ftello64(), or that we have neither! */
#if (defined(CONFIG_HAVE_ftell) && !defined(CONFIG_HAVE_fseek)) || \
    (!defined(CONFIG_HAVE_ftell) && defined(CONFIG_HAVE_fseek))
#undef CONFIG_HAVE_ftell
#undef CONFIG_HAVE_fseek
#endif /* Not both */
#if (defined(CONFIG_HAVE_ftello) && !defined(CONFIG_HAVE_fseeko)) || \
    (!defined(CONFIG_HAVE_ftello) && defined(CONFIG_HAVE_fseeko))
#undef CONFIG_HAVE_ftello
#undef CONFIG_HAVE_fseeko
#endif /* Not both */
#if (defined(CONFIG_HAVE_ftello64) && !defined(CONFIG_HAVE_fseeko64)) || \
    (!defined(CONFIG_HAVE_ftello64) && defined(CONFIG_HAVE_fseeko64))
#undef CONFIG_HAVE_ftello64
#undef CONFIG_HAVE_fseeko64
#endif /* Not both */

#ifndef CONFIG_SIZEOF_OFF_T
#define CONFIG_SIZEOF_OFF_T __SIZEOF_POINTER__
#endif /* !CONFIG_SIZEOF_OFF_T */

#if (defined(CONFIG_HAVE_fseek) && !defined(CONFIG_HAVE_fseeko)) && \
    (CONFIG_SIZEOF_OFF_T <= __SIZEOF_LONG__)
#define CONFIG_HAVE_fseeko 1
#define CONFIG_HAVE_ftello 1
#define fseeko fseek
#define ftello ftell
#endif /* fseeko = fseek */

#if defined(CONFIG_HAVE_fseeko64) && !defined(CONFIG_HAVE_fseeko)
#define CONFIG_HAVE_fseeko 1
#define CONFIG_HAVE_ftello 1
#define fseeko fseeko64
#define ftello ftello64
#endif /* fseeko = fseeko64 */

#if !defined(CONFIG_HAVE_fseek) && defined(CONFIG_HAVE_fseeko)
#define CONFIG_HAVE_fseek 1
#define CONFIG_HAVE_ftell 1
#define fseek fseeko
#define ftell (long int)ftello
#endif /* fseeko = fseeko64 */

#if defined(CONFIG_HAVE__fileno) && !defined(CONFIG_HAVE_fileno)
#define CONFIG_HAVE_fileno 1
#define fileno _fileno
#endif /* fileno = _fileno */

#if defined(CONFIG_HAVE_getc) && !defined(CONFIG_HAVE_fgetc)
#define CONFIG_HAVE_fgetc 1
#define fgetc getc
#endif /* fgetc = getc */
#if defined(CONFIG_HAVE_fgetc) && !defined(CONFIG_HAVE_getc)
#define CONFIG_HAVE_getc 1
#define getc fgetc
#endif /* getc = fgetc */
#if defined(CONFIG_HAVE_putc) && !defined(CONFIG_HAVE_fputc)
#define CONFIG_HAVE_fputc 1
#define fputc putc
#endif /* fputc = putc */
#if defined(CONFIG_HAVE_fputc) && !defined(CONFIG_HAVE_putc)
#define CONFIG_HAVE_putc 1
#define putc fputc
#endif /* putc = fputc */

#if defined(CONFIG_HAVE_MAP_ANON) && !defined(CONFIG_HAVE_MAP_ANONYMOUS)
#define CONFIG_HAVE_MAP_ANONYMOUS 1
#define MAP_ANONYMOUS MAP_ANON
#endif /* MAP_ANONYMOUS = MAP_ANON */

#if defined(CONFIG_HAVE_MAP_ANONYMOUS) && !defined(CONFIG_HAVE_MAP_ANON)
#define CONFIG_HAVE_MAP_ANON 1
#define MAP_ANON MAP_ANONYMOUS
#endif /* MAP_ANON = MAP_ANONYMOUS */



/* Configure O_* flags for `open()' */

/* Set optional flags to no-ops */
#ifndef O_BINARY
#ifdef __O_BINARY
#define O_BINARY __O_BINARY
#elif defined(_O_BINARY)
#define O_BINARY _O_BINARY
#elif defined(__O_RAW)
#define O_BINARY __O_RAW
#elif defined(_O_RAW)
#define O_BINARY _O_RAW
#elif defined(O_RAW)
#define O_BINARY O_RAW
#else
#define O_BINARY 0
#endif
#endif /* !O_BINARY */

#ifndef O_SHORT_LIVED
#ifdef __O_SHORT_LIVED
#define O_SHORT_LIVED __O_SHORT_LIVED
#elif defined(_O_SHORT_LIVED)
#define O_SHORT_LIVED _O_SHORT_LIVED
#else
#define O_SHORT_LIVED 0
#endif
#endif /* !O_SHORT_LIVED */

#ifndef O_SEQUENTIAL
#ifdef __O_SEQUENTIAL
#define O_SEQUENTIAL __O_SEQUENTIAL
#elif defined(_O_SEQUENTIAL)
#define O_SEQUENTIAL _O_SEQUENTIAL
#else
#define O_SEQUENTIAL 0
#endif
#endif /* !O_SEQUENTIAL */

#ifndef O_RANDOM
#ifdef __O_RANDOM
#define O_RANDOM __O_RANDOM
#elif defined(_O_RANDOM)
#define O_RANDOM _O_RANDOM
#else
#define O_RANDOM 0
#endif
#endif /* !O_RANDOM */

#ifndef O_PATH
#ifdef __O_PATH
#define O_PATH __O_PATH
#elif defined(_O_PATH)
#define O_PATH _O_PATH
#else
#define O_PATH 0
#endif
#endif /* !O_PATH */

#ifndef O_NOATIME
#ifdef __O_NOATIME
#define O_NOATIME __O_NOATIME
#elif defined(_O_NOATIME)
#define O_NOATIME _O_NOATIME
#else
#define O_NOATIME 0
#endif
#endif /* !O_NOATIME */

#ifndef O_NOCTTY
#ifdef __O_NOCTTY
#define O_NOCTTY __O_NOCTTY
#elif defined(_O_NOCTTY)
#define O_NOCTTY _O_NOCTTY
#else
#define O_NOCTTY 0
#endif
#endif /* !O_NOCTTY */


#ifndef O_TEXT
#ifdef __O_TEXT
#define O_TEXT __O_TEXT
#elif defined(_O_TEXT)
#define O_TEXT _O_TEXT
#endif
#endif /* !O_TEXT */

#ifndef O_WTEXT
#ifdef __O_WTEXT
#define O_WTEXT __O_WTEXT
#elif defined(_O_WTEXT)
#define O_WTEXT _O_WTEXT
#endif
#endif /* !O_WTEXT */

#ifndef O_U16TEXT
#ifdef __O_U16TEXT
#define O_U16TEXT __O_U16TEXT
#elif defined(_O_U16TEXT)
#define O_U16TEXT _O_U16TEXT
#endif
#endif /* !O_U16TEXT */

#ifndef O_U8TEXT
#ifdef __O_U8TEXT
#define O_U8TEXT __O_U8TEXT
#elif defined(_O_U8TEXT)
#define O_U8TEXT _O_U8TEXT
#endif
#endif /* !O_U8TEXT */

#ifndef O_TEMPORARY
#ifdef __O_TEMPORARY
#define O_TEMPORARY __O_TEMPORARY
#elif defined(_O_TEMPORARY)
#define O_TEMPORARY _O_TEMPORARY
#endif
#endif /* !O_TEMPORARY */

#ifndef O_OBTAIN_DIR
#ifdef __O_OBTAIN_DIR
#define O_OBTAIN_DIR __O_OBTAIN_DIR
#elif defined(_O_OBTAIN_DIR)
#define O_OBTAIN_DIR _O_OBTAIN_DIR
#endif
#endif /* !O_OBTAIN_DIR */

#ifndef O_CREAT
#ifdef __O_CREAT
#define O_CREAT __O_CREAT
#elif defined(_O_CREAT)
#define O_CREAT _O_CREAT
#endif
#endif /* !O_CREAT */

#ifndef O_TRUNC
#ifdef __O_TRUNC
#define O_TRUNC __O_TRUNC
#elif defined(_O_TRUNC)
#define O_TRUNC _O_TRUNC
#endif
#endif /* !O_TRUNC */

#ifndef O_RDONLY
#ifdef __O_RDONLY
#define O_RDONLY __O_RDONLY
#elif defined(_O_RDONLY)
#define O_RDONLY _O_RDONLY
#endif
#endif /* !O_RDONLY */

#ifndef O_WRONLY
#ifdef __O_WRONLY
#define O_WRONLY __O_WRONLY
#elif defined(_O_WRONLY)
#define O_WRONLY _O_WRONLY
#endif
#endif /* !O_WRONLY */

#ifndef O_RDWR
#ifdef __O_RDWR
#define O_RDWR __O_RDWR
#elif defined(_O_RDWR)
#define O_RDWR _O_RDWR
#endif
#endif /* !O_RDWR */

#ifndef O_ACCMODE
#ifdef __O_ACCMODE
#define O_ACCMODE __O_ACCMODE
#elif defined(_O_ACCMODE)
#define O_ACCMODE _O_ACCMODE
#elif 0 /* The combination might overlap with something else... */
#define O_ACCMODE (O_RDONLY|O_WRONLY|O_RDWR)
#endif
#endif /* !O_ACCMODE */

#ifndef O_CLOEXEC
#ifdef __O_NOINHERIT
#define O_CLOEXEC __O_NOINHERIT
#elif defined(_O_NOINHERIT)
#define O_CLOEXEC _O_NOINHERIT
#elif defined(O_NOINHERIT)
#define O_CLOEXEC O_NOINHERIT
#elif defined(__O_CLOEXEC)
#define O_CLOEXEC __O_CLOEXEC
#elif defined(_O_CLOEXEC)
#define O_CLOEXEC _O_CLOEXEC
#endif
#endif /* !O_CLOEXEC */

#ifndef O_EXCL
#ifdef __O_EXCL
#define O_EXCL __O_EXCL
#elif defined(_O_EXCL)
#define O_EXCL _O_EXCL
#endif
#endif /* !O_EXCL */

#ifndef O_APPEND
#ifdef __O_APPEND
#define O_APPEND __O_APPEND
#elif defined(_O_APPEND)
#define O_APPEND _O_APPEND
#endif
#endif /* !O_APPEND */

#ifndef O_NONBLOCK
#ifdef __O_NONBLOCK
#define O_NONBLOCK __O_NONBLOCK
#elif defined(_O_NONBLOCK)
#define O_NONBLOCK _O_NONBLOCK
#elif defined(__O_NDELAY)
#define O_NONBLOCK __O_NDELAY
#elif defined(_O_NDELAY)
#define O_NONBLOCK _O_NDELAY
#elif defined(O_NDELAY)
#define O_NONBLOCK O_NDELAY
#endif
#endif /* !O_NONBLOCK */

#ifndef O_RSYNC
#ifdef __O_RSYNC
#define O_RSYNC __O_RSYNC
#elif defined(_O_RSYNC)
#define O_RSYNC _O_RSYNC
#endif
#endif /* !O_RSYNC */

#ifndef O_SYNC
#ifdef __O_SYNC
#define O_SYNC __O_SYNC
#elif defined(_O_SYNC)
#define O_SYNC _O_SYNC
#endif
#endif /* !O_SYNC */

#ifndef O_DSYNC
#ifdef __O_DSYNC
#define O_DSYNC __O_DSYNC
#elif defined(_O_DSYNC)
#define O_DSYNC _O_DSYNC
#endif
#endif /* !O_DSYNC */

#ifndef O_ASYNC
#ifdef __O_ASYNC
#define O_ASYNC __O_ASYNC
#elif defined(_O_ASYNC)
#define O_ASYNC _O_ASYNC
#endif
#endif /* !O_ASYNC */

#ifndef O_DIRECT
#ifdef __O_DIRECT
#define O_DIRECT __O_DIRECT
#elif defined(_O_DIRECT)
#define O_DIRECT _O_DIRECT
#endif
#endif /* O_DIRECT */

#ifndef O_LARGEFILE
#ifdef __O_LARGEFILE
#define O_LARGEFILE __O_LARGEFILE
#elif defined(_O_LARGEFILE)
#define O_LARGEFILE _O_LARGEFILE
#endif
#endif /* !O_LARGEFILE */

#ifndef O_DIRECTORY
#ifdef __O_DIRECTORY
#define O_DIRECTORY __O_DIRECTORY
#elif defined(_O_DIRECTORY)
#define O_DIRECTORY _O_DIRECTORY
#endif
#endif /* !O_DIRECTORY */

#ifndef O_NOFOLLOW
#ifdef __O_NOFOLLOW
#define O_NOFOLLOW __O_NOFOLLOW
#elif defined(_O_NOFOLLOW)
#define O_NOFOLLOW _O_NOFOLLOW
#endif
#endif /* !O_NOFOLLOW */

#ifndef O_TMPFILE
#ifdef __O_TMPFILE
#define O_TMPFILE __O_TMPFILE
#elif defined(_O_TMPFILE)
#define O_TMPFILE _O_TMPFILE
#endif
#endif /* !O_TMPFILE */

#ifndef O_CLOFORK
#ifdef __O_CLOFORK
#define O_CLOFORK __O_CLOFORK
#elif defined(_O_CLOFORK)
#define O_CLOFORK _O_CLOFORK
#endif
#endif /* !O_CLOFORK */

#ifndef O_SYMLINK
#ifdef __O_SYMLINK
#define O_SYMLINK __O_SYMLINK
#elif defined(_O_SYMLINK)
#define O_SYMLINK _O_SYMLINK
#endif
#endif /* !O_SYMLINK */

#ifndef O_DOSPATH
#ifdef __O_DOSPATH
#define O_DOSPATH __O_DOSPATH
#elif defined(_O_DOSPATH)
#define O_DOSPATH _O_DOSPATH
#elif defined(_MSC_VER)
#define O_DOSPATH 0
#endif
#endif /* !O_DOSPATH */

#ifndef O_SHLOCK
#ifdef __O_SHLOCK
#define O_SHLOCK __O_SHLOCK
#elif defined(_O_SHLOCK)
#define O_SHLOCK _O_SHLOCK
#endif
#endif /* !O_SHLOCK */

#ifndef O_EXLOCK
#ifdef __O_EXLOCK
#define O_EXLOCK __O_EXLOCK
#elif defined(_O_EXLOCK)
#define O_EXLOCK _O_EXLOCK
#endif
#endif /* !O_EXLOCK */

#ifndef O_XATTR
#ifdef __O_XATTR
#define O_XATTR __O_XATTR
#elif defined(_O_XATTR)
#define O_XATTR _O_XATTR
#endif
#endif /* !O_XATTR */

#ifndef O_EXEC
#ifdef __O_EXEC
#define O_EXEC __O_EXEC
#elif defined(_O_EXEC)
#define O_EXEC _O_EXEC
#endif
#endif /* !O_EXEC */

#ifndef O_SEARCH
#ifdef __O_SEARCH
#define O_SEARCH __O_SEARCH
#elif defined(_O_SEARCH)
#define O_SEARCH _O_SEARCH
#endif
#endif /* !O_SEARCH */

#ifndef O_TTY_INIT
#ifdef __O_TTY_INIT
#define O_TTY_INIT __O_TTY_INIT
#elif defined(_O_TTY_INIT)
#define O_TTY_INIT _O_TTY_INIT
#endif
#endif /* !O_TTY_INIT */

#ifndef O_NOLINKS
#ifdef __O_NOLINKS
#define O_NOLINKS __O_NOLINKS
#elif defined(_O_NOLINKS)
#define O_NOLINKS _O_NOLINKS
#endif
#endif /* !O_NOLINKS */

#if defined(CONFIG_HAVE_open64) && !defined(CONFIG_HAVE_open)
#define CONFIG_HAVE_open 1
#define open open64
#endif /* open = open64 */

#if defined(CONFIG_HAVE_creat64) && !defined(CONFIG_HAVE_creat)
#define CONFIG_HAVE_creat 1
#define creat creat64
#endif /* creat = creat64 */

#if defined(CONFIG_HAVE_open) && !defined(CONFIG_HAVE_open64)
#define CONFIG_HAVE_open64 1
#ifdef O_LARGEFILE
#define open64(filename, oflags, ...) open(filename, (oflags) | O_LARGEFILE, ##__VA_ARGS__)
#else /* O_LARGEFILE */
#define open64 open
#endif /* !O_LARGEFILE */
#endif /* open64 = open */

#if defined(CONFIG_HAVE_open) && !defined(CONFIG_HAVE_creat) && \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_creat 1
#ifdef O_WRONLY
#define creat(filename, mode) open(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define creat(filename, mode) open(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* creat = open */

#if defined(CONFIG_HAVE_open64) && !defined(CONFIG_HAVE_creat64) && \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_creat64 1
#ifdef O_WRONLY
#define creat64(filename, mode) open64(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define creat64(filename, mode) open64(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* creat64 = open64 */

#if defined(CONFIG_HAVE_wopen) && !defined(CONFIG_HAVE_wopen64)
#define CONFIG_HAVE_wopen64 1
#ifdef O_LARGEFILE
#define wopen64(filename, oflags, ...) wopen(filename, (oflags) | O_LARGEFILE, ##__VA_ARGS__)
#else /* O_LARGEFILE */
#define wopen64 wopen
#endif /* !O_LARGEFILE */
#endif /* wopen64 = wopen */

#if defined(CONFIG_HAVE_wopen) && !defined(CONFIG_HAVE_wcreat) && \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_wcreat 1
#ifdef O_WRONLY
#define wcreat(filename, mode) wopen(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define wcreat(filename, mode) wopen(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* wcreat = wopen */

#if defined(CONFIG_HAVE_wopen64) && !defined(CONFIG_HAVE_wcreat64) && \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_wcreat64 1
#ifdef O_WRONLY
#define wcreat64(filename, mode) wopen64(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define wcreat64(filename, mode) wopen64(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* wcreat64 = wopen64 */


#if defined(CONFIG_HAVE_pthread_suspend_np) && !defined(CONFIG_HAVE_pthread_suspend)
#define CONFIG_HAVE_pthread_suspend 1
#define pthread_suspend pthread_suspend_np
#endif /* pthread_suspend = pthread_suspend_np */

#if defined(CONFIG_HAVE_pthread_suspend) && !defined(CONFIG_HAVE_pthread_suspend_np)
#define CONFIG_HAVE_pthread_suspend_np 1
#define pthread_suspend_np pthread_suspend
#endif /* pthread_suspend_np = pthread_suspend */

#if defined(CONFIG_HAVE_pthread_unsuspend_np) && !defined(CONFIG_HAVE_pthread_continue)
#define CONFIG_HAVE_pthread_continue 1
#define pthread_continue pthread_unsuspend_np
#endif /* pthread_continue = pthread_unsuspend_np */

#if defined(CONFIG_HAVE_pthread_continue) && !defined(CONFIG_HAVE_pthread_unsuspend_np)
#define CONFIG_HAVE_pthread_unsuspend_np 1
#define pthread_unsuspend_np pthread_continue
#endif /* pthread_unsuspend_np = pthread_continue */

/* We'd need both suspend() and continue() functions (they only come as pairs!) */
#if (defined(CONFIG_HAVE_pthread_continue) && !defined(CONFIG_HAVE_pthread_suspend)) || \
    (!defined(CONFIG_HAVE_pthread_continue) && defined(CONFIG_HAVE_pthread_suspend))
#undef CONFIG_HAVE_pthread_suspend
#undef CONFIG_HAVE_pthread_continue
#undef CONFIG_HAVE_pthread_suspend_np
#undef CONFIG_HAVE_pthread_unsuspend_np
#endif /* pthread_continue + pthread_suspend */

#if defined(CONFIG_HAVE_pthread_setname_np) && !defined(CONFIG_HAVE_pthread_setname)
#define CONFIG_HAVE_pthread_setname 1
#define pthread_setname pthread_setname_np
#endif /* pthread_setname = pthread_setname_np */

#if defined(CONFIG_HAVE_pthread_setname) && !defined(CONFIG_HAVE_pthread_setname_np)
#define CONFIG_HAVE_pthread_setname_np 1
#define pthread_setname_np pthread_setname
#endif /* pthread_setname_np = pthread_setname */

#ifndef CONFIG_HAVE_abort
#define CONFIG_HAVE_abort 1
#define abort() _DeeAssert_Fail(NULL, __FILE__, __LINE__)
#endif /* !CONFIG_HAVE_abort */

#if !defined(CONFIG_HAVE_pause) && defined(CONFIG_HAVE_select)
#define CONFIG_HAVE_pause 1
#define pause() select(0, NULL, NULL, NULL, NULL)
#endif /* pause = select */


#if defined(_MSC_VER) || defined(__USE_DOS)
#define EXEC_STRING_VECTOR_TYPE char const *const *
#else /* _MSC_VER || __USE_DOS */
#define EXEC_STRING_VECTOR_TYPE char *const *
#endif /* !_MSC_VER && !__USE_DOS */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif /* !EXIT_SUCCESS */

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif /* !EXIT_FAILURE */

#ifndef EOF
#define EOF (-1)
#endif /* !EOF */

#ifdef _MSC_VER
#undef STDIN_FILENO
#undef STDOUT_FILENO
#undef STDERR_FILENO
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif /* _MSC_VER */

#ifdef CONFIG_HAVE_ERRNO_H
#define CONFIG_HAVE_errno 1
#define DeeSystem_GetErrno()  errno
#define DeeSystem_SetErrno(v) (errno=(v))
#else /* CONFIG_HAVE_ERRNO_H */
#define DeeSystem_GetErrno()  0
#define DeeSystem_SetErrno(v) (void)0
#endif /* !CONFIG_HAVE_ERRNO_H */


#if !defined(CONFIG_HAVE_memcasecmp) && defined(CONFIG_HAVE__memicmp)
#define CONFIG_HAVE_memcasecmp 1
#define memcasecmp _memicmp
#endif /* memcasecmp = _memicmp */

#ifndef CONFIG_HAVE_tolower
#define CONFIG_HAVE_tolower 1
#define tolower(ch) ((ch) >= 'A' && (ch) <= 'Z' ? ((ch) + ('a' - 'A')) : (ch))
#endif /* !CONFIG_HAVE_tolower */

#ifndef CONFIG_HAVE_toupper
#define CONFIG_HAVE_toupper 1
#define toupper(ch) ((ch) >= 'a' && (ch) <= 'z' ? ((ch) - ('a' - 'A')) : (ch))
#endif /* !CONFIG_HAVE_toupper */





#define DeeSystem_DEFINE_memrchr(name)                            \
	LOCAL void *name(void const *__restrict p, int c, size_t n) { \
		uint8_t *iter = (uint8_t *)p + n;                         \
		while (iter != (uint8_t *)p) {                            \
			if (*--iter == c)                                     \
				return iter;                                      \
		}                                                         \
		return NULL;                                              \
	}

#define DeeSystem_DEFINE_memmem(name)                                                        \
	LOCAL void *name(void const *__restrict haystack, size_t haystack_length,                \
	                 void const *__restrict needle, size_t needle_length) {                  \
		uint8_t *candidate;                                                                  \
		uint8_t marker;                                                                      \
		if unlikely(!needle_length || needle_length > haystack_length)                       \
			return NULL;                                                                     \
		haystack_length -= (needle_length - 1), marker = *(uint8_t *)needle;                 \
		while ((candidate = (uint8_t *)memchr(haystack, marker, haystack_length)) != NULL) { \
			if (memcmp(candidate, needle, needle_length) == 0)                               \
				return (void *)candidate;                                                    \
			++candidate;                                                                     \
			haystack_length = ((uint8_t *)haystack + haystack_length) - candidate;           \
			haystack        = (void const *)candidate;                                       \
		}                                                                                    \
		return NULL;                                                                         \
	}

#define DeeSystem_DEFINE_memrmem(name)                                              \
	LOCAL void *name(void const *__restrict haystack, size_t haystack_length,       \
	                 void const *__restrict needle, size_t needle_length) {         \
		void const *candidate;                                                      \
		uint8_t marker;                                                             \
		if unlikely(!needle_length || needle_length > haystack_length)              \
			return NULL;                                                            \
		haystack_length -= needle_length - 1;                                       \
		marker = *(uint8_t *)needle;                                                \
		while ((candidate = memrchr(haystack, marker, haystack_length)) != NULL) {  \
			if (memcmp(candidate, needle, needle_length) == 0)                      \
				return (void *)candidate;                                           \
			haystack_length = (size_t)((uint8_t *)candidate - (uint8_t *)haystack); \
		}                                                                           \
		return NULL;                                                                \
	}

#define DeeSystem_DEFINE_strnlen(name)                              \
	LOCAL size_t name(char const *__restrict str, size_t maxlen) {  \
		size_t result;                                              \
		for (result = 0; maxlen && *str; --maxlen, ++str, ++result) \
			;                                                       \
		return result;                                              \
	}

#define DeeSystem_DEFINE_wcslen(name)                   \
	LOCAL size_t name(dwchar_t const *__restrict str) { \
		size_t result;                                  \
		for (result = 0; *str; ++str, ++result)         \
			;                                           \
		return result;                                  \
	}

#define DeeSystem_DEFINE_rawmemchr(name)                \
	LOCAL void *name(void const *__restrict p, int c) { \
		uint8_t *haystack = (uint8_t *)p;               \
		for (;; ++haystack) {                           \
			if (*haystack == (uint8_t)c)                \
				break;                                  \
		}                                               \
		return haystack;                                \
	}

#define DeeSystem_DEFINE_rawmemrchr(name)               \
	LOCAL void *name(void const *__restrict p, int c) { \
		uint8_t *haystack = (uint8_t *)p;               \
		for (;;) {                                      \
			if (*--haystack == (uint8_t)c)              \
				break;                                  \
		}                                               \
		return haystack;                                \
	}

#define DeeSystem_DEFINE_memend(name)                             \
	LOCAL void *name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;                         \
		for (; num_bytes--; ++haystack) {                         \
			if (*haystack == (uint8_t)byte)                       \
				break;                                            \
		}                                                         \
		return haystack;                                          \
	}

#define DeeSystem_DEFINE_memxend(name)                            \
	LOCAL void *name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;                         \
		for (; num_bytes--; ++haystack) {                         \
			if (*haystack != (uint8_t)byte)                       \
				break;                                            \
		}                                                         \
		return haystack;                                          \
	}

#define DeeSystem_DEFINE_memrend(name)                            \
	LOCAL void *name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;                         \
		haystack += num_bytes;                                    \
		while (num_bytes--) {                                     \
			if (*--haystack == (uint8_t)byte)                     \
				break;                                            \
		}                                                         \
		return haystack;                                          \
	}

#define DeeSystem_DEFINE_memlen(name)                                          \
	LOCAL size_t name(void const *p, int byte, size_t num_bytes) {             \
		return (size_t)((uintptr_t)memend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memxlen(name)                                          \
	LOCAL size_t name(void const *p, int byte, size_t num_bytes) {              \
		return (size_t)((uintptr_t)memxend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memrlen(name)                                          \
	LOCAL size_t name(void const *p, int byte, size_t num_bytes) {              \
		return (size_t)((uintptr_t)memrend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_rawmemlen(name)                               \
	LOCAL size_t name(void const *p, int byte) {                       \
		return (size_t)((uintptr_t)rawmemchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_rawmemrlen(name)                               \
	LOCAL size_t name(void const *p, int byte) {                        \
		return (size_t)((uintptr_t)rawmemrchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memxchr(name)                            \
	LOCAL void *name(void const *__restrict p, int c, size_t n) { \
		uint8_t *haystack = (uint8_t *)p;                         \
		for (; n--; ++haystack) {                                 \
			if (*haystack != (uint8_t)c)                          \
				return haystack;                                  \
		}                                                         \
		return NULL;                                              \
	}

#define DeeSystem_DEFINE_rawmemxchr(name)               \
	LOCAL void *name(void const *__restrict p, int c) { \
		uint8_t *haystack = (uint8_t *)p;               \
		for (;; ++haystack) {                           \
			if (*haystack != (uint8_t)c)                \
				break;                                  \
		}                                               \
		return haystack;                                \
	}

#define DeeSystem_DEFINE_rawmemxlen(name)                               \
	LOCAL size_t name(void const *p, int byte) {                        \
		return (size_t)((uintptr_t)rawmemxchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memcasecmp(name)                    \
	LOCAL int name(void const *a, void const *b, size_t n) { \
		uint8_t *pa = (uint8_t *)a, *pb = (uint8_t *)b;      \
		uint8_t av, bv;                                      \
		av = bv = 0;                                         \
		while (n-- &&                                        \
		       (((av = *pa++) == (bv = *pb++)) ||            \
		        (av = tolower(av), bv = tolower(bv),         \
		         av == bv)))                                 \
			;                                                \
		return (int)av - (int)bv;                            \
	}

#define DeeSystem_DEFINE_memcasemem(name)                                               \
	LOCAL void *name(void const *__restrict haystack, size_t haystack_len,              \
	                 void const *__restrict needle, size_t needle_len) {                \
		void const *candidate;                                                          \
		uint8_t marker1, marker2;                                                       \
		if                                                                              \
			unlikely(!needle_len || needle_len > haystack_len)                          \
		return NULL;                                                                    \
		haystack_len -= needle_len;                                                     \
		marker1 = (uint8_t)tolower(*(uint8_t *)needle);                                 \
		marker2 = (uint8_t)toupper(*(uint8_t *)needle);                                 \
		while ((candidate = memchr(haystack, marker1, haystack_len)) != NULL ||         \
		       (candidate = memchr(haystack, marker2, haystack_len)) != NULL) {         \
			if (memcasecmp(candidate, needle, needle_len) == 0)                         \
				return (void *)candidate;                                               \
			haystack_len = ((uintptr_t)haystack + haystack_len) - (uintptr_t)candidate; \
			haystack     = (void const *)((uintptr_t)candidate + 1);                    \
		}                                                                               \
		return NULL;                                                                    \
	}


#define DeeSystem_DEFINE_memrev(name)                  \
	LOCAL void *name(void *__restrict buf, size_t n) { \
		uint8_t *iter, *end;                           \
		end = (iter = (uint8_t *)buf) + n;             \
		while (iter < end) {                           \
			uint8_t temp = *iter;                      \
			*iter++      = *--end;                     \
			*end         = temp;                       \
		}                                              \
		return buf;                                    \
	}

#define DeeSystem_DEFINE_memxrchr(name)                           \
	LOCAL void *name(void const *__restrict p, int c, size_t n) { \
		uint8_t *haystack = (uint8_t *)p + n;                     \
		while (n--) {                                             \
			if (*--haystack != (uint8_t)c)                        \
				return haystack;                                  \
		}                                                         \
		return NULL;                                              \
	}

#define DeeSystem_DEFINE_memxrend(name)                           \
	LOCAL void *name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;                         \
		haystack += num_bytes;                                    \
		while (num_bytes--) {                                     \
			if (*--haystack != (uint8_t)byte)                     \
				break;                                            \
		}                                                         \
		return haystack;                                          \
	}

#define DeeSystem_DEFINE_memxrlen(name)                                          \
	LOCAL size_t name(void const *p, int byte, size_t num_bytes) {               \
		return (size_t)((uintptr_t)memxrend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_rawmemxrchr(name)              \
	LOCAL void *name(void const *__restrict p, int c) { \
		uint8_t *haystack = (uint8_t *)p;               \
		for (;;) {                                      \
			if (*--haystack != (uint8_t)c)              \
				break;                                  \
		}                                               \
		return haystack;                                \
	}

#define DeeSystem_DEFINE_rawmemxrlen(name)                               \
	LOCAL size_t name(void const *p, int byte) {                         \
		return (size_t)((uintptr_t)rawmemxrchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memcasermem(name)                                                     \
	LOCAL void *_##name##_memlowerrchr(void const *__restrict p, uint8_t c, size_t n) {        \
		uint8_t *iter = (uint8_t *)p + n;                                                      \
		while (iter-- != (uint8_t *)p) {                                                       \
			if ((uint8_t)tolower(*iter) == c)                                                  \
				return iter;                                                                   \
		}                                                                                      \
		return NULL;                                                                           \
	}                                                                                          \
	LOCAL void *name(void const *__restrict haystack, size_t haystack_len,                     \
	                 void const *__restrict needle, size_t needle_len) {                       \
		void const *candidate;                                                                 \
		uint8_t marker;                                                                        \
		if                                                                                     \
			unlikely(!needle_len || needle_len > haystack_len)                                 \
		return NULL;                                                                           \
		haystack_len -= needle_len;                                                            \
		marker = (uint8_t)tolower(*(uint8_t *)needle);                                         \
		while ((candidate = _##name##_memlowerrchr(haystack, marker, haystack_len)) != NULL) { \
			if (memcasecmp(candidate, needle, needle_len) == 0)                                \
				return (void *)candidate;                                                      \
			if                                                                                 \
				unlikely(candidate == haystack)                                                \
			break;                                                                             \
			haystack_len = (((uintptr_t)candidate) - 1) - (uintptr_t)haystack;                 \
		}                                                                                      \
		return NULL;                                                                           \
	}

#define DeeSystem_DEFINE_qsort(name)                                                \
	LOCAL void _##name##_swapmemory(uint8_t *a, uint8_t *b,                         \
	                                size_t num_bytes) {                             \
		size_t i;                                                                   \
		for (i = 0; i < num_bytes; ++i) {                                           \
			uint8_t temp;                                                           \
			temp = a[i];                                                            \
			a[i] = b[i];                                                            \
			b[i] = temp;                                                            \
		}                                                                           \
	}                                                                               \
	LOCAL void name(void *base, size_t count, size_t size,                          \
	                int (*compare)(void const *a, void const *b)) {                 \
		size_t i, j;                                                                \
		/* Not actually quick-sort, but this function's only used as fallback       \
		 * when the linked libc doesn't offer a real qsort function, so it's ok! */ \
		for (i = 0; i < count; ++i) {                                               \
			for (j = i; j < count; ++j) {                                           \
				uint8_t *a = (uint8_t *)base + j * size;                            \
				uint8_t *b = (uint8_t *)base + i * size;                            \
				if ((*compare)(a, b) < 0)                                           \
					_##name##_swapmemory(a, b, size);                               \
			}                                                                       \
		}                                                                           \
	}




/* errno classification helper macros */

/*[[[deemon
import * from deemon;
import * from util;
local n = 4;

local errno_names = {
	"ENOENT",
	"ENOTDIR",
	"ENAMETOOLONG",
	"ELOOP",
	"EPERM",
	"EACCES",
	"EROFS"
};

for (local x: errno_names) {
	print "#ifdef",x;
	print "#define DeePrivateSystem_IF_HAVE_",;
	print x,;
	print "(tt, ff) tt";
	print "#else /" "*", x, "*" "/";
	print "#define DeePrivateSystem_IF_HAVE_",;
	print x,;
	print "(tt, ff) ff";
	print "#endif /" "* !",;
	print x,"*" "/";
}


function printMacro(text) {
	local longest_line_length = 0;
	local lines = List(text.splitlines(false));
	for (local x: lines) {
		x = #x.expandtabs(4);
		if (longest_line_length < x)
			longest_line_length = x;
	}
	for (local i, x: enumerate(lines)) {
		print x,;
		if (i == #lines - 1) {
			print;
		} else {
			x = #x.expandtabs(4);
			print " " * (longest_line_length - x),;
			print " \\";
		}
	}
}

for (local x: [1:n+1]) {
	File.Writer fp;
	fp << "#define DeePrivateSystem_IF_E" << x << "(errno, ";
	fp << ", ".join(for (local i: [:x]) "e" + (i + 1));
	fp << ", ...)\n";
	fp << "	do {\n";
	fp << "		if (";
	fp << " || ".join(for (local i: [:x]) "(errno) == e" + (i + 1));
	fp << ") {\n";
	fp << "			__VA_ARGS__;\n";
	fp << "		}\n";
	fp << "	} __WHILE0";
	printMacro(fp.string);
}

function printIfElseSelection(fp, prefix, checked, unchecked) {
	if (!unchecked) {
		if (!checked) {
			fp << "(void)0";
		} else {
			fp << "DeePrivateSystem_IF_E" << #checked << "(errno, "
				<< ", ".join(checked) << ", __VA_ARGS__)";
		}
	} else {
		local first = unchecked[0];
		local other = List(unchecked[1:]);
		fp << "DeePrivateSystem_IF_HAVE_##" << first << "(\n";
		fp << prefix << "\t";
		printIfElseSelection(fp, prefix + "\t", checked + { first }, other);
		fp << "," << "\n";
		fp << prefix << "\t";
		printIfElseSelection(fp, prefix + "\t", checked, other);
		fp << ")";
	}
}

for (local x: [1:n+1]) {
	File.Writer fp;
	fp << "#define DeeSystem_IF_E" << x << "(errno, ";
	fp << ", ".join(for (local i: [:x]) "e" + (i + 1));
	fp << ", ...)\n";
	fp << "\t";
	printIfElseSelection(fp, "\t", [], List(for (local i: [:x]) "e" + (i + 1)));
	printMacro(fp.string);
}

]]]*/
#ifdef ENOENT
#define DeePrivateSystem_IF_HAVE_ENOENT(tt, ff) tt
#else /* ENOENT */
#define DeePrivateSystem_IF_HAVE_ENOENT(tt, ff) ff
#endif /* !ENOENT */
#ifdef ENOTDIR
#define DeePrivateSystem_IF_HAVE_ENOTDIR(tt, ff) tt
#else /* ENOTDIR */
#define DeePrivateSystem_IF_HAVE_ENOTDIR(tt, ff) ff
#endif /* !ENOTDIR */
#ifdef ENAMETOOLONG
#define DeePrivateSystem_IF_HAVE_ENAMETOOLONG(tt, ff) tt
#else /* ENAMETOOLONG */
#define DeePrivateSystem_IF_HAVE_ENAMETOOLONG(tt, ff) ff
#endif /* !ENAMETOOLONG */
#ifdef ELOOP
#define DeePrivateSystem_IF_HAVE_ELOOP(tt, ff) tt
#else /* ELOOP */
#define DeePrivateSystem_IF_HAVE_ELOOP(tt, ff) ff
#endif /* !ELOOP */
#ifdef EPERM
#define DeePrivateSystem_IF_HAVE_EPERM(tt, ff) tt
#else /* EPERM */
#define DeePrivateSystem_IF_HAVE_EPERM(tt, ff) ff
#endif /* !EPERM */
#ifdef EACCES
#define DeePrivateSystem_IF_HAVE_EACCES(tt, ff) tt
#else /* EACCES */
#define DeePrivateSystem_IF_HAVE_EACCES(tt, ff) ff
#endif /* !EACCES */
#ifdef EROFS
#define DeePrivateSystem_IF_HAVE_EROFS(tt, ff) tt
#else /* EROFS */
#define DeePrivateSystem_IF_HAVE_EROFS(tt, ff) ff
#endif /* !EROFS */
#define DeePrivateSystem_IF_E1(errno, e1, ...) \
	do {                                       \
		if ((errno) == e1) {                   \
			__VA_ARGS__;                       \
		}                                      \
	} __WHILE0
#define DeePrivateSystem_IF_E2(errno, e1, e2, ...) \
	do {                                           \
		if ((errno) == e1 || (errno) == e2) {      \
			__VA_ARGS__;                           \
		}                                          \
	} __WHILE0
#define DeePrivateSystem_IF_E3(errno, e1, e2, e3, ...)         \
	do {                                                       \
		if ((errno) == e1 || (errno) == e2 || (errno) == e3) { \
			__VA_ARGS__;                                       \
		}                                                      \
	} __WHILE0
#define DeePrivateSystem_IF_E4(errno, e1, e2, e3, e4, ...)                      \
	do {                                                                        \
		if ((errno) == e1 || (errno) == e2 || (errno) == e3 || (errno) == e4) { \
			__VA_ARGS__;                                                        \
		}                                                                       \
	} __WHILE0
#define DeeSystem_IF_E1(errno, e1, ...)                 \
	DeePrivateSystem_IF_HAVE_##e1(                      \
		DeePrivateSystem_IF_E1(errno, e1, __VA_ARGS__), \
		(void)0)
#define DeeSystem_IF_E2(errno, e1, e2, ...)                     \
	DeePrivateSystem_IF_HAVE_##e1(                              \
		DeePrivateSystem_IF_HAVE_##e2(                          \
			DeePrivateSystem_IF_E2(errno, e1, e2, __VA_ARGS__), \
			DeePrivateSystem_IF_E1(errno, e1, __VA_ARGS__)),    \
		DeePrivateSystem_IF_HAVE_##e2(                          \
			DeePrivateSystem_IF_E1(errno, e2, __VA_ARGS__),     \
			(void)0))
#define DeeSystem_IF_E3(errno, e1, e2, e3, ...)                         \
	DeePrivateSystem_IF_HAVE_##e1(                                      \
		DeePrivateSystem_IF_HAVE_##e2(                                  \
			DeePrivateSystem_IF_HAVE_##e3(                              \
				DeePrivateSystem_IF_E3(errno, e1, e2, e3, __VA_ARGS__), \
				DeePrivateSystem_IF_E2(errno, e1, e2, __VA_ARGS__)),    \
			DeePrivateSystem_IF_HAVE_##e3(                              \
				DeePrivateSystem_IF_E2(errno, e1, e3, __VA_ARGS__),     \
				DeePrivateSystem_IF_E1(errno, e1, __VA_ARGS__))),       \
		DeePrivateSystem_IF_HAVE_##e2(                                  \
			DeePrivateSystem_IF_HAVE_##e3(                              \
				DeePrivateSystem_IF_E2(errno, e2, e3, __VA_ARGS__),     \
				DeePrivateSystem_IF_E1(errno, e2, __VA_ARGS__)),        \
			DeePrivateSystem_IF_HAVE_##e3(                              \
				DeePrivateSystem_IF_E1(errno, e3, __VA_ARGS__),         \
				(void)0)))
#define DeeSystem_IF_E4(errno, e1, e2, e3, e4, ...)                             \
	DeePrivateSystem_IF_HAVE_##e1(                                              \
		DeePrivateSystem_IF_HAVE_##e2(                                          \
			DeePrivateSystem_IF_HAVE_##e3(                                      \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E4(errno, e1, e2, e3, e4, __VA_ARGS__), \
					DeePrivateSystem_IF_E3(errno, e1, e2, e3, __VA_ARGS__)),    \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E3(errno, e1, e2, e4, __VA_ARGS__),     \
					DeePrivateSystem_IF_E2(errno, e1, e2, __VA_ARGS__))),       \
			DeePrivateSystem_IF_HAVE_##e3(                                      \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E3(errno, e1, e3, e4, __VA_ARGS__),     \
					DeePrivateSystem_IF_E2(errno, e1, e3, __VA_ARGS__)),        \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E2(errno, e1, e4, __VA_ARGS__),         \
					DeePrivateSystem_IF_E1(errno, e1, __VA_ARGS__)))),          \
		DeePrivateSystem_IF_HAVE_##e2(                                          \
			DeePrivateSystem_IF_HAVE_##e3(                                      \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E3(errno, e2, e3, e4, __VA_ARGS__),     \
					DeePrivateSystem_IF_E2(errno, e2, e3, __VA_ARGS__)),        \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E2(errno, e2, e4, __VA_ARGS__),         \
					DeePrivateSystem_IF_E1(errno, e2, __VA_ARGS__))),           \
			DeePrivateSystem_IF_HAVE_##e3(                                      \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E2(errno, e3, e4, __VA_ARGS__),         \
					DeePrivateSystem_IF_E1(errno, e3, __VA_ARGS__)),            \
				DeePrivateSystem_IF_HAVE_##e4(                                  \
					DeePrivateSystem_IF_E1(errno, e4, __VA_ARGS__),             \
					(void)0))))
//[[[end]]]


#endif /* !GUARD_DEEMON_SYSTEM_FEATURES_H */