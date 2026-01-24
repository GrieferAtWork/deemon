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
#ifndef GUARD_DEX_TIME_LIBTIME_C
#define GUARD_DEX_TIME_LIBTIME_C 1
#define CONFIG_BUILDING_LIBTIME
#define DEE_SOURCE

#include "libtime.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>     /* atomic_write */

#include <hybrid/byteorder.h>       /* __BYTE_ORDER__, __ORDER_BIG_ENDIAN__ */
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/int128.h>          /* __HYBRID_UINT128_INIT16N, __hybrid_int128_*, __hybrid_uint128_* */
#include <hybrid/unaligned.h>       /* UNALIGNED_GET64 */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* INT32_MAX, INT32_MIN, INT64_C, UINT16_C, UINT32_C, UINT64_C, int8_t, int32_t, int64_t, uintN_t, uintptr_t */

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifndef INT32_MIN
#include <hybrid/limitcore.h> /* __INT32_MAX__, __INT32_MIN__ */
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */

#ifndef INT32_MAX
#include <hybrid/limitcore.h>
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#ifdef CONFIG_HAVE_memcasecmp
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#else /* CONFIG_HAVE_memcasecmp */
#define MEMCASEEQ(a, b, s) dee_memcaseeq((uint8_t *)(a), (uint8_t *)(b), s)
LOCAL WUNUSED NONNULL((1, 2)) bool
dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
	while (s--) {
		if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
			return false;
		++a;
		++b;
	}
	return true;
}
#endif /* !CONFIG_HAVE_memcasecmp */


#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FILETIME_GET64(x) (((x) << 32) | ((x) >> 32))
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#define FILETIME_GET64(x) (x)
#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */


#undef time_now_utc_USE_GetSystemTimePreciseAsFileTime
#undef time_now_utc_USE_clock_gettime64
#undef time_now_utc_USE_gettimeofday64
#undef time_now_utc_USE_time64
#undef time_now_utc_USE_clock_gettime
#undef time_now_utc_USE_gettimeofday
#undef time_now_utc_USE_time
#undef time_now_utc_USE_DeeSystem_GetWalltime
#ifdef CONFIG_HOST_WINDOWS
#define time_now_utc_USE_GetSystemTimePreciseAsFileTime
#else /* CONFIG_HOST_WINDOWS */
#if defined(CONFIG_HAVE_clock_gettime64) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define time_now_utc_USE_clock_gettime64
#endif /* CONFIG_HAVE_clock_gettime64 && CONFIG_HAVE_CLOCK_REALTIME */
#ifdef CONFIG_HAVE_gettimeofday64
#define time_now_utc_USE_gettimeofday64
#endif /* CONFIG_HAVE_gettimeofday64 */
#ifdef CONFIG_HAVE_time64
#define time_now_utc_USE_time64
#endif /* CONFIG_HAVE_time64 */
#if defined(CONFIG_HAVE_clock_gettime) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define time_now_utc_USE_clock_gettime
#endif /* CONFIG_HAVE_clock_gettime && CONFIG_HAVE_CLOCK_REALTIME */
#ifdef CONFIG_HAVE_gettimeofday
#define time_now_utc_USE_gettimeofday
#endif /* CONFIG_HAVE_gettimeofday */
#ifdef CONFIG_HAVE_time
#define time_now_utc_USE_time
#else /* CONFIG_HAVE_time */
#define time_now_utc_USE_DeeSystem_GetWalltime
#endif /* !CONFIG_HAVE_time */
#endif /* !CONFIG_HOST_WINDOWS */

#undef Xgettimezone
#ifdef CONFIG_HAVE_gettimeofday
#define Xgettimezone(tz) gettimeofday(NULL, tz)
#elif defined(CONFIG_HAVE_gettimeofday64)
#define Xgettimezone(tz) gettimeofday64(NULL, tz)
#endif /* ... */

#undef time_now_local_USE_GetSystemTimePreciseAsFileTime
#undef time_now_local_USE_time_now_utc_AND_tzset_AND_timezone
#undef time_now_local_USE_time_now_utc
#ifdef CONFIG_HOST_WINDOWS
#define time_now_local_USE_GetSystemTimePreciseAsFileTime
#elif defined(CONFIG_HAVE_timezone) && defined(CONFIG_HAVE_tzset)
#define time_now_local_USE_time_now_utc_AND_tzset_AND_timezone
#else /* ... */
#define time_now_local_USE_time_now_utc
#endif /* !... */


#ifdef time_now_utc_USE_GetSystemTimePreciseAsFileTime
#undef GetSystemTimePreciseAsFileTime
typedef void(WINAPI *LPGETSYSTEMTIMEPRECISEASFILETIME)(LPFILETIME lpSystemTimeAsFileTime);
static LPGETSYSTEMTIMEPRECISEASFILETIME pdyn_GetSystemTimePreciseAsFileTime = NULL;
#define GetSystemTimePreciseAsFileTime (*pdyn_GetSystemTimePreciseAsFileTime)
PRIVATE WCHAR const wKernel32[]    = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
PRIVATE WCHAR const wKernel32Dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };
PRIVATE HMODULE DCALL GetKernel32Handle(void) {
	HMODULE hKernel32;
	hKernel32 = GetModuleHandleW(wKernel32);
	if (!hKernel32)
		hKernel32 = LoadLibraryW(wKernel32Dll);
	return hKernel32;
}
#define WANT_NANOSECONDS_01_01_1601
#endif /* time_now_utc_USE_GetSystemTimePreciseAsFileTime */

#ifdef time_now_local_USE_GetSystemTimePreciseAsFileTime
#define WANT_NANOSECONDS_01_01_1601
#endif /* time_now_local_USE_GetSystemTimePreciseAsFileTime */

#ifdef CONFIG_HOST_WINDOWS
/* Also needed for `_mkFILETIME' */
#define WANT_NANOSECONDS_01_01_1601
#endif /* CONFIG_HOST_WINDOWS */


/* Nano-seconds from 01-01-0000 to 01-01-1601 */
/*[[[deemon
import * from time;
print("#ifdef WANT_NANOSECONDS_01_01_1601");
print("#undef WANT_NANOSECONDS_01_01_1601");
print("static Dee_uint128_t const NANOSECONDS_01_01_1601 =");
print("__HYBRID_UINT128_INIT16N(0x",
	", 0x".join(
		Time(year: 1601, month: 1, day: 1)
			.nanoseconds
			.tostr(16, 32)
			.segments(4)
	), ");");
print("#endif /" "* WANT_NANOSECONDS_01_01_1601 *" "/");
]]]*/
#ifdef WANT_NANOSECONDS_01_01_1601
#undef WANT_NANOSECONDS_01_01_1601
static Dee_uint128_t const NANOSECONDS_01_01_1601 =
__HYBRID_UINT128_INIT16N(0x0000, 0x0000, 0x0000, 0x0002, 0xbd24, 0xd971, 0x356e, 0x0000);
#endif /* WANT_NANOSECONDS_01_01_1601 */
/*[[[end]]]*/



/* Return the current time in UTC */
INTERN NONNULL((1)) void DFCALL
time_now_utc(Dee_int128_t *__restrict p_result) {

#ifdef time_now_utc_USE_GetSystemTimePreciseAsFileTime
	uint64_t filetime;

	DBG_ALIGNMENT_DISABLE();
	if (pdyn_GetSystemTimePreciseAsFileTime == NULL) {
		HMODULE hKernel32 = GetKernel32Handle();
		if (!hKernel32) {
			atomic_write((void **)&pdyn_GetSystemTimePreciseAsFileTime, (void *)(uintptr_t)-1);
		} else {
			LPGETSYSTEMTIMEPRECISEASFILETIME func;
			func = (LPGETSYSTEMTIMEPRECISEASFILETIME)GetProcAddress(hKernel32, "GetSystemTimePreciseAsFileTime");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			atomic_write(&pdyn_GetSystemTimePreciseAsFileTime, func);
		}
	}
	if (pdyn_GetSystemTimePreciseAsFileTime != (LPGETSYSTEMTIMEPRECISEASFILETIME)-1) {
		GetSystemTimePreciseAsFileTime((LPFILETIME)&filetime);
	} else {
		GetSystemTimeAsFileTime((LPFILETIME)&filetime);
	}
	DBG_ALIGNMENT_ENABLE();

	__hybrid_uint128_set64(*(Dee_uint128_t *)p_result, FILETIME_GET64(filetime));
	__hybrid_uint128_mul8(*(Dee_uint128_t *)p_result, 100);
	__hybrid_uint128_add128(*(Dee_uint128_t *)p_result, NANOSECONDS_01_01_1601);
	return;
#endif /* time_now_utc_USE_GetSystemTimePreciseAsFileTime */

#ifdef time_now_utc_USE_clock_gettime64
	{
		struct timespec64 ts;
		if likely(clock_gettime64(CLOCK_REALTIME, &ts) == 0) {
			__hybrid_int128_set(*p_result, ts.tv_sec);
			__hybrid_uint128_add64(*(Dee_uint128_t *)p_result, SECONDS_01_01_1970);
			__hybrid_uint128_mul32(*(Dee_uint128_t *)p_result, NANOSECONDS_PER_SECOND);
			__hybrid_uint128_add32(*(Dee_uint128_t *)p_result, ts.tv_nsec);
			return;
		}
	}
#endif /* time_now_utc_USE_clock_gettime64 */

#ifdef time_now_utc_USE_gettimeofday64
	{
		struct timeval64 ts;
		if likely(gettimeofday64(&ts, NULL) == 0) {
			__hybrid_int128_set(*p_result, ts.tv_sec);
			__hybrid_uint128_add64(*(Dee_uint128_t *)p_result, SECONDS_01_01_1970);
			__hybrid_uint128_mul32(*(Dee_uint128_t *)p_result, NANOSECONDS_PER_SECOND);
			__hybrid_uint128_add32(*(Dee_uint128_t *)p_result, ts.tv_usec * 1000);
			return;
		}
	}
#endif /* time_now_utc_USE_gettimeofday64 */

#ifdef time_now_utc_USE_time64
	{
		time64_t t = time64(NULL);
		__hybrid_int128_set(*p_result, t);
			__hybrid_uint128_add64(*(Dee_uint128_t *)p_result, SECONDS_01_01_1970);
		__hybrid_uint128_mul32(*(Dee_uint128_t *)p_result, NANOSECONDS_PER_SECOND);
		return;
	}
#endif /* time_now_utc_USE_time64 */

#ifdef time_now_utc_USE_clock_gettime
	{
		struct timespec ts;
		if likely(clock_gettime(CLOCK_REALTIME, &ts) == 0) {
			__hybrid_int128_set(*p_result, ts.tv_sec);
			__hybrid_uint128_add64(*(Dee_uint128_t *)p_result, SECONDS_01_01_1970);
			__hybrid_uint128_mul32(*(Dee_uint128_t *)p_result, NANOSECONDS_PER_SECOND);
			__hybrid_uint128_add32(*(Dee_uint128_t *)p_result, ts.tv_nsec);
			return;
		}
	}
#endif /* time_now_utc_USE_clock_gettime */

#ifdef time_now_utc_USE_gettimeofday
	{
		struct timeval tv;
		if likely(gettimeofday(&tv, NULL) == 0) {
			__hybrid_int128_set(*p_result, tv.tv_sec);
			__hybrid_uint128_add64(*(Dee_uint128_t *)p_result, SECONDS_01_01_1970);
			__hybrid_uint128_mul32(*(Dee_uint128_t *)p_result, NANOSECONDS_PER_SECOND);
			__hybrid_uint128_add32(*(Dee_uint128_t *)p_result, tv.tv_usec * 1000);
			return;
		}
	}
#endif /* time_now_utc_USE_gettimeofday */

#ifdef time_now_utc_USE_time
	{
		time_t t = time(NULL);
		__hybrid_int128_set(*p_result, t);
		__hybrid_uint128_add64(*(Dee_uint128_t *)p_result, SECONDS_01_01_1970);
		__hybrid_uint128_mul32(*(Dee_uint128_t *)p_result, NANOSECONDS_PER_SECOND);
		return;
	}
#endif /* time_now_utc_USE_time */

#ifdef time_now_utc_USE_DeeSystem_GetWalltime
	{
		uint64_t walltime_microseconds = DeeSystem_GetWalltime();
		uint64_t walltime_seconds      = (uint64_t)(walltime_microseconds / UINT32_C(1000000));
		uint32_t walltime_microsecond  = (uint32_t)(walltime_microseconds % UINT32_C(1000000));
		__hybrid_uint128_set64(*(Dee_uint128_t *)p_result, walltime_seconds);
		__hybrid_uint128_add64(*(Dee_uint128_t *)p_result, SECONDS_01_01_1970);
		__hybrid_uint128_mul32(*(Dee_uint128_t *)p_result, NANOSECONDS_PER_SECOND);
		__hybrid_uint128_add32(*(Dee_uint128_t *)p_result, walltime_microsecond * UINT32_C(1000));
		return;
	}
#endif /* time_now_utc_USE_DeeSystem_GetWalltime */
}



#ifdef time_now_local_USE_time_now_utc_AND_tzset_AND_timezone
#define WANT_call_tzset
#endif /* time_now_local_USE_time_now_utc_AND_tzset_AND_timezone */

#ifdef WANT_call_tzset
PRIVATE int64_t timezone_nanoseconds = 0;
PRIVATE bool did_call_tzset = false;
PRIVATE void do_call_tzset(void) {
	int64_t tz_value;
	tzset();
	tz_value = (int64_t)timezone; /* seconds West of UTC */
	tz_value *= NANOSECONDS_PER_SECOND;
	COMPILER_WRITE_BARRIER();
	timezone_nanoseconds = tz_value;
	COMPILER_WRITE_BARRIER();
	did_call_tzset = true;
}
#define call_tzset()           \
	do {                       \
		if (!did_call_tzset) { \
			do_call_tzset();   \
		}                      \
	}	__WHILE0
#endif /* WANT_call_tzset */

/* Return the current time in local time */
INTERN NONNULL((1)) void DFCALL
time_now_local(Dee_int128_t *__restrict p_result) {

#ifdef time_now_local_USE_GetSystemTimePreciseAsFileTime
	uint64_t utc_filetime;
	uint64_t filetime;

	DBG_ALIGNMENT_DISABLE();
	if (pdyn_GetSystemTimePreciseAsFileTime == NULL) {
		HMODULE hKernel32 = GetKernel32Handle();
		if (!hKernel32) {
			atomic_write((void **)&pdyn_GetSystemTimePreciseAsFileTime, (void *)(uintptr_t)-1);
		} else {
			LPGETSYSTEMTIMEPRECISEASFILETIME func;
			func = (LPGETSYSTEMTIMEPRECISEASFILETIME)GetProcAddress(hKernel32, "GetSystemTimePreciseAsFileTime");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			atomic_write(&pdyn_GetSystemTimePreciseAsFileTime, func);
		}
	}
	if (pdyn_GetSystemTimePreciseAsFileTime != (LPGETSYSTEMTIMEPRECISEASFILETIME)-1) {
		GetSystemTimePreciseAsFileTime((LPFILETIME)&utc_filetime);
	} else {
		GetSystemTimeAsFileTime((LPFILETIME)&utc_filetime);
	}
	FileTimeToLocalFileTime((LPFILETIME)&utc_filetime, (LPFILETIME)&filetime);
	DBG_ALIGNMENT_ENABLE();

	__hybrid_uint128_set64(*(Dee_uint128_t *)p_result, FILETIME_GET64(filetime));
	__hybrid_uint128_mul8(*(Dee_uint128_t *)p_result, 100);
	__hybrid_uint128_add128(*(Dee_uint128_t *)p_result, NANOSECONDS_01_01_1601);
	return;
#endif /* time_now_local_USE_GetSystemTimePreciseAsFileTime */

#ifdef time_now_local_USE_time_now_utc_AND_tzset_AND_timezone
	time_now_utc(p_result);
	call_tzset();
	__hybrid_int128_add64(*p_result, timezone_nanoseconds);
#endif /* time_now_local_USE_time_now_utc_AND_tzset_AND_timezone */

#ifdef time_now_local_USE_time_now_utc
	time_now_utc(p_result);
#endif /* time_now_local_USE_time_now_utc */
}

#define NAMEOF_JAN "Jan\0January\0"
#define NAMEOF_FEB "Feb\0February\0"
#define NAMEOF_MAR "Mar\0March\0"
#define NAMEOF_APR "Apr\0April\0"
#define NAMEOF_MAY "May\0May\0"
#define NAMEOF_JUN "Jun\0June\0"
#define NAMEOF_JUL "Jul\0July\0"
#define NAMEOF_AUG "Aug\0August\0"
#define NAMEOF_SEP "Sep\0September\0"
#define NAMEOF_OCT "Oct\0October\0"
#define NAMEOF_NOV "Nov\0November\0"
#define NAMEOF_DEC "Dec\0December"

#define OFFSETOF_NAMEOF_JAN 0
#define OFFSETOF_NAMEOF_FEB (OFFSETOF_NAMEOF_JAN + COMPILER_STRLEN(NAMEOF_JAN))
#define OFFSETOF_NAMEOF_MAR (OFFSETOF_NAMEOF_FEB + COMPILER_STRLEN(NAMEOF_FEB))
#define OFFSETOF_NAMEOF_APR (OFFSETOF_NAMEOF_MAR + COMPILER_STRLEN(NAMEOF_MAR))
#define OFFSETOF_NAMEOF_MAY (OFFSETOF_NAMEOF_APR + COMPILER_STRLEN(NAMEOF_APR))
#define OFFSETOF_NAMEOF_JUN (OFFSETOF_NAMEOF_MAY + COMPILER_STRLEN(NAMEOF_MAY))
#define OFFSETOF_NAMEOF_JUL (OFFSETOF_NAMEOF_JUN + COMPILER_STRLEN(NAMEOF_JUN))
#define OFFSETOF_NAMEOF_AUG (OFFSETOF_NAMEOF_JUL + COMPILER_STRLEN(NAMEOF_JUL))
#define OFFSETOF_NAMEOF_SEP (OFFSETOF_NAMEOF_AUG + COMPILER_STRLEN(NAMEOF_AUG))
#define OFFSETOF_NAMEOF_OCT (OFFSETOF_NAMEOF_SEP + COMPILER_STRLEN(NAMEOF_SEP))
#define OFFSETOF_NAMEOF_NOV (OFFSETOF_NAMEOF_OCT + COMPILER_STRLEN(NAMEOF_OCT))
#define OFFSETOF_NAMEOF_DEC (OFFSETOF_NAMEOF_NOV + COMPILER_STRLEN(NAMEOF_NOV))

INTERN_CONST char const abbr_wday_names[7][4]  = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
INTERN_CONST char const full_wday_names[7][10] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
INTERN_CONST char const am_pm[2][3]            = { "AM", "PM" };
INTERN_CONST char const am_pm_lower[2][3]      = { "am", "pm" };

/* Month names in English. */
INTERN_CONST char const month_names[] =
NAMEOF_JAN NAMEOF_FEB NAMEOF_MAR NAMEOF_APR NAMEOF_MAY NAMEOF_JUN
NAMEOF_JUL NAMEOF_AUG NAMEOF_SEP NAMEOF_OCT NAMEOF_NOV NAMEOF_DEC;

