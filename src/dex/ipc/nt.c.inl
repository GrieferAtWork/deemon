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
#ifndef GUARD_DEX_IPC_NT_C_INL
#define GUARD_DEX_IPC_NT_C_INL 1
#define DEE_SOURCE

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h> /* DeeNTSystem_IsIntr() */
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <Windows.h>

#include "libipc.h"
//#include <Winternl.h>

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else /* ... */
#   define PATH_MAX 260
#endif /* !... */
#endif /* !PATH_MAX */


DECL_BEGIN

/* Structures are taken from `https://www.nirsoft.net/kernel_struct/vista/RTL_USER_PROCESS_PARAMETERS.html' */

#ifndef _MSC_VER
#undef PVOID64
typedef uint64_t fixed_PVOID64;
#define PVOID64 fixed_PVOID64
#endif /* !_MSC_VER */

#if __SIZEOF_POINTER__ == 4
typedef void    *PVOID32;
typedef WCHAR   *NWPSTR32, *LPWSTR32, *PWSTR32;
typedef PVOID64  NWPSTR64,  LPWSTR64,  PWSTR64;
typedef CHAR    *PCHAR32,  *LPCH32,   *PCH32;
typedef PVOID64  PCHAR64,   LPCH64,    PCH64;
#else /* __SIZEOF_POINTER__ == 4 */
typedef uint32_t PVOID32;
typedef PVOID32  NWPSTR32,  LPWSTR32,  PWSTR32;
typedef WCHAR   *NWPSTR64, *LPWSTR64, *PWSTR64;
typedef PVOID32  PCHAR32,   LPCH32,    PCH32;
typedef CHAR    *PCHAR64,  *LPCH64,   *PCH64;
#endif /* __SIZEOF_POINTER__ != 4 */

typedef struct _UNICODE_STRING32 {
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR32 Buffer;
} UNICODE_STRING32;
typedef UNICODE_STRING32 *PUNICODE_STRING32;
typedef const UNICODE_STRING32 *PCUNICODE_STRING32;

typedef struct _UNICODE_STRING64 {
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR64 Buffer;
} UNICODE_STRING64;
typedef UNICODE_STRING64 *PUNICODE_STRING64;
typedef const UNICODE_STRING64 *PCUNICODE_STRING64;


typedef struct _CURDIR32 {
	UNICODE_STRING32 DosPath;
	PVOID32          Handle;
} CURDIR32, *PCURDIR32;
typedef struct _CURDIR64 {
	UNICODE_STRING64 DosPath;
	PVOID64          Handle;
} CURDIR64, *PCURDIR64;

typedef struct _STRING32 {
	USHORT  Length;
	USHORT  MaximumLength;
	PCHAR32 Buffer;
} STRING32;
typedef STRING32 *PSTRING32;
typedef struct _STRING64 {
	USHORT  Length;
	USHORT  MaximumLength;
	PCHAR64 Buffer;
} STRING64;
typedef STRING64 *PSTRING64;

typedef struct _RTL_DRIVE_LETTER_CURDIR32 {
	WORD     Flags;
	WORD     Length;
	ULONG    TimeStamp;
	STRING32 DosPath;
} RTL_DRIVE_LETTER_CURDIR32, *PRTL_DRIVE_LETTER_CURDIR32;
typedef struct _RTL_DRIVE_LETTER_CURDIR64 {
	WORD     Flags;
	WORD     Length;
	ULONG    TimeStamp;
	STRING64 DosPath;
} RTL_DRIVE_LETTER_CURDIR64, *PRTL_DRIVE_LETTER_CURDIR64;


typedef struct _RTL_USER_PROCESS_PARAMETERS32 {
	ULONG                     MaximumLength;
	ULONG                     Length;
	ULONG                     Flags;
	ULONG                     DebugFlags;
	PVOID32                   ConsoleHandle;
	ULONG                     ConsoleFlags;
	PVOID32                   StandardInput;
	PVOID32                   StandardOutput;
	PVOID32                   StandardError;
	CURDIR32                  CurrentDirectory;
	UNICODE_STRING32          DllPath;
	UNICODE_STRING32          ImagePathName;
	UNICODE_STRING32          CommandLine;
	PVOID32                   Environment;
	ULONG                     StartingX;
	ULONG                     StartingY;
	ULONG                     CountX;
	ULONG                     CountY;
	ULONG                     CountCharsX;
	ULONG                     CountCharsY;
	ULONG                     FillAttribute;
	ULONG                     WindowFlags;
	ULONG                     ShowWindowFlags;
	UNICODE_STRING32          WindowTitle;
	UNICODE_STRING32          DesktopInfo;
	UNICODE_STRING32          ShellInfo;
	UNICODE_STRING32          RuntimeData;
	RTL_DRIVE_LETTER_CURDIR32 CurrentDirectores[32];
	ULONG                     EnvironmentSize;
} RTL_USER_PROCESS_PARAMETERS32, *PRTL_USER_PROCESS_PARAMETERS32;



