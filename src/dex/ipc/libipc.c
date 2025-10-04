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
#ifndef GUARD_DEX_IPC_LIBIPC_C
#define GUARD_DEX_IPC_LIBIPC_C 1
#define DEE_SOURCE

#include "libipc.h"

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/objmethod.h>
#include <deemon/system.h>

#ifndef __INTELLISENSE__
#include "pipe.c.inl"
#include "process.c.inl"
#else /* !__INTELLISENSE__ */
#define WANT_DeeObject_AsFileSystemPathString
#define WANT_ipc_unimplemented
#define WANT_err_file_not_found
DECL_BEGIN
PRIVATE WUNUSED DREF DeeStringObject *DCALL process_get_shell(void);
DECL_END
#endif /* __INTELLISENSE__ */

DECL_BEGIN


#ifdef WANT_DeeObject_AsFileSystemPathString
#undef WANT_DeeObject_AsFileSystemPathString
INTERN WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
DeeObject_AsFileSystemPathString(DeeObject *__restrict self) {
	if (DeeString_Check(self)) {
		Dee_Incref(self);
		return (DREF DeeStringObject *)self;
	} else {
#ifdef CONFIG_HOST_WINDOWS
		void *hExeHandle;
		hExeHandle = DeeNTSystem_GetHandle(self);
		if unlikely(hExeHandle == (void *)-1)
			return NULL;
		return (DREF DeeStringObject *)DeeNTSystem_GetFilenameOfHandle(hExeHandle);
#else /* CONFIG_HOST_WINDOWS */
		int fd = DeeUnixSystem_GetFD(self);
		if unlikely(fd == -1)
			return NULL;
		return (DREF DeeStringObject *)DeeSystem_GetFilenameOfFD(fd);
#endif /* !CONFIG_HOST_WINDOWS */
	}
}
#endif /* WANT_DeeObject_AsFileSystemPathString */


#ifdef WANT_ipc_unimplemented
#undef WANT_ipc_unimplemented
INTERN ATTR_COLD int DCALL ipc_unimplemented(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "IPC function not supported");
}
#endif /* !WANT_ipc_unimplemented */

#ifdef WANT_err_file_not_found
#undef WANT_err_file_not_found
INTERN ATTR_COLD NONNULL((1)) int DCALL
err_file_not_found_string(char const *__restrict filename) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "File %q could not be found",
	                       filename);
}
#endif /* !WANT_err_file_not_found */

PRIVATE WUNUSED DREF DeeObject *DCALL
ipc_SHELL_get_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return (DREF DeeObject *)process_get_shell();
}
PRIVATE DEFINE_CMETHOD(ipc_SHELL_get, &ipc_SHELL_get_f, METHOD_FPURECALL);


PRIVATE struct dex_symbol symbols[] = {
	{ "Process", (DeeObject *)&DeeProcess_Type },
	{ "Pipe", (DeeObject *)&DeePipe_Type },
	{ "SHELL", (DeeObject *)&ipc_SHELL_get, MODSYM_FREADONLY | MODSYM_FPROPERTY,
	  DOC("->?Dstring\n"
	      "The absolute filename of the used system shell command interpreter program\n"
	      "When the $\"$SHELL\" environment variable is defined, this global evaluates "
	      /**/ "to that variable's contents. Otherwise (except on windows), the system "
	      /**/ "file #C{/etc/shells} is read line-by-line, and the first line containing "
	      /**/ "an absolute filename that points to an executable file is used. If that "
	      /**/ "file doesn't exist or does not specify a valid shell program, a system-"
	      /**/ "specific default is used, which is #C{cmd.exe} on windows, and #C{/bin/sh} "
	      /**/ "on all other systems.") },
	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols      = */ symbols,
	/* .d_init         = */ NULL,
#ifdef HAVE_libipc_fini
	/* .d_fini         = */ &libipc_fini,
#else /* HAVE_libipc_fini */
	/* .d_fini         = */ NULL,
#endif /* !HAVE_libipc_fini */
	/* .d_import_names = */ { NULL }
};

DECL_END

#endif /* !GUARD_DEX_IPC_LIBIPC_C */