#define days2nano(n) (UINT64_C(n) * NANOSECONDS_PER_DAY)
INTERN_CONST struct month const month_info[2][MONTHS_PER_YEAR + 1] = {
	/* No leap year: */
	{
		{ days2nano(0), 31, OFFSETOF_NAMEOF_JAN },
		{ days2nano(31), 28, OFFSETOF_NAMEOF_FEB },
		{ days2nano(59), 31, OFFSETOF_NAMEOF_MAR },
		{ days2nano(90), 30, OFFSETOF_NAMEOF_APR },
		{ days2nano(120), 31, OFFSETOF_NAMEOF_MAY },
		{ days2nano(151), 30, OFFSETOF_NAMEOF_JUN },
		{ days2nano(181), 31, OFFSETOF_NAMEOF_JUL },
		{ days2nano(212), 31, OFFSETOF_NAMEOF_AUG },
		{ days2nano(243), 30, OFFSETOF_NAMEOF_SEP },
		{ days2nano(273), 31, OFFSETOF_NAMEOF_OCT },
		{ days2nano(304), 30, OFFSETOF_NAMEOF_NOV },
		{ days2nano(334), 31, OFFSETOF_NAMEOF_DEC },
		{ days2nano(365), 0, 0 }
	},

	/* leap year: */
	{
		{ days2nano(0), 31, OFFSETOF_NAMEOF_JAN },
		{ days2nano(31), 29, OFFSETOF_NAMEOF_FEB },
		{ days2nano(60), 31, OFFSETOF_NAMEOF_MAR },
		{ days2nano(91), 30, OFFSETOF_NAMEOF_APR },
		{ days2nano(121), 31, OFFSETOF_NAMEOF_MAY },
		{ days2nano(152), 30, OFFSETOF_NAMEOF_JUN },
		{ days2nano(182), 31, OFFSETOF_NAMEOF_JUL },
		{ days2nano(213), 31, OFFSETOF_NAMEOF_AUG },
		{ days2nano(244), 30, OFFSETOF_NAMEOF_SEP },
		{ days2nano(274), 31, OFFSETOF_NAMEOF_OCT },
		{ days2nano(305), 30, OFFSETOF_NAMEOF_NOV },
		{ days2nano(335), 31, OFFSETOF_NAMEOF_DEC },
		{ days2nano(366), 31, 0 }
	}
};
#undef days2nano


/* Check if the year referenced by the year-counter `*p_year' is a leap-year */
INTERN WUNUSED NONNULL((1)) bool DFCALL
time_years_isleapyear(Dee_int128_t const *__restrict p_year) {
	Dee_int128_t year_mod_400;
	Dee_int128_t year_mod_100;
	Dee_int128_t year_mod_4;

	year_mod_400 = *p_year;
	__hybrid_int128_floormod16(year_mod_400, 400);
	if (__hybrid_int128_get16(year_mod_400) == 0)
		return true;

	year_mod_100 = *p_year;
	__hybrid_int128_floormod8(year_mod_100, 100);
	if (__hybrid_int128_get8(year_mod_100) == 0)
		return false;

	year_mod_4 = *p_year;
	__hybrid_int128_floormod8(year_mod_4, 4);
	return __hybrid_int128_get8(year_mod_4) == 0;
}


/*[[[deemon
import * from deemon;

@@Known-good function to convert days-since-01-01-0000 to a date (year, month, day-of-month)
@@NOTE: `month` and `day-of-month` are both 1-based here
@@From: http://pmyers.pcug.org.au/General/JulianDates.htm
function good_day2date(j: int): (int, int, int) {
	j = j - 59;
	local y = (4 * j - 1) / 146097;
	j = 4 * j - 1 - 146097 * y;
	local d = j / 4;
	j = (4 * d + 3) / 1461;
	d = 4 * d + 3 - 1461 * j;
	d = (d + 4) / 4;
	local m = (5 * d - 3) / 153;
	d = 5 * d - 3 - 153 * m;
	d = (d + 5) / 5;
	y = 100 * y + j;
	if (m < 10) {
		m = m + 3;
	} else {
		m = m - 9;
		y = y + 1;
	}
	return (y, m, d);
}

@@Known-good function to convert (year, month, day-of-month) into days-since-01-01-0000
@@NOTE: `month` and `day-of-month` are both 1-based here
@@From: http://pmyers.pcug.org.au/General/JulianDates.htm
function good_date2day(y: int, m: int, d: int): int {
	if (m > 2) {
		m = m - 3;
	} else {
		m = m + 9;
		y = y - 1;
	}
	local c = y / 100;
	local ya = y - 100 * c;
	local j = (146097 * c) / 4 + (1461 * ya) / 4 + (153 * m + 2) / 5 + d + 59;
	return j;
}

local y400_table: {int...} = Tuple(
	for (local y: [:401])
		good_date2day(y, 1, 1));

// From: https://wiki.osdev.org/Julian_Day_Number
//function fast_year2day(year: int): int {
//	local ydiv = year / 400;
//	local ymod = year % 400;
//	return ydiv * 146097 + y400_table[ymod];
//}
//function fast_day2year(value) {
//	local base = 400 * (value / 146097);
//	local temp = (value % 146097);
//	local temp2 = temp;
//	local greg_yr = 0;
//	if (temp > 73048) {
//		temp -= 73048;
//		greg_yr = 200;
//	}
//	greg_yr += (temp << 10) / 374014;
//	if (y400_table[greg_yr + 1] <= temp2)
//		++greg_yr;
//	return base + greg_yr;
//}

print("PRIVATE uint32_t const y400_table[401] = {");
for (local line: y400_table.segments(6))
	print("	", ", ".join(for (local x: line) f"UINT32_C({x})"), ",");
print("};");
]]]*/
PRIVATE uint32_t const y400_table[401] = {
	UINT32_C(0), UINT32_C(366), UINT32_C(731), UINT32_C(1096), UINT32_C(1461), UINT32_C(1827),
	UINT32_C(2192), UINT32_C(2557), UINT32_C(2922), UINT32_C(3288), UINT32_C(3653), UINT32_C(4018),
	UINT32_C(4383), UINT32_C(4749), UINT32_C(5114), UINT32_C(5479), UINT32_C(5844), UINT32_C(6210),
	UINT32_C(6575), UINT32_C(6940), UINT32_C(7305), UINT32_C(7671), UINT32_C(8036), UINT32_C(8401),
	UINT32_C(8766), UINT32_C(9132), UINT32_C(9497), UINT32_C(9862), UINT32_C(10227), UINT32_C(10593),
	UINT32_C(10958), UINT32_C(11323), UINT32_C(11688), UINT32_C(12054), UINT32_C(12419), UINT32_C(12784),
	UINT32_C(13149), UINT32_C(13515), UINT32_C(13880), UINT32_C(14245), UINT32_C(14610), UINT32_C(14976),
	UINT32_C(15341), UINT32_C(15706), UINT32_C(16071), UINT32_C(16437), UINT32_C(16802), UINT32_C(17167),
	UINT32_C(17532), UINT32_C(17898), UINT32_C(18263), UINT32_C(18628), UINT32_C(18993), UINT32_C(19359),
	UINT32_C(19724), UINT32_C(20089), UINT32_C(20454), UINT32_C(20820), UINT32_C(21185), UINT32_C(21550),
	UINT32_C(21915), UINT32_C(22281), UINT32_C(22646), UINT32_C(23011), UINT32_C(23376), UINT32_C(23742),
	UINT32_C(24107), UINT32_C(24472), UINT32_C(24837), UINT32_C(25203), UINT32_C(25568), UINT32_C(25933),
	UINT32_C(26298), UINT32_C(26664), UINT32_C(27029), UINT32_C(27394), UINT32_C(27759), UINT32_C(28125),
	UINT32_C(28490), UINT32_C(28855), UINT32_C(29220), UINT32_C(29586), UINT32_C(29951), UINT32_C(30316),
	UINT32_C(30681), UINT32_C(31047), UINT32_C(31412), UINT32_C(31777), UINT32_C(32142), UINT32_C(32508),
	UINT32_C(32873), UINT32_C(33238), UINT32_C(33603), UINT32_C(33969), UINT32_C(34334), UINT32_C(34699),
	UINT32_C(35064), UINT32_C(35430), UINT32_C(35795), UINT32_C(36160), UINT32_C(36525), UINT32_C(36890),
	UINT32_C(37255), UINT32_C(37620), UINT32_C(37985), UINT32_C(38351), UINT32_C(38716), UINT32_C(39081),
	UINT32_C(39446), UINT32_C(39812), UINT32_C(40177), UINT32_C(40542), UINT32_C(40907), UINT32_C(41273),
	UINT32_C(41638), UINT32_C(42003), UINT32_C(42368), UINT32_C(42734), UINT32_C(43099), UINT32_C(43464),
	UINT32_C(43829), UINT32_C(44195), UINT32_C(44560), UINT32_C(44925), UINT32_C(45290), UINT32_C(45656),
	UINT32_C(46021), UINT32_C(46386), UINT32_C(46751), UINT32_C(47117), UINT32_C(47482), UINT32_C(47847),
	UINT32_C(48212), UINT32_C(48578), UINT32_C(48943), UINT32_C(49308), UINT32_C(49673), UINT32_C(50039),
	UINT32_C(50404), UINT32_C(50769), UINT32_C(51134), UINT32_C(51500), UINT32_C(51865), UINT32_C(52230),
	UINT32_C(52595), UINT32_C(52961), UINT32_C(53326), UINT32_C(53691), UINT32_C(54056), UINT32_C(54422),
	UINT32_C(54787), UINT32_C(55152), UINT32_C(55517), UINT32_C(55883), UINT32_C(56248), UINT32_C(56613),
	UINT32_C(56978), UINT32_C(57344), UINT32_C(57709), UINT32_C(58074), UINT32_C(58439), UINT32_C(58805),
	UINT32_C(59170), UINT32_C(59535), UINT32_C(59900), UINT32_C(60266), UINT32_C(60631), UINT32_C(60996),
	UINT32_C(61361), UINT32_C(61727), UINT32_C(62092), UINT32_C(62457), UINT32_C(62822), UINT32_C(63188),
	UINT32_C(63553), UINT32_C(63918), UINT32_C(64283), UINT32_C(64649), UINT32_C(65014), UINT32_C(65379),
	UINT32_C(65744), UINT32_C(66110), UINT32_C(66475), UINT32_C(66840), UINT32_C(67205), UINT32_C(67571),
	UINT32_C(67936), UINT32_C(68301), UINT32_C(68666), UINT32_C(69032), UINT32_C(69397), UINT32_C(69762),
	UINT32_C(70127), UINT32_C(70493), UINT32_C(70858), UINT32_C(71223), UINT32_C(71588), UINT32_C(71954),
	UINT32_C(72319), UINT32_C(72684), UINT32_C(73049), UINT32_C(73414), UINT32_C(73779), UINT32_C(74144),
	UINT32_C(74509), UINT32_C(74875), UINT32_C(75240), UINT32_C(75605), UINT32_C(75970), UINT32_C(76336),
	UINT32_C(76701), UINT32_C(77066), UINT32_C(77431), UINT32_C(77797), UINT32_C(78162), UINT32_C(78527),
	UINT32_C(78892), UINT32_C(79258), UINT32_C(79623), UINT32_C(79988), UINT32_C(80353), UINT32_C(80719),
	UINT32_C(81084), UINT32_C(81449), UINT32_C(81814), UINT32_C(82180), UINT32_C(82545), UINT32_C(82910),
	UINT32_C(83275), UINT32_C(83641), UINT32_C(84006), UINT32_C(84371), UINT32_C(84736), UINT32_C(85102),
	UINT32_C(85467), UINT32_C(85832), UINT32_C(86197), UINT32_C(86563), UINT32_C(86928), UINT32_C(87293),
	UINT32_C(87658), UINT32_C(88024), UINT32_C(88389), UINT32_C(88754), UINT32_C(89119), UINT32_C(89485),
	UINT32_C(89850), UINT32_C(90215), UINT32_C(90580), UINT32_C(90946), UINT32_C(91311), UINT32_C(91676),
	UINT32_C(92041), UINT32_C(92407), UINT32_C(92772), UINT32_C(93137), UINT32_C(93502), UINT32_C(93868),
	UINT32_C(94233), UINT32_C(94598), UINT32_C(94963), UINT32_C(95329), UINT32_C(95694), UINT32_C(96059),
	UINT32_C(96424), UINT32_C(96790), UINT32_C(97155), UINT32_C(97520), UINT32_C(97885), UINT32_C(98251),
	UINT32_C(98616), UINT32_C(98981), UINT32_C(99346), UINT32_C(99712), UINT32_C(100077), UINT32_C(100442),
	UINT32_C(100807), UINT32_C(101173), UINT32_C(101538), UINT32_C(101903), UINT32_C(102268), UINT32_C(102634),
	UINT32_C(102999), UINT32_C(103364), UINT32_C(103729), UINT32_C(104095), UINT32_C(104460), UINT32_C(104825),
	UINT32_C(105190), UINT32_C(105556), UINT32_C(105921), UINT32_C(106286), UINT32_C(106651), UINT32_C(107017),
	UINT32_C(107382), UINT32_C(107747), UINT32_C(108112), UINT32_C(108478), UINT32_C(108843), UINT32_C(109208),
	UINT32_C(109573), UINT32_C(109938), UINT32_C(110303), UINT32_C(110668), UINT32_C(111033), UINT32_C(111399),
	UINT32_C(111764), UINT32_C(112129), UINT32_C(112494), UINT32_C(112860), UINT32_C(113225), UINT32_C(113590),
	UINT32_C(113955), UINT32_C(114321), UINT32_C(114686), UINT32_C(115051), UINT32_C(115416), UINT32_C(115782),
	UINT32_C(116147), UINT32_C(116512), UINT32_C(116877), UINT32_C(117243), UINT32_C(117608), UINT32_C(117973),
	UINT32_C(118338), UINT32_C(118704), UINT32_C(119069), UINT32_C(119434), UINT32_C(119799), UINT32_C(120165),
	UINT32_C(120530), UINT32_C(120895), UINT32_C(121260), UINT32_C(121626), UINT32_C(121991), UINT32_C(122356),
	UINT32_C(122721), UINT32_C(123087), UINT32_C(123452), UINT32_C(123817), UINT32_C(124182), UINT32_C(124548),
	UINT32_C(124913), UINT32_C(125278), UINT32_C(125643), UINT32_C(126009), UINT32_C(126374), UINT32_C(126739),
	UINT32_C(127104), UINT32_C(127470), UINT32_C(127835), UINT32_C(128200), UINT32_C(128565), UINT32_C(128931),
	UINT32_C(129296), UINT32_C(129661), UINT32_C(130026), UINT32_C(130392), UINT32_C(130757), UINT32_C(131122),
	UINT32_C(131487), UINT32_C(131853), UINT32_C(132218), UINT32_C(132583), UINT32_C(132948), UINT32_C(133314),
	UINT32_C(133679), UINT32_C(134044), UINT32_C(134409), UINT32_C(134775), UINT32_C(135140), UINT32_C(135505),
	UINT32_C(135870), UINT32_C(136236), UINT32_C(136601), UINT32_C(136966), UINT32_C(137331), UINT32_C(137697),
	UINT32_C(138062), UINT32_C(138427), UINT32_C(138792), UINT32_C(139158), UINT32_C(139523), UINT32_C(139888),
	UINT32_C(140253), UINT32_C(140619), UINT32_C(140984), UINT32_C(141349), UINT32_C(141714), UINT32_C(142080),
	UINT32_C(142445), UINT32_C(142810), UINT32_C(143175), UINT32_C(143541), UINT32_C(143906), UINT32_C(144271),
	UINT32_C(144636), UINT32_C(145002), UINT32_C(145367), UINT32_C(145732), UINT32_C(146097),
};
/*[[[end]]]*/

/* Convert between days-since-01-01-0000 and that the relevant year.
 * When converting from year-to-day, return that year's 01-01-XXXX. */
INTERN NONNULL((1)) void DFCALL
time_inplace_day2year(Dee_int128_t *__restrict p_value) {
	/* Inaccurate version of this function would look like:
	 * >> return (400 * value) / 146097;
	 *
	 * The accurate implementation below is derived from:
	 * >> https://wiki.osdev.org/Julian_Day_Number */
	Dee_int128_t base;
	uint32_t temp, temp2, greg_yr;
	__hybrid_int128_floordivmod32(*p_value, DAYS_PER_400_YEARS, base, temp);
	__hybrid_int128_mul16(base, UINT16_C(400));
	temp2   = temp;
	greg_yr = 0;
	if (temp > 73048) { /* For years 200 to 399 */
		temp -= 73048;  /* this shifts the calculation by 1 day */
		greg_yr = 200;
	}
	greg_yr += (temp << 10) / 374014;     /* 374014 is a magic number */
	if (y400_table[greg_yr + 1] <= temp2) /* fix if greg_yr is off by 1 */
		++greg_yr;
	__hybrid_int128_add32(base, greg_yr);
	*p_value = base;
}

INTERN NONNULL((1)) void DFCALL
time_inplace_year2day(Dee_int128_t *__restrict p_value) {
	Dee_int128_t ydiv;
	uint16_t ymod;
	__hybrid_int128_floordivmod16(*p_value, UINT16_C(400), ydiv, ymod);
	__hybrid_int128_mul32(ydiv, DAYS_PER_400_YEARS);
	__hybrid_int128_add32(ydiv, y400_table[ymod]);
	*p_value = ydiv;
}


/* Convert between a specific month and that month's starting nano-second */
INTERN NONNULL((1)) void DFCALL
time_inplace_nanosecond2month(Dee_int128_t *__restrict p_value) {
	Dee_int128_t year = *p_value;
	Dee_int128_t start_of_year_nanoseconds;
	Dee_int128_t nanoseconds_since_start_of_year;
	struct month const *months_of_year;
	unsigned int month_id;
	time_inplace_nanosecond2year(&year);
	start_of_year_nanoseconds = year;
	time_inplace_year2nanosecond(&start_of_year_nanoseconds);
	nanoseconds_since_start_of_year = *p_value;
	__hybrid_int128_sub128(nanoseconds_since_start_of_year, start_of_year_nanoseconds);
	months_of_year = month_info_for_year(&year);
	for (month_id = 0;; ++month_id) {
		uint64_t month_end;
		ASSERT(month_id < MONTHS_PER_YEAR);
		month_end = month_getend(&months_of_year[month_id]);
		if (__hybrid_int128_lo64(nanoseconds_since_start_of_year, month_end))
			break;
	}
	__hybrid_int128_mul8(year, MONTHS_PER_YEAR);
	__hybrid_int128_add8(year, month_id);
	*p_value = year;
}

