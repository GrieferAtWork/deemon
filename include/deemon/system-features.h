/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_SYSTEM_FEATURES_H
#define GUARD_DEEMON_SYSTEM_FEATURES_H 1

#include "api.h"
/**/

#include <hybrid/host.h>
#include <hybrid/typecore.h>

#include <stddef.h>

/*[[[deemon
import * from deemon;
#pragma extension("-fno-character-literals")
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
function feature(name, default_requirements, test = none) {
	print("#ifdef CONFIG_NO_", name);
	print("#undef CONFIG_HAVE_", name);
	if (default_requirements == "1") {
		print("#else");
	} else if (default_requirements in ["", "0"]) {
		print("#elif 0");
	} else {
		print("#elif !defined(CONFIG_HAVE_", name, ") && \\");
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
			print(default_requirements[:p].rstrip(), " \\");
			print "       ",;
			default_requirements = default_requirements[p:].lstrip(),;
		}
		print(default_requirements, ")");
	}
	print("#define CONFIG_HAVE_", name, " 1");
	print("#endif");
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
	} else if (default_requirements != "1") {
		default_requirements = "__has_include(<{}>) || (defined(__NO_has_include) && {})".format({ name, default_requirements });
	} else {
		default_requirements = "defined(__NO_has_include) || __has_include(<{}>)".format({ name });
	}
	local featnam = header_featnam(name);
	(none)feature(featnam, default_requirements);
}
function header(name, default_requirements = "") {
	known_headers.append(name);
	header_nostdinc(name, default_requirements);
}
function sizeof(name) {
	// CONFIG_SIZEOF_ + name.upper()
}

#define var      func
#define typ      func
#define constant func
function constant_nonzero(name) {
	print("#ifdef CONFIG_NO_", name);
	print("#undef CONFIG_HAVE_", name);
	print("#elif defined(CONFIG_HAVE_", name, ")");
	print("#elif defined(", name, ")");
	print("#if ", name, " != 0");
	print("#define CONFIG_HAVE_", name, " 1");
	print("#else /" "* ", name, " *" "/");
	print("#define CONFIG_NO_", name, " 1");
	print("#endif /" "* ", name, " *" "/");
	print("#elif defined(__", name, "__defined)");
	print("#define CONFIG_HAVE_", name, " 1");
	print("#endif");
	print;
}
function func(name, default_requirements = "", check_defined = 2, test = none) {
	if (default_requirements != "1" && check_defined) {
		default_requirements = addparen(default_requirements);
		if (default_requirements !in ["", "0"])
			default_requirements = " || " + default_requirements;
		if (check_defined !is int || check_defined >= 2) {
			default_requirements = "defined({}) || defined(__{}_defined){}"
				.format({ name, name, default_requirements });
		//} else {
		//	default_requirements = "defined({}){}"
		//		.format({ name, default_requirements });
		}
	}
	(none)feature(name, default_requirements);
}
function isenabled(name) {
	return "(defined({}) && {}+0 != 0)".format({ name, name });
}
function include_known_headers() {
	for (local name: known_headers) {
		local featnam = header_featnam(name);
		print("#ifdef CONFIG_HAVE_", featnam);
		print("#include <", name, ">");
		print("#endif /" "* CONFIG_HAVE_", featnam, " *" "/");
		print;
	}
}


local linux = "defined(__linux__) || defined(__linux) || defined(linux)";
local unix = linux + " || defined(__unix__) || defined(__unix) || defined(unix)";
local cygwin = "defined(__CYGWIN__) || defined(__CYGWIN32__)";
local kos = "defined(__KOS__)";
local msvc = "defined(_MSC_VER)";
local stdc = "1";

//The following line is needed by the configure script.
//BEGIN:FEATURES

header("io.h", addparen(msvc) + " || " + addparen(kos));
header("direct.h", addparen(msvc) + " || " + addparen(kos));
header("process.h", addparen(msvc) + " || " + addparen(kos));
header("sys/stat.h", addparen(msvc) + " || " + addparen(unix));
header("fcntl.h", addparen(msvc) + " || " + addparen(unix));
header("sys/fcntl.h", addparen(linux) + " || " + addparen(kos));
header("sys/ioctl.h", unix);
header("alloca.h", addparen(cygwin) + " || " + addparen(linux) + " || " + addparen(kos));
header("malloc.h", addparen(msvc) + " || " + addparen(cygwin) + " || " + addparen(linux) + " || " + addparen(kos));
header("ioctl.h");
header("dirent.h", unix);
header("unistd.h", unix);
header("sys/unistd.h", cygwin);
header("errno.h", addparen(msvc) + " || " + addparen(unix));
header("sys/errno.h");
header("stdio.h", stdc);
header("stdlib.h", stdc);
header("features.h", unix);
header("sched.h", unix);
header("signal.h", unix);
header("sys/signal.h", addparen(linux) + " || " + addparen(kos));
header("sys/syscall.h", addparen(linux) + " || " + addparen(kos));
header("pthread.h", unix);
header("sys/types.h", addparen(linux) + " || " + addparen(kos));
header("semaphore.h", addparen(linux) + " || " + addparen(kos));
header("time.h", addparen(msvc) + " || " + addparen(unix));
header("sys/time.h", addparen(linux) + " || " + addparen(kos));
header("sys/mman.h", addparen(linux) + " || " + addparen(kos));
header("sys/resource.h", addparen(linux) + " || " + addparen(kos));
header("sys/wait.h", unix);
header("wait.h", addparen(linux) + " || " + addparen(kos));
header("sys/signalfd.h", addparen(linux) + " || " + addparen(kos));
header("ctype.h", stdc);
header("string.h", stdc);
header("strings.h", unix);
header("wchar.h", stdc);
header("dlfcn.h", unix);

header_nostdinc("crtdbg.h", addparen(msvc) + " || defined(__KOS_SYSTEM_HEADERS__)");
header_nostdinc("float.h", stdc);
header_nostdinc("limits.h", stdc);
header_nostdinc("setjmp.h", stdc);
header_nostdinc("link.h");
header_nostdinc("bluetooth/bluetooth.h");
header_nostdinc("bluetooth/rfcomm.h");
header_nostdinc("bluetooth/l2cap.h");
header_nostdinc("bluetooth/sco.h");
header_nostdinc("bluetooth/hci.h");
header_nostdinc("bluetooth.h");
header_nostdinc("linux/netlink.h", addparen(linux) + " || " + addparen(kos));
header_nostdinc("asm/types.h", addparen(linux) + " || " + addparen(kos));
header_nostdinc("sys/un.h", addparen(linux) + " || " + addparen(kos));
header_nostdinc("sys/socket.h", unix);
header_nostdinc("sys/select.h", addparen(linux) + " || " + addparen(kos));
header_nostdinc("netdb.h", unix);
header_nostdinc("math.h", stdc);

include_known_headers();

#define functest(name, ...) \
	func(name[:name.index("(")], __VA_ARGS__)

func("_Exit", "defined(__USE_ISOC99)", test: "_Exit(0);");
func("_exit", addparen(msvc) + " || " + addparen(unix), test: "_exit(0);");
func("exit", stdc, test: "exit(0);");
func("atexit", stdc, test: "extern void foo(void); return atexit(&foo);");

func("execv", unix, test: 'char *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return execv("a", argv);');
func("_execv", msvc, test: 'char const *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return _execv("a", argv);');
func("execve", unix, test: 'char *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return execve("a", argv, argv);');
func("_execve", msvc, test: 'char const *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return _execve("a", argv, argv);');
func("execvp", unix, test: 'char *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return execvp("a", argv);');
func("_execvp", msvc, test: 'char const *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return _execvp("a", argv);');
func("execvpe", unix, test: 'char *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return execvpe("a", argv, argv);');
func("_execvpe", msvc, test: 'char const *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return _execvpe("a", argv, argv);');
func("wexecv", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wexecv(s, argv);");
func("_wexecv", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wexecv(s, argv);");
func("wexecve", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wexecve(s, argv, argv);");
func("_wexecve", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wexecve(s, argv, argv);");
func("wexecvp", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wexecvp(s, argv);");
func("_wexecvp", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wexecvp(s, argv);");
func("wexecvpe", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wexecvpe(s, argv, argv);");
func("_wexecvpe", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wexecvpe(s, argv, argv);");
func("spawnv", msvc, test: 'char *argv[2]; argv[0] = "a"; argv[1] = 0; return spawnv(42, "a", argv);');
func("_spawnv", msvc, test: 'char const *argv[2]; argv[0] = "a"; argv[1] = 0; return _spawnv(42, "a", argv);');
func("spawnve", msvc, test: 'char *argv[2]; argv[0] = "a"; argv[1] = 0; return spawnve(42, "a", argv, argv);');
func("_spawnve", msvc, test: 'char const *argv[2]; argv[0] = "a"; argv[1] = 0; return _spawnve(42, "a", argv, argv);');
func("spawnvp", msvc, test: 'char *argv[2]; argv[0] = "a"; argv[1] = 0; return spawnvp(42, "a", argv);');
func("_spawnvp", msvc, test: 'char const *argv[2]; argv[0] = "a"; argv[1] = 0; return _spawnvp(42, "a", argv);');
func("spawnvpe", msvc, test: 'char *argv[2]; argv[0] = "a"; argv[1] = 0; return spawnvpe(42, "a", argv, argv);');
func("_spawnvpe", msvc, test: 'char const *argv[2]; argv[0] = "a"; argv[1] = 0; return _spawnvpe(42, "a", argv, argv);');
func("wspawnv", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wspawnv(42, s, argv);");
func("_wspawnv", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wspawnv(42, s, argv);");
func("wspawnve", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wspawnve(42, s, argv, argv);");
func("_wspawnve", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wspawnve(42, s, argv, argv);");
func("wspawnvp", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wspawnvp(42, s, argv);");
func("_wspawnvp", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wspawnvp(42, s, argv);");
func("wspawnvpe", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t *argv[2]; argv[0] = s; argv[1] = 0; return wspawnvpe(42, s, argv, argv);");
func("_wspawnvpe", msvc, test: "wchar_t s[] = { 'a', 0 }; wchar_t const *argv[2]; argv[0] = s; argv[1] = 0; return _wspawnvpe(42, s, argv, argv);");
func("fexecve", "defined(__USE_XOPEN2K8)", test: 'char *argv[2]; argv[0] = (char *)"a"; argv[1] = 0; return fexecve(42, argv, argv);');

func("cwait", msvc, test: 'int st; return cwait(&st, 42, 43);');
func("_cwait", msvc, test: 'int st; return cwait(&st, 42, 43);');
func("wait", unix, test: 'int st; return wait(&st);');
func("waitpid", unix, test: 'int st; return waitpid(42, &st, 0);');
func("wait4", addparen(linux) + " || " + addparen(kos), test: 'int st; struct rusage ru; return wait4(42, &st, 0, &ru);');
func("waitid", addparen(linux) + " || " + addparen(kos), test: 'siginfo_t si; return waitid(P_ALL, 42, &si, WEXITED);');
func("sigprocmask", unix, test: "sigset_t os; return sigprocmask(SIG_SETMASK, NULL, &os);");
functest("detach(42)", kos + " && defined(__USE_KOS) && __KOS_VERSION__ >= 300");

functest('system("echo hi")', stdc);
func("wsystem", test: "wchar_t c[] = { 'e', 'c', 'h', 'o', ' ', 'h', 'i', 0 }; return wsystem(c);");
func("_wsystem", msvc, test: "wchar_t c[] = { 'e', 'c', 'h', 'o', ' ', 'h', 'i', 0 }; return _wsystem(c);");

functest('creat("foo.txt", 0644)', unix);
functest('_creat("foo.txt", 0644)', msvc);
func("_wcreat", "defined(_WIO_DEFINED) || " + addparen(msvc), test: "wchar_t s[] = { 'a', 0 }; return _wcreat(s, 0644);");

functest('open("foo.txt", O_RDONLY)', unix);
functest('_open("foo.txt", O_RDONLY)', msvc);
func("_wopen", "defined(_WIO_DEFINED) || " + addparen(msvc), test: "wchar_t s[] = { 'a', 0 }; return _wopen(s, O_RDONLY);");
functest('open64("foo.txt", O_RDONLY)', "defined(__USE_LARGEFILE64)");
functest("fcntl(42, 7) && fcntl(42, 7, 21)", "(defined(CONFIG_HAVE_FCNTL_H) || defined(CONFIG_HAVE_SYS_FCNTL_H)) && " + addparen(unix));

constant("F_SETFD");
constant("F_SETFL");
constant("FD_CLOEXEC");
constant_nonzero("O_BINARY");
constant_nonzero("__O_BINARY");
constant_nonzero("_O_BINARY");
constant_nonzero("__O_RAW");
constant_nonzero("_O_RAW");
constant_nonzero("O_RAW");
constant_nonzero("O_SHORT_LIVED");
constant_nonzero("__O_SHORT_LIVED");
constant_nonzero("_O_SHORT_LIVED");
constant_nonzero("O_SEQUENTIAL");
constant_nonzero("__O_SEQUENTIAL");
constant_nonzero("_O_SEQUENTIAL");
constant_nonzero("O_RANDOM");
constant_nonzero("__O_RANDOM");
constant_nonzero("_O_RANDOM");
constant_nonzero("O_PATH");
constant_nonzero("__O_PATH");
constant_nonzero("_O_PATH");
constant_nonzero("O_NOATIME");
constant_nonzero("__O_NOATIME");
constant_nonzero("_O_NOATIME");
constant_nonzero("O_NOCTTY");
constant_nonzero("__O_NOCTTY");
constant_nonzero("_O_NOCTTY");
constant_nonzero("O_TEXT");
constant_nonzero("__O_TEXT");
constant_nonzero("_O_TEXT");
constant_nonzero("O_WTEXT");
constant_nonzero("__O_WTEXT");
constant_nonzero("_O_WTEXT");
constant_nonzero("O_U16TEXT");
constant_nonzero("__O_U16TEXT");
constant_nonzero("_O_U16TEXT");
constant_nonzero("O_U8TEXT");
constant_nonzero("__O_U8TEXT");
constant_nonzero("_O_U8TEXT");
constant_nonzero("O_TEMPORARY");
constant_nonzero("__O_TEMPORARY");
constant_nonzero("_O_TEMPORARY");
constant_nonzero("O_OBTAIN_DIR");
constant_nonzero("__O_OBTAIN_DIR");
constant_nonzero("_O_OBTAIN_DIR");
constant_nonzero("O_CREAT");
constant_nonzero("__O_CREAT");
constant_nonzero("_O_CREAT");
constant_nonzero("O_TRUNC");
constant_nonzero("__O_TRUNC");
constant_nonzero("_O_TRUNC");
constant("O_RDONLY");
constant("__O_RDONLY");
constant("_O_RDONLY");
constant("O_WRONLY");
constant("__O_WRONLY");
constant("_O_WRONLY");
constant("O_RDWR");
constant("__O_RDWR");
constant("_O_RDWR");
constant("O_ACCMODE");
constant("__O_ACCMODE");
constant("_O_ACCMODE");
constant_nonzero("O_CLOEXEC");
constant_nonzero("__O_NOINHERIT");
constant_nonzero("_O_NOINHERIT");
constant_nonzero("O_NOINHERIT");
constant_nonzero("__O_CLOEXEC");
constant_nonzero("_O_CLOEXEC");
constant_nonzero("O_EXCL");
constant_nonzero("__O_EXCL");
constant_nonzero("_O_EXCL");
constant_nonzero("O_APPEND");
constant_nonzero("__O_APPEND");
constant_nonzero("_O_APPEND");
constant_nonzero("O_NONBLOCK");
constant_nonzero("__O_NONBLOCK");
constant_nonzero("_O_NONBLOCK");
constant_nonzero("__O_NDELAY");
constant_nonzero("_O_NDELAY");
constant_nonzero("O_NDELAY");
constant_nonzero("O_RSYNC");
constant_nonzero("__O_RSYNC");
constant_nonzero("_O_RSYNC");
constant_nonzero("O_SYNC");
constant_nonzero("__O_SYNC");
constant_nonzero("_O_SYNC");
constant_nonzero("O_DSYNC");
constant_nonzero("__O_DSYNC");
constant_nonzero("_O_DSYNC");
constant_nonzero("O_ASYNC");
constant_nonzero("__O_ASYNC");
constant_nonzero("_O_ASYNC");
constant_nonzero("O_DIRECT");
constant_nonzero("__O_DIRECT");
constant_nonzero("_O_DIRECT");
constant_nonzero("O_LARGEFILE");
constant_nonzero("__O_LARGEFILE");
constant_nonzero("_O_LARGEFILE");
constant_nonzero("O_DIRECTORY");
constant_nonzero("__O_DIRECTORY");
constant_nonzero("_O_DIRECTORY");
constant_nonzero("O_NOFOLLOW");
constant_nonzero("__O_NOFOLLOW");
constant_nonzero("_O_NOFOLLOW");
constant_nonzero("O_TMPFILE");
constant_nonzero("__O_TMPFILE");
constant_nonzero("_O_TMPFILE");
constant_nonzero("O_CLOFORK");
constant_nonzero("__O_CLOFORK");
constant_nonzero("_O_CLOFORK");
constant_nonzero("O_SYMLINK");
constant_nonzero("__O_SYMLINK");
constant_nonzero("_O_SYMLINK");
constant_nonzero("O_DOSPATH");
constant_nonzero("__O_DOSPATH");
constant_nonzero("_O_DOSPATH");
constant_nonzero("O_SHLOCK");
constant_nonzero("__O_SHLOCK");
constant_nonzero("_O_SHLOCK");
constant_nonzero("O_EXLOCK");
constant_nonzero("__O_EXLOCK");
constant_nonzero("_O_EXLOCK");
constant_nonzero("O_XATTR");
constant_nonzero("__O_XATTR");
constant_nonzero("_O_XATTR");
constant_nonzero("O_EXEC");
constant_nonzero("__O_EXEC");
constant_nonzero("_O_EXEC");
constant_nonzero("O_SEARCH");
constant_nonzero("__O_SEARCH");
constant_nonzero("_O_SEARCH");
constant_nonzero("O_TTY_INIT");
constant_nonzero("__O_TTY_INIT");
constant_nonzero("_O_TTY_INIT");
constant_nonzero("O_NOLINKS");
constant_nonzero("__O_NOLINKS");
constant_nonzero("_O_NOLINKS");

constant("AT_SYMLINK_NOFOLLOW");
constant("AT_REMOVEDIR");
constant("AT_EACCESS");
constant("AT_SYMLINK_FOLLOW");
constant("AT_NO_AUTOMOUNT");
constant("AT_EMPTY_PATH");
constant("AT_SYMLINK_REGULAR");
constant("AT_CHANGE_CTIME");
//constant("AT_READLINK_REQSIZE"); // Note needed (Only used by; and also implied by `freadlinkat()')
constant("AT_REMOVEREG");
constant("AT_ALTPATH");
constant("AT_DOSPATH");
constant("AT_FDCWD");
constant("AT_FDROOT");
constant("AT_THIS_TASK");
constant("AT_THIS_PROCESS");
constant("AT_PARENT_PROCESS");
constant("AT_GROUP_LEADER");
constant("AT_SESSION_LEADER");
constant("AT_DOS_DRIVEMIN");
constant("AT_DOS_DRIVEMAX");
functest("AT_FDDRIVE_CWD('C')", check_defined: 1);
functest("AT_FDDRIVE_ROOT('C')", check_defined: 1);


func("read", unix, test: 'char buf[7]; return (int)read(0, buf, 7);');
func("_read", msvc, test: 'char buf[7]; return (int)_read(0, buf, 7);');

func("write", unix, test: 'char const buf[] = "foo"; return (int)write(1, buf, 3);');
func("_write", msvc, test: 'char const buf[] = "foo"; return (int)_write(1, buf, 3);');

func("lseek", unix, test: "return (int)lseek(1, 0, SEEK_SET);");
func("lseek64", "defined(__USE_LARGEFILE64)", test: "return (int)lseek64(1, 0, SEEK_SET);");
func("_lseek", msvc, test: "return (int)_lseek(1, 0, SEEK_SET);");
func("_lseek64", test: "return (int)_lseek64(1, 0, SEEK_SET);");
func("_lseeki64", msvc, test: "return (int)_lseeki64(1, 0, SEEK_SET);");

functest('chdir("..")', unix);
functest('_chdir("..")', msvc);
func("wchdir", test: "wchar_t c[] = { 'f', 'o', 'o', 0 }; return wchdir(c, 0755);");
func("_wchdir", "defined(_WDIRECT_DEFINED)", test: "wchar_t c[] = { 'f', 'o', 'o', 0 }; return _wchdir(c);");

func("readlink", "defined(CONFIG_HAVE_UNISTD_H) && (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))", test: 'char buf[256]; return (int)readlink("foo", buf, 256);');
func("freadlinkat", "defined(CONFIG_HAVE_UNISTD_H) && defined(__USE_KOS) && defined(__CRT_HAVE_freadlinkat) && defined(AT_READLINK_REQSIZE)", test: 'char buf[256]; return (int)freadlinkat(AT_FDCWD, "foo", buf, 256, AT_READLINK_REQSIZE);');

func("stat", "defined(CONFIG_HAVE_SYS_STAT_H)", test: 'struct stat st; return stat("foo", &st);');
func("fstat", "defined(CONFIG_HAVE_SYS_STAT_H)", test: 'struct stat st; return fstat(1, &st);');
func("lstat", "defined(CONFIG_HAVE_SYS_STAT_H) && (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))", test: 'struct stat st; return lstat("foo", &st);');
func("stat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64)", test: 'struct stat64 st; return stat64("foo", &st);');
func("fstat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64)", test: 'struct stat64 st; return fstat64(1, &st);');
func("lstat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64) && (defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))", test: 'struct stat64 st; return lstat64("foo", &st);');
func("fstatat", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)", test: 'struct stat st; return fstatat(AT_FDCWD, "foo", &st, 0);');
func("fstatat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_LARGEFILE64) && defined(__USE_ATFILE)", test: 'struct stat64 st; return fstatat64(AT_FDCWD, "foo", &st, 0);');
func("_wstat", "defined(_WIO_DEFINED) || " + addparen(msvc), test: "struct stat st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return _wstat(c, &st);");
func("wstat", test: "struct stat st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return wstat(c, &st);");
func("_wstat64", "defined(_WIO_DEFINED) && defined(__USE_LARGEFILE64)", test: "struct stat64 st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return _wstat64(c, &st);");
func("wstat64", test: "struct stat64 st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return wstat64(c, &st);");
func("_wlstat", test: "struct stat st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return _wlstat(c, &st);");
func("wlstat", test: "struct stat st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return wlstat(c, &st);");
func("_wlstat64", test: "struct stat64 st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return _wlstat64(c, &st);");
func("wlstat64", test: "struct stat64 st; wchar_t c[] = { 'f', 'o', 'o', 0 }; return wlstat64(c, &st);");
// syscall_ulong_t st_[acm]timensec
feature("STAT_ST_NSEC", "defined(CONFIG_HAVE_stat) && (defined(_STATBUF_ST_NSEC) && !defined(__USE_XOPEN2K8))", test: "struct stat st; st.st_atimensec = st.st_ctimensec = st.st_mtimensec = 0; return 0;");
// struct timespec st_[acm]tim
feature("STAT_ST_TIM", "defined(CONFIG_HAVE_stat) && (defined(_STATBUF_ST_TIM) || (defined(_STATBUF_ST_NSEC) && defined(__USE_XOPEN2K8)))", test: "struct stat st; st.st_atim.tv_sec = st.st_ctim.tv_sec = st.st_mtim.tv_sec = 0; st.st_atim.tv_nsec = st.st_ctim.tv_nsec = st.st_mtim.tv_nsec = 0; return st.st_atim.tv_sec + st.st_ctim.tv_sec + st.st_mtim.tv_sec + st.st_atim.tv_nsec + st.st_ctim.tv_nsec + st.st_mtim.tv_nsec;");
// struct timespec st_[acm]timespec
feature("STAT_ST_TIMESPEC", "defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_TIMESPEC)", test: "struct stat st; st.st_atimespec.tv_sec = st.st_ctimespec.tv_sec = st.st_mtimespec.tv_sec = 0; st.st_atimespec.tv_nsec = st.st_ctimespec.tv_nsec = st.st_mtimespec.tv_nsec = 0; return st.st_atimespec.tv_sec + st.st_ctimespec.tv_sec + st.st_mtimespec.tv_sec + st.st_atimespec.tv_nsec + st.st_ctimespec.tv_nsec + st.st_mtimespec.tv_nsec;");

functest('mkdir("foo", 0755)', "defined(CONFIG_HAVE_SYS_STAT_H) && " + addparen(unix));
functest('_mkdir("foo")', msvc);
func("wmkdir", test: "wchar_t c[] = { 'f', 'o', 'o', 0 }; return wmkdir(c, 0755);");
func("_wmkdir", msvc, test: "wchar_t c[] = { 'f', 'o', 'o', 0 }; return _wmkdir(c);");
functest('chmod("foo", 0777)', "defined(CONFIG_HAVE_SYS_STAT_H) && " + addparen(unix));
functest('_chmod("foo", 0777)', msvc);
func("_wchmod", "defined(_WIO_DEFINED) || " + addparen(msvc), test: "wchar_t c[] = { 'f', 'o', 'o', 0 }; return _wchmod(c, 0777);");
functest('mkfifo("foo", 0666)', "defined(CONFIG_HAVE_SYS_STAT_H) && " + addparen(unix));
functest('lchmod("foo", 0666)', "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_MISC)");
functest('fchmodat(AT_FDCWD, "foo", 0666, 0)', "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
functest('mkdirat(AT_FDCWD, "foo", 0755)', "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
functest('fmkdirat(AT_FDCWD, "foo", 0755, 0)', "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_KOS) && defined(__USE_ATFILE)");
functest('mkfifoat(AT_FDCWD, "foo", 0666)', "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)");
functest('fchmod(1, 0644)', "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_POSIX)");
functest('mknod("foo", 0644, (dev_t)123)', "defined(CONFIG_HAVE_SYS_STAT_H) && (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED))");
functest('mknodat(AT_FDCWD, "foo", 0644, (dev_t)123)', "defined(CONFIG_HAVE_SYS_STAT_H) && (defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)) && defined(__USE_ATFILE)");
func("utimensat", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE)", test: 'struct timespec ts[2]; ts[0].tv_sec = 0; ts[0].tv_nsec = 0; ts[1] = ts[0]; return utimensat(AT_FDCWD, "foo", ts, 0);');
func("utimensat64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_ATFILE) && defined(__USE_TIME64)", test: 'struct timespec64 ts[2]; ts[0].tv_sec = 0; ts[0].tv_nsec = 0; ts[1] = ts[0]; return utimensat64(AT_FDCWD, "foo", ts, 0);');
func("futimens", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_XOPEN2K8)", test: 'struct timespec ts[2]; ts[0].tv_sec = 0; ts[0].tv_nsec = 0; ts[1] = ts[0]; return futimens(1, ts);');
func("futimens64", "defined(CONFIG_HAVE_SYS_STAT_H) && defined(__USE_XOPEN2K8) && defined(__USE_TIME64)", test: 'struct timespec64 ts[2]; ts[0].tv_sec = 0; ts[0].tv_nsec = 0; ts[1] = ts[0]; return futimens64(1, ts);');

functest("time(NULL)", "defined(CONFIG_HAVE_TIME_H)");
functest("time64(NULL)", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_TIME64)");
func("clock_gettime", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309)", test: "struct timespec ts; return clock_gettime(0, &ts);");
func("clock_gettime64", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309) && defined(__USE_TIME64)", test: "struct timespec64 ts; return clock_gettime64(0, &ts);");
constant("CLOCK_REALTIME", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309)");
func("gettimeofday", "defined(CONFIG_HAVE_SYS_TIME_H)", test: "struct timeval tv; return gettimeofday(&tv, NULL);");
func("gettimeofday64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_TIME64)", test: "struct timeval64 tv; return gettimeofday64(&tv, NULL);");
func("utimes", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_MISC)", test: 'struct timeval tv[2]; tv[0].tv_sec = 0; tv[0].tv_usec = 0; tv[1] = tv[0]; return utimes("foo", tv);');
func("utimes64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_MISC) && defined(__USE_TIME64)", test: 'struct timeval64 tv[2]; tv[0].tv_sec = 0; tv[0].tv_usec = 0; tv[1] = tv[0]; return utimes64("foo", tv);');
func("lutimes", "defined(CONFIG_HAVE_SYS_TIME_H)", test: 'struct timeval tv[2]; tv[0].tv_sec = 0; tv[0].tv_usec = 0; tv[1] = tv[0]; return lutimes("foo", tv);');
func("lutimes64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_TIME64)", test: 'struct timeval64 tv[2]; tv[0].tv_sec = 0; tv[0].tv_usec = 0; tv[1] = tv[0]; return lutimes64("foo", tv);');
func("futimesat", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_GNU)", test: 'struct timeval tv[2]; tv[0].tv_sec = 0; tv[0].tv_usec = 0; tv[1] = tv[0]; return futimesat(AT_FDCWD, "foo", tv);');
func("futimesat64", "defined(CONFIG_HAVE_SYS_TIME_H) && defined(__USE_GNU) && defined(__USE_TIME64)", test: 'struct timeval64 tv[2]; tv[0].tv_sec = 0; tv[0].tv_usec = 0; tv[1] = tv[0]; return futimesat(AT_FDCWD, "foo", tv);');


print "#ifdef _MSC_VER";
print "#define F_OK     0";
print "#define X_OK     1 // Not supported?";
print "#define W_OK     2";
print "#define R_OK     4";
print "#endif";
print;

functest('euidaccess("foo", F_OK)', "defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_GNU)");
functest('eaccess("foo", F_OK)', "defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_GNU)");
functest('faccessat(AT_FDCWD, "foo", F_OK, 0)', "defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK) && defined(__USE_ATFILE)");
functest('access("foo", F_OK)', "(defined(CONFIG_HAVE_UNISTD_H) || !" + addparen(msvc) + ") && defined(F_OK) && defined(X_OK) && defined(W_OK) && defined(R_OK)");
functest('_access("foo", F_OK)', msvc);
func("_waccess", "defined(_WIO_DEFINED) || " + addparen(msvc), test: "wchar_t c[] = { 'f', 'o', 'o', 0 }; return _waccess(c, F_OK);");

functest('fchownat(AT_FDCWD, "foo", 0, 0, 0)', "defined(__USE_ATFILE)");

func("pread", "defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)", test: "char buf[7]; return (int)pread(1, buf, 7, 1234);");
func("pwrite", "defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)", test: 'char const buf[] = "foo"; return (int)pwrite(1, buf, 3, 1234);');
func("pread64", "defined(__USE_LARGEFILE64) && (defined(__USE_UNIX98) || defined(__USE_XOPEN2K8))", test: "char buf[7]; return (int)pread64(1, buf, 7, 1234);");
func("pwrite64", "defined(__USE_LARGEFILE64) && (defined(__USE_UNIX98) || defined(__USE_XOPEN2K8))", test: 'char const buf[] = "foo"; return (int)pwrite64(1, buf, 3, 1234);');

functest("close(1)", unix);
functest("_close(1)", msvc);

func("sync", unix, test: "sync(); return 0;");
functest("fsync(1)", isenabled("_POSIX_FSYNC") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + ")");
functest("fdatasync(1)", unix);
functest("_commit(1)", msvc);

functest("getpid()", unix);
functest("_getpid()", msvc);

functest("umask(0111)", "defined(CONFIG_HAVE_SYS_STAT_H)");
functest("_umask(0111)", msvc);

functest("dup(1)", unix);
functest("_dup(1)", msvc);

functest("dup2(1, 2)", unix);
functest("_dup2(1, 2)", msvc);

functest("dup3(1, 2, 0)", "defined(__USE_GNU)");

functest("isatty(1)", unix);
functest("_isatty(1)", msvc);

func("getcwd", unix, test: 'char buf[256]; char *p = getcwd(buf, 256); return p != 0;');
func("_getcwd", msvc, test: 'char buf[256]; char *p = _getcwd(buf, 256); return p != 0;');
func("wgetcwd", test: 'wchar_t buf[256]; wchar_t *p = wgetcwd(buf, 256); return p != 0;');
func("_wgetcwd", "defined(_WDIRECT_DEFINED)", test: 'wchar_t buf[256]; wchar_t *p = _wgetcwd(buf, 256); return p != 0;');

functest('unlink("foo.txt")', unix);
functest('_unlink("foo.txt")', "defined(_CRT_DIRECTORY_DEFINED)");
functest('rmdir("foo")', "defined(CONFIG_HAVE_UNISTD_H)");
functest('_rmdir("foo")', "defined(CONFIG_HAVE_DIRECT_H)");
functest('unlinkat(AT_FDCWD, "foo", AT_REMOVEDIR)', "defined(CONFIG_HAVE_UNISTD_H) && defined(__USE_ATFILE)");
functest('remove("foo.txt")', "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
functest('rename("foo.txt", "bar.txt")', "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc));
func("wunlink", test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; return _wunlink(s);");
func("_wunlink", "defined(_WIO_DEFINED) || " + addparen(msvc), test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; return _wunlink(s);");
func('wrmdir', test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; return wrmdir(s);");
func('_wrmdir', "defined(_WIO_DEFINED) || " + addparen(msvc), test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; return _wrmdir(s);");
func("wremove", test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; return _wremove(s);");
func("_wremove", "defined(_WSTDIO_DEFINED)", test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; return _wremove(s);");
func("wrename", test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; wchar_t t[] = { 'b', 'a', 'r', '.', 't', 'x', 't', 0 }; return wrename(s, t);");
func("_wrename", "defined(_WIO_DEFINED) || " + addparen(msvc), test: "wchar_t s[] = { 'f', 'o', 'o', '.', 't', 'x', 't', 0 }; wchar_t t[] = { 'b', 'a', 'r', '.', 't', 'x', 't', 0 }; return _wrename(s, t);");

func("getenv", "defined(CONFIG_HAVE_STDLIB_H) && " + addparen(stdc), test: 'return getenv("PATH") ? 0 : 1;');

func("wcslen", "defined(CONFIG_HAVE_WCHAR_H) && " + addparen(stdc), test: "wchar_t s[] = { 'a', 'b', 'c', 0 }; return (int)wcslen(s);");

func("qsort", "defined(CONFIG_HAVE_STDLIB_H) && " + addparen(stdc), test: 'extern int cmpch(void const *a, void const *b); char buf[] = "foobar"; qsort(buf, 6, 1, &cmpch); return 0;');

functest('truncate("foo.txt", 42)', addparen(unix) + " || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)");
functest('truncate64("foo.txt", 42)', "defined(__USE_LARGEFILE64) && (" + addparen(unix) + " || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8))");
functest("ftruncate(1, 42)", addparen(unix) + " || defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)");
functest("ftruncate64(1, 42)", "defined(__USE_LARGEFILE64) && (" + addparen(unix) + " || defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K))");
functest("_chsize(1, 42)", msvc);
functest("_chsize_s(1, 42)", msvc);

functest("getpgid(0)", isenabled("_POSIX_JOB_CONTROL"));
functest("setpgid(0, 0)", isenabled("_POSIX_JOB_CONTROL"));
functest("setreuid(0, 0)", isenabled("_POSIX_SAVED_IDS") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + ")");
functest("nice(0)", "defined(__USE_MISC) || defined(__USE_XOPEN) || (" + isenabled("_POSIX_PRIORITY_SCHEDULING") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + "))");

func("mmap", isenabled("_POSIX_MAPPED_FILES") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + ")", test: "return mmap(NULL, 1, 0, 0, -1, 0) == (void *)0;");
func("mmap64", "defined(__USE_LARGEFILE64) && (" + isenabled("_POSIX_MAPPED_FILES") + " || (!defined(CONFIG_HAVE_UNISTD_H) && " + addparen(unix) + "))", test: "return mmap64(NULL, 1, 0, 0, -1, 0) == (void *)0;");
func("munmap", "CONFIG_HAVE_mmap", test: 'char buf[] = "foobar"; return munmap(buf, 6);');
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

func("pipe", "defined(CONFIG_HAVE_UNISTD_H) || " + addparen(unix), "int fds[2]; return pipe(fds);");
func("pipe2", "defined(__USE_GNU)", "int fds[2]; return pipe2(fds, 0);");
func("_pipe", msvc, "int fds[2]; return _pipe(fds, 4096, 0);");

functest("usleep(42)", "defined(CONFIG_HAVE_UNISTD_H) && ((defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || defined(__USE_MISC))");
typ("useconds_t", "defined(CONFIG_HAVE_UNISTD_H) && (defined(__USE_XOPEN) || defined(__USE_XOPEN2K))");
func("nanosleep", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309)", test: "struct timespec ts; ts.tv_sec = 0; ts.tv_nsec = 123; return nanosleep(&ts, NULL);");
func("nanosleep64", "defined(CONFIG_HAVE_TIME_H) && defined(__USE_POSIX199309) && defined(__USE_TIME64)", test: "struct timespec64 ts; ts.tv_sec = 0; ts.tv_nsec = 123; return nanosleep64(&ts, NULL);");

functest("fork()", unix);
func("vfork", "(defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || defined(__USE_MISC)", test: "if (vfork() == 0) for (;;); return 0;");
functest("fchown(1, 0, 0)", "defined(CONFIG_HAVE_UNISTD_H) || " + addparen(unix));
functest("fchdir(1)", "defined(CONFIG_HAVE_UNISTD_H) || " + addparen(unix));

functest("pause()", unix);
functest("select(0, NULL, NULL, NULL, NULL)", "defined(CONFIG_HAVE_SYS_SELECT_H) && " + addparen(unix));
functest("pselect(0, NULL, NULL, NULL, (struct timespec *)0, (sigset_t *)0)", "defined(CONFIG_HAVE_SYS_SELECT_H) && " + addparen(unix));

func("__iob_func", addparen(msvc) + " && !defined(_ACRTIMP_ALT)", test: "return __iob_func() == (FILE *)0;");
func("__acrt_iob_func", addparen(msvc) + " && defined(_ACRTIMP_ALT)", test: "return __acrt_iob_func(0) == (FILE *)0;");

func("fseek", "defined(CONFIG_HAVE_STDIO_H)", test: "extern FILE *fp; return fseek(fp, 0, SEEK_SET);");
func("ftell", "defined(CONFIG_HAVE_STDIO_H)", test: "extern FILE *fp; return ftell(fp) != 0;");
func("fseek64", "defined(CONFIG_HAVE_STDIO_H) && 0", test: "extern FILE *fp; return fseek64(fp, 0, SEEK_SET);");
func("ftell64", "defined(CONFIG_HAVE_STDIO_H) && 0", test: "extern FILE *fp; return ftell64(fp) != 0;");
func("_fseek64", "defined(CONFIG_HAVE_STDIO_H) && 0", test: "extern FILE *fp; return _fseek64(fp, 0, SEEK_SET);");
func("_ftell64", "defined(CONFIG_HAVE_STDIO_H) && 0", test: "extern FILE *fp; return _ftell64(fp) != 0;");
func("fseeko", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(unix), test: "extern FILE *fp; return fseeko(fp, 0, SEEK_SET);");
func("ftello", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(unix), test: "extern FILE *fp; return ftello(fp) != 0;");
func("fseeko64", "defined(CONFIG_HAVE_STDIO_H) && defined(__USE_LARGEFILE64)", test: "extern FILE *fp; return fseeko64(fp, 0, SEEK_SET);");
func("ftello64", "defined(CONFIG_HAVE_STDIO_H) && defined(__USE_LARGEFILE64)", test: "extern FILE *fp; return ftello64(fp) != 0;");
func("_fseeki64", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(msvc), test: "extern FILE *fp; return _fseeki64(fp, 0, SEEK_SET);");
func("_ftelli64", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(msvc), test: "extern FILE *fp; return _ftelli64(fp) != 0;");
func("fflush", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return fflush(fp);");
func("ferror", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return ferror(fp);");
func("fclose", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return fclose(fp);");
func("fileno", "defined(CONFIG_HAVE_STDIO_H) && " + "!" + addparen(msvc), test: "extern FILE *fp; return fileno(fp);");
func("_fileno", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(msvc), test: "extern FILE *fp; return _fileno(fp);");
func("fftruncate", "defined(__USE_KOS)", test: "extern FILE *fp; return fftruncate(fp, 0);");
func("fftruncate64", "defined(__USE_KOS) && defined(__USE_LARGEFILE64)", test: "extern FILE *fp; return fftruncate64(fp, 0);");
func("getc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return getc(fp);");
func("fgetc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return fgetc(fp);");
func("putc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return putc('!', fp);");
func("fputc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return fputc('!', fp);");
func("fread", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; int buf[4]; return fread(buf, sizeof(int), 4, fp) == 4;");
func("fwrite", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: 'extern FILE *fp; char buf[] = "text"; return fwrite(buf, sizeof(char), 4, fp) == 4;');
func("ungetc", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; return ungetc('!', fp);");
func("setvbuf", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "extern FILE *fp; extern char buf[256]; return setvbuf(fp, buf, 0, 256);");
constant("_IONBF");
constant("_IOFBF");
constant("_IOLBF");
func("fopen", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: 'return fopen("foo.txt", "r") != (FILE *)0;');
func("fopen64", "defined(CONFIG_HAVE_STDIO_H) && defined(__USE_LARGEFILE64)", test: 'return fopen64("foo.txt", "r") != (FILE *)0;');
func("fprintf", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: 'extern FILE *fp; return (int)fprintf(fp, "foo = %d", 42);');
var("stdin", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "return stdin != (FILE *)0;");
var("stdout", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "return stdout != (FILE *)0;");
var("stderr", "defined(CONFIG_HAVE_STDIO_H) && " + addparen(stdc), test: "return stderr != (FILE *)0;");

func("sem_init", "defined(CONFIG_HAVE_SEMAPHORE_H)", test: "extern sem_t sem; return sem_init(&sem, 0, 0);");
func("sem_destroy", "defined(CONFIG_HAVE_SEMAPHORE_H)", test: "extern sem_t sem; return sem_destroy(&sem);");
func("sem_wait", "defined(CONFIG_HAVE_SEMAPHORE_H)", test: "extern sem_t sem; return sem_wait(&sem);");
func("sem_trywait", "defined(CONFIG_HAVE_SEMAPHORE_H)", test: "extern sem_t sem; return sem_trywait(&sem);");
func("sem_post", "defined(CONFIG_HAVE_SEMAPHORE_H)", test: "extern sem_t sem; return sem_post(&sem);");
func("sem_timedwait", "defined(CONFIG_HAVE_SEMAPHORE_H) && defined(__USE_XOPEN2K)", test: "extern sem_t sem; extern struct timespec ts; return sem_timedwait(&sem, &ts);");
func("sem_timedwait64", "defined(CONFIG_HAVE_SEMAPHORE_H) && defined(__USE_XOPEN2K) && defined(__USE_TIME64)", test: "extern sem_t sem; extern struct timespec64 ts; return sem_timedwait64(&sem, &ts);");

func("pthread_suspend", "defined(CONFIG_HAVE_PTHREAD_H) && 0", test: "extern pthread_t pt; return pthread_suspend(pt);");
func("pthread_continue", "defined(CONFIG_HAVE_PTHREAD_H) && 0", test: "extern pthread_t pt; return pthread_continue(pt);");
func("pthread_suspend_np", "defined(CONFIG_HAVE_PTHREAD_H) && 0", test: "extern pthread_t pt; return pthread_suspend_np(pt);");
func("pthread_unsuspend_np", "defined(CONFIG_HAVE_PTHREAD_H) && 0", test: "extern pthread_t pt; return pthread_unsuspend_np(pt);");
func("pthread_setname", "defined(CONFIG_HAVE_PTHREAD_H) && 0", test: 'extern pthread_t pt; return pthread_setname(pt, "foothread");');
func("pthread_setname_np", "defined(CONFIG_HAVE_PTHREAD_H) && defined(__USE_GNU)", test: 'extern pthread_t pt; return pthread_setname_np(pt, "foothread");');

func("abort", "defined(CONFIG_HAVE_STDLIB_H) && " + addparen(stdc), test: "abort();");
func("strerror", "defined(CONFIG_HAVE_STRING_H) && " + addparen(stdc), test: "char *p = strerror(1); return p != NULL;");
func("strerror_s", "defined(__USE_KOS)", test: "char const *p = strerror_s(1); return p != NULL;");
func("strerrorname_s", "defined(__USE_KOS)", test: "char const *p = strerrorname_s(1); return p != NULL;");

func("dlopen", "defined(CONFIG_HAVE_DLFCN_H)", test: 'extern void *dl; dl = dlopen("foo.so", 0); return dl != NULL;');
func("dlclose", "defined(CONFIG_HAVE_DLFCN_H)", test: 'extern void *dl; return dlclose(dl);');
func("dlsym", "defined(CONFIG_HAVE_DLFCN_H)", test: 'extern void *dl; void *s = dlsym(dl, "foo"); return s != NULL;');
func("dlmodulename", "defined(CONFIG_HAVE_DLFCN_H) && defined(__USE_KOS)", test: 'extern void *dl; char const *n = dlmodulename(dl); return n != NULL;');
constant("RTLD_GLOBAL");
constant("RTLD_LOCAL");
constant("RTLD_LAZY");
constant("RTLD_NOW");

functest('_memicmp("a", "A", 1)', msvc);
functest('memicmp("a", "A", 1)', msvc);
functest('memcasecmp("a", "A", 1)', "defined(__USE_KOS)");

functest('_stricmp("a", "A")', msvc);
functest('_strcmpi("a", "A")', msvc);
functest('stricmp("a", "A")', msvc);
functest('strcmpi("a", "A")', msvc);
functest('strcasecmp("a", "A")', "defined(__USE_KOS)");

func("memchr", stdc, test: "extern char *buf; void *p = memchr(buf, '!', 123); return p != NULL;");
func("memrchr", "defined(__USE_GNU)", test: "extern char *buf; void *p = memrchr(buf, '!', 123); return p != NULL;");
func("rawmemchr", "defined(__USE_GNU)", test: "extern char *buf; void *p = rawmemchr(buf, '!'); return p == buf;");
functest('atoi("42")', stdc);
functest('strlen("foo")', stdc);
functest('strchr("foo", 102)', stdc);
functest('strrchr("foo", 102)', stdc);
functest('strnlen("foo", 3)', "defined(__USE_XOPEN2K8) || defined(__USE_DOS) || (defined(_MSC_VER) && !defined(__KOS_SYSTEM_HEADERS__))");

func("bzero", "defined(CONFIG_HAVE_STRINGS_H)", test: "extern void *a; bzero(a, 42); return 0;");

// NOTE: The GNU-variant of memmem() returns the start of the haystack
//       when `needle_length == 0', however for this case, deemon requires
//       that `NULL' be returned, as deemon considers an empty string not
//       to be contained ~in-between two other characters~
// KOS provides this behavior when given the `_MEMMEM_EMPTY_NEEDLE_SOURCE' option,
// which reports back an ACK in the form of `__USE_MEMMEM_EMPTY_NEEDLE_NULL'
// NOTE: Because we need the specific KOS-variant of this (and because the KOS variant
//       is guarantied to report back its presence with `__USE_MEMMEM_EMPTY_NEEDLE_NULL'),
//       we don't include these configure options as part of the autoconf testing (which
//       we do by wrapping `func' with parenthesis so that `./configure' can't identify it
//       as a configure test)
(func)("memmem", "defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL)", check_defined: false);
(func)("memrmem", "defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL)", check_defined: false);
(func)("memcasemem", "defined(__USE_KOS) && defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL)", check_defined: false);
(func)("memcasermem", "defined(__memcasermem_defined) && defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL)", check_defined: false);

(func)("memmemw", "0", check_defined: false);
(func)("memmeml", "0", check_defined: false);
(func)("memmemq", "0", check_defined: false);
(func)("memrmemw", "0", check_defined: false);
(func)("memrmeml", "0", check_defined: false);
(func)("memrmemq", "0", check_defined: false);

func("memcpy", stdc, test: "extern void *a; extern void const *b; return memcpy(a, b, 16) == a;");
func("memset", stdc, test: "extern void *a; return memset(a, 0, 42) == a;");
func("memmove", stdc, test: "extern void *a; extern void const *b; return memmove(a, b, 16) == a;");
func("memcmp", stdc, test: "extern void const *a, *b; return memcmp(a, b, 16) == 0;");

func("memcpyc", "defined(__USE_KOS)", test: "extern char *a, *b; memcpyc(a, b, 16, sizeof(char)); return 0;");
func("memcpyw", "defined(__USE_KOS)", test: "extern void *a, *b; memcpyw(a, b, 16); return 0;");
func("memcpyl", "defined(__USE_KOS)", test: "extern void *a, *b; memcpyl(a, b, 16); return 0;");
func("memcpyq", "defined(__USE_KOS)", test: "extern void *a, *b; memcpyq(a, b, 16); return 0;");
func("memmovec", "defined(__USE_KOS)", test: "extern char *a, *b; memmovec(a, b, 16, sizeof(char)); return 0;");
func("memmovew", "defined(__USE_KOS)", test: "extern void *a, *b; memmovew(a, b, 16); return 0;");
func("memmovel", "defined(__USE_KOS)", test: "extern void *a, *b; memmovel(a, b, 16); return 0;");
func("memmoveq", "defined(__USE_KOS)", test: "extern void *a, *b; memmoveq(a, b, 16); return 0;");
func("memmoveup", "defined(__USE_KOS)", test: "extern void *a, *b; memmoveup(a, b, 16); return 0;");
func("memmoveupc", "defined(__USE_KOS)", test: "extern char *a, *b; memmoveupc(a, b, 16, sizeof(char)); return 0;");
func("memmoveupw", "defined(__USE_KOS)", test: "extern void *a, *b; memmoveupw(a, b, 16); return 0;");
func("memmoveupl", "defined(__USE_KOS)", test: "extern void *a, *b; memmoveupl(a, b, 16); return 0;");
func("memmoveupq", "defined(__USE_KOS)", test: "extern void *a, *b; memmoveupq(a, b, 16); return 0;");
func("memmovedown", "defined(__USE_KOS)", test: "extern void *a, *b; memmovedown(a, b, 16); return 0;");
func("memmovedownc", "defined(__USE_KOS)", test: "extern char *a, *b; memmovedownc(a, b, 16, sizeof(char)); return 0;");
func("memmovedownw", "defined(__USE_KOS)", test: "extern void *a, *b; memmovedownw(a, b, 16); return 0;");
func("memmovedownl", "defined(__USE_KOS)", test: "extern void *a, *b; memmovedownl(a, b, 16); return 0;");
func("memmovedownq", "defined(__USE_KOS)", test: "extern void *a, *b; memmovedownq(a, b, 16); return 0;");
func("memsetw", "defined(__USE_KOS)", test: "extern void *a; memsetw(a, 0xcc, 16); return 0;");
func("memsetl", "defined(__USE_KOS)", test: "extern void *a; memsetl(a, 0xcc, 16); return 0;");
func("memsetq", "defined(__USE_KOS)", test: "extern void *a; memsetq(a, 0xcc, 16); return 0;");
func("memchrw", "defined(__USE_KOS)", test: "extern void const *a; return memchrw(a, 0xcc, 16) != 0;");
func("memchrl", "defined(__USE_KOS)", test: "extern void const *a; return memchrl(a, 0xcc, 16) != 0;");
func("memchrq", "defined(__USE_KOS)", test: "extern void const *a; return memchrq(a, 0xcc, 16) != 0;");
func("memrchrw", "defined(__USE_KOS)", test: "extern void const *a; return memrchrw(a, 0xcc, 16) != 0;");
func("memrchrl", "defined(__USE_KOS)", test: "extern void const *a; return memrchrl(a, 0xcc, 16) != 0;");
func("memrchrq", "defined(__USE_KOS)", test: "extern void const *a; return memrchrq(a, 0xcc, 16) != 0;");
func("memcmpw", "defined(__USE_KOS)", test: "extern void const *a, *b; return memcmpw(a, b, 16) == 0;");
func("memcmpl", "defined(__USE_KOS)", test: "extern void const *a, *b; return memcmpl(a, b, 16) == 0;");
func("memcmpq", "defined(__USE_KOS)", test: "extern void const *a, *b; return memcmpq(a, b, 16) == 0;");

func("rawmemrchr", "defined(__USE_KOS)", test: "extern void const *buf; void *p = rawmemrchr(buf, '!'); return p == buf - 1;");
func("memend", "defined(__USE_KOS)", test: "extern void const *buf; void *p = memend(buf, '!', 123); return p == buf;");
func("memrend", "defined(__USE_KOS)", test: "extern void const *buf; void *p = memrend(buf, '!', 123); return p == buf;");
func("memlen", "defined(__USE_KOS)", test: "extern void const *buf; size_t s = memlen(buf, '!', 123); return s == 0;");
func("memrlen", "defined(__USE_KOS)", test: "extern void const *buf; size_t s = memrlen(buf, '!', 123); return s == 0;");
func("rawmemlen", "defined(__USE_KOS)", test: "extern void const *buf; size_t s = rawmemlen(buf, '!'); return s == 0;");
func("rawmemrlen", "defined(__USE_KOS)", test: "extern void const *buf; size_t s = rawmemrlen(buf, '!'); return s == 0;");
func("memrev", "defined(__USE_KOS)", test: "extern void *buf; void *p = memrev(buf, 123); return p == buf;");
func("strend", "defined(__USE_KOS)", test: "extern char const *buf; char *p = strend(buf); return p == buf + 123;");
func("strnend", "defined(__USE_KOS)", test: "extern char const *buf; char *p = strnend(buf, 3); return p == buf + 3;");

func("memxend", "defined(__USE_STRING_XCHR)", test: "extern void const *buf; void *p = memxend(buf, '!', 123); return p == buf;");
func("memxlen", "defined(__USE_STRING_XCHR)", test: "extern void const *buf; size_t s = memxlen(buf, '!', 123); return s == 0;");
func("memxchr", "defined(__USE_STRING_XCHR)", test: "extern void const *buf; void *p = memxchr(buf, '!', 123); return p == buf;");
func("rawmemxchr", "defined(__USE_STRING_XCHR)", test: "extern void const *buf; void *p = rawmemxchr(buf, '!'); return p == buf;");
func("rawmemxlen", "defined(__USE_STRING_XCHR)", test: "extern void const *buf; size_t s = rawmemxlen(buf, '!'); return s == 0;");
func("memxrchr", test: "extern void const *buf; void *p = memxrchr(buf, '!', 123); return p != NULL;");
func("memxrend", test: "extern void const *buf; void *p = memxrend(buf, '!', 123); return p == buf;");
func("memxrlen", test: "extern void const *buf; size_t s = memxrlen(buf, '!', 123); return s == 0;");
func("rawmemxrchr", test: "extern void const *buf; void *p = rawmemxrchr(buf, '!'); return p == buf;");
func("rawmemxrlen", test: "extern void const *buf; size_t s = rawmemxrlen(buf, '!'); return s == 0;");

functest("tolower('!')", "defined(CONFIG_HAVE_CTYPE_H)");
functest("toupper('!')", "defined(CONFIG_HAVE_CTYPE_H)");
functest("islower('!')", "defined(CONFIG_HAVE_CTYPE_H)");
functest("isupper('!')", "defined(CONFIG_HAVE_CTYPE_H)");
functest("isdigit('!')", "defined(CONFIG_HAVE_CTYPE_H)");
functest("isalpha('!')", "defined(CONFIG_HAVE_CTYPE_H)");
functest("isalnum('!')", "defined(CONFIG_HAVE_CTYPE_H)");

// CRT-specific functions
func("_dosmaperr", msvc + " || (defined(__CRT_DOS) && defined(__CRT_HAVE__dosmaperr))", check_defined: false, test: "_dosmaperr(42); return 0;");
functest("errno_nt2kos(42)", "defined(__CRT_HAVE_errno_nt2kos)", check_defined: false);
functest("errno_kos2nt(42)", "defined(__CRT_HAVE_errno_kos2nt)", check_defined: false);
func("_get_osfhandle", msvc + " || defined(__CYGWIN__) || defined(__CYGWIN32__)", test: "intptr_t fh = _get_osfhandle(1); return fh != -1;");
func("get_osfhandle", "defined(__CYGWIN__) || defined(__CYGWIN32__)", test: "intptr_t fh = get_osfhandle(1); return fh != -1;");
functest("open_osfhandle(1234, 0)");
functest("_open_osfhandle(1234, 0)", msvc);
var("errno", "defined(CONFIG_HAVE_ERRNO_H)");
var("_errno");
var("__errno");
var("_doserrno", msvc);
var("doserrno");

var("environ");
var("_environ");
var("__environ");

functest("sysconf(0)", "defined(CONFIG_HAVE_UNISTD_H) || " + addparen(unix));
functest("_sysconf(0)");
functest("mpctl(0, 0, 0)", "defined(__hpux__)");
functest("sysctl(0, 0, 0, 0, 0, 0)");

constant("_SC_NPROCESSORS_ONLN");
constant("_SC_NPROC_ONLN");
constant("MPC_GETNUMSPUS", "defined(__hpux__)");
constant("CTL_HW");
constant("HW_AVAILCPU");
constant("HW_NCPU");

func("alloca", "defined(CONFIG_HAVE_ALLOCA_H)", test: "void *p = alloca(42); return p != 0;");
func("_alloca", msvc, test: "void *p = _alloca(42); return p != 0;");

// Math functions
functest("fabs(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("sin(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("cos(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("tan(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("asin(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("acos(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("atan(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("sinh(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("cosh(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("tanh(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("asinh(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("acosh(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("atanh(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("copysign(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("atan2(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("exp(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("exp2(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("expm1(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("erf(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("erfc(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("fabs(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("sqrt(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("log(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("log2(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("logb(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("log1p(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("log10(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("cbrt(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("tgamma(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("lgamma(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("pow(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("ceil(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("floor(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("round(1.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("fmod(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("hypot(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("remainder(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("nextafter(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
functest("fdim(1.0, 2.0)", "defined(CONFIG_HAVE_MATH_H)");
func("isnan", "defined(CONFIG_HAVE_MATH_H)", test: "return isnan(1.0) != 0;");
func("isinf", "defined(CONFIG_HAVE_MATH_H)", test: "return isinf(1.0) != 0;");
func("isfinite", "defined(CONFIG_HAVE_MATH_H)", test: "return isfinite(1.0) != 0;");
func("isnormal", "defined(CONFIG_HAVE_MATH_H)", test: "return isnormal(1.0) != 0;");
func("signbit", "defined(CONFIG_HAVE_MATH_H)", test: "return signbit(1.0) != 0;");
func("isgreater", "defined(CONFIG_HAVE_MATH_H)", test: "return isgreater(1.0, 2.0) != 0;");
func("isgreaterequal", "defined(CONFIG_HAVE_MATH_H)", test: "return isgreaterequal(1.0, 2.0) != 0;");
func("isless", "defined(CONFIG_HAVE_MATH_H)", test: "return isless(1.0, 2.0) != 0;");
func("islessequal", "defined(CONFIG_HAVE_MATH_H)", test: "return islessequal(1.0, 2.0) != 0;");
func("islessgreater", "defined(CONFIG_HAVE_MATH_H)", test: "return islessgreater(1.0, 2.0) != 0;");
func("isunordered", "defined(CONFIG_HAVE_MATH_H)", test: "return isunordered(1.0, 2.0) != 0;");
func("ilogb", "defined(CONFIG_HAVE_MATH_H)", test: "return ilogb(1.0) != 0;");
func("frexp", "defined(CONFIG_HAVE_MATH_H)", test: "extern int ex; return frexp(1.0, &ex) != 0.0;");
func("modf", "defined(CONFIG_HAVE_MATH_H)", test: "extern double y; return modf(1.0, &y) != 0.0;");
func("ldexp", "defined(CONFIG_HAVE_MATH_H)", test: "return ldexp(1.0, 1) != 0.0;");
func("sincos", "defined(CONFIG_HAVE_MATH_H) && defined(__USE_GNU)", test: "extern double x, y, z; sincos(x, &y, &z); return 0;");
func("asincos", "", test: "extern double x, y, z; asincos(x, &y, &z); return 0;");
func("sincosh", "", test: "extern double x, y, z; sincosh(x, &y, &z); return 0;");
func("asincosh", "", test: "extern double x, y, z; asincosh(x, &y, &z); return 0;");
func("scalbn", "defined(CONFIG_HAVE_MATH_H)", test: "return scalbn(1.0, 1) != 0.0;");
func("scalbln", "defined(CONFIG_HAVE_MATH_H)", test: "return scalbln(1.0, 1L) != 0.0;");
func("remquo", "defined(CONFIG_HAVE_MATH_H)", test: "extern double x, y; extern int z; return remquo(x, y, &z) != 0.0;");

sizeof("off_t");

// unistd.h and stdlib.h
func("realpath", "", test: 'extern char const path[]; extern char buf[]; char c = *realpath(path, buf); return c != 0;');
func("frealpath", "", test: 'extern char buf[]; char c = *frealpath(2, buf, 42); return c != 0;');
func("frealpath4", "", test: 'extern char buf[]; char c = *frealpath4(2, buf, 42, 0); return c != 0;');
func("frealpathat", "", test: 'extern char buf[]; char c = *frealpathat(2, "foobar", buf, 42, 0); return c != 0;');
func("resolvepath", "", test: 'extern char buf[]; return resolvepath("/foobar", buf, 42);');

// dirent.h
constant("DT_UNKNOWN", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_FIFO", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_CHR", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_DIR", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_BLK", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_REG", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_LNK", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_SOCK", "defined(CONFIG_HAVE_DIRENT_H)");
constant("DT_WHT", "defined(CONFIG_HAVE_DIRENT_H)");
func("IFTODT", "defined(CONFIG_HAVE_DIRENT_H)", test: "return IFTODT(42);");
func("DTTOIF", "defined(CONFIG_HAVE_DIRENT_H)", test: "return DTTOIF(42);");
func("opendir", "defined(CONFIG_HAVE_DIRENT_H)", test: 'DIR *d = opendir("/"); return d != 0;');
func("fdopendir", "", test: 'DIR *d = fdopendir(1); return d != 0;');
func("closedir", "", test: 'extern DIR *d; return closedir(d);');
func("readdir", "", test: 'extern DIR *d; struct dirent *e = readdir(d); return e != 0;');
func("readdir64", "", test: 'extern DIR *d; struct dirent64 *e = readdir64(d); return e != 0;');
func("dirfd", "", test: 'extern DIR *d; return dirfd(d);');
feature("DIRENT_D_INO", "defined(CONFIG_HAVE_readdir) && (defined(__linux__) || defined(__KOS__) || defined(_DIRENT_HAVE_D_INO))", test: "extern struct dirent *e; return e->d_ino != 0;");
feature("DIRENT_D_FILENO", "defined(CONFIG_HAVE_readdir) && (defined(d_fileno) || defined(_DIRENT_HAVE_D_FILENO))", test: "extern struct dirent *e; return e->d_fileno != 0;");
feature("DIRENT_D_OFF", "defined(CONFIG_HAVE_readdir) && (defined(d_off) || defined(_DIRENT_HAVE_D_OFF))", test: "extern struct dirent *e; return e->d_off != 0;");
feature("DIRENT_D_NAMLEN", "defined(CONFIG_HAVE_readdir) && (defined(d_namlen) || defined(_DIRENT_HAVE_D_NAMLEN))", test: "extern struct dirent *e; return e->d_namlen != 0;");
feature("DIRENT_D_RECLEN", "defined(CONFIG_HAVE_readdir) && (defined(d_reclen) || defined(_DIRENT_HAVE_D_RECLEN))", test: "extern struct dirent *e; return e->d_reclen != 0;");
feature("DIRENT_D_TYPE", "defined(CONFIG_HAVE_readdir) && (defined(d_type) || defined(_DIRENT_HAVE_D_TYPE))", test: "extern struct dirent *e; return e->d_type != 0;");
feature("DIRENT_D_TYPE_SZ_1", "", test: "extern struct dirent *e; extern int x[sizeof(e->d_type) == 1 ? 1 : -1]; return x[0];");
feature("DIRENT_D_TYPE_SZ_2", "", test: "extern struct dirent *e; extern int x[sizeof(e->d_type) == 2 ? 1 : -1]; return x[0];");
feature("DIRENT_D_TYPE_SZ_4", "", test: "extern struct dirent *e; extern int x[sizeof(e->d_type) == 4 ? 1 : -1]; return x[0];");



//END:FEATURES

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

#ifdef CONFIG_NO_ALLOCA_H
#undef CONFIG_HAVE_ALLOCA_H
#elif !defined(CONFIG_HAVE_ALLOCA_H) && \
      (__has_include(<alloca.h>) || (defined(__NO_has_include) && ((defined(__CYGWIN__) || \
       defined(__CYGWIN32__)) || (defined(__linux__) || defined(__linux) || defined(linux)) || \
       defined(__KOS__))))
#define CONFIG_HAVE_ALLOCA_H 1
#endif

#ifdef CONFIG_NO_MALLOC_H
#undef CONFIG_HAVE_MALLOC_H
#elif !defined(CONFIG_HAVE_MALLOC_H) && \
      (__has_include(<malloc.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       (defined(__CYGWIN__) || defined(__CYGWIN32__)) || (defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_MALLOC_H 1
#endif

#ifdef CONFIG_NO_IOCTL_H
#undef CONFIG_HAVE_IOCTL_H
#elif !defined(CONFIG_HAVE_IOCTL_H) && \
      (__has_include(<ioctl.h>))
#define CONFIG_HAVE_IOCTL_H 1
#endif

#ifdef CONFIG_NO_DIRENT_H
#undef CONFIG_HAVE_DIRENT_H
#elif !defined(CONFIG_HAVE_DIRENT_H) && \
      (__has_include(<dirent.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_DIRENT_H 1
#endif

#ifdef CONFIG_NO_UNISTD_H
#undef CONFIG_HAVE_UNISTD_H
#elif !defined(CONFIG_HAVE_UNISTD_H) && \
      (__has_include(<unistd.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_UNISTD_H 1
#endif

#ifdef CONFIG_NO_SYS_UNISTD_H
#undef CONFIG_HAVE_SYS_UNISTD_H
#elif !defined(CONFIG_HAVE_SYS_UNISTD_H) && \
      (__has_include(<sys/unistd.h>) || (defined(__NO_has_include) && (defined(__CYGWIN__) || \
       defined(__CYGWIN32__))))
#define CONFIG_HAVE_SYS_UNISTD_H 1
#endif

#ifdef CONFIG_NO_ERRNO_H
#undef CONFIG_HAVE_ERRNO_H
#elif !defined(CONFIG_HAVE_ERRNO_H) && \
      (__has_include(<errno.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix)))))
#define CONFIG_HAVE_ERRNO_H 1
#endif

#ifdef CONFIG_NO_SYS_ERRNO_H
#undef CONFIG_HAVE_SYS_ERRNO_H
#elif !defined(CONFIG_HAVE_SYS_ERRNO_H) && \
      (__has_include(<sys/errno.h>))
#define CONFIG_HAVE_SYS_ERRNO_H 1
#endif

#ifdef CONFIG_NO_STDIO_H
#undef CONFIG_HAVE_STDIO_H
#elif !defined(CONFIG_HAVE_STDIO_H) && \
      (defined(__NO_has_include) || __has_include(<stdio.h>))
#define CONFIG_HAVE_STDIO_H 1
#endif

#ifdef CONFIG_NO_STDLIB_H
#undef CONFIG_HAVE_STDLIB_H
#elif !defined(CONFIG_HAVE_STDLIB_H) && \
      (defined(__NO_has_include) || __has_include(<stdlib.h>))
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

#ifdef CONFIG_NO_SYS_RESOURCE_H
#undef CONFIG_HAVE_SYS_RESOURCE_H
#elif !defined(CONFIG_HAVE_SYS_RESOURCE_H) && \
      (__has_include(<sys/resource.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_RESOURCE_H 1
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
#elif !defined(CONFIG_HAVE_CTYPE_H) && \
      (defined(__NO_has_include) || __has_include(<ctype.h>))
#define CONFIG_HAVE_CTYPE_H 1
#endif

#ifdef CONFIG_NO_STRING_H
#undef CONFIG_HAVE_STRING_H
#elif !defined(CONFIG_HAVE_STRING_H) && \
      (defined(__NO_has_include) || __has_include(<string.h>))
#define CONFIG_HAVE_STRING_H 1
#endif

#ifdef CONFIG_NO_STRINGS_H
#undef CONFIG_HAVE_STRINGS_H
#elif !defined(CONFIG_HAVE_STRINGS_H) && \
      (__has_include(<strings.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_STRINGS_H 1
#endif

#ifdef CONFIG_NO_WCHAR_H
#undef CONFIG_HAVE_WCHAR_H
#elif !defined(CONFIG_HAVE_WCHAR_H) && \
      (defined(__NO_has_include) || __has_include(<wchar.h>))
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

#ifdef CONFIG_NO_CRTDBG_H
#undef CONFIG_HAVE_CRTDBG_H
#elif !defined(CONFIG_HAVE_CRTDBG_H) && \
      (__has_include(<crtdbg.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       defined(__KOS_SYSTEM_HEADERS__))))
#define CONFIG_HAVE_CRTDBG_H 1
#endif

#ifdef CONFIG_NO_FLOAT_H
#undef CONFIG_HAVE_FLOAT_H
#elif !defined(CONFIG_HAVE_FLOAT_H) && \
      (defined(__NO_has_include) || __has_include(<float.h>))
#define CONFIG_HAVE_FLOAT_H 1
#endif

#ifdef CONFIG_NO_LIMITS_H
#undef CONFIG_HAVE_LIMITS_H
#elif !defined(CONFIG_HAVE_LIMITS_H) && \
      (defined(__NO_has_include) || __has_include(<limits.h>))
#define CONFIG_HAVE_LIMITS_H 1
#endif

#ifdef CONFIG_NO_SETJMP_H
#undef CONFIG_HAVE_SETJMP_H
#elif !defined(CONFIG_HAVE_SETJMP_H) && \
      (defined(__NO_has_include) || __has_include(<setjmp.h>))
#define CONFIG_HAVE_SETJMP_H 1
#endif

#ifdef CONFIG_NO_LINK_H
#undef CONFIG_HAVE_LINK_H
#elif !defined(CONFIG_HAVE_LINK_H) && \
      (__has_include(<link.h>))
#define CONFIG_HAVE_LINK_H 1
#endif

#ifdef CONFIG_NO_BLUETOOTH_BLUETOOTH_H
#undef CONFIG_HAVE_BLUETOOTH_BLUETOOTH_H
#elif !defined(CONFIG_HAVE_BLUETOOTH_BLUETOOTH_H) && \
      (__has_include(<bluetooth/bluetooth.h>))
#define CONFIG_HAVE_BLUETOOTH_BLUETOOTH_H 1
#endif

#ifdef CONFIG_NO_BLUETOOTH_RFCOMM_H
#undef CONFIG_HAVE_BLUETOOTH_RFCOMM_H
#elif !defined(CONFIG_HAVE_BLUETOOTH_RFCOMM_H) && \
      (__has_include(<bluetooth/rfcomm.h>))
#define CONFIG_HAVE_BLUETOOTH_RFCOMM_H 1
#endif

#ifdef CONFIG_NO_BLUETOOTH_L2CAP_H
#undef CONFIG_HAVE_BLUETOOTH_L2CAP_H
#elif !defined(CONFIG_HAVE_BLUETOOTH_L2CAP_H) && \
      (__has_include(<bluetooth/l2cap.h>))
#define CONFIG_HAVE_BLUETOOTH_L2CAP_H 1
#endif

#ifdef CONFIG_NO_BLUETOOTH_SCO_H
#undef CONFIG_HAVE_BLUETOOTH_SCO_H
#elif !defined(CONFIG_HAVE_BLUETOOTH_SCO_H) && \
      (__has_include(<bluetooth/sco.h>))
#define CONFIG_HAVE_BLUETOOTH_SCO_H 1
#endif

#ifdef CONFIG_NO_BLUETOOTH_HCI_H
#undef CONFIG_HAVE_BLUETOOTH_HCI_H
#elif !defined(CONFIG_HAVE_BLUETOOTH_HCI_H) && \
      (__has_include(<bluetooth/hci.h>))
#define CONFIG_HAVE_BLUETOOTH_HCI_H 1
#endif

#ifdef CONFIG_NO_BLUETOOTH_H
#undef CONFIG_HAVE_BLUETOOTH_H
#elif !defined(CONFIG_HAVE_BLUETOOTH_H) && \
      (__has_include(<bluetooth.h>))
#define CONFIG_HAVE_BLUETOOTH_H 1
#endif

#ifdef CONFIG_NO_LINUX_NETLINK_H
#undef CONFIG_HAVE_LINUX_NETLINK_H
#elif !defined(CONFIG_HAVE_LINUX_NETLINK_H) && \
      (__has_include(<linux/netlink.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_LINUX_NETLINK_H 1
#endif

#ifdef CONFIG_NO_ASM_TYPES_H
#undef CONFIG_HAVE_ASM_TYPES_H
#elif !defined(CONFIG_HAVE_ASM_TYPES_H) && \
      (__has_include(<asm/types.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_ASM_TYPES_H 1
#endif

#ifdef CONFIG_NO_SYS_UN_H
#undef CONFIG_HAVE_SYS_UN_H
#elif !defined(CONFIG_HAVE_SYS_UN_H) && \
      (__has_include(<sys/un.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_UN_H 1
#endif

#ifdef CONFIG_NO_SYS_SOCKET_H
#undef CONFIG_HAVE_SYS_SOCKET_H
#elif !defined(CONFIG_HAVE_SYS_SOCKET_H) && \
      (__has_include(<sys/socket.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_SYS_SOCKET_H 1
#endif

#ifdef CONFIG_NO_SYS_SELECT_H
#undef CONFIG_HAVE_SYS_SELECT_H
#elif !defined(CONFIG_HAVE_SYS_SELECT_H) && \
      (__has_include(<sys/select.h>) || (defined(__NO_has_include) && ((defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_SYS_SELECT_H 1
#endif

#ifdef CONFIG_NO_NETDB_H
#undef CONFIG_HAVE_NETDB_H
#elif !defined(CONFIG_HAVE_NETDB_H) && \
      (__has_include(<netdb.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_NETDB_H 1
#endif

#ifdef CONFIG_NO_MATH_H
#undef CONFIG_HAVE_MATH_H
#elif !defined(CONFIG_HAVE_MATH_H) && \
      (defined(__NO_has_include) || __has_include(<math.h>))
#define CONFIG_HAVE_MATH_H 1
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

#ifdef CONFIG_HAVE_ALLOCA_H
#include <alloca.h>
#endif /* CONFIG_HAVE_ALLOCA_H */

#ifdef CONFIG_HAVE_MALLOC_H
#include <malloc.h>
#endif /* CONFIG_HAVE_MALLOC_H */

#ifdef CONFIG_HAVE_IOCTL_H
#include <ioctl.h>
#endif /* CONFIG_HAVE_IOCTL_H */

#ifdef CONFIG_HAVE_DIRENT_H
#include <dirent.h>
#endif /* CONFIG_HAVE_DIRENT_H */

#ifdef CONFIG_HAVE_UNISTD_H
#include <unistd.h>
#endif /* CONFIG_HAVE_UNISTD_H */

#ifdef CONFIG_HAVE_SYS_UNISTD_H
#include <sys/unistd.h>
#endif /* CONFIG_HAVE_SYS_UNISTD_H */

#ifdef CONFIG_HAVE_ERRNO_H
#include <errno.h>
#endif /* CONFIG_HAVE_ERRNO_H */

#ifdef CONFIG_HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#endif /* CONFIG_HAVE_SYS_ERRNO_H */

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

#ifdef CONFIG_HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif /* CONFIG_HAVE_SYS_RESOURCE_H */

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

#ifdef CONFIG_HAVE_STRINGS_H
#include <strings.h>
#endif /* CONFIG_HAVE_STRINGS_H */

#ifdef CONFIG_HAVE_WCHAR_H
#include <wchar.h>
#endif /* CONFIG_HAVE_WCHAR_H */

#ifdef CONFIG_HAVE_DLFCN_H
#include <dlfcn.h>
#endif /* CONFIG_HAVE_DLFCN_H */

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

#ifdef CONFIG_NO__execv
#undef CONFIG_HAVE__execv
#elif !defined(CONFIG_HAVE__execv) && \
      (defined(_execv) || defined(___execv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execv 1
#endif

#ifdef CONFIG_NO_execve
#undef CONFIG_HAVE_execve
#elif !defined(CONFIG_HAVE_execve) && \
      (defined(execve) || defined(__execve_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_execve 1
#endif

#ifdef CONFIG_NO__execve
#undef CONFIG_HAVE__execve
#elif !defined(CONFIG_HAVE__execve) && \
      (defined(_execve) || defined(___execve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execve 1
#endif

#ifdef CONFIG_NO_execvp
#undef CONFIG_HAVE_execvp
#elif !defined(CONFIG_HAVE_execvp) && \
      (defined(execvp) || defined(__execvp_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_execvp 1
#endif

#ifdef CONFIG_NO__execvp
#undef CONFIG_HAVE__execvp
#elif !defined(CONFIG_HAVE__execvp) && \
      (defined(_execvp) || defined(___execvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execvp 1
#endif

#ifdef CONFIG_NO_execvpe
#undef CONFIG_HAVE_execvpe
#elif !defined(CONFIG_HAVE_execvpe) && \
      (defined(execvpe) || defined(__execvpe_defined) || (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix)))
#define CONFIG_HAVE_execvpe 1
#endif

#ifdef CONFIG_NO__execvpe
#undef CONFIG_HAVE__execvpe
#elif !defined(CONFIG_HAVE__execvpe) && \
      (defined(_execvpe) || defined(___execvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__execvpe 1
#endif

#ifdef CONFIG_NO_wexecv
#undef CONFIG_HAVE_wexecv
#elif !defined(CONFIG_HAVE_wexecv) && \
      (defined(wexecv) || defined(__wexecv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wexecv 1
#endif

#ifdef CONFIG_NO__wexecv
#undef CONFIG_HAVE__wexecv
#elif !defined(CONFIG_HAVE__wexecv) && \
      (defined(_wexecv) || defined(___wexecv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecv 1
#endif

#ifdef CONFIG_NO_wexecve
#undef CONFIG_HAVE_wexecve
#elif !defined(CONFIG_HAVE_wexecve) && \
      (defined(wexecve) || defined(__wexecve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wexecve 1
#endif

#ifdef CONFIG_NO__wexecve
#undef CONFIG_HAVE__wexecve
#elif !defined(CONFIG_HAVE__wexecve) && \
      (defined(_wexecve) || defined(___wexecve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecve 1
#endif

#ifdef CONFIG_NO_wexecvp
#undef CONFIG_HAVE_wexecvp
#elif !defined(CONFIG_HAVE_wexecvp) && \
      (defined(wexecvp) || defined(__wexecvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wexecvp 1
#endif

#ifdef CONFIG_NO__wexecvp
#undef CONFIG_HAVE__wexecvp
#elif !defined(CONFIG_HAVE__wexecvp) && \
      (defined(_wexecvp) || defined(___wexecvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecvp 1
#endif

#ifdef CONFIG_NO_wexecvpe
#undef CONFIG_HAVE_wexecvpe
#elif !defined(CONFIG_HAVE_wexecvpe) && \
      (defined(wexecvpe) || defined(__wexecvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wexecvpe 1
#endif

#ifdef CONFIG_NO__wexecvpe
#undef CONFIG_HAVE__wexecvpe
#elif !defined(CONFIG_HAVE__wexecvpe) && \
      (defined(_wexecvpe) || defined(___wexecvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wexecvpe 1
#endif

#ifdef CONFIG_NO_spawnv
#undef CONFIG_HAVE_spawnv
#elif !defined(CONFIG_HAVE_spawnv) && \
      (defined(spawnv) || defined(__spawnv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_spawnv 1
#endif

#ifdef CONFIG_NO__spawnv
#undef CONFIG_HAVE__spawnv
#elif !defined(CONFIG_HAVE__spawnv) && \
      (defined(_spawnv) || defined(___spawnv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnv 1
#endif

#ifdef CONFIG_NO_spawnve
#undef CONFIG_HAVE_spawnve
#elif !defined(CONFIG_HAVE_spawnve) && \
      (defined(spawnve) || defined(__spawnve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_spawnve 1
#endif

#ifdef CONFIG_NO__spawnve
#undef CONFIG_HAVE__spawnve
#elif !defined(CONFIG_HAVE__spawnve) && \
      (defined(_spawnve) || defined(___spawnve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnve 1
#endif

#ifdef CONFIG_NO_spawnvp
#undef CONFIG_HAVE_spawnvp
#elif !defined(CONFIG_HAVE_spawnvp) && \
      (defined(spawnvp) || defined(__spawnvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_spawnvp 1
#endif

#ifdef CONFIG_NO__spawnvp
#undef CONFIG_HAVE__spawnvp
#elif !defined(CONFIG_HAVE__spawnvp) && \
      (defined(_spawnvp) || defined(___spawnvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnvp 1
#endif

#ifdef CONFIG_NO_spawnvpe
#undef CONFIG_HAVE_spawnvpe
#elif !defined(CONFIG_HAVE_spawnvpe) && \
      (defined(spawnvpe) || defined(__spawnvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_spawnvpe 1
#endif

#ifdef CONFIG_NO__spawnvpe
#undef CONFIG_HAVE__spawnvpe
#elif !defined(CONFIG_HAVE__spawnvpe) && \
      (defined(_spawnvpe) || defined(___spawnvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__spawnvpe 1
#endif

#ifdef CONFIG_NO_wspawnv
#undef CONFIG_HAVE_wspawnv
#elif !defined(CONFIG_HAVE_wspawnv) && \
      (defined(wspawnv) || defined(__wspawnv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wspawnv 1
#endif

#ifdef CONFIG_NO__wspawnv
#undef CONFIG_HAVE__wspawnv
#elif !defined(CONFIG_HAVE__wspawnv) && \
      (defined(_wspawnv) || defined(___wspawnv_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnv 1
#endif

#ifdef CONFIG_NO_wspawnve
#undef CONFIG_HAVE_wspawnve
#elif !defined(CONFIG_HAVE_wspawnve) && \
      (defined(wspawnve) || defined(__wspawnve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wspawnve 1
#endif

#ifdef CONFIG_NO__wspawnve
#undef CONFIG_HAVE__wspawnve
#elif !defined(CONFIG_HAVE__wspawnve) && \
      (defined(_wspawnve) || defined(___wspawnve_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnve 1
#endif

#ifdef CONFIG_NO_wspawnvp
#undef CONFIG_HAVE_wspawnvp
#elif !defined(CONFIG_HAVE_wspawnvp) && \
      (defined(wspawnvp) || defined(__wspawnvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wspawnvp 1
#endif

#ifdef CONFIG_NO__wspawnvp
#undef CONFIG_HAVE__wspawnvp
#elif !defined(CONFIG_HAVE__wspawnvp) && \
      (defined(_wspawnvp) || defined(___wspawnvp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnvp 1
#endif

#ifdef CONFIG_NO_wspawnvpe
#undef CONFIG_HAVE_wspawnvpe
#elif !defined(CONFIG_HAVE_wspawnvpe) && \
      (defined(wspawnvpe) || defined(__wspawnvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_wspawnvpe 1
#endif

#ifdef CONFIG_NO__wspawnvpe
#undef CONFIG_HAVE__wspawnvpe
#elif !defined(CONFIG_HAVE__wspawnvpe) && \
      (defined(_wspawnvpe) || defined(___wspawnvpe_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wspawnvpe 1
#endif

#ifdef CONFIG_NO_fexecve
#undef CONFIG_HAVE_fexecve
#elif !defined(CONFIG_HAVE_fexecve) && \
      (defined(fexecve) || defined(__fexecve_defined) || defined(__USE_XOPEN2K8))
#define CONFIG_HAVE_fexecve 1
#endif

#ifdef CONFIG_NO_cwait
#undef CONFIG_HAVE_cwait
#elif !defined(CONFIG_HAVE_cwait) && \
      (defined(cwait) || defined(__cwait_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_cwait 1
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

#ifdef CONFIG_NO_wsystem
#undef CONFIG_HAVE_wsystem
#elif !defined(CONFIG_HAVE_wsystem) && \
      (defined(wsystem) || defined(__wsystem_defined))
#define CONFIG_HAVE_wsystem 1
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
      (defined(_wcreat) || defined(___wcreat_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
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
      (defined(_wopen) || defined(___wopen_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
#define CONFIG_HAVE__wopen 1
#endif

#ifdef CONFIG_NO_open64
#undef CONFIG_HAVE_open64
#elif !defined(CONFIG_HAVE_open64) && \
      (defined(open64) || defined(__open64_defined) || defined(__USE_LARGEFILE64))
#define CONFIG_HAVE_open64 1
#endif

#ifdef CONFIG_NO_fcntl
#undef CONFIG_HAVE_fcntl
#elif !defined(CONFIG_HAVE_fcntl) && \
      (defined(fcntl) || defined(__fcntl_defined) || ((defined(CONFIG_HAVE_FCNTL_H) || \
       defined(CONFIG_HAVE_SYS_FCNTL_H)) && (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix))))
#define CONFIG_HAVE_fcntl 1
#endif

#ifdef CONFIG_NO_F_SETFD
#undef CONFIG_HAVE_F_SETFD
#elif !defined(CONFIG_HAVE_F_SETFD) && \
      (defined(F_SETFD) || defined(__F_SETFD_defined))
#define CONFIG_HAVE_F_SETFD 1
#endif

#ifdef CONFIG_NO_F_SETFL
#undef CONFIG_HAVE_F_SETFL
#elif !defined(CONFIG_HAVE_F_SETFL) && \
      (defined(F_SETFL) || defined(__F_SETFL_defined))
#define CONFIG_HAVE_F_SETFL 1
#endif

#ifdef CONFIG_NO_FD_CLOEXEC
#undef CONFIG_HAVE_FD_CLOEXEC
#elif !defined(CONFIG_HAVE_FD_CLOEXEC) && \
      (defined(FD_CLOEXEC) || defined(__FD_CLOEXEC_defined))
#define CONFIG_HAVE_FD_CLOEXEC 1
#endif

#ifdef CONFIG_NO_O_BINARY
#undef CONFIG_HAVE_O_BINARY
#elif defined(CONFIG_HAVE_O_BINARY)
#elif defined(O_BINARY)
#if O_BINARY != 0
#define CONFIG_HAVE_O_BINARY 1
#else /* O_BINARY */
#define CONFIG_NO_O_BINARY 1
#endif /* O_BINARY */
#elif defined(__O_BINARY__defined)
#define CONFIG_HAVE_O_BINARY 1
#endif

#ifdef CONFIG_NO___O_BINARY
#undef CONFIG_HAVE___O_BINARY
#elif defined(CONFIG_HAVE___O_BINARY)
#elif defined(__O_BINARY)
#if __O_BINARY != 0
#define CONFIG_HAVE___O_BINARY 1
#else /* __O_BINARY */
#define CONFIG_NO___O_BINARY 1
#endif /* __O_BINARY */
#elif defined(____O_BINARY__defined)
#define CONFIG_HAVE___O_BINARY 1
#endif

#ifdef CONFIG_NO__O_BINARY
#undef CONFIG_HAVE__O_BINARY
#elif defined(CONFIG_HAVE__O_BINARY)
#elif defined(_O_BINARY)
#if _O_BINARY != 0
#define CONFIG_HAVE__O_BINARY 1
#else /* _O_BINARY */
#define CONFIG_NO__O_BINARY 1
#endif /* _O_BINARY */
#elif defined(___O_BINARY__defined)
#define CONFIG_HAVE__O_BINARY 1
#endif

#ifdef CONFIG_NO___O_RAW
#undef CONFIG_HAVE___O_RAW
#elif defined(CONFIG_HAVE___O_RAW)
#elif defined(__O_RAW)
#if __O_RAW != 0
#define CONFIG_HAVE___O_RAW 1
#else /* __O_RAW */
#define CONFIG_NO___O_RAW 1
#endif /* __O_RAW */
#elif defined(____O_RAW__defined)
#define CONFIG_HAVE___O_RAW 1
#endif

#ifdef CONFIG_NO__O_RAW
#undef CONFIG_HAVE__O_RAW
#elif defined(CONFIG_HAVE__O_RAW)
#elif defined(_O_RAW)
#if _O_RAW != 0
#define CONFIG_HAVE__O_RAW 1
#else /* _O_RAW */
#define CONFIG_NO__O_RAW 1
#endif /* _O_RAW */
#elif defined(___O_RAW__defined)
#define CONFIG_HAVE__O_RAW 1
#endif

#ifdef CONFIG_NO_O_RAW
#undef CONFIG_HAVE_O_RAW
#elif defined(CONFIG_HAVE_O_RAW)
#elif defined(O_RAW)
#if O_RAW != 0
#define CONFIG_HAVE_O_RAW 1
#else /* O_RAW */
#define CONFIG_NO_O_RAW 1
#endif /* O_RAW */
#elif defined(__O_RAW__defined)
#define CONFIG_HAVE_O_RAW 1
#endif

#ifdef CONFIG_NO_O_SHORT_LIVED
#undef CONFIG_HAVE_O_SHORT_LIVED
#elif defined(CONFIG_HAVE_O_SHORT_LIVED)
#elif defined(O_SHORT_LIVED)
#if O_SHORT_LIVED != 0
#define CONFIG_HAVE_O_SHORT_LIVED 1
#else /* O_SHORT_LIVED */
#define CONFIG_NO_O_SHORT_LIVED 1
#endif /* O_SHORT_LIVED */
#elif defined(__O_SHORT_LIVED__defined)
#define CONFIG_HAVE_O_SHORT_LIVED 1
#endif

#ifdef CONFIG_NO___O_SHORT_LIVED
#undef CONFIG_HAVE___O_SHORT_LIVED
#elif defined(CONFIG_HAVE___O_SHORT_LIVED)
#elif defined(__O_SHORT_LIVED)
#if __O_SHORT_LIVED != 0
#define CONFIG_HAVE___O_SHORT_LIVED 1
#else /* __O_SHORT_LIVED */
#define CONFIG_NO___O_SHORT_LIVED 1
#endif /* __O_SHORT_LIVED */
#elif defined(____O_SHORT_LIVED__defined)
#define CONFIG_HAVE___O_SHORT_LIVED 1
#endif

#ifdef CONFIG_NO__O_SHORT_LIVED
#undef CONFIG_HAVE__O_SHORT_LIVED
#elif defined(CONFIG_HAVE__O_SHORT_LIVED)
#elif defined(_O_SHORT_LIVED)
#if _O_SHORT_LIVED != 0
#define CONFIG_HAVE__O_SHORT_LIVED 1
#else /* _O_SHORT_LIVED */
#define CONFIG_NO__O_SHORT_LIVED 1
#endif /* _O_SHORT_LIVED */
#elif defined(___O_SHORT_LIVED__defined)
#define CONFIG_HAVE__O_SHORT_LIVED 1
#endif

#ifdef CONFIG_NO_O_SEQUENTIAL
#undef CONFIG_HAVE_O_SEQUENTIAL
#elif defined(CONFIG_HAVE_O_SEQUENTIAL)
#elif defined(O_SEQUENTIAL)
#if O_SEQUENTIAL != 0
#define CONFIG_HAVE_O_SEQUENTIAL 1
#else /* O_SEQUENTIAL */
#define CONFIG_NO_O_SEQUENTIAL 1
#endif /* O_SEQUENTIAL */
#elif defined(__O_SEQUENTIAL__defined)
#define CONFIG_HAVE_O_SEQUENTIAL 1
#endif

#ifdef CONFIG_NO___O_SEQUENTIAL
#undef CONFIG_HAVE___O_SEQUENTIAL
#elif defined(CONFIG_HAVE___O_SEQUENTIAL)
#elif defined(__O_SEQUENTIAL)
#if __O_SEQUENTIAL != 0
#define CONFIG_HAVE___O_SEQUENTIAL 1
#else /* __O_SEQUENTIAL */
#define CONFIG_NO___O_SEQUENTIAL 1
#endif /* __O_SEQUENTIAL */
#elif defined(____O_SEQUENTIAL__defined)
#define CONFIG_HAVE___O_SEQUENTIAL 1
#endif

#ifdef CONFIG_NO__O_SEQUENTIAL
#undef CONFIG_HAVE__O_SEQUENTIAL
#elif defined(CONFIG_HAVE__O_SEQUENTIAL)
#elif defined(_O_SEQUENTIAL)
#if _O_SEQUENTIAL != 0
#define CONFIG_HAVE__O_SEQUENTIAL 1
#else /* _O_SEQUENTIAL */
#define CONFIG_NO__O_SEQUENTIAL 1
#endif /* _O_SEQUENTIAL */
#elif defined(___O_SEQUENTIAL__defined)
#define CONFIG_HAVE__O_SEQUENTIAL 1
#endif

#ifdef CONFIG_NO_O_RANDOM
#undef CONFIG_HAVE_O_RANDOM
#elif defined(CONFIG_HAVE_O_RANDOM)
#elif defined(O_RANDOM)
#if O_RANDOM != 0
#define CONFIG_HAVE_O_RANDOM 1
#else /* O_RANDOM */
#define CONFIG_NO_O_RANDOM 1
#endif /* O_RANDOM */
#elif defined(__O_RANDOM__defined)
#define CONFIG_HAVE_O_RANDOM 1
#endif

#ifdef CONFIG_NO___O_RANDOM
#undef CONFIG_HAVE___O_RANDOM
#elif defined(CONFIG_HAVE___O_RANDOM)
#elif defined(__O_RANDOM)
#if __O_RANDOM != 0
#define CONFIG_HAVE___O_RANDOM 1
#else /* __O_RANDOM */
#define CONFIG_NO___O_RANDOM 1
#endif /* __O_RANDOM */
#elif defined(____O_RANDOM__defined)
#define CONFIG_HAVE___O_RANDOM 1
#endif

#ifdef CONFIG_NO__O_RANDOM
#undef CONFIG_HAVE__O_RANDOM
#elif defined(CONFIG_HAVE__O_RANDOM)
#elif defined(_O_RANDOM)
#if _O_RANDOM != 0
#define CONFIG_HAVE__O_RANDOM 1
#else /* _O_RANDOM */
#define CONFIG_NO__O_RANDOM 1
#endif /* _O_RANDOM */
#elif defined(___O_RANDOM__defined)
#define CONFIG_HAVE__O_RANDOM 1
#endif

#ifdef CONFIG_NO_O_PATH
#undef CONFIG_HAVE_O_PATH
#elif defined(CONFIG_HAVE_O_PATH)
#elif defined(O_PATH)
#if O_PATH != 0
#define CONFIG_HAVE_O_PATH 1
#else /* O_PATH */
#define CONFIG_NO_O_PATH 1
#endif /* O_PATH */
#elif defined(__O_PATH__defined)
#define CONFIG_HAVE_O_PATH 1
#endif

#ifdef CONFIG_NO___O_PATH
#undef CONFIG_HAVE___O_PATH
#elif defined(CONFIG_HAVE___O_PATH)
#elif defined(__O_PATH)
#if __O_PATH != 0
#define CONFIG_HAVE___O_PATH 1
#else /* __O_PATH */
#define CONFIG_NO___O_PATH 1
#endif /* __O_PATH */
#elif defined(____O_PATH__defined)
#define CONFIG_HAVE___O_PATH 1
#endif

#ifdef CONFIG_NO__O_PATH
#undef CONFIG_HAVE__O_PATH
#elif defined(CONFIG_HAVE__O_PATH)
#elif defined(_O_PATH)
#if _O_PATH != 0
#define CONFIG_HAVE__O_PATH 1
#else /* _O_PATH */
#define CONFIG_NO__O_PATH 1
#endif /* _O_PATH */
#elif defined(___O_PATH__defined)
#define CONFIG_HAVE__O_PATH 1
#endif

#ifdef CONFIG_NO_O_NOATIME
#undef CONFIG_HAVE_O_NOATIME
#elif defined(CONFIG_HAVE_O_NOATIME)
#elif defined(O_NOATIME)
#if O_NOATIME != 0
#define CONFIG_HAVE_O_NOATIME 1
#else /* O_NOATIME */
#define CONFIG_NO_O_NOATIME 1
#endif /* O_NOATIME */
#elif defined(__O_NOATIME__defined)
#define CONFIG_HAVE_O_NOATIME 1
#endif

#ifdef CONFIG_NO___O_NOATIME
#undef CONFIG_HAVE___O_NOATIME
#elif defined(CONFIG_HAVE___O_NOATIME)
#elif defined(__O_NOATIME)
#if __O_NOATIME != 0
#define CONFIG_HAVE___O_NOATIME 1
#else /* __O_NOATIME */
#define CONFIG_NO___O_NOATIME 1
#endif /* __O_NOATIME */
#elif defined(____O_NOATIME__defined)
#define CONFIG_HAVE___O_NOATIME 1
#endif

#ifdef CONFIG_NO__O_NOATIME
#undef CONFIG_HAVE__O_NOATIME
#elif defined(CONFIG_HAVE__O_NOATIME)
#elif defined(_O_NOATIME)
#if _O_NOATIME != 0
#define CONFIG_HAVE__O_NOATIME 1
#else /* _O_NOATIME */
#define CONFIG_NO__O_NOATIME 1
#endif /* _O_NOATIME */
#elif defined(___O_NOATIME__defined)
#define CONFIG_HAVE__O_NOATIME 1
#endif

#ifdef CONFIG_NO_O_NOCTTY
#undef CONFIG_HAVE_O_NOCTTY
#elif defined(CONFIG_HAVE_O_NOCTTY)
#elif defined(O_NOCTTY)
#if O_NOCTTY != 0
#define CONFIG_HAVE_O_NOCTTY 1
#else /* O_NOCTTY */
#define CONFIG_NO_O_NOCTTY 1
#endif /* O_NOCTTY */
#elif defined(__O_NOCTTY__defined)
#define CONFIG_HAVE_O_NOCTTY 1
#endif

#ifdef CONFIG_NO___O_NOCTTY
#undef CONFIG_HAVE___O_NOCTTY
#elif defined(CONFIG_HAVE___O_NOCTTY)
#elif defined(__O_NOCTTY)
#if __O_NOCTTY != 0
#define CONFIG_HAVE___O_NOCTTY 1
#else /* __O_NOCTTY */
#define CONFIG_NO___O_NOCTTY 1
#endif /* __O_NOCTTY */
#elif defined(____O_NOCTTY__defined)
#define CONFIG_HAVE___O_NOCTTY 1
#endif

#ifdef CONFIG_NO__O_NOCTTY
#undef CONFIG_HAVE__O_NOCTTY
#elif defined(CONFIG_HAVE__O_NOCTTY)
#elif defined(_O_NOCTTY)
#if _O_NOCTTY != 0
#define CONFIG_HAVE__O_NOCTTY 1
#else /* _O_NOCTTY */
#define CONFIG_NO__O_NOCTTY 1
#endif /* _O_NOCTTY */
#elif defined(___O_NOCTTY__defined)
#define CONFIG_HAVE__O_NOCTTY 1
#endif

#ifdef CONFIG_NO_O_TEXT
#undef CONFIG_HAVE_O_TEXT
#elif defined(CONFIG_HAVE_O_TEXT)
#elif defined(O_TEXT)
#if O_TEXT != 0
#define CONFIG_HAVE_O_TEXT 1
#else /* O_TEXT */
#define CONFIG_NO_O_TEXT 1
#endif /* O_TEXT */
#elif defined(__O_TEXT__defined)
#define CONFIG_HAVE_O_TEXT 1
#endif

#ifdef CONFIG_NO___O_TEXT
#undef CONFIG_HAVE___O_TEXT
#elif defined(CONFIG_HAVE___O_TEXT)
#elif defined(__O_TEXT)
#if __O_TEXT != 0
#define CONFIG_HAVE___O_TEXT 1
#else /* __O_TEXT */
#define CONFIG_NO___O_TEXT 1
#endif /* __O_TEXT */
#elif defined(____O_TEXT__defined)
#define CONFIG_HAVE___O_TEXT 1
#endif

#ifdef CONFIG_NO__O_TEXT
#undef CONFIG_HAVE__O_TEXT
#elif defined(CONFIG_HAVE__O_TEXT)
#elif defined(_O_TEXT)
#if _O_TEXT != 0
#define CONFIG_HAVE__O_TEXT 1
#else /* _O_TEXT */
#define CONFIG_NO__O_TEXT 1
#endif /* _O_TEXT */
#elif defined(___O_TEXT__defined)
#define CONFIG_HAVE__O_TEXT 1
#endif

#ifdef CONFIG_NO_O_WTEXT
#undef CONFIG_HAVE_O_WTEXT
#elif defined(CONFIG_HAVE_O_WTEXT)
#elif defined(O_WTEXT)
#if O_WTEXT != 0
#define CONFIG_HAVE_O_WTEXT 1
#else /* O_WTEXT */
#define CONFIG_NO_O_WTEXT 1
#endif /* O_WTEXT */
#elif defined(__O_WTEXT__defined)
#define CONFIG_HAVE_O_WTEXT 1
#endif

#ifdef CONFIG_NO___O_WTEXT
#undef CONFIG_HAVE___O_WTEXT
#elif defined(CONFIG_HAVE___O_WTEXT)
#elif defined(__O_WTEXT)
#if __O_WTEXT != 0
#define CONFIG_HAVE___O_WTEXT 1
#else /* __O_WTEXT */
#define CONFIG_NO___O_WTEXT 1
#endif /* __O_WTEXT */
#elif defined(____O_WTEXT__defined)
#define CONFIG_HAVE___O_WTEXT 1
#endif

#ifdef CONFIG_NO__O_WTEXT
#undef CONFIG_HAVE__O_WTEXT
#elif defined(CONFIG_HAVE__O_WTEXT)
#elif defined(_O_WTEXT)
#if _O_WTEXT != 0
#define CONFIG_HAVE__O_WTEXT 1
#else /* _O_WTEXT */
#define CONFIG_NO__O_WTEXT 1
#endif /* _O_WTEXT */
#elif defined(___O_WTEXT__defined)
#define CONFIG_HAVE__O_WTEXT 1
#endif

#ifdef CONFIG_NO_O_U16TEXT
#undef CONFIG_HAVE_O_U16TEXT
#elif defined(CONFIG_HAVE_O_U16TEXT)
#elif defined(O_U16TEXT)
#if O_U16TEXT != 0
#define CONFIG_HAVE_O_U16TEXT 1
#else /* O_U16TEXT */
#define CONFIG_NO_O_U16TEXT 1
#endif /* O_U16TEXT */
#elif defined(__O_U16TEXT__defined)
#define CONFIG_HAVE_O_U16TEXT 1
#endif

#ifdef CONFIG_NO___O_U16TEXT
#undef CONFIG_HAVE___O_U16TEXT
#elif defined(CONFIG_HAVE___O_U16TEXT)
#elif defined(__O_U16TEXT)
#if __O_U16TEXT != 0
#define CONFIG_HAVE___O_U16TEXT 1
#else /* __O_U16TEXT */
#define CONFIG_NO___O_U16TEXT 1
#endif /* __O_U16TEXT */
#elif defined(____O_U16TEXT__defined)
#define CONFIG_HAVE___O_U16TEXT 1
#endif

#ifdef CONFIG_NO__O_U16TEXT
#undef CONFIG_HAVE__O_U16TEXT
#elif defined(CONFIG_HAVE__O_U16TEXT)
#elif defined(_O_U16TEXT)
#if _O_U16TEXT != 0
#define CONFIG_HAVE__O_U16TEXT 1
#else /* _O_U16TEXT */
#define CONFIG_NO__O_U16TEXT 1
#endif /* _O_U16TEXT */
#elif defined(___O_U16TEXT__defined)
#define CONFIG_HAVE__O_U16TEXT 1
#endif

#ifdef CONFIG_NO_O_U8TEXT
#undef CONFIG_HAVE_O_U8TEXT
#elif defined(CONFIG_HAVE_O_U8TEXT)
#elif defined(O_U8TEXT)
#if O_U8TEXT != 0
#define CONFIG_HAVE_O_U8TEXT 1
#else /* O_U8TEXT */
#define CONFIG_NO_O_U8TEXT 1
#endif /* O_U8TEXT */
#elif defined(__O_U8TEXT__defined)
#define CONFIG_HAVE_O_U8TEXT 1
#endif

#ifdef CONFIG_NO___O_U8TEXT
#undef CONFIG_HAVE___O_U8TEXT
#elif defined(CONFIG_HAVE___O_U8TEXT)
#elif defined(__O_U8TEXT)
#if __O_U8TEXT != 0
#define CONFIG_HAVE___O_U8TEXT 1
#else /* __O_U8TEXT */
#define CONFIG_NO___O_U8TEXT 1
#endif /* __O_U8TEXT */
#elif defined(____O_U8TEXT__defined)
#define CONFIG_HAVE___O_U8TEXT 1
#endif

#ifdef CONFIG_NO__O_U8TEXT
#undef CONFIG_HAVE__O_U8TEXT
#elif defined(CONFIG_HAVE__O_U8TEXT)
#elif defined(_O_U8TEXT)
#if _O_U8TEXT != 0
#define CONFIG_HAVE__O_U8TEXT 1
#else /* _O_U8TEXT */
#define CONFIG_NO__O_U8TEXT 1
#endif /* _O_U8TEXT */
#elif defined(___O_U8TEXT__defined)
#define CONFIG_HAVE__O_U8TEXT 1
#endif

#ifdef CONFIG_NO_O_TEMPORARY
#undef CONFIG_HAVE_O_TEMPORARY
#elif defined(CONFIG_HAVE_O_TEMPORARY)
#elif defined(O_TEMPORARY)
#if O_TEMPORARY != 0
#define CONFIG_HAVE_O_TEMPORARY 1
#else /* O_TEMPORARY */
#define CONFIG_NO_O_TEMPORARY 1
#endif /* O_TEMPORARY */
#elif defined(__O_TEMPORARY__defined)
#define CONFIG_HAVE_O_TEMPORARY 1
#endif

#ifdef CONFIG_NO___O_TEMPORARY
#undef CONFIG_HAVE___O_TEMPORARY
#elif defined(CONFIG_HAVE___O_TEMPORARY)
#elif defined(__O_TEMPORARY)
#if __O_TEMPORARY != 0
#define CONFIG_HAVE___O_TEMPORARY 1
#else /* __O_TEMPORARY */
#define CONFIG_NO___O_TEMPORARY 1
#endif /* __O_TEMPORARY */
#elif defined(____O_TEMPORARY__defined)
#define CONFIG_HAVE___O_TEMPORARY 1
#endif

#ifdef CONFIG_NO__O_TEMPORARY
#undef CONFIG_HAVE__O_TEMPORARY
#elif defined(CONFIG_HAVE__O_TEMPORARY)
#elif defined(_O_TEMPORARY)
#if _O_TEMPORARY != 0
#define CONFIG_HAVE__O_TEMPORARY 1
#else /* _O_TEMPORARY */
#define CONFIG_NO__O_TEMPORARY 1
#endif /* _O_TEMPORARY */
#elif defined(___O_TEMPORARY__defined)
#define CONFIG_HAVE__O_TEMPORARY 1
#endif

#ifdef CONFIG_NO_O_OBTAIN_DIR
#undef CONFIG_HAVE_O_OBTAIN_DIR
#elif defined(CONFIG_HAVE_O_OBTAIN_DIR)
#elif defined(O_OBTAIN_DIR)
#if O_OBTAIN_DIR != 0
#define CONFIG_HAVE_O_OBTAIN_DIR 1
#else /* O_OBTAIN_DIR */
#define CONFIG_NO_O_OBTAIN_DIR 1
#endif /* O_OBTAIN_DIR */
#elif defined(__O_OBTAIN_DIR__defined)
#define CONFIG_HAVE_O_OBTAIN_DIR 1
#endif

#ifdef CONFIG_NO___O_OBTAIN_DIR
#undef CONFIG_HAVE___O_OBTAIN_DIR
#elif defined(CONFIG_HAVE___O_OBTAIN_DIR)
#elif defined(__O_OBTAIN_DIR)
#if __O_OBTAIN_DIR != 0
#define CONFIG_HAVE___O_OBTAIN_DIR 1
#else /* __O_OBTAIN_DIR */
#define CONFIG_NO___O_OBTAIN_DIR 1
#endif /* __O_OBTAIN_DIR */
#elif defined(____O_OBTAIN_DIR__defined)
#define CONFIG_HAVE___O_OBTAIN_DIR 1
#endif

#ifdef CONFIG_NO__O_OBTAIN_DIR
#undef CONFIG_HAVE__O_OBTAIN_DIR
#elif defined(CONFIG_HAVE__O_OBTAIN_DIR)
#elif defined(_O_OBTAIN_DIR)
#if _O_OBTAIN_DIR != 0
#define CONFIG_HAVE__O_OBTAIN_DIR 1
#else /* _O_OBTAIN_DIR */
#define CONFIG_NO__O_OBTAIN_DIR 1
#endif /* _O_OBTAIN_DIR */
#elif defined(___O_OBTAIN_DIR__defined)
#define CONFIG_HAVE__O_OBTAIN_DIR 1
#endif

#ifdef CONFIG_NO_O_CREAT
#undef CONFIG_HAVE_O_CREAT
#elif defined(CONFIG_HAVE_O_CREAT)
#elif defined(O_CREAT)
#if O_CREAT != 0
#define CONFIG_HAVE_O_CREAT 1
#else /* O_CREAT */
#define CONFIG_NO_O_CREAT 1
#endif /* O_CREAT */
#elif defined(__O_CREAT__defined)
#define CONFIG_HAVE_O_CREAT 1
#endif

#ifdef CONFIG_NO___O_CREAT
#undef CONFIG_HAVE___O_CREAT
#elif defined(CONFIG_HAVE___O_CREAT)
#elif defined(__O_CREAT)
#if __O_CREAT != 0
#define CONFIG_HAVE___O_CREAT 1
#else /* __O_CREAT */
#define CONFIG_NO___O_CREAT 1
#endif /* __O_CREAT */
#elif defined(____O_CREAT__defined)
#define CONFIG_HAVE___O_CREAT 1
#endif

#ifdef CONFIG_NO__O_CREAT
#undef CONFIG_HAVE__O_CREAT
#elif defined(CONFIG_HAVE__O_CREAT)
#elif defined(_O_CREAT)
#if _O_CREAT != 0
#define CONFIG_HAVE__O_CREAT 1
#else /* _O_CREAT */
#define CONFIG_NO__O_CREAT 1
#endif /* _O_CREAT */
#elif defined(___O_CREAT__defined)
#define CONFIG_HAVE__O_CREAT 1
#endif

#ifdef CONFIG_NO_O_TRUNC
#undef CONFIG_HAVE_O_TRUNC
#elif defined(CONFIG_HAVE_O_TRUNC)
#elif defined(O_TRUNC)
#if O_TRUNC != 0
#define CONFIG_HAVE_O_TRUNC 1
#else /* O_TRUNC */
#define CONFIG_NO_O_TRUNC 1
#endif /* O_TRUNC */
#elif defined(__O_TRUNC__defined)
#define CONFIG_HAVE_O_TRUNC 1
#endif

#ifdef CONFIG_NO___O_TRUNC
#undef CONFIG_HAVE___O_TRUNC
#elif defined(CONFIG_HAVE___O_TRUNC)
#elif defined(__O_TRUNC)
#if __O_TRUNC != 0
#define CONFIG_HAVE___O_TRUNC 1
#else /* __O_TRUNC */
#define CONFIG_NO___O_TRUNC 1
#endif /* __O_TRUNC */
#elif defined(____O_TRUNC__defined)
#define CONFIG_HAVE___O_TRUNC 1
#endif

#ifdef CONFIG_NO__O_TRUNC
#undef CONFIG_HAVE__O_TRUNC
#elif defined(CONFIG_HAVE__O_TRUNC)
#elif defined(_O_TRUNC)
#if _O_TRUNC != 0
#define CONFIG_HAVE__O_TRUNC 1
#else /* _O_TRUNC */
#define CONFIG_NO__O_TRUNC 1
#endif /* _O_TRUNC */
#elif defined(___O_TRUNC__defined)
#define CONFIG_HAVE__O_TRUNC 1
#endif

#ifdef CONFIG_NO_O_RDONLY
#undef CONFIG_HAVE_O_RDONLY
#elif !defined(CONFIG_HAVE_O_RDONLY) && \
      (defined(O_RDONLY) || defined(__O_RDONLY_defined))
#define CONFIG_HAVE_O_RDONLY 1
#endif

#ifdef CONFIG_NO___O_RDONLY
#undef CONFIG_HAVE___O_RDONLY
#elif !defined(CONFIG_HAVE___O_RDONLY) && \
      (defined(__O_RDONLY) || defined(____O_RDONLY_defined))
#define CONFIG_HAVE___O_RDONLY 1
#endif

#ifdef CONFIG_NO__O_RDONLY
#undef CONFIG_HAVE__O_RDONLY
#elif !defined(CONFIG_HAVE__O_RDONLY) && \
      (defined(_O_RDONLY) || defined(___O_RDONLY_defined))
#define CONFIG_HAVE__O_RDONLY 1
#endif

#ifdef CONFIG_NO_O_WRONLY
#undef CONFIG_HAVE_O_WRONLY
#elif !defined(CONFIG_HAVE_O_WRONLY) && \
      (defined(O_WRONLY) || defined(__O_WRONLY_defined))
#define CONFIG_HAVE_O_WRONLY 1
#endif

#ifdef CONFIG_NO___O_WRONLY
#undef CONFIG_HAVE___O_WRONLY
#elif !defined(CONFIG_HAVE___O_WRONLY) && \
      (defined(__O_WRONLY) || defined(____O_WRONLY_defined))
#define CONFIG_HAVE___O_WRONLY 1
#endif

#ifdef CONFIG_NO__O_WRONLY
#undef CONFIG_HAVE__O_WRONLY
#elif !defined(CONFIG_HAVE__O_WRONLY) && \
      (defined(_O_WRONLY) || defined(___O_WRONLY_defined))
#define CONFIG_HAVE__O_WRONLY 1
#endif

#ifdef CONFIG_NO_O_RDWR
#undef CONFIG_HAVE_O_RDWR
#elif !defined(CONFIG_HAVE_O_RDWR) && \
      (defined(O_RDWR) || defined(__O_RDWR_defined))
#define CONFIG_HAVE_O_RDWR 1
#endif

#ifdef CONFIG_NO___O_RDWR
#undef CONFIG_HAVE___O_RDWR
#elif !defined(CONFIG_HAVE___O_RDWR) && \
      (defined(__O_RDWR) || defined(____O_RDWR_defined))
#define CONFIG_HAVE___O_RDWR 1
#endif

#ifdef CONFIG_NO__O_RDWR
#undef CONFIG_HAVE__O_RDWR
#elif !defined(CONFIG_HAVE__O_RDWR) && \
      (defined(_O_RDWR) || defined(___O_RDWR_defined))
#define CONFIG_HAVE__O_RDWR 1
#endif

#ifdef CONFIG_NO_O_ACCMODE
#undef CONFIG_HAVE_O_ACCMODE
#elif !defined(CONFIG_HAVE_O_ACCMODE) && \
      (defined(O_ACCMODE) || defined(__O_ACCMODE_defined))
#define CONFIG_HAVE_O_ACCMODE 1
#endif

#ifdef CONFIG_NO___O_ACCMODE
#undef CONFIG_HAVE___O_ACCMODE
#elif !defined(CONFIG_HAVE___O_ACCMODE) && \
      (defined(__O_ACCMODE) || defined(____O_ACCMODE_defined))
#define CONFIG_HAVE___O_ACCMODE 1
#endif

#ifdef CONFIG_NO__O_ACCMODE
#undef CONFIG_HAVE__O_ACCMODE
#elif !defined(CONFIG_HAVE__O_ACCMODE) && \
      (defined(_O_ACCMODE) || defined(___O_ACCMODE_defined))
#define CONFIG_HAVE__O_ACCMODE 1
#endif

#ifdef CONFIG_NO_O_CLOEXEC
#undef CONFIG_HAVE_O_CLOEXEC
#elif defined(CONFIG_HAVE_O_CLOEXEC)
#elif defined(O_CLOEXEC)
#if O_CLOEXEC != 0
#define CONFIG_HAVE_O_CLOEXEC 1
#else /* O_CLOEXEC */
#define CONFIG_NO_O_CLOEXEC 1
#endif /* O_CLOEXEC */
#elif defined(__O_CLOEXEC__defined)
#define CONFIG_HAVE_O_CLOEXEC 1
#endif

#ifdef CONFIG_NO___O_NOINHERIT
#undef CONFIG_HAVE___O_NOINHERIT
#elif defined(CONFIG_HAVE___O_NOINHERIT)
#elif defined(__O_NOINHERIT)
#if __O_NOINHERIT != 0
#define CONFIG_HAVE___O_NOINHERIT 1
#else /* __O_NOINHERIT */
#define CONFIG_NO___O_NOINHERIT 1
#endif /* __O_NOINHERIT */
#elif defined(____O_NOINHERIT__defined)
#define CONFIG_HAVE___O_NOINHERIT 1
#endif

#ifdef CONFIG_NO__O_NOINHERIT
#undef CONFIG_HAVE__O_NOINHERIT
#elif defined(CONFIG_HAVE__O_NOINHERIT)
#elif defined(_O_NOINHERIT)
#if _O_NOINHERIT != 0
#define CONFIG_HAVE__O_NOINHERIT 1
#else /* _O_NOINHERIT */
#define CONFIG_NO__O_NOINHERIT 1
#endif /* _O_NOINHERIT */
#elif defined(___O_NOINHERIT__defined)
#define CONFIG_HAVE__O_NOINHERIT 1
#endif

#ifdef CONFIG_NO_O_NOINHERIT
#undef CONFIG_HAVE_O_NOINHERIT
#elif defined(CONFIG_HAVE_O_NOINHERIT)
#elif defined(O_NOINHERIT)
#if O_NOINHERIT != 0
#define CONFIG_HAVE_O_NOINHERIT 1
#else /* O_NOINHERIT */
#define CONFIG_NO_O_NOINHERIT 1
#endif /* O_NOINHERIT */
#elif defined(__O_NOINHERIT__defined)
#define CONFIG_HAVE_O_NOINHERIT 1
#endif

#ifdef CONFIG_NO___O_CLOEXEC
#undef CONFIG_HAVE___O_CLOEXEC
#elif defined(CONFIG_HAVE___O_CLOEXEC)
#elif defined(__O_CLOEXEC)
#if __O_CLOEXEC != 0
#define CONFIG_HAVE___O_CLOEXEC 1
#else /* __O_CLOEXEC */
#define CONFIG_NO___O_CLOEXEC 1
#endif /* __O_CLOEXEC */
#elif defined(____O_CLOEXEC__defined)
#define CONFIG_HAVE___O_CLOEXEC 1
#endif

#ifdef CONFIG_NO__O_CLOEXEC
#undef CONFIG_HAVE__O_CLOEXEC
#elif defined(CONFIG_HAVE__O_CLOEXEC)
#elif defined(_O_CLOEXEC)
#if _O_CLOEXEC != 0
#define CONFIG_HAVE__O_CLOEXEC 1
#else /* _O_CLOEXEC */
#define CONFIG_NO__O_CLOEXEC 1
#endif /* _O_CLOEXEC */
#elif defined(___O_CLOEXEC__defined)
#define CONFIG_HAVE__O_CLOEXEC 1
#endif

#ifdef CONFIG_NO_O_EXCL
#undef CONFIG_HAVE_O_EXCL
#elif defined(CONFIG_HAVE_O_EXCL)
#elif defined(O_EXCL)
#if O_EXCL != 0
#define CONFIG_HAVE_O_EXCL 1
#else /* O_EXCL */
#define CONFIG_NO_O_EXCL 1
#endif /* O_EXCL */
#elif defined(__O_EXCL__defined)
#define CONFIG_HAVE_O_EXCL 1
#endif

#ifdef CONFIG_NO___O_EXCL
#undef CONFIG_HAVE___O_EXCL
#elif defined(CONFIG_HAVE___O_EXCL)
#elif defined(__O_EXCL)
#if __O_EXCL != 0
#define CONFIG_HAVE___O_EXCL 1
#else /* __O_EXCL */
#define CONFIG_NO___O_EXCL 1
#endif /* __O_EXCL */
#elif defined(____O_EXCL__defined)
#define CONFIG_HAVE___O_EXCL 1
#endif

#ifdef CONFIG_NO__O_EXCL
#undef CONFIG_HAVE__O_EXCL
#elif defined(CONFIG_HAVE__O_EXCL)
#elif defined(_O_EXCL)
#if _O_EXCL != 0
#define CONFIG_HAVE__O_EXCL 1
#else /* _O_EXCL */
#define CONFIG_NO__O_EXCL 1
#endif /* _O_EXCL */
#elif defined(___O_EXCL__defined)
#define CONFIG_HAVE__O_EXCL 1
#endif

#ifdef CONFIG_NO_O_APPEND
#undef CONFIG_HAVE_O_APPEND
#elif defined(CONFIG_HAVE_O_APPEND)
#elif defined(O_APPEND)
#if O_APPEND != 0
#define CONFIG_HAVE_O_APPEND 1
#else /* O_APPEND */
#define CONFIG_NO_O_APPEND 1
#endif /* O_APPEND */
#elif defined(__O_APPEND__defined)
#define CONFIG_HAVE_O_APPEND 1
#endif

#ifdef CONFIG_NO___O_APPEND
#undef CONFIG_HAVE___O_APPEND
#elif defined(CONFIG_HAVE___O_APPEND)
#elif defined(__O_APPEND)
#if __O_APPEND != 0
#define CONFIG_HAVE___O_APPEND 1
#else /* __O_APPEND */
#define CONFIG_NO___O_APPEND 1
#endif /* __O_APPEND */
#elif defined(____O_APPEND__defined)
#define CONFIG_HAVE___O_APPEND 1
#endif

#ifdef CONFIG_NO__O_APPEND
#undef CONFIG_HAVE__O_APPEND
#elif defined(CONFIG_HAVE__O_APPEND)
#elif defined(_O_APPEND)
#if _O_APPEND != 0
#define CONFIG_HAVE__O_APPEND 1
#else /* _O_APPEND */
#define CONFIG_NO__O_APPEND 1
#endif /* _O_APPEND */
#elif defined(___O_APPEND__defined)
#define CONFIG_HAVE__O_APPEND 1
#endif

#ifdef CONFIG_NO_O_NONBLOCK
#undef CONFIG_HAVE_O_NONBLOCK
#elif defined(CONFIG_HAVE_O_NONBLOCK)
#elif defined(O_NONBLOCK)
#if O_NONBLOCK != 0
#define CONFIG_HAVE_O_NONBLOCK 1
#else /* O_NONBLOCK */
#define CONFIG_NO_O_NONBLOCK 1
#endif /* O_NONBLOCK */
#elif defined(__O_NONBLOCK__defined)
#define CONFIG_HAVE_O_NONBLOCK 1
#endif

#ifdef CONFIG_NO___O_NONBLOCK
#undef CONFIG_HAVE___O_NONBLOCK
#elif defined(CONFIG_HAVE___O_NONBLOCK)
#elif defined(__O_NONBLOCK)
#if __O_NONBLOCK != 0
#define CONFIG_HAVE___O_NONBLOCK 1
#else /* __O_NONBLOCK */
#define CONFIG_NO___O_NONBLOCK 1
#endif /* __O_NONBLOCK */
#elif defined(____O_NONBLOCK__defined)
#define CONFIG_HAVE___O_NONBLOCK 1
#endif

#ifdef CONFIG_NO__O_NONBLOCK
#undef CONFIG_HAVE__O_NONBLOCK
#elif defined(CONFIG_HAVE__O_NONBLOCK)
#elif defined(_O_NONBLOCK)
#if _O_NONBLOCK != 0
#define CONFIG_HAVE__O_NONBLOCK 1
#else /* _O_NONBLOCK */
#define CONFIG_NO__O_NONBLOCK 1
#endif /* _O_NONBLOCK */
#elif defined(___O_NONBLOCK__defined)
#define CONFIG_HAVE__O_NONBLOCK 1
#endif

#ifdef CONFIG_NO___O_NDELAY
#undef CONFIG_HAVE___O_NDELAY
#elif defined(CONFIG_HAVE___O_NDELAY)
#elif defined(__O_NDELAY)
#if __O_NDELAY != 0
#define CONFIG_HAVE___O_NDELAY 1
#else /* __O_NDELAY */
#define CONFIG_NO___O_NDELAY 1
#endif /* __O_NDELAY */
#elif defined(____O_NDELAY__defined)
#define CONFIG_HAVE___O_NDELAY 1
#endif

#ifdef CONFIG_NO__O_NDELAY
#undef CONFIG_HAVE__O_NDELAY
#elif defined(CONFIG_HAVE__O_NDELAY)
#elif defined(_O_NDELAY)
#if _O_NDELAY != 0
#define CONFIG_HAVE__O_NDELAY 1
#else /* _O_NDELAY */
#define CONFIG_NO__O_NDELAY 1
#endif /* _O_NDELAY */
#elif defined(___O_NDELAY__defined)
#define CONFIG_HAVE__O_NDELAY 1
#endif

#ifdef CONFIG_NO_O_NDELAY
#undef CONFIG_HAVE_O_NDELAY
#elif defined(CONFIG_HAVE_O_NDELAY)
#elif defined(O_NDELAY)
#if O_NDELAY != 0
#define CONFIG_HAVE_O_NDELAY 1
#else /* O_NDELAY */
#define CONFIG_NO_O_NDELAY 1
#endif /* O_NDELAY */
#elif defined(__O_NDELAY__defined)
#define CONFIG_HAVE_O_NDELAY 1
#endif

#ifdef CONFIG_NO_O_RSYNC
#undef CONFIG_HAVE_O_RSYNC
#elif defined(CONFIG_HAVE_O_RSYNC)
#elif defined(O_RSYNC)
#if O_RSYNC != 0
#define CONFIG_HAVE_O_RSYNC 1
#else /* O_RSYNC */
#define CONFIG_NO_O_RSYNC 1
#endif /* O_RSYNC */
#elif defined(__O_RSYNC__defined)
#define CONFIG_HAVE_O_RSYNC 1
#endif

#ifdef CONFIG_NO___O_RSYNC
#undef CONFIG_HAVE___O_RSYNC
#elif defined(CONFIG_HAVE___O_RSYNC)
#elif defined(__O_RSYNC)
#if __O_RSYNC != 0
#define CONFIG_HAVE___O_RSYNC 1
#else /* __O_RSYNC */
#define CONFIG_NO___O_RSYNC 1
#endif /* __O_RSYNC */
#elif defined(____O_RSYNC__defined)
#define CONFIG_HAVE___O_RSYNC 1
#endif

#ifdef CONFIG_NO__O_RSYNC
#undef CONFIG_HAVE__O_RSYNC
#elif defined(CONFIG_HAVE__O_RSYNC)
#elif defined(_O_RSYNC)
#if _O_RSYNC != 0
#define CONFIG_HAVE__O_RSYNC 1
#else /* _O_RSYNC */
#define CONFIG_NO__O_RSYNC 1
#endif /* _O_RSYNC */
#elif defined(___O_RSYNC__defined)
#define CONFIG_HAVE__O_RSYNC 1
#endif

#ifdef CONFIG_NO_O_SYNC
#undef CONFIG_HAVE_O_SYNC
#elif defined(CONFIG_HAVE_O_SYNC)
#elif defined(O_SYNC)
#if O_SYNC != 0
#define CONFIG_HAVE_O_SYNC 1
#else /* O_SYNC */
#define CONFIG_NO_O_SYNC 1
#endif /* O_SYNC */
#elif defined(__O_SYNC__defined)
#define CONFIG_HAVE_O_SYNC 1
#endif

#ifdef CONFIG_NO___O_SYNC
#undef CONFIG_HAVE___O_SYNC
#elif defined(CONFIG_HAVE___O_SYNC)
#elif defined(__O_SYNC)
#if __O_SYNC != 0
#define CONFIG_HAVE___O_SYNC 1
#else /* __O_SYNC */
#define CONFIG_NO___O_SYNC 1
#endif /* __O_SYNC */
#elif defined(____O_SYNC__defined)
#define CONFIG_HAVE___O_SYNC 1
#endif

#ifdef CONFIG_NO__O_SYNC
#undef CONFIG_HAVE__O_SYNC
#elif defined(CONFIG_HAVE__O_SYNC)
#elif defined(_O_SYNC)
#if _O_SYNC != 0
#define CONFIG_HAVE__O_SYNC 1
#else /* _O_SYNC */
#define CONFIG_NO__O_SYNC 1
#endif /* _O_SYNC */
#elif defined(___O_SYNC__defined)
#define CONFIG_HAVE__O_SYNC 1
#endif

#ifdef CONFIG_NO_O_DSYNC
#undef CONFIG_HAVE_O_DSYNC
#elif defined(CONFIG_HAVE_O_DSYNC)
#elif defined(O_DSYNC)
#if O_DSYNC != 0
#define CONFIG_HAVE_O_DSYNC 1
#else /* O_DSYNC */
#define CONFIG_NO_O_DSYNC 1
#endif /* O_DSYNC */
#elif defined(__O_DSYNC__defined)
#define CONFIG_HAVE_O_DSYNC 1
#endif

#ifdef CONFIG_NO___O_DSYNC
#undef CONFIG_HAVE___O_DSYNC
#elif defined(CONFIG_HAVE___O_DSYNC)
#elif defined(__O_DSYNC)
#if __O_DSYNC != 0
#define CONFIG_HAVE___O_DSYNC 1
#else /* __O_DSYNC */
#define CONFIG_NO___O_DSYNC 1
#endif /* __O_DSYNC */
#elif defined(____O_DSYNC__defined)
#define CONFIG_HAVE___O_DSYNC 1
#endif

#ifdef CONFIG_NO__O_DSYNC
#undef CONFIG_HAVE__O_DSYNC
#elif defined(CONFIG_HAVE__O_DSYNC)
#elif defined(_O_DSYNC)
#if _O_DSYNC != 0
#define CONFIG_HAVE__O_DSYNC 1
#else /* _O_DSYNC */
#define CONFIG_NO__O_DSYNC 1
#endif /* _O_DSYNC */
#elif defined(___O_DSYNC__defined)
#define CONFIG_HAVE__O_DSYNC 1
#endif

#ifdef CONFIG_NO_O_ASYNC
#undef CONFIG_HAVE_O_ASYNC
#elif defined(CONFIG_HAVE_O_ASYNC)
#elif defined(O_ASYNC)
#if O_ASYNC != 0
#define CONFIG_HAVE_O_ASYNC 1
#else /* O_ASYNC */
#define CONFIG_NO_O_ASYNC 1
#endif /* O_ASYNC */
#elif defined(__O_ASYNC__defined)
#define CONFIG_HAVE_O_ASYNC 1
#endif

#ifdef CONFIG_NO___O_ASYNC
#undef CONFIG_HAVE___O_ASYNC
#elif defined(CONFIG_HAVE___O_ASYNC)
#elif defined(__O_ASYNC)
#if __O_ASYNC != 0
#define CONFIG_HAVE___O_ASYNC 1
#else /* __O_ASYNC */
#define CONFIG_NO___O_ASYNC 1
#endif /* __O_ASYNC */
#elif defined(____O_ASYNC__defined)
#define CONFIG_HAVE___O_ASYNC 1
#endif

#ifdef CONFIG_NO__O_ASYNC
#undef CONFIG_HAVE__O_ASYNC
#elif defined(CONFIG_HAVE__O_ASYNC)
#elif defined(_O_ASYNC)
#if _O_ASYNC != 0
#define CONFIG_HAVE__O_ASYNC 1
#else /* _O_ASYNC */
#define CONFIG_NO__O_ASYNC 1
#endif /* _O_ASYNC */
#elif defined(___O_ASYNC__defined)
#define CONFIG_HAVE__O_ASYNC 1
#endif

#ifdef CONFIG_NO_O_DIRECT
#undef CONFIG_HAVE_O_DIRECT
#elif defined(CONFIG_HAVE_O_DIRECT)
#elif defined(O_DIRECT)
#if O_DIRECT != 0
#define CONFIG_HAVE_O_DIRECT 1
#else /* O_DIRECT */
#define CONFIG_NO_O_DIRECT 1
#endif /* O_DIRECT */
#elif defined(__O_DIRECT__defined)
#define CONFIG_HAVE_O_DIRECT 1
#endif

#ifdef CONFIG_NO___O_DIRECT
#undef CONFIG_HAVE___O_DIRECT
#elif defined(CONFIG_HAVE___O_DIRECT)
#elif defined(__O_DIRECT)
#if __O_DIRECT != 0
#define CONFIG_HAVE___O_DIRECT 1
#else /* __O_DIRECT */
#define CONFIG_NO___O_DIRECT 1
#endif /* __O_DIRECT */
#elif defined(____O_DIRECT__defined)
#define CONFIG_HAVE___O_DIRECT 1
#endif

#ifdef CONFIG_NO__O_DIRECT
#undef CONFIG_HAVE__O_DIRECT
#elif defined(CONFIG_HAVE__O_DIRECT)
#elif defined(_O_DIRECT)
#if _O_DIRECT != 0
#define CONFIG_HAVE__O_DIRECT 1
#else /* _O_DIRECT */
#define CONFIG_NO__O_DIRECT 1
#endif /* _O_DIRECT */
#elif defined(___O_DIRECT__defined)
#define CONFIG_HAVE__O_DIRECT 1
#endif

#ifdef CONFIG_NO_O_LARGEFILE
#undef CONFIG_HAVE_O_LARGEFILE
#elif defined(CONFIG_HAVE_O_LARGEFILE)
#elif defined(O_LARGEFILE)
#if O_LARGEFILE != 0
#define CONFIG_HAVE_O_LARGEFILE 1
#else /* O_LARGEFILE */
#define CONFIG_NO_O_LARGEFILE 1
#endif /* O_LARGEFILE */
#elif defined(__O_LARGEFILE__defined)
#define CONFIG_HAVE_O_LARGEFILE 1
#endif

#ifdef CONFIG_NO___O_LARGEFILE
#undef CONFIG_HAVE___O_LARGEFILE
#elif defined(CONFIG_HAVE___O_LARGEFILE)
#elif defined(__O_LARGEFILE)
#if __O_LARGEFILE != 0
#define CONFIG_HAVE___O_LARGEFILE 1
#else /* __O_LARGEFILE */
#define CONFIG_NO___O_LARGEFILE 1
#endif /* __O_LARGEFILE */
#elif defined(____O_LARGEFILE__defined)
#define CONFIG_HAVE___O_LARGEFILE 1
#endif

#ifdef CONFIG_NO__O_LARGEFILE
#undef CONFIG_HAVE__O_LARGEFILE
#elif defined(CONFIG_HAVE__O_LARGEFILE)
#elif defined(_O_LARGEFILE)
#if _O_LARGEFILE != 0
#define CONFIG_HAVE__O_LARGEFILE 1
#else /* _O_LARGEFILE */
#define CONFIG_NO__O_LARGEFILE 1
#endif /* _O_LARGEFILE */
#elif defined(___O_LARGEFILE__defined)
#define CONFIG_HAVE__O_LARGEFILE 1
#endif

#ifdef CONFIG_NO_O_DIRECTORY
#undef CONFIG_HAVE_O_DIRECTORY
#elif defined(CONFIG_HAVE_O_DIRECTORY)
#elif defined(O_DIRECTORY)
#if O_DIRECTORY != 0
#define CONFIG_HAVE_O_DIRECTORY 1
#else /* O_DIRECTORY */
#define CONFIG_NO_O_DIRECTORY 1
#endif /* O_DIRECTORY */
#elif defined(__O_DIRECTORY__defined)
#define CONFIG_HAVE_O_DIRECTORY 1
#endif

#ifdef CONFIG_NO___O_DIRECTORY
#undef CONFIG_HAVE___O_DIRECTORY
#elif defined(CONFIG_HAVE___O_DIRECTORY)
#elif defined(__O_DIRECTORY)
#if __O_DIRECTORY != 0
#define CONFIG_HAVE___O_DIRECTORY 1
#else /* __O_DIRECTORY */
#define CONFIG_NO___O_DIRECTORY 1
#endif /* __O_DIRECTORY */
#elif defined(____O_DIRECTORY__defined)
#define CONFIG_HAVE___O_DIRECTORY 1
#endif

#ifdef CONFIG_NO__O_DIRECTORY
#undef CONFIG_HAVE__O_DIRECTORY
#elif defined(CONFIG_HAVE__O_DIRECTORY)
#elif defined(_O_DIRECTORY)
#if _O_DIRECTORY != 0
#define CONFIG_HAVE__O_DIRECTORY 1
#else /* _O_DIRECTORY */
#define CONFIG_NO__O_DIRECTORY 1
#endif /* _O_DIRECTORY */
#elif defined(___O_DIRECTORY__defined)
#define CONFIG_HAVE__O_DIRECTORY 1
#endif

#ifdef CONFIG_NO_O_NOFOLLOW
#undef CONFIG_HAVE_O_NOFOLLOW
#elif defined(CONFIG_HAVE_O_NOFOLLOW)
#elif defined(O_NOFOLLOW)
#if O_NOFOLLOW != 0
#define CONFIG_HAVE_O_NOFOLLOW 1
#else /* O_NOFOLLOW */
#define CONFIG_NO_O_NOFOLLOW 1
#endif /* O_NOFOLLOW */
#elif defined(__O_NOFOLLOW__defined)
#define CONFIG_HAVE_O_NOFOLLOW 1
#endif

#ifdef CONFIG_NO___O_NOFOLLOW
#undef CONFIG_HAVE___O_NOFOLLOW
#elif defined(CONFIG_HAVE___O_NOFOLLOW)
#elif defined(__O_NOFOLLOW)
#if __O_NOFOLLOW != 0
#define CONFIG_HAVE___O_NOFOLLOW 1
#else /* __O_NOFOLLOW */
#define CONFIG_NO___O_NOFOLLOW 1
#endif /* __O_NOFOLLOW */
#elif defined(____O_NOFOLLOW__defined)
#define CONFIG_HAVE___O_NOFOLLOW 1
#endif

#ifdef CONFIG_NO__O_NOFOLLOW
#undef CONFIG_HAVE__O_NOFOLLOW
#elif defined(CONFIG_HAVE__O_NOFOLLOW)
#elif defined(_O_NOFOLLOW)
#if _O_NOFOLLOW != 0
#define CONFIG_HAVE__O_NOFOLLOW 1
#else /* _O_NOFOLLOW */
#define CONFIG_NO__O_NOFOLLOW 1
#endif /* _O_NOFOLLOW */
#elif defined(___O_NOFOLLOW__defined)
#define CONFIG_HAVE__O_NOFOLLOW 1
#endif

#ifdef CONFIG_NO_O_TMPFILE
#undef CONFIG_HAVE_O_TMPFILE
#elif defined(CONFIG_HAVE_O_TMPFILE)
#elif defined(O_TMPFILE)
#if O_TMPFILE != 0
#define CONFIG_HAVE_O_TMPFILE 1
#else /* O_TMPFILE */
#define CONFIG_NO_O_TMPFILE 1
#endif /* O_TMPFILE */
#elif defined(__O_TMPFILE__defined)
#define CONFIG_HAVE_O_TMPFILE 1
#endif

#ifdef CONFIG_NO___O_TMPFILE
#undef CONFIG_HAVE___O_TMPFILE
#elif defined(CONFIG_HAVE___O_TMPFILE)
#elif defined(__O_TMPFILE)
#if __O_TMPFILE != 0
#define CONFIG_HAVE___O_TMPFILE 1
#else /* __O_TMPFILE */
#define CONFIG_NO___O_TMPFILE 1
#endif /* __O_TMPFILE */
#elif defined(____O_TMPFILE__defined)
#define CONFIG_HAVE___O_TMPFILE 1
#endif

#ifdef CONFIG_NO__O_TMPFILE
#undef CONFIG_HAVE__O_TMPFILE
#elif defined(CONFIG_HAVE__O_TMPFILE)
#elif defined(_O_TMPFILE)
#if _O_TMPFILE != 0
#define CONFIG_HAVE__O_TMPFILE 1
#else /* _O_TMPFILE */
#define CONFIG_NO__O_TMPFILE 1
#endif /* _O_TMPFILE */
#elif defined(___O_TMPFILE__defined)
#define CONFIG_HAVE__O_TMPFILE 1
#endif

#ifdef CONFIG_NO_O_CLOFORK
#undef CONFIG_HAVE_O_CLOFORK
#elif defined(CONFIG_HAVE_O_CLOFORK)
#elif defined(O_CLOFORK)
#if O_CLOFORK != 0
#define CONFIG_HAVE_O_CLOFORK 1
#else /* O_CLOFORK */
#define CONFIG_NO_O_CLOFORK 1
#endif /* O_CLOFORK */
#elif defined(__O_CLOFORK__defined)
#define CONFIG_HAVE_O_CLOFORK 1
#endif

#ifdef CONFIG_NO___O_CLOFORK
#undef CONFIG_HAVE___O_CLOFORK
#elif defined(CONFIG_HAVE___O_CLOFORK)
#elif defined(__O_CLOFORK)
#if __O_CLOFORK != 0
#define CONFIG_HAVE___O_CLOFORK 1
#else /* __O_CLOFORK */
#define CONFIG_NO___O_CLOFORK 1
#endif /* __O_CLOFORK */
#elif defined(____O_CLOFORK__defined)
#define CONFIG_HAVE___O_CLOFORK 1
#endif

#ifdef CONFIG_NO__O_CLOFORK
#undef CONFIG_HAVE__O_CLOFORK
#elif defined(CONFIG_HAVE__O_CLOFORK)
#elif defined(_O_CLOFORK)
#if _O_CLOFORK != 0
#define CONFIG_HAVE__O_CLOFORK 1
#else /* _O_CLOFORK */
#define CONFIG_NO__O_CLOFORK 1
#endif /* _O_CLOFORK */
#elif defined(___O_CLOFORK__defined)
#define CONFIG_HAVE__O_CLOFORK 1
#endif

#ifdef CONFIG_NO_O_SYMLINK
#undef CONFIG_HAVE_O_SYMLINK
#elif defined(CONFIG_HAVE_O_SYMLINK)
#elif defined(O_SYMLINK)
#if O_SYMLINK != 0
#define CONFIG_HAVE_O_SYMLINK 1
#else /* O_SYMLINK */
#define CONFIG_NO_O_SYMLINK 1
#endif /* O_SYMLINK */
#elif defined(__O_SYMLINK__defined)
#define CONFIG_HAVE_O_SYMLINK 1
#endif

#ifdef CONFIG_NO___O_SYMLINK
#undef CONFIG_HAVE___O_SYMLINK
#elif defined(CONFIG_HAVE___O_SYMLINK)
#elif defined(__O_SYMLINK)
#if __O_SYMLINK != 0
#define CONFIG_HAVE___O_SYMLINK 1
#else /* __O_SYMLINK */
#define CONFIG_NO___O_SYMLINK 1
#endif /* __O_SYMLINK */
#elif defined(____O_SYMLINK__defined)
#define CONFIG_HAVE___O_SYMLINK 1
#endif

#ifdef CONFIG_NO__O_SYMLINK
#undef CONFIG_HAVE__O_SYMLINK
#elif defined(CONFIG_HAVE__O_SYMLINK)
#elif defined(_O_SYMLINK)
#if _O_SYMLINK != 0
#define CONFIG_HAVE__O_SYMLINK 1
#else /* _O_SYMLINK */
#define CONFIG_NO__O_SYMLINK 1
#endif /* _O_SYMLINK */
#elif defined(___O_SYMLINK__defined)
#define CONFIG_HAVE__O_SYMLINK 1
#endif

#ifdef CONFIG_NO_O_DOSPATH
#undef CONFIG_HAVE_O_DOSPATH
#elif defined(CONFIG_HAVE_O_DOSPATH)
#elif defined(O_DOSPATH)
#if O_DOSPATH != 0
#define CONFIG_HAVE_O_DOSPATH 1
#else /* O_DOSPATH */
#define CONFIG_NO_O_DOSPATH 1
#endif /* O_DOSPATH */
#elif defined(__O_DOSPATH__defined)
#define CONFIG_HAVE_O_DOSPATH 1
#endif

#ifdef CONFIG_NO___O_DOSPATH
#undef CONFIG_HAVE___O_DOSPATH
#elif defined(CONFIG_HAVE___O_DOSPATH)
#elif defined(__O_DOSPATH)
#if __O_DOSPATH != 0
#define CONFIG_HAVE___O_DOSPATH 1
#else /* __O_DOSPATH */
#define CONFIG_NO___O_DOSPATH 1
#endif /* __O_DOSPATH */
#elif defined(____O_DOSPATH__defined)
#define CONFIG_HAVE___O_DOSPATH 1
#endif

#ifdef CONFIG_NO__O_DOSPATH
#undef CONFIG_HAVE__O_DOSPATH
#elif defined(CONFIG_HAVE__O_DOSPATH)
#elif defined(_O_DOSPATH)
#if _O_DOSPATH != 0
#define CONFIG_HAVE__O_DOSPATH 1
#else /* _O_DOSPATH */
#define CONFIG_NO__O_DOSPATH 1
#endif /* _O_DOSPATH */
#elif defined(___O_DOSPATH__defined)
#define CONFIG_HAVE__O_DOSPATH 1
#endif

#ifdef CONFIG_NO_O_SHLOCK
#undef CONFIG_HAVE_O_SHLOCK
#elif defined(CONFIG_HAVE_O_SHLOCK)
#elif defined(O_SHLOCK)
#if O_SHLOCK != 0
#define CONFIG_HAVE_O_SHLOCK 1
#else /* O_SHLOCK */
#define CONFIG_NO_O_SHLOCK 1
#endif /* O_SHLOCK */
#elif defined(__O_SHLOCK__defined)
#define CONFIG_HAVE_O_SHLOCK 1
#endif

#ifdef CONFIG_NO___O_SHLOCK
#undef CONFIG_HAVE___O_SHLOCK
#elif defined(CONFIG_HAVE___O_SHLOCK)
#elif defined(__O_SHLOCK)
#if __O_SHLOCK != 0
#define CONFIG_HAVE___O_SHLOCK 1
#else /* __O_SHLOCK */
#define CONFIG_NO___O_SHLOCK 1
#endif /* __O_SHLOCK */
#elif defined(____O_SHLOCK__defined)
#define CONFIG_HAVE___O_SHLOCK 1
#endif

#ifdef CONFIG_NO__O_SHLOCK
#undef CONFIG_HAVE__O_SHLOCK
#elif defined(CONFIG_HAVE__O_SHLOCK)
#elif defined(_O_SHLOCK)
#if _O_SHLOCK != 0
#define CONFIG_HAVE__O_SHLOCK 1
#else /* _O_SHLOCK */
#define CONFIG_NO__O_SHLOCK 1
#endif /* _O_SHLOCK */
#elif defined(___O_SHLOCK__defined)
#define CONFIG_HAVE__O_SHLOCK 1
#endif

#ifdef CONFIG_NO_O_EXLOCK
#undef CONFIG_HAVE_O_EXLOCK
#elif defined(CONFIG_HAVE_O_EXLOCK)
#elif defined(O_EXLOCK)
#if O_EXLOCK != 0
#define CONFIG_HAVE_O_EXLOCK 1
#else /* O_EXLOCK */
#define CONFIG_NO_O_EXLOCK 1
#endif /* O_EXLOCK */
#elif defined(__O_EXLOCK__defined)
#define CONFIG_HAVE_O_EXLOCK 1
#endif

#ifdef CONFIG_NO___O_EXLOCK
#undef CONFIG_HAVE___O_EXLOCK
#elif defined(CONFIG_HAVE___O_EXLOCK)
#elif defined(__O_EXLOCK)
#if __O_EXLOCK != 0
#define CONFIG_HAVE___O_EXLOCK 1
#else /* __O_EXLOCK */
#define CONFIG_NO___O_EXLOCK 1
#endif /* __O_EXLOCK */
#elif defined(____O_EXLOCK__defined)
#define CONFIG_HAVE___O_EXLOCK 1
#endif

#ifdef CONFIG_NO__O_EXLOCK
#undef CONFIG_HAVE__O_EXLOCK
#elif defined(CONFIG_HAVE__O_EXLOCK)
#elif defined(_O_EXLOCK)
#if _O_EXLOCK != 0
#define CONFIG_HAVE__O_EXLOCK 1
#else /* _O_EXLOCK */
#define CONFIG_NO__O_EXLOCK 1
#endif /* _O_EXLOCK */
#elif defined(___O_EXLOCK__defined)
#define CONFIG_HAVE__O_EXLOCK 1
#endif

#ifdef CONFIG_NO_O_XATTR
#undef CONFIG_HAVE_O_XATTR
#elif defined(CONFIG_HAVE_O_XATTR)
#elif defined(O_XATTR)
#if O_XATTR != 0
#define CONFIG_HAVE_O_XATTR 1
#else /* O_XATTR */
#define CONFIG_NO_O_XATTR 1
#endif /* O_XATTR */
#elif defined(__O_XATTR__defined)
#define CONFIG_HAVE_O_XATTR 1
#endif

#ifdef CONFIG_NO___O_XATTR
#undef CONFIG_HAVE___O_XATTR
#elif defined(CONFIG_HAVE___O_XATTR)
#elif defined(__O_XATTR)
#if __O_XATTR != 0
#define CONFIG_HAVE___O_XATTR 1
#else /* __O_XATTR */
#define CONFIG_NO___O_XATTR 1
#endif /* __O_XATTR */
#elif defined(____O_XATTR__defined)
#define CONFIG_HAVE___O_XATTR 1
#endif

#ifdef CONFIG_NO__O_XATTR
#undef CONFIG_HAVE__O_XATTR
#elif defined(CONFIG_HAVE__O_XATTR)
#elif defined(_O_XATTR)
#if _O_XATTR != 0
#define CONFIG_HAVE__O_XATTR 1
#else /* _O_XATTR */
#define CONFIG_NO__O_XATTR 1
#endif /* _O_XATTR */
#elif defined(___O_XATTR__defined)
#define CONFIG_HAVE__O_XATTR 1
#endif

#ifdef CONFIG_NO_O_EXEC
#undef CONFIG_HAVE_O_EXEC
#elif defined(CONFIG_HAVE_O_EXEC)
#elif defined(O_EXEC)
#if O_EXEC != 0
#define CONFIG_HAVE_O_EXEC 1
#else /* O_EXEC */
#define CONFIG_NO_O_EXEC 1
#endif /* O_EXEC */
#elif defined(__O_EXEC__defined)
#define CONFIG_HAVE_O_EXEC 1
#endif

#ifdef CONFIG_NO___O_EXEC
#undef CONFIG_HAVE___O_EXEC
#elif defined(CONFIG_HAVE___O_EXEC)
#elif defined(__O_EXEC)
#if __O_EXEC != 0
#define CONFIG_HAVE___O_EXEC 1
#else /* __O_EXEC */
#define CONFIG_NO___O_EXEC 1
#endif /* __O_EXEC */
#elif defined(____O_EXEC__defined)
#define CONFIG_HAVE___O_EXEC 1
#endif

#ifdef CONFIG_NO__O_EXEC
#undef CONFIG_HAVE__O_EXEC
#elif defined(CONFIG_HAVE__O_EXEC)
#elif defined(_O_EXEC)
#if _O_EXEC != 0
#define CONFIG_HAVE__O_EXEC 1
#else /* _O_EXEC */
#define CONFIG_NO__O_EXEC 1
#endif /* _O_EXEC */
#elif defined(___O_EXEC__defined)
#define CONFIG_HAVE__O_EXEC 1
#endif

#ifdef CONFIG_NO_O_SEARCH
#undef CONFIG_HAVE_O_SEARCH
#elif defined(CONFIG_HAVE_O_SEARCH)
#elif defined(O_SEARCH)
#if O_SEARCH != 0
#define CONFIG_HAVE_O_SEARCH 1
#else /* O_SEARCH */
#define CONFIG_NO_O_SEARCH 1
#endif /* O_SEARCH */
#elif defined(__O_SEARCH__defined)
#define CONFIG_HAVE_O_SEARCH 1
#endif

#ifdef CONFIG_NO___O_SEARCH
#undef CONFIG_HAVE___O_SEARCH
#elif defined(CONFIG_HAVE___O_SEARCH)
#elif defined(__O_SEARCH)
#if __O_SEARCH != 0
#define CONFIG_HAVE___O_SEARCH 1
#else /* __O_SEARCH */
#define CONFIG_NO___O_SEARCH 1
#endif /* __O_SEARCH */
#elif defined(____O_SEARCH__defined)
#define CONFIG_HAVE___O_SEARCH 1
#endif

#ifdef CONFIG_NO__O_SEARCH
#undef CONFIG_HAVE__O_SEARCH
#elif defined(CONFIG_HAVE__O_SEARCH)
#elif defined(_O_SEARCH)
#if _O_SEARCH != 0
#define CONFIG_HAVE__O_SEARCH 1
#else /* _O_SEARCH */
#define CONFIG_NO__O_SEARCH 1
#endif /* _O_SEARCH */
#elif defined(___O_SEARCH__defined)
#define CONFIG_HAVE__O_SEARCH 1
#endif

#ifdef CONFIG_NO_O_TTY_INIT
#undef CONFIG_HAVE_O_TTY_INIT
#elif defined(CONFIG_HAVE_O_TTY_INIT)
#elif defined(O_TTY_INIT)
#if O_TTY_INIT != 0
#define CONFIG_HAVE_O_TTY_INIT 1
#else /* O_TTY_INIT */
#define CONFIG_NO_O_TTY_INIT 1
#endif /* O_TTY_INIT */
#elif defined(__O_TTY_INIT__defined)
#define CONFIG_HAVE_O_TTY_INIT 1
#endif

#ifdef CONFIG_NO___O_TTY_INIT
#undef CONFIG_HAVE___O_TTY_INIT
#elif defined(CONFIG_HAVE___O_TTY_INIT)
#elif defined(__O_TTY_INIT)
#if __O_TTY_INIT != 0
#define CONFIG_HAVE___O_TTY_INIT 1
#else /* __O_TTY_INIT */
#define CONFIG_NO___O_TTY_INIT 1
#endif /* __O_TTY_INIT */
#elif defined(____O_TTY_INIT__defined)
#define CONFIG_HAVE___O_TTY_INIT 1
#endif

#ifdef CONFIG_NO__O_TTY_INIT
#undef CONFIG_HAVE__O_TTY_INIT
#elif defined(CONFIG_HAVE__O_TTY_INIT)
#elif defined(_O_TTY_INIT)
#if _O_TTY_INIT != 0
#define CONFIG_HAVE__O_TTY_INIT 1
#else /* _O_TTY_INIT */
#define CONFIG_NO__O_TTY_INIT 1
#endif /* _O_TTY_INIT */
#elif defined(___O_TTY_INIT__defined)
#define CONFIG_HAVE__O_TTY_INIT 1
#endif

#ifdef CONFIG_NO_O_NOLINKS
#undef CONFIG_HAVE_O_NOLINKS
#elif defined(CONFIG_HAVE_O_NOLINKS)
#elif defined(O_NOLINKS)
#if O_NOLINKS != 0
#define CONFIG_HAVE_O_NOLINKS 1
#else /* O_NOLINKS */
#define CONFIG_NO_O_NOLINKS 1
#endif /* O_NOLINKS */
#elif defined(__O_NOLINKS__defined)
#define CONFIG_HAVE_O_NOLINKS 1
#endif

#ifdef CONFIG_NO___O_NOLINKS
#undef CONFIG_HAVE___O_NOLINKS
#elif defined(CONFIG_HAVE___O_NOLINKS)
#elif defined(__O_NOLINKS)
#if __O_NOLINKS != 0
#define CONFIG_HAVE___O_NOLINKS 1
#else /* __O_NOLINKS */
#define CONFIG_NO___O_NOLINKS 1
#endif /* __O_NOLINKS */
#elif defined(____O_NOLINKS__defined)
#define CONFIG_HAVE___O_NOLINKS 1
#endif

#ifdef CONFIG_NO__O_NOLINKS
#undef CONFIG_HAVE__O_NOLINKS
#elif defined(CONFIG_HAVE__O_NOLINKS)
#elif defined(_O_NOLINKS)
#if _O_NOLINKS != 0
#define CONFIG_HAVE__O_NOLINKS 1
#else /* _O_NOLINKS */
#define CONFIG_NO__O_NOLINKS 1
#endif /* _O_NOLINKS */
#elif defined(___O_NOLINKS__defined)
#define CONFIG_HAVE__O_NOLINKS 1
#endif

#ifdef CONFIG_NO_AT_SYMLINK_NOFOLLOW
#undef CONFIG_HAVE_AT_SYMLINK_NOFOLLOW
#elif !defined(CONFIG_HAVE_AT_SYMLINK_NOFOLLOW) && \
      (defined(AT_SYMLINK_NOFOLLOW) || defined(__AT_SYMLINK_NOFOLLOW_defined))
#define CONFIG_HAVE_AT_SYMLINK_NOFOLLOW 1
#endif

#ifdef CONFIG_NO_AT_REMOVEDIR
#undef CONFIG_HAVE_AT_REMOVEDIR
#elif !defined(CONFIG_HAVE_AT_REMOVEDIR) && \
      (defined(AT_REMOVEDIR) || defined(__AT_REMOVEDIR_defined))
#define CONFIG_HAVE_AT_REMOVEDIR 1
#endif

#ifdef CONFIG_NO_AT_EACCESS
#undef CONFIG_HAVE_AT_EACCESS
#elif !defined(CONFIG_HAVE_AT_EACCESS) && \
      (defined(AT_EACCESS) || defined(__AT_EACCESS_defined))
#define CONFIG_HAVE_AT_EACCESS 1
#endif

#ifdef CONFIG_NO_AT_SYMLINK_FOLLOW
#undef CONFIG_HAVE_AT_SYMLINK_FOLLOW
#elif !defined(CONFIG_HAVE_AT_SYMLINK_FOLLOW) && \
      (defined(AT_SYMLINK_FOLLOW) || defined(__AT_SYMLINK_FOLLOW_defined))
#define CONFIG_HAVE_AT_SYMLINK_FOLLOW 1
#endif

#ifdef CONFIG_NO_AT_NO_AUTOMOUNT
#undef CONFIG_HAVE_AT_NO_AUTOMOUNT
#elif !defined(CONFIG_HAVE_AT_NO_AUTOMOUNT) && \
      (defined(AT_NO_AUTOMOUNT) || defined(__AT_NO_AUTOMOUNT_defined))
#define CONFIG_HAVE_AT_NO_AUTOMOUNT 1
#endif

#ifdef CONFIG_NO_AT_EMPTY_PATH
#undef CONFIG_HAVE_AT_EMPTY_PATH
#elif !defined(CONFIG_HAVE_AT_EMPTY_PATH) && \
      (defined(AT_EMPTY_PATH) || defined(__AT_EMPTY_PATH_defined))
#define CONFIG_HAVE_AT_EMPTY_PATH 1
#endif

#ifdef CONFIG_NO_AT_SYMLINK_REGULAR
#undef CONFIG_HAVE_AT_SYMLINK_REGULAR
#elif !defined(CONFIG_HAVE_AT_SYMLINK_REGULAR) && \
      (defined(AT_SYMLINK_REGULAR) || defined(__AT_SYMLINK_REGULAR_defined))
#define CONFIG_HAVE_AT_SYMLINK_REGULAR 1
#endif

#ifdef CONFIG_NO_AT_CHANGE_CTIME
#undef CONFIG_HAVE_AT_CHANGE_CTIME
#elif !defined(CONFIG_HAVE_AT_CHANGE_CTIME) && \
      (defined(AT_CHANGE_CTIME) || defined(__AT_CHANGE_CTIME_defined))
#define CONFIG_HAVE_AT_CHANGE_CTIME 1
#endif

#ifdef CONFIG_NO_AT_REMOVEREG
#undef CONFIG_HAVE_AT_REMOVEREG
#elif !defined(CONFIG_HAVE_AT_REMOVEREG) && \
      (defined(AT_REMOVEREG) || defined(__AT_REMOVEREG_defined))
#define CONFIG_HAVE_AT_REMOVEREG 1
#endif

#ifdef CONFIG_NO_AT_ALTPATH
#undef CONFIG_HAVE_AT_ALTPATH
#elif !defined(CONFIG_HAVE_AT_ALTPATH) && \
      (defined(AT_ALTPATH) || defined(__AT_ALTPATH_defined))
#define CONFIG_HAVE_AT_ALTPATH 1
#endif

#ifdef CONFIG_NO_AT_DOSPATH
#undef CONFIG_HAVE_AT_DOSPATH
#elif !defined(CONFIG_HAVE_AT_DOSPATH) && \
      (defined(AT_DOSPATH) || defined(__AT_DOSPATH_defined))
#define CONFIG_HAVE_AT_DOSPATH 1
#endif

#ifdef CONFIG_NO_AT_FDCWD
#undef CONFIG_HAVE_AT_FDCWD
#elif !defined(CONFIG_HAVE_AT_FDCWD) && \
      (defined(AT_FDCWD) || defined(__AT_FDCWD_defined))
#define CONFIG_HAVE_AT_FDCWD 1
#endif

#ifdef CONFIG_NO_AT_FDROOT
#undef CONFIG_HAVE_AT_FDROOT
#elif !defined(CONFIG_HAVE_AT_FDROOT) && \
      (defined(AT_FDROOT) || defined(__AT_FDROOT_defined))
#define CONFIG_HAVE_AT_FDROOT 1
#endif

#ifdef CONFIG_NO_AT_THIS_TASK
#undef CONFIG_HAVE_AT_THIS_TASK
#elif !defined(CONFIG_HAVE_AT_THIS_TASK) && \
      (defined(AT_THIS_TASK) || defined(__AT_THIS_TASK_defined))
#define CONFIG_HAVE_AT_THIS_TASK 1
#endif

#ifdef CONFIG_NO_AT_THIS_PROCESS
#undef CONFIG_HAVE_AT_THIS_PROCESS
#elif !defined(CONFIG_HAVE_AT_THIS_PROCESS) && \
      (defined(AT_THIS_PROCESS) || defined(__AT_THIS_PROCESS_defined))
#define CONFIG_HAVE_AT_THIS_PROCESS 1
#endif

#ifdef CONFIG_NO_AT_PARENT_PROCESS
#undef CONFIG_HAVE_AT_PARENT_PROCESS
#elif !defined(CONFIG_HAVE_AT_PARENT_PROCESS) && \
      (defined(AT_PARENT_PROCESS) || defined(__AT_PARENT_PROCESS_defined))
#define CONFIG_HAVE_AT_PARENT_PROCESS 1
#endif

#ifdef CONFIG_NO_AT_GROUP_LEADER
#undef CONFIG_HAVE_AT_GROUP_LEADER
#elif !defined(CONFIG_HAVE_AT_GROUP_LEADER) && \
      (defined(AT_GROUP_LEADER) || defined(__AT_GROUP_LEADER_defined))
#define CONFIG_HAVE_AT_GROUP_LEADER 1
#endif

#ifdef CONFIG_NO_AT_SESSION_LEADER
#undef CONFIG_HAVE_AT_SESSION_LEADER
#elif !defined(CONFIG_HAVE_AT_SESSION_LEADER) && \
      (defined(AT_SESSION_LEADER) || defined(__AT_SESSION_LEADER_defined))
#define CONFIG_HAVE_AT_SESSION_LEADER 1
#endif

#ifdef CONFIG_NO_AT_DOS_DRIVEMIN
#undef CONFIG_HAVE_AT_DOS_DRIVEMIN
#elif !defined(CONFIG_HAVE_AT_DOS_DRIVEMIN) && \
      (defined(AT_DOS_DRIVEMIN) || defined(__AT_DOS_DRIVEMIN_defined))
#define CONFIG_HAVE_AT_DOS_DRIVEMIN 1
#endif

#ifdef CONFIG_NO_AT_DOS_DRIVEMAX
#undef CONFIG_HAVE_AT_DOS_DRIVEMAX
#elif !defined(CONFIG_HAVE_AT_DOS_DRIVEMAX) && \
      (defined(AT_DOS_DRIVEMAX) || defined(__AT_DOS_DRIVEMAX_defined))
#define CONFIG_HAVE_AT_DOS_DRIVEMAX 1
#endif

#ifdef CONFIG_NO_AT_FDDRIVE_CWD
#undef CONFIG_HAVE_AT_FDDRIVE_CWD
#elif 0
#define CONFIG_HAVE_AT_FDDRIVE_CWD 1
#endif

#ifdef CONFIG_NO_AT_FDDRIVE_ROOT
#undef CONFIG_HAVE_AT_FDDRIVE_ROOT
#elif 0
#define CONFIG_HAVE_AT_FDDRIVE_ROOT 1
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

#ifdef CONFIG_NO__lseek64
#undef CONFIG_HAVE__lseek64
#elif !defined(CONFIG_HAVE__lseek64) && \
      (defined(_lseek64) || defined(___lseek64_defined))
#define CONFIG_HAVE__lseek64 1
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

#ifdef CONFIG_NO_wchdir
#undef CONFIG_HAVE_wchdir
#elif !defined(CONFIG_HAVE_wchdir) && \
      (defined(wchdir) || defined(__wchdir_defined))
#define CONFIG_HAVE_wchdir 1
#endif

#ifdef CONFIG_NO__wchdir
#undef CONFIG_HAVE__wchdir
#elif !defined(CONFIG_HAVE__wchdir) && \
      (defined(_wchdir) || defined(___wchdir_defined) || defined(_WDIRECT_DEFINED))
#define CONFIG_HAVE__wchdir 1
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
       defined(__USE_KOS) && defined(__CRT_HAVE_freadlinkat) && defined(AT_READLINK_REQSIZE)))
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

#ifdef CONFIG_NO__wstat
#undef CONFIG_HAVE__wstat
#elif !defined(CONFIG_HAVE__wstat) && \
      (defined(_wstat) || defined(___wstat_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
#define CONFIG_HAVE__wstat 1
#endif

#ifdef CONFIG_NO_wstat
#undef CONFIG_HAVE_wstat
#elif !defined(CONFIG_HAVE_wstat) && \
      (defined(wstat) || defined(__wstat_defined))
#define CONFIG_HAVE_wstat 1
#endif

#ifdef CONFIG_NO__wstat64
#undef CONFIG_HAVE__wstat64
#elif !defined(CONFIG_HAVE__wstat64) && \
      (defined(_wstat64) || defined(___wstat64_defined) || (defined(_WIO_DEFINED) && \
       defined(__USE_LARGEFILE64)))
#define CONFIG_HAVE__wstat64 1
#endif

#ifdef CONFIG_NO_wstat64
#undef CONFIG_HAVE_wstat64
#elif !defined(CONFIG_HAVE_wstat64) && \
      (defined(wstat64) || defined(__wstat64_defined))
#define CONFIG_HAVE_wstat64 1
#endif

#ifdef CONFIG_NO__wlstat
#undef CONFIG_HAVE__wlstat
#elif !defined(CONFIG_HAVE__wlstat) && \
      (defined(_wlstat) || defined(___wlstat_defined))
#define CONFIG_HAVE__wlstat 1
#endif

#ifdef CONFIG_NO_wlstat
#undef CONFIG_HAVE_wlstat
#elif !defined(CONFIG_HAVE_wlstat) && \
      (defined(wlstat) || defined(__wlstat_defined))
#define CONFIG_HAVE_wlstat 1
#endif

#ifdef CONFIG_NO__wlstat64
#undef CONFIG_HAVE__wlstat64
#elif !defined(CONFIG_HAVE__wlstat64) && \
      (defined(_wlstat64) || defined(___wlstat64_defined))
#define CONFIG_HAVE__wlstat64 1
#endif

#ifdef CONFIG_NO_wlstat64
#undef CONFIG_HAVE_wlstat64
#elif !defined(CONFIG_HAVE_wlstat64) && \
      (defined(wlstat64) || defined(__wlstat64_defined))
#define CONFIG_HAVE_wlstat64 1
#endif

#ifdef CONFIG_NO_STAT_ST_NSEC
#undef CONFIG_HAVE_STAT_ST_NSEC
#elif !defined(CONFIG_HAVE_STAT_ST_NSEC) && \
      (defined(CONFIG_HAVE_stat) && (defined(_STATBUF_ST_NSEC) && !defined(__USE_XOPEN2K8)))
#define CONFIG_HAVE_STAT_ST_NSEC 1
#endif

#ifdef CONFIG_NO_STAT_ST_TIM
#undef CONFIG_HAVE_STAT_ST_TIM
#elif !defined(CONFIG_HAVE_STAT_ST_TIM) && \
      (defined(CONFIG_HAVE_stat) && (defined(_STATBUF_ST_TIM) || (defined(_STATBUF_ST_NSEC) && \
       defined(__USE_XOPEN2K8))))
#define CONFIG_HAVE_STAT_ST_TIM 1
#endif

#ifdef CONFIG_NO_STAT_ST_TIMESPEC
#undef CONFIG_HAVE_STAT_ST_TIMESPEC
#elif !defined(CONFIG_HAVE_STAT_ST_TIMESPEC) && \
      (defined(CONFIG_HAVE_stat) && defined(_STATBUF_ST_TIMESPEC))
#define CONFIG_HAVE_STAT_ST_TIMESPEC 1
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

#ifdef CONFIG_NO_wmkdir
#undef CONFIG_HAVE_wmkdir
#elif !defined(CONFIG_HAVE_wmkdir) && \
      (defined(wmkdir) || defined(__wmkdir_defined))
#define CONFIG_HAVE_wmkdir 1
#endif

#ifdef CONFIG_NO__wmkdir
#undef CONFIG_HAVE__wmkdir
#elif !defined(CONFIG_HAVE__wmkdir) && \
      (defined(_wmkdir) || defined(___wmkdir_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__wmkdir 1
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
      (defined(_wchmod) || defined(___wchmod_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
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

#ifdef CONFIG_NO_fmkdirat
#undef CONFIG_HAVE_fmkdirat
#elif !defined(CONFIG_HAVE_fmkdirat) && \
      (defined(fmkdirat) || defined(__fmkdirat_defined) || (defined(CONFIG_HAVE_SYS_STAT_H) && \
       defined(__USE_KOS) && defined(__USE_ATFILE)))
#define CONFIG_HAVE_fmkdirat 1
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

#ifdef _MSC_VER
#define F_OK     0
#define X_OK     1 // Not supported?
#define W_OK     2
#define R_OK     4
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

#ifdef CONFIG_NO_access
#undef CONFIG_HAVE_access
#elif !defined(CONFIG_HAVE_access) && \
      (defined(access) || defined(__access_defined) || ((defined(CONFIG_HAVE_UNISTD_H) || \
       !defined(_MSC_VER)) && defined(F_OK) && defined(X_OK) && defined(W_OK) && \
       defined(R_OK)))
#define CONFIG_HAVE_access 1
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
      (defined(_waccess) || defined(___waccess_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
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

#ifdef CONFIG_NO_rmdir
#undef CONFIG_HAVE_rmdir
#elif !defined(CONFIG_HAVE_rmdir) && \
      (defined(rmdir) || defined(__rmdir_defined) || defined(CONFIG_HAVE_UNISTD_H))
#define CONFIG_HAVE_rmdir 1
#endif

#ifdef CONFIG_NO__rmdir
#undef CONFIG_HAVE__rmdir
#elif !defined(CONFIG_HAVE__rmdir) && \
      (defined(_rmdir) || defined(___rmdir_defined) || defined(CONFIG_HAVE_DIRECT_H))
#define CONFIG_HAVE__rmdir 1
#endif

#ifdef CONFIG_NO_unlinkat
#undef CONFIG_HAVE_unlinkat
#elif !defined(CONFIG_HAVE_unlinkat) && \
      (defined(unlinkat) || defined(__unlinkat_defined) || (defined(CONFIG_HAVE_UNISTD_H) && \
       defined(__USE_ATFILE)))
#define CONFIG_HAVE_unlinkat 1
#endif

#ifdef CONFIG_NO_remove
#undef CONFIG_HAVE_remove
#elif !defined(CONFIG_HAVE_remove) && \
      (defined(remove) || defined(__remove_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_remove 1
#endif

#ifdef CONFIG_NO_rename
#undef CONFIG_HAVE_rename
#elif !defined(CONFIG_HAVE_rename) && \
      (defined(rename) || defined(__rename_defined) || defined(CONFIG_HAVE_STDIO_H))
#define CONFIG_HAVE_rename 1
#endif

#ifdef CONFIG_NO_wunlink
#undef CONFIG_HAVE_wunlink
#elif !defined(CONFIG_HAVE_wunlink) && \
      (defined(wunlink) || defined(__wunlink_defined))
#define CONFIG_HAVE_wunlink 1
#endif

#ifdef CONFIG_NO__wunlink
#undef CONFIG_HAVE__wunlink
#elif !defined(CONFIG_HAVE__wunlink) && \
      (defined(_wunlink) || defined(___wunlink_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
#define CONFIG_HAVE__wunlink 1
#endif

#ifdef CONFIG_NO_wrmdir
#undef CONFIG_HAVE_wrmdir
#elif !defined(CONFIG_HAVE_wrmdir) && \
      (defined(wrmdir) || defined(__wrmdir_defined))
#define CONFIG_HAVE_wrmdir 1
#endif

#ifdef CONFIG_NO__wrmdir
#undef CONFIG_HAVE__wrmdir
#elif !defined(CONFIG_HAVE__wrmdir) && \
      (defined(_wrmdir) || defined(___wrmdir_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
#define CONFIG_HAVE__wrmdir 1
#endif

#ifdef CONFIG_NO_wremove
#undef CONFIG_HAVE_wremove
#elif !defined(CONFIG_HAVE_wremove) && \
      (defined(wremove) || defined(__wremove_defined))
#define CONFIG_HAVE_wremove 1
#endif

#ifdef CONFIG_NO__wremove
#undef CONFIG_HAVE__wremove
#elif !defined(CONFIG_HAVE__wremove) && \
      (defined(_wremove) || defined(___wremove_defined) || defined(_WSTDIO_DEFINED))
#define CONFIG_HAVE__wremove 1
#endif

#ifdef CONFIG_NO_wrename
#undef CONFIG_HAVE_wrename
#elif !defined(CONFIG_HAVE_wrename) && \
      (defined(wrename) || defined(__wrename_defined))
#define CONFIG_HAVE_wrename 1
#endif

#ifdef CONFIG_NO__wrename
#undef CONFIG_HAVE__wrename
#elif !defined(CONFIG_HAVE__wrename) && \
      (defined(_wrename) || defined(___wrename_defined) || (defined(_WIO_DEFINED) || \
       defined(_MSC_VER)))
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

#ifdef CONFIG_NO_pselect
#undef CONFIG_HAVE_pselect
#elif !defined(CONFIG_HAVE_pselect) && \
      (defined(pselect) || defined(__pselect_defined) || (defined(CONFIG_HAVE_SYS_SELECT_H) && \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_pselect 1
#endif

#ifdef CONFIG_NO___iob_func
#undef CONFIG_HAVE___iob_func
#elif !defined(CONFIG_HAVE___iob_func) && \
      (defined(__iob_func) || defined(____iob_func_defined) || (defined(_MSC_VER) && \
       !defined(_ACRTIMP_ALT)))
#define CONFIG_HAVE___iob_func 1
#endif

#ifdef CONFIG_NO___acrt_iob_func
#undef CONFIG_HAVE___acrt_iob_func
#elif !defined(CONFIG_HAVE___acrt_iob_func) && \
      (defined(__acrt_iob_func) || defined(____acrt_iob_func_defined) || (defined(_MSC_VER) && \
       defined(_ACRTIMP_ALT)))
#define CONFIG_HAVE___acrt_iob_func 1
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

#ifdef CONFIG_NO__fseek64
#undef CONFIG_HAVE__fseek64
#elif !defined(CONFIG_HAVE__fseek64) && \
      (defined(_fseek64) || defined(___fseek64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       0))
#define CONFIG_HAVE__fseek64 1
#endif

#ifdef CONFIG_NO__ftell64
#undef CONFIG_HAVE__ftell64
#elif !defined(CONFIG_HAVE__ftell64) && \
      (defined(_ftell64) || defined(___ftell64_defined) || (defined(CONFIG_HAVE_STDIO_H) && \
       0))
#define CONFIG_HAVE__ftell64 1
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

#ifdef CONFIG_NO__IONBF
#undef CONFIG_HAVE__IONBF
#elif !defined(CONFIG_HAVE__IONBF) && \
      (defined(_IONBF) || defined(___IONBF_defined))
#define CONFIG_HAVE__IONBF 1
#endif

#ifdef CONFIG_NO__IOFBF
#undef CONFIG_HAVE__IOFBF
#elif !defined(CONFIG_HAVE__IOFBF) && \
      (defined(_IOFBF) || defined(___IOFBF_defined))
#define CONFIG_HAVE__IOFBF 1
#endif

#ifdef CONFIG_NO__IOLBF
#undef CONFIG_HAVE__IOLBF
#elif !defined(CONFIG_HAVE__IOLBF) && \
      (defined(_IOLBF) || defined(___IOLBF_defined))
#define CONFIG_HAVE__IOLBF 1
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

#ifdef CONFIG_NO_strerror_s
#undef CONFIG_HAVE_strerror_s
#elif !defined(CONFIG_HAVE_strerror_s) && \
      (defined(strerror_s) || defined(__strerror_s_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_strerror_s 1
#endif

#ifdef CONFIG_NO_strerrorname_s
#undef CONFIG_HAVE_strerrorname_s
#elif !defined(CONFIG_HAVE_strerrorname_s) && \
      (defined(strerrorname_s) || defined(__strerrorname_s_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_strerrorname_s 1
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

#ifdef CONFIG_NO_dlmodulename
#undef CONFIG_HAVE_dlmodulename
#elif !defined(CONFIG_HAVE_dlmodulename) && \
      (defined(dlmodulename) || defined(__dlmodulename_defined) || (defined(CONFIG_HAVE_DLFCN_H) && \
       defined(__USE_KOS)))
#define CONFIG_HAVE_dlmodulename 1
#endif

#ifdef CONFIG_NO_RTLD_GLOBAL
#undef CONFIG_HAVE_RTLD_GLOBAL
#elif !defined(CONFIG_HAVE_RTLD_GLOBAL) && \
      (defined(RTLD_GLOBAL) || defined(__RTLD_GLOBAL_defined))
#define CONFIG_HAVE_RTLD_GLOBAL 1
#endif

#ifdef CONFIG_NO_RTLD_LOCAL
#undef CONFIG_HAVE_RTLD_LOCAL
#elif !defined(CONFIG_HAVE_RTLD_LOCAL) && \
      (defined(RTLD_LOCAL) || defined(__RTLD_LOCAL_defined))
#define CONFIG_HAVE_RTLD_LOCAL 1
#endif

#ifdef CONFIG_NO_RTLD_LAZY
#undef CONFIG_HAVE_RTLD_LAZY
#elif !defined(CONFIG_HAVE_RTLD_LAZY) && \
      (defined(RTLD_LAZY) || defined(__RTLD_LAZY_defined))
#define CONFIG_HAVE_RTLD_LAZY 1
#endif

#ifdef CONFIG_NO_RTLD_NOW
#undef CONFIG_HAVE_RTLD_NOW
#elif !defined(CONFIG_HAVE_RTLD_NOW) && \
      (defined(RTLD_NOW) || defined(__RTLD_NOW_defined))
#define CONFIG_HAVE_RTLD_NOW 1
#endif

#ifdef CONFIG_NO__memicmp
#undef CONFIG_HAVE__memicmp
#elif !defined(CONFIG_HAVE__memicmp) && \
      (defined(_memicmp) || defined(___memicmp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__memicmp 1
#endif

#ifdef CONFIG_NO_memicmp
#undef CONFIG_HAVE_memicmp
#elif !defined(CONFIG_HAVE_memicmp) && \
      (defined(memicmp) || defined(__memicmp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_memicmp 1
#endif

#ifdef CONFIG_NO_memcasecmp
#undef CONFIG_HAVE_memcasecmp
#elif !defined(CONFIG_HAVE_memcasecmp) && \
      (defined(memcasecmp) || defined(__memcasecmp_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcasecmp 1
#endif

#ifdef CONFIG_NO__stricmp
#undef CONFIG_HAVE__stricmp
#elif !defined(CONFIG_HAVE__stricmp) && \
      (defined(_stricmp) || defined(___stricmp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__stricmp 1
#endif

#ifdef CONFIG_NO__strcmpi
#undef CONFIG_HAVE__strcmpi
#elif !defined(CONFIG_HAVE__strcmpi) && \
      (defined(_strcmpi) || defined(___strcmpi_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__strcmpi 1
#endif

#ifdef CONFIG_NO_stricmp
#undef CONFIG_HAVE_stricmp
#elif !defined(CONFIG_HAVE_stricmp) && \
      (defined(stricmp) || defined(__stricmp_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_stricmp 1
#endif

#ifdef CONFIG_NO_strcmpi
#undef CONFIG_HAVE_strcmpi
#elif !defined(CONFIG_HAVE_strcmpi) && \
      (defined(strcmpi) || defined(__strcmpi_defined) || defined(_MSC_VER))
#define CONFIG_HAVE_strcmpi 1
#endif

#ifdef CONFIG_NO_strcasecmp
#undef CONFIG_HAVE_strcasecmp
#elif !defined(CONFIG_HAVE_strcasecmp) && \
      (defined(strcasecmp) || defined(__strcasecmp_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_strcasecmp 1
#endif

#ifdef CONFIG_NO_memchr
#undef CONFIG_HAVE_memchr
#else
#define CONFIG_HAVE_memchr 1
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

#ifdef CONFIG_NO_atoi
#undef CONFIG_HAVE_atoi
#else
#define CONFIG_HAVE_atoi 1
#endif

#ifdef CONFIG_NO_strlen
#undef CONFIG_HAVE_strlen
#else
#define CONFIG_HAVE_strlen 1
#endif

#ifdef CONFIG_NO_strchr
#undef CONFIG_HAVE_strchr
#else
#define CONFIG_HAVE_strchr 1
#endif

#ifdef CONFIG_NO_strrchr
#undef CONFIG_HAVE_strrchr
#else
#define CONFIG_HAVE_strrchr 1
#endif

#ifdef CONFIG_NO_strnlen
#undef CONFIG_HAVE_strnlen
#elif !defined(CONFIG_HAVE_strnlen) && \
      (defined(strnlen) || defined(__strnlen_defined) || (defined(__USE_XOPEN2K8) || \
       defined(__USE_DOS) || (defined(_MSC_VER) && !defined(__KOS_SYSTEM_HEADERS__))))
#define CONFIG_HAVE_strnlen 1
#endif

#ifdef CONFIG_NO_bzero
#undef CONFIG_HAVE_bzero
#elif !defined(CONFIG_HAVE_bzero) && \
      (defined(bzero) || defined(__bzero_defined) || defined(CONFIG_HAVE_STRINGS_H))
#define CONFIG_HAVE_bzero 1
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

#ifdef CONFIG_NO_memcasemem
#undef CONFIG_HAVE_memcasemem
#elif !defined(CONFIG_HAVE_memcasemem) && \
      (defined(__USE_KOS) && defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL))
#define CONFIG_HAVE_memcasemem 1
#endif

#ifdef CONFIG_NO_memcasermem
#undef CONFIG_HAVE_memcasermem
#elif !defined(CONFIG_HAVE_memcasermem) && \
      (defined(__memcasermem_defined) && defined(__USE_MEMMEM_EMPTY_NEEDLE_NULL))
#define CONFIG_HAVE_memcasermem 1
#endif

#ifdef CONFIG_NO_memmemw
#undef CONFIG_HAVE_memmemw
#elif 0
#define CONFIG_HAVE_memmemw 1
#endif

#ifdef CONFIG_NO_memmeml
#undef CONFIG_HAVE_memmeml
#elif 0
#define CONFIG_HAVE_memmeml 1
#endif

#ifdef CONFIG_NO_memmemq
#undef CONFIG_HAVE_memmemq
#elif 0
#define CONFIG_HAVE_memmemq 1
#endif

#ifdef CONFIG_NO_memrmemw
#undef CONFIG_HAVE_memrmemw
#elif 0
#define CONFIG_HAVE_memrmemw 1
#endif

#ifdef CONFIG_NO_memrmeml
#undef CONFIG_HAVE_memrmeml
#elif 0
#define CONFIG_HAVE_memrmeml 1
#endif

#ifdef CONFIG_NO_memrmemq
#undef CONFIG_HAVE_memrmemq
#elif 0
#define CONFIG_HAVE_memrmemq 1
#endif

#ifdef CONFIG_NO_memcpy
#undef CONFIG_HAVE_memcpy
#else
#define CONFIG_HAVE_memcpy 1
#endif

#ifdef CONFIG_NO_memset
#undef CONFIG_HAVE_memset
#else
#define CONFIG_HAVE_memset 1
#endif

#ifdef CONFIG_NO_memmove
#undef CONFIG_HAVE_memmove
#else
#define CONFIG_HAVE_memmove 1
#endif

#ifdef CONFIG_NO_memcmp
#undef CONFIG_HAVE_memcmp
#else
#define CONFIG_HAVE_memcmp 1
#endif

#ifdef CONFIG_NO_memcpyc
#undef CONFIG_HAVE_memcpyc
#elif !defined(CONFIG_HAVE_memcpyc) && \
      (defined(memcpyc) || defined(__memcpyc_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcpyc 1
#endif

#ifdef CONFIG_NO_memcpyw
#undef CONFIG_HAVE_memcpyw
#elif !defined(CONFIG_HAVE_memcpyw) && \
      (defined(memcpyw) || defined(__memcpyw_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcpyw 1
#endif

#ifdef CONFIG_NO_memcpyl
#undef CONFIG_HAVE_memcpyl
#elif !defined(CONFIG_HAVE_memcpyl) && \
      (defined(memcpyl) || defined(__memcpyl_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcpyl 1
#endif

#ifdef CONFIG_NO_memcpyq
#undef CONFIG_HAVE_memcpyq
#elif !defined(CONFIG_HAVE_memcpyq) && \
      (defined(memcpyq) || defined(__memcpyq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcpyq 1
#endif

#ifdef CONFIG_NO_memmovec
#undef CONFIG_HAVE_memmovec
#elif !defined(CONFIG_HAVE_memmovec) && \
      (defined(memmovec) || defined(__memmovec_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovec 1
#endif

#ifdef CONFIG_NO_memmovew
#undef CONFIG_HAVE_memmovew
#elif !defined(CONFIG_HAVE_memmovew) && \
      (defined(memmovew) || defined(__memmovew_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovew 1
#endif

#ifdef CONFIG_NO_memmovel
#undef CONFIG_HAVE_memmovel
#elif !defined(CONFIG_HAVE_memmovel) && \
      (defined(memmovel) || defined(__memmovel_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovel 1
#endif

#ifdef CONFIG_NO_memmoveq
#undef CONFIG_HAVE_memmoveq
#elif !defined(CONFIG_HAVE_memmoveq) && \
      (defined(memmoveq) || defined(__memmoveq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmoveq 1
#endif

#ifdef CONFIG_NO_memmoveup
#undef CONFIG_HAVE_memmoveup
#elif !defined(CONFIG_HAVE_memmoveup) && \
      (defined(memmoveup) || defined(__memmoveup_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmoveup 1
#endif

#ifdef CONFIG_NO_memmoveupc
#undef CONFIG_HAVE_memmoveupc
#elif !defined(CONFIG_HAVE_memmoveupc) && \
      (defined(memmoveupc) || defined(__memmoveupc_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmoveupc 1
#endif

#ifdef CONFIG_NO_memmoveupw
#undef CONFIG_HAVE_memmoveupw
#elif !defined(CONFIG_HAVE_memmoveupw) && \
      (defined(memmoveupw) || defined(__memmoveupw_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmoveupw 1
#endif

#ifdef CONFIG_NO_memmoveupl
#undef CONFIG_HAVE_memmoveupl
#elif !defined(CONFIG_HAVE_memmoveupl) && \
      (defined(memmoveupl) || defined(__memmoveupl_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmoveupl 1
#endif

#ifdef CONFIG_NO_memmoveupq
#undef CONFIG_HAVE_memmoveupq
#elif !defined(CONFIG_HAVE_memmoveupq) && \
      (defined(memmoveupq) || defined(__memmoveupq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmoveupq 1
#endif

#ifdef CONFIG_NO_memmovedown
#undef CONFIG_HAVE_memmovedown
#elif !defined(CONFIG_HAVE_memmovedown) && \
      (defined(memmovedown) || defined(__memmovedown_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovedown 1
#endif

#ifdef CONFIG_NO_memmovedownc
#undef CONFIG_HAVE_memmovedownc
#elif !defined(CONFIG_HAVE_memmovedownc) && \
      (defined(memmovedownc) || defined(__memmovedownc_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovedownc 1
#endif

#ifdef CONFIG_NO_memmovedownw
#undef CONFIG_HAVE_memmovedownw
#elif !defined(CONFIG_HAVE_memmovedownw) && \
      (defined(memmovedownw) || defined(__memmovedownw_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovedownw 1
#endif

#ifdef CONFIG_NO_memmovedownl
#undef CONFIG_HAVE_memmovedownl
#elif !defined(CONFIG_HAVE_memmovedownl) && \
      (defined(memmovedownl) || defined(__memmovedownl_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovedownl 1
#endif

#ifdef CONFIG_NO_memmovedownq
#undef CONFIG_HAVE_memmovedownq
#elif !defined(CONFIG_HAVE_memmovedownq) && \
      (defined(memmovedownq) || defined(__memmovedownq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memmovedownq 1
#endif

#ifdef CONFIG_NO_memsetw
#undef CONFIG_HAVE_memsetw
#elif !defined(CONFIG_HAVE_memsetw) && \
      (defined(memsetw) || defined(__memsetw_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memsetw 1
#endif

#ifdef CONFIG_NO_memsetl
#undef CONFIG_HAVE_memsetl
#elif !defined(CONFIG_HAVE_memsetl) && \
      (defined(memsetl) || defined(__memsetl_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memsetl 1
#endif

#ifdef CONFIG_NO_memsetq
#undef CONFIG_HAVE_memsetq
#elif !defined(CONFIG_HAVE_memsetq) && \
      (defined(memsetq) || defined(__memsetq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memsetq 1
#endif

#ifdef CONFIG_NO_memchrw
#undef CONFIG_HAVE_memchrw
#elif !defined(CONFIG_HAVE_memchrw) && \
      (defined(memchrw) || defined(__memchrw_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memchrw 1
#endif

#ifdef CONFIG_NO_memchrl
#undef CONFIG_HAVE_memchrl
#elif !defined(CONFIG_HAVE_memchrl) && \
      (defined(memchrl) || defined(__memchrl_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memchrl 1
#endif

#ifdef CONFIG_NO_memchrq
#undef CONFIG_HAVE_memchrq
#elif !defined(CONFIG_HAVE_memchrq) && \
      (defined(memchrq) || defined(__memchrq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memchrq 1
#endif

#ifdef CONFIG_NO_memrchrw
#undef CONFIG_HAVE_memrchrw
#elif !defined(CONFIG_HAVE_memrchrw) && \
      (defined(memrchrw) || defined(__memrchrw_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrchrw 1
#endif

#ifdef CONFIG_NO_memrchrl
#undef CONFIG_HAVE_memrchrl
#elif !defined(CONFIG_HAVE_memrchrl) && \
      (defined(memrchrl) || defined(__memrchrl_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrchrl 1
#endif

#ifdef CONFIG_NO_memrchrq
#undef CONFIG_HAVE_memrchrq
#elif !defined(CONFIG_HAVE_memrchrq) && \
      (defined(memrchrq) || defined(__memrchrq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrchrq 1
#endif

#ifdef CONFIG_NO_memcmpw
#undef CONFIG_HAVE_memcmpw
#elif !defined(CONFIG_HAVE_memcmpw) && \
      (defined(memcmpw) || defined(__memcmpw_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcmpw 1
#endif

#ifdef CONFIG_NO_memcmpl
#undef CONFIG_HAVE_memcmpl
#elif !defined(CONFIG_HAVE_memcmpl) && \
      (defined(memcmpl) || defined(__memcmpl_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcmpl 1
#endif

#ifdef CONFIG_NO_memcmpq
#undef CONFIG_HAVE_memcmpq
#elif !defined(CONFIG_HAVE_memcmpq) && \
      (defined(memcmpq) || defined(__memcmpq_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memcmpq 1
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

#ifdef CONFIG_NO_memrev
#undef CONFIG_HAVE_memrev
#elif !defined(CONFIG_HAVE_memrev) && \
      (defined(memrev) || defined(__memrev_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrev 1
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

#ifdef CONFIG_NO_islower
#undef CONFIG_HAVE_islower
#elif !defined(CONFIG_HAVE_islower) && \
      (defined(islower) || defined(__islower_defined) || defined(CONFIG_HAVE_CTYPE_H))
#define CONFIG_HAVE_islower 1
#endif

#ifdef CONFIG_NO_isupper
#undef CONFIG_HAVE_isupper
#elif !defined(CONFIG_HAVE_isupper) && \
      (defined(isupper) || defined(__isupper_defined) || defined(CONFIG_HAVE_CTYPE_H))
#define CONFIG_HAVE_isupper 1
#endif

#ifdef CONFIG_NO_isdigit
#undef CONFIG_HAVE_isdigit
#elif !defined(CONFIG_HAVE_isdigit) && \
      (defined(isdigit) || defined(__isdigit_defined) || defined(CONFIG_HAVE_CTYPE_H))
#define CONFIG_HAVE_isdigit 1
#endif

#ifdef CONFIG_NO_isalpha
#undef CONFIG_HAVE_isalpha
#elif !defined(CONFIG_HAVE_isalpha) && \
      (defined(isalpha) || defined(__isalpha_defined) || defined(CONFIG_HAVE_CTYPE_H))
#define CONFIG_HAVE_isalpha 1
#endif

#ifdef CONFIG_NO_isalnum
#undef CONFIG_HAVE_isalnum
#elif !defined(CONFIG_HAVE_isalnum) && \
      (defined(isalnum) || defined(__isalnum_defined) || defined(CONFIG_HAVE_CTYPE_H))
#define CONFIG_HAVE_isalnum 1
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

#ifdef CONFIG_NO_errno_kos2nt
#undef CONFIG_HAVE_errno_kos2nt
#elif !defined(CONFIG_HAVE_errno_kos2nt) && \
      (defined(__CRT_HAVE_errno_kos2nt))
#define CONFIG_HAVE_errno_kos2nt 1
#endif

#ifdef CONFIG_NO__get_osfhandle
#undef CONFIG_HAVE__get_osfhandle
#elif !defined(CONFIG_HAVE__get_osfhandle) && \
      (defined(_get_osfhandle) || defined(___get_osfhandle_defined) || (defined(_MSC_VER) || \
       defined(__CYGWIN__) || defined(__CYGWIN32__)))
#define CONFIG_HAVE__get_osfhandle 1
#endif

#ifdef CONFIG_NO_get_osfhandle
#undef CONFIG_HAVE_get_osfhandle
#elif !defined(CONFIG_HAVE_get_osfhandle) && \
      (defined(get_osfhandle) || defined(__get_osfhandle_defined) || (defined(__CYGWIN__) || \
       defined(__CYGWIN32__)))
#define CONFIG_HAVE_get_osfhandle 1
#endif

#ifdef CONFIG_NO_open_osfhandle
#undef CONFIG_HAVE_open_osfhandle
#elif !defined(CONFIG_HAVE_open_osfhandle) && \
      (defined(open_osfhandle) || defined(__open_osfhandle_defined))
#define CONFIG_HAVE_open_osfhandle 1
#endif

#ifdef CONFIG_NO__open_osfhandle
#undef CONFIG_HAVE__open_osfhandle
#elif !defined(CONFIG_HAVE__open_osfhandle) && \
      (defined(_open_osfhandle) || defined(___open_osfhandle_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__open_osfhandle 1
#endif

#ifdef CONFIG_NO_errno
#undef CONFIG_HAVE_errno
#elif !defined(CONFIG_HAVE_errno) && \
      (defined(errno) || defined(__errno_defined) || defined(CONFIG_HAVE_ERRNO_H))
#define CONFIG_HAVE_errno 1
#endif

#ifdef CONFIG_NO__errno
#undef CONFIG_HAVE__errno
#elif !defined(CONFIG_HAVE__errno) && \
      (defined(_errno) || defined(___errno_defined))
#define CONFIG_HAVE__errno 1
#endif

#ifdef CONFIG_NO___errno
#undef CONFIG_HAVE___errno
#elif !defined(CONFIG_HAVE___errno) && \
      (defined(__errno) || defined(____errno_defined))
#define CONFIG_HAVE___errno 1
#endif

#ifdef CONFIG_NO__doserrno
#undef CONFIG_HAVE__doserrno
#elif !defined(CONFIG_HAVE__doserrno) && \
      (defined(_doserrno) || defined(___doserrno_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__doserrno 1
#endif

#ifdef CONFIG_NO_doserrno
#undef CONFIG_HAVE_doserrno
#elif !defined(CONFIG_HAVE_doserrno) && \
      (defined(doserrno) || defined(__doserrno_defined))
#define CONFIG_HAVE_doserrno 1
#endif

#ifdef CONFIG_NO_environ
#undef CONFIG_HAVE_environ
#elif !defined(CONFIG_HAVE_environ) && \
      (defined(environ) || defined(__environ_defined))
#define CONFIG_HAVE_environ 1
#endif

#ifdef CONFIG_NO__environ
#undef CONFIG_HAVE__environ
#elif !defined(CONFIG_HAVE__environ) && \
      (defined(_environ) || defined(___environ_defined))
#define CONFIG_HAVE__environ 1
#endif

#ifdef CONFIG_NO___environ
#undef CONFIG_HAVE___environ
#elif !defined(CONFIG_HAVE___environ) && \
      (defined(__environ) || defined(____environ_defined))
#define CONFIG_HAVE___environ 1
#endif

#ifdef CONFIG_NO_sysconf
#undef CONFIG_HAVE_sysconf
#elif !defined(CONFIG_HAVE_sysconf) && \
      (defined(sysconf) || defined(__sysconf_defined) || (defined(CONFIG_HAVE_UNISTD_H) || \
       (defined(__linux__) || defined(__linux) || defined(linux) || defined(__unix__) || \
       defined(__unix) || defined(unix))))
#define CONFIG_HAVE_sysconf 1
#endif

#ifdef CONFIG_NO__sysconf
#undef CONFIG_HAVE__sysconf
#elif !defined(CONFIG_HAVE__sysconf) && \
      (defined(_sysconf) || defined(___sysconf_defined))
#define CONFIG_HAVE__sysconf 1
#endif

#ifdef CONFIG_NO_mpctl
#undef CONFIG_HAVE_mpctl
#elif !defined(CONFIG_HAVE_mpctl) && \
      (defined(mpctl) || defined(__mpctl_defined) || defined(__hpux__))
#define CONFIG_HAVE_mpctl 1
#endif

#ifdef CONFIG_NO_sysctl
#undef CONFIG_HAVE_sysctl
#elif !defined(CONFIG_HAVE_sysctl) && \
      (defined(sysctl) || defined(__sysctl_defined))
#define CONFIG_HAVE_sysctl 1
#endif

#ifdef CONFIG_NO__SC_NPROCESSORS_ONLN
#undef CONFIG_HAVE__SC_NPROCESSORS_ONLN
#elif !defined(CONFIG_HAVE__SC_NPROCESSORS_ONLN) && \
      (defined(_SC_NPROCESSORS_ONLN) || defined(___SC_NPROCESSORS_ONLN_defined))
#define CONFIG_HAVE__SC_NPROCESSORS_ONLN 1
#endif

#ifdef CONFIG_NO__SC_NPROC_ONLN
#undef CONFIG_HAVE__SC_NPROC_ONLN
#elif !defined(CONFIG_HAVE__SC_NPROC_ONLN) && \
      (defined(_SC_NPROC_ONLN) || defined(___SC_NPROC_ONLN_defined))
#define CONFIG_HAVE__SC_NPROC_ONLN 1
#endif

#ifdef CONFIG_NO_MPC_GETNUMSPUS
#undef CONFIG_HAVE_MPC_GETNUMSPUS
#elif !defined(CONFIG_HAVE_MPC_GETNUMSPUS) && \
      (defined(MPC_GETNUMSPUS) || defined(__MPC_GETNUMSPUS_defined) || defined(__hpux__))
#define CONFIG_HAVE_MPC_GETNUMSPUS 1
#endif

#ifdef CONFIG_NO_CTL_HW
#undef CONFIG_HAVE_CTL_HW
#elif !defined(CONFIG_HAVE_CTL_HW) && \
      (defined(CTL_HW) || defined(__CTL_HW_defined))
#define CONFIG_HAVE_CTL_HW 1
#endif

#ifdef CONFIG_NO_HW_AVAILCPU
#undef CONFIG_HAVE_HW_AVAILCPU
#elif !defined(CONFIG_HAVE_HW_AVAILCPU) && \
      (defined(HW_AVAILCPU) || defined(__HW_AVAILCPU_defined))
#define CONFIG_HAVE_HW_AVAILCPU 1
#endif

#ifdef CONFIG_NO_HW_NCPU
#undef CONFIG_HAVE_HW_NCPU
#elif !defined(CONFIG_HAVE_HW_NCPU) && \
      (defined(HW_NCPU) || defined(__HW_NCPU_defined))
#define CONFIG_HAVE_HW_NCPU 1
#endif

#ifdef CONFIG_NO_alloca
#undef CONFIG_HAVE_alloca
#elif !defined(CONFIG_HAVE_alloca) && \
      (defined(alloca) || defined(__alloca_defined) || defined(CONFIG_HAVE_ALLOCA_H))
#define CONFIG_HAVE_alloca 1
#endif

#ifdef CONFIG_NO__alloca
#undef CONFIG_HAVE__alloca
#elif !defined(CONFIG_HAVE__alloca) && \
      (defined(_alloca) || defined(___alloca_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__alloca 1
#endif

#ifdef CONFIG_NO_fabs
#undef CONFIG_HAVE_fabs
#elif !defined(CONFIG_HAVE_fabs) && \
      (defined(fabs) || defined(__fabs_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_fabs 1
#endif

#ifdef CONFIG_NO_sin
#undef CONFIG_HAVE_sin
#elif !defined(CONFIG_HAVE_sin) && \
      (defined(sin) || defined(__sin_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_sin 1
#endif

#ifdef CONFIG_NO_cos
#undef CONFIG_HAVE_cos
#elif !defined(CONFIG_HAVE_cos) && \
      (defined(cos) || defined(__cos_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_cos 1
#endif

#ifdef CONFIG_NO_tan
#undef CONFIG_HAVE_tan
#elif !defined(CONFIG_HAVE_tan) && \
      (defined(tan) || defined(__tan_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_tan 1
#endif

#ifdef CONFIG_NO_asin
#undef CONFIG_HAVE_asin
#elif !defined(CONFIG_HAVE_asin) && \
      (defined(asin) || defined(__asin_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_asin 1
#endif

#ifdef CONFIG_NO_acos
#undef CONFIG_HAVE_acos
#elif !defined(CONFIG_HAVE_acos) && \
      (defined(acos) || defined(__acos_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_acos 1
#endif

#ifdef CONFIG_NO_atan
#undef CONFIG_HAVE_atan
#elif !defined(CONFIG_HAVE_atan) && \
      (defined(atan) || defined(__atan_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_atan 1
#endif

#ifdef CONFIG_NO_sinh
#undef CONFIG_HAVE_sinh
#elif !defined(CONFIG_HAVE_sinh) && \
      (defined(sinh) || defined(__sinh_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_sinh 1
#endif

#ifdef CONFIG_NO_cosh
#undef CONFIG_HAVE_cosh
#elif !defined(CONFIG_HAVE_cosh) && \
      (defined(cosh) || defined(__cosh_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_cosh 1
#endif

#ifdef CONFIG_NO_tanh
#undef CONFIG_HAVE_tanh
#elif !defined(CONFIG_HAVE_tanh) && \
      (defined(tanh) || defined(__tanh_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_tanh 1
#endif

#ifdef CONFIG_NO_asinh
#undef CONFIG_HAVE_asinh
#elif !defined(CONFIG_HAVE_asinh) && \
      (defined(asinh) || defined(__asinh_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_asinh 1
#endif

#ifdef CONFIG_NO_acosh
#undef CONFIG_HAVE_acosh
#elif !defined(CONFIG_HAVE_acosh) && \
      (defined(acosh) || defined(__acosh_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_acosh 1
#endif

#ifdef CONFIG_NO_atanh
#undef CONFIG_HAVE_atanh
#elif !defined(CONFIG_HAVE_atanh) && \
      (defined(atanh) || defined(__atanh_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_atanh 1
#endif

#ifdef CONFIG_NO_copysign
#undef CONFIG_HAVE_copysign
#elif !defined(CONFIG_HAVE_copysign) && \
      (defined(copysign) || defined(__copysign_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_copysign 1
#endif

#ifdef CONFIG_NO_atan2
#undef CONFIG_HAVE_atan2
#elif !defined(CONFIG_HAVE_atan2) && \
      (defined(atan2) || defined(__atan2_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_atan2 1
#endif

#ifdef CONFIG_NO_exp
#undef CONFIG_HAVE_exp
#elif !defined(CONFIG_HAVE_exp) && \
      (defined(exp) || defined(__exp_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_exp 1
#endif

#ifdef CONFIG_NO_exp2
#undef CONFIG_HAVE_exp2
#elif !defined(CONFIG_HAVE_exp2) && \
      (defined(exp2) || defined(__exp2_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_exp2 1
#endif

#ifdef CONFIG_NO_expm1
#undef CONFIG_HAVE_expm1
#elif !defined(CONFIG_HAVE_expm1) && \
      (defined(expm1) || defined(__expm1_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_expm1 1
#endif

#ifdef CONFIG_NO_erf
#undef CONFIG_HAVE_erf
#elif !defined(CONFIG_HAVE_erf) && \
      (defined(erf) || defined(__erf_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_erf 1
#endif

#ifdef CONFIG_NO_erfc
#undef CONFIG_HAVE_erfc
#elif !defined(CONFIG_HAVE_erfc) && \
      (defined(erfc) || defined(__erfc_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_erfc 1
#endif

#ifdef CONFIG_NO_fabs
#undef CONFIG_HAVE_fabs
#elif !defined(CONFIG_HAVE_fabs) && \
      (defined(fabs) || defined(__fabs_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_fabs 1
#endif

#ifdef CONFIG_NO_sqrt
#undef CONFIG_HAVE_sqrt
#elif !defined(CONFIG_HAVE_sqrt) && \
      (defined(sqrt) || defined(__sqrt_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_sqrt 1
#endif

#ifdef CONFIG_NO_log
#undef CONFIG_HAVE_log
#elif !defined(CONFIG_HAVE_log) && \
      (defined(log) || defined(__log_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_log 1
#endif

#ifdef CONFIG_NO_log2
#undef CONFIG_HAVE_log2
#elif !defined(CONFIG_HAVE_log2) && \
      (defined(log2) || defined(__log2_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_log2 1
#endif

#ifdef CONFIG_NO_logb
#undef CONFIG_HAVE_logb
#elif !defined(CONFIG_HAVE_logb) && \
      (defined(logb) || defined(__logb_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_logb 1
#endif

#ifdef CONFIG_NO_log1p
#undef CONFIG_HAVE_log1p
#elif !defined(CONFIG_HAVE_log1p) && \
      (defined(log1p) || defined(__log1p_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_log1p 1
#endif

#ifdef CONFIG_NO_log10
#undef CONFIG_HAVE_log10
#elif !defined(CONFIG_HAVE_log10) && \
      (defined(log10) || defined(__log10_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_log10 1
#endif

#ifdef CONFIG_NO_cbrt
#undef CONFIG_HAVE_cbrt
#elif !defined(CONFIG_HAVE_cbrt) && \
      (defined(cbrt) || defined(__cbrt_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_cbrt 1
#endif

#ifdef CONFIG_NO_tgamma
#undef CONFIG_HAVE_tgamma
#elif !defined(CONFIG_HAVE_tgamma) && \
      (defined(tgamma) || defined(__tgamma_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_tgamma 1
#endif

#ifdef CONFIG_NO_lgamma
#undef CONFIG_HAVE_lgamma
#elif !defined(CONFIG_HAVE_lgamma) && \
      (defined(lgamma) || defined(__lgamma_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_lgamma 1
#endif

#ifdef CONFIG_NO_pow
#undef CONFIG_HAVE_pow
#elif !defined(CONFIG_HAVE_pow) && \
      (defined(pow) || defined(__pow_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_pow 1
#endif

#ifdef CONFIG_NO_ceil
#undef CONFIG_HAVE_ceil
#elif !defined(CONFIG_HAVE_ceil) && \
      (defined(ceil) || defined(__ceil_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_ceil 1
#endif

#ifdef CONFIG_NO_floor
#undef CONFIG_HAVE_floor
#elif !defined(CONFIG_HAVE_floor) && \
      (defined(floor) || defined(__floor_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_floor 1
#endif

#ifdef CONFIG_NO_round
#undef CONFIG_HAVE_round
#elif !defined(CONFIG_HAVE_round) && \
      (defined(round) || defined(__round_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_round 1
#endif

#ifdef CONFIG_NO_fmod
#undef CONFIG_HAVE_fmod
#elif !defined(CONFIG_HAVE_fmod) && \
      (defined(fmod) || defined(__fmod_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_fmod 1
#endif

#ifdef CONFIG_NO_hypot
#undef CONFIG_HAVE_hypot
#elif !defined(CONFIG_HAVE_hypot) && \
      (defined(hypot) || defined(__hypot_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_hypot 1
#endif

#ifdef CONFIG_NO_remainder
#undef CONFIG_HAVE_remainder
#elif !defined(CONFIG_HAVE_remainder) && \
      (defined(remainder) || defined(__remainder_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_remainder 1
#endif

#ifdef CONFIG_NO_nextafter
#undef CONFIG_HAVE_nextafter
#elif !defined(CONFIG_HAVE_nextafter) && \
      (defined(nextafter) || defined(__nextafter_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_nextafter 1
#endif

#ifdef CONFIG_NO_fdim
#undef CONFIG_HAVE_fdim
#elif !defined(CONFIG_HAVE_fdim) && \
      (defined(fdim) || defined(__fdim_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_fdim 1
#endif

#ifdef CONFIG_NO_isnan
#undef CONFIG_HAVE_isnan
#elif !defined(CONFIG_HAVE_isnan) && \
      (defined(isnan) || defined(__isnan_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isnan 1
#endif

#ifdef CONFIG_NO_isinf
#undef CONFIG_HAVE_isinf
#elif !defined(CONFIG_HAVE_isinf) && \
      (defined(isinf) || defined(__isinf_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isinf 1
#endif

#ifdef CONFIG_NO_isfinite
#undef CONFIG_HAVE_isfinite
#elif !defined(CONFIG_HAVE_isfinite) && \
      (defined(isfinite) || defined(__isfinite_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isfinite 1
#endif

#ifdef CONFIG_NO_isnormal
#undef CONFIG_HAVE_isnormal
#elif !defined(CONFIG_HAVE_isnormal) && \
      (defined(isnormal) || defined(__isnormal_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isnormal 1
#endif

#ifdef CONFIG_NO_signbit
#undef CONFIG_HAVE_signbit
#elif !defined(CONFIG_HAVE_signbit) && \
      (defined(signbit) || defined(__signbit_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_signbit 1
#endif

#ifdef CONFIG_NO_isgreater
#undef CONFIG_HAVE_isgreater
#elif !defined(CONFIG_HAVE_isgreater) && \
      (defined(isgreater) || defined(__isgreater_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isgreater 1
#endif

#ifdef CONFIG_NO_isgreaterequal
#undef CONFIG_HAVE_isgreaterequal
#elif !defined(CONFIG_HAVE_isgreaterequal) && \
      (defined(isgreaterequal) || defined(__isgreaterequal_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isgreaterequal 1
#endif

#ifdef CONFIG_NO_isless
#undef CONFIG_HAVE_isless
#elif !defined(CONFIG_HAVE_isless) && \
      (defined(isless) || defined(__isless_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isless 1
#endif

#ifdef CONFIG_NO_islessequal
#undef CONFIG_HAVE_islessequal
#elif !defined(CONFIG_HAVE_islessequal) && \
      (defined(islessequal) || defined(__islessequal_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_islessequal 1
#endif

#ifdef CONFIG_NO_islessgreater
#undef CONFIG_HAVE_islessgreater
#elif !defined(CONFIG_HAVE_islessgreater) && \
      (defined(islessgreater) || defined(__islessgreater_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_islessgreater 1
#endif

#ifdef CONFIG_NO_isunordered
#undef CONFIG_HAVE_isunordered
#elif !defined(CONFIG_HAVE_isunordered) && \
      (defined(isunordered) || defined(__isunordered_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_isunordered 1
#endif

#ifdef CONFIG_NO_ilogb
#undef CONFIG_HAVE_ilogb
#elif !defined(CONFIG_HAVE_ilogb) && \
      (defined(ilogb) || defined(__ilogb_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_ilogb 1
#endif

#ifdef CONFIG_NO_frexp
#undef CONFIG_HAVE_frexp
#elif !defined(CONFIG_HAVE_frexp) && \
      (defined(frexp) || defined(__frexp_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_frexp 1
#endif

#ifdef CONFIG_NO_modf
#undef CONFIG_HAVE_modf
#elif !defined(CONFIG_HAVE_modf) && \
      (defined(modf) || defined(__modf_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_modf 1
#endif

#ifdef CONFIG_NO_ldexp
#undef CONFIG_HAVE_ldexp
#elif !defined(CONFIG_HAVE_ldexp) && \
      (defined(ldexp) || defined(__ldexp_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_ldexp 1
#endif

#ifdef CONFIG_NO_sincos
#undef CONFIG_HAVE_sincos
#elif !defined(CONFIG_HAVE_sincos) && \
      (defined(sincos) || defined(__sincos_defined) || (defined(CONFIG_HAVE_MATH_H) && \
       defined(__USE_GNU)))
#define CONFIG_HAVE_sincos 1
#endif

#ifdef CONFIG_NO_asincos
#undef CONFIG_HAVE_asincos
#elif !defined(CONFIG_HAVE_asincos) && \
      (defined(asincos) || defined(__asincos_defined))
#define CONFIG_HAVE_asincos 1
#endif

#ifdef CONFIG_NO_sincosh
#undef CONFIG_HAVE_sincosh
#elif !defined(CONFIG_HAVE_sincosh) && \
      (defined(sincosh) || defined(__sincosh_defined))
#define CONFIG_HAVE_sincosh 1
#endif

#ifdef CONFIG_NO_asincosh
#undef CONFIG_HAVE_asincosh
#elif !defined(CONFIG_HAVE_asincosh) && \
      (defined(asincosh) || defined(__asincosh_defined))
#define CONFIG_HAVE_asincosh 1
#endif

#ifdef CONFIG_NO_scalbn
#undef CONFIG_HAVE_scalbn
#elif !defined(CONFIG_HAVE_scalbn) && \
      (defined(scalbn) || defined(__scalbn_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_scalbn 1
#endif

#ifdef CONFIG_NO_scalbln
#undef CONFIG_HAVE_scalbln
#elif !defined(CONFIG_HAVE_scalbln) && \
      (defined(scalbln) || defined(__scalbln_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_scalbln 1
#endif

#ifdef CONFIG_NO_remquo
#undef CONFIG_HAVE_remquo
#elif !defined(CONFIG_HAVE_remquo) && \
      (defined(remquo) || defined(__remquo_defined) || defined(CONFIG_HAVE_MATH_H))
#define CONFIG_HAVE_remquo 1
#endif

#ifdef CONFIG_NO_realpath
#undef CONFIG_HAVE_realpath
#elif !defined(CONFIG_HAVE_realpath) && \
      (defined(realpath) || defined(__realpath_defined))
#define CONFIG_HAVE_realpath 1
#endif

#ifdef CONFIG_NO_frealpath
#undef CONFIG_HAVE_frealpath
#elif !defined(CONFIG_HAVE_frealpath) && \
      (defined(frealpath) || defined(__frealpath_defined))
#define CONFIG_HAVE_frealpath 1
#endif

#ifdef CONFIG_NO_frealpath4
#undef CONFIG_HAVE_frealpath4
#elif !defined(CONFIG_HAVE_frealpath4) && \
      (defined(frealpath4) || defined(__frealpath4_defined))
#define CONFIG_HAVE_frealpath4 1
#endif

#ifdef CONFIG_NO_frealpathat
#undef CONFIG_HAVE_frealpathat
#elif !defined(CONFIG_HAVE_frealpathat) && \
      (defined(frealpathat) || defined(__frealpathat_defined))
#define CONFIG_HAVE_frealpathat 1
#endif

#ifdef CONFIG_NO_resolvepath
#undef CONFIG_HAVE_resolvepath
#elif !defined(CONFIG_HAVE_resolvepath) && \
      (defined(resolvepath) || defined(__resolvepath_defined))
#define CONFIG_HAVE_resolvepath 1
#endif

#ifdef CONFIG_NO_DT_UNKNOWN
#undef CONFIG_HAVE_DT_UNKNOWN
#elif !defined(CONFIG_HAVE_DT_UNKNOWN) && \
      (defined(DT_UNKNOWN) || defined(__DT_UNKNOWN_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_UNKNOWN 1
#endif

#ifdef CONFIG_NO_DT_FIFO
#undef CONFIG_HAVE_DT_FIFO
#elif !defined(CONFIG_HAVE_DT_FIFO) && \
      (defined(DT_FIFO) || defined(__DT_FIFO_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_FIFO 1
#endif

#ifdef CONFIG_NO_DT_CHR
#undef CONFIG_HAVE_DT_CHR
#elif !defined(CONFIG_HAVE_DT_CHR) && \
      (defined(DT_CHR) || defined(__DT_CHR_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_CHR 1
#endif

#ifdef CONFIG_NO_DT_DIR
#undef CONFIG_HAVE_DT_DIR
#elif !defined(CONFIG_HAVE_DT_DIR) && \
      (defined(DT_DIR) || defined(__DT_DIR_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_DIR 1
#endif

#ifdef CONFIG_NO_DT_BLK
#undef CONFIG_HAVE_DT_BLK
#elif !defined(CONFIG_HAVE_DT_BLK) && \
      (defined(DT_BLK) || defined(__DT_BLK_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_BLK 1
#endif

#ifdef CONFIG_NO_DT_REG
#undef CONFIG_HAVE_DT_REG
#elif !defined(CONFIG_HAVE_DT_REG) && \
      (defined(DT_REG) || defined(__DT_REG_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_REG 1
#endif

#ifdef CONFIG_NO_DT_LNK
#undef CONFIG_HAVE_DT_LNK
#elif !defined(CONFIG_HAVE_DT_LNK) && \
      (defined(DT_LNK) || defined(__DT_LNK_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_LNK 1
#endif

#ifdef CONFIG_NO_DT_SOCK
#undef CONFIG_HAVE_DT_SOCK
#elif !defined(CONFIG_HAVE_DT_SOCK) && \
      (defined(DT_SOCK) || defined(__DT_SOCK_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_SOCK 1
#endif

#ifdef CONFIG_NO_DT_WHT
#undef CONFIG_HAVE_DT_WHT
#elif !defined(CONFIG_HAVE_DT_WHT) && \
      (defined(DT_WHT) || defined(__DT_WHT_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DT_WHT 1
#endif

#ifdef CONFIG_NO_IFTODT
#undef CONFIG_HAVE_IFTODT
#elif !defined(CONFIG_HAVE_IFTODT) && \
      (defined(IFTODT) || defined(__IFTODT_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_IFTODT 1
#endif

#ifdef CONFIG_NO_DTTOIF
#undef CONFIG_HAVE_DTTOIF
#elif !defined(CONFIG_HAVE_DTTOIF) && \
      (defined(DTTOIF) || defined(__DTTOIF_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_DTTOIF 1
#endif

#ifdef CONFIG_NO_opendir
#undef CONFIG_HAVE_opendir
#elif !defined(CONFIG_HAVE_opendir) && \
      (defined(opendir) || defined(__opendir_defined) || defined(CONFIG_HAVE_DIRENT_H))
#define CONFIG_HAVE_opendir 1
#endif

#ifdef CONFIG_NO_fdopendir
#undef CONFIG_HAVE_fdopendir
#elif !defined(CONFIG_HAVE_fdopendir) && \
      (defined(fdopendir) || defined(__fdopendir_defined))
#define CONFIG_HAVE_fdopendir 1
#endif

#ifdef CONFIG_NO_closedir
#undef CONFIG_HAVE_closedir
#elif !defined(CONFIG_HAVE_closedir) && \
      (defined(closedir) || defined(__closedir_defined))
#define CONFIG_HAVE_closedir 1
#endif

#ifdef CONFIG_NO_readdir
#undef CONFIG_HAVE_readdir
#elif !defined(CONFIG_HAVE_readdir) && \
      (defined(readdir) || defined(__readdir_defined))
#define CONFIG_HAVE_readdir 1
#endif

#ifdef CONFIG_NO_readdir64
#undef CONFIG_HAVE_readdir64
#elif !defined(CONFIG_HAVE_readdir64) && \
      (defined(readdir64) || defined(__readdir64_defined))
#define CONFIG_HAVE_readdir64 1
#endif

#ifdef CONFIG_NO_dirfd
#undef CONFIG_HAVE_dirfd
#elif !defined(CONFIG_HAVE_dirfd) && \
      (defined(dirfd) || defined(__dirfd_defined))
#define CONFIG_HAVE_dirfd 1
#endif

#ifdef CONFIG_NO_DIRENT_D_INO
#undef CONFIG_HAVE_DIRENT_D_INO
#elif !defined(CONFIG_HAVE_DIRENT_D_INO) && \
      (defined(CONFIG_HAVE_readdir) && (defined(__linux__) || defined(__KOS__) || \
       defined(_DIRENT_HAVE_D_INO)))
#define CONFIG_HAVE_DIRENT_D_INO 1
#endif

#ifdef CONFIG_NO_DIRENT_D_FILENO
#undef CONFIG_HAVE_DIRENT_D_FILENO
#elif !defined(CONFIG_HAVE_DIRENT_D_FILENO) && \
      (defined(CONFIG_HAVE_readdir) && (defined(d_fileno) || defined(_DIRENT_HAVE_D_FILENO)))
#define CONFIG_HAVE_DIRENT_D_FILENO 1
#endif

#ifdef CONFIG_NO_DIRENT_D_OFF
#undef CONFIG_HAVE_DIRENT_D_OFF
#elif !defined(CONFIG_HAVE_DIRENT_D_OFF) && \
      (defined(CONFIG_HAVE_readdir) && (defined(d_off) || defined(_DIRENT_HAVE_D_OFF)))
#define CONFIG_HAVE_DIRENT_D_OFF 1
#endif

#ifdef CONFIG_NO_DIRENT_D_NAMLEN
#undef CONFIG_HAVE_DIRENT_D_NAMLEN
#elif !defined(CONFIG_HAVE_DIRENT_D_NAMLEN) && \
      (defined(CONFIG_HAVE_readdir) && (defined(d_namlen) || defined(_DIRENT_HAVE_D_NAMLEN)))
#define CONFIG_HAVE_DIRENT_D_NAMLEN 1
#endif

#ifdef CONFIG_NO_DIRENT_D_RECLEN
#undef CONFIG_HAVE_DIRENT_D_RECLEN
#elif !defined(CONFIG_HAVE_DIRENT_D_RECLEN) && \
      (defined(CONFIG_HAVE_readdir) && (defined(d_reclen) || defined(_DIRENT_HAVE_D_RECLEN)))
#define CONFIG_HAVE_DIRENT_D_RECLEN 1
#endif

#ifdef CONFIG_NO_DIRENT_D_TYPE
#undef CONFIG_HAVE_DIRENT_D_TYPE
#elif !defined(CONFIG_HAVE_DIRENT_D_TYPE) && \
      (defined(CONFIG_HAVE_readdir) && (defined(d_type) || defined(_DIRENT_HAVE_D_TYPE)))
#define CONFIG_HAVE_DIRENT_D_TYPE 1
#endif

#ifdef CONFIG_NO_DIRENT_D_TYPE_SZ_1
#undef CONFIG_HAVE_DIRENT_D_TYPE_SZ_1
#elif 0
#define CONFIG_HAVE_DIRENT_D_TYPE_SZ_1 1
#endif

#ifdef CONFIG_NO_DIRENT_D_TYPE_SZ_2
#undef CONFIG_HAVE_DIRENT_D_TYPE_SZ_2
#elif 0
#define CONFIG_HAVE_DIRENT_D_TYPE_SZ_2 1
#endif

#ifdef CONFIG_NO_DIRENT_D_TYPE_SZ_4
#undef CONFIG_HAVE_DIRENT_D_TYPE_SZ_4
#elif 0
#define CONFIG_HAVE_DIRENT_D_TYPE_SZ_4 1
#endif
//[[[end]]]

#if (!defined(CONFIG_HAVE_DIRENT_D_TYPE_SZ_1) && \
     !defined(CONFIG_HAVE_DIRENT_D_TYPE_SZ_2) && \
     !defined(CONFIG_HAVE_DIRENT_D_TYPE_SZ_4))
#define CONFIG_HAVE_DIRENT_D_TYPE_SZ_1
#endif /* !CONFIG_HAVE_DIRENT_D_TYPE_SZ_... */


/* Figure out of the host has a /proc filesystem.
 * NOTE: This configure option simply controls a couple of
 *       optional features and only impacts things that may
 *       be attempted at runtime to implement various functions.
 *       One such function is dex:posix.cpu_count(), which tries
 *       to make use of `/proc/cpuinfo' when this option is enabled.
 * Anything that uses this feature still functions properly, even
 * when a /proc filesystem is actually available at runtime.
 * Furthermore, certain components may simply assume that a /proc
 * filesystem is available at runtime, irregardless of this configure
 * option (such as the unix implementation of dex:ipc) */
#ifdef CONFIG_NO_PROCFS
#undef CONFIG_HAVE_PROCFS
#elif !defined(CONFIG_HAVE_PROCFS) && defined(CONFIG_HOST_UNIX)
#define CONFIG_HAVE_PROCFS 1
#endif



/* Figure out if we want to prefer wchar_t-functions over char-functions */
#ifdef CONFIG_PREFER_WCHAR_FUNCTIONS
#undef CONFIG_PREFER_CHAR_FUNCTIONS
#elif !defined(CONFIG_PREFER_CHAR_FUNCTIONS)
#ifdef CONFIG_HOST_WINDOWS
#define CONFIG_PREFER_WCHAR_FUNCTIONS 1
#else /* CONFIG_HOST_WINDOWS */
#define CONFIG_PREFER_CHAR_FUNCTIONS 1
#endif /* !CONFIG_HOST_WINDOWS */
#endif


#if 1
#define CONFIG_HAVE_fopen_binary 1 /* fopen() accepts the "b" mode flag */
#endif


/* Substitute some known function aliases */
#if defined(CONFIG_HAVE__exit) && !defined(CONFIG_HAVE__Exit)
#define CONFIG_HAVE__Exit 1
#undef _Exit
#define _Exit(exit_code) _exit(exit_code)
#endif /* _Exit = _exit */

#if defined(CONFIG_HAVE__execv) && !defined(CONFIG_HAVE_execv)
#define CONFIG_HAVE_execv 1
#undef execv
#define execv(path, argv) _execv(path, (char const *const *)(argv))
#endif /* execv = _execv */

#if defined(CONFIG_HAVE__execve) && !defined(CONFIG_HAVE_execve)
#define CONFIG_HAVE_execve 1
#undef execve
#define execve(path, argv, envp) _execve(path, (char const *const *)(argv), (char const *const *)(envp))
#endif /* execve = _execve */

#if defined(CONFIG_HAVE__execvp) && !defined(CONFIG_HAVE_execvp)
#define CONFIG_HAVE_execvp 1
#undef execvp
#define execvp(file, argv) _execvp(file, (char const *const *)(argv))
#endif /* execvp = _execvp */

#if defined(CONFIG_HAVE__execvpe) && !defined(CONFIG_HAVE_execvpe)
#define CONFIG_HAVE_execvpe 1
#undef execvpe
#define execvpe(file, argv, envp) _execvpe(file, (char const *const *)(argv), (char const *const *)(envp))
#endif /* execvpe = _execvpe */

#if defined(CONFIG_HAVE__wexecv) && !defined(CONFIG_HAVE_wexecv)
#define CONFIG_HAVE_wexecv 1
#undef wexecv
#define wexecv(path, argv) _wexecv(path, (wchar_t const *const *)(argv))
#endif /* wexecv = _wexecv */

#if defined(CONFIG_HAVE__wexecve) && !defined(CONFIG_HAVE_wexecve)
#define CONFIG_HAVE_wexecve 1
#undef wexecve
#define wexecve(path, argv, envp) _wexecve(path, (wchar_t const *const *)(argv), (wchar_t const *const *)(envp))
#endif /* wexecve = _wexecve */

#if defined(CONFIG_HAVE__wexecvp) && !defined(CONFIG_HAVE_wexecvp)
#define CONFIG_HAVE_wexecvp 1
#undef wexecvp
#define wexecvp(file, argv) _wexecvp(file, (wchar_t const *const *)(argv))
#endif /* wexecvp = _wexecvp */

#if defined(CONFIG_HAVE__wexecvpe) && !defined(CONFIG_HAVE_wexecvpe)
#define CONFIG_HAVE_wexecvpe 1
#undef wexecvpe
#define wexecvpe(file, argv, envp) _wexecvpe(file, (wchar_t const *const *)(argv), (wchar_t const *const *)(envp))
#endif /* wexecvpe = _wexecvpe */

#if defined(CONFIG_HAVE__spawnv) && !defined(CONFIG_HAVE_spawnv)
#define CONFIG_HAVE_spawnv 1
#undef spawnv
#define spawnv(mode, path, argv) _spawnv(mode, path, (char const *const *)(argv))
#endif /* spawnv = _spawnv */

#if defined(CONFIG_HAVE__spawnve) && !defined(CONFIG_HAVE_spawnve)
#define CONFIG_HAVE_spawnve 1
#undef spawnve
#define spawnve(mode, path, argv, envp) _spawnve(mode, path, (char const *const *)(argv), (char const *const *)(envp))
#endif /* spawnve = _spawnve */

#if defined(CONFIG_HAVE__spawnvp) && !defined(CONFIG_HAVE_spawnvp)
#define CONFIG_HAVE_spawnvp 1
#undef spawnvp
#define spawnvp(mode, file, argv) _spawnvp(mode, file, (char const *const *)(argv))
#endif /* spawnvp = _spawnvp */

#if defined(CONFIG_HAVE__spawnvpe) && !defined(CONFIG_HAVE_spawnvpe)
#define CONFIG_HAVE_spawnvpe 1
#undef spawnvpe
#define spawnvpe(mode, file, argv, envp) _spawnvpe(mode, file, (char const *const *)(argv), (char const *const *)(envp))
#endif /* spawnvpe = _spawnvpe */

#if defined(CONFIG_HAVE__wspawnv) && !defined(CONFIG_HAVE_wspawnv)
#define CONFIG_HAVE_wspawnv 1
#undef wspawnv
#define wspawnv(mode, path, argv) _wspawnv(mode, path, (wchar_t const *const *)(argv))
#endif /* wspawnv = _wspawnv */

#if defined(CONFIG_HAVE__wspawnve) && !defined(CONFIG_HAVE_wspawnve)
#define CONFIG_HAVE_wspawnve 1
#undef wspawnve
#define wspawnve(mode, path, argv, envp) _wspawnve(mode, path, (wchar_t const *const *)(argv), (wchar_t const *const *)(envp))
#endif /* wspawnve = _wspawnve */

#if defined(CONFIG_HAVE__wspawnvp) && !defined(CONFIG_HAVE_wspawnvp)
#define CONFIG_HAVE_wspawnvp 1
#undef wspawnvp
#define wspawnvp(mode, file, argv) _wspawnvp(mode, file, (wchar_t const *const *)(argv))
#endif /* wspawnvp = _wspawnvp */

#if defined(CONFIG_HAVE__wspawnvpe) && !defined(CONFIG_HAVE_wspawnvpe)
#define CONFIG_HAVE_wspawnvpe 1
#undef wspawnvpe
#define wspawnvpe(mode, file, argv, envp) _wspawnvpe(mode, file, (wchar_t const *const *)(argv), (wchar_t const *const *)(envp))
#endif /* wspawnvpe = _wspawnvpe */

#if defined(CONFIG_HAVE__wstat) && !defined(CONFIG_HAVE_wstat)
#define CONFIG_HAVE_wstat 1
#undef wstat
#define wstat _wstat
#endif /* wstat = _wstat */

#if defined(CONFIG_HAVE__wstat64) && !defined(CONFIG_HAVE_wstat64)
#define CONFIG_HAVE_wstat64 1
#undef wstat64
#define wstat64 _wstat64
#endif /* wstat64 = _wstat64 */

#if defined(CONFIG_HAVE__wlstat) && !defined(CONFIG_HAVE_wlstat)
#define CONFIG_HAVE_wlstat 1
#undef wlstat
#define wlstat _wlstat
#endif /* wlstat = _wlstat */

#if defined(CONFIG_HAVE__wlstat64) && !defined(CONFIG_HAVE_wlstat64)
#define CONFIG_HAVE_wlstat64 1
#undef wlstat64
#define wlstat64 _wlstat64
#endif /* wlstat64 = _wlstat64 */

#if defined(CONFIG_HAVE__cwait) && !defined(CONFIG_HAVE_cwait)
#define CONFIG_HAVE_cwait 1
#undef cwait
#define cwait _cwait
#endif /* cwait = _cwait */

#if defined(CONFIG_HAVE__wsystem) && !defined(CONFIG_HAVE_wsystem)
#define CONFIG_HAVE_wsystem 1
#undef wsystem
#define wsystem _wsystem
#endif /* wsystem = _wsystem */

#if defined(CONFIG_HAVE__open) && !defined(CONFIG_HAVE_open)
#define CONFIG_HAVE_open 1
#undef open
#define open _open
#endif /* open = _open */

#if defined(CONFIG_HAVE__creat) && !defined(CONFIG_HAVE_creat)
#define CONFIG_HAVE_creat 1
#undef creat
#define creat _creat
#endif /* creat = _creat */

#if defined(CONFIG_HAVE__chmod) && !defined(CONFIG_HAVE_chmod)
#define CONFIG_HAVE_chmod 1
#undef chmod
#define chmod _chmod
#endif /* chmod = _chmod */

#if defined(CONFIG_HAVE__unlink) && !defined(CONFIG_HAVE_unlink)
#define CONFIG_HAVE_unlink 1
#undef unlink
#define unlink _unlink
#endif /* unlink = _unlink */

#if defined(CONFIG_HAVE__wunlink) && !defined(CONFIG_HAVE_wunlink)
#define CONFIG_HAVE_wunlink 1
#undef wunlink
#define wunlink _wunlink
#endif /* wunlink = _wunlink */

#if defined(CONFIG_HAVE__rmdir) && !defined(CONFIG_HAVE_rmdir)
#define CONFIG_HAVE_rmdir 1
#undef rmdir
#define rmdir _rmdir
#endif /* rmdir = _rmdir */

#if defined(CONFIG_HAVE__wrmdir) && !defined(CONFIG_HAVE_wrmdir)
#define CONFIG_HAVE_wrmdir 1
#undef wrmdir
#define wrmdir _wrmdir
#endif /* wrmdir = _wrmdir */

#if defined(CONFIG_HAVE__remove) && !defined(CONFIG_HAVE_remove)
#define CONFIG_HAVE_remove 1
#undef remove
#define remove _remove
#endif /* remove = _remove */

#if defined(CONFIG_HAVE__wremove) && !defined(CONFIG_HAVE_wremove)
#define CONFIG_HAVE_wremove 1
#undef wremove
#define wremove _wremove
#endif /* wremove = _wremove */

#if defined(CONFIG_HAVE__rename) && !defined(CONFIG_HAVE_rename)
#define CONFIG_HAVE_rename 1
#undef rename
#define rename _rename
#endif /* rename = _rename */

#if defined(CONFIG_HAVE__wrename) && !defined(CONFIG_HAVE_wrename)
#define CONFIG_HAVE_wrename 1
#undef wrename
#define wrename _wrename
#endif /* wrename = _wrename */

#if defined(CONFIG_HAVE__wopen) && !defined(CONFIG_HAVE_wopen)
#define CONFIG_HAVE_wopen 1
#undef wopen
#define wopen _wopen
#endif /* wopen = _wopen */

#if defined(CONFIG_HAVE__read) && !defined(CONFIG_HAVE_read)
#define CONFIG_HAVE_read 1
#undef read
#define read(fd, buf, bufsize) ((Dee_ssize_t)_read(fd, buf, (unsigned int)(bufsize)))
#endif /* read = _read */

#if defined(CONFIG_HAVE__write) && !defined(CONFIG_HAVE_write)
#define CONFIG_HAVE_write 1
#undef write
#define write(fd, buf, bufsize) ((Dee_ssize_t)_write(fd, buf, (unsigned int)(bufsize)))
#endif /* write = _write */

#if defined(CONFIG_HAVE__lseek) && !defined(CONFIG_HAVE_lseek)
#define CONFIG_HAVE_lseek 1
#undef lseek
#define lseek _lseek
#endif /* lseek = _lseek */

#if defined(CONFIG_HAVE__lseeki64) && !defined(CONFIG_HAVE_lseek64)
#define CONFIG_HAVE_lseek64 1
#undef lseek64
#define lseek64 _lseeki64
#endif /* lseek64 = _lseeki64 */

#if defined(CONFIG_HAVE__lseek64) && !defined(CONFIG_HAVE_lseek64)
#define CONFIG_HAVE_lseek64 1
#undef lseek64
#define lseek64 _lseek64
#endif /* lseek64 = _lseek64 */

#if defined(CONFIG_HAVE__close) && !defined(CONFIG_HAVE_close)
#define CONFIG_HAVE_close 1
#undef close
#define close _close
#endif /* close = _close */

#if defined(CONFIG_HAVE__commit) && !defined(CONFIG_HAVE_fsync)
#define CONFIG_HAVE_fsync 1
#undef fsync
#define fsync _commit
#endif /* fsync = _commit */

#if defined(CONFIG_HAVE__chsize) && !defined(CONFIG_HAVE_ftruncate)
#define CONFIG_HAVE_ftruncate 1
#undef ftruncate
#define ftruncate _chsize
#endif /* ftruncate = _chsize */

#if defined(CONFIG_HAVE__chsize_s) && !defined(CONFIG_HAVE_ftruncate64)
#define CONFIG_HAVE_ftruncate64 1
#undef ftruncate64
#define ftruncate64 _chsize_s
#endif /* ftruncate64 = _chsize_s */

#if defined(CONFIG_HAVE__chdir) && !defined(CONFIG_HAVE_chdir)
#define CONFIG_HAVE_chdir 1
#undef chdir
#define chdir _chdir
#endif /* chdir = _chdir */

#if defined(CONFIG_HAVE__wchdir) && !defined(CONFIG_HAVE_wchdir)
#define CONFIG_HAVE_wchdir 1
#undef wchdir
#define wchdir _wchdir
#endif /* wchdir = _wchdir */

#if defined(CONFIG_HAVE__getpid) && !defined(CONFIG_HAVE_getpid)
#define CONFIG_HAVE_getpid 1
#undef getpid
#define getpid _getpid
#endif /* getpid = _getpid */

#if defined(CONFIG_HAVE__umask) && !defined(CONFIG_HAVE_umask)
#define CONFIG_HAVE_umask 1
#undef umask
#define umask _umask
#endif /* umask = _umask */

#if defined(CONFIG_HAVE__dup) && !defined(CONFIG_HAVE_dup)
#define CONFIG_HAVE_dup 1
#undef dup
#define dup _dup
#endif /* dup = _dup */

#if defined(CONFIG_HAVE__dup2) && !defined(CONFIG_HAVE_dup2)
#define CONFIG_HAVE_dup2 1
#undef dup2
#define dup2 _dup2
#endif /* dup2 = _dup2 */

#if defined(CONFIG_HAVE__isatty) && !defined(CONFIG_HAVE_isatty)
#define CONFIG_HAVE_isatty 1
#undef isatty
#define isatty _isatty
#endif /* isatty = _isatty */

#if defined(CONFIG_HAVE__getcwd) && !defined(CONFIG_HAVE_getcwd)
#define CONFIG_HAVE_getcwd 1
#undef getcwd
#define getcwd _getcwd
#endif /* getcwd = _getcwd */

#if defined(CONFIG_HAVE__wgetcwd) && !defined(CONFIG_HAVE_wgetcwd)
#define CONFIG_HAVE_wgetcwd 1
#undef wgetcwd
#define wgetcwd _wgetcwd
#endif /* wgetcwd = _wgetcwd */

#if defined(CONFIG_HAVE__access) && !defined(CONFIG_HAVE_access)
#define CONFIG_HAVE_access 1
#undef access
#define access _access
#endif /* access = _access */

#if defined(CONFIG_HAVE__waccess) && !defined(CONFIG_HAVE_waccess)
#define CONFIG_HAVE_waccess 1
#undef waccess
#define waccess _waccess
#endif /* waccess = _waccess */

#if defined(CONFIG_HAVE_euidaccess) && !defined(CONFIG_HAVE_eaccess)
#define CONFIG_HAVE_eaccess 1
#undef eaccess
#define eaccess euidaccess
#endif /* eaccess = euidaccess */

#if defined(CONFIG_HAVE_eaccess) && !defined(CONFIG_HAVE_euidaccess)
#define CONFIG_HAVE_euidaccess 1
#undef euidaccess
#define euidaccess eaccess
#endif /* euidaccess = eaccess */

#if defined(CONFIG_HAVE_fork) && !defined(CONFIG_HAVE_vfork)
#define CONFIG_HAVE_vfork 1
#undef vfork
#define vfork fork
#endif /* vfork = fork */

#if defined(CONFIG_HAVE__pipe) && !defined(CONFIG_HAVE_pipe)
#define CONFIG_HAVE_pipe 1
#undef pipe
#define pipe(fds) _pipe(fds, 4096, O_BINARY)
#endif /* pipe = _pipe */

#if defined(CONFIG_HAVE__mkdir) && !defined(CONFIG_HAVE_mkdir)
#define CONFIG_HAVE_mkdir 1
#undef mkdir
#define mkdir(name, mode) _mkdir(name)
#endif /* mkdir = _mkdir */

#if defined(CONFIG_HAVE__wmkdir) && !defined(CONFIG_HAVE_wmkdir)
#define CONFIG_HAVE_wmkdir 1
#undef wmkdir
#define wmkdir(name, mode) _wmkdir(name)
#endif /* mkdir = _mkdir */

#if defined(CONFIG_HAVE__fseeki64) && !defined(CONFIG_HAVE_fseeko64)
#define CONFIG_HAVE_fseeko64 1
#undef fseeko64
#define fseeko64 _fseeki64
#endif /* fseeko64 = _fseeki64 */

#if defined(CONFIG_HAVE__fseek64) && !defined(CONFIG_HAVE_fseeko64)
#define CONFIG_HAVE_fseeko64 1
#undef fseeko64
#define fseeko64 _fseek64
#endif /* fseeko64 = _fseek64 */

#if defined(CONFIG_HAVE_fseek64) && !defined(CONFIG_HAVE_fseeko64)
#define CONFIG_HAVE_fseeko64 1
#undef fseeko64
#define fseeko64 fseek64
#endif /* fseeko64 = fseek64 */

#if defined(CONFIG_HAVE__ftelli64) && !defined(CONFIG_HAVE_ftello64)
#define CONFIG_HAVE_ftello64 1
#undef ftello64
#define ftello64 _ftelli64
#endif /* ftello64 = _ftelli64 */

#if defined(CONFIG_HAVE__ftell64) && !defined(CONFIG_HAVE_ftello64)
#define CONFIG_HAVE_ftello64 1
#undef ftello64
#define ftello64 _ftell64
#endif /* ftello64 = _ftell64 */

#if defined(CONFIG_HAVE_ftell64) && !defined(CONFIG_HAVE_ftello64)
#define CONFIG_HAVE_ftello64 1
#undef ftello64
#define ftello64 ftell64
#endif /* ftello64 = ftell64 */

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
#undef fseeko
#undef ftello
#define fseeko fseek
#define ftello ftell
#endif /* fseeko = fseek */

#if defined(CONFIG_HAVE_fseeko64) && !defined(CONFIG_HAVE_fseeko)
#define CONFIG_HAVE_fseeko 1
#define CONFIG_HAVE_ftello 1
#undef fseeko
#undef ftello
#define fseeko fseeko64
#define ftello ftello64
#endif /* fseeko = fseeko64 */

#if !defined(CONFIG_HAVE_fseek) && defined(CONFIG_HAVE_fseeko)
#define CONFIG_HAVE_fseek 1
#define CONFIG_HAVE_ftell 1
#undef fseek
#undef ftell
#define fseek fseeko
#define ftell (long int)ftello
#endif /* fseeko = fseeko64 */

#if defined(CONFIG_HAVE__fileno) && !defined(CONFIG_HAVE_fileno)
#define CONFIG_HAVE_fileno 1
#undef fileno
#define fileno _fileno
#endif /* fileno = _fileno */

#if defined(CONFIG_HAVE_getc) && !defined(CONFIG_HAVE_fgetc)
#define CONFIG_HAVE_fgetc 1
#undef fgetc
#define fgetc getc
#endif /* fgetc = getc */

#if defined(CONFIG_HAVE_fgetc) && !defined(CONFIG_HAVE_getc)
#define CONFIG_HAVE_getc 1
#undef getc
#define getc fgetc
#endif /* getc = fgetc */

#if defined(CONFIG_HAVE_putc) && !defined(CONFIG_HAVE_fputc)
#define CONFIG_HAVE_fputc 1
#undef fputc
#define fputc putc
#endif /* fputc = putc */

#if defined(CONFIG_HAVE_fputc) && !defined(CONFIG_HAVE_putc)
#define CONFIG_HAVE_putc 1
#undef putc
#define putc fputc
#endif /* putc = fputc */

#if defined(CONFIG_HAVE_MAP_ANON) && !defined(CONFIG_HAVE_MAP_ANONYMOUS)
#define CONFIG_HAVE_MAP_ANONYMOUS 1
#undef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif /* MAP_ANONYMOUS = MAP_ANON */

#if defined(CONFIG_HAVE_MAP_ANONYMOUS) && !defined(CONFIG_HAVE_MAP_ANON)
#define CONFIG_HAVE_MAP_ANON 1
#undef MAP_ANON
#define MAP_ANON MAP_ANONYMOUS
#endif /* MAP_ANON = MAP_ANONYMOUS */

#ifndef CONFIG_HAVE_environ
#ifdef CONFIG_HAVE__environ
#define CONFIG_HAVE_environ 1
#undef environ
#define environ    _environ
#elif defined(CONFIG_HAVE___environ)
#define CONFIG_HAVE_environ 1
#undef environ
#define environ   __environ
#endif /* environ = __environ */
#endif /* !CONFIG_HAVE_environ*/

#if !defined(CONFIG_HAVE_execv) && defined(CONFIG_HAVE_execve) && defined(CONFIG_HAVE_environ)
#define CONFIG_HAVE_execv 1
#undef execv
#define execv(path, argv) execve(path, argv, environ)
#endif /* execv = execve */

#if !defined(CONFIG_HAVE_execvp) && defined(CONFIG_HAVE_execvpe) && defined(CONFIG_HAVE_environ)
#define CONFIG_HAVE_execpv 1
#undef execvp
#define execvp(path, argv) execvpe(path, argv, environ)
#endif /* execvp = execvpe */

#if !defined(CONFIG_HAVE_wexecv) && defined(CONFIG_HAVE_wexecve) && defined(CONFIG_HAVE_environ)
#define CONFIG_HAVE_wexecv 1
#undef wexecv
#define wexecv(path, argv) wexecve(path, argv, environ)
#endif /* wexecv = wexecve */

#if !defined(CONFIG_HAVE_wexecvp) && defined(CONFIG_HAVE_wexecvpe) && defined(CONFIG_HAVE_environ)
#define CONFIG_HAVE_wexecpv 1
#undef wexecvp
#define wexecvp(path, argv) wexecvpe(path, argv, environ)
#endif /* wexecvp = wexecvpe */

#if defined(CONFIG_HAVE__sysconf) && !defined(CONFIG_HAVE_sysconf)
#define CONFIG_HAVE_sysconf 1
#undef sysconf
#define sysconf _sysconf
#endif /* sysconf = _sysconf */

/* Try to substitute alloca() with alternatives */
#ifndef CONFIG_HAVE_alloca
#ifdef CONFIG_HAVE__alloca
#define CONFIG_HAVE_alloca 1
#define alloca _alloca
#elif __has_builtin(__builtin_alloca)
#define alloca __builtin_alloca
#elif __has_builtin(__builtin_alloca_with_align)
#include <hybrid/typecore.h>
#define alloca(s) __builtin_alloca_with_align(s, __SIZEOF_POINTER__)
#else /* ... */
#include <hybrid/__alloca.h>
#ifdef __hybrid_alloca
#define CONFIG_HAVE_alloca 1
#define alloca __hybrid_alloca
#endif /* __hybrid_alloca */
#endif /* !... */
#endif /* !CONFIG_HAVE_alloca */

#ifndef CONFIG_HAVE_strend
#define CONFIG_HAVE_strend 1
#define strend(s)  ((s) + strlen(s))
#endif /* !CONFIG_HAVE_strend */


/* Configure O_* flags for `open()' */

/* Set optional flags to no-ops */
#ifndef CONFIG_HAVE_O_BINARY
#ifdef CONFIG_HAVE___O_BINARY
#define CONFIG_HAVE_O_BINARY 1
#define O_BINARY __O_BINARY
#elif defined(CONFIG_HAVE__O_BINARY)
#define CONFIG_HAVE_O_BINARY 1
#define O_BINARY _O_BINARY
#elif defined(CONFIG_HAVE___O_RAW)
#define CONFIG_HAVE_O_BINARY 1
#define O_BINARY __O_RAW
#elif defined(CONFIG_HAVE__O_RAW)
#define CONFIG_HAVE_O_BINARY 1
#define O_BINARY _O_RAW
#elif defined(CONFIG_HAVE_O_RAW)
#define CONFIG_HAVE_O_BINARY 1
#define O_BINARY O_RAW
#else
#define O_BINARY 0
#endif
#elif !defined(O_BINARY)
#define O_BINARY O_BINARY
#endif /* !CONFIG_HAVE_O_BINARY */

#ifndef CONFIG_HAVE_O_SHORT_LIVED
#ifdef CONFIG_HAVE___O_SHORT_LIVED
#define CONFIG_HAVE_O_SHORT_LIVED 1
#define O_SHORT_LIVED __O_SHORT_LIVED
#elif defined(CONFIG_HAVE__O_SHORT_LIVED)
#define CONFIG_HAVE_O_SHORT_LIVED 1
#define O_SHORT_LIVED _O_SHORT_LIVED
#else
#define O_SHORT_LIVED 0
#endif
#elif !defined(O_SHORT_LIVED)
#define O_SHORT_LIVED O_SHORT_LIVED
#endif /* !CONFIG_HAVE_O_SHORT_LIVED */

#ifndef CONFIG_HAVE_O_SEQUENTIAL
#ifdef CONFIG_HAVE___O_SEQUENTIAL
#define CONFIG_HAVE_O_SEQUENTIAL 1
#define O_SEQUENTIAL __O_SEQUENTIAL
#elif defined(CONFIG_HAVE__O_SEQUENTIAL)
#define CONFIG_HAVE_O_SEQUENTIAL 1
#define O_SEQUENTIAL _O_SEQUENTIAL
#else
#define O_SEQUENTIAL 0
#endif
#elif !defined(O_SEQUENTIAL)
#define O_SEQUENTIAL O_SEQUENTIAL
#endif /* !CONFIG_HAVE_O_SEQUENTIAL */

#ifndef CONFIG_HAVE_O_RANDOM
#ifdef CONFIG_HAVE___O_RANDOM
#define CONFIG_HAVE_O_RANDOM 1
#define O_RANDOM __O_RANDOM
#elif defined(CONFIG_HAVE__O_RANDOM)
#define CONFIG_HAVE_O_RANDOM 1
#define O_RANDOM _O_RANDOM
#else
#define O_RANDOM 0
#endif
#elif !defined(O_RANDOM)
#define O_RANDOM O_RANDOM
#endif /* !CONFIG_HAVE_O_RANDOM */

#ifndef CONFIG_HAVE_O_PATH
#ifdef CONFIG_HAVE___O_PATH
#define CONFIG_HAVE_O_PATH 1
#define O_PATH __O_PATH
#elif defined(CONFIG_HAVE__O_PATH)
#define CONFIG_HAVE_O_PATH 1
#define O_PATH _O_PATH
#else
#define O_PATH 0
#endif
#elif !defined(O_PATH)
#define O_PATH O_PATH
#endif /* !CONFIG_HAVE_O_PATH */

#ifndef CONFIG_HAVE_O_NOATIME
#ifdef CONFIG_HAVE___O_NOATIME
#define CONFIG_HAVE_O_NOATIME 1
#define O_NOATIME __O_NOATIME
#elif defined(CONFIG_HAVE__O_NOATIME)
#define CONFIG_HAVE_O_NOATIME 1
#define O_NOATIME _O_NOATIME
#else
#define O_NOATIME 0
#endif
#elif !defined(O_NOATIME)
#define O_NOATIME O_NOATIME
#endif /* !CONFIG_HAVE_O_NOATIME */

#ifndef CONFIG_HAVE_O_NOCTTY
#ifdef CONFIG_HAVE___O_NOCTTY
#define CONFIG_HAVE_O_NOCTTY 1
#define O_NOCTTY __O_NOCTTY
#elif defined(CONFIG_HAVE__O_NOCTTY)
#define CONFIG_HAVE_O_NOCTTY 1
#define O_NOCTTY _O_NOCTTY
#else
#define O_NOCTTY 0
#endif
#elif !defined(O_NOCTTY)
#define O_NOCTTY O_NOCTTY
#endif /* !CONFIG_HAVE_O_NOCTTY */


#ifndef CONFIG_HAVE_O_TEXT
#ifdef CONFIG_HAVE___O_TEXT
#define CONFIG_HAVE_O_TEXT 1
#define O_TEXT __O_TEXT
#elif defined(CONFIG_HAVE__O_TEXT)
#define CONFIG_HAVE_O_TEXT 1
#define O_TEXT _O_TEXT
#endif
#elif !defined(O_TEXT)
#define O_TEXT O_TEXT
#endif /* !CONFIG_HAVE_O_TEXT */

#ifndef CONFIG_HAVE_O_WTEXT
#ifdef CONFIG_HAVE___O_WTEXT
#define CONFIG_HAVE_O_WTEXT 1
#define O_WTEXT __O_WTEXT
#elif defined(CONFIG_HAVE__O_WTEXT)
#define CONFIG_HAVE_O_WTEXT 1
#define O_WTEXT _O_WTEXT
#endif
#elif !defined(O_WTEXT)
#define O_WTEXT O_WTEXT
#endif /* !CONFIG_HAVE_O_WTEXT */

#ifndef CONFIG_HAVE_O_U16TEXT
#ifdef CONFIG_HAVE___O_U16TEXT
#define CONFIG_HAVE_O_U16TEXT 1
#define O_U16TEXT __O_U16TEXT
#elif defined(CONFIG_HAVE__O_U16TEXT)
#define CONFIG_HAVE_O_U16TEXT 1
#define O_U16TEXT _O_U16TEXT
#endif
#elif !defined(O_U16TEXT)
#define O_U16TEXT O_U16TEXT
#endif /* !CONFIG_HAVE_O_U16TEXT */

#ifndef CONFIG_HAVE_O_U8TEXT
#ifdef CONFIG_HAVE___O_U8TEXT
#define CONFIG_HAVE_O_U8TEXT 1
#define O_U8TEXT __O_U8TEXT
#elif defined(CONFIG_HAVE__O_U8TEXT)
#define CONFIG_HAVE_O_U8TEXT 1
#define O_U8TEXT _O_U8TEXT
#endif
#elif !defined(O_U8TEXT)
#define O_U8TEXT O_U8TEXT
#endif /* !CONFIG_HAVE_O_U8TEXT */

#ifndef CONFIG_HAVE_O_TEMPORARY
#ifdef CONFIG_HAVE___O_TEMPORARY
#define CONFIG_HAVE_O_TEMPORARY 1
#define O_TEMPORARY __O_TEMPORARY
#elif defined(CONFIG_HAVE__O_TEMPORARY)
#define CONFIG_HAVE_O_TEMPORARY 1
#define O_TEMPORARY _O_TEMPORARY
#endif
#elif !defined(O_TEMPORARY)
#define O_TEMPORARY O_TEMPORARY
#endif /* !CONFIG_HAVE_O_TEMPORARY */

#ifndef CONFIG_HAVE_O_OBTAIN_DIR
#ifdef CONFIG_HAVE___O_OBTAIN_DIR
#define CONFIG_HAVE_O_OBTAIN_DIR 1
#define O_OBTAIN_DIR __O_OBTAIN_DIR
#elif defined(CONFIG_HAVE__O_OBTAIN_DIR)
#define CONFIG_HAVE_O_OBTAIN_DIR 1
#define O_OBTAIN_DIR _O_OBTAIN_DIR
#endif
#elif !defined(O_OBTAIN_DIR)
#define O_OBTAIN_DIR O_OBTAIN_DIR
#endif /* !CONFIG_HAVE_O_OBTAIN_DIR */

#ifndef CONFIG_HAVE_O_CREAT
#ifdef CONFIG_HAVE___O_CREAT
#define CONFIG_HAVE_O_CREAT 1
#define O_CREAT __O_CREAT
#elif defined(CONFIG_HAVE__O_CREAT)
#define CONFIG_HAVE_O_CREAT 1
#define O_CREAT _O_CREAT
#endif
#elif !defined(O_CREAT)
#define O_CREAT O_CREAT
#endif /* !CONFIG_HAVE_O_CREAT */

#ifndef CONFIG_HAVE_O_TRUNC
#ifdef CONFIG_HAVE___O_TRUNC
#define CONFIG_HAVE_O_TRUNC 1
#define O_TRUNC __O_TRUNC
#elif defined(CONFIG_HAVE__O_TRUNC)
#define CONFIG_HAVE_O_TRUNC 1
#define O_TRUNC _O_TRUNC
#endif
#elif !defined(O_TRUNC)
#define O_TRUNC O_TRUNC
#endif /* !CONFIG_HAVE_O_TRUNC */

#ifndef CONFIG_HAVE_O_RDONLY
#ifdef CONFIG_HAVE___O_RDONLY
#define CONFIG_HAVE_O_RDONLY 1
#define O_RDONLY __O_RDONLY
#elif defined(CONFIG_HAVE__O_RDONLY)
#define CONFIG_HAVE_O_RDONLY 1
#define O_RDONLY _O_RDONLY
#endif
#elif !defined(O_RDONLY)
#define O_RDONLY O_RDONLY
#endif /* !CONFIG_HAVE_O_RDONLY */

#ifndef CONFIG_HAVE_O_WRONLY
#ifdef CONFIG_HAVE___O_WRONLY
#define CONFIG_HAVE_O_WRONLY 1
#define O_WRONLY __O_WRONLY
#elif defined(CONFIG_HAVE__O_WRONLY)
#define CONFIG_HAVE_O_WRONLY 1
#define O_WRONLY _O_WRONLY
#endif
#elif !defined(O_WRONLY)
#define O_WRONLY O_WRONLY
#endif /* !CONFIG_HAVE_O_WRONLY */

#ifndef CONFIG_HAVE_O_RDWR
#ifdef CONFIG_HAVE___O_RDWR
#define CONFIG_HAVE_O_RDWR 1
#define O_RDWR __O_RDWR
#elif defined(CONFIG_HAVE__O_RDWR)
#define CONFIG_HAVE_O_RDWR 1
#define O_RDWR _O_RDWR
#endif
#elif !defined(O_RDWR)
#define O_RDWR O_RDWR
#endif /* !CONFIG_HAVE_O_RDWR */

#ifndef CONFIG_HAVE_O_ACCMODE
#ifdef CONFIG_HAVE___O_ACCMODE
#define CONFIG_HAVE_O_ACCMODE 1
#define O_ACCMODE __O_ACCMODE
#elif defined(CONFIG_HAVE__O_ACCMODE)
#define CONFIG_HAVE_O_ACCMODE 1
#define O_ACCMODE _O_ACCMODE
#elif 0 /* The combination might overlap with something else... */
#define CONFIG_HAVE_O_ACCMODE 1
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif
#elif !defined(O_ACCMODE)
#define O_ACCMODE O_ACCMODE
#endif /* !CONFIG_HAVE_O_ACCMODE */

#ifndef CONFIG_HAVE_O_CLOEXEC
#ifdef CONFIG_HAVE___O_NOINHERIT
#define CONFIG_HAVE_O_CLOEXEC 1
#define O_CLOEXEC __O_NOINHERIT
#elif defined(CONFIG_HAVE__O_NOINHERIT)
#define CONFIG_HAVE_O_CLOEXEC 1
#define O_CLOEXEC _O_NOINHERIT
#elif defined(CONFIG_HAVE_O_NOINHERIT)
#define CONFIG_HAVE_O_CLOEXEC 1
#define O_CLOEXEC O_NOINHERIT
#elif defined(CONFIG_HAVE___O_CLOEXEC)
#define CONFIG_HAVE_O_CLOEXEC 1
#define O_CLOEXEC __O_CLOEXEC
#elif defined(CONFIG_HAVE__O_CLOEXEC)
#define CONFIG_HAVE_O_CLOEXEC 1
#define O_CLOEXEC _O_CLOEXEC
#endif
#elif !defined(O_CLOEXEC)
#define O_CLOEXEC O_CLOEXEC
#endif /* !CONFIG_HAVE_O_CLOEXEC */

#ifndef CONFIG_HAVE_O_EXCL
#ifdef CONFIG_HAVE___O_EXCL
#define CONFIG_HAVE_O_EXCL 1
#define O_EXCL __O_EXCL
#elif defined(CONFIG_HAVE__O_EXCL)
#define CONFIG_HAVE_O_EXCL 1
#define O_EXCL _O_EXCL
#endif
#elif !defined(O_EXCL)
#define O_EXCL O_EXCL
#endif /* !CONFIG_HAVE_O_EXCL */

#ifndef CONFIG_HAVE_O_APPEND
#ifdef CONFIG_HAVE___O_APPEND
#define CONFIG_HAVE_O_APPEND 1
#define O_APPEND __O_APPEND
#elif defined(CONFIG_HAVE__O_APPEND)
#define CONFIG_HAVE_O_APPEND 1
#define O_APPEND _O_APPEND
#endif
#elif !defined(O_APPEND)
#define O_APPEND O_APPEND
#endif /* !CONFIG_HAVE_O_APPEND */

#ifndef CONFIG_HAVE_O_NONBLOCK
#ifdef CONFIG_HAVE___O_NONBLOCK
#define CONFIG_HAVE_O_NONBLOCK 1
#define O_NONBLOCK __O_NONBLOCK
#elif defined(CONFIG_HAVE__O_NONBLOCK)
#define CONFIG_HAVE_O_NONBLOCK 1
#define O_NONBLOCK _O_NONBLOCK
#elif defined(CONFIG_HAVE___O_NDELAY)
#define CONFIG_HAVE_O_NONBLOCK 1
#define O_NONBLOCK __O_NDELAY
#elif defined(CONFIG_HAVE__O_NDELAY)
#define CONFIG_HAVE_O_NONBLOCK 1
#define O_NONBLOCK _O_NDELAY
#elif defined(CONFIG_HAVE_O_NDELAY)
#define CONFIG_HAVE_O_NONBLOCK 1
#define O_NONBLOCK O_NDELAY
#endif
#elif !defined(O_NONBLOCK)
#define O_NONBLOCK O_NONBLOCK
#endif /* !CONFIG_HAVE_O_NONBLOCK */

#ifndef CONFIG_HAVE_O_RSYNC
#ifdef CONFIG_HAVE___O_RSYNC
#define CONFIG_HAVE_O_RSYNC 1
#define O_RSYNC __O_RSYNC
#elif defined(CONFIG_HAVE__O_RSYNC)
#define CONFIG_HAVE_O_RSYNC 1
#define O_RSYNC _O_RSYNC
#endif
#elif !defined(O_RSYNC)
#define O_RSYNC O_RSYNC
#endif /* !CONFIG_HAVE_O_RSYNC */

#ifndef CONFIG_HAVE_O_SYNC
#ifdef CONFIG_HAVE___O_SYNC
#define CONFIG_HAVE_O_SYNC 1
#define O_SYNC __O_SYNC
#elif defined(CONFIG_HAVE__O_SYNC)
#define CONFIG_HAVE_O_SYNC 1
#define O_SYNC _O_SYNC
#endif
#elif !defined(O_SYNC)
#define O_SYNC O_SYNC
#endif /* !CONFIG_HAVE_O_SYNC */

#ifndef CONFIG_HAVE_O_DSYNC
#ifdef CONFIG_HAVE___O_DSYNC
#define CONFIG_HAVE_O_DSYNC 1
#define O_DSYNC __O_DSYNC
#elif defined(CONFIG_HAVE__O_DSYNC)
#define CONFIG_HAVE_O_DSYNC 1
#define O_DSYNC _O_DSYNC
#endif
#elif !defined(O_DSYNC)
#define O_DSYNC O_DSYNC
#endif /* !CONFIG_HAVE_O_DSYNC */

#ifndef CONFIG_HAVE_O_ASYNC
#ifdef CONFIG_HAVE___O_ASYNC
#define CONFIG_HAVE_O_ASYNC 1
#define O_ASYNC __O_ASYNC
#elif defined(CONFIG_HAVE__O_ASYNC)
#define CONFIG_HAVE_O_ASYNC 1
#define O_ASYNC _O_ASYNC
#endif
#elif !defined(O_ASYNC)
#define O_ASYNC O_ASYNC
#endif /* !CONFIG_HAVE_O_ASYNC */

#ifndef CONFIG_HAVE_O_DIRECT
#ifdef CONFIG_HAVE___O_DIRECT
#define CONFIG_HAVE_O_DIRECT 1
#define O_DIRECT __O_DIRECT
#elif defined(CONFIG_HAVE__O_DIRECT)
#define CONFIG_HAVE_O_DIRECT 1
#define O_DIRECT _O_DIRECT
#endif
#endif /* O_DIRECT */

#ifndef CONFIG_HAVE_O_LARGEFILE
#ifdef CONFIG_HAVE___O_LARGEFILE
#define CONFIG_HAVE_O_LARGEFILE 1
#define O_LARGEFILE __O_LARGEFILE
#elif defined(CONFIG_HAVE__O_LARGEFILE)
#define CONFIG_HAVE_O_LARGEFILE 1
#define O_LARGEFILE _O_LARGEFILE
#endif
#elif !defined(O_LARGEFILE)
#define O_LARGEFILE O_LARGEFILE
#endif /* !CONFIG_HAVE_O_LARGEFILE */

#ifndef CONFIG_HAVE_O_DIRECTORY
#ifdef CONFIG_HAVE___O_DIRECTORY
#define CONFIG_HAVE_O_DIRECTORY 1
#define O_DIRECTORY __O_DIRECTORY
#elif defined(CONFIG_HAVE__O_DIRECTORY)
#define CONFIG_HAVE_O_DIRECTORY 1
#define O_DIRECTORY _O_DIRECTORY
#endif
#elif !defined(O_DIRECTORY)
#define O_DIRECTORY O_DIRECTORY
#endif /* !CONFIG_HAVE_O_DIRECTORY */

#ifndef CONFIG_HAVE_O_NOFOLLOW
#ifdef CONFIG_HAVE___O_NOFOLLOW
#define CONFIG_HAVE_O_NOFOLLOW 1
#define O_NOFOLLOW __O_NOFOLLOW
#elif defined(CONFIG_HAVE__O_NOFOLLOW)
#define CONFIG_HAVE_O_NOFOLLOW 1
#define O_NOFOLLOW _O_NOFOLLOW
#endif
#elif !defined(O_NOFOLLOW)
#define O_NOFOLLOW O_NOFOLLOW
#endif /* !CONFIG_HAVE_O_NOFOLLOW */

#ifndef CONFIG_HAVE_O_TMPFILE
#ifdef CONFIG_HAVE___O_TMPFILE
#define CONFIG_HAVE_O_TMPFILE 1
#define O_TMPFILE __O_TMPFILE
#elif defined(CONFIG_HAVE__O_TMPFILE)
#define CONFIG_HAVE_O_TMPFILE 1
#define O_TMPFILE _O_TMPFILE
#endif
#elif !defined(O_TMPFILE)
#define O_TMPFILE O_TMPFILE
#endif /* !CONFIG_HAVE_O_TMPFILE */

#ifndef CONFIG_HAVE_O_CLOFORK
#ifdef CONFIG_HAVE___O_CLOFORK
#define CONFIG_HAVE_O_CLOFORK 1
#define O_CLOFORK __O_CLOFORK
#elif defined(CONFIG_HAVE__O_CLOFORK)
#define CONFIG_HAVE_O_CLOFORK 1
#define O_CLOFORK _O_CLOFORK
#endif
#elif !defined(O_CLOFORK)
#define O_CLOFORK O_CLOFORK
#endif /* !CONFIG_HAVE_O_CLOFORK */

#ifndef CONFIG_HAVE_O_SYMLINK
#ifdef CONFIG_HAVE___O_SYMLINK
#define CONFIG_HAVE_O_SYMLINK 1
#define O_SYMLINK __O_SYMLINK
#elif defined(CONFIG_HAVE__O_SYMLINK)
#define CONFIG_HAVE_O_SYMLINK 1
#define O_SYMLINK _O_SYMLINK
#endif
#elif !defined(O_SYMLINK)
#define O_SYMLINK O_SYMLINK
#endif /* !CONFIG_HAVE_O_SYMLINK */

#ifndef CONFIG_HAVE_O_DOSPATH
#ifdef CONFIG_HAVE___O_DOSPATH
#define CONFIG_HAVE_O_DOSPATH 1
#define O_DOSPATH __O_DOSPATH
#elif defined(CONFIG_HAVE__O_DOSPATH)
#define CONFIG_HAVE_O_DOSPATH 1
#define O_DOSPATH _O_DOSPATH
#elif defined(CONFIG_HAVE__MSC_VER)
#define O_DOSPATH 0
#endif
#elif !defined(O_DOSPATH)
#define O_DOSPATH O_DOSPATH
#endif /* !CONFIG_HAVE_O_DOSPATH */

#ifndef CONFIG_HAVE_O_SHLOCK
#ifdef CONFIG_HAVE___O_SHLOCK
#define CONFIG_HAVE_O_SHLOCK 1
#define O_SHLOCK __O_SHLOCK
#elif defined(CONFIG_HAVE__O_SHLOCK)
#define CONFIG_HAVE_O_SHLOCK 1
#define O_SHLOCK _O_SHLOCK
#endif
#elif !defined(O_SHLOCK)
#define O_SHLOCK O_SHLOCK
#endif /* !CONFIG_HAVE_O_SHLOCK */

#ifndef CONFIG_HAVE_O_EXLOCK
#ifdef CONFIG_HAVE___O_EXLOCK
#define CONFIG_HAVE_O_EXLOCK 1
#define O_EXLOCK __O_EXLOCK
#elif defined(CONFIG_HAVE__O_EXLOCK)
#define CONFIG_HAVE_O_EXLOCK 1
#define O_EXLOCK _O_EXLOCK
#endif
#elif !defined(O_EXLOCK)
#define O_EXLOCK O_EXLOCK
#endif /* !CONFIG_HAVE_O_EXLOCK */

#ifndef CONFIG_HAVE_O_XATTR
#ifdef CONFIG_HAVE___O_XATTR
#define CONFIG_HAVE_O_XATTR 1
#define O_XATTR __O_XATTR
#elif defined(CONFIG_HAVE__O_XATTR)
#define CONFIG_HAVE_O_XATTR 1
#define O_XATTR _O_XATTR
#endif
#elif !defined(O_XATTR)
#define O_XATTR O_XATTR
#endif /* !CONFIG_HAVE_O_XATTR */

#ifndef CONFIG_HAVE_O_EXEC
#ifdef CONFIG_HAVE___O_EXEC
#define CONFIG_HAVE_O_EXEC 1
#define O_EXEC __O_EXEC
#elif defined(CONFIG_HAVE__O_EXEC)
#define CONFIG_HAVE_O_EXEC 1
#define O_EXEC _O_EXEC
#endif
#elif !defined(O_EXEC)
#define O_EXEC O_EXEC
#endif /* !CONFIG_HAVE_O_EXEC */

#ifndef CONFIG_HAVE_O_SEARCH
#ifdef CONFIG_HAVE___O_SEARCH
#define CONFIG_HAVE_O_SEARCH 1
#define O_SEARCH __O_SEARCH
#elif defined(CONFIG_HAVE__O_SEARCH)
#define CONFIG_HAVE_O_SEARCH 1
#define O_SEARCH _O_SEARCH
#endif
#elif !defined(O_SEARCH)
#define O_SEARCH O_SEARCH
#endif /* !CONFIG_HAVE_O_SEARCH */

#ifndef CONFIG_HAVE_O_TTY_INIT
#ifdef CONFIG_HAVE___O_TTY_INIT
#define CONFIG_HAVE_O_TTY_INIT 1
#define O_TTY_INIT __O_TTY_INIT
#elif defined(CONFIG_HAVE__O_TTY_INIT)
#define CONFIG_HAVE_O_TTY_INIT 1
#define O_TTY_INIT _O_TTY_INIT
#endif
#elif !defined(O_TTY_INIT)
#define O_TTY_INIT O_TTY_INIT
#endif /* !CONFIG_HAVE_O_TTY_INIT */

#ifndef CONFIG_HAVE_O_NOLINKS
#ifdef CONFIG_HAVE___O_NOLINKS
#define CONFIG_HAVE_O_NOLINKS 1
#define O_NOLINKS __O_NOLINKS
#elif defined(CONFIG_HAVE__O_NOLINKS)
#define CONFIG_HAVE_O_NOLINKS 1
#define O_NOLINKS _O_NOLINKS
#endif
#elif !defined(O_NOLINKS)
#define O_NOLINKS O_NOLINKS
#endif /* !CONFIG_HAVE_O_NOLINKS */

#if defined(CONFIG_HAVE_open64) && !defined(CONFIG_HAVE_open)
#define CONFIG_HAVE_open 1
#undef open
#define open open64
#endif /* open = open64 */

#if defined(CONFIG_HAVE_creat64) && !defined(CONFIG_HAVE_creat)
#define CONFIG_HAVE_creat 1
#undef creat
#define creat creat64
#endif /* creat = creat64 */

#if defined(CONFIG_HAVE_open) && !defined(CONFIG_HAVE_open64) && defined(CONFIG_HAVE_O_LARGEFILE)
#define CONFIG_HAVE_open64 1
#undef open64
#define open64(filename, oflags, ...) open(filename, (oflags) | O_LARGEFILE, ##__VA_ARGS__)
#endif /* open64 = open */

#if defined(CONFIG_HAVE_open) && !defined(CONFIG_HAVE_creat) && \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_creat 1
#undef creat
#ifdef O_WRONLY
#define creat(filename, mode) open(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define creat(filename, mode) open(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* creat = open */

#if defined(CONFIG_HAVE_open64) && !defined(CONFIG_HAVE_creat64) && \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_creat64 1
#undef creat64
#ifdef O_WRONLY
#define creat64(filename, mode) open64(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define creat64(filename, mode) open64(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* creat64 = open64 */

#if defined(CONFIG_HAVE_wopen) && !defined(CONFIG_HAVE_wopen64) && defined(CONFIG_HAVE_O_LARGEFILE)
#define CONFIG_HAVE_wopen64 1
#undef wopen64
#define wopen64(filename, oflags, ...) wopen(filename, (oflags) | O_LARGEFILE, ##__VA_ARGS__)
#endif /* wopen64 = wopen */

#ifndef CONFIG_HAVE_wcreat
#if defined(CONFIG_HAVE_wcreat64)
#define CONFIG_HAVE_wcreat 1
#undef wcreat
#define wcreat wcreat64 /* wcreat = wcreat64 */
#elif defined(CONFIG_HAVE_wopen) && \
     (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_wcreat 1
#undef wcreat
#ifdef O_WRONLY
#define wcreat(filename, mode) wopen(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define wcreat(filename, mode) wopen(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* wcreat = wopen */
#endif /* !CONFIG_HAVE_wcreat */

#if defined(CONFIG_HAVE_wopen64) && \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC))
#define CONFIG_HAVE_wcreat64 1
#undef wcreat64
#ifdef O_WRONLY
#define wcreat64(filename, mode) wopen64(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)
#else /* O_WRONLY */
#define wcreat64(filename, mode) wopen64(filename, O_CREAT | O_RDWR | O_TRUNC, mode)
#endif /* !O_WRONLY */
#endif /* wcreat64 = wopen64 */

#if !defined(CONFIG_HAVE_get_osfhandle) && defined(CONFIG_HAVE__get_osfhandle)
#define CONFIG_HAVE_get_osfhandle 1
#undef get_osfhandle
#define get_osfhandle _get_osfhandle
#endif /* get_osfhandle = _get_osfhandle */

#if !defined(CONFIG_HAVE_open_osfhandle) && defined(CONFIG_HAVE__open_osfhandle)
#define CONFIG_HAVE_open_osfhandle 1
#undef open_osfhandle
#define open_osfhandle _open_osfhandle
#endif /* open_osfhandle = _open_osfhandle */

#if defined(CONFIG_HAVE_pthread_suspend_np) && !defined(CONFIG_HAVE_pthread_suspend)
#define CONFIG_HAVE_pthread_suspend 1
#undef pthread_suspend
#define pthread_suspend pthread_suspend_np
#endif /* pthread_suspend = pthread_suspend_np */

#if defined(CONFIG_HAVE_pthread_suspend) && !defined(CONFIG_HAVE_pthread_suspend_np)
#define CONFIG_HAVE_pthread_suspend_np 1
#undef pthread_suspend_np
#define pthread_suspend_np pthread_suspend
#endif /* pthread_suspend_np = pthread_suspend */

#if defined(CONFIG_HAVE_pthread_unsuspend_np) && !defined(CONFIG_HAVE_pthread_continue)
#define CONFIG_HAVE_pthread_continue 1
#undef pthread_continue
#define pthread_continue pthread_unsuspend_np
#endif /* pthread_continue = pthread_unsuspend_np */

#if defined(CONFIG_HAVE_pthread_continue) && !defined(CONFIG_HAVE_pthread_unsuspend_np)
#define CONFIG_HAVE_pthread_unsuspend_np 1
#undef pthread_unsuspend_np
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
#undef pthread_setname
#define pthread_setname pthread_setname_np
#endif /* pthread_setname = pthread_setname_np */

#if defined(CONFIG_HAVE_pthread_setname) && !defined(CONFIG_HAVE_pthread_setname_np)
#define CONFIG_HAVE_pthread_setname_np 1
#undef pthread_setname_np
#define pthread_setname_np pthread_setname
#endif /* pthread_setname_np = pthread_setname */

#ifndef CONFIG_HAVE_abort
#define CONFIG_HAVE_abort 1
#undef abort
#define abort() _DeeAssert_Fail(NULL, __FILE__, __LINE__)
#endif /* !CONFIG_HAVE_abort */

#if !defined(CONFIG_HAVE_pause) && defined(CONFIG_HAVE_select)
#define CONFIG_HAVE_pause 1
#undef pause
#define pause() select(0, NULL, NULL, NULL, NULL)
#endif /* pause = select */

#if !defined(CONFIG_HAVE_doserrno) && defined(CONFIG_HAVE__doserrno)
#define CONFIG_HAVE_doserrno 1
#undef doserrno
#define doserrno _doserrno
#endif /* doserrno = _doserrno */

/* Really not needed on KOS... */
#ifdef __KOS__
#undef CONFIG_HAVE_doserrno
#undef CONFIG_HAVE__doserrno
#endif /* __KOS__ */


#if defined(_MSC_VER) || defined(__USE_DOS_ALTERATIONS)
#define EXEC_STRING_VECTOR_TYPE char const *const *
#else /* _MSC_VER || __USE_DOS_ALTERATIONS */
#define EXEC_STRING_VECTOR_TYPE char *const *
#endif /* !_MSC_VER && !__USE_DOS_ALTERATIONS */

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

#ifndef EOK
#ifdef ENOERR
#define EOK ENOERR
#elif defined(ENOERROR)
#define EOK ENOERROR
#else /* ... */
#define EOK 0
#endif /* !... */
#endif /* !EOK */

#ifndef NULL
#ifdef __NULLPTR
#define NULL __NULLPTR
#elif defined(__cplusplus)
#define NULL 0
#else /* __cplusplus */
#define NULL ((void *)0)
#endif /* !__cplusplus */
#endif /* !NULL */

#if defined(CONFIG_HAVE__errno) && !defined(CONFIG_HAVE_errno)
#define CONFIG_HAVE_errno 1
#undef errno
#define errno _errno
#endif /* errno = _errno */

#if defined(CONFIG_HAVE___errno) && !defined(CONFIG_HAVE_errno)
#define CONFIG_HAVE_errno 1
#undef errno
#define errno __errno
#endif /* errno = __errno */

#ifdef CONFIG_HAVE_errno
#define DeeSystem_GetErrno()  errno
#define DeeSystem_SetErrno(v) (errno = (v))
#else /* CONFIG_HAVE_errno */
#define DeeSystem_GetErrno()  0
#define DeeSystem_SetErrno(v) (void)0
#endif /* !CONFIG_HAVE_errno */

#ifndef CONFIG_HAVE_memcasecmp
#undef memcasecmp
#ifdef CONFIG_HAVE__memicmp
#define CONFIG_HAVE_memcasecmp 1
#define memcasecmp _memicmp
#elif defined(CONFIG_HAVE_memicmp)
#define CONFIG_HAVE_memcasecmp 1
#define memcasecmp memicmp
#endif /* ... */
#endif /* !CONFIG_HAVE_memcasecmp */

#ifndef CONFIG_HAVE_strcasecmp
#undef strcasecmp
#ifdef CONFIG_HAVE__stricmp
#define CONFIG_HAVE_strcasecmp 1
#define strcasecmp _stricmp
#elif defined(CONFIG_HAVE__strcmpi)
#define CONFIG_HAVE_strcasecmp 1
#define strcasecmp _strcmpi
#elif defined(CONFIG_HAVE_stricmp)
#define CONFIG_HAVE_strcasecmp 1
#define strcasecmp stricmp
#elif defined(CONFIG_HAVE_strcmpi)
#define CONFIG_HAVE_strcasecmp 1
#define strcasecmp strcmpi
#endif /* ... */
#endif /* !CONFIG_HAVE_strcasecmp */

#ifndef CONFIG_HAVE_tolower
#define CONFIG_HAVE_tolower 1
#undef tolower
#define tolower(ch) ((ch) >= 'A' && (ch) <= 'Z' ? ((ch) + ('a' - 'A')) : (ch))
#endif /* !CONFIG_HAVE_tolower */

#ifndef CONFIG_HAVE_toupper
#define CONFIG_HAVE_toupper 1
#undef toupper
#define toupper(ch) ((ch) >= 'a' && (ch) <= 'z' ? ((ch) - ('a' - 'A')) : (ch))
#endif /* !CONFIG_HAVE_toupper */

#ifndef CONFIG_HAVE_isupper
#define CONFIG_HAVE_isupper 1
#undef isupper
#define isupper(ch) ((ch) >= 'A' && (ch) <= 'Z')
#endif /* !CONFIG_HAVE_isupper */

#ifndef CONFIG_HAVE_islower
#define CONFIG_HAVE_islower 1
#undef islower
#define islower(ch) ((ch) >= 'a' && (ch) <= 'z')
#endif /* !CONFIG_HAVE_islower */

#ifndef CONFIG_HAVE_isalpha
#define CONFIG_HAVE_isalpha 1
#undef isalpha
#define isalpha(ch) (islower(ch) || isupper(ch))
#endif /* !CONFIG_HAVE_isalpha */

#ifndef CONFIG_HAVE_isdigit
#define CONFIG_HAVE_isdigit 1
#undef isdigit
#define isdigit(ch) ((ch) >= '0' && (ch) <= '9')
#endif /* !CONFIG_HAVE_isdigit */

#ifndef CONFIG_HAVE_isalnum
#define CONFIG_HAVE_isalnum 1
#undef isalnum
#define isalnum(ch) (isalpha(ch) || isdigit(ch))
#endif /* !CONFIG_HAVE_isalnum */


/* Mandatory <string.h> features */
#ifndef CONFIG_HAVE_strlen
#define CONFIG_HAVE_strlen 1
DECL_BEGIN
#undef strlen
#define strlen dee_strlen
LOCAL WUNUSED NONNULL((1)) size_t dee_strlen(char const *str) {
	size_t result;
	for (result = 0; str[result]; ++result)
		;
	return result;
}
DECL_END
#endif /* !CONFIG_HAVE_strlen */

#ifndef CONFIG_HAVE_strchr
#define CONFIG_HAVE_strchr 1
DECL_BEGIN
#undef strchr
#define strchr dee_strchr
LOCAL WUNUSED NONNULL((1)) char *
dee_strchr(char const *haystack, char needle) {
	for (;; ++haystack) {
		char ch = *haystack;
		if (ch == needle)
			return (char *)haystack;
		if (!ch)
			break;
	}
	return NULL;
}
DECL_END
#endif /* !CONFIG_HAVE_strchr */

#ifndef CONFIG_HAVE_memcpy
#define CONFIG_HAVE_memcpy 1
DECL_BEGIN
#undef memcpy
#define memcpy dee_memcpy
LOCAL WUNUSED NONNULL((1, 2)) void *
dee_memcpy(void *__restrict dst, void const *__restrict src, size_t num_bytes) {
	uint8_t *pdst = (uint8_t *)dst;
	uint8_t const *psrc = (uint8_t const *)src;
	while (num_bytes--)
		*pdst++ = *psrc++;
	return dst;
}
DECL_END
#endif /* !CONFIG_HAVE_memcpy */

#ifndef CONFIG_HAVE_memset
#define CONFIG_HAVE_memset 1
DECL_BEGIN
#undef memset
#define memset dee_memset
LOCAL WUNUSED NONNULL((1)) void *
dee_memset(void *__restrict dst, int byte, size_t num_bytes) {
	uint8_t *pdst = (uint8_t *)dst;
	while (num_bytes--)
		*pdst++ = (uint8_t)(unsigned int)byte;
	return dst;
}
DECL_END
#endif /* !CONFIG_HAVE_memset */

#ifndef CONFIG_HAVE_memmove
#define CONFIG_HAVE_memmove 1
DECL_BEGIN
#undef memmove
#define memmove dee_memmove
LOCAL WUNUSED NONNULL((1, 2)) void *
dee_memmove(void *dst, void const *src, size_t num_bytes) {
	uint8_t *pdst;
	uint8_t const *psrc;
	if (dst <= src) {
		pdst = (uint8_t *)dst;
		psrc = (uint8_t const *)src;
		while (num_bytes--)
			*pdst++ = *psrc++;
	} else {
		pdst = (uint8_t *)dst + num_bytes;
		psrc = (uint8_t const *)src + num_bytes;
		while (num_bytes--)
			*--pdst = *--psrc;
	}
	return dst;
}
DECL_END
#endif /* !CONFIG_HAVE_memmove */


#define _DeeSystem_DEFINE_memchrT(rT, T, Tneedle, name)   \
	LOCAL WUNUSED NONNULL((1)) rT *                       \
	name(void const *__restrict p, Tneedle c, size_t n) { \
		T const *hay_iter = (T const *)p;                 \
		for (; n--; ++hay_iter) {                         \
			if unlikely(*hay_iter == (T)c)                \
				return (rT *)hay_iter;                    \
		}                                                 \
		return NULL;                                      \
	}

#define _DeeSystem_DEFINE_memrchrT(rT, T, Tneedle, name)  \
	LOCAL WUNUSED NONNULL((1)) rT *                       \
	name(void const *__restrict p, Tneedle c, size_t n) { \
		T const *iter = (T const *)p + n;                 \
		while (iter != (T const *)p) {                    \
			if unlikely(*--iter == (T)c)                  \
				return (rT *)iter;                        \
		}                                                 \
		return NULL;                                      \
	}

#define _DeeSystem_DEFINE_memcmpT(rT, T, name)        \
	LOCAL WUNUSED NONNULL((1)) rT                     \
	name(void const *s1, void const *s2, size_t n) {  \
		T const *p1 = (T const *)s1;                  \
		T const *p2 = (T const *)s2;                  \
		T v1, v2;                                     \
		v1 = v2 = 0;                                  \
		while (n-- && ((v1 = *p1++) == (v2 = *p2++))) \
			;                                         \
		return (rT)v1 - (rT)v2;                       \
	}

#define DeeSystem_DEFINE_memrchr(name) _DeeSystem_DEFINE_memrchrT(void, uint8_t, int, name)
#define DeeSystem_DEFINE_memrchrw(name) _DeeSystem_DEFINE_memrchrT(uint16_t, uint16_t, uint16_t, name)
#define DeeSystem_DEFINE_memrchrl(name) _DeeSystem_DEFINE_memrchrT(uint32_t, uint32_t, uint32_t, name)
#define DeeSystem_DEFINE_memrchrq(name) _DeeSystem_DEFINE_memrchrT(uint64_t, uint64_t, uint64_t, name)

#define DeeSystem_DEFINE_memchrw(name) _DeeSystem_DEFINE_memchrT(uint16_t, uint16_t, uint16_t, name)
#define DeeSystem_DEFINE_memchrl(name) _DeeSystem_DEFINE_memchrT(uint32_t, uint32_t, uint32_t, name)
#define DeeSystem_DEFINE_memchrq(name) _DeeSystem_DEFINE_memchrT(uint64_t, uint64_t, uint64_t, name)

#define DeeSystem_DEFINE_memcmpw(name) _DeeSystem_DEFINE_memcmpT(int16_t, int16_t, name)
#define DeeSystem_DEFINE_memcmpl(name) _DeeSystem_DEFINE_memcmpT(int32_t, int32_t, name)
#define DeeSystem_DEFINE_memcmpq(name) _DeeSystem_DEFINE_memcmpT(int64_t, int64_t, name)

#ifndef CONFIG_HAVE_memchr
#define CONFIG_HAVE_memchr 1
DECL_BEGIN
#undef memchr
#define memchr dee_memchr
_DeeSystem_DEFINE_memchrT(void, uint8_t, int, dee_memchr)
DECL_END
#endif /* !CONFIG_HAVE_memchr */

#ifndef CONFIG_HAVE_memcmp
#define CONFIG_HAVE_memcmp 1
DECL_BEGIN
#undef memcmp
#define memcmp dee_memcmp
_DeeSystem_DEFINE_memcmpT(int, uint8_t, dee_memcmp)
DECL_END
#endif /* !CONFIG_HAVE_memcmp */

#define _DeeSystem_DEFINE_memmemT(rT, T, memchr, memeq, name)                          \
	LOCAL ATTR_PURE WUNUSED rT *                                                       \
	name(void const *__restrict haystack, size_t haystack_length,                      \
	     void const *__restrict needle, size_t needle_length) {                        \
		T *candidate;                                                                  \
		T marker;                                                                      \
		if unlikely(!needle_length || needle_length > haystack_length)                 \
			return NULL;                                                               \
		haystack_length -= (needle_length - 1), marker = *(T *)needle;                 \
		while ((candidate = (T *)memchr(haystack, marker, haystack_length)) != NULL) { \
			if (memeq(candidate, needle, needle_length))                               \
				return (rT *)candidate;                                                \
			++candidate;                                                               \
			haystack_length = ((T *)haystack + haystack_length) - candidate;           \
			haystack        = (void const *)candidate;                                 \
		}                                                                              \
		return NULL;                                                                   \
	}

#define _DeeSystem_DEFINE_memrmemT(rT, T, memrchr, memeq, name)                    \
	LOCAL ATTR_PURE WUNUSED rT *                                                   \
	name(void const *__restrict haystack, size_t haystack_length,                  \
	               void const *__restrict needle, size_t needle_length) {          \
		void const *candidate;                                                     \
		T marker;                                                                  \
		if unlikely(!needle_length || needle_length > haystack_length)             \
			return NULL;                                                           \
		haystack_length -= needle_length - 1;                                      \
		marker = *(T *)needle;                                                     \
		while ((candidate = memrchr(haystack, marker, haystack_length)) != NULL) { \
			if (memeq(candidate, needle, needle_length))                           \
				return (rT *)candidate;                                            \
			haystack_length = (size_t)((T *)candidate - (T *)haystack);            \
		}                                                                          \
		return NULL;                                                               \
	}

#define DeeSystem_DEFINE_memmem(name) _DeeSystem_DEFINE_memmemT(void, uint8_t, memchr, 0 == memcmp, name)
#define DeeSystem_DEFINE_memmemw(name, memchrw, memeqw) _DeeSystem_DEFINE_memmemT(uint16_t, uint16_t, memchrw, memeqw, name)
#define DeeSystem_DEFINE_memmeml(name, memchrl, memeql) _DeeSystem_DEFINE_memmemT(uint32_t, uint32_t, memchrl, memeql, name)
#define DeeSystem_DEFINE_memmemq(name, memchrq, memeqq) _DeeSystem_DEFINE_memmemT(uint64_t, uint64_t, memchrq, memeqq, name)

#define DeeSystem_DEFINE_memrmem(name) _DeeSystem_DEFINE_memrmemT(void, uint8_t, memrchr, 0 == memcmp, name)
#define DeeSystem_DEFINE_memrmemw(name, memrchrw, memeqw) _DeeSystem_DEFINE_memrmemT(uint16_t, uint16_t, memrchrw, memeqw, name)
#define DeeSystem_DEFINE_memrmeml(name, memrchrl, memeql) _DeeSystem_DEFINE_memrmemT(uint32_t, uint32_t, memrchrl, memeql, name)
#define DeeSystem_DEFINE_memrmemq(name, memrchrq, memeqq) _DeeSystem_DEFINE_memrmemT(uint64_t, uint64_t, memrchrq, memeqq, name)

#define DeeSystem_DEFINE_strnlen(name)                              \
	LOCAL ATTR_PURE WUNUSED size_t                                  \
	name(char const *__restrict str, size_t maxlen) {               \
		size_t result;                                              \
		for (result = 0; maxlen && *str; --maxlen, ++str, ++result) \
			;                                                       \
		return result;                                              \
	}

#define DeeSystem_DEFINE_XSTRLEN(name, T)       \
	LOCAL ATTR_PURE WUNUSED NONNULL((1)) size_t \
	name(T const *__restrict str) {             \
		size_t result;                          \
		for (result = 0; *str; ++str, ++result) \
			;                                   \
		return result;                          \
	}

#define DeeSystem_DEFINE_wcslen(name) \
	DeeSystem_DEFINE_XSTRLEN(name, Dee_wchar_t)

#define DeeSystem_DEFINE_rawmemchr(name)        \
	LOCAL ATTR_PURE WUNUSED NONNULL((1)) void * \
	name(void const *__restrict p, int c) {     \
		uint8_t *haystack = (uint8_t *)p;       \
		for (;; ++haystack) {                   \
			if (*haystack == (uint8_t)c)        \
				break;                          \
		}                                       \
		return haystack;                        \
	}

#define DeeSystem_DEFINE_rawmemrchr(name)       \
	LOCAL ATTR_PURE WUNUSED NONNULL((1)) void * \
	name(void const *__restrict p, int c) {     \
		uint8_t *haystack = (uint8_t *)p;       \
		for (;;) {                              \
			if (*--haystack == (uint8_t)c)      \
				break;                          \
		}                                       \
		return haystack;                        \
	}

#define DeeSystem_DEFINE_memend(name)                 \
	LOCAL ATTR_PURE WUNUSED void *                    \
	name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;             \
		for (; num_bytes--; ++haystack) {             \
			if (*haystack == (uint8_t)byte)           \
				break;                                \
		}                                             \
		return haystack;                              \
	}

#define DeeSystem_DEFINE_memxend(name)                \
	LOCAL ATTR_PURE WUNUSED void *                    \
	name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;             \
		for (; num_bytes--; ++haystack) {             \
			if (*haystack != (uint8_t)byte)           \
				break;                                \
		}                                             \
		return haystack;                              \
	}

#define DeeSystem_DEFINE_memrend(name)                \
	LOCAL ATTR_PURE WUNUSED void *                    \
	name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;             \
		haystack += num_bytes;                        \
		while (num_bytes--) {                         \
			if (*--haystack == (uint8_t)byte)         \
				break;                                \
		}                                             \
		return haystack;                              \
	}

#define DeeSystem_DEFINE_memlen(name)                                          \
	LOCAL ATTR_PURE WUNUSED size_t                                             \
	name(void const *p, int byte, size_t num_bytes) {                          \
		return (size_t)((uintptr_t)memend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memxlen(name)                                          \
	LOCAL ATTR_PURE WUNUSED size_t                                              \
	name(void const *p, int byte, size_t num_bytes) {                           \
		return (size_t)((uintptr_t)memxend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memrlen(name)                                          \
	LOCAL ATTR_PURE WUNUSED size_t                                              \
	name(void const *p, int byte, size_t num_bytes) {                           \
		return (size_t)((uintptr_t)memrend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_rawmemlen(name)                               \
	LOCAL ATTR_PURE WUNUSED size_t                                     \
	name(void const *p, int byte) {                                    \
		return (size_t)((uintptr_t)rawmemchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_rawmemrlen(name)                               \
	LOCAL ATTR_PURE WUNUSED size_t                                      \
	name(void const *p, int byte) {                                     \
		return (size_t)((uintptr_t)rawmemrchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memxchr(name)                \
	LOCAL ATTR_PURE WUNUSED void *                    \
	name(void const *__restrict p, int c, size_t n) { \
		uint8_t *haystack = (uint8_t *)p;             \
		for (; n--; ++haystack) {                     \
			if (*haystack != (uint8_t)c)              \
				return haystack;                      \
		}                                             \
		return NULL;                                  \
	}

#define DeeSystem_DEFINE_rawmemxchr(name)   \
	LOCAL ATTR_PURE WUNUSED void *          \
	name(void const *__restrict p, int c) { \
		uint8_t *haystack = (uint8_t *)p;   \
		for (;; ++haystack) {               \
			if (*haystack != (uint8_t)c)    \
				break;                      \
		}                                   \
		return haystack;                    \
	}

#define DeeSystem_DEFINE_rawmemxlen(name)                               \
	LOCAL ATTR_PURE WUNUSED size_t                                      \
	name(void const *p, int byte) {                                     \
		return (size_t)((uintptr_t)rawmemxchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memcasecmp(name)               \
	LOCAL ATTR_PURE WUNUSED int                         \
	name(void const *a, void const *b, size_t n) {      \
		uint8_t *pa = (uint8_t *)a, *pb = (uint8_t *)b; \
		uint8_t av, bv;                                 \
		av = bv = 0;                                    \
		while (n-- &&                                   \
		       (((av = *pa++) == (bv = *pb++)) ||       \
		        (av = tolower(av), bv = tolower(bv),    \
		         av == bv)))                            \
			;                                           \
		return (int)av - (int)bv;                       \
	}

#define DeeSystem_DEFINE_memcasemem(name)                                               \
	LOCAL ATTR_PURE WUNUSED void *                                                      \
	name(void const *__restrict haystack, size_t haystack_len,                          \
	                 void const *__restrict needle, size_t needle_len) {                \
		void const *candidate;                                                          \
		uint8_t marker1, marker2;                                                       \
		if unlikely(!needle_len || needle_len > haystack_len)                           \
			return NULL;                                                                \
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

#define DeeSystem_DEFINE_memxrchr(name)               \
	LOCAL ATTR_PURE WUNUSED void *                    \
	name(void const *__restrict p, int c, size_t n) { \
		uint8_t *haystack = (uint8_t *)p + n;         \
		while (n--) {                                 \
			if (*--haystack != (uint8_t)c)            \
				return haystack;                      \
		}                                             \
		return NULL;                                  \
	}

#define DeeSystem_DEFINE_memxrend(name)               \
	LOCAL ATTR_PURE WUNUSED void *                    \
	name(void const *p, int byte, size_t num_bytes) { \
		uint8_t *haystack = (uint8_t *)p;             \
		haystack += num_bytes;                        \
		while (num_bytes--) {                         \
			if (*--haystack != (uint8_t)byte)         \
				break;                                \
		}                                             \
		return haystack;                              \
	}

#define DeeSystem_DEFINE_memxrlen(name)                                          \
	LOCAL ATTR_PURE WUNUSED size_t                                               \
	name(void const *p, int byte, size_t num_bytes) {                            \
		return (size_t)((uintptr_t)memxrend(p, byte, num_bytes) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_rawmemxrchr(name)  \
	LOCAL ATTR_PURE WUNUSED void *          \
	name(void const *__restrict p, int c) { \
		uint8_t *haystack = (uint8_t *)p;   \
		for (;;) {                          \
			if (*--haystack != (uint8_t)c)  \
				break;                      \
		}                                   \
		return haystack;                    \
	}

#define DeeSystem_DEFINE_rawmemxrlen(name)                               \
	LOCAL ATTR_PURE WUNUSED NONNULL((1)) size_t                          \
	name(void const *p, int byte) {                                      \
		return (size_t)((uintptr_t)rawmemxrchr(p, byte) - (uintptr_t)p); \
	}

#define DeeSystem_DEFINE_memcasermem(name)                                                     \
	LOCAL ATTR_PURE WUNUSED void *                                                             \
	_##name##_memlowerrchr(void const *__restrict p, uint8_t c, size_t n) {                    \
		uint8_t *iter = (uint8_t *)p + n;                                                      \
		while (iter-- != (uint8_t *)p) {                                                       \
			if ((uint8_t)tolower(*iter) == c)                                                  \
				return iter;                                                                   \
		}                                                                                      \
		return NULL;                                                                           \
	}                                                                                          \
	LOCAL ATTR_PURE WUNUSED void *                                                             \
	name(void const *__restrict haystack, size_t haystack_len,                                 \
	     void const *__restrict needle, size_t needle_len) {                                   \
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
			if unlikely(candidate == haystack)                                                 \
				break;                                                                         \
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

#define _DeeSystem_DEFINE_memsetT(T, name)                \
	LOCAL void *name(void *__restrict p, T c, size_t n) { \
		T *dst = (T *)p;                                  \
		while (n--)                                       \
			*dst++ = c;                                   \
		return p;                                         \
	}
#define DeeSystem_DEFINE_memsetw(name) _DeeSystem_DEFINE_memsetT(uint16_t, name)
#define DeeSystem_DEFINE_memsetl(name) _DeeSystem_DEFINE_memsetT(uint32_t, name)
#define DeeSystem_DEFINE_memsetq(name) _DeeSystem_DEFINE_memsetT(uint64_t, name)


#ifdef NDEBUG
#define DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size)   (void)0
#define DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size) (void)0
#else /* NDEBUG */
#define DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size)                \
	Dee_ASSERTF((dst) >= (src), "memmoveupc(%p, %p, %Iu, %Iu): Wrong direction", \
	            (void *)(dst), (void *)(src), (size_t)(elem_count), (size_t)(elem_size))
#define DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size)                \
	Dee_ASSERTF((dst) <= (src), "memmovedownc(%p, %p, %Iu, %Iu): Wrong direction", \
	            (void *)(dst), (void *)(src), (size_t)(elem_count), (size_t)(elem_size))
#endif /* !NDEBUG */


#ifndef CONFIG_HAVE_memmoveup
#define CONFIG_HAVE_memmoveup 1
#undef memmoveup
#define memmoveup(dst, src, n)     (DeeSytemAssert_MemmoveUp(dst, src, n, 1), memmove(dst, src, n))
#define dee_memmoveup(dst, src, n) memmove(dst, src, n)
#endif /* !CONFIG_HAVE_memmoveup */

#ifndef CONFIG_HAVE_memmovedown
#define CONFIG_HAVE_memmovedown 1
#undef memmovedown
#define memmovedown(dst, src, n)     (DeeSytemAssert_MemmoveDown(dst, src, n, 1), memmove(dst, src, n))
#define dee_memmovedown(dst, src, n) memmove(dst, src, n)
#endif /* !CONFIG_HAVE_memmovedown */


/* KOS's multi-byte memory function extensions */
#ifndef CONFIG_HAVE_memcpyw
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#define CONFIG_HAVE_memcpyw 1
DECL_BEGIN
#undef memcpyw
#ifdef __x86_64__
extern void __movsw(unsigned short *, unsigned short const *, unsigned __int64);
#define memcpyw(dst, src, n)                              \
	(__movsw((unsigned short *)(void *)(dst),             \
	         (unsigned short const *)(void const *)(src), \
	         (unsigned __int64)(n)),                      \
	 (uint16_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __movsw(unsigned short *, unsigned short const *, unsigned int);
#define memcpyw(dst, src, n)                              \
	(__movsw((unsigned short *)(void *)(dst),             \
	         (unsigned short const *)(void const *)(src), \
	         (unsigned int)(n)),                          \
	 (uint16_t *)(void *)(dst))
#endif /* !__x86_64__ */
#pragma intrinsic(__movsw)
DECL_END
#endif /* _MSC_VER && (__i386__ || __x86_64__) */
#endif /* !CONFIG_HAVE_memcpyw */

#ifndef CONFIG_HAVE_memcpyl
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#define CONFIG_HAVE_memcpyl 1
DECL_BEGIN
#undef memcpyl
#ifdef __x86_64__
extern void __movsd(unsigned long *, unsigned long const *, unsigned __int64);
#define memcpyl(dst, src, n)                             \
	(__movsd((unsigned long *)(void *)(dst),             \
	         (unsigned long const *)(void const *)(src), \
	         (unsigned __int64)(n)),                     \
	 (uint32_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __movsd(unsigned long *, unsigned long const *, unsigned int);
#define memcpyl(dst, src, n)                             \
	(__movsd((unsigned long *)(void *)(dst),             \
	         (unsigned long const *)(void const *)(src), \
	         (unsigned int)(n)),                         \
	 (uint32_t *)(void *)(dst))
#endif /* !__x86_64__ */
#pragma intrinsic(__movsd)
DECL_END
#endif /* _MSC_VER && (__i386__ || __x86_64__) */
#endif /* !CONFIG_HAVE_memcpyl */

#ifndef CONFIG_HAVE_memcpyq
#if defined(_MSC_VER) && defined(__x86_64__)
#define CONFIG_HAVE_memcpyq 1
DECL_BEGIN
extern void __movsq(unsigned long long *, unsigned long long const *, unsigned __int64);
#undef memcpyq
#define memcpyq(dst, src, n)                                  \
	(__movsq((unsigned long long *)(void *)(dst),             \
	         (unsigned long long const *)(void const *)(src), \
	         (unsigned __int64)(n)),                          \
	 (uint64_t *)(void *)(dst))
#pragma intrinsic(__movsq)
DECL_END
#endif /* _MSC_VER && __x86_64__ */
#endif /* !CONFIG_HAVE_memcpyq */

#ifndef CONFIG_HAVE_memcpyc
#define CONFIG_HAVE_memcpyc 1
#undef memcpyc
#if defined(CONFIG_HAVE_memcpyw) && defined(CONFIG_HAVE_memcpyl) && defined(CONFIG_HAVE_memcpyq)
#define memcpyc(dst, src, elem_count, elem_size) \
	((elem_size) == 2                            \
	 ? (void *)memcpyw(dst, src, elem_count)     \
	 : (elem_size) == 4                          \
	   ? (void *)memcpyl(dst, src, elem_count)   \
	   : (elem_size) == 8                        \
	     ? (void *)memcpyq(dst, src, elem_count) \
	     : memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memcpyw) && defined(CONFIG_HAVE_memcpyl)
#define memcpyc(dst, src, elem_count, elem_size) \
	((elem_size) == 2                            \
	 ? (void *)memcpyw(dst, src, elem_count)     \
	 : (elem_size) == 4                          \
	   ? (void *)memcpyl(dst, src, elem_count)   \
	   : memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memcpyw) && defined(CONFIG_HAVE_memcpyq)
#define memcpyc(dst, src, elem_count, elem_size) \
	((elem_size) == 2                            \
	 ? (void *)memcpyw(dst, src, elem_count)     \
	 : (elem_size) == 8                          \
	   ? (void *)memcpyq(dst, src, elem_count)   \
	   : memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memcpyl) && defined(CONFIG_HAVE_memcpyq)
#define memcpyc(dst, src, elem_count, elem_size) \
	((elem_size) == 4                            \
	 ? (void *)memcpyl(dst, src, elem_count)     \
	 : (elem_size) == 8                          \
	   ? (void *)memcpyq(dst, src, elem_count)   \
	   : memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memcpyq)
#define memcpyc(dst, src, elem_count, elem_size) \
	((elem_size) == 8                            \
	 ? (void *)memcpyq(dst, src, elem_count)     \
	 : memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memcpyl)
#define memcpyc(dst, src, elem_count, elem_size) \
	((elem_size) == 4                            \
	 ? (void *)memcpyl(dst, src, elem_count)     \
	 : memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memcpyw)
#define memcpyc(dst, src, elem_count, elem_size) \
	((elem_size) == 2                            \
	 ? (void *)memcpyw(dst, src, elem_count)     \
	 : memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#else /* ... */
#define memcpyc(dst, src, elem_count, elem_size) \
	memcpy(dst, src, (size_t)(elem_count) * (size_t)(elem_size))
#endif /* !... */
#endif /* !CONFIG_HAVE_memcpyc */

#ifndef CONFIG_HAVE_memcpyq
#define CONFIG_HAVE_memcpyq 1
#undef memcpyq
#ifdef CONFIG_HAVE_memcpyl
#define memcpyq(dst, src, n) (uint64_t *)memcpyl(dst, src, (n) * 2)
#elif defined(CONFIG_HAVE_memcpyw)
#define memcpyq(dst, src, n) (uint64_t *)memcpyw(dst, src, (n) * 4)
#else /* ... */
#define memcpyq(dst, src, n) (uint64_t *)memcpy(dst, src, (n) * 8)
#endif /* !... */
#endif /* !CONFIG_HAVE_memcpyq */

#ifndef CONFIG_HAVE_memcpyl
#define CONFIG_HAVE_memcpyl 1
#undef memcpyl
#ifdef CONFIG_HAVE_memcpyw
#define memcpyl(dst, src, n) (uint32_t *)memcpyw(dst, src, (n) * 2)
#else /* CONFIG_HAVE_memcpyw */
#define memcpyl(dst, src, n) (uint32_t *)memcpy(dst, src, (n) * 4)
#endif /* !CONFIG_HAVE_memcpyw */
#endif /* !CONFIG_HAVE_memcpyl */

#ifndef CONFIG_HAVE_memcpyw
#define CONFIG_HAVE_memcpyw 1
#undef memcpyw
#define memcpyw(dst, src, n) (uint16_t *)memcpy(dst, src, (n) * 2)
#endif /* !CONFIG_HAVE_memcpyw */

#ifndef CONFIG_HAVE_memmovec
#define CONFIG_HAVE_memmovec 1
#undef memmovec
#if defined(CONFIG_HAVE_memmovew) && defined(CONFIG_HAVE_memmovel) && defined(CONFIG_HAVE_memmoveq)
#define memmovec(dst, src, elem_count, elem_size) \
	((elem_size) == 2                             \
	 ? (void *)memmovew(dst, src, elem_count)     \
	 : (elem_size) == 4                           \
	   ? (void *)memmovel(dst, src, elem_count)   \
	   : (elem_size) == 8                         \
	     ? (void *)memmoveq(dst, src, elem_count) \
	     : memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovew) && defined(CONFIG_HAVE_memmovel)
#define memmovec(dst, src, elem_count, elem_size) \
	((elem_size) == 2                             \
	 ? (void *)memmovew(dst, src, elem_count)     \
	 : (elem_size) == 4                           \
	   ? (void *)memmovel(dst, src, elem_count)   \
	   : memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovew) && defined(CONFIG_HAVE_memmoveq)
#define memmovec(dst, src, elem_count, elem_size) \
	((elem_size) == 2                             \
	 ? (void *)memmovew(dst, src, elem_count)     \
	 : (elem_size) == 8                           \
	   ? (void *)memmoveq(dst, src, elem_count)   \
	   : memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovel) && defined(CONFIG_HAVE_memmoveq)
#define memmovec(dst, src, elem_count, elem_size) \
	((elem_size) == 4                             \
	 ? (void *)memmovel(dst, src, elem_count)     \
	 : (elem_size) == 8                           \
	   ? (void *)memmoveq(dst, src, elem_count)   \
	   : memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmoveq)
#define memmovec(dst, src, elem_count, elem_size) \
	((elem_size) == 8                             \
	 ? (void *)memmoveq(dst, src, elem_count)     \
	 : memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovel)
#define memmovec(dst, src, elem_count, elem_size) \
	((elem_size) == 4                             \
	 ? (void *)memmovel(dst, src, elem_count)     \
	 : memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovew)
#define memmovec(dst, src, elem_count, elem_size) \
	((elem_size) == 2                             \
	 ? (void *)memmovew(dst, src, elem_count)     \
	 : memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#else /* ... */
#define memmovec(dst, src, elem_count, elem_size) \
	memmove(dst, src, (size_t)(elem_count) * (size_t)(elem_size))
#endif /* !... */
#endif /* !CONFIG_HAVE_memmovec */

#ifndef CONFIG_HAVE_memmoveq
#define CONFIG_HAVE_memmoveq 1
#undef memmoveq
#ifdef CONFIG_HAVE_memmovel
#define memmoveq(dst, src, n) (uint64_t *)memmovel(dst, src, (n) * 2)
#elif defined(CONFIG_HAVE_memmovew)
#define memmoveq(dst, src, n) (uint64_t *)memmovew(dst, src, (n) * 4)
#else /* ... */
#define memmoveq(dst, src, n) (uint64_t *)memmove(dst, src, (n) * 8)
#endif /* !... */
#endif /* !CONFIG_HAVE_memmoveq */

#ifndef CONFIG_HAVE_memmovel
#define CONFIG_HAVE_memmovel 1
#undef memmovel
#ifdef CONFIG_HAVE_memmovew
#define memmovel(dst, src, n) (uint32_t *)memmovew(dst, src, (n) * 2)
#else /* CONFIG_HAVE_memmovew */
#define memmovel(dst, src, n) (uint32_t *)memmove(dst, src, (n) * 4)
#endif /* !CONFIG_HAVE_memmovew */
#endif /* !CONFIG_HAVE_memmovel */

#ifndef CONFIG_HAVE_memmovew
#define CONFIG_HAVE_memmovew 1
#undef memmovew
#define memmovew(dst, src, n) (uint16_t *)memmove(dst, src, (n) * 2)
#endif /* !CONFIG_HAVE_memmovew */

#ifndef dee_memmoveup
#define dee_memmoveup memmoveup
#endif /* !dee_memmoveup */

#ifndef CONFIG_HAVE_memmoveupc
#define CONFIG_HAVE_memmoveupc 1
#undef memmoveupc
#if defined(CONFIG_HAVE_memmoveupw) && defined(CONFIG_HAVE_memmoveupl) && defined(CONFIG_HAVE_memmoveupq)
#define memmoveupc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                           \
	 ? (void *)memmoveupw(dst, src, elem_count)                 \
	 : (elem_size) == 4                                         \
	   ? (void *)memmoveupl(dst, src, elem_count)               \
	   : (elem_size) == 8                                       \
	     ? (void *)memmoveupq(dst, src, elem_count)             \
	     : dee_memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmoveupw) && defined(CONFIG_HAVE_memmoveupl)
#define memmoveupc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                           \
	 ? (void *)memmoveupw(dst, src, elem_count)                 \
	 : (elem_size) == 4                                         \
	   ? (void *)memmoveupl(dst, src, elem_count)               \
	   : dee_memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmoveupw) && defined(CONFIG_HAVE_memmoveupq)
#define memmoveupc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                           \
	 ? (void *)memmoveupw(dst, src, elem_count)                 \
	 : (elem_size) == 8                                         \
	   ? (void *)memmoveupq(dst, src, elem_count)               \
	   : dee_memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmoveupl) && defined(CONFIG_HAVE_memmoveupq)
#define memmoveupc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size), \
	 (elem_size) == 4                                           \
	 ? (void *)memmoveupl(dst, src, elem_count)                 \
	 : (elem_size) == 8                                         \
	   ? (void *)memmoveupq(dst, src, elem_count)               \
	   : dee_memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmoveupq)
#define memmoveupc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size), \
	 (elem_size) == 8                                           \
	 ? (void *)memmoveupq(dst, src, elem_count)                 \
	 : dee_memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmoveupl)
#define memmoveupc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size), \
	 (elem_size) == 4                                           \
	 ? (void *)memmoveupl(dst, src, elem_count)                 \
	 : dee_memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmoveupw)
#define memmoveupc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveUp(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                           \
	 ? (void *)memmoveupw(dst, src, elem_count)                 \
	 : dee_memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#else /* ... */
#define memmoveupc(dst, src, elem_count, elem_size) \
	memmoveup(dst, src, (size_t)(elem_count) * (size_t)(elem_size))
#endif /* !... */
#endif /* !CONFIG_HAVE_memmoveupc */

#ifndef CONFIG_HAVE_memmoveupq
#define CONFIG_HAVE_memmoveupq 1
#undef memmoveupq
#ifdef CONFIG_HAVE_memmoveupl
#define memmoveupq(dst, src, n) (uint64_t *)memmoveupl(dst, src, (n) * 2)
#elif defined(CONFIG_HAVE_memmoveupw)
#define memmoveupq(dst, src, n) (uint64_t *)memmoveupw(dst, src, (n) * 4)
#else /* ... */
#define memmoveupq(dst, src, n)                \
	(DeeSytemAssert_MemmoveUp(dst, src, n, 8), \
	 (uint64_t *)dee_memmoveup(dst, src, (n)*8))
#endif /* !... */
#endif /* !CONFIG_HAVE_memmoveupq */

#ifndef CONFIG_HAVE_memmoveupl
#define CONFIG_HAVE_memmoveupl 1
#undef memmoveupl
#ifdef CONFIG_HAVE_memmoveupw
#define memmoveupl(dst, src, n) (uint32_t *)memmoveupw(dst, src, (n)*2)
#else /* CONFIG_HAVE_memmoveupw */
#define memmoveupl(dst, src, n)                \
	(DeeSytemAssert_MemmoveUp(dst, src, n, 4), \
	 (uint32_t *)dee_memmoveup(dst, src, (n)*4))
#endif /* !CONFIG_HAVE_memmoveupw */
#endif /* !CONFIG_HAVE_memmoveupl */

#ifndef CONFIG_HAVE_memmoveupw
#define CONFIG_HAVE_memmoveupw 1
#undef memmoveupw
#define memmoveupw(dst, src, n)                \
	(DeeSytemAssert_MemmoveUp(dst, src, n, 2), \
	 (uint16_t *)dee_memmoveup(dst, src, (n)*2))
#endif /* !CONFIG_HAVE_memmoveupw */

#ifndef dee_memmovedown
#define dee_memmovedown memmovedown
#endif /* !dee_memmovedown */

#ifndef CONFIG_HAVE_memmovedownc
#define CONFIG_HAVE_memmovedownc 1
#undef memmovedownc
#if defined(CONFIG_HAVE_memmovedownw) && defined(CONFIG_HAVE_memmovedownl) && defined(CONFIG_HAVE_memmovedownq)
#define memmovedownc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                             \
	 ? (void *)memmovedownw(dst, src, elem_count)                 \
	 : (elem_size) == 4                                           \
	   ? (void *)memmovedownl(dst, src, elem_count)               \
	   : (elem_size) == 8                                         \
	     ? (void *)memmovedownq(dst, src, elem_count)             \
	     : dee_memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovedownw) && defined(CONFIG_HAVE_memmovedownl)
#define memmovedownc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                             \
	 ? (void *)memmovedownw(dst, src, elem_count)                 \
	 : (elem_size) == 4                                           \
	   ? (void *)memmovedownl(dst, src, elem_count)               \
	   : dee_memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovedownw) && defined(CONFIG_HAVE_memmovedownq)
#define memmovedownc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                             \
	 ? (void *)memmovedownw(dst, src, elem_count)                 \
	 : (elem_size) == 8                                           \
	   ? (void *)memmovedownq(dst, src, elem_count)               \
	   : dee_memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovedownl) && defined(CONFIG_HAVE_memmovedownq)
#define memmovedownc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size), \
	 (elem_size) == 4                                             \
	 ? (void *)memmovedownl(dst, src, elem_count)                 \
	 : (elem_size) == 8                                           \
	   ? (void *)memmovedownq(dst, src, elem_count)               \
	   : dee_memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovedownq)
#define memmovedownc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size), \
	 (elem_size) == 8                                             \
	 ? (void *)memmovedownq(dst, src, elem_count)                 \
	 : dee_memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovedownl)
#define memmovedownc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size), \
	 (elem_size) == 4                                             \
	 ? (void *)memmovedownl(dst, src, elem_count)                 \
	 : dee_memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_memmovedownw)
#define memmovedownc(dst, src, elem_count, elem_size)             \
	(DeeSytemAssert_MemmoveDown(dst, src, elem_count, elem_size), \
	 (elem_size) == 2                                             \
	 ? (void *)memmovedownw(dst, src, elem_count)                 \
	 : dee_memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size)))
#else /* ... */
#define memmovedownc(dst, src, elem_count, elem_size) \
	memmovedown(dst, src, (size_t)(elem_count) * (size_t)(elem_size))
#endif /* !... */
#endif /* !CONFIG_HAVE_memmovedownc */

#ifndef CONFIG_HAVE_memmovedownq
#define CONFIG_HAVE_memmovedownq 1
#undef memmovedownq
#ifdef CONFIG_HAVE_memmovedownl
#define memmovedownq(dst, src, n) (uint64_t *)memmovedownl(dst, src, (n)*2)
#elif defined(CONFIG_HAVE_memmovedownw)
#define memmovedownq(dst, src, n) (uint64_t *)memmovedownw(dst, src, (n)*4)
#else /* ... */
#define memmovedownq(dst, src, n)                \
	(DeeSytemAssert_MemmoveDown(dst, src, n, 8), \
	 (uint64_t *)dee_memmovedown(dst, src, (n)*8))
#endif /* !... */
#endif /* !CONFIG_HAVE_memmovedownq */

#ifndef CONFIG_HAVE_memmovedownl
#define CONFIG_HAVE_memmovedownl 1
#undef memmovedownl
#ifdef CONFIG_HAVE_memmovedownw
#define memmovedownl(dst, src, n) (uint32_t *)memmovedownw(dst, src, (n)*2)
#else /* CONFIG_HAVE_memmovedownw */
#define memmovedownl(dst, src, n)                \
	(DeeSytemAssert_MemmoveDown(dst, src, n, 4), \
	 (uint32_t *)dee_memmovedown(dst, src, (n)*4))
#endif /* !CONFIG_HAVE_memmovedownw */
#endif /* !CONFIG_HAVE_memmovedownl */

#ifndef CONFIG_HAVE_memmovedownw
#define CONFIG_HAVE_memmovedownw 1
#undef memmovedownw
#define memmovedownw(dst, src, n)                \
	(DeeSytemAssert_MemmoveDown(dst, src, n, 2), \
	 (uint16_t *)dee_memmovedown(dst, src, (n)*2))
#endif /* !CONFIG_HAVE_memmovedownw */

#ifndef CONFIG_HAVE_memsetw
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#define CONFIG_HAVE_memsetw 1
DECL_BEGIN
#undef memsetw
#ifdef __x86_64__
extern void __stosw(unsigned short *, unsigned short, unsigned __int64);
#define memsetw(dst, c, n)                    \
	(__stosw((unsigned short *)(void *)(dst), \
	         (unsigned short)(c),             \
	         (unsigned __int64)(n)),          \
	 (uint16_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __stosw(unsigned short *, unsigned short, unsigned int);
#define memsetw(dst, c, n)                    \
	(__stosw((unsigned short *)(void *)(dst), \
	         (unsigned short)(c),             \
	         (unsigned int)(n)),              \
	 (uint16_t *)(void *)(dst))
#endif /* !__x86_64__ */
#pragma intrinsic(__stosw)
DECL_END
#endif /* _MSC_VER && (__i386__ || __x86_64__) */
#endif /* !CONFIG_HAVE_memsetw */

#ifndef CONFIG_HAVE_memsetl
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#define CONFIG_HAVE_memsetl 1
DECL_BEGIN
#undef memsetl
#ifdef __x86_64__
extern void __stosd(unsigned long *, unsigned long, unsigned __int64);
#define memsetl(dst, c, n)                   \
	(__stosd((unsigned long *)(void *)(dst), \
	         (unsigned long)(c),             \
	         (unsigned __int64)(n)),         \
	 (uint32_t *)(void *)(dst))
#else /* __x86_64__ */
extern void __stosd(unsigned long *, unsigned long, unsigned int);
#define memsetl(dst, c, n)                   \
	(__stosd((unsigned long *)(void *)(dst), \
	         (unsigned long)(c),             \
	         (unsigned int)(n)),             \
	 (uint32_t *)(void *)(dst))
#endif /* !__x86_64__ */
#pragma intrinsic(__stosd)
DECL_END
#endif /* _MSC_VER && (__i386__ || __x86_64__) */
#endif /* !CONFIG_HAVE_memsetl */

#ifndef CONFIG_HAVE_memsetq
#if defined(_MSC_VER) && defined(__x86_64__)
#define CONFIG_HAVE_memsetq 1
DECL_BEGIN
extern void __stosq(unsigned long long *, unsigned long long, unsigned __int64);
#undef memsetq
#define memsetq(dst, c, n)                        \
	(__stosq((unsigned long long *)(void *)(dst), \
	         (unsigned long long)(c),             \
	         (unsigned __int64)(n)),              \
	 (uint64_t *)(void *)(dst))
#pragma intrinsic(__stosq)
DECL_END
#endif /* _MSC_VER && __x86_64__ */
#endif /* !CONFIG_HAVE_memsetq */

#if !defined(CONFIG_HAVE_bzeroq) && defined(CONFIG_HAVE_memsetq)
#define CONFIG_HAVE_bzeroq 1
#undef bzeroq
#define bzeroq(dst, num_qwords) (void)memsetq(dst, 0, num_qwords)
#endif /* !CONFIG_HAVE_bzeroq && CONFIG_HAVE_memsetq */

#if !defined(CONFIG_HAVE_bzerol) && defined(CONFIG_HAVE_memsetl)
#define CONFIG_HAVE_bzerol 1
#undef bzerol
#define bzerol(dst, num_dwords) (void)memsetl(dst, 0, num_dwords)
#endif /* !CONFIG_HAVE_bzerol */

#if !defined(CONFIG_HAVE_bzerow) && defined(CONFIG_HAVE_memsetw)
#define CONFIG_HAVE_bzerow 1
#undef bzerow
#define bzerow(dst, num_words) (void)memsetw(dst, 0, num_words)
#endif /* !CONFIG_HAVE_bzerow */

#ifndef CONFIG_HAVE_bzero
#define CONFIG_HAVE_bzero 1
#undef bzero
#define bzero(dst, num_bytes) (void)memset(dst, 0, num_bytes)
#endif /* !CONFIG_HAVE_bzero */

#ifndef CONFIG_HAVE_bzeroc
#define CONFIG_HAVE_bzeroc 1
#undef bzeroc
#if defined(CONFIG_HAVE_bzerow) && defined(CONFIG_HAVE_bzerol) && defined(CONFIG_HAVE_bzeroq)
#define bzeroc(dst, elem_count, elem_size) \
	((elem_size) == 2                      \
	 ? bzerow(dst, elem_count)             \
	 : (elem_size) == 4                    \
	   ? bzerol(dst, elem_count)           \
	   : (elem_size) == 8                  \
	     ? bzeroq(dst, elem_count)         \
	     : bzero(dst, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_bzerow) && defined(CONFIG_HAVE_bzerol)
#define bzeroc(dst, elem_count, elem_size) \
	((elem_size) == 2                      \
	 ? bzerow(dst, elem_count)             \
	 : (elem_size) == 4                    \
	   ? bzerol(dst, elem_count)           \
	   : bzero(dst, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_bzerow) && defined(CONFIG_HAVE_bzeroq)
#define bzeroc(dst, elem_count, elem_size) \
	((elem_size) == 2                      \
	 ? bzerow(dst, elem_count)             \
	 : (elem_size) == 8                    \
	   ? bzeroq(dst, elem_count)           \
	   : bzero(dst, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_bzerol) && defined(CONFIG_HAVE_bzeroq)
#define bzeroc(dst, elem_count, elem_size) \
	((elem_size) == 4                      \
	 ? bzerol(dst, elem_count)             \
	 : (elem_size) == 8                    \
	   ? bzeroq(dst, elem_count)           \
	   : bzero(dst, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_bzeroq)
#define bzeroc(dst, elem_count, elem_size) \
	((elem_size) == 8                      \
	 ? bzeroq(dst, elem_count)             \
	 : bzero(dst, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_bzerol)
#define bzeroc(dst, elem_count, elem_size) \
	((elem_size) == 4                      \
	 ? bzerol(dst, elem_count)             \
	 : bzero(dst, (size_t)(elem_count) * (size_t)(elem_size)))
#elif defined(CONFIG_HAVE_bzerow)
#define bzeroc(dst, elem_count, elem_size) \
	((elem_size) == 2                      \
	 ? bzerow(dst, elem_count)             \
	 : bzero(dst, (size_t)(elem_count) * (size_t)(elem_size)))
#else /* ... */
#define bzeroc(dst, elem_count, elem_size) \
	bzero(dst, (size_t)(elem_count) * (size_t)(elem_size))
#endif /* !... */
#endif /* !CONFIG_HAVE_bzeroc */


#ifndef CONFIG_HAVE_bzeroq
#define CONFIG_HAVE_bzeroq 1
#undef bzeroq
#ifdef CONFIG_HAVE_bzerol
#define bzeroq(dst, num_qwords) bzerol(dst, (num_qwords) * 2)
#elif defined(CONFIG_HAVE_memsetl)
#define bzeroq(dst, num_qwords) (void)memsetl(dst, 0, (num_qwords) * 2)
#elif defined(CONFIG_HAVE_bzerow)
#define bzeroq(dst, num_qwords) bzerow(dst, (num_qwords) * 4)
#elif defined(CONFIG_HAVE_memsetw)
#define bzeroq(dst, num_qwords) (void)memsetw(dst, 0, (num_qwords) * 4)
#else /* ... */
#define bzeroq(dst, num_qwords) bzero(dst, (num_qwords) * 8)
#endif /* !... */
#endif /* !CONFIG_HAVE_bzeroq */

#ifndef CONFIG_HAVE_bzerol
#define CONFIG_HAVE_bzerol 1
#undef bzerol
#ifdef CONFIG_HAVE_bzerow
#define bzerol(dst, num_dwords) bzerow(dst, (num_dwords) * 2)
#elif defined(CONFIG_HAVE_memsetw)
#define bzerol(dst, num_dwords) (void)memsetw(dst, 0, (num_dwords) * 2)
#else /* ... */
#define bzerol(dst, num_dwords) bzero(dst, (num_dwords) * 4)
#endif /* !... */
#endif /* !CONFIG_HAVE_bzerol */

#ifndef CONFIG_HAVE_bzerow
#define CONFIG_HAVE_bzerow 1
#undef bzerow
#define bzerow(dst, num_words) bzero(dst, (num_words) * 2)
#endif /* !CONFIG_HAVE_bzerow */

/* NOTE: `memsetp' is enabled on a per-file basis by writing:
 * >> #ifndef CONFIG_HAVE_memsetp
 * >> #define memsetp(dst, pointer, num_pointers) \
 * >> 	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
 * >> DeeSystem_DEFINE_memsetp(dee_memsetp)
 * >> #endif // !CONFIG_HAVE_memsetp
 */
#undef memsetp
#undef CONFIG_HAVE_memsetp
#if __SIZEOF_POINTER__ == 4
#ifdef CONFIG_HAVE_memsetl
#define CONFIG_HAVE_memsetp 1
#define memsetp(dst, pointer, num_pointers) \
	memsetl(dst, (uint32_t)(pointer), num_pointers)
#else /* CONFIG_HAVE_memsetl */
#define DeeSystem_DEFINE_memsetp DeeSystem_DEFINE_memsetl
#endif /* !CONFIG_HAVE_memsetl */
#elif __SIZEOF_POINTER__ == 8
#ifdef CONFIG_HAVE_memsetq
#define CONFIG_HAVE_memsetp 1
#define memsetp(dst, pointer, num_pointers) \
	memsetq(dst, (uint64_t)(pointer), num_pointers)
#else /* CONFIG_HAVE_memsetq */
#define DeeSystem_DEFINE_memsetp DeeSystem_DEFINE_memsetq
#endif /* !CONFIG_HAVE_memsetq */
#elif __SIZEOF_POINTER__ == 2
#ifdef CONFIG_HAVE_memsetw
#define CONFIG_HAVE_memsetp 1
#define memsetp(dst, pointer, num_pointers) \
	memsetw(dst, (uint16_t)(pointer), num_pointers)
#else /* CONFIG_HAVE_memsetw */
#define DeeSystem_DEFINE_memsetp DeeSystem_DEFINE_memsetw
#endif /* !CONFIG_HAVE_memsetw */
#elif __SIZEOF_POINTER__ == 1
#define CONFIG_HAVE_memsetp 1
#define memsetp(dst, pointer, num_pointers) \
	memset(dst, (int)(unsigned int)(__UINT8_TYPE__)(pointer), num_pointers)
#elif !defined(__DEEMON__)
#error "Unsupported __SIZEOF_POINTER__"
#endif




/* Single-byte variants of multi-byte string functions */
#undef memcpyb
#undef memmoveb
#undef memmoveupb
#undef memmovedownb
#undef memsetb
#undef memchrb
#undef bzerob
#define memcpyb(dst, src, n)      (uint8_t *)memcpy(dst, src, n)
#define memmoveb(dst, src, n)     (uint8_t *)memmove(dst, src, n)
#define memmoveupb(dst, src, n)   (uint8_t *)memmoveup(dst, src, n)
#define memmovedownb(dst, src, n) (uint8_t *)memmovedown(dst, src, n)
#define memsetb(p, c, n)          (uint8_t *)memset(p, c, n)
#define memchrb(p, c, n)          (uint8_t *)memchr(p, c, n)
#define memcmpb(s1, s2, n)        (int8_t)memcmp(s1, s2, n)
#define bzerob(dst, num_bytes)    bzero(dst, num_bytes)


#undef memcpyp
#undef memmovep
#undef memmoveupp
#undef memmovedownp
#if __SIZEOF_POINTER__ == 4
#define memcpyp      memcpyl
#define memmovep     memmovel
#define memmoveupp   memmoveupl
#define memmovedownp memmovedownl
#elif __SIZEOF_POINTER__ == 8
#define memcpyp      memcpyq
#define memmovep     memmoveq
#define memmoveupp   memmoveupq
#define memmovedownp memmovedownq
#elif __SIZEOF_POINTER__ == 2
#define memcpyp      memcpyw
#define memmovep     memmovew
#define memmoveupp   memmoveupw
#define memmovedownp memmovedownw
#elif __SIZEOF_POINTER__ == 1
#define memcpyp      memcpyb
#define memmovep     memmoveb
#define memmoveupp   memmoveupb
#define memmovedownp memmovedownb
#else /* __SIZEOF_POINTER__ == ... */
#define memcpyp(dst, src, num_pointers)      memcpyc(dst, src, num_pointers, __SIZEOF_POINTER__)
#define memmovep(dst, src, num_pointers)     memmovec(dst, src, num_pointers, __SIZEOF_POINTER__)
#define memmoveupp(dst, src, num_pointers)   memmoveupc(dst, src, num_pointers, __SIZEOF_POINTER__)
#define memmovedownp(dst, src, num_pointers) memmovedownc(dst, src, num_pointers, __SIZEOF_POINTER__)
#endif /* __SIZEOF_POINTER__ != ... */


//func("memsetw", "defined(__USE_KOS)", test: "extern char *a; memsetw(a, 0xcc, 16); return 0;");
//func("memsetl", "defined(__USE_KOS)", test: "extern char *a; memsetl(a, 0xcc, 16); return 0;");
//func("memsetq", "defined(__USE_KOS)", test: "extern char *a; memsetq(a, 0xcc, 16); return 0;");




/* errno classification helper macros */

/*[[[deemon
import * from deemon;
import * from util;
local n = 4;

local errno_names = {
	"EACCES",
	"EBADF",
	"EBUSY",
	"EEXIST",
	"EFBIG",
	"EINVAL",
	"EISDIR",
	"ELOOP",
	"ENAMETOOLONG",
	"ENOENT",
	"ENOMEM",
	"ENOSYS",
	"ENOTDIR",
	"ENOTSUP",
	"ENOTTY",
	"ENXIO",
	"EOPNOTSUPP",
	"EPERM",
	"EROFS",
	"ETXTBSY",
	"ENOTEMPTY",
	"EXDEV",
	"ENOLINK",
	"EINTR",
};

for (local x: errno_names) {
	print "#ifdef", x;
	print "#define DeePrivateSystem_IF_HAVE_",;
	print x,;
	print "(tt, ff) tt";
	print "#else /" "*", x, "*" "/";
	print "#define DeePrivateSystem_IF_HAVE_",;
	print x,;
	print "(tt, ff) ff";
	print "#endif /" "* !",;
	print x, "*" "/";
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
	fp << "	}	__WHILE0";
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
#ifdef EACCES
#define DeePrivateSystem_IF_HAVE_EACCES(tt, ff) tt
#else /* EACCES */
#define DeePrivateSystem_IF_HAVE_EACCES(tt, ff) ff
#endif /* !EACCES */
#ifdef EBADF
#define DeePrivateSystem_IF_HAVE_EBADF(tt, ff) tt
#else /* EBADF */
#define DeePrivateSystem_IF_HAVE_EBADF(tt, ff) ff
#endif /* !EBADF */
#ifdef EBUSY
#define DeePrivateSystem_IF_HAVE_EBUSY(tt, ff) tt
#else /* EBUSY */
#define DeePrivateSystem_IF_HAVE_EBUSY(tt, ff) ff
#endif /* !EBUSY */
#ifdef EEXIST
#define DeePrivateSystem_IF_HAVE_EEXIST(tt, ff) tt
#else /* EEXIST */
#define DeePrivateSystem_IF_HAVE_EEXIST(tt, ff) ff
#endif /* !EEXIST */
#ifdef EFBIG
#define DeePrivateSystem_IF_HAVE_EFBIG(tt, ff) tt
#else /* EFBIG */
#define DeePrivateSystem_IF_HAVE_EFBIG(tt, ff) ff
#endif /* !EFBIG */
#ifdef EINVAL
#define DeePrivateSystem_IF_HAVE_EINVAL(tt, ff) tt
#else /* EINVAL */
#define DeePrivateSystem_IF_HAVE_EINVAL(tt, ff) ff
#endif /* !EINVAL */
#ifdef EISDIR
#define DeePrivateSystem_IF_HAVE_EISDIR(tt, ff) tt
#else /* EISDIR */
#define DeePrivateSystem_IF_HAVE_EISDIR(tt, ff) ff
#endif /* !EISDIR */
#ifdef ELOOP
#define DeePrivateSystem_IF_HAVE_ELOOP(tt, ff) tt
#else /* ELOOP */
#define DeePrivateSystem_IF_HAVE_ELOOP(tt, ff) ff
#endif /* !ELOOP */
#ifdef ENAMETOOLONG
#define DeePrivateSystem_IF_HAVE_ENAMETOOLONG(tt, ff) tt
#else /* ENAMETOOLONG */
#define DeePrivateSystem_IF_HAVE_ENAMETOOLONG(tt, ff) ff
#endif /* !ENAMETOOLONG */
#ifdef ENOENT
#define DeePrivateSystem_IF_HAVE_ENOENT(tt, ff) tt
#else /* ENOENT */
#define DeePrivateSystem_IF_HAVE_ENOENT(tt, ff) ff
#endif /* !ENOENT */
#ifdef ENOMEM
#define DeePrivateSystem_IF_HAVE_ENOMEM(tt, ff) tt
#else /* ENOMEM */
#define DeePrivateSystem_IF_HAVE_ENOMEM(tt, ff) ff
#endif /* !ENOMEM */
#ifdef ENOSYS
#define DeePrivateSystem_IF_HAVE_ENOSYS(tt, ff) tt
#else /* ENOSYS */
#define DeePrivateSystem_IF_HAVE_ENOSYS(tt, ff) ff
#endif /* !ENOSYS */
#ifdef ENOTDIR
#define DeePrivateSystem_IF_HAVE_ENOTDIR(tt, ff) tt
#else /* ENOTDIR */
#define DeePrivateSystem_IF_HAVE_ENOTDIR(tt, ff) ff
#endif /* !ENOTDIR */
#ifdef ENOTSUP
#define DeePrivateSystem_IF_HAVE_ENOTSUP(tt, ff) tt
#else /* ENOTSUP */
#define DeePrivateSystem_IF_HAVE_ENOTSUP(tt, ff) ff
#endif /* !ENOTSUP */
#ifdef ENOTTY
#define DeePrivateSystem_IF_HAVE_ENOTTY(tt, ff) tt
#else /* ENOTTY */
#define DeePrivateSystem_IF_HAVE_ENOTTY(tt, ff) ff
#endif /* !ENOTTY */
#ifdef ENXIO
#define DeePrivateSystem_IF_HAVE_ENXIO(tt, ff) tt
#else /* ENXIO */
#define DeePrivateSystem_IF_HAVE_ENXIO(tt, ff) ff
#endif /* !ENXIO */
#ifdef EOPNOTSUPP
#define DeePrivateSystem_IF_HAVE_EOPNOTSUPP(tt, ff) tt
#else /* EOPNOTSUPP */
#define DeePrivateSystem_IF_HAVE_EOPNOTSUPP(tt, ff) ff
#endif /* !EOPNOTSUPP */
#ifdef EPERM
#define DeePrivateSystem_IF_HAVE_EPERM(tt, ff) tt
#else /* EPERM */
#define DeePrivateSystem_IF_HAVE_EPERM(tt, ff) ff
#endif /* !EPERM */
#ifdef EROFS
#define DeePrivateSystem_IF_HAVE_EROFS(tt, ff) tt
#else /* EROFS */
#define DeePrivateSystem_IF_HAVE_EROFS(tt, ff) ff
#endif /* !EROFS */
#ifdef ETXTBSY
#define DeePrivateSystem_IF_HAVE_ETXTBSY(tt, ff) tt
#else /* ETXTBSY */
#define DeePrivateSystem_IF_HAVE_ETXTBSY(tt, ff) ff
#endif /* !ETXTBSY */
#ifdef ENOTEMPTY
#define DeePrivateSystem_IF_HAVE_ENOTEMPTY(tt, ff) tt
#else /* ENOTEMPTY */
#define DeePrivateSystem_IF_HAVE_ENOTEMPTY(tt, ff) ff
#endif /* !ENOTEMPTY */
#ifdef EXDEV
#define DeePrivateSystem_IF_HAVE_EXDEV(tt, ff) tt
#else /* EXDEV */
#define DeePrivateSystem_IF_HAVE_EXDEV(tt, ff) ff
#endif /* !EXDEV */
#ifdef ENOLINK
#define DeePrivateSystem_IF_HAVE_ENOLINK(tt, ff) tt
#else /* ENOLINK */
#define DeePrivateSystem_IF_HAVE_ENOLINK(tt, ff) ff
#endif /* !ENOLINK */
#ifdef EINTR
#define DeePrivateSystem_IF_HAVE_EINTR(tt, ff) tt
#else /* EINTR */
#define DeePrivateSystem_IF_HAVE_EINTR(tt, ff) ff
#endif /* !EINTR */
#define DeePrivateSystem_IF_E1(errno, e1, ...) \
	do {                                       \
		if ((errno) == e1) {                   \
			__VA_ARGS__;                       \
		}                                      \
	}	__WHILE0
#define DeePrivateSystem_IF_E2(errno, e1, e2, ...) \
	do {                                           \
		if ((errno) == e1 || (errno) == e2) {      \
			__VA_ARGS__;                           \
		}                                          \
	}	__WHILE0
#define DeePrivateSystem_IF_E3(errno, e1, e2, e3, ...)         \
	do {                                                       \
		if ((errno) == e1 || (errno) == e2 || (errno) == e3) { \
			__VA_ARGS__;                                       \
		}                                                      \
	}	__WHILE0
#define DeePrivateSystem_IF_E4(errno, e1, e2, e3, e4, ...)                      \
	do {                                                                        \
		if ((errno) == e1 || (errno) == e2 || (errno) == e3 || (errno) == e4) { \
			__VA_ARGS__;                                                        \
		}                                                                       \
	}	__WHILE0
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


#ifdef GUARD_DEEMON_FILE_H
#if (defined(DeeSysFS_IS_INT) && !defined(CONFIG_HAVE_close)) || \
    (defined(DeeSysFS_IS_FILE) && !defined(CONFIG_HAVE_fclose))
#undef DeeSysFD_Close
#define DeeSysFD_Close(x) (void)0
#endif /* !CONFIG_HAVE_close */
#endif /* GUARD_DEEMON_FILE_H */


#ifdef CONFIG_BUILDING_DEEMON
/* Figure out how to implement the shared library system API */
#undef DeeSystem_DlOpen_USE_LOADLIBRARY
#undef DeeSystem_DlOpen_USE_DLFCN
#undef DeeSystem_DlOpen_USE_STUB
#if defined(CONFIG_HAVE_dlopen) && defined(CONFIG_HAVE_dlsym)
#define DeeSystem_DlOpen_USE_DLFCN 1
#elif defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_DlOpen_USE_LOADLIBRARY 1
#else /* ... */
#define DeeSystem_DlOpen_USE_STUB 1
#endif /* !... */
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_SYSTEM_FEATURES_H */
