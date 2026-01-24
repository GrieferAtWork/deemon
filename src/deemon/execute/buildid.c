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
#ifndef GUARD_DEEMON_EXECUTE_BUILDID_C
#define GUARD_DEEMON_EXECUTE_BUILDID_C 1

#include <deemon/api.h>

#include <deemon/dex.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/types.h>
#include <deemon/util/atomic.h>     /* atomic_or, atomic_read */

#include <hybrid/host.h>     /* __ARCH_PAGESIZE */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include <stddef.h> /* NULL */
#include <stdint.h> /* UINT16_C, UINT32_C, UINT64_C, uint32_t, uint64_t, uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* @return: 0 : File not found?
 * @return: (uint64_t)-1: Error */
PRIVATE WUNUSED NONNULL((1)) uint64_t DCALL
DeeModule_GetFileLastModified(DeeModuleObject *__restrict self) {
	uint64_t result;
	DREF DeeObject *filename = DeeModule_GetFileName(self);
	if unlikely(!ITER_ISOK(filename))
		return filename == NULL ? (uint64_t)-1 : 0;
	result = DeeSystem_GetLastModified(filename);
	Dee_Decref_likely(filename);
	return result;
}


#ifdef CONFIG_NO_DEX
#ifdef CONFIG_HAVE___dex_buildid__
INTDEF __BYTE_TYPE__ __dex_buildid__[];
#define HAVE_DeeModule_GetBuildId_ofcore
PRIVATE ATTR_RETNONNULL WUNUSED union Dee_module_buildid const *DCALL
DeeModule_GetBuildId_ofcore(void) {
	memcpy(&DeeModule_Deemon.mo_buildid, __dex_buildid__ + 16, 16);
	return &DeeModule_Deemon.mo_buildid;
}
#elif defined(CONFIG_HAVE___dex_builduuid64__)
INTDEF __BYTE_TYPE__ __dex_builduuid64_0__[];
INTDEF __BYTE_TYPE__ __dex_builduuid64_1__[];
#define HAVE_DeeModule_GetBuildId_ofcore
PRIVATE ATTR_RETNONNULL WUNUSED union Dee_module_buildid const *DCALL
DeeModule_GetBuildId_ofcore(void) {
	DeeModule_Deemon.mo_buildid.mbi_word64[0] = (uint64_t)__dex_builduuid64_0__;
	DeeModule_Deemon.mo_buildid.mbi_word64[1] = (uint64_t)__dex_builduuid64_1__;
	return &DeeModule_Deemon.mo_buildid;
}
#elif defined(CONFIG_HAVE___dex_builduuid32__)
INTDEF __BYTE_TYPE__ __dex_builduuid32_0__[];
INTDEF __BYTE_TYPE__ __dex_builduuid32_1__[];
INTDEF __BYTE_TYPE__ __dex_builduuid32_2__[];
INTDEF __BYTE_TYPE__ __dex_builduuid32_3__[];
#define HAVE_DeeModule_GetBuildId_ofcore
PRIVATE ATTR_RETNONNULL WUNUSED union Dee_module_buildid const *DCALL
DeeModule_GetBuildId_ofcore(void) {
	DeeModule_Deemon.mo_buildid.mbi_word32[0] = (uint32_t)__dex_builduuid32_0__;
	DeeModule_Deemon.mo_buildid.mbi_word32[1] = (uint32_t)__dex_builduuid32_1__;
	DeeModule_Deemon.mo_buildid.mbi_word32[2] = (uint32_t)__dex_builduuid32_2__;
	DeeModule_Deemon.mo_buildid.mbi_word32[3] = (uint32_t)__dex_builduuid32_3__;
	return &DeeModule_Deemon.mo_buildid;
}
#elif defined(_MSC_VER) && !defined(__clang__) && defined(__PE__)
extern /*IMAGE_DOS_HEADER*/ __BYTE_TYPE__ const __ImageBase[];
LOCAL uint32_t get_TimeDateStamp(void) {
	uint32_t e_lfanew = *(uint32_t const *)(__ImageBase + 60);
	return *(uint32_t const *)(__ImageBase + e_lfanew + 8); /* Caution: this is 32-bit... */
}
#define HAVE_DeeModule_GetBuildId_ofcore
PRIVATE ATTR_RETNONNULL WUNUSED union Dee_module_buildid const *DCALL
DeeModule_GetBuildId_ofcore(void) {
	uint32_t ts = get_TimeDateStamp();
	DeeModule_Deemon.mo_buildid.mbi_word32[0] = DeeModule_Deemon.mo_buildid.mbi_word32[2] = ts;
	DeeModule_Deemon.mo_buildid.mbi_word32[1] = DeeModule_Deemon.mo_buildid.mbi_word32[3] = ~ts;
	return &DeeModule_Deemon.mo_buildid;
}
#endif /* ... */