INTERN NONNULL((1)) void DFCALL
time_inplace_month2nanosecond(Dee_int128_t *__restrict p_value) {
	Dee_int128_t year;
	uint8_t month_in_year;
	struct month const *months_of_year;
	uint64_t month_start;
	__hybrid_int128_floordivmod8(*p_value, MONTHS_PER_YEAR, year, month_in_year);
	ASSERT(month_in_year < MONTHS_PER_YEAR);
	months_of_year = month_info_for_year(&year);
	month_start    = month_getstart(&months_of_year[month_in_year]);
	time_inplace_year2nanosecond(&year);
	__hybrid_int128_add64(year, month_start);
	*p_value = year;
}

INTERN WUNUSED NONNULL((1)) uint8_t DFCALL
DeeTime_GetRepr8(DeeTimeObject const *__restrict self, uint8_t repr) {
	Dee_int128_t result;
	_DeeTime_GetRepr(&result, self, repr);
	return __hybrid_int128_get8(result);
}

INTERN WUNUSED NONNULL((1)) uint32_t DFCALL
DeeTime_GetRepr32(DeeTimeObject const *__restrict self, uint8_t repr) {
	Dee_int128_t result;
	_DeeTime_GetRepr(&result, self, repr);
	return __hybrid_int128_get32(result);
}

INTERN WUNUSED NONNULL((1)) Dee_int128_t DFCALL
DeeTime_GetRepr(DeeTimeObject const *__restrict self,
                uint8_t repr) {
	Dee_int128_t result;
	_DeeTime_GetRepr(&result, self, repr);
	return result;
}


/* Return the integer value for the specified representation of `self' */
INTERN NONNULL((1, 2)) void DFCALL
_DeeTime_GetRepr(Dee_int128_t *__restrict p_result,
                 DeeTimeObject const *__restrict self,
                 uint8_t repr) {
	Dee_int128_t nanoseconds;

	/* Check for special case: `self' represents months */
	if unlikely(self->t_type == TIME_TYPE_MONTHS) {
		switch (repr) {

		case TIME_REPR_NANOSECOND:
		case TIME_REPR_MICROSECOND:
		case TIME_REPR_MILLISECOND:
		case TIME_REPR_SECOND:
		case TIME_REPR_MINUTE:
		case TIME_REPR_HOUR:
		case TIME_REPR_WDAY:
		case TIME_REPR_MWEEK:
		case TIME_REPR_MDAY:
			__hybrid_int128_setzero(*p_result);
			return;

		case TIME_REPR_MONTH:
			*p_result = self->t_months;
			__hybrid_int128_floormod8(*p_result, MONTHS_PER_YEAR);
			__hybrid_int128_inc(*p_result); /* 1-based */
			return;

		case TIME_REPR_MONTHS:
			*p_result = self->t_months;
			return;

		case TIME_REPR_YEARS:
			*p_result = self->t_months;
			__hybrid_int128_floordiv8(*p_result, MONTHS_PER_YEAR);
			return;

		case TIME_REPR_DECADES:
			*p_result = self->t_months;
			__hybrid_int128_floordiv8(*p_result, MONTHS_PER_DECADE);
			return;

		case TIME_REPR_CENTURIES:
			*p_result = self->t_months;
			__hybrid_int128_floordiv16(*p_result, MONTHS_PER_CENTURY);
			return;

		case TIME_REPR_MILLENNIA:
			*p_result = self->t_months;
			__hybrid_int128_floordiv16(*p_result, MONTHS_PER_MILLENNIUM);
			return;

		default: break;
		}
		nanoseconds = self->t_months;
		time_inplace_month2nanosecond(&nanoseconds);
	} else {
		nanoseconds = self->t_nanos;
	}

	switch (repr) {

	case TIME_REPR_NANOSECOND: /* Nanosecond in Second */
		*p_result = nanoseconds;
		__hybrid_int128_floormod32(*p_result, NANOSECONDS_PER_SECOND);
		break;

	case TIME_REPR_MICROSECOND: /* Microsecond in Second */
		*p_result = nanoseconds;
		__hybrid_int128_floormod32(*p_result, NANOSECONDS_PER_SECOND);
		__hybrid_int128_floordiv16(*p_result, NANOSECONDS_PER_MICROSECOND);
		break;

	case TIME_REPR_MILLISECOND: /* Millisecond in Second */
		*p_result = nanoseconds;
		__hybrid_int128_floormod32(*p_result, NANOSECONDS_PER_SECOND);
		__hybrid_int128_floordiv32(*p_result, NANOSECONDS_PER_MILLISECOND);
		break;

	case TIME_REPR_SECOND: /* Second in Minute */
		*p_result = nanoseconds;
		__hybrid_int128_floormod64(*p_result, NANOSECONDS_PER_MINUTE);
		__hybrid_int128_floordiv32(*p_result, NANOSECONDS_PER_SECOND);
		break;

	case TIME_REPR_MINUTE: /* Minute in Hour */
		*p_result = nanoseconds;
		__hybrid_int128_floormod64(*p_result, NANOSECONDS_PER_HOUR);
		__hybrid_int128_floordiv64(*p_result, NANOSECONDS_PER_MINUTE);
		break;

	case TIME_REPR_HOUR: /* Hour in Day */
		*p_result = nanoseconds;
		__hybrid_int128_floormod64(*p_result, NANOSECONDS_PER_DAY);
		__hybrid_int128_floordiv64(*p_result, NANOSECONDS_PER_HOUR);
		break;

	case TIME_REPR_WDAY: /* Day in Week */
		*p_result = nanoseconds;
		__hybrid_int128_sub64(*p_result, NANOSECONDS_PER_DAY);
		__hybrid_int128_floormod64(*p_result, NANOSECONDS_PER_WEEK);
		__hybrid_int128_floordiv64(*p_result, NANOSECONDS_PER_DAY);
		break;

	case TIME_REPR_MWEEK: { /* Week in Month */
		Dee_int128_t start_of_month = nanoseconds;
		Dee_int128_t start_of_week0;
		time_inplace_nanosecond2month(&start_of_month);
		time_inplace_month2nanosecond(&start_of_month);
		start_of_week0 = start_of_month;
		time_inplace_nanoseconds2weeks(&start_of_week0);
		time_inplace_weeks2nanoseconds(&start_of_week0);
		__hybrid_int128_add64(start_of_week0, NANOSECONDS_PER_DAY); /* Because 01-01-0000 was a saturday */
		*p_result = nanoseconds;
		__hybrid_int128_sub128(*p_result, start_of_week0);
		__hybrid_int128_floordiv64(*p_result, NANOSECONDS_PER_WEEK);
		if (__hybrid_int128_eq128(start_of_week0, start_of_month))
			__hybrid_int128_inc(*p_result); /* There is no week#0 */
	}	break;

	case TIME_REPR_MONTH: /* Month in Year */
		time_get_month(&nanoseconds, p_result);
		__hybrid_int128_floormod8(*p_result, MONTHS_PER_YEAR);
		__hybrid_int128_inc(*p_result); /* 1-based */
		break;

	case TIME_REPR_MONTHS: /* Total months */
		time_get_months(&nanoseconds, p_result);
		break;

	case TIME_REPR_YEAR: /* Year */
		time_get_year(&nanoseconds, p_result);
		break;

	case TIME_REPR_YEARS: /* Total years */
		time_get_years(&nanoseconds, p_result);
		break;

	case TIME_REPR_DECADE: /* Decade */
		time_get_decade(&nanoseconds, p_result);
		break;

	case TIME_REPR_DECADES: /* Total decades */
		time_get_decades(&nanoseconds, p_result);
		break;

	case TIME_REPR_CENTURY: /* Century */
		time_get_century(&nanoseconds, p_result);
		break;

	case TIME_REPR_CENTURIES: /* Total centuries */
		time_get_centuries(&nanoseconds, p_result);
		break;

	case TIME_REPR_MILLENNIUM: /* Millennium */
		time_get_millennium(&nanoseconds, p_result);
		break;

	case TIME_REPR_MILLENNIA: /* Total millennia */
		time_get_millennia(&nanoseconds, p_result);
		break;

	case TIME_REPR_MDAY: { /* Day in Month */
		Dee_int128_t start_of_month = nanoseconds;
		time_inplace_nanosecond2month(&start_of_month);
		time_inplace_month2nanosecond(&start_of_month);
		*p_result = nanoseconds;
		__hybrid_int128_sub128(*p_result, start_of_month);
		__hybrid_int128_floordiv64(*p_result, NANOSECONDS_PER_DAY);
		__hybrid_int128_inc(*p_result); /* 1-based */
	}	break;

	case TIME_REPR_YDAY: { /* Day in Year */
		Dee_int128_t start_of_year = nanoseconds;
		time_inplace_nanosecond2year(&start_of_year);
		time_inplace_year2nanosecond(&start_of_year);
		*p_result = nanoseconds;
		__hybrid_int128_sub128(*p_result, start_of_year);
		__hybrid_int128_floordiv64(*p_result, NANOSECONDS_PER_DAY);
		__hybrid_int128_inc(*p_result);
	}	break;

	case TIME_REPR_YWEEK: { /* Week in Year */
		Dee_int128_t start_of_year = nanoseconds;
		Dee_int128_t start_of_week0;
		time_inplace_nanosecond2year(&start_of_year);
		time_inplace_year2nanosecond(&start_of_year);
		start_of_week0 = start_of_year;
		time_inplace_nanoseconds2weeks(&start_of_week0);
		time_inplace_weeks2nanoseconds(&start_of_week0);
		__hybrid_int128_add64(start_of_week0, NANOSECONDS_PER_DAY); /* Because 01-01-0000 was a saturday */
		*p_result = nanoseconds;
		__hybrid_int128_sub128(*p_result, start_of_week0);
		__hybrid_int128_floordiv64(*p_result, NANOSECONDS_PER_WEEK);
		if (__hybrid_int128_eq128(start_of_week0, start_of_year))
			__hybrid_int128_inc(*p_result); /* There is no week#0 */
	}	break;

	case TIME_REPR_NANOSECONDS: /* Total nanoseconds */
		time_get_nanoseconds(&nanoseconds, p_result);
		break;

	case TIME_REPR_MICROSECONDS: /* Total microseconds */
		time_get_microseconds(&nanoseconds, p_result);
		break;

	case TIME_REPR_MILLISECONDS: /* Total milliseconds */
		time_get_milliseconds(&nanoseconds, p_result);
		break;

	case TIME_REPR_SECONDS: /* Total seconds */
		time_get_seconds(&nanoseconds, p_result);
		break;

	case TIME_REPR_MINUTES: /* Total minutes */
		time_get_minutes(&nanoseconds, p_result);
		break;

	case TIME_REPR_HOURS: /* Total hours */
		time_get_hours(&nanoseconds, p_result);
		break;

	case TIME_REPR_DAYS: /* Total days */
		time_get_days(&nanoseconds, p_result);
		break;

	case TIME_REPR_WEEKS: /* Total weeks */
		time_get_weeks(&nanoseconds, p_result);
		break;

	default: __builtin_unreachable();
	}
}

LOCAL NONNULL((1)) void DFCALL
DeeTime_MakeTimestamp(DeeTimeObject *__restrict self) {
	/* Ensure that `self' uses nano-seconds, and change it to a timestamp */
	if unlikely(self->t_typekind != TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP)) {
		if (self->t_type == TIME_TYPE_MONTHS)
			time_inplace_months2nanoseconds(&self->t_nanos);
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	}
}

/* Set the integer value for the specified representation of `self' */
INTERN NONNULL((1, 2)) void DFCALL
DeeTime_SetRepr(DeeTimeObject *__restrict self,
                Dee_int128_t const *__restrict p_value,
                uint8_t repr) {
	switch (repr) {

	case TIME_REPR_MONTHS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
		self->t_months   = *p_value;
		return;

	case TIME_REPR_YEARS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
		self->t_months   = *p_value;
		__hybrid_int128_mul8(self->t_months, MONTHS_PER_YEAR);
		return;

	case TIME_REPR_DECADES:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
		self->t_months   = *p_value;
		__hybrid_int128_mul8(self->t_months, MONTHS_PER_DECADE);
		return;

	case TIME_REPR_CENTURIES:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
		self->t_months   = *p_value;
		__hybrid_int128_mul16(self->t_months, MONTHS_PER_CENTURY);
		return;

	case TIME_REPR_MILLENNIA:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
		self->t_months   = *p_value;
		__hybrid_int128_mul16(self->t_months, MONTHS_PER_MILLENNIUM);
		return;

	case TIME_REPR_NANOSECONDS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		return;

	case TIME_REPR_MICROSECONDS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		time_inplace_microseconds2nanoseconds(&self->t_nanos);
		return;

	case TIME_REPR_MILLISECONDS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		time_inplace_milliseconds2nanoseconds(&self->t_nanos);
		return;

	case TIME_REPR_SECONDS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		time_inplace_seconds2nanoseconds(&self->t_nanos);
		return;

	case TIME_REPR_MINUTES:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		time_inplace_minutes2nanoseconds(&self->t_nanos);
		return;

	case TIME_REPR_HOURS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		time_inplace_hours2nanoseconds(&self->t_nanos);
		return;

	case TIME_REPR_DAYS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		time_inplace_days2nanoseconds(&self->t_nanos);
		return;

	case TIME_REPR_WEEKS:
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		self->t_nanos    = *p_value;
		time_inplace_weeks2nanoseconds(&self->t_nanos);
		return;

	default: break;
	}

	DeeTime_MakeTimestamp(self);
	switch (repr) {

	case TIME_REPR_NANOSECOND: /* Nanosecond in Second */
		time_inplace_clearmod32(&self->t_nanos, NANOSECONDS_PER_SECOND);
		__hybrid_int128_add128(self->t_nanos, *p_value);
		break;

	case TIME_REPR_MICROSECOND: { /* Microsecond in Second */
		Dee_int128_t value = *p_value;
		__hybrid_int128_mul16(value, NANOSECONDS_PER_MICROSECOND);
		time_inplace_clearmod32(&self->t_nanos, NANOSECONDS_PER_SECOND);
		__hybrid_int128_add128(self->t_nanos, value);
	}	break;

	case TIME_REPR_MILLISECOND: { /* Millisecond in Second */
		Dee_int128_t value = *p_value;
		__hybrid_int128_mul32(value, NANOSECONDS_PER_MILLISECOND);
		time_inplace_clearmod32(&self->t_nanos, NANOSECONDS_PER_SECOND);
		__hybrid_int128_add128(self->t_nanos, value);
	}	break;

	case TIME_REPR_SECOND: { /* Second in Minute */
		Dee_int128_t value = *p_value;
		__hybrid_int128_mul32(value, NANOSECONDS_PER_SECOND);
		time_inplace_clearmod64_keepmod64(&self->t_nanos,
		                                  NANOSECONDS_PER_MINUTE,
		                                  NANOSECONDS_PER_SECOND);
		__hybrid_int128_add128(self->t_nanos, value);
	}	break;

	case TIME_REPR_MINUTE: { /* Minute in Hour */
		Dee_int128_t value = *p_value;
		__hybrid_int128_mul64(value, NANOSECONDS_PER_MINUTE);
		time_inplace_clearmod64_keepmod64(&self->t_nanos,
		                                  NANOSECONDS_PER_HOUR,
		                                  NANOSECONDS_PER_MINUTE);
		__hybrid_int128_add128(self->t_nanos, value);
	}	break;

	case TIME_REPR_HOUR: { /* Hour in Day */
		Dee_int128_t value = *p_value;
		__hybrid_int128_mul64(value, NANOSECONDS_PER_HOUR);
		time_inplace_clearmod64_keepmod64(&self->t_nanos,
		                                  NANOSECONDS_PER_DAY,
		                                  NANOSECONDS_PER_HOUR);
		__hybrid_int128_add128(self->t_nanos, value);
	}	break;

	case TIME_REPR_WDAY: { /* Day in Week */
		Dee_int128_t value = *p_value;
		__hybrid_int128_mul64(value, NANOSECONDS_PER_DAY);
		time_inplace_clearmod64_keepmod64(&self->t_nanos,
		                                  NANOSECONDS_PER_WEEK,
		                                  NANOSECONDS_PER_DAY);
		__hybrid_int128_add128(self->t_nanos, value);
	}	break;

	case TIME_REPR_MWEEK: { /* Week in Month */
		Dee_int128_t delta, old_value;
		_DeeTime_GetRepr(&old_value, self, TIME_REPR_MWEEK);
		delta = *p_value;
		__hybrid_int128_sub128(delta, old_value);
		__hybrid_int128_mul64(delta, NANOSECONDS_PER_WEEK);
		__hybrid_int128_add128(self->t_nanos, delta);
	}	break;

	case TIME_REPR_MONTH: { /* Month in Year */
		Dee_int128_t old_months, nano_since_start_of_month, start_of_month;
		Dee_int128_t new_months, month_delta, new_year;
		Dee_int128_t new_month_length;
		uint16_t new_month_length_in_days;
		uint8_t old_month, new_month;
		struct month const *new_year_months;

		/* Figure out the accurate month number since "0" */
		old_months = self->t_nanos;
		time_inplace_nanosecond2month(&old_months);

		/* Figure out the # of days since the start of the relevant month. */
		start_of_month = old_months;
		time_inplace_month2nanosecond(&start_of_month);
		nano_since_start_of_month = self->t_nanos;
		__hybrid_int128_sub128(nano_since_start_of_month, start_of_month);

		/* Figure out the month within the relevant year, and determine delta. */
		__hybrid_int128_floormod8_r(old_months, MONTHS_PER_YEAR, old_month);
		++old_month; /* 1-based */
		month_delta = *p_value;
		__hybrid_int128_sub8(month_delta, old_month);

		/* Apply delta to months */
		new_months = old_months;
		__hybrid_int128_add128(new_months, month_delta);

		/* Figure out the new, relevant year+month */
		__hybrid_int128_floordivmod8(new_months, MONTHS_PER_YEAR, new_year, new_month);
		new_year_months = month_info_for_year(&new_year);
		new_month_length_in_days = month_getlen(&new_year_months[new_month]);
		__hybrid_uint128_set16(*(Dee_uint128_t *)&new_month_length, new_month_length_in_days);
		time_inplace_days2nanoseconds(&new_month_length);

		/* Clamp day-of-month to the max valid value in the context of the new month.
		 * This is needed for:
		 * >> local t = Time(year: 1976, month: 1, day: 31);
		 * >> t.month = 2;
		 * >> print repr t;
		 * >> assert t == Time(year: 1976, month: 2, day: 29, hour: 23, minute: 59, second: 59, nanosecond: 999999999);
		 * Without this, the date's month would unexpectedly be in March instead. */
		if (__hybrid_int128_ge128(nano_since_start_of_month, new_month_length)) {
			nano_since_start_of_month = new_month_length;
			__hybrid_int128_dec(nano_since_start_of_month);
		}

		/* Combine `new_months' and `nano_since_start_of_month' to form a new timestamp. */
		time_inplace_month2nanosecond(&new_months);
		__hybrid_int128_add128(new_months, nano_since_start_of_month);
		self->t_nanos = new_months;
	}	break;

	case TIME_REPR_YEAR: { /* Year */
		Dee_int128_t start_of_year = self->t_nanos;
		Dee_int128_t time_since_start_of_year;
		time_inplace_nanosecond2year(&start_of_year);
		time_inplace_year2nanosecond(&start_of_year);
		time_since_start_of_year = self->t_nanos;
		__hybrid_int128_sub128(time_since_start_of_year, start_of_year);
		self->t_nanos = *p_value;
		time_inplace_year2nanosecond(&self->t_nanos);
		__hybrid_int128_add128(self->t_nanos, time_since_start_of_year);
	}	break;

	case TIME_REPR_DECADE: { /* Decade */
		Dee_int128_t start_of_decade = self->t_nanos;
		Dee_int128_t time_since_start_of_decade;
		time_inplace_nanosecond2decade(&start_of_decade);
		time_inplace_decade2nanosecond(&start_of_decade);
		time_since_start_of_decade = self->t_nanos;
		__hybrid_int128_sub128(time_since_start_of_decade, start_of_decade);
		self->t_nanos = *p_value;
		time_inplace_decade2nanosecond(&self->t_nanos);
		__hybrid_int128_add128(self->t_nanos, time_since_start_of_decade);
	}	break;

	case TIME_REPR_CENTURY: { /* Century */
		Dee_int128_t start_of_century = self->t_nanos;
		Dee_int128_t time_since_start_of_century;
		time_inplace_nanosecond2century(&start_of_century);
		time_inplace_century2nanosecond(&start_of_century);
		time_since_start_of_century = self->t_nanos;
		__hybrid_int128_sub128(time_since_start_of_century, start_of_century);
		self->t_nanos = *p_value;
		time_inplace_century2nanosecond(&self->t_nanos);
		__hybrid_int128_add128(self->t_nanos, time_since_start_of_century);
	}	break;

	case TIME_REPR_MILLENNIUM: { /* Millennium */
		Dee_int128_t start_of_millennium = self->t_nanos;
		Dee_int128_t time_since_start_of_millennium;
		time_inplace_nanosecond2millennium(&start_of_millennium);
		time_inplace_millennium2nanosecond(&start_of_millennium);
		time_since_start_of_millennium = self->t_nanos;
		__hybrid_int128_sub128(time_since_start_of_millennium, start_of_millennium);
		self->t_nanos = *p_value;
		time_inplace_millennium2nanosecond(&self->t_nanos);
		__hybrid_int128_add128(self->t_nanos, time_since_start_of_millennium);
	}	break;

	case TIME_REPR_MDAY: { /* Day in Month */
		Dee_int128_t old_day_of_month, delta_days;
		_DeeTime_GetRepr(&old_day_of_month, self, TIME_REPR_MDAY);
		delta_days = *p_value;
		__hybrid_int128_sub128(delta_days, old_day_of_month);
		__hybrid_int128_mul64(delta_days, NANOSECONDS_PER_DAY);
		__hybrid_int128_add128(self->t_nanos, delta_days);
	}	break;

	case TIME_REPR_YDAY: { /* Day in Year */
		Dee_int128_t old_day_of_year, delta_days;
		_DeeTime_GetRepr(&old_day_of_year, self, TIME_REPR_YDAY);
		delta_days = *p_value;
		__hybrid_int128_sub128(delta_days, old_day_of_year);
		__hybrid_int128_mul64(delta_days, NANOSECONDS_PER_DAY);
		__hybrid_int128_add128(self->t_nanos, delta_days);
	}	break;

	case TIME_REPR_YWEEK: { /* Week in Year */
		Dee_int128_t old_week_of_year, delta_weeks;
		_DeeTime_GetRepr(&old_week_of_year, self, TIME_REPR_YDAY);
		delta_weeks = *p_value;
		__hybrid_int128_sub128(delta_weeks, old_week_of_year);
		__hybrid_int128_mul64(delta_weeks, NANOSECONDS_PER_WEEK);
		__hybrid_int128_add128(self->t_nanos, delta_weeks);
	}	break;

	default: __builtin_unreachable();
	}
}


