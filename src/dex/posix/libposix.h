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
#ifndef GUARD_DEX_POSIX_LIBPOSIX_H
#define GUARD_DEX_POSIX_LIBPOSIX_H 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/sched/yield.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

/* Imported module access. */
#define FS_MODULE   DEX.d_imports[0]


#if defined(EINTR) && !defined(__INTELLISENSE__)
#define EINTR_LABEL(again) \
	again:
#define HANDLE_EINTR(error, again, err_label) \
	if ((error) == EINTR) {                   \
		if (DeeThread_CheckInterrupt())       \
			goto err_label;                   \
		goto again;                           \
	}
#else
#define EINTR_LABEL(again)                    /* nothing */
#define HANDLE_EINTR(error, again, err_label) /* nothing */
#endif

#define HANDLE_ENOENT_ENOTDIR(error, err_label, ...)                    \
	DeeSystem_IF_E2(error, ENOENT, ENOTDIR, {                           \
		DeeError_SysThrowf(&DeeError_FileNotFound, error, __VA_ARGS__); \
		goto err_label;                                                 \
	});

#define HANDLE_ENOENT_ENOTDIR(error, err_label, ...)                    \
	DeeSystem_IF_E2(error, ENOENT, ENOTDIR, {                           \
		DeeError_SysThrowf(&DeeError_FileNotFound, error, __VA_ARGS__); \
		goto err_label;                                                 \
	});
#define HANDLE_ENXIO_EISDIR(error, err_label, ...)                      \
	DeeSystem_IF_E2(error, ENXIO, EISDIR, {                             \
		DeeError_SysThrowf(&DeeError_ReadOnlyFile, error, __VA_ARGS__); \
		goto err_label;                                                 \
	});
#define HANDLE_EROFS_ETXTBSY(error, err_label, ...)                     \
	DeeSystem_IF_E2(error, EROFS, ETXTBSY, {                            \
		DeeError_SysThrowf(&DeeError_ReadOnlyFile, error, __VA_ARGS__); \
		goto err_label;                                                 \
	});
#define HANDLE_EACCES(error, err_label, ...)                               \
	DeeSystem_IF_E1(error, EACCES, {                                       \
		DeeError_SysThrowf(&DeeError_FileAccessError, error, __VA_ARGS__); \
		goto err_label;                                                    \
	});
#define HANDLE_EEXIST_IF(error, cond, err_label, ...)                          \
	DeeSystem_IF_E1(error, EEXIST, {                                           \
		if (cond) {                                                            \
			DeeError_SysThrowf(&DeeError_FileAccessError, error, __VA_ARGS__); \
			goto err_label;                                                    \
		}                                                                      \
	});
#define HANDLE_EINVAL(error, err_label, ...)                \
	DeeSystem_IF_E1(error, EINVAL, {                        \
		DeeError_Throwf(&DeeError_ValueError, __VA_ARGS__); \
		goto err_label;                                     \
	});
#define HANDLE_ENOMEM(error, err_label, ...)              \
	DeeSystem_IF_E1(error, ENOMEM, {                      \
		DeeError_Throwf(&DeeError_NoMemory, __VA_ARGS__); \
		goto err_label;                                   \
	});
#define HANDLE_EBADF(error, err_label, ...)                 \
	DeeSystem_IF_E1(error, EBADF, {                         \
		DeeError_Throwf(&DeeError_FileClosed, __VA_ARGS__); \
		goto err_label;                                     \
	});
#define HANDLE_EFBIG_EINVAL(error, err_label, ...)               \
	DeeSystem_IF_E2(error, EFBIG, EINVAL, {                      \
		DeeError_Throwf(&DeeError_IntegerOverflow, __VA_ARGS__); \
		goto err_label;                                          \
	});
#define HANDLE_ENOSYS(error, err_label, name)             \
	DeeSystem_IF_E3(error, ENOSYS, ENOTSUP, EOPNOTSUPP, { \
		posix_err_unsupported(name);                   \
		goto err_label;                                   \
	});


PRIVATE ATTR_NOINLINE ATTR_UNUSED ATTR_COLD int DCALL
posix_err_unsupported(char const *__restrict name);
#undef NEED_ERR_UNSUPPORTED

PRIVATE WUNUSED DREF /*String*/ DeeObject *DCALL
libposix_get_dfd_filename(int dfd, /*utf-8*/ char const *filename, int atflags);
#undef NEED_GET_DFD_FILENAME

#if defined(ENOSYS) || defined(ENOTSUP) || defined(EOPNOTSUPP)
#define NEED_ERR_UNSUPPORTED 1
#endif /* ENOSYS || ENOTSUP || EOPNOTSUPP */

DECL_END

#endif /* !GUARD_DEX_POSIX_LIBPOSIX_H */