#ifndef HAVE_DeeModule_GetBuildId_ofcore
#if defined(__DATE__) && defined(__TIME__)
#define BUILD_TS __DATE__ "|" __TIME__
PRIVATE char const deemon_build_ts[] = BUILD_TS;
#define HAVE_deemon_build_ts
#endif /* __DATE__ && __TIME__ */
#endif /* !HAVE_DeeModule_GetBuildId_ofcore */
#endif /* CONFIG_NO_DEX */


#if !defined(CONFIG_NO_DEX) || defined(HAVE_deemon_build_ts)
#if defined(CONFIG_NO_DEX) && defined(HAVE_deemon_build_ts)
LOCAL ATTR_CONST WUNUSED uint64_t parse_build_ts(void)
#define build_ts deemon_build_ts
#else /* CONFIG_NO_DEX && HAVE_deemon_build_ts */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) uint64_t DCALL
parse_build_ts(char const *build_ts)
#endif /* !CONFIG_NO_DEX || !HAVE_deemon_build_ts */
{
	/* A couple of helper macros taken from the libtime DEX. */
#define time_year2day(value) ((146097 * (value)) / 400) /* Not exact, but go enough for our purpose */
#define SECONDS_PER_MINUTE UINT64_C(60)
#define MINUTES_PER_HOUR   UINT64_C(60)
#define HOURS_PER_DAY      UINT64_C(24)
#define SECONDS_PER_HOUR   UINT16_C(3600)
#define SECONDS_PER_DAY    UINT32_C(86400)

#define TIMESTAMP_MON  (build_ts)
#define TIMESTAMP_MDAY (build_ts + 4)
#define TIMESTAMP_YEAR (build_ts + 7)
#define TIMESTAMP_HOUR (build_ts + 12)
#define TIMESTAMP_MIN  (build_ts + 15)
#define TIMESTAMP_SEC  (build_ts + 18)
	unsigned int monthday;
	unsigned int monthstart;
	unsigned int year;
	unsigned int is_leap_year;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;

#define MONTHSTART_JAN is_leap_year ? 0 : 0
#define MONTHSTART_FEB is_leap_year ? 31 : 31
#define MONTHSTART_MAR is_leap_year ? 60 : 59
#define MONTHSTART_APR is_leap_year ? 91 : 90
#define MONTHSTART_MAY is_leap_year ? 121 : 120
#define MONTHSTART_JUN is_leap_year ? 152 : 151
#define MONTHSTART_JUL is_leap_year ? 182 : 181
#define MONTHSTART_AUG is_leap_year ? 213 : 212
#define MONTHSTART_SEP is_leap_year ? 244 : 243
#define MONTHSTART_OCT is_leap_year ? 274 : 273
#define MONTHSTART_NOV is_leap_year ? 305 : 304
#define MONTHSTART_DEC is_leap_year ? 335 : 334
	/* With any luck, any decent optimizer will get rid of most of this stuff...
	 * NOTE: That's also the reason why everything is written without
	 *       loops, or the use of api functions such as sscanf().
	 *       Additionally, every variable is assigned to once (potentially
	 *       in different branches of the same if-statement who's condition
	 *       should already be known at compile-time), meaning that constant
	 *       propagation should be fairly easy to achieve. */
	monthday = (((TIMESTAMP_MDAY[0] - '0') * 10) + (TIMESTAMP_MDAY[1] - '0')) - 1;
	year = (((TIMESTAMP_YEAR[0] - '0') * 1000) +
	        ((TIMESTAMP_YEAR[1] - '0') * 100) +
	        ((TIMESTAMP_YEAR[2] - '0') * 10) +
	        (TIMESTAMP_YEAR[3] - '0'));
	is_leap_year = !(year % 400) || ((year % 100) && !(year % 4));
	if (TIMESTAMP_MON[0] == 'J') {
		if (TIMESTAMP_MON[1] == 'a') {
			monthstart = MONTHSTART_JAN;
		} else if (TIMESTAMP_MON[2] == 'n') {
			monthstart = MONTHSTART_JUN;
		} else {
			monthstart = MONTHSTART_JUL;
		}
	} else if (TIMESTAMP_MON[0] == 'F') {
		monthstart = MONTHSTART_FEB;
	} else if (TIMESTAMP_MON[0] == 'M') {
		if (TIMESTAMP_MON[2] == 'r') {
			monthstart = MONTHSTART_MAR;
		} else {
			monthstart = MONTHSTART_MAY;
		}
	} else if (TIMESTAMP_MON[0] == 'A') {
		if (TIMESTAMP_MON[1] == 'p') {
			monthstart = MONTHSTART_APR;
		} else {
			monthstart = MONTHSTART_AUG;
		}
	} else if (TIMESTAMP_MON[0] == 'S') {
		monthstart = MONTHSTART_SEP;
	} else if (TIMESTAMP_MON[0] == 'O') {
		monthstart = MONTHSTART_OCT;
	} else if (TIMESTAMP_MON[0] == 'N') {
		monthstart = MONTHSTART_NOV;
	} else {
		monthstart = MONTHSTART_DEC;
	}

	/* Figure out the time. */
	hour   = ((TIMESTAMP_HOUR[0] - '0') * 10 + (TIMESTAMP_HOUR[1] - '0'));
	minute = ((TIMESTAMP_MIN[0] - '0') * 10 + (TIMESTAMP_MIN[1] - '0'));
	second = ((TIMESTAMP_SEC[0] - '0') * 10 + (TIMESTAMP_SEC[1] - '0'));

	/* Pack everything together and return the result. */
	return (((uint64_t)((time_year2day(year) + monthstart + monthday) -
	                    /* Subtract the year 1970 from the result. */
	                    time_year2day(1970)) *
	         SECONDS_PER_DAY) +
	        /* Add the hour, minute and second to the calculation. */
	        ((uint64_t)hour * SECONDS_PER_HOUR) +
	        ((uint64_t)minute * SECONDS_PER_MINUTE) +
	        ((uint64_t)second));
#undef time_year2day
#undef SECONDS_PER_MINUTE
#undef MINUTES_PER_HOUR
#undef HOURS_PER_DAY
#undef SECONDS_PER_HOUR
#undef SECONDS_PER_DAY
#undef TIMESTAMP_MON
#undef TIMESTAMP_MDAY
#undef TIMESTAMP_YEAR
#undef TIMESTAMP_HOUR
#undef TIMESTAMP_MIN
#undef TIMESTAMP_SEC
#undef MONTHSTART_JAN
#undef MONTHSTART_FEB
#undef MONTHSTART_MAR
#undef MONTHSTART_APR
#undef MONTHSTART_MAY
#undef MONTHSTART_JUN
#undef MONTHSTART_JUL
#undef MONTHSTART_AUG
#undef MONTHSTART_SEP
#undef MONTHSTART_OCT
#undef MONTHSTART_NOV
#undef MONTHSTART_DEC
}
#undef build_ts
#endif /* !CONFIG_NO_DEX || HAVE_deemon_build_ts */