/* Time representation selection
 * >> x = now();
 * >> print x.hour;      // 19
 * >> print x.hour+1;    // 20
 * >> print type x.hour; // time
 */
struct repr_name {
	char    name[15]; /* Name */
	uint8_t repr;     /* One of `TIME_REPR_*' */
};

/* Representation descriptor database. */
PRIVATE struct repr_name const repr_desc[] = {
	{ "nanosecond", TIME_REPR_NANOSECOND },
	{ "microsecond", TIME_REPR_MICROSECOND },
	{ "millisecond", TIME_REPR_MILLISECOND },
	{ "second", TIME_REPR_SECOND },
	{ "minute", TIME_REPR_MINUTE },
	{ "hour", TIME_REPR_HOUR },
	{ "wday", TIME_REPR_WDAY },
	{ "mweek", TIME_REPR_MWEEK },
	{ "month", TIME_REPR_MONTH },
	{ "year", TIME_REPR_YEAR },
	{ "decade", TIME_REPR_DECADE },
	{ "century", TIME_REPR_CENTURY },
	{ "millennium", TIME_REPR_MILLENNIUM },
	{ "mday", TIME_REPR_MDAY },
	{ "yday", TIME_REPR_YDAY },
	{ "yweek", TIME_REPR_YWEEK },
	{ "nanoseconds", TIME_REPR_NANOSECONDS },
	{ "microseconds", TIME_REPR_MICROSECONDS },
	{ "milliseconds", TIME_REPR_MILLISECONDS },
	{ "seconds", TIME_REPR_SECONDS },
	{ "minutes", TIME_REPR_MINUTES },
	{ "hours", TIME_REPR_HOURS },
	{ "days", TIME_REPR_DAYS },
	{ "weeks", TIME_REPR_WEEKS },
	{ "months", TIME_REPR_MONTHS },
	{ "years", TIME_REPR_YEARS },
	{ "decades", TIME_REPR_DECADES },
	{ "centuries", TIME_REPR_CENTURIES },
	{ "millennia", TIME_REPR_MILLENNIA },
};

PRIVATE WUNUSED NONNULL((1)) uint8_t DCALL
get_repr_id(char const *__restrict name, size_t length) {
	uint8_t result = TIME_REPR_INVALID;
	struct repr_name const *iter;
	if (length < COMPILER_LENOF(repr_desc[0].name)) {
		for (iter = repr_desc;
		     iter < COMPILER_ENDOF(repr_desc); ++iter) {
			if (MEMCASEEQ(iter->name, name, length * sizeof(char))) {
				result = iter->repr;
				break;
			}
		}
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1, 3, 4)) Dee_ssize_t DCALL
time_format(Dee_formatprinter_t printer, void *arg,
            char const *__restrict format,
            DeeTimeObject *__restrict self) {
	/* TODO: re-write this function from scratch! */
#define print(p, s) DO(err, DeeFormat_Print(printer, arg, p, s))
#define printf(...) DO(err, DeeFormat_Printf(printer, arg, __VA_ARGS__))
	Dee_ssize_t temp, result = 0;
	Dee_int128_t number;
	char const *text;
	for (;;) {
		char ch = *format++;
		switch (ch) {

		case '\0':
			goto done;

		case '%':
			ch = *format++;
			switch (ch) {
				/* TODO: @begin locale_dependent */

			case 'a':
				text = get_wday_abbr(DeeTime_GetRepr8(self, TIME_REPR_WDAY));
print_text:
				print(text, strlen(text));
				break;

			case 'A':
				text = get_wday_full(DeeTime_GetRepr8(self, TIME_REPR_WDAY));
				goto print_text;

			case 'h':
			case 'b':
				text = get_month_abbr(DeeTime_GetRepr8(self, TIME_REPR_MONTH) - 1);
				goto print_text;

			case 'B':
				text = get_month_full(DeeTime_GetRepr8(self, TIME_REPR_MONTH) - 1);
				goto print_text;

			case 'c':
				printf("%s %s %0.2" PRFu8 " %0.2" PRFu8 ":%0.2" PRFu8 ":%0.2" PRFu8 " %" PRFd128,
				       get_wday_abbr(DeeTime_GetRepr8(self, TIME_REPR_WDAY)),
				       get_month_abbr(DeeTime_GetRepr8(self, TIME_REPR_MONTH) - 1),
				       DeeTime_GetRepr8(self, TIME_REPR_MDAY),
				       DeeTime_GetRepr8(self, TIME_REPR_HOUR),
				       DeeTime_GetRepr8(self, TIME_REPR_MINUTE),
				       DeeTime_GetRepr8(self, TIME_REPR_SECOND),
				       DeeTime_GetRepr(self, TIME_REPR_YEAR));
				break;

			case 'x':
				printf("%0.2" PRFu8 "/%0.2" PRFu8 "/%0.2" PRFu128,
				       DeeTime_GetRepr8(self, TIME_REPR_MONTH),
				       DeeTime_GetRepr8(self, TIME_REPR_MDAY),
				       DeeTime_GetRepr(self, TIME_REPR_YEAR));
				break;

			case 'X':
				printf("%0.2" PRFu8 ":%0.2" PRFu8 ":%0.2" PRFu8,
				       DeeTime_GetRepr8(self, TIME_REPR_HOUR),
				       DeeTime_GetRepr8(self, TIME_REPR_MINUTE),
				       DeeTime_GetRepr8(self, TIME_REPR_SECOND));
				break;

			case 'z':
				/* TODO: ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100) |
				 *       If timezone cannot be determined, no characters +100 */
				break;

			case 'Z':
				/* TODO: Timezone name or abbreviation * |
				 *       If timezone cannot be determined, no characters CDT */
				break;

				/* TODO: @end locale_dependent */
			case 'C':
				_DeeTime_GetRepr(&number, self, TIME_REPR_YEAR);
				__hybrid_int128_floordiv8(number, 100);
				__hybrid_int128_floormod8(number, 100);
print_number_2:
				printf("%0.2" PRFd128, number);
				break;

			case 'd':
				_DeeTime_GetRepr(&number, self, TIME_REPR_MDAY);
				goto print_number_2;

			case 'D':
				_DeeTime_GetRepr(&number, self, TIME_REPR_MDAY);
				__hybrid_int128_floormod8(number, 100);
				printf("%0.2" PRFu8 "/%0.2" PRFu8 "/%0.2" PRFu8,
				       DeeTime_GetRepr8(self, TIME_REPR_MONTH),
				       DeeTime_GetRepr8(self, TIME_REPR_MDAY),
				       __hybrid_int128_get8(number));
				break;

			case 'e':
				_DeeTime_GetRepr(&number, self, TIME_REPR_MDAY);
/*print_number_2_spc:*/
				printf("%02" PRFu128, number);
				break;

			case 'F':
				printf("%0.4" PRFd128 "-%0.2u-%0.2u",
				       DeeTime_GetRepr(self, TIME_REPR_YEAR),
				       DeeTime_GetRepr8(self, TIME_REPR_MONTH),
				       DeeTime_GetRepr8(self, TIME_REPR_MDAY));
				break;

			case 'H':
				_DeeTime_GetRepr(&number, self, TIME_REPR_HOUR);
				goto print_number_2;

			case 'I':
				_DeeTime_GetRepr(&number, self, TIME_REPR_HOUR);
				__hybrid_int128_floormod8(number, 12);
				goto print_number_2;

			case 'j':
				_DeeTime_GetRepr(&number, self, TIME_REPR_YDAY);
/*print_number_3_spc:*/
				printf("%3" PRFd128, number);
				break;

			case 'm':
				_DeeTime_GetRepr(&number, self, TIME_REPR_MONTH);
				goto print_number_2;

			case 'M':
				_DeeTime_GetRepr(&number, self, TIME_REPR_MINUTE);
				goto print_number_2;

			case 'p':
				_DeeTime_GetRepr(&number, self, TIME_REPR_HOUR);
				__hybrid_int128_floordiv8(number, 12);
				text = am_pm[__hybrid_int128_get8(number)];
				goto print_text;

			case 'r': {
				Dee_int128_t ampm;
				_DeeTime_GetRepr(&number, self, TIME_REPR_HOUR);
				ampm = number;
				__hybrid_int128_floordiv8(ampm, 12);
				__hybrid_int128_floormod8(number, 12);
				printf("%0.2" PRFu8 ":%0.2" PRFu8 ":%0.2" PRFu8 " %s",
				       __hybrid_int128_get8(number),
				       DeeTime_GetRepr8(self, TIME_REPR_MINUTE),
				       DeeTime_GetRepr8(self, TIME_REPR_SECOND),
				       am_pm_lower[__hybrid_int128_get8(ampm)]);
			}	break;

			case 'R':
				printf("%0.2" PRFu8 ":%0.2" PRFu8,
				       DeeTime_GetRepr8(self, TIME_REPR_HOUR),
				       DeeTime_GetRepr8(self, TIME_REPR_MINUTE));
				break;

			case 'S':
				_DeeTime_GetRepr(&number, self, TIME_REPR_SECOND);
				goto print_number_2;

			case 'T':
				printf("%0.2" PRFu8 ":%0.2" PRFu8 ":%0.2" PRFu8,
				       DeeTime_GetRepr8(self, TIME_REPR_HOUR),
				       DeeTime_GetRepr8(self, TIME_REPR_MINUTE),
				       DeeTime_GetRepr8(self, TIME_REPR_SECOND));
				break;

			case 'u':
				_DeeTime_GetRepr(&number, self, TIME_REPR_WDAY);
				__hybrid_int128_add8(number, 6);
				__hybrid_int128_floormod8(number, 7);
				__hybrid_int128_inc(number);
				goto print_number_2;

			case 'w':
				_DeeTime_GetRepr(&number, self, TIME_REPR_WDAY);
				goto print_number_2;

			case 'y':
				_DeeTime_GetRepr(&number, self, TIME_REPR_YEAR);
				__hybrid_int128_floormod8(number, 100);
				goto print_number_2;

			case 'Y':
				_DeeTime_GetRepr(&number, self, TIME_REPR_YEAR);
				printf("%" PRFd128, number);
				break;

				/* I don't understand this week-based stuff.
				 * I read the wikipedia article, but I still don't really get it.
				 * >> So this might be supported in the future when I understand it...
				 * %g  Week-based year, last two digits (00-99) 01
				 * %G  Week-based year 2001
				 * %U  Week number with the first Sunday as the first day of week one (00-53) 33
				 * %V  ISO 8601 week number (00-53) 34
				 * %W  Week number with the first Monday as the first day of week one (00-53) 34 */

			case 'n':
				print("\n", 1);
				break;

			case 't':
				print("\t", 1);
				break;

			case '%':
			case '\0':
				print("%", 1);
				if (!ch)
					goto done;
				break;

			case '[': {
				char const *tag_begin, *tag_end, *mode_begin, *mode_end;
				unsigned int bracket_recursion = 1;
				int repr_mode, width = 0;
				uint8_t attribute_id;
				/* Extended formatting */
				mode_end = mode_begin = tag_begin = format;
				for (;;) {
					ch = *format++;
					if (ch == ']') {
						if (!--bracket_recursion) {
							tag_end = format - 1;
							break;
						}
					} else if (ch == '[') {
						++bracket_recursion;
					} else if (ch == ':' && bracket_recursion == 1) {
						mode_end = format - 1, tag_begin = format;
					} else if (!ch) {
						tag_end = format;
						break;
					}
				}
				if (mode_begin != mode_end) {
					if (*mode_begin == 'n' || *mode_begin == 's' ||
					    *mode_begin == 'S' || *mode_begin == ' ') {
						repr_mode = *mode_begin++;
					} else {
						repr_mode = 0;
					}
					/* Parse the width modifier */
					while (mode_begin != mode_end) {
						if (*mode_begin >= '0' && *mode_begin <= '9') {
							width = width * 10 + (*mode_begin - '0');
						} else {
							DeeError_Throwf(&DeeError_ValueError,
							                "Expected digits, or end of repr-mode, but got %.1q in '%%[%#$q]'",
							                mode_begin, (size_t)(tag_end - mode_begin), mode_begin);
							goto err_m1;
						}
						++mode_begin;
					}
				} else {
					repr_mode = 0;
				}
				attribute_id = get_repr_id(tag_begin, (size_t)(tag_end - tag_begin));
				if unlikely(attribute_id == TIME_REPR_INVALID) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Unknown/Invalid time attribute: %$q",
					                (size_t)(tag_end - tag_begin), tag_begin);
					goto err_m1;
				}
				_DeeTime_GetRepr(&number, self, attribute_id);
				switch (repr_mode) {
				case 's':
				case 'S': {
					char const *repr_value;
					if (attribute_id == TIME_REPR_MONTH) {
						ASSERT(__hybrid_int128_is8bit(number));
						ASSERT(__hybrid_int128_get8(number) < 12 + 1);
						repr_value = (repr_mode == 'S'
						              ? get_month_full(__hybrid_int128_get8(number) - 1)
						              : get_month_abbr(__hybrid_int128_get8(number) - 1));
					} else if likely(attribute_id == TIME_REPR_WDAY) {
						ASSERT(__hybrid_int128_is8bit(number));
						ASSERT(__hybrid_int128_get8(number) < 7);
						repr_value = (repr_mode == 'S'
						              ? get_wday_full(__hybrid_int128_get8(number))
						              : get_wday_abbr(__hybrid_int128_get8(number)));
					} else {
						DeeError_Throwf(&DeeError_ValueError,
						                "Cannot use attribute %$q with 's'/'S' in '%%[%#$q]'",
						                (size_t)(tag_end - tag_begin), tag_begin,
						                (size_t)(tag_end - mode_begin), mode_begin);
						goto err_m1;
					}
					print(repr_value, strlen(repr_value));
				}	break;

				default: {
					PRIVATE char const _suffix_values[] = "stndrdth";
#if 0
					struct DeeStringWriterFormatSpec fmt_spec = DeeStringWriterFormatSpec_INIT_BASIC(10);
					if (width) {
						fmt_spec.has_width = 1;
						fmt_spec.width     = (unsigned int)width;
						if (repr_mode != ' ')
							fmt_spec.pad_zero = 1;
					}
					if (ascii_printer_SpecWriteUInt64(printer, number, &fmt_spec) < 0)
						goto err;
#else
					printf("%" PRFd128, number);
#endif
					if (repr_mode == 'n') {
						size_t suffix_offset = 6;
						if (__hybrid_int128_is8bit(number)) {
							int8_t value = __hybrid_int128_get8(number);
							if (value < 0)
								value = -value;
							if (value >= 1 && value <= 3)
								suffix_offset = ((uint8_t)value - 1) * 2;
						}
						print(_suffix_values + suffix_offset, 2);
					}
				}	break;
				}
			}	break;

			default:
				print(format - 2, 2);
				break;
			}
			break;

		default:
/*def:*/
			/* TODO: Use a flush system here! */
			print(&ch, 1);
			break;
		}
	}
done:
	return result;
err_m1:
	temp = -1;
err:
	return temp;
#undef printf
#undef print
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_int(DeeTimeObject *__restrict self) {
	/* Return as nano-seconds */
	Dee_int128_t result;
	DeeTime_AsNano(self, &result);
	return DeeInt_NewInt128(result);
}