typedef struct _RTL_USER_PROCESS_PARAMETERS64 {
	ULONG                     MaximumLength;
	ULONG                     Length;
	ULONG                     Flags;
	ULONG                     DebugFlags;
	PVOID64                   ConsoleHandle;
	ULONG                     ConsoleFlags;
	PVOID64                   StandardInput;
	PVOID64                   StandardOutput;
	PVOID64                   StandardError;
	CURDIR64                  CurrentDirectory;
	UNICODE_STRING64          DllPath;
	UNICODE_STRING64          ImagePathName;
	UNICODE_STRING64          CommandLine;
	PVOID64                   Environment;
	ULONG                     StartingX;
	ULONG                     StartingY;
	ULONG                     CountX;
	ULONG                     CountY;
	ULONG                     CountCharsX;
	ULONG                     CountCharsY;
	ULONG                     FillAttribute;
	ULONG                     WindowFlags;
	ULONG                     ShowWindowFlags;
	UNICODE_STRING64          WindowTitle;
	UNICODE_STRING64          DesktopInfo;
	UNICODE_STRING64          ShellInfo;
	UNICODE_STRING64          RuntimeData;
	RTL_DRIVE_LETTER_CURDIR64 CurrentDirectores[32];
	ULONG                     EnvironmentSize;
} RTL_USER_PROCESS_PARAMETERS64, *PRTL_USER_PROCESS_PARAMETERS64;

/* Assert known, good offsets. */
STATIC_ASSERT(offsetof(RTL_USER_PROCESS_PARAMETERS32, CurrentDirectory) == 36);
STATIC_ASSERT(offsetof(RTL_USER_PROCESS_PARAMETERS64, CurrentDirectory) == 56);
STATIC_ASSERT(offsetof(RTL_USER_PROCESS_PARAMETERS32, CommandLine)      == 64);
STATIC_ASSERT(offsetof(RTL_USER_PROCESS_PARAMETERS64, CommandLine)      == 112);


#define PROCATTR_STANDARDINPUT         0 /* DREF DeeSystemFileObject * */
#define PROCATTR_STANDARDOUTPUT        1 /* DREF DeeSystemFileObject * */
#define PROCATTR_STANDARDERROR         2 /* DREF DeeSystemFileObject * */
#define PROCATTR_CURRENTDIRECTORY      3 /* DREF DeeStringObject * */
#define PROCATTR_DLLPATH               4 /* DREF DeeStringObject * */
#define PROCATTR_IMAGEPATHNAME         5 /* DREF DeeStringObject * */
#define PROCATTR_COMMANDLINE           6 /* DREF DeeStringObject * */
#define PROCATTR_ENVIRONMENT           7 /* DREF DeeSequenceObject * -- {(string, string)...} */

/* Read the attribute `dwAttributeId' (One of `PROCATTR_*')
 * for the given process `*lphProcess' with id `dwProcessId'.
 * @param: lphProcess:     [in|out] A handle to the process (may be replaced by a different handle, though the old handle is not closed)
 * @param: dwProcessId:     The ID of the process in question.
 * @param: dwAttributeType: The attribute that should be accessed (One of `PROCATTR_*').
 * @return: * :             An object encapsulating the value referred to by `dwAttributeId'
 * @return: NULL:           An error occurred. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nt_GetProcessAttribute(HANDLE *__restrict lphProcess,
                       DWORD dwProcessId,
                       DWORD dwAttributeId) {
	/* TODO */
	(void)lphProcess;
	(void)dwProcessId;
	(void)dwAttributeId;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}


DECL_END

#endif /* !GUARD_DEX_IPC_NT_C_INL */