#ifndef CONFIG_NO_DEX
#undef DeeModule_GetBuildId_ofdex_USE__PE_TimeDateStamp /* Look at "TimeDateStamp" field of PE header */
#if defined(DeeSystem_DlOpen_USE_LoadLibrary) && defined(__PE__)
#define DeeModule_GetBuildId_ofdex_USE__PE_TimeDateStamp
#endif /* DeeSystem_DlOpen_USE_LoadLibrary && __PE__ */

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) union Dee_module_buildid const *DCALL
DeeModule_GetBuildId_ofdex_uncached(DeeModuleObject *__restrict self) {
	struct Dee_module_dexdata const *dexdata = self->mo_moddata.mo_dexdata;
	/* Check for custom override, as per:
	 * - ADDR(.note.gnu.build-id) (s.a. "/configure")
	 * - Random UUID set during build
	 *
	 * Must also check (and ignore) the special value "16", since
	 * that one can happen when '__dex_buildid__' wasn't defined,
	 * by a DEX module compiled with 'CONFIG_HAVE___dex_buildid__'.
	 *
	 * Admittedly, that shouldn't happen, but we want to be robust
	 * when it comes to detection of build IDs, and since we allow
	 * for ATTR_WEAK to deal with '__dex_buildid__' missing, that
	 * is something that can happen. */
#if defined(__ARCH_PAGESIZE_MIN) && __ARCH_PAGESIZE_MIN > 16
	if (dexdata->mdx_buildid && (uintptr_t)dexdata->mdx_buildid >= __ARCH_PAGESIZE_MIN)
#else /* __ARCH_PAGESIZE_MIN > 16 */
	if (dexdata->mdx_buildid && (uintptr_t)dexdata->mdx_buildid != 16)
#endif /* !__ARCH_PAGESIZE_MIN <= 16 */
	{
		self->mo_buildid = *dexdata->mdx_buildid;
		if (self->mo_buildid.mbi_word64[0] != 0 || self->mo_buildid.mbi_word64[1] != 0)
			return &self->mo_buildid;
	}

	/* Use "TimeDateStamp" from module's PE header on Windows */
#ifdef DeeModule_GetBuildId_ofdex_USE__PE_TimeDateStamp
	if (dexdata->mdx_handle != NULL &&
	    dexdata->mdx_handle != DeeSystem_DlOpen_FAILED) {
		byte_t *ImageBase      = (byte_t *)dexdata->mdx_handle;
		uint32_t e_lfanew      = *(uint32_t const *)(ImageBase + 60);
		uint32_t TimeDateStamp = *(uint32_t const *)(ImageBase + e_lfanew + 8); /* 32-bit timestamp */
		if (TimeDateStamp != 0) {
			self->mo_buildid.mbi_word32[0] = self->mo_buildid.mbi_word32[2] = TimeDateStamp;
			self->mo_buildid.mbi_word32[1] = self->mo_buildid.mbi_word32[3] = ~TimeDateStamp;
			return &self->mo_buildid;
		}
	}
#endif /* DeeModule_GetBuildId_ofdex_USE__PE_TimeDateStamp */

	if (dexdata->mdx_buildts != NULL && strlen(dexdata->mdx_buildts) == 20) {
		uint64_t ts = parse_build_ts(dexdata->mdx_buildts);
		self->mo_buildid.mbi_word64[0] = ts;
		self->mo_buildid.mbi_word64[1] = ~ts;
		return &self->mo_buildid;
	}

	/* No build ID embedded :( */
	(void)self;
	return NULL;
}
#elif defined(HAVE_deemon_build_ts) && !defined(HAVE_DeeModule_GetBuildId_ofcore)
#define HAVE_DeeModule_GetBuildId_ofcore
PRIVATE ATTR_RETNONNULL WUNUSED union Dee_module_buildid const *DCALL
DeeModule_GetBuildId_ofcore(void) {
	uint64_t ts = parse_build_ts();
	DeeModule_Deemon.mo_buildid.mbi_word64[0] = ts;
	DeeModule_Deemon.mo_buildid.mbi_word64[1] = ~ts;
	return &DeeModule_Deemon.mo_buildid;
}
#endif /* ... */

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) union Dee_module_buildid const *DCALL
DeeModule_GetBuildId_uncached(DeeModuleObject *__restrict self) {
	ASSERTF(Dee_TYPE(self) != &DeeModuleDir_Type,
	        "Dir-modules should pre-initialize their build-id "
	        "to '0' and set their 'Dee_MODULE_FHASBUILDID' flag");

	/* Handling for dex modules */
#ifndef CONFIG_NO_DEX
	if (Dee_TYPE(self) == &DeeModuleDex_Type) {
		union Dee_module_buildid const *result;
		result = DeeModule_GetBuildId_ofdex_uncached(self);
		if (result)
			return result;
	}
#else /* !CONFIG_NO_DEX */
	ASSERTF(self == &DeeModule_Deemon,
	        "DIR modules are already asserted, DEE modules always pre-calculate "
	        "their build IDs (as an MD5 hash), and since DEX modules are disabled "
	        "and 'DeeModule_Empty' has 'Dee_MODULE_FHASBUILDID' pre-set, that only "
	        "leaves the deemon core at this point!");
#ifdef HAVE_DeeModule_GetBuildId_ofcore
	return DeeModule_GetBuildId_ofcore();
#endif /* HAVE_DeeModule_GetBuildId_ofcore */
#endif /* CONFIG_NO_DEX */

#if !defined(CONFIG_NO_DEX) || !defined(HAVE_DeeModule_GetBuildId_ofcore)
	/* Fallback: use last-modified timestamp of `DeeModule_GetFileName()' */
	{
		uint64_t ts = DeeModule_GetFileLastModified(self);
		if unlikely(ts == (uint64_t)-1)
			goto err;
		if (ts != 0) {
			self->mo_buildid.mbi_word64[0] = ts;
			self->mo_buildid.mbi_word64[1] = ~ts;
			return &self->mo_buildid;
		}
	}

	/* Fall-fallback: unable to determine Build ID -> set it to all zeroes */
	self->mo_buildid.mbi_word64[0] = 0;
	self->mo_buildid.mbi_word64[1] = 0;
	return &self->mo_buildid;
err:
	return NULL;
#endif /* !defined(CONFIG_NO_DEX) || !defined(HAVE_DeeModule_GetBuildId_ofcore) */
}