PRIVATE int DCALL
int128_overflow(Dee_int128_t const *__restrict value,
                unsigned int after_bits) {
	return DeeError_Throwf(&DeeError_IntegerOverflow,
	                       "%s integer overflow after %u bits in %" PRFd128,
	                       __hybrid_int128_isneg(*value)
	                       ? "negative"
	                       : "positive",
	                       after_bits,
	                       *value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_int64(DeeTimeObject *__restrict self,
           int64_t *__restrict p_result) {
	Dee_int128_t result;
	DeeTime_AsNano(self, &result);
	if unlikely(!__hybrid_int128_is64bit(result)) {
		if (__hybrid_uint128_is64bit(*(Dee_uint128_t const *)&result)) {
			*(uint64_t *)p_result = __hybrid_uint128_get64(*(Dee_uint128_t const *)&result);
			return INT_SIGNED;
		}
		return int128_overflow(&result, 64);
	}
	*p_result = __hybrid_int128_get64(result);
	return INT_SIGNED;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_int32(DeeTimeObject *__restrict self,
           int32_t *__restrict p_result) {
	Dee_int128_t result;
	DeeTime_AsNano(self, &result);
	if unlikely(!__hybrid_int128_is32bit(result)) {
		if (__hybrid_uint128_is32bit(*(Dee_uint128_t const *)&result)) {
			*(uint32_t *)p_result = __hybrid_uint128_get32(*(Dee_uint128_t const *)&result);
			return INT_SIGNED;
		}
		return int128_overflow(&result, 32);
	}
	*p_result = __hybrid_int128_get32(result);
	return INT_SIGNED;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
time_doformat_string(DeeTimeObject *__restrict self,
                     char const *__restrict format) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (time_format(&unicode_printer_print, &printer, format, self) < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_doformat(DeeTimeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("format", params: """
	char const *format;
""", docStringPrefix: "libtime");]]]*/
#define libtime_format_params "format:?Dstring"
	struct {
		char const *format;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, "s:format", &args))
		goto err;
/*[[[end]]]*/
	return time_doformat_string(self, args.format);
err:
	return NULL;
}

/* From http://stackoverflow.com/questions/5590429/calculating-daylight-savings-time-from-only-date */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
determine_isdst(DeeTimeObject *__restrict self) {
	int previousSunday;
	uint8_t month, mday, wday;
	month = DeeTime_GetRepr8(self, TIME_REPR_MONTH) - 1;

	//January, February, and December are out.
	if (month < 3 || month > 11)
		return false;

	//April to October are in
	if (month > 3 && month < 11)
		return true;
	mday = DeeTime_GetRepr8(self, TIME_REPR_MDAY);
	wday = DeeTime_GetRepr8(self, TIME_REPR_WDAY);
	previousSunday = mday - wday;

	//In march, we are DST if our previous Sunday was on or after the 8th.
	if (month == 3)
		return previousSunday >= 8;

	//In November we must be before the first Sunday to be dst.
	//That means the previous Sunday must be before the 1st.
	return previousSunday <= 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_isdst(DeeTimeObject *__restrict self) {
	return_bool(determine_isdst(self));
}

PRIVATE struct type_method tpconst time_methods[] = {
	TYPE_METHOD_F("format", &time_doformat, METHOD_FNOREFESCAPE,
	              "(" libtime_format_params ")->?Dstring\n"
	              "Format @this ?. object using a given strftime-style @format string"),
	TYPE_METHOD_F("__format__", &time_doformat, METHOD_FNOREFESCAPE,
	              "(" libtime_format_params ")->?Dstring\n"
	              "Internal alias for ?#format"),
	/* TODO: set(year?:?Dint,month?:?Dint,day?:?Dint,hour?:?Dint,minute?:?Dint,second?:?Dint,nanosecond?:?Dint)->?.
	 * Assign new value(s) for the named properties.
	 *
	 * Behaves like this:
	 * >> function set(**kwds): Time {
	 * >>     for (local key, value: kwds)
	 * >>         this.operator . (key) = value;
	 * >>     return this;
	 * >> } */

	/* TODO: with(year?:?Dint,month?:?Dint,day?:?Dint,hour?:?Dint,minute?:?Dint,second?:?Dint,nanosecond?:?Dint)->?.
	 * Copy the Time object and assign new value(s) for the named properties.
	 *
	 * Behaves like this:
	 * >> function with(**kwds): Time {
	 * >>     local result = copy this;
	 * >>     for (local key, value: kwds)
	 * >>         result.operator . (key) = value;
	 * >>     return result;
	 * >> } */
	TYPE_METHOD_END
};


#define DEFINE_LIBTIME_AS(name, repr)                           \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL          \
	time_getval_##name(DeeTimeObject *__restrict self) {        \
		Dee_int128_t value;                                     \
		_DeeTime_GetRepr(&value, self, repr);                   \
		return DeeInt_NewInt128(value);                         \
	}                                                           \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL                   \
	time_setval_##name(DeeTimeObject *self, DeeObject *value) { \
		Dee_int128_t tval;                                      \
		if (DeeObject_AsInt128(value, &tval))                   \
			goto err;                                           \
		DeeTime_SetRepr(self, &tval, repr);                     \
		return 0;                                               \
	err:                                                        \
		return -1;                                              \
	}                                                           \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                      \
	time_delval_##name(DeeTimeObject *__restrict self) {        \
		Dee_int128_t tval;                                      \
		__hybrid_int128_setzero(tval);                          \
		DeeTime_SetRepr(self, &tval, repr);                     \
		return 0;                                               \
	}
DEFINE_LIBTIME_AS(nanosecond, TIME_REPR_NANOSECOND)
DEFINE_LIBTIME_AS(microsecond, TIME_REPR_MICROSECOND)
DEFINE_LIBTIME_AS(millisecond, TIME_REPR_MILLISECOND)
DEFINE_LIBTIME_AS(second, TIME_REPR_SECOND)
DEFINE_LIBTIME_AS(minute, TIME_REPR_MINUTE)
DEFINE_LIBTIME_AS(hour, TIME_REPR_HOUR)
DEFINE_LIBTIME_AS(wday, TIME_REPR_WDAY)
DEFINE_LIBTIME_AS(mweek, TIME_REPR_MWEEK)
DEFINE_LIBTIME_AS(month, TIME_REPR_MONTH)
DEFINE_LIBTIME_AS(year, TIME_REPR_YEAR)
DEFINE_LIBTIME_AS(decade, TIME_REPR_DECADE)
DEFINE_LIBTIME_AS(century, TIME_REPR_CENTURY)
DEFINE_LIBTIME_AS(millennium, TIME_REPR_MILLENNIUM)
DEFINE_LIBTIME_AS(mday, TIME_REPR_MDAY)
DEFINE_LIBTIME_AS(yday, TIME_REPR_YDAY)
DEFINE_LIBTIME_AS(yweek, TIME_REPR_YWEEK)
DEFINE_LIBTIME_AS(nanoseconds, TIME_REPR_NANOSECONDS)
DEFINE_LIBTIME_AS(microseconds, TIME_REPR_MICROSECONDS)
DEFINE_LIBTIME_AS(milliseconds, TIME_REPR_MILLISECONDS)
DEFINE_LIBTIME_AS(seconds, TIME_REPR_SECONDS)
DEFINE_LIBTIME_AS(minutes, TIME_REPR_MINUTES)
DEFINE_LIBTIME_AS(hours, TIME_REPR_HOURS)
DEFINE_LIBTIME_AS(days, TIME_REPR_DAYS)
DEFINE_LIBTIME_AS(weeks, TIME_REPR_WEEKS)
DEFINE_LIBTIME_AS(months, TIME_REPR_MONTHS)
DEFINE_LIBTIME_AS(years, TIME_REPR_YEARS)
DEFINE_LIBTIME_AS(decades, TIME_REPR_DECADES)
DEFINE_LIBTIME_AS(centuries, TIME_REPR_CENTURIES)
DEFINE_LIBTIME_AS(millennia, TIME_REPR_MILLENNIA)
#undef DEFINE_LIBTIME_AS

PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_timepart_get(DeeTimeObject *__restrict self) {
	DREF DeeTimeObject *result;
	if unlikely(!DeeTime_IsTimestamp(self))
		goto err_canot_get_timepart_of_non_timestamp;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_type  = self->t_type;
	result->t_kind  = TIME_KIND_TIMESTAMP;
	result->t_nanos = self->t_nanos;
	if likely(result->t_type == TIME_TYPE_NANOSECONDS) {
		__hybrid_int128_floormod64(result->t_nanos, NANOSECONDS_PER_DAY);
	} else {
		__hybrid_int128_setzero(result->t_months);
	}
	return result;
err_canot_get_timepart_of_non_timestamp:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot get time-part of non-timestamp time object %r",
	                self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_timepart_set(DeeTimeObject *self,
                  DeeObject *value) {
	Dee_int128_t addend;
	if (DeeObject_AsInt128(value, &addend))
		goto err;
	DeeTime_MakeNano(self);
	time_inplace_clearmod64(&self->t_nanos, NANOSECONDS_PER_DAY);
	__hybrid_int128_floormod64(addend, NANOSECONDS_PER_DAY);
	__hybrid_int128_add64(self->t_nanos, __hybrid_int128_get64(addend));
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
time_timepart_del(DeeTimeObject *__restrict self) {
	return time_timepart_set(self, Dee_None);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_datepart_get(DeeTimeObject *__restrict self) {
	DREF DeeTimeObject *result;
	if unlikely(!DeeTime_IsTimestamp(self))
		goto err_canot_get_datepart_of_non_timestamp;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_type  = self->t_type;
	result->t_kind  = TIME_KIND_TIMESTAMP;
	result->t_nanos = self->t_nanos;
	if (self->t_typekind == TIME_TYPE_NANOSECONDS)
		time_inplace_clearmod64(&result->t_nanos, NANOSECONDS_PER_DAY);
	return result;
err_canot_get_datepart_of_non_timestamp:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot get date-part of non-timestamp time object %r",
	                self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_datepart_set(DeeTimeObject *self,
                  DeeObject *value) {
	Dee_int128_t addend;
	if (DeeObject_AsInt128(value, &addend))
		goto err;
	if (self->t_type == TIME_TYPE_MONTHS) {
		self->t_type = TIME_TYPE_NANOSECONDS;
		__hybrid_int128_setzero(self->t_nanos);
	}
	time_inplace_clearmod64(&addend, NANOSECONDS_PER_DAY);
	__hybrid_int128_floormod64(self->t_nanos, NANOSECONDS_PER_DAY);
	__hybrid_int128_add128(self->t_nanos, addend);
	self->t_kind = TIME_KIND_TIMESTAMP;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
time_datepart_del(DeeTimeObject *__restrict self) {
	return time_datepart_set(self, Dee_None);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_get_time_t(DeeTimeObject *__restrict self) {
	Dee_int128_t result;
	DeeTime_AsNano(self, &result);
	__hybrid_int128_floordiv32(result, NANOSECONDS_PER_SECOND);
	__hybrid_int128_sub64(result, SECONDS_01_01_1970);
	return DeeInt_NewInt128(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_set_time_t(DeeTimeObject *self,
                DeeObject *value) {
	Dee_int128_t newval;
	uint32_t extra_nano;
	if (DeeObject_AsInt128(value, &newval))
		goto err;
	/* NOTE: Must keep nano-seconds */
	__hybrid_int128_add64(newval, SECONDS_01_01_1970);
	__hybrid_int128_mul32(newval, NANOSECONDS_PER_SECOND);
	__hybrid_int128_floormod32_r(self->t_nanos, NANOSECONDS_PER_SECOND, extra_nano);
	__hybrid_int128_add32(newval, extra_nano);
	self->t_nanos    = newval;
	self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
time_del_time_t(DeeTimeObject *__restrict self) {
	return time_set_time_t(self, Dee_None);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_get_istimestamp(DeeTimeObject *__restrict self) {
	return_bool(DeeTime_IsTimestamp(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_get_isdelta(DeeTimeObject *__restrict self) {
	return_bool(DeeTime_IsDelta(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_get_astimestamp(DeeTimeObject *__restrict self) {
	if (self->t_kind != TIME_KIND_TIMESTAMP) {
		if (DeeObject_IsShared(self)) {
			DREF DeeTimeObject *result;
			result = DeeObject_MALLOC(DeeTimeObject);
			if likely(result) {
				DeeObject_Init(result, &DeeTime_Type);
				result->t_nanos = self->t_nanos;
				result->t_type  = self->t_type;
				result->t_kind  = TIME_KIND_TIMESTAMP;
			}
			return result;
		}
		self->t_kind = TIME_KIND_TIMESTAMP;
	}
	return_reference_(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_get_asdelta(DeeTimeObject *__restrict self) {
	if (self->t_kind != TIME_KIND_DELTA) {
		if (DeeObject_IsShared(self)) {
			DREF DeeTimeObject *result;
			result = DeeObject_MALLOC(DeeTimeObject);
			if likely(result) {
				DeeObject_Init(result, &DeeTime_Type);
				result->t_nanos = self->t_nanos;
				result->t_type  = self->t_type;
				result->t_kind  = TIME_KIND_DELTA;
			}
			return result;
		}
		self->t_kind = TIME_KIND_DELTA;
	}
	return_reference_(self);
}

PRIVATE struct type_getset tpconst time_getsets[] = {
	/* Access to individual time parts by name */
#define DEFINE_LIBTIME_AS_FIELD(name, doc) \
	TYPE_GETSET_AB_F(#name, &time_getval_##name, &time_delval_##name, &time_setval_##name, METHOD_FNOREFESCAPE, DOC("->?Dint\n" doc))
	DEFINE_LIBTIME_AS_FIELD(nanosecond, "Nanosecond of second"),
	DEFINE_LIBTIME_AS_FIELD(microsecond, "Microsecond of second"),
	DEFINE_LIBTIME_AS_FIELD(millisecond, "Millisecond of second"),
	DEFINE_LIBTIME_AS_FIELD(second, "Second of minute"),
	DEFINE_LIBTIME_AS_FIELD(minute, "Minute of Hour"),
	DEFINE_LIBTIME_AS_FIELD(hour, "Hour of day"),
	DEFINE_LIBTIME_AS_FIELD(wday, "Day of week (0-based; 0 is Sunday)"),
	DEFINE_LIBTIME_AS_FIELD(mweek, "Week of month (week 1 starts on the first sunday of the month; if the month doesn't start on a sunday, week 0 exists)"),
	DEFINE_LIBTIME_AS_FIELD(month, "Month of year (1-based)"),
	DEFINE_LIBTIME_AS_FIELD(year, "Year"),
	DEFINE_LIBTIME_AS_FIELD(decade, "Decade"),
	DEFINE_LIBTIME_AS_FIELD(century, "Century"),
	DEFINE_LIBTIME_AS_FIELD(millennium, "Millennium"),
	DEFINE_LIBTIME_AS_FIELD(mday, "Day of month (1-based)"),
	DEFINE_LIBTIME_AS_FIELD(yday, "Day of year (1-based)"),
	DEFINE_LIBTIME_AS_FIELD(yweek, "Week of year (week 1 starts on the first sunday of the year; if the year doesn't start on a sunday, week 0 exists)"),
	DEFINE_LIBTIME_AS_FIELD(nanoseconds, "Total nanoseconds (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(microseconds, "Total microseconds (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(milliseconds, "Total milliseconds (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(seconds, "Total seconds (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(minutes, "Total minutes (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(hours, "Total hours (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(days, "Total days (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(weeks, "Total weeks (since #C{01-01-0000})"),
	DEFINE_LIBTIME_AS_FIELD(months, "Total months (since #C{01-01-0000}, using the average of $2_629_746 seconds per month)"),
	DEFINE_LIBTIME_AS_FIELD(years, "Total years (since #C{01-01-0000}, using the average of $31_556_952 seconds per year)"),
	DEFINE_LIBTIME_AS_FIELD(decades, "Total decades (since #C{01-01-0000}, using the average of $315_569_520 seconds per decade)"),
	DEFINE_LIBTIME_AS_FIELD(centuries, "Total centuries (since #C{01-01-0000}, using the average of $3_155_695_200 seconds per century)"),
	DEFINE_LIBTIME_AS_FIELD(millennia, "Total millennia (since #C{01-01-0000}, using the average of $31_556_952_000 seconds per millennium)"),
#undef DEFINE_LIBTIME_AS_FIELD

	TYPE_GETSET_AB_F("time_t", &time_get_time_t, &time_del_time_t, &time_set_time_t, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Get/set the time as the number of seconds since #C{01-01-1970}"),

	TYPE_GETTER_AB_F("istimestamp", &time_get_istimestamp, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Check if @this ?. object represents a timestamp (as opposed to ?#isdelta)"),
	TYPE_GETTER_AB_F("isdelta", &time_get_isdelta, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Check if @this ?. object represents a delta (as opposed to ?#istimestamp)"),
	TYPE_GETTER_AB("astimestamp", &time_get_astimestamp,
	               "->?.\n"
	               "Re-return @this ?. object as a timestamp object. If @this is already a timestamp "
	               /**/ "(as per ?#istimestamp), simply re-return @this (no copy is created). "
	               /**/ "When @this is a delta-time object (as per ?#isdelta), return a timestamp "
	               /**/ "that is equal to ${Time(year: 0, month: 1, day: 1) + this}"),
	TYPE_GETTER_AB("asdelta", &time_get_asdelta,
	               "->?.\n"
	               "Re-return @this ?. object as a delta-time object. If @this is already a delta-time "
	               /**/ "object (as per ?#isdelta), simply re-return @this (no copy is created). "
	               /**/ "When @this is a timestamp (as per ?#istimestamp), return a delta-time "
	               /**/ "object that is equal to ${this - Time(year: 0, month: 1, day: 1)}"),

	/* Deprecated aliases */
	TYPE_GETSET_AB_F("time", &time_timepart_get, &time_timepart_del, &time_timepart_set, METHOD_FNOREFESCAPE,
	                 "->?GTime\nDeprecated alias for ?#timepart"),
	TYPE_GETSET_AB_F("date", &time_datepart_get, &time_datepart_del, &time_datepart_set, METHOD_FNOREFESCAPE,
	                 "->?GTime\nDeprecated alias for ?#datepart"),
#define DEFINE_DEPRECATED_TIME_AS_FIELD(name, alias_for)                                                                      \
	TYPE_GETSET_AB_F(name, &time_getval_##alias_for, &time_delval_##alias_for, &time_setval_##alias_for, METHOD_FNOREFESCAPE, \
	                 "->?Dint\nDeprecated alias for ?#" #alias_for)
	DEFINE_DEPRECATED_TIME_AS_FIELD("mic", microsecond),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mil", millisecond),
	DEFINE_DEPRECATED_TIME_AS_FIELD("sec", second),
	DEFINE_DEPRECATED_TIME_AS_FIELD("min", minute),
	DEFINE_DEPRECATED_TIME_AS_FIELD("hor", hour),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mwek", mweek),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mon", month),
	DEFINE_DEPRECATED_TIME_AS_FIELD("yer", year),
	DEFINE_DEPRECATED_TIME_AS_FIELD("dec", decade),
	DEFINE_DEPRECATED_TIME_AS_FIELD("cen", century),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mll", millennium),
	DEFINE_DEPRECATED_TIME_AS_FIELD("ywek", yweek),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mics", microseconds),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mils", milliseconds),
	DEFINE_DEPRECATED_TIME_AS_FIELD("secs", seconds),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mins", minutes),
	DEFINE_DEPRECATED_TIME_AS_FIELD("hors", hours),
	DEFINE_DEPRECATED_TIME_AS_FIELD("weks", weeks),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mons", months),
	DEFINE_DEPRECATED_TIME_AS_FIELD("yers", years),
	DEFINE_DEPRECATED_TIME_AS_FIELD("decs", decades),
	DEFINE_DEPRECATED_TIME_AS_FIELD("cens", centuries),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mlls", millennia),
	DEFINE_DEPRECATED_TIME_AS_FIELD("weekday", wday),
	DEFINE_DEPRECATED_TIME_AS_FIELD("monthweek", mweek),
	DEFINE_DEPRECATED_TIME_AS_FIELD("monthday", mday),
	DEFINE_DEPRECATED_TIME_AS_FIELD("yearday", yday),
	DEFINE_DEPRECATED_TIME_AS_FIELD("yearweek", yweek),
	DEFINE_DEPRECATED_TIME_AS_FIELD("msecond", millisecond),
	DEFINE_DEPRECATED_TIME_AS_FIELD("mseconds", milliseconds),
	DEFINE_DEPRECATED_TIME_AS_FIELD("millenia", millennia),
#undef DEFINE_DEPRECATED_TIME_AS_FIELD

	TYPE_GETSET_AB_F("timepart", &time_timepart_get, &time_timepart_del, &time_timepart_set, METHOD_FNOREFESCAPE,
	                 "->?GTime\n"
	                 "#tValueError{(get-only) @this ?. object isn't a timestamp (s.a. ?#istimestamp)}"
	                 "Read/write the time portion of @this time object, that is everything below the "
	                 /**/ "day-threshold, including ?#hour, ?#minute, ?#second and ?#nanosecond\n"
	                 "When setting, the passed objected is interpreted as an integer describing the "
	                 /**/ "number of nanoseconds since the day began"),
	TYPE_GETSET_AB_F("datepart", &time_datepart_get, &time_datepart_del, &time_datepart_set, METHOD_FNOREFESCAPE,
	                 "->?GTime\n"
	                 "#tValueError{(get-only) @this ?. object isn't a timestamp (s.a. ?#istimestamp)}"
	                 "#tValueError{Attempted to assign a time value with a non-zero ?#timepart}"
	                 "Read/write the date portion of @this time object, that is everything "
	                 /**/ "above the day-threshold, including ?#mday, ?#month and ?#year\n"
	                 "When setting, the passed objected is interpreted as an integer "
	                 /**/ "describing the number of nanoseconds since #C{01-01-0000}"),

	TYPE_GETTER_AB_F("isdst", &time_isdst, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if DaylightSavingsTime is in active at @this time\n"
	                 "Note that this implementation does not perform any special "
	                 /**/ "handling no matter if daylight savings is active or not.\n"
	                 "Also note that this implementation does not account for different "
	                 /**/ "algorithms that may be used around the world to determine isdst "
	                 /**/ "being enabled, or certain countries/timezones that may not even "
	                 /**/ "have daylight savings time."),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_neg(DeeTimeObject *__restrict self) {
	DREF DeeTimeObject *result;
	if (!DeeTime_IsDelta(self))
		goto err_cannot_negate;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_nanos    = self->t_nanos;
	result->t_typekind = self->t_typekind;
	__hybrid_int128_neg(result->t_nanos);
	return result;
err_cannot_negate:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot negate timestamp %r",
	                self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_add(DeeTimeObject *self, DeeTimeObject *other) {
	DREF DeeTimeObject *result;
	if (DeeObject_AssertType(other, &DeeTime_Type))
		goto err;
	if (DeeTime_IsDelta(self)) {
		if (!DeeTime_IsDelta(other)) {
			/* Keep the delta on the right side */
			DeeTimeObject *temp;
			temp  = self;
			self  = other;
			other = temp;
		}
	} else {
		if (!DeeTime_IsDelta(other))
			goto err_cannot_add_timestamp_to_timestamp;
	}
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	if (self->t_type == other->t_type) {
		result->t_type  = self->t_type;
		result->t_nanos = self->t_nanos;
		__hybrid_int128_add128(result->t_nanos, other->t_nanos);
	} else if (self->t_type == TIME_TYPE_NANOSECONDS) {
		/* Add months to `result' */
		Dee_int128_t month;
		ASSERT(other->t_type == TIME_TYPE_MONTHS);
		result->t_nanos = self->t_nanos;
		result->t_type  = TIME_TYPE_NANOSECONDS;
		_DeeTime_GetRepr(&month, result, TIME_REPR_MONTH);
		__hybrid_int128_add128(month, other->t_months);
		DeeTime_SetRepr(result, &month, TIME_REPR_MONTH);
	} else {
		/* Add nano-seconds `result' */
		ASSERT(self->t_type == TIME_TYPE_MONTHS);
		ASSERT(other->t_type == TIME_TYPE_NANOSECONDS);
		result->t_type  = TIME_TYPE_NANOSECONDS;
		result->t_nanos = self->t_months;
		time_inplace_months2nanoseconds(&result->t_nanos);
		__hybrid_int128_add128(result->t_nanos, other->t_nanos);
	}

	/* Use the representation of the non-delta operand
	 * (thus turning the resulting into a timestamp) */
	result->t_kind = self->t_kind;
	if (DeeTime_IsDelta(self))
		result->t_kind = other->t_kind;
	DeeObject_Init(result, &DeeTime_Type);
	return result;
err_cannot_add_timestamp_to_timestamp:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot add timestamps %r and %r",
	                self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_sub(DeeTimeObject *self, DeeTimeObject *other) {
	DREF DeeTimeObject *result;
	if (DeeObject_AssertType(other, &DeeTime_Type))
		goto err;
	if (DeeTime_IsDelta(self) && !DeeTime_IsDelta(other))
		goto err_cannot_subtract_timestamp_from_delta;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	if (self->t_type == other->t_type) {
		result->t_type  = self->t_type;
		result->t_nanos = self->t_nanos;
		__hybrid_int128_sub128(result->t_nanos, other->t_nanos);
	} else if (self->t_type == TIME_TYPE_NANOSECONDS) {
		/* Add months to `result' */
		Dee_int128_t month;
		ASSERT(other->t_type == TIME_TYPE_MONTHS);
		result->t_nanos = self->t_nanos;
		result->t_type  = TIME_TYPE_NANOSECONDS;
		_DeeTime_GetRepr(&month, result, TIME_REPR_MONTH);
		__hybrid_int128_sub128(month, other->t_months);
		DeeTime_SetRepr(result, &month, TIME_REPR_MONTH);
	} else {
		/* Add nano-seconds `result' */
		ASSERT(self->t_type == TIME_TYPE_MONTHS);
		ASSERT(other->t_type == TIME_TYPE_NANOSECONDS);
		result->t_type  = TIME_TYPE_NANOSECONDS;
		result->t_nanos = self->t_months;
		time_inplace_months2nanoseconds(&result->t_nanos);
		__hybrid_int128_sub128(result->t_nanos, other->t_nanos);
	}

	result->t_kind = self->t_kind;
	if (!DeeTime_IsDelta(self) && !DeeTime_IsDelta(other))
		result->t_kind = TIME_KIND_DELTA; /* Delta between 2 timestamp */
	DeeObject_Init(result, &DeeTime_Type);
	return result;
err_cannot_subtract_timestamp_from_delta:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot subtract timestamps %r from delta %r",
	                self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_mul(DeeTimeObject *self, DeeObject *other) {
	DREF DeeTimeObject *result;
	Dee_int128_t multiplier;
	if (DeeObject_AsInt128(other, &multiplier))
		goto err;
	if (!DeeTime_IsDelta(self))
		goto err_cannot_multiply_timestamp;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_typekind = self->t_typekind;
	result->t_nanos    = self->t_nanos;
	__hybrid_int128_mul128(result->t_nanos, multiplier);
	DeeObject_Init(result, &DeeTime_Type);
	return result;
err_cannot_multiply_timestamp:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot multiply timestamp %r by %" PRFd128,
	                self, multiplier);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
time_div(DeeTimeObject *self, DeeObject *other) {
	DREF DeeTimeObject *result;
	Dee_int128_t rhs, res;
	if unlikely(!DeeTime_IsDelta(self))
		goto err_cannot_divide_timestamp;
	if (DeeTime_Check(other)) {
		DeeTime_AsNano((DeeTimeObject *)other, &rhs);
		if unlikely(__hybrid_int128_iszero(rhs))
			goto err_divzero;
		DeeTime_AsNano(self, &res);
		__hybrid_int128_floordiv128(res, rhs);
		return DeeInt_NewInt128(res);
	}

	if (DeeObject_AsInt128(other, &rhs))
		goto err;
	if unlikely(__hybrid_int128_iszero(rhs))
		goto err_divzero;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_type  = self->t_type;
	result->t_kind  = TIME_KIND_DELTA;
	result->t_nanos = self->t_nanos;
	__hybrid_int128_floordiv128(result->t_nanos, rhs);
	DeeObject_Init(result, &DeeTime_Type);
done:
	return Dee_AsObject(result);
err_cannot_divide_timestamp:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot divide timestamp %r by %r",
	                self, other);
	goto err;
err_divzero:
	DeeRT_ErrDivideByZero(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
time_mod(DeeTimeObject *self, DeeObject *other) {
	DREF DeeTimeObject *result;
	Dee_int128_t rhs;
	if unlikely(!DeeTime_IsDelta(self))
		goto err_cannot_divide_timestamp;
	if (DeeObject_AsInt128(other, &rhs))
		goto err;
	if unlikely(__hybrid_int128_iszero(rhs))
		goto err_divzero;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_type  = self->t_type;
	result->t_kind  = TIME_KIND_DELTA;
	result->t_nanos = self->t_nanos;
	__hybrid_int128_floormod128(result->t_nanos, rhs);
	DeeObject_Init(result, &DeeTime_Type);
done:
	return Dee_AsObject(result);
err_cannot_divide_timestamp:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot mod timestamp %r by %r",
	                self, other);
	goto err;
err_divzero:
	DeeRT_ErrDivideByZero(self, other);
err:
	return NULL;
}


PRIVATE struct type_math time_math = {
	/* .tp_int32  = */ (int(DCALL *)(DeeObject *__restrict, int32_t *__restrict))&time_int32,
	/* .tp_int64  = */ (int(DCALL *)(DeeObject *__restrict, int64_t *__restrict))&time_int64,
	/* .tp_double = */ NULL,
	/* .tp_int    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_int,
	/* .tp_inv    = */ NULL,
	/* .tp_pos    = */ &DeeObject_NewRef,
	/* .tp_neg    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_neg,
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_add,
	/* .tp_sub    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_sub,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_mul,
	/* .tp_div    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_div,
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_mod,
	/* .tp_shl    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_shr    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_and    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_or     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_xor    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_pow    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL
};


/*[[[deemon (print_CMethod from rt.gen.unpack)("gmtime", "");]]]*/
#define libtime_gmtime_params ""
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_gmtime_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libtime_gmtime, &libtime_gmtime_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_gmtime_f_impl(void)
/*[[[end]]]*/
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	time_now_utc(&result->t_nanos);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("localtime", "");]]]*/
#define libtime_localtime_params ""
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_localtime_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libtime_localtime, &libtime_localtime_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_localtime_f_impl(void)
/*[[[end]]]*/
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	time_now_local(&result->t_nanos);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("tick", "");]]]*/
#define libtime_tick_params ""
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_tick_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libtime_tick, &libtime_tick_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_tick_f_impl(void)
/*[[[end]]]*/
{
	uint64_t tick;
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;

	/* Load the current tick. */
	tick = DeeThread_GetTimeMicroSeconds(); /* TODO: Use a higher resolution source (can go up to nanoseconds) */
	__hybrid_int128_set64(result->t_nanos, tick);
	__hybrid_int128_mul16(result->t_nanos, NANOSECONDS_PER_MICROSECOND);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}


/* NOTE: All of this stuff can't be CONSTEXPR because `Time' objects are mutable :(
 * XXX: Maybe reconsider if Time objects really need to be mutable... The only reason
 *      right now is due to setters like `Time.second = 42'. I feel like it would be
 *      nicer if these weren't writable, but instead there was `Time.with(second: 42)'
 *      in order to construct a new time object with certain fields changed to other
 *      values. */

/*[[[deemon
import * from deemon;
import print_CMethod from rt.gen.unpack;
function def(what: string, where: string, mul: string) {
	print_CMethod(what, "Dee_int128_t value");
	print("{");
	print("	DREF DeeTimeObject *result;");
	print("	result = DeeObject_MALLOC(DeeTimeObject);");
	print("	if unlikely(!result)");
	print("		goto err;");
	print("	result->", where, " = value;");
	if (mul)
		print("	", mul, ";");
	print("	result->t_typekind = TIME_TYPEKIND(", {
		"t_nanos": "TIME_TYPE_NANOSECONDS",
		"t_months": "TIME_TYPE_MONTHS",
	}[where], ", TIME_KIND_DELTA);");
	print("	DeeObject_Init(result, &DeeTime_Type);");
	print("	return Dee_AsObject(result);");
	print("err:");
	print("	return NULL;");
	print("}");
	print;
}

def("nanoseconds", "t_nanos", "");
def("microseconds", "t_nanos", "__hybrid_int128_mul16(result->t_nanos, NANOSECONDS_PER_MICROSECOND)");
def("milliseconds", "t_nanos", "__hybrid_int128_mul32(result->t_nanos, NANOSECONDS_PER_MILLISECOND)");
def("seconds", "t_nanos", "__hybrid_int128_mul32(result->t_nanos, NANOSECONDS_PER_SECOND)");
def("minutes", "t_nanos", "__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_MINUTE)");
def("hours", "t_nanos", "__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_HOUR)");
def("days", "t_nanos", "__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_DAY)");
def("weeks", "t_nanos", "__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_WEEK)");
def("months", "t_months", "");
def("years", "t_months", "__hybrid_int128_mul8(result->t_months, MONTHS_PER_YEAR)");
def("decades", "t_months", "__hybrid_int128_mul8(result->t_months, MONTHS_PER_DECADE)");
def("centuries", "t_months", "__hybrid_int128_mul16(result->t_months, MONTHS_PER_CENTURY)");
def("millennia", "t_months", "__hybrid_int128_mul16(result->t_months, MONTHS_PER_MILLENNIUM)");
]]]*/
#define libtime_nanoseconds_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_nanoseconds_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_nanoseconds_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_nanoseconds_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_nanoseconds, &libtime_nanoseconds_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_nanoseconds_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_microseconds_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_microseconds_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_microseconds_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_microseconds_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_microseconds, &libtime_microseconds_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_microseconds_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	__hybrid_int128_mul16(result->t_nanos, NANOSECONDS_PER_MICROSECOND);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_milliseconds_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_milliseconds_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_milliseconds_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_milliseconds_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_milliseconds, &libtime_milliseconds_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_milliseconds_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	__hybrid_int128_mul32(result->t_nanos, NANOSECONDS_PER_MILLISECOND);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_seconds_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_seconds_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_seconds_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_seconds_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_seconds, &libtime_seconds_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_seconds_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	__hybrid_int128_mul32(result->t_nanos, NANOSECONDS_PER_SECOND);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_minutes_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_minutes_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_minutes_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_minutes_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_minutes, &libtime_minutes_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_minutes_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_MINUTE);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_hours_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_hours_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_hours_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_hours_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_hours, &libtime_hours_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_hours_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_HOUR);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_days_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_days_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_days_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_days_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_days, &libtime_days_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_days_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_DAY);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_weeks_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_weeks_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_weeks_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_weeks_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_weeks, &libtime_weeks_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_weeks_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_nanos = value;
	__hybrid_int128_mul64(result->t_nanos, NANOSECONDS_PER_WEEK);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_months_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_months_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_months_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_months_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_months, &libtime_months_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_months_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_months = value;
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_years_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_years_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_years_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_years_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_years, &libtime_years_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_years_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_months = value;
	__hybrid_int128_mul8(result->t_months, MONTHS_PER_YEAR);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_decades_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_decades_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_decades_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_decades_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_decades, &libtime_decades_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_decades_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_months = value;
	__hybrid_int128_mul8(result->t_months, MONTHS_PER_DECADE);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_centuries_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_centuries_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_centuries_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_centuries_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_centuries, &libtime_centuries_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_centuries_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_months = value;
	__hybrid_int128_mul16(result->t_months, MONTHS_PER_CENTURY);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#define libtime_millennia_params "value:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_millennia_f_impl(Dee_int128_t value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime_millennia_f(DeeObject *__restrict arg0) {
	Dee_int128_t value;
	if (DeeObject_AsInt128(arg0, &value))
		goto err;
	return libtime_millennia_f_impl(value);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime_millennia, &libtime_millennia_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_millennia_f_impl(Dee_int128_t value)
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_months = value;
	__hybrid_int128_mul16(result->t_months, MONTHS_PER_MILLENNIUM);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_MONTHS, TIME_KIND_DELTA);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}
/*[[[end]]]*/

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("maketime", """
	Dee_int128_t hour = 0,
	Dee_int128_t minute = 0,
	Dee_int128_t second = 0,
	Dee_int128_t nanosecond = 0,
""");]]]*/
#define libtime_maketime_params "hour=!0,minute=!0,second=!0,nanosecond=!0"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_maketime_f_impl(Dee_int128_t hour, Dee_int128_t minute, Dee_int128_t second, Dee_int128_t nanosecond);
#ifndef DEFINED_kwlist__hour_minute_second_nanosecond
#define DEFINED_kwlist__hour_minute_second_nanosecond
PRIVATE DEFINE_KWLIST(kwlist__hour_minute_second_nanosecond, { KEX("hour", 0x1a9ed795, 0x10f3e1a52760b6c5), KEX("minute", 0x951f07e5, 0x841bf13791a969b), KEX("second", 0x1a084e7b, 0x3058181b9a44b68c), KEX("nanosecond", 0xe8e3c3b6, 0x16f16d5d9832d2aa), KEND });
#endif /* !DEFINED_kwlist__hour_minute_second_nanosecond */
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_maketime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		Dee_int128_t hour;
		Dee_int128_t minute;
		Dee_int128_t second;
		Dee_int128_t nanosecond;
	} args;
	__hybrid_int128_setzero(args.hour);
	__hybrid_int128_setzero(args.minute);
	__hybrid_int128_setzero(args.second);
	__hybrid_int128_setzero(args.nanosecond);
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hour_minute_second_nanosecond, "|" UNPd128 UNPd128 UNPd128 UNPd128 ":maketime", &args))
		goto err;
	return libtime_maketime_f_impl(args.hour, args.minute, args.second, args.nanosecond);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libtime_maketime, &libtime_maketime_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_maketime_f_impl(Dee_int128_t hour, Dee_int128_t minute, Dee_int128_t second, Dee_int128_t nanosecond)
/*[[[end]]]*/
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	__hybrid_int128_setzero(result->t_nanos);
	DeeTime_SetRepr(result, &hour, TIME_REPR_HOUR);
	DeeTime_SetRepr(result, &minute, TIME_REPR_MINUTE);
	DeeTime_SetRepr(result, &second, TIME_REPR_SECOND);
	DeeTime_SetRepr(result, &nanosecond, TIME_REPR_NANOSECOND);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("makedate", """
	Dee_int128_t year = 0,
	Dee_int128_t month = 1,
	Dee_int128_t day = 1,
""");]]]*/
#define libtime_makedate_params "year=!0,month=!1,day=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_makedate_f_impl(Dee_int128_t year, Dee_int128_t month, Dee_int128_t day);
#ifndef DEFINED_kwlist__year_month_day
#define DEFINED_kwlist__year_month_day
PRIVATE DEFINE_KWLIST(kwlist__year_month_day, { KEX("year", 0x310a1818, 0xa8d044c5ea490cb6), KEX("month", 0x51ca185c, 0xbe9d67504ee78acc), KEX("day", 0xe0fe498d, 0x8e12be46fd64d268), KEND });
#endif /* !DEFINED_kwlist__year_month_day */
PRIVATE WUNUSED DREF DeeObject *DCALL libtime_makedate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		Dee_int128_t year;
		Dee_int128_t month;
		Dee_int128_t day;
	} args;
	__hybrid_int128_setzero(args.year);
	__hybrid_int128_setone(args.month);
	__hybrid_int128_setone(args.day);
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__year_month_day, "|" UNPd128 UNPd128 UNPd128 ":makedate", &args))
		goto err;
	return libtime_makedate_f_impl(args.year, args.month, args.day);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libtime_makedate, &libtime_makedate_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime_makedate_f_impl(Dee_int128_t year, Dee_int128_t month, Dee_int128_t day)
/*[[[end]]]*/
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	__hybrid_int128_setzero(result->t_nanos);
	DeeTime_SetRepr(result, &year, TIME_REPR_YEAR);
	DeeTime_SetRepr(result, &month, TIME_REPR_MONTH);
	DeeTime_SetRepr(result, &day, TIME_REPR_MDAY);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}


/*[[[deemon
import * from time;
print("#define UNIX_TIME_T_BASE_SECONDS INT64_C(", Time(year: 1970, month: 1, day: 1).seconds.hex(), ")");
]]]*/
#define UNIX_TIME_T_BASE_SECONDS INT64_C(0xe79747c00)
/*[[[end]]]*/

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTime_NewUnix(int64_t seconds_since_01_01_1970,
                uint32_t extra_nanoseconds) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	__hybrid_int128_set64(result->t_nanos, seconds_since_01_01_1970);
	__hybrid_int128_add64(result->t_nanos, UNIX_TIME_T_BASE_SECONDS);
	__hybrid_int128_mul32(result->t_nanos, NANOSECONDS_PER_SECOND);
	__hybrid_int128_add32(result->t_nanos, extra_nanoseconds);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	DeeObject_Init(result, &DeeTime_Type);
done:
	return Dee_AsObject(result);
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("_mkunix", """
	int64_t time_t;
	uint32_t nanosecond = 0;
""");]]]*/
#define libtime__mkunix_params "time_t:?Dint,nanosecond=!0"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime__mkunix_f_impl(int64_t time_t_, uint32_t nanosecond);
PRIVATE WUNUSED DREF DeeObject *DCALL libtime__mkunix_f(size_t argc, DeeObject *const *argv) {
	struct {
		int64_t time_t_;
		uint32_t nanosecond;
	} args;
	args.nanosecond = 0;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "_mkunix", &args, &args.time_t_, UNPd64, DeeObject_AsInt64, &args.nanosecond, UNPu32, DeeObject_AsUInt32);
	return libtime__mkunix_f_impl(args.time_t_, args.nanosecond);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(libtime__mkunix, &libtime__mkunix_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime__mkunix_f_impl(int64_t time_t_, uint32_t nanosecond)
/*[[[end]]]*/
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	__hybrid_int128_set64(result->t_nanos, time_t_);
	__hybrid_int128_add64(result->t_nanos, UNIX_TIME_T_BASE_SECONDS);
	__hybrid_int128_mul32(result->t_nanos, NANOSECONDS_PER_SECOND);
	__hybrid_int128_add32(result->t_nanos, nanosecond);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

#if defined(CONFIG_HOST_WINDOWS) || defined(__DEEMON__)
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeTime_NewFILETIME(void const *filetime) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	__hybrid_uint128_set64(*(Dee_uint128_t *)&result->t_nanos, UNALIGNED_GET64(filetime));
	__hybrid_uint128_mul8(*(Dee_uint128_t *)&result->t_nanos, 100);
	__hybrid_uint128_add128(*(Dee_uint128_t *)&result->t_nanos, NANOSECONDS_01_01_1601);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	DeeObject_Init(result, &DeeTime_Type);
done:
	return Dee_AsObject(result);
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("_mkFILETIME", "uint64_t FILETIME");]]]*/
#define libtime__mkFILETIME_params "FILETIME:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime__mkFILETIME_f_impl(uint64_t FILETIME_);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libtime__mkFILETIME_f(DeeObject *__restrict arg0) {
	uint64_t FILETIME_;
	if (DeeObject_AsUInt64(arg0, &FILETIME_))
		goto err;
	return libtime__mkFILETIME_f_impl(FILETIME_);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libtime__mkFILETIME, &libtime__mkFILETIME_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libtime__mkFILETIME_f_impl(uint64_t FILETIME_)
/*[[[end]]]*/
{
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	__hybrid_uint128_set64(result->t_unanos, FILETIME_);
	__hybrid_uint128_mul8(result->t_unanos, 100);
	__hybrid_uint128_add128(result->t_unanos, NANOSECONDS_01_01_1601);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	DeeObject_Init(result, &DeeTime_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}
#endif /* CONFIG_HOST_WINDOWS */


PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_class_from_time_t(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	if (DeeArg_UnpackStruct(argc, argv, UNPd128 ":from_time_t", &result->t_nanos))
		goto err_r;
	__hybrid_int128_add64(result->t_nanos, SECONDS_01_01_1970);
	__hybrid_int128_mul32(result->t_nanos, NANOSECONDS_PER_SECOND);
	result->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	DeeObject_Init(result, &DeeTime_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_freq(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "freq");
	return DeeInt_NewInt(NANOSECONDS_PER_SECOND);
err:
	return NULL;
}
PRIVATE struct type_method tpconst time_class_methods[] = {
	TYPE_METHOD("freq", &time_class_freq,
	            "->?Dint\n"
	            "Deprecated. Always returns $1000000"),
	TYPE_METHOD("from_time_t", &time_class_from_time_t,
	            "(time_t_value:?Dint)->?.\n"
	            "Deprecated (use ${Time(time_t: time_t_value)} instead)"),
	TYPE_METHOD_END,
};

PRIVATE struct type_member tpconst time_class_members[] = {
	/* For backwards compatibility with the old deemon (which
	 * did everything as part of the `time' builtin type) */
	TYPE_MEMBER_CONST_DOC("now", &libtime_localtime,
	                      "->?.\n"
	                      "Deprecated. Use ?Glocaltime instead"),
	TYPE_MEMBER_CONST_DOC("tick", &libtime_tick,
	                      "->?.\n"
	                      "Deprecated. Use ?Gtick instead"),
	TYPE_MEMBER_CONST_DOC("time", &libtime_maketime,
	                      "(" libtime_maketime_params ")->?.\n"
	                      "Deprecated. Use ?Gmaketime or ?GTime instead"),
	TYPE_MEMBER_CONST_DOC("date", &libtime_makedate,
	                      "(" libtime_makedate_params ")->?.\n"
	                      "Deprecated. Use ?Gmakedate or ?GTime instead"),
#define DEFINE_DELTA_CALLBACK(name, func) \
	TYPE_MEMBER_CONST_DOC(name, &libtime_##func, \
	                      "(value:?Dint)->?.\n"  \
	                      "Deprecated. Use ?G" #func " instead")
	DEFINE_DELTA_CALLBACK("mseconds", milliseconds),
	DEFINE_DELTA_CALLBACK("seconds", seconds),
	DEFINE_DELTA_CALLBACK("minutes", minutes),
	DEFINE_DELTA_CALLBACK("hours", hours),
	DEFINE_DELTA_CALLBACK("days", days),
	DEFINE_DELTA_CALLBACK("weeks", weeks),
	DEFINE_DELTA_CALLBACK("months", months),
	DEFINE_DELTA_CALLBACK("years", years),
#undef DEFINE_DELTA_CALLBACK
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
time_bool(DeeTimeObject *__restrict self) {
	return !__hybrid_int128_iszero(self->t_nanos);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_copy(DeeTimeObject *__restrict self,
          DeeTimeObject *__restrict other) {
	self->t_typekind = other->t_typekind;
	self->t_nanos = other->t_nanos;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_serialize(DeeTimeObject *__restrict self,
               DeeSerial *__restrict writer,
               Dee_seraddr_t addr) {
	DeeTimeObject *out = DeeSerial_Addr2Mem(writer, addr, DeeTimeObject);
	out->t_typekind = self->t_typekind;
	out->t_nanos    = self->t_nanos;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_assign(DeeTimeObject *self, DeeTimeObject *other) {
	if (DeeTime_Check(other)) {
		self->t_typekind = other->t_typekind;
		self->t_nanos    = other->t_nanos;
	} else {
		if (DeeObject_AsInt128((DeeObject *)other, &self->t_nanos))
			goto err;
		self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_TIMESTAMP);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
time_ctor(DeeTimeObject *__restrict self) {
	/* Produce a zero-delta */
	self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
	__hybrid_int128_setzero(self->t_nanos);
	return 0;
}

/* @return:  0: Success (Argument is present)
 * @return:  1: Argument not present
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 5, 7)) int DFCALL
time_init_getarg(DeeKwArgs *__restrict kwds,
                 size_t argc, DeeObject *const *argv,
                 size_t default_argi,
                 char const *__restrict argname, Dee_hash_t hash,
                 Dee_int128_t *__restrict p_result) {
	DeeObject *arg;
	if (default_argi < argc)
		return DeeObject_AsInt128(argv[default_argi], p_result);
	arg = DeeKwArgs_TryGetItemNRStringHash(kwds, argname, hash);
	if (arg == ITER_DONE)
		return 1;
	if (arg == NULL)
		return -1;
	return DeeObject_AsInt128(arg, p_result);
}


PRIVATE NONNULL((1, 2)) void DFCALL
time_init_addrepr(DeeTimeObject *__restrict self,
                  Dee_int128_t const *__restrict p_value,
                  uint8_t repr) {
	if (self->t_kind == TIME_KIND_INVALID) {
		DeeTime_SetRepr(self, p_value, repr);
	} else {
		Dee_int128_t oldval;
		oldval = self->t_nanos;
		DeeTime_SetRepr(self, p_value, repr);
		__hybrid_int128_add128(self->t_nanos, oldval);
	}
}

PRIVATE NONNULL((1, 2)) void DFCALL
time_init_addrepr_months(DeeTimeObject *__restrict self,
                         Dee_int128_t const *__restrict p_value,
                         uint8_t repr) {
	if (self->t_kind == TIME_KIND_INVALID) {
		DeeTime_SetRepr(self, p_value, repr);
	} else if (self->t_type == TIME_TYPE_MONTHS) {
		Dee_int128_t oldval;
		oldval = self->t_months;
		DeeTime_SetRepr(self, p_value, repr);
		__hybrid_int128_add128(self->t_months, oldval);
	} else {
		Dee_int128_t oldval;
		oldval = self->t_nanos;
		ASSERT(self->t_type == TIME_TYPE_NANOSECONDS);
		DeeTime_SetRepr(self, p_value, repr);
		ASSERT(self->t_type == TIME_TYPE_MONTHS);
		time_inplace_month2nanosecond(&self->t_months);
		self->t_type = TIME_TYPE_NANOSECONDS;
		__hybrid_int128_add128(self->t_nanos, oldval);
	}
}

PRIVATE NONNULL((1, 2)) void DFCALL
time_init_setrepr(DeeTimeObject *__restrict self,
                  Dee_int128_t const *__restrict p_value,
                  uint8_t repr) {
	self->t_kind = TIME_KIND_TIMESTAMP;
	DeeTime_SetRepr(self, p_value, repr);
}

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("nanoseconds");
print define_Dee_HashStr("microseconds");
print define_Dee_HashStr("milliseconds");
print define_Dee_HashStr("seconds");
print define_Dee_HashStr("minutes");
print define_Dee_HashStr("hours");
print define_Dee_HashStr("days");
print define_Dee_HashStr("weeks");
print define_Dee_HashStr("months");
print define_Dee_HashStr("years");
print define_Dee_HashStr("decades");
print define_Dee_HashStr("centuries");
print define_Dee_HashStr("millennia");
print define_Dee_HashStr("year");
print define_Dee_HashStr("month");
print define_Dee_HashStr("day");
print define_Dee_HashStr("hour");
print define_Dee_HashStr("minute");
print define_Dee_HashStr("second");
print define_Dee_HashStr("nanosecond");
print define_Dee_HashStr("time_t");
]]]*/
#define Dee_HashStr__nanoseconds _Dee_HashSelectC(0x587ebc6, 0xcb733bf64e31051)
#define Dee_HashStr__microseconds _Dee_HashSelectC(0xd0f0ca9e, 0x21fdc701b3dee431)
#define Dee_HashStr__milliseconds _Dee_HashSelectC(0x258c8584, 0x4ae62fd98df3908b)
#define Dee_HashStr__seconds _Dee_HashSelectC(0x60e8b29f, 0x75b48546f982eb0)
#define Dee_HashStr__minutes _Dee_HashSelectC(0x697b99e0, 0x2b17a96eb6daa7a1)
#define Dee_HashStr__hours _Dee_HashSelectC(0xb7820007, 0x699674a9e26e1066)
#define Dee_HashStr__days _Dee_HashSelectC(0x64ba5ac8, 0xbbe238b514802b8f)
#define Dee_HashStr__weeks _Dee_HashSelectC(0x480ea47c, 0x232b45f396262210)
#define Dee_HashStr__months _Dee_HashSelectC(0x343842c8, 0x659b8ab01b268775)
#define Dee_HashStr__years _Dee_HashSelectC(0x8db899c8, 0x6a5238e03b1fb05a)
#define Dee_HashStr__decades _Dee_HashSelectC(0x78678682, 0xb4b017a5614d049)
#define Dee_HashStr__centuries _Dee_HashSelectC(0xae87212e, 0x77b9ecd75eae79f5)
#define Dee_HashStr__millennia _Dee_HashSelectC(0x5d46b99f, 0x7a369d205687a08f)
#define Dee_HashStr__year _Dee_HashSelectC(0x310a1818, 0xa8d044c5ea490cb6)
#define Dee_HashStr__month _Dee_HashSelectC(0x51ca185c, 0xbe9d67504ee78acc)
#define Dee_HashStr__day _Dee_HashSelectC(0xe0fe498d, 0x8e12be46fd64d268)
#define Dee_HashStr__hour _Dee_HashSelectC(0x1a9ed795, 0x10f3e1a52760b6c5)
#define Dee_HashStr__minute _Dee_HashSelectC(0x951f07e5, 0x841bf13791a969b)
#define Dee_HashStr__second _Dee_HashSelectC(0x1a084e7b, 0x3058181b9a44b68c)
#define Dee_HashStr__nanosecond _Dee_HashSelectC(0xe8e3c3b6, 0x16f16d5d9832d2aa)
#define Dee_HashStr__time_t _Dee_HashSelectC(0xad3d6b3f, 0x79f5c2060abaa727)
/*[[[end]]]*/


PRIVATE WUNUSED NONNULL((1)) int DCALL
time_init_kw(DeeTimeObject *__restrict self,
             size_t argc, DeeObject *const *argv,
             DeeObject *kw) {
	/* >> (nanoseconds?:?Dint
	 * >> ,microseconds?:?Dint
	 * >> ,milliseconds?:?Dint
	 * >> ,seconds?:?Dint
	 * >> ,minutes?:?Dint
	 * >> ,hours?:?Dint
	 * >> ,days?:?Dint
	 * >> ,weeks?:?Dint
	 * >> ,months?:?Dint
	 * >> ,years?:?Dint
	 * >> ,decades?:?Dint
	 * >> ,centuries?:?Dint
	 * >> ,millennia?:?Dint
	 * >> ,year?:?Dint
	 * >> ,month?:?Dint
	 * >> ,day?:?Dint
	 * >> ,hour?:?Dint
	 * >> ,minute?:?Dint
	 * >> ,second?:?Dint
	 * >> ,nanosecond?:?Dint
	 * >> ,time_t?:?Dint) */
	DeeKwArgs kwds;
	Dee_int128_t arg;
	int error;
	if (DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
#define IF_LOADARG(i, name)                                   \
	if ((error = time_init_getarg(&kwds, argc, argv, i,       \
	                              #name, Dee_HashStr__##name, \
	                              &arg)) > 0)                 \
		;                                                     \
	else if unlikely(error < 0)                               \
		goto err;                                             \
	else
	self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_INVALID);
	__hybrid_int128_setzero(self->t_nanos);
	IF_LOADARG(0, nanoseconds) {
		self->t_kind  = TIME_KIND_DELTA;
		self->t_nanos = arg;
	}
	IF_LOADARG(1, microseconds) {
		time_init_addrepr(self, &arg, TIME_REPR_MICROSECONDS);
	}
	IF_LOADARG(2, milliseconds) {
		time_init_addrepr(self, &arg, TIME_REPR_MILLISECONDS);
	}
	IF_LOADARG(3, seconds) {
		time_init_addrepr(self, &arg, TIME_REPR_SECONDS);
	}
	IF_LOADARG(4, minutes) {
		time_init_addrepr(self, &arg, TIME_REPR_MINUTES);
	}
	IF_LOADARG(5, hours) {
		time_init_addrepr(self, &arg, TIME_REPR_HOURS);
	}
	IF_LOADARG(6, days) {
		time_init_addrepr(self, &arg, TIME_REPR_DAYS);
	}
	IF_LOADARG(7, weeks) {
		time_init_addrepr(self, &arg, TIME_REPR_WEEKS);
	}
	IF_LOADARG(8, months) {
		time_init_addrepr_months(self, &arg, TIME_REPR_MONTHS);
	}
	IF_LOADARG(9, years) {
		time_init_addrepr_months(self, &arg, TIME_REPR_YEARS);
	}
	IF_LOADARG(10, decades) {
		time_init_addrepr_months(self, &arg, TIME_REPR_DECADES);
	}
	IF_LOADARG(11, centuries) {
		time_init_addrepr_months(self, &arg, TIME_REPR_CENTURIES);
	}
	IF_LOADARG(12, millennia) {
		time_init_addrepr_months(self, &arg, TIME_REPR_MILLENNIA);
	}
	if (argc > 12 || DeeKwArgs_MaybeHaveMoreArgs(&kwds)) {
		union u_time_repr_kind addend_kind;
		Dee_int128_t addend;
		addend_kind.t_typekind = self->t_typekind;
		addend                 = self->t_nanos;
		self->t_typekind       = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_INVALID);
		__hybrid_int128_setzero(self->t_nanos);
		IF_LOADARG(13, year) {
			time_init_setrepr(self, &arg, TIME_REPR_YEAR);
		}
		IF_LOADARG(14, month) {
			time_init_setrepr(self, &arg, TIME_REPR_MONTH);
		}
		IF_LOADARG(15, day) {
			time_init_setrepr(self, &arg, TIME_REPR_MDAY);
		}
		IF_LOADARG(16, hour) {
			time_init_setrepr(self, &arg, TIME_REPR_HOUR);
		}
		IF_LOADARG(17, minute) {
			time_init_setrepr(self, &arg, TIME_REPR_MINUTE);
		}
		IF_LOADARG(18, second) {
			time_init_setrepr(self, &arg, TIME_REPR_SECOND);
		}
		IF_LOADARG(19, nanosecond) {
			time_init_setrepr(self, &arg, TIME_REPR_NANOSECOND);
		}
		IF_LOADARG(20, time_t) {
			__hybrid_int128_add64(arg, SECONDS_01_01_1970);
			__hybrid_int128_mul32(arg, NANOSECONDS_PER_SECOND);
			if (!__hybrid_int128_iszero(self->t_nanos)) {
				/* Keep nano-seconds set by the `nanosecond' property. */
				uint32_t extra_nano;
				__hybrid_int128_floormod32_r(self->t_nanos, NANOSECONDS_PER_SECOND, extra_nano);
				__hybrid_int128_add32(arg, extra_nano);
			}
			self->t_nanos = arg;
			self->t_typekind = TIME_TYPEKIND(TIME_TYPE_NANOSECONDS, TIME_KIND_DELTA);
		}
		if (self->t_kind != TIME_KIND_INVALID) {
			/* Produce a timestamp */
			ASSERT(self->t_kind == TIME_KIND_TIMESTAMP);
			if (addend_kind.t_kind != TIME_KIND_INVALID) {
				if (addend_kind.t_type == TIME_TYPE_MONTHS) {
					_DeeTime_GetRepr(&arg, self, TIME_REPR_MONTH);
					__hybrid_int128_add128(arg, addend);
					DeeTime_SetRepr(self, &arg, TIME_REPR_MONTH);
				} else {
					DeeTime_MakeNano(self); /* Shouldn't be necessary (but better be safe) */
					__hybrid_int128_add128(self->t_nanos, addend);
				}
			}
		} else {
			/* Produce a delta */
			self->t_typekind = addend_kind.t_typekind;
			self->t_nanos    = addend;
		}
	}

	/* Fallback: produce a zero-delta */
	if (self->t_kind == TIME_KIND_INVALID)
		self->t_kind = TIME_KIND_DELTA;
#undef IF_LOADARG
	if unlikely(DeeKwArgs_Done(&kwds, argc, "Time"))
		goto err;
	if unlikely(argc > 21)
		return DeeArg_BadArgcEx("Time", argc, 0, 21);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
time_print(DeeTimeObject *__restrict self,
           Dee_formatprinter_t printer, void *arg) {
	/* TODO: Re-design me! (and include special handling for delta timestamps) */
	return time_format(printer, arg, "%A, the %[n:mday] of %B %Y, %H:%M:%S", self);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
time_printrepr(DeeTimeObject *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "Time(");
	if unlikely(result < 0)
		goto done;
	if (DeeTime_IsDelta(self)) {
		/* Delta representation */
		if (self->t_type == TIME_TYPE_MONTHS) {
			Dee_int128_t years;
			uint8_t months;
			__hybrid_int128_floordivmod8(self->t_months, MONTHS_PER_YEAR,
			                        years, months);
			if (__hybrid_int128_iszero(years)) {
				temp = DeeFormat_Printf(printer, arg, "months: %" PRFu8, months);
			} else if (months == 0) {
				temp = DeeFormat_Printf(printer, arg, "years: %" PRFu128, years);
			} else {
				temp = DeeFormat_Printf(printer, arg, "years: %" PRFu128 ", months: %" PRFu8, years, months);
			}
			if unlikely(temp < 0)
				goto err;
			result += temp;
		} else if (!__hybrid_int128_iszero(self->t_nanos)) {
			Dee_int128_t div, nano = self->t_nanos;
			uint64_t temp64;
			uint32_t temp32;
			uint16_t temp16;
			char const *label;
			if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_CENTURY_AVG, div, temp64), temp64 == 0)) {
				label = "centuries";
			} else if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_DECADE_AVG, div, temp64), temp64 == 0)) {
				label = "decades";
			} else if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_YEAR_AVG, div, temp64), temp64 == 0)) {
				label = "years";
			} else if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_MONTH_AVG, div, temp64), temp64 == 0)) {
				label = "months";
			} else if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_WEEK, div, temp64), temp64 == 0)) {
				label = "weeks";
			} else if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_DAY, div, temp64), temp64 == 0)) {
				label = "days";
			} else if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_HOUR, div, temp64), temp64 == 0)) {
				label = "hours";
			} else if ((__hybrid_int128_floordivmod64(nano, NANOSECONDS_PER_MINUTE, div, temp64), temp64 == 0)) {
				label = "minutes";
			} else if ((__hybrid_int128_floordivmod32(nano, NANOSECONDS_PER_SECOND, div, temp32), temp32 == 0)) {
				label = "seconds";
			} else if ((__hybrid_int128_floordivmod32(nano, NANOSECONDS_PER_MILLISECOND, div, temp32), temp32 == 0)) {
				label = "milliseconds";
			} else if ((__hybrid_int128_floordivmod16(nano, NANOSECONDS_PER_MICROSECOND, div, temp16), temp16 == 0)) {
				label = "microseconds";
			} else {
				label = "nanoseconds";
				div   = nano;
			}
			DO(err, DeeFormat_Printf(printer, arg, "%s: %" PRFu128, label, div));
		}
	} else {
		uint32_t extra_nanoseconds;
		/* Timestamp representation */
		DO(err, DeeFormat_Printf(printer, arg,
		                         "year: %" PRFd128 ", "
		                         "month: %" PRFu8 ", "
		                         "day: %" PRFu8 ", "
		                         "hour: %" PRFu8 ", "
		                         "minute: %" PRFu8 ", "
		                         "second: %" PRFu8,
		                         DeeTime_GetRepr(self, TIME_REPR_YEAR),
		                         DeeTime_GetRepr8(self, TIME_REPR_MONTH),
		                         DeeTime_GetRepr8(self, TIME_REPR_MDAY),
		                         DeeTime_GetRepr8(self, TIME_REPR_HOUR),
		                         DeeTime_GetRepr8(self, TIME_REPR_MINUTE),
		                         DeeTime_GetRepr8(self, TIME_REPR_SECOND)));
		extra_nanoseconds = DeeTime_GetRepr32(self, TIME_REPR_NANOSECOND);
		if (extra_nanoseconds != 0)
			DO(err, DeeFormat_Printf(printer, arg, ", nanosecond: %" PRFu32, extra_nanoseconds));
	}
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err:
	return temp;
}


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
time_hash(DeeTimeObject *__restrict self) {
	return Dee_HashPtr(&self->t_nanos, 16);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_compare(DeeTimeObject *self, DeeObject *other) {
	Dee_int128_t lhs, rhs;
	if (DeeObject_AsInt128(other, &rhs))
		goto err;
	DeeTime_AsNano(self, &lhs);
	if (__hybrid_int128_lo128(lhs, rhs))
		return Dee_COMPARE_LO;
	if (__hybrid_int128_gr128(lhs, rhs))
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp time_cmp = {
	/* .tp_hash       = */ (Dee_hash_t(DCALL *)(DeeObject *__restrict self))&time_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&time_compare,
};


INTERN DeeTypeObject DeeTime_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Time",
	/* .tp_doc      = */ DOC("(nanoseconds?:?Dint,microseconds?:?Dint,milliseconds?:?Dint"
	                         ",seconds?:?Dint,minutes?:?Dint,hours?:?Dint,days?:?Dint,weeks?:?Dint,months?:?Dint"
	                         ",years?:?Dint,decades?:?Dint,centuries?:?Dint,millennia?:?Dint"
	                         ",year?:?Dint,month?:?Dint,day?:?Dint,hour?:?Dint,minute?:?Dint,second?:?Dint,nanosecond?:?Dint"
	                         ",time_t?:?Dint)\n"
	                         "Construct a new time object from the given arguments.\n"
	                         "Arguments whose names are singular can be used to construct a timestamp for a "
	                         /**/ "specific point in time (in this case, the time object is a ?#istimestamp). @time_t "
	                         /**/ "can be specified in place of @year, @month, @day, @hour, @minute and @second\n"
	                         "Arguments whose names are plural can be specific to construct a ?#isdelta. "
	                         /**/ "When multiple time deltas are specified, those deltas are added together.\n"
	                         "When both singular, and plural arguments are given, the constructed time "
	                         /**/ "object is a ?#istimestamp equal to that described by the singular "
	                         /**/ "arguments, and off-set by what is specified by the plural arguments\n"
	                         "When no arguments are specific, a zero-delta is constructed\n"
	                         "\n"

	                         "int->\n"
	                         "Return then number of nano-seconds described by @this time-object. "
	                         /**/ "In the case of ?#isdelta, the length of that delta is returned. In the "
	                         /**/ "case of ?#istimestamp, the total nano-seconds since #C{01-01-0000} are returned.\n"
	                         "\n"

	                         "str->\n"
	                         "Returns value of @this time object when it was constructed to "
	                         /**/ "represent an explicit view (such as through use of ?Gdays, "
	                         /**/ "or through a sub-view such as ?#days), or return the time "
	                         /**/ "represented in a human-readable fashion\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns a string representation of the components of @this time object\n"
	                         "\n"

	                         "int->\n"
	                         "Returns the value of @this time object as the number of nanoseconds since #C{01-01-0000} "
	                         /**/ "for timestamps (s.a. ?#istimestamp), or the number of delta-nanoseconds for delta time "
	                         /**/ "objects (s.a. ?#isdelta).\n"
	                         "This operator allows time objects to be passed to system functions that take "
	                         /**/ "integer timeouts in nanoseconds, such as :Thread.sleep or ?Aaccept?Enet:socket."
	                         "\n"

	                         "-(other:?.)->?.\n"
	                         "+(other:?.)->?.\n"
	                         "Adding or subtracting time objects affects time deltas as follows:\n"
	                         "#T{lhs|op|rhs|result~"
	                         /**/ "?#istimestamp|${+}|?#istimestamp|A :ValueError is thrown&"
	                         /**/ "?#istimestamp|${-}|?#istimestamp|?#isdelta&"
	                         /**/ "?#istimestamp|${+}|?#isdelta|?#istimestamp&"
	                         /**/ "?#istimestamp|${-}|?#isdelta|?#istimestamp&"
	                         /**/ "?#isdelta|${+}|?#istimestamp|?#istimestamp&"
	                         /**/ "?#isdelta|${-}|?#istimestamp|A :ValueError is thrown&"
	                         /**/ "?#isdelta|${+}|?#isdelta|?#isdelta&"
	                         /**/ "?#isdelta|${-}|?#isdelta|?#isdelta&"
	                         "}\n"
	                         "\n"

	                         "*(other:?Dint)->?.\n"
	                         "#tValueError{@this ?. object isn't a delta (s.a. ?#isdelta)}"
	                         "Multiply a delta time object by the given amount.\n"
	                         "\n"

	                         "/(other:?.)->?Dint\n"
	                         "#tValueError{@this ?. object isn't a delta (s.a. ?#isdelta)}"
	                         "#tDivideByZero{@other represents $0 nano-seconds}"
	                         "Divide 2 delta time object by each other\n"
	                         "\n"

	                         "/(other:?Dint)->?.\n"
	                         "#tValueError{@this ?. object isn't a delta (s.a. ?#isdelta)}"
	                         "#tDivideByZero{@other is $0}"
	                         "Divide a delta time object by the given amount.\n"
	                         "\n"

	                         "%(other:?.)->?.\n"
	                         "#tValueError{@this ?. object isn't a delta (s.a. ?#isdelta)}"
	                         "#tDivideByZero{@other represents $0 nano-seconds}"
	                         "Get the remainder from dividing 2 delta time object by each other.\n"
	                         "\n"

	                         "%(other:?Dint)->?.\n"
	                         "#tValueError{@this ?. object isn't a delta (s.a. ?#isdelta)}"
	                         "#tDivideByZero{@other is $0}"
	                         "Get the remainder from dividing @this time delta by the given amount."),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeTimeObject,
			/* tp_ctor:        */ &time_ctor,
			/* tp_copy_ctor:   */ &time_copy,
			/* tp_deep_ctor:   */ &time_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &time_init_kw,
			/* tp_serialize:   */ &time_serialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ (int(DCALL *)(DeeObject *__restric, DeeObject *__restrict))&time_assign,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int(DCALL *)(DeeObject *__restrict))&time_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&time_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&time_printrepr
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &time_math,
	/* .tp_cmp           = */ &time_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ time_methods,
	/* .tp_getsets       = */ time_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ time_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ time_class_members,
};


