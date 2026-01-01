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
#ifndef GUARD_DEEMON_SYSTEM_ERROR_H
#define GUARD_DEEMON_SYSTEM_ERROR_H 1

#include "api.h"
/**/

#include "system-features.h"

/* Helper macros for saving/restoring the current:
 *   - errno
 *   - _doserrno
 *   - GetLastError()
 */
#undef DeeSystemError_HAVE_GetLastError
#undef DeeSystemError_HAVE_errno
#undef DeeSystemError_HAVE_doserrno
#ifdef CONFIG_HOST_WINDOWS
#define DeeSystemError_HAVE_GetLastError
#endif /* CONFIG_HOST_WINDOWS */
#ifdef CONFIG_HAVE_errno
#define DeeSystemError_HAVE_errno
#endif /* CONFIG_HAVE_errno */
#ifdef CONFIG_HAVE_doserrno
#define DeeSystemError_HAVE_doserrno
#endif /* CONFIG_HAVE_doserrno */

#ifdef DeeSystemError_HAVE_GetLastError
#include <Windows.h>
#endif /* DeeSystemError_HAVE_GetLastError */

DECL_BEGIN

#undef DeeSystemError_HAVE_ANY
typedef struct {
#ifdef DeeSystemError_HAVE_GetLastError
#define DeeSystemError_HAVE_ANY 1
	DWORD  se_GetLastError; /* Saved `errno' value */
#define _DeeSystemError_SAVE_GETLASTERROR(st) ((st).se_GetLastError = GetLastError())
#define _DeeSystemError_LOAD_GETLASTERROR(st) (SetLastError((st).se_GetLastError))
#else /* DeeSystemError_HAVE_GetLastError */
#define _DeeSystemError_SAVE_GETLASTERROR(st) (void)0
#define _DeeSystemError_LOAD_GETLASTERROR(st) (void)0
#endif /* !DeeSystemError_HAVE_GetLastError */

#ifdef DeeSystemError_HAVE_errno
#define DeeSystemError_HAVE_ANY 1
	int    se_errno;        /* Saved `errno' value */
#define _DeeSystemError_SAVE_ERRNO(st) ((st).se_errno = DeeSystem_GetErrno())
#define _DeeSystemError_LOAD_ERRNO(st) (DeeSystem_SetErrno((st).se_errno))
#else /* DeeSystemError_HAVE_errno */
#define _DeeSystemError_SAVE_ERRNO(st) (void)0
#define _DeeSystemError_LOAD_ERRNO(st) (void)0
#endif /* !DeeSystemError_HAVE_errno */

#ifdef DeeSystemError_HAVE_doserrno
#define DeeSystemError_HAVE_ANY 1
	int    se_doserrno;     /* Saved `doserrno' value */
#define _DeeSystemError_SAVE_DOSERRNO(st) ((st).se_doserrno = doserrno)
#define _DeeSystemError_LOAD_DOSERRNO(st) (doserrno = (st).se_doserrno)
#else /* DeeSystemError_HAVE_doserrno */
#define _DeeSystemError_SAVE_DOSERRNO(st) (void)0
#define _DeeSystemError_LOAD_DOSERRNO(st) (void)0
#endif /* !DeeSystemError_HAVE_doserrno */

#ifndef DeeSystemError_HAVE_ANY
	int _se_placeholder;
#endif /* !DeeSystemError_HAVE_ANY */
} DeeSystemError;

/* Save/Restore the current System Error state. */
#define DeeSystemError_Save(st)             \
	(_DeeSystemError_SAVE_GETLASTERROR(st), \
	 _DeeSystemError_SAVE_ERRNO(st),        \
	 _DeeSystemError_SAVE_DOSERRNO(st))
#define DeeSystemError_Load(st)             \
	(_DeeSystemError_LOAD_GETLASTERROR(st), \
	 _DeeSystemError_LOAD_ERRNO(st),        \
	 _DeeSystemError_LOAD_DOSERRNO(st))


/* Push/Pop the current system error state. */
#ifdef DeeSystemError_HAVE_ANY
#define DeeSystemError_Push()               \
	do {                                    \
		DeeSystemError _saved_syserr;       \
		DeeSystemError_Save(_saved_syserr)
#define DeeSystemError_Break()              \
		DeeSystemError_Load(_saved_syserr)
#define DeeSystemError_Pop()                \
		DeeSystemError_Load(_saved_syserr); \
	}	__WHILE0
#else /* DeeSystemError_HAVE_ANY */
#define DeeSystemError_Push()  do {
#define DeeSystemError_Break() (void)0
#define DeeSystemError_Pop()   (void)0; }	__WHILE0
#endif /* !DeeSystemError_HAVE_ANY */


DECL_END


#endif /* !GUARD_DEEMON_SYSTEM_ERROR_H */