/* Return the "Build ID" of this module, which is an opaque
 * identifier that can be treated as a hash for the specific version that is
 * loaded into the module "self". When the module unloads and is then re-loaded,
 * this hash might change, in which case dependents of "self" should also be
 * re-build (potentially causing their build IDs to change also).
 *
 * @return: * :   The module's build ID
 * @return: NULL: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) union Dee_module_buildid const *DCALL
DeeModule_GetBuildId(DeeModuleObject *__restrict self) {
	union Dee_module_buildid const *result;
	if (atomic_read(&self->mo_flags) & Dee_MODULE_FHASBUILDID)
		return &self->mo_buildid;
	result = DeeModule_GetBuildId_uncached(self);
	ASSERT(result == NULL || result == &self->mo_buildid);
	if (result) {
		COMPILER_WRITE_BARRIER();
		atomic_or(&self->mo_flags, Dee_MODULE_FHASBUILDID);
		COMPILER_BARRIER();
	}
	return &self->mo_buildid;
}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#ifdef CONFIG_NO_DEC
PRIVATE uint64_t exec_timestamp = (uint64_t)-1;
#define HAS_EXEC_TIMESTAMP    (exec_timestamp != (uint64_t)-1)
#define SET_EXEC_TIMESTAMP(x) (exec_timestamp = (x))
#else /* CONFIG_NO_DEC */
#define exec_timestamp        DeeModule_Deemon.mo_ctime
#define HAS_EXEC_TIMESTAMP    (DeeModule_Deemon.mo_flags & Dee_MODULE_FHASCTIME)
#define SET_EXEC_TIMESTAMP(x) (exec_timestamp = (x), atomic_or(&DeeModule_Deemon.mo_flags, Dee_MODULE_FHASCTIME))
#endif /* !CONFIG_NO_DEC */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