DEX_BEGIN

DEX_MEMBER_F_NODOC("Time", &DeeTime_Type, MODSYM_FREADONLY),
DEX_MEMBER_F("gmtime", &libtime_gmtime, MODSYM_FREADONLY,
             "->?GTime\n"
             "Returns the current time in UTC (s.a. ?Glocaltime)"),
DEX_MEMBER_F("localtime", &libtime_localtime, MODSYM_FREADONLY,
             "->?GTime\n"
             "Returns the current time in the host's local timezone (s.a. ?Ggmtime)"),
/* TODO: timezone()->?GTime
	* Returns the ?Aisdelta?GTime delta that gets added to ?Ggmtime in order to produce ?Glocaltime
	* XXX: That's not how that works -- the delta of a timezone isn't constant and can only be
	*      calculated when given a specific point in (UTC) time. */
DEX_MEMBER_F("tick", &libtime_tick, MODSYM_FREADONLY,
             "->?GTime\n"
             "Returns the current tick suitable for high-precision timings.\n"
             "The tick itself is offset from some undefined point in time, meaning that the only "
             /**/ "meaningful use, is to subtract the return values of two calls to this function."),
DEX_MEMBER_F("maketime", &libtime_maketime, MODSYM_FREADONLY,
             "(" libtime_maketime_params ")->?GTime\n"
             "Construct a new ?GTime object using the given arguments for the "
             /**/ "sub-day portion, while filling in the remainder as all zeroes:\n"
             "${"
             /**/ "import Time from time;\n"
             /**/ "Time(hour: hour, minute: minute, second: second, nanosecond: nanosecond);"
             "}"),
