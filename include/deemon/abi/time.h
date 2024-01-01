/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_ABI_TIME_H
#define GUARD_DEEMON_ABI_TIME_H 1

#include "../api.h"
/**/

#include "../format.h" /* DEE_PCKuPTR */
#include "../module.h"
#include "../object.h"
/**/

#include <hybrid/__unaligned.h>

/* Helpers for interfacing with Time-like objects, as defined by the `time' module */

DECL_BEGIN

#ifndef DeeTime_C_SYMBOL_NAME__DeeTime_NewUnix
#define DeeTime_C_SYMBOL_NAME__DeeTime_NewUnix     "DeeTime_NewUnix"
#define DeeTime_C_SYMBOL_NAME__DeeTime_NewFILETIME "DeeTime_NewFILETIME"
#endif /* !DeeTime_C_SYMBOL_NAME__DeeTime_NewUnix */

#define DECLARE_DeeTime_NewUnix()                     \
	WUNUSED DREF DeeObject *DCALL                     \
	DeeTime_NewUnix(int64_t seconds_since_01_01_1970, \
	                uint32_t extra_nanoseconds)

#define DEFINE_DeeTime_NewUnix(get_time_module)                               \
	WUNUSED DREF DeeObject *DCALL                                             \
	DeeTime_NewUnix(int64_t seconds_since_01_01_1970,                         \
	                uint32_t extra_nanoseconds) {                             \
		typedef WUNUSED_T DREF DeeObject *                                    \
		(DCALL *PDEETIME_NEWUNIX)(int64_t seconds_since_01_01_1970,           \
		                          uint32_t extra_nanoseconds);                \
		static PDEETIME_NEWUNIX _c_DeeTime_NewUnix = NULL;                    \
		if (*(void **)&_c_DeeTime_NewUnix != (void *)-1) {                    \
			if (_c_DeeTime_NewUnix == NULL) {                                 \
				DeeObject *time_module = get_time_module();                   \
				if unlikely(time_module == NULL)                              \
					return NULL;                                              \
				*(void **)&_c_DeeTime_NewUnix = DeeModule_GetNativeSymbol(    \
						time_module, DeeTime_C_SYMBOL_NAME__DeeTime_NewUnix); \
				if (_c_DeeTime_NewUnix == NULL) {                             \
					*(void **)&_c_DeeTime_NewUnix = (void *)-1;               \
					goto fallback;                                            \
				}                                                             \
			}                                                                 \
			return (*_c_DeeTime_NewUnix)(seconds_since_01_01_1970,            \
			                             extra_nanoseconds);                  \
		} else {                                                              \
			DeeObject *time_module;                                           \
	fallback:                                                                 \
			time_module = get_time_module();                                  \
			if unlikely(time_module == NULL)                                  \
				return NULL;                                                  \
			return DeeObject_CallAttrStringf(time_module, "_mkunix",          \
			                                 DEE_PCKd64 DEE_PCKu32,           \
			                                 seconds_since_01_01_1970,        \
			                                 extra_nanoseconds);              \
		}                                                                     \
	}

#define DECLARE_DeeTime_NewFILETIME()          \
	WUNUSED NONNULL((1)) DREF DeeObject *DCALL \
	DeeTime_NewFILETIME(void const *p_filetime)

#define DEFINE_DeeTime_NewFILETIME(get_time_module)                                  \
	WUNUSED NONNULL((1)) DREF DeeObject *DCALL                                       \
	DeeTime_NewFILETIME(void const *p_filetime) {                                    \
		typedef WUNUSED_T DREF DeeObject *                                           \
		(DCALL *PDEETIME_NEWFILETIME)(void const *p_filetime);                       \
		static PDEETIME_NEWFILETIME _c_DeeTime_NewFILETIME = NULL;                   \
		if (*(void **)&_c_DeeTime_NewFILETIME != (void *)-1) {                       \
			if (_c_DeeTime_NewFILETIME == NULL) {                                    \
				DeeObject *time_module = get_time_module();                          \
				if unlikely(time_module == NULL)                                     \
					return NULL;                                                     \
				*(void **)&_c_DeeTime_NewFILETIME = DeeModule_GetNativeSymbol(       \
						time_module, DeeTime_C_SYMBOL_NAME__DeeTime_NewFILETIME);    \
				if (_c_DeeTime_NewFILETIME == NULL) {                                \
					*(void **)&_c_DeeTime_NewFILETIME = (void *)-1;                  \
					goto fallback;                                                   \
				}                                                                    \
			}                                                                        \
			return (*_c_DeeTime_NewFILETIME)(p_filetime);                            \
		} else {                                                                     \
			DeeObject *time_module;                                                  \
	fallback:                                                                        \
			time_module = get_time_module();                                         \
			if unlikely(time_module == NULL)                                         \
				return NULL;                                                         \
			return DeeObject_CallAttrStringf(time_module, "_mkFILETIME", DEE_PCKu64, \
			                                 __hybrid_unaligned_get64(p_filetime));  \
		}                                                                            \
	}


DECL_END

#endif /* !GUARD_DEEMON_ABI_TIME_H */