/* A couple of helper macros taken from the libtime DEX. */
#define time_year2day(value) ((146097 * (value)) / 400) /* Not exact, but go enough for our purpose */
#define MICROSECONDS_PER_MILLISECOND UINT64_C(1000)
#define MILLISECONDS_PER_SECOND      UINT64_C(1000)
#define SECONDS_PER_MINUTE           UINT64_C(60)
#define MINUTES_PER_HOUR             UINT64_C(60)
#define HOURS_PER_DAY                UINT64_C(24)
#define MICROSECONDS_PER_SECOND      (MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND)
#define MICROSECONDS_PER_MINUTE      (MICROSECONDS_PER_SECOND * SECONDS_PER_MINUTE)
#define MICROSECONDS_PER_HOUR        (MICROSECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define MICROSECONDS_PER_DAY         (MICROSECONDS_PER_HOUR * HOURS_PER_DAY)


#if defined(_MSC_VER) && !defined(__clang__) && defined(__PE__)
extern /*IMAGE_DOS_HEADER*/ __BYTE_TYPE__ const __ImageBase[];
LOCAL uint64_t parse_timestamp(void) {
	uint32_t e_lfanew      = *(uint32_t const *)(__ImageBase + 60);
	uint32_t TimeDateStamp = *(uint32_t const *)(__ImageBase + e_lfanew + 8); /* Problem: this is 32-bit... */
	return (uint64_t)TimeDateStamp * MICROSECONDS_PER_SECOND;
}
#else /* ... */
/* The timestamp when deemon was compiled, generated as `__DATE__ "|" __TIME__'
 * CAUTION (and why we try not to use this variant):
 *    this timestamp string is LOCALTIME! (but we'd need it to be UTC)
 * That's why we only use this variant as the last-case fallback! */