DEX_MEMBER_F("makedate", &libtime_makedate, MODSYM_FREADONLY,
             "(" libtime_makedate_params ")->?GTime\n"
             "Construct a new ?GTime object using the given arguments for the "
             /**/ "post-day portion, while filling in the remainder as all zeroes:\n"
             "${"
             /**/ "import Time from time;\n"
             /**/ "Time(year: year, month: month, day: day);"
             "}"),

/* Export various functions for constructing time deltas.
	* NOTE: These functions are highly useful for specifying timeouts,
	*       and are actually the only portable way of specifying them,
	*       as it is implementation-specific what's the time resolution
	*       that's used by functions accepting timeouts (in the GATW
	*       implementation it's nanoseconds, but that wasn't always the
	*       case, as a one point, it was microseconds):
	* >> import seconds from time;
	* >> import Thread from deemon;
	* >>
	* >> print "Begin waiting for 2 seconds";
	* >> Thread.sleep(seconds(2));
	* >> print "Done waiting";
	*/
#define DEFINE_DELTA_CALLBACK(name)                        \
	DEX_MEMBER_F(#name, &libtime_##name, MODSYM_FREADONLY, \
	             "(value:?Dint)->?GTime\n"                 \
	             "#r{A time delta of @value " #name "}")
DEFINE_DELTA_CALLBACK(nanoseconds),
DEFINE_DELTA_CALLBACK(microseconds),
DEFINE_DELTA_CALLBACK(milliseconds),
DEFINE_DELTA_CALLBACK(seconds),
DEFINE_DELTA_CALLBACK(minutes),
DEFINE_DELTA_CALLBACK(hours),
DEFINE_DELTA_CALLBACK(days),
DEFINE_DELTA_CALLBACK(weeks),
DEFINE_DELTA_CALLBACK(months),
DEFINE_DELTA_CALLBACK(years),
DEFINE_DELTA_CALLBACK(decades),
DEFINE_DELTA_CALLBACK(centuries),
DEFINE_DELTA_CALLBACK(millennia),
#undef DEFINE_DELTA_CALLBACK

#define DEFINE_DEPRECATED_DELTA_ALIAS(name, alias_for)         \
	DEX_MEMBER_F(name, &libtime_##alias_for, MODSYM_FREADONLY, \
	             "(value:?Dint)->?GTime\n"                     \
	             "Deprecated alias for ?G" #alias_for)
DEFINE_DEPRECATED_DELTA_ALIAS("mics", microseconds),
DEFINE_DEPRECATED_DELTA_ALIAS("mils", milliseconds),
DEFINE_DEPRECATED_DELTA_ALIAS("secs", seconds),
DEFINE_DEPRECATED_DELTA_ALIAS("mins", minutes),
DEFINE_DEPRECATED_DELTA_ALIAS("hors", hours),
DEFINE_DEPRECATED_DELTA_ALIAS("weks", weeks),
DEFINE_DEPRECATED_DELTA_ALIAS("mons", months),
DEFINE_DEPRECATED_DELTA_ALIAS("yers", years),
DEFINE_DEPRECATED_DELTA_ALIAS("decs", decades),
DEFINE_DEPRECATED_DELTA_ALIAS("cens", centuries),
DEFINE_DEPRECATED_DELTA_ALIAS("mlls", millennia),
#undef DEFINE_DEPRECATED_DELTA_ALIAS

DEX_MEMBER_F("now", &libtime_localtime, MODSYM_FREADONLY,
             "->?GTime\n"
             "Deprecated alias for ?Glocaltime"),

DEX_MEMBER_F("_mkunix", &libtime__mkunix, MODSYM_FREADONLY,
             "(" libtime__mkunix_params ")->?GTime\n"
             "Construct a new anonymous timestamp object, from @time_t as seconds-"
             /**/ "since-#C{01-01-1970}, and the accompanying extra @nanosecond addend."),
#ifdef CONFIG_HOST_WINDOWS
DEX_MEMBER_F("_mkFILETIME", &libtime__mkFILETIME, MODSYM_FREADONLY,
             "(" libtime__mkFILETIME_params ")->?GTime\n"
             "Construct a new anonymous timestamp object, from 1/100th nanoseconds since #C{01-01-1601}"),
#endif /* CONFIG_HOST_WINDOWS */
DEX_END(NULL, NULL, NULL);

DECL_END


#endif /* !GUARD_DEX_TIME_LIBTIME_C */