PRIVATE char const exec_timestamp_str[] = __DATE__ "|" __TIME__;

#define TIMESTAMP_MON  (exec_timestamp_str)
#define TIMESTAMP_MDAY (exec_timestamp_str + 4)
#define TIMESTAMP_YEAR (exec_timestamp_str + 7)
#define TIMESTAMP_HOUR (exec_timestamp_str + 12)
#define TIMESTAMP_MIN  (exec_timestamp_str + 15)
#define TIMESTAMP_SEC  (exec_timestamp_str + 18)


LOCAL uint64_t parse_timestamp(void) {
	unsigned int monthday;
	unsigned int monthstart;
	unsigned int year;
	unsigned int is_leap_year;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;

#define MONTHSTART_JAN is_leap_year ? 0 : 0
#define MONTHSTART_FEB is_leap_year ? 31 : 31
#define MONTHSTART_MAR is_leap_year ? 60 : 59
#define MONTHSTART_APR is_leap_year ? 91 : 90
#define MONTHSTART_MAY is_leap_year ? 121 : 120
#define MONTHSTART_JUN is_leap_year ? 152 : 151
#define MONTHSTART_JUL is_leap_year ? 182 : 181
#define MONTHSTART_AUG is_leap_year ? 213 : 212
#define MONTHSTART_SEP is_leap_year ? 244 : 243
#define MONTHSTART_OCT is_leap_year ? 274 : 273
#define MONTHSTART_NOV is_leap_year ? 305 : 304
#define MONTHSTART_DEC is_leap_year ? 335 : 334
	/* With any luck, any decent optimizer will get rid of most of this stuff...
	 * NOTE: That's also the reason why everything is written without
	 *       loops, or the use of api functions such as sscanf().
	 *       Additionally, every variable is assigned to once (potentially
	 *       in different branches of the same if-statement who's condition
	 *       should already be known at compile-time), meaning that constant
	 *       propagation should be fairly easy to achieve. */
	monthday = (((TIMESTAMP_MDAY[0] - '0') * 10) + (TIMESTAMP_MDAY[1] - '0')) - 1;
	year = (((TIMESTAMP_YEAR[0] - '0') * 1000) +
	        ((TIMESTAMP_YEAR[1] - '0') * 100) +
	        ((TIMESTAMP_YEAR[2] - '0') * 10) +
	        (TIMESTAMP_YEAR[3] - '0'));
	is_leap_year = !(year % 400) || ((year % 100) && !(year % 4));
	if (TIMESTAMP_MON[0] == 'J') {
		if (TIMESTAMP_MON[1] == 'a') {
			monthstart = MONTHSTART_JAN;
		} else if (TIMESTAMP_MON[2] == 'n') {
			monthstart = MONTHSTART_JUN;
		} else {
			monthstart = MONTHSTART_JUL;
		}
	} else if (TIMESTAMP_MON[0] == 'F') {
		monthstart = MONTHSTART_FEB;
	} else if (TIMESTAMP_MON[0] == 'M') {
		if (TIMESTAMP_MON[2] == 'r') {
			monthstart = MONTHSTART_MAR;
		} else {
			monthstart = MONTHSTART_MAY;
		}
	} else if (TIMESTAMP_MON[0] == 'A') {
		if (TIMESTAMP_MON[1] == 'p') {
			monthstart = MONTHSTART_APR;
		} else {
			monthstart = MONTHSTART_AUG;
		}
	} else if (TIMESTAMP_MON[0] == 'S') {
		monthstart = MONTHSTART_SEP;
	} else if (TIMESTAMP_MON[0] == 'O') {
		monthstart = MONTHSTART_OCT;
	} else if (TIMESTAMP_MON[0] == 'N') {
		monthstart = MONTHSTART_NOV;
	} else {
		monthstart = MONTHSTART_DEC;
	}

	/* Figure out the time. */
	hour   = ((TIMESTAMP_HOUR[0] - '0') * 10 + (TIMESTAMP_HOUR[1] - '0'));
	minute = ((TIMESTAMP_MIN[0] - '0') * 10 + (TIMESTAMP_MIN[1] - '0'));
	second = ((TIMESTAMP_SEC[0] - '0') * 10 + (TIMESTAMP_SEC[1] - '0'));

	/* Pack everything together and return the result. */
	return (((uint64_t)((time_year2day(year) + monthstart + monthday) -
	                    /* Subtract the year 1970 from the result. */
	                    time_year2day(1970)) *
	         MICROSECONDS_PER_DAY) +
	        /* Add the hour, minute and second to the calculation. */
	        ((uint64_t)hour * MICROSECONDS_PER_HOUR) +
	        ((uint64_t)minute * MICROSECONDS_PER_MINUTE) +
	        ((uint64_t)second * MICROSECONDS_PER_SECOND));
}
#endif /* !... */


PUBLIC uint64_t DCALL DeeExec_GetTimestamp(void) {
	uint64_t result = exec_timestamp;
	if (!HAS_EXEC_TIMESTAMP) {
		result = parse_timestamp();
		SET_EXEC_TIMESTAMP(result);
	}
	return result;
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_BUILDID_C */
