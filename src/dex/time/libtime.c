/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_TIME_LIBTIME_C
#define GUARD_DEX_TIME_LIBTIME_C 1
#define CONFIG_BUILDING_LIBTIME 1
#define DEE_SOURCE 1

#include "libtime.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h>

#include <hybrid/atomic.h>

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifndef INT32_MIN
#include <hybrid/limitcore.h>
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */

#ifndef INT32_MAX
#include <hybrid/limitcore.h>
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */

DECL_BEGIN

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#ifdef CONFIG_HAVE_memcasecmp
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#else /* CONFIG_HAVE_memcasecmp */
#define MEMCASEEQ(a, b, s) dee_memcaseeq((uint8_t *)(a), (uint8_t *)(b), s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
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
#define FILETIME_GET64(x) (((x) << 32)|((x) >> 32))
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#define FILETIME_GET64(x)   (x)
#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */

#ifdef CONFIG_HOST_WINDOWS
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
#endif /* CONFIG_HOST_WINDOWS */


INTERN WUNUSED dtime_t DCALL time_now(void) {
#ifdef CONFIG_HOST_WINDOWS
#define FILETIME_PER_SECONDS 10000000 /* 100 nanoseconds / 0.1 microseconds. */
	dtime_t result;
	uint64_t filetime;
	SYSTEMTIME systime;
	DBG_ALIGNMENT_DISABLE();
	if (pdyn_GetSystemTimePreciseAsFileTime == NULL) {
		HMODULE hKernel32 = GetKernel32Handle();
		if (!hKernel32)
			ATOMIC_WRITE(*(void **)&pdyn_GetSystemTimePreciseAsFileTime, (void *)(uintptr_t)-1);
		else {
			LPGETSYSTEMTIMEPRECISEASFILETIME func;
			func = (LPGETSYSTEMTIMEPRECISEASFILETIME)GetProcAddress(hKernel32, "GetSystemTimePreciseAsFileTime");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			ATOMIC_WRITE(pdyn_GetSystemTimePreciseAsFileTime, func);
		}
	}
	if (pdyn_GetSystemTimePreciseAsFileTime != (LPGETSYSTEMTIMEPRECISEASFILETIME)-1) {
		GetSystemTimePreciseAsFileTime((LPFILETIME)&filetime);
	} else {
		GetSystemTimeAsFileTime((LPFILETIME)&filetime);
	}
	/* System-time only has millisecond-precision, so we copy over that part. */
	result = (FILETIME_GET64(filetime) / (FILETIME_PER_SECONDS /
	                                      MICROSECONDS_PER_SECOND)) %
	         MICROSECONDS_PER_MILLISECOND;
	FileTimeToSystemTime((LPFILETIME)&filetime, &systime);
	SystemTimeToTzSpecificLocalTime(NULL, &systime, &systime);
	SystemTimeToFileTime(&systime, (LPFILETIME)&filetime);
	DBG_ALIGNMENT_ENABLE();
	/* Copy over millisecond information and everything above. */
	result += (FILETIME_GET64(filetime) / (FILETIME_PER_SECONDS / MICROSECONDS_PER_SECOND));
	/* Window's filetime started counting on 01.01.1601, but we've started on 01.01.0000 */
	return result + time_yer2day(1601) * MICROSECONDS_PER_DAY;
#else
	/* TODO: clock_gettime() */
	/* TODO: gettimeofday() */
	time_t now;
	DBG_ALIGNMENT_DISABLE();
	now = time(NULL);
	DBG_ALIGNMENT_ENABLE();
	return ((dtime_t)now * MICROSECONDS_PER_SECOND +
	        time_yer2day(1970) * MICROSECONDS_PER_DAY);
#endif
}

#define NAMEOF_JAN "Jan\0" "January\0"
#define NAMEOF_FEB "Feb\0" "February\0"
#define NAMEOF_MAR "Mar\0" "March\0"
#define NAMEOF_APR "Apr\0" "April\0"
#define NAMEOF_MAY "May\0" "May\0"
#define NAMEOF_JUN "Jun\0" "June\0"
#define NAMEOF_JUL "Jul\0" "July\0"
#define NAMEOF_AUG "Aug\0" "August\0"
#define NAMEOF_SEP "Sep\0" "September\0"
#define NAMEOF_OCT "Oct\0" "October\0"
#define NAMEOF_NOV "Nov\0" "November\0"
#define NAMEOF_DEC "Dec\0" "December"

#define OFFSETOF_NAMEOF_JAN  0
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

PRIVATE char const abbr_wday_names[7][4]  = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
PRIVATE char const full_wday_names[7][10] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
PRIVATE char const am_pm[2][3]            = { "AM", "PM" };
PRIVATE char const am_pm_lower[2][3]      = { "am", "pm" };

#define GETSTRING_WDAY_ABBR(i)  (abbr_wday_names[(unsigned int)(i)])
#define GETSTRING_WDAY_FULL(i)  (full_wday_names[(unsigned int)(i)])
#define GETSTRING_MONTH_ABBR(i) (month_names + month_info[0][(unsigned int)(i)].m_name)
#define GETSTRING_MONTH_FULL(i) (month_names + month_info[0][(unsigned int)(i)].m_name + 4)

/* Month names in English. */
PRIVATE char const month_names[] =
	NAMEOF_JAN
	NAMEOF_FEB
	NAMEOF_MAR
	NAMEOF_APR
	NAMEOF_MAY
	NAMEOF_JUN
	NAMEOF_JUL
	NAMEOF_AUG
	NAMEOF_SEP
	NAMEOF_OCT
	NAMEOF_NOV
	NAMEOF_DEC
;

struct month {
	uint16_t m_start; /* Start of the month */
	uint16_t m_end;   /* End of the month */
	uint16_t m_len;   /* == end-start */
	uint16_t m_name;  /* Offset into month_names to the month's name.
	                   * NOTE: The full name can be found at `m_name+4' */
};

PRIVATE unsigned int const year_length[2] = { 365, 366 };

PRIVATE struct month const month_info[2][MONTHS_PER_YEAR] = {
	/* No leap year: */
	{
		{ 0, 31, 31, OFFSETOF_NAMEOF_JAN },
		{ 31, 59, 28, OFFSETOF_NAMEOF_FEB },
		{ 59, 90, 31, OFFSETOF_NAMEOF_MAR },
		{ 90, 120, 30, OFFSETOF_NAMEOF_APR },
		{ 120, 151, 31, OFFSETOF_NAMEOF_MAY },
		{ 151, 181, 30, OFFSETOF_NAMEOF_JUN },
		{ 181, 212, 31, OFFSETOF_NAMEOF_JUL },
		{ 212, 243, 31, OFFSETOF_NAMEOF_AUG },
		{ 243, 273, 30, OFFSETOF_NAMEOF_SEP },
		{ 273, 304, 31, OFFSETOF_NAMEOF_OCT },
		{ 304, 334, 30, OFFSETOF_NAMEOF_NOV },
		{ 334, 365, 31, OFFSETOF_NAMEOF_DEC }
	},
	/* leap year: */
	{
		{ 0, 31, 31, OFFSETOF_NAMEOF_JAN },
		{ 31, 60, 29, OFFSETOF_NAMEOF_FEB },
		{ 60, 91, 31, OFFSETOF_NAMEOF_MAR },
		{ 91, 121, 30, OFFSETOF_NAMEOF_APR },
		{ 121, 152, 31, OFFSETOF_NAMEOF_MAY },
		{ 152, 182, 30, OFFSETOF_NAMEOF_JUN },
		{ 182, 213, 31, OFFSETOF_NAMEOF_JUL },
		{ 213, 244, 31, OFFSETOF_NAMEOF_AUG },
		{ 244, 274, 30, OFFSETOF_NAMEOF_SEP },
		{ 274, 305, 31, OFFSETOF_NAMEOF_OCT },
		{ 305, 335, 30, OFFSETOF_NAMEOF_NOV },
		{ 335, 366, 31, OFFSETOF_NAMEOF_DEC }
	}
};


#define time_day2mon(x) time_day2mon((dtime_half_t)(x))
#define time_mon2day(x) time_mon2day((dtime_half_t)(x))

PRIVATE WUNUSED dtime_half_t
(TIMECALL time_day2mon)(dtime_half_t x) {
	unsigned int i;
	struct month const *chain;
	dtime_half_t years = time_day2yer(x);
	x -= time_yer2day(years);
	chain = month_info[time_isleapyear(years)];
	for (i = 0; x >= chain[i].m_end && i < MONTHS_PER_YEAR; ++i)
		;
	return (years * MONTHS_PER_YEAR) + i;
}

PRIVATE WUNUSED dtime_half_t
(TIMECALL time_mon2day)(dtime_half_t x) {
	dtime_half_t years  = x / MONTHS_PER_YEAR;
	dtime_half_t result = month_info[time_isleapyear(years)][x % MONTHS_PER_YEAR].m_start;
	return result + time_yer2day(years);
}


PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeTime_New)(uint64_t microseconds) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_kind = TIME_KIND(TIME_MICROSECONDS,
	                           TIME_REPR_NONE);
	result->t_time = microseconds;
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeTime_New_(dtime_t microseconds, uint16_t kind) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_kind = kind;
	result->t_time = microseconds;
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeTime_NewMonths_(dtime_half_t num_months, uint16_t kind) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_kind   = kind;
	result->t_months = num_months;
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) dtime_t DCALL
DeeTime_Get(DeeTimeObject const *__restrict self) {
	if (self->t_type == TIME_MICROSECONDS)
		return self->t_time;
	return ((dtime_t)time_mon2day(self->t_months)) * MICROSECONDS_PER_DAY;
}

#ifdef HAVE_128BIT_TIME
INTERN WUNUSED DREF DeeObject *DCALL
DeeTime_New64_(uint64_t microseconds, uint16_t kind) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_kind = kind;
	_Time_Set64(*result, microseconds);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) uint64_t DCALL
DeeTime_Get64(DeeTimeObject const *__restrict self) {
	if (self->t_type == TIME_MICROSECONDS)
		return _Time_Get64(*self);
	return (uint64_t)(((uint64_t)time_mon2day(self->t_months)) * MICROSECONDS_PER_DAY);
}
#endif /* HAVE_128BIT_TIME */

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
	{ "mic", TIME_REPR_MIC },
	{ "mil", TIME_REPR_MIL },
	{ "sec", TIME_REPR_SEC },
	{ "min", TIME_REPR_MIN },
	{ "hor", TIME_REPR_HOR },
	{ "wday", TIME_REPR_WDAY },
	{ "mwek", TIME_REPR_MWEK },
	{ "mon", TIME_REPR_MON },
	{ "yer", TIME_REPR_YER },
	{ "dec", TIME_REPR_DEC },
	{ "cen", TIME_REPR_CEN },
	{ "mll", TIME_REPR_MLL },
	{ "mday", TIME_REPR_MDAY },
	{ "yday", TIME_REPR_YDAY },
	{ "ywek", TIME_REPR_YWEK },
	{ "mics", TIME_REPR_MICS },
	{ "mils", TIME_REPR_MILS },
	{ "secs", TIME_REPR_SECS },
	{ "mins", TIME_REPR_MINS },
	{ "hors", TIME_REPR_HORS },
	{ "days", TIME_REPR_DAYS },
	{ "weks", TIME_REPR_WEKS },
	{ "mons", TIME_REPR_MONS },
	{ "yers", TIME_REPR_YER },
	{ "decs", TIME_REPR_DEC },
	{ "cens", TIME_REPR_CEN },
	{ "mlls", TIME_REPR_MLL },
	{ "mweek", TIME_REPR_MWEK }, /* Middle-way alias */
	{ "yweek", TIME_REPR_YWEK }, /* Middle-way alias */
	{ "Microsecond", TIME_REPR_MIC },
	{ "Millisecond", TIME_REPR_MIL },
	{ "Second", TIME_REPR_SEC },
	{ "Minute", TIME_REPR_MIN },
	{ "Hour", TIME_REPR_HOR },
	{ "Weekday", TIME_REPR_WDAY },
	{ "MonthWeek", TIME_REPR_MWEK },
	{ "Month", TIME_REPR_MON },
	{ "Year", TIME_REPR_YER },
	{ "Decade", TIME_REPR_DEC },
	{ "Century", TIME_REPR_CEN },
	{ "Millennium", TIME_REPR_MLL },
	{ "MonthDay", TIME_REPR_MDAY },
	{ "YearDay", TIME_REPR_YDAY },
	{ "YearWeek", TIME_REPR_YWEK },
	{ "Microseconds", TIME_REPR_MICS },
	{ "Milliseconds", TIME_REPR_MILS },
	{ "Seconds", TIME_REPR_SECS },
	{ "Minutes", TIME_REPR_MINS },
	{ "Hours", TIME_REPR_HORS },
	{ "Days", TIME_REPR_DAYS },
	{ "Weeks", TIME_REPR_WEKS },
	{ "Months", TIME_REPR_MONS },
	{ "Years", TIME_REPR_YER },
	{ "Decades", TIME_REPR_DEC },
	{ "Centuries", TIME_REPR_CEN },
	{ "Millennia", TIME_REPR_MLL }
};

PRIVATE WUNUSED char const *DCALL
get_repr_name(uint8_t repr) {
	struct repr_name const *iter;
	char const *result = NULL;
	/* Search for the last entry (which contants the longest name) */
	for (iter = repr_desc;
	     iter != COMPILER_ENDOF(repr_desc); ++iter) {
		if (iter->repr == repr)
			result = iter->name;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) uint8_t DCALL
get_repr_id(char const *__restrict name, size_t length) {
	uint8_t result = TIME_REPR_NONE;
	struct repr_name const *iter;
	if (length < COMPILER_LENOF(repr_desc[0].name)) {
		for (iter = repr_desc;
		     iter != COMPILER_ENDOF(repr_desc); ++iter) {
			if (MEMCASEEQ(iter->name, name, length * sizeof(char))) {
				result = iter->repr;
				break;
			}
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_asrepr(DeeTimeObject *__restrict self, uint8_t repr) {
	DREF DeeTimeObject *result;
	/* Optimization: When the same representation,
	 *               just re-return the given base-time. */
	if (self->t_repr == repr)
		goto return_self;
	/* Optimization: When not shared, inplace-modify the given time. */
	if (!DeeObject_IsShared(self)) {
		self->t_repr = repr;
		goto return_self;
	}
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_type = self->t_type;
	result->t_repr = repr;
	result->t_time = self->t_time;
done:
	return result;
return_self:
	return_reference_(self);
}

PRIVATE WUNUSED DREF DeeTimeObject *DCALL
time_as(DeeTimeObject *self,
        size_t argc, DeeObject *const *argv) {
	struct repr_name const *iter;
	DeeObject *name;
	char const *str;
	size_t len;
	if (DeeArg_Unpack(argc, argv, "o:as", &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		return NULL;
	str = DeeString_STR(name);
	len = DeeString_SIZE(name);
	if (len < COMPILER_LENOF(repr_desc[0].name)) {
		for (iter = repr_desc;
		     iter != COMPILER_ENDOF(repr_desc); ++iter) {
			if (MEMCASEEQ(iter->name, str, len * sizeof(char)))
				return time_asrepr(self, iter->repr);
		}
	}
	DeeError_Throwf(&DeeError_ValueError,
	                "Unknown string representation %r",
	                name);
	return NULL;
}

PRIVATE WUNUSED dtime_t DCALL
time_getint(DeeTimeObject *__restrict self,
            uint8_t repr) {
	dtime_t msecs, result;
	if (self->t_type == TIME_MONTHS) {
		switch (repr) {

		case TIME_REPR_MIC:
		case TIME_REPR_MIL:
		case TIME_REPR_SEC:
		case TIME_REPR_MIN:
		case TIME_REPR_HOR:
		case TIME_REPR_MDAY:
			result = 0;
			goto done;

		case TIME_REPR_YDAY: {
			dtime_half_t year;
			year   = self->t_months / MONTHS_PER_YEAR;
			result = month_info[time_isleapyear(year)][self->t_months % MONTHS_PER_YEAR].m_start;
			result += time_yer2day(year);
		}	goto done;

		case TIME_REPR_MON:
			result = self->t_months % MONTHS_PER_YEAR;
			goto done;

		case TIME_REPR_YER:
			result = self->t_months / MONTHS_PER_YEAR;
			goto done;

		case TIME_REPR_DEC:
			result = self->t_months / (MONTHS_PER_YEAR * YEARS_PER_DECADE);
			goto done;

		case TIME_REPR_CEN:
			result = self->t_months / (MONTHS_PER_YEAR * YEARS_PER_DECADE * DECADES_PER_CENTURY);
			goto done;

		case TIME_REPR_MLL:
			result = self->t_months / (MONTHS_PER_YEAR * YEARS_PER_DECADE * DECADES_PER_CENTURY * CENTURIES_PER_MILLENIUM);
			goto done;

		case TIME_REPR_MONS:
			result = self->t_months;
			goto done;

		default: break;
		}
	}
	msecs = DeeTime_Get(self);
	switch (repr) {

	case TIME_REPR_MIC:
		result = time_get_microseconds(msecs) % MICROSECONDS_PER_MILLISECOND;
		goto done;

	case TIME_REPR_MIL:
		result = time_get_milliseconds(msecs) % MILLISECONDS_PER_SECOND;
		goto done;

	case TIME_REPR_SEC:
		result = time_get_seconds(msecs) % SECONDS_PER_MINUTE;
		goto done;

	case TIME_REPR_MIN:
		result = time_get_minutes(msecs) % MINUTES_PER_HOUR;
		goto done;

	case TIME_REPR_HOR:
		result = time_get_hours(msecs) % HOURS_PER_DAY;
		goto done;

	case TIME_REPR_WDAY:
		result = time_get_days(msecs) % DAYS_PER_WEEK;
		goto done;

	case TIME_REPR_MON:
		result = time_get_months(msecs) % MONTHS_PER_YEAR;
		goto done;

	case TIME_REPR_YER:
		result = time_get_years(msecs);
		goto done;

	case TIME_REPR_DEC:
		result = time_get_decades(msecs);
		goto done;

	case TIME_REPR_CEN:
		result = time_get_centuries(msecs);
		goto done;

	case TIME_REPR_MLL:
		result = time_get_millenia(msecs);
		goto done;

	case TIME_REPR_YDAY:
		result = time_get_days(msecs);
		result -= time_yer2day(time_day2yer(result));
		break;

	case TIME_REPR_MDAY: {
		dtime_half_t month;
		dtime_half_t year;
		result = time_get_days(msecs);
		month  = time_day2mon(result);
		year   = month / MONTHS_PER_YEAR;
		month %= MONTHS_PER_YEAR;
		result -= time_yer2day(year);
		/* Result is now the year-day. */
		result -= month_info[time_isleapyear(year)][month].m_start;
	}	goto done;

	case TIME_REPR_MWEK: {
		dtime_half_t month;
		dtime_half_t month_start;
		result      = time_get_days(msecs);
		month       = time_day2mon(result);
		month_start = time_mon2day(month);
		result /= DAYS_PER_WEEK;
		month_start /= DAYS_PER_WEEK;
		result = result - month_start;
	}	goto done;

	case TIME_REPR_YWEK: {
		dtime_half_t year;
		dtime_half_t year_start;
		result     = time_get_days(msecs);
		year       = (dtime_half_t)time_day2yer(result);
		year_start = time_yer2day(year);
		result /= DAYS_PER_WEEK;
		year_start /= DAYS_PER_WEEK;
		result = result - year_start;
	}	goto done;

	case TIME_REPR_MICS:
		result = time_get_microseconds(msecs);
		goto done;

	case TIME_REPR_MILS:
		result = time_get_milliseconds(msecs);
		goto done;

	case TIME_REPR_SECS:
		result = time_get_seconds(msecs);
		goto done;

	case TIME_REPR_MINS:
		result = time_get_minutes(msecs);
		goto done;

	case TIME_REPR_HORS:
		result = time_get_hours(msecs);
		goto done;

	case TIME_REPR_DAYS:
		result = time_get_days(msecs);
		goto done;

	case TIME_REPR_WEKS:
		result = time_get_weeks(msecs);
		goto done;

	case TIME_REPR_MONS:
		result = time_get_months(msecs);
		goto done;

	default: __builtin_unreachable();
	}
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
time_setint(DeeTimeObject *__restrict self,
            uint8_t repr, dtime_t value) {
	dtime_t newval;
	switch (repr) {

	case TIME_REPR_MIC:
		if ((dutime_t)value >= MICROSECONDS_PER_MILLISECOND)
			goto err_invalid_value;
		newval = DeeTime_Get(self);
		newval = ((newval / MICROSECONDS_PER_MILLISECOND) *
		          MICROSECONDS_PER_MILLISECOND) +
		         value;
		break;

	case TIME_REPR_MIL:
		if ((dutime_t)value >= MILLISECONDS_PER_SECOND)
			goto err_invalid_value;
		newval = DeeTime_Get(self);
		newval = (((newval / MICROSECONDS_PER_MILLISECOND) + value) * MICROSECONDS_PER_MILLISECOND) +
		         (newval % MICROSECONDS_PER_MILLISECOND);
		break;

	case TIME_REPR_SEC:
		if ((dutime_t)value >= SECONDS_PER_MINUTE)
			goto err_invalid_value;
		newval = DeeTime_Get(self);
		newval = (((newval / MICROSECONDS_PER_SECOND) + value) * MICROSECONDS_PER_SECOND) +
		         (newval % MICROSECONDS_PER_SECOND);
		break;

	case TIME_REPR_MIN:
		if ((dutime_t)value >= MINUTES_PER_HOUR)
			goto err_invalid_value;
		newval = DeeTime_Get(self);
		newval = (((newval / MICROSECONDS_PER_SECOND) + value) * MICROSECONDS_PER_SECOND) +
		         (newval % MICROSECONDS_PER_SECOND);
		break;

	case TIME_REPR_HOR:
		if ((dutime_t)value >= HOURS_PER_DAY)
			goto err_invalid_value;
		newval = DeeTime_Get(self);
		newval = (((newval / MICROSECONDS_PER_HOUR) + value) * MICROSECONDS_PER_HOUR) +
		         (newval % MICROSECONDS_PER_HOUR);
		break;

	case TIME_REPR_WDAY:
		if ((dutime_t)value >= DAYS_PER_WEEK)
			goto err_invalid_value;
		newval = DeeTime_Get(self);
		newval = (((newval / MICROSECONDS_PER_DAY) + value) * MICROSECONDS_PER_DAY) +
		         (newval % MICROSECONDS_PER_DAY);
		break;

	case TIME_REPR_MON: {
		dtime_half_t year;
		dtime_half_t month_day;
		dtime_half_t old_month;
		if ((dutime_t)value >= MONTHS_PER_YEAR)
			goto err_invalid_value;
		if (self->t_type == TIME_MONTHS) {
			self->t_months = (dtime_half_t)value;
			goto done;
		}
		newval    = DeeTime_Get(self);
		month_day = (dtime_half_t)time_get_days(newval);
		old_month = time_day2mon(month_day);
		year      = old_month / MONTHS_PER_YEAR;
		month_day = (dtime_t)time_yer2day(year);
		month_day -= month_day;
		month_day -= month_info[time_isleapyear(year)][(unsigned int)old_month % MONTHS_PER_YEAR].m_start;
		month_day += month_info[time_isleapyear(year)][(unsigned int)value % MONTHS_PER_YEAR].m_start;
		/* Add the offset into the month (May roll over into the next month/year if the day doesn't exist) */
		month_day += month_day;
		newval = (((newval / MICROSECONDS_PER_DAY) + month_day) * MICROSECONDS_PER_DAY) +
		         (newval % MICROSECONDS_PER_DAY);
	}	break;

	{
		dtime_half_t arg, year, days;
	case TIME_REPR_MLL:
		arg = (dtime_half_t)value * YEARS_PER_MILLENIUM;
		goto do_set_year;

	case TIME_REPR_CEN:
		arg = (dtime_half_t)value * YEARS_PER_CENTURY;
		goto do_set_year;

	case TIME_REPR_DEC:
		arg = (dtime_half_t)value * YEARS_PER_DECADE;
		goto do_set_year;

	case TIME_REPR_YER:
		arg = (dtime_half_t)value;
do_set_year:
		if (self->t_type == TIME_MONTHS) {
			self->t_months = (dtime_half_t)value * MONTHS_PER_YEAR;
			goto done;
		}
		newval = DeeTime_Get(self);
		days   = (dtime_half_t)(newval / MICROSECONDS_PER_DAY);
		year   = time_day2yer(days);
		days   = time_yer2day(year);
		days += time_yer2day(arg);
		newval -= (dtime_t)days * MICROSECONDS_PER_DAY;
	}	break;

	case TIME_REPR_YDAY: {
		dtime_half_t new_days;
		dtime_half_t year;
		newval   = DeeTime_Get(self);
		new_days = (dtime_half_t)(newval / MICROSECONDS_PER_DAY);
		year     = time_day2yer(new_days);
		if ((dutime_t)value >= year_length[time_isleapyear(year)])
			goto err_invalid_value;
		new_days = time_yer2day(year); /* Truncate to the start of the year. */
		new_days += (dtime_half_t)value;
		newval = ((newval % MICROSECONDS_PER_DAY) +
		          (dtime_t)new_days * MICROSECONDS_PER_DAY);
	}	break;

	case TIME_REPR_MDAY: {
		dtime_half_t months;
		dtime_half_t year;
		dtime_half_t new_days;
		struct month const *info;
		newval   = DeeTime_Get(self);
		new_days = (dtime_half_t)(newval / MICROSECONDS_PER_DAY);
		months   = time_day2mon(new_days);
		year     = months / MONTHS_PER_YEAR;
		new_days = time_yer2day(year); /* Start of year */
		info     = &month_info[time_isleapyear(year)][months % MONTHS_PER_YEAR];
		if ((dutime_t)value >= info->m_len)
			goto err_invalid_value;
		new_days += info->m_start;       /* Start of month */
		new_days += (dtime_half_t)value; /* Add given offset. */
		newval = ((newval % MICROSECONDS_PER_DAY) +
		          (dtime_t)new_days * MICROSECONDS_PER_DAY);
	}	break;

	case TIME_REPR_MWEK: {
		dtime_half_t new_weeks;
		dtime_half_t days;
		dtime_half_t month;
		dtime_half_t month_start;
		/* Because there isn't a conclusive ruling as to what month a
		 * week crossing over between two should truly belong to, this
		 * implementation defines the first week (week 0) as the one that
		 * may cross over from the previous month to the current, yet still
		 * does allow setting a week even when that week will cause the
		 * month to change and point to the following one.
		 * Because this would only leave an exception for February in non-leap years,
		 * we can already pre-check the limit of everything else using this static check.
		 * NOTE: We compare using `>' and `>=' because we allow setting the last
		 *       week for the chance that it bleeds over to the next month! */
		if ((dutime_t)value > ceildiv(MAX_DAYS_PER_MONTH, DAYS_PER_WEEK))
			goto err_invalid_value;
		newval      = DeeTime_Get(self);
		days        = (dtime_half_t)time_get_days(newval);
		month       = time_day2mon(days);
		month_start = time_mon2day(month);
		/* Special case: Changing the week of February in a non-leap year
		 *               limits the max number of weeks to 5, rather than
		 *               the usual 6:
		 *               >> ceildiv(31, 7) --> 6
		 *               >> ceildiv(30, 7) --> 6
		 *               >> ceildiv(29, 7) --> 6
		 *               >> ceildiv(28, 7) --> 5 // This case!
		 */
		if ((month % MONTHS_PER_YEAR) == 1 &&
		    (unsigned int)value > 28 / DAYS_PER_WEEK &&
		    !time_isleapyear(month / MONTHS_PER_YEAR))
			goto err_invalid_value;
		new_weeks = month_start / DAYS_PER_WEEK;
		new_weeks += (dtime_half_t)value; /* Add given offset. */
		days %= DAYS_PER_WEEK;
		newval = (((dtime_t)newval % MICROSECONDS_PER_WEEK) +
		          ((dtime_t)days * MICROSECONDS_PER_DAY) +
		          ((dtime_t)new_weeks * MICROSECONDS_PER_WEEK));
	}	break;

	case TIME_REPR_YWEK: {
		dtime_half_t year;
		dtime_half_t days;
		/* Just as with Month-week, we allow the user
		 * to let the week bleed into the next year.
		 * Yet because `ceildiv(365, 7) == ceildiv(366, 7)', there is no
		 * special case to separate between leap and non-leap years.
		 * NOTE: We compare using `>' and `>=' because we allow setting the last
		 *       week for the chance that it bleeds over to the next year! */
		if ((dutime_t)value > ceildiv(MAX_DAYS_PER_YEAR, DAYS_PER_WEEK))
			goto err_invalid_value;
		newval = DeeTime_Get(self);
		days   = (dtime_half_t)(newval / MICROSECONDS_PER_DAY);
		year   = time_day2yer(days);
		days   = time_yer2day(year);
		newval = (((dtime_t)newval % MICROSECONDS_PER_WEEK) + /* Sub-week time. */
		          ((dtime_t)days * MICROSECONDS_PER_DAY) +    /* Year start */
		          ((dtime_t)value * MICROSECONDS_PER_WEEK));  /* Year week */
	}	break;

	case TIME_REPR_MICS:
		newval = value;
		break;

	case TIME_REPR_MILS:
		newval = ((DeeTime_Get(self) % MICROSECONDS_PER_MILLISECOND) +
		          (value * MICROSECONDS_PER_MILLISECOND));
		break;

	case TIME_REPR_SECS:
		newval = ((DeeTime_Get(self) % MICROSECONDS_PER_SECOND) +
		          (value * MICROSECONDS_PER_SECOND));
		break;

	case TIME_REPR_MINS:
		newval = ((DeeTime_Get(self) % MICROSECONDS_PER_MINUTE) +
		          (value * MICROSECONDS_PER_MINUTE));
		break;

	case TIME_REPR_HORS:
		newval = ((DeeTime_Get(self) % MICROSECONDS_PER_HOUR) +
		          (value * MICROSECONDS_PER_HOUR));
		break;

	case TIME_REPR_DAYS:
		newval = ((DeeTime_Get(self) % MICROSECONDS_PER_DAY) +
		          (value * MICROSECONDS_PER_DAY));
		break;

	case TIME_REPR_WEKS:
		newval = ((DeeTime_Get(self) % MICROSECONDS_PER_WEEK) +
		          (value * MICROSECONDS_PER_WEEK));
		break;

	case TIME_REPR_MONS: {
		dtime_half_t month_start;
		dtime_half_t year;
		if (self->t_type == TIME_MONTHS) {
			self->t_months = (dtime_half_t)value;
			goto done;
		}
		newval      = DeeTime_Get(self);
		month_start = (dtime_half_t)(newval / MICROSECONDS_PER_DAY);
		month_start = time_day2mon(month_start);
		month_start = time_mon2day(month_start);
		year        = (dtime_half_t)value / MONTHS_PER_YEAR;
		value %= MONTHS_PER_YEAR;
		newval -= (dtime_t)month_start * MICROSECONDS_PER_DAY;                    /* Strip year + month. */
		newval += time_yer2day(year);                                             /* Add start of year. */
		newval += month_info[time_isleapyear(year)][(unsigned int)value].m_start; /* Add start of month. */
	}	break;

	default: __builtin_unreachable();
	}
	self->t_type = TIME_MICROSECONDS;
	self->t_time = newval;
done:
	return 0;
err_invalid_value:
	DeeError_Throwf(&DeeError_ValueError,
#ifdef HAVE_128BIT_TIME
	                "Invalid value %K for %s", DeeInt_NewS128(value),
#else /* HAVE_128BIT_TIME */
	                "Invalid value %I64d for %s", value,
#endif /* !HAVE_128BIT_TIME */
	                get_repr_name(repr));
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
time_format(struct unicode_printer *__restrict printer,
            char const *__restrict format,
            DeeTimeObject *__restrict self) {
#define print(p, s)                                                    \
	do {                                                               \
		if unlikely((temp = unicode_printer_print(printer, p, s)) < 0) \
			goto err;                                                  \
		result += temp;                                                \
	} __WHILE0
#define printf(...)                                                            \
	do {                                                                       \
		if unlikely((temp = unicode_printer_printf(printer, __VA_ARGS__)) < 0) \
			goto err;                                                          \
		result += temp;                                                        \
	} __WHILE0
	unsigned int number;
	char const *text;
	dssize_t result = 0, temp;
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
				text = GETSTRING_WDAY_ABBR(time_getint(self, TIME_REPR_WDAY));
print_text:
				print(text, strlen(text));
				break;

			case 'A':
				text = GETSTRING_WDAY_FULL(time_getint(self, TIME_REPR_WDAY));
				goto print_text;

			case 'h':
			case 'b':
				text = GETSTRING_MONTH_ABBR(time_getint(self, TIME_REPR_MON));
				goto print_text;

			case 'B':
				text = GETSTRING_MONTH_FULL(time_getint(self, TIME_REPR_MON));
				goto print_text;

			case 'c':
				printf("%s %s %0.2u %0.2u:%0.2u:%0.2u %u",
				       GETSTRING_WDAY_ABBR(time_getint(self, TIME_REPR_WDAY)),
				       GETSTRING_MONTH_ABBR(time_getint(self, TIME_REPR_MON)),
				       (unsigned int)time_getint(self, TIME_REPR_MDAY) + 1,
				       (unsigned int)time_getint(self, TIME_REPR_HOR),
				       (unsigned int)time_getint(self, TIME_REPR_MIN),
				       (unsigned int)time_getint(self, TIME_REPR_SEC),
				       (unsigned int)time_getint(self, TIME_REPR_YER));
				break;

			case 'x':
				printf("%0.2u/%0.2u/%0.2u",
				       (unsigned int)time_getint(self, TIME_REPR_MON) + 1,
				       (unsigned int)time_getint(self, TIME_REPR_MDAY) + 1,
				       (unsigned int)time_getint(self, TIME_REPR_YER));
				break;

			case 'X':
				printf("%0.2u:%0.2u:%0.2u",
				       (unsigned int)time_getint(self, TIME_REPR_HOR),
				       (unsigned int)time_getint(self, TIME_REPR_MIN),
				       (unsigned int)time_getint(self, TIME_REPR_SEC));
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
				number = ((unsigned int)time_getint(self, TIME_REPR_YER) / 100) % 100;
print_number_2:
				printf("%0.2u", number);
				break;

			case 'd':
				number = (unsigned int)time_getint(self, TIME_REPR_MDAY) + 1;
				goto print_number_2;

			case 'D':
				printf("%0.2u/%0.2u/%0.2u",
				       (unsigned int)time_getint(self, TIME_REPR_MON) + 1,
				       (unsigned int)time_getint(self, TIME_REPR_MDAY) + 1,
				       (unsigned int)(time_getint(self, TIME_REPR_YER) % 100));
				break;

			case 'e':
				number = (unsigned int)time_getint(self, TIME_REPR_MDAY) + 1;
/*print_number_2_spc:*/
				printf("%02u", number);
				break;

			case 'F':
				printf("%0.4u-%0.2u-%0.2u",
				       (unsigned int)time_getint(self, TIME_REPR_YER),
				       (unsigned int)time_getint(self, TIME_REPR_MON) + 1,
				       (unsigned int)time_getint(self, TIME_REPR_MDAY) + 1);
				break;

			case 'H':
				number = (unsigned int)time_getint(self, TIME_REPR_HOR);
				goto print_number_2;

			case 'I':
				number = (unsigned int)time_getint(self, TIME_REPR_HOR) % 12;
				goto print_number_2;

			case 'j':
				number = (unsigned int)time_getint(self, TIME_REPR_YDAY) + 1;
/*print_number_3_spc:*/
				printf("%3u", number);
				break;

			case 'm':
				number = (unsigned int)time_getint(self, TIME_REPR_MON) + 1;
				goto print_number_2;

			case 'M':
				number = (unsigned int)time_getint(self, TIME_REPR_MIN);
				goto print_number_2;

			case 'p':
				text = am_pm[(unsigned int)time_getint(self, TIME_REPR_HOR) / 12];
				goto print_text;

			case 'r':
				printf("%0.2u:%0.2u:%0.2u %s",
				       (unsigned int)time_getint(self, TIME_REPR_HOR) % 12,
				       (unsigned int)time_getint(self, TIME_REPR_MIN),
				       (unsigned int)time_getint(self, TIME_REPR_SEC),
				       am_pm_lower[(unsigned int)time_getint(self, TIME_REPR_HOR) / 12]);
				break;

			case 'R':
				printf("%0.2u:%0.2u",
				       (unsigned int)time_getint(self, TIME_REPR_HOR),
				       (unsigned int)time_getint(self, TIME_REPR_MIN));
				break;

			case 'S':
				number = (unsigned int)time_getint(self, TIME_REPR_SEC);
				goto print_number_2;

			case 'T':
				printf("%0.2u:%0.2u:%0.2u",
				       (unsigned int)time_getint(self, TIME_REPR_HOR),
				       (unsigned int)time_getint(self, TIME_REPR_MIN),
				       (unsigned int)time_getint(self, TIME_REPR_SEC));
				break;

			case 'u':
				number = 1 + (((unsigned int)time_getint(self, TIME_REPR_WDAY) + 6) % 7);
				goto print_number_2;

			case 'w':
				number = (unsigned int)time_getint(self, TIME_REPR_WDAY);
				goto print_number_2;

			case 'y':
				number = (unsigned int)time_getint(self, TIME_REPR_YER) % 100;
				goto print_number_2;

			case 'Y':
				printf(DUTIME_HALF_PRINTF,
				       (dtime_half_t)time_getint(self, TIME_REPR_YER));
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
				dtime_t attribval;
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
					    *mode_begin == 'S' || *mode_begin == ' ')
						repr_mode = *mode_begin++;
					else {
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
				if unlikely(attribute_id == TIME_REPR_NONE) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Unknown/Invalid time attribute: %$q",
					                (size_t)(tag_end - tag_begin), tag_begin);
					goto err_m1;
				}
				attribval = time_getint(self, attribute_id);
				switch (repr_mode) {

				case 's':
				case 'S': {
					char const *repr_value;
					if (attribute_id == TIME_REPR_MON) {
						ASSERT(attribval < 12);
						repr_value = (repr_mode == 'S'
						              ? GETSTRING_MONTH_FULL(attribval)
						              : GETSTRING_MONTH_ABBR(attribval));
					} else if likely(attribute_id == TIME_REPR_WDAY) {
						ASSERT(attribval < 7);
						repr_value = (repr_mode == 'S'
						              ? GETSTRING_WDAY_FULL(attribval)
						              : GETSTRING_WDAY_ABBR(attribval));
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
					PRIVATE char const _suffix_values[] = "st"
					                                      "nd"
					                                      "rd"
					                                      "th";
#if 0
					struct DeeStringWriterFormatSpec fmt_spec = DeeStringWriterFormatSpec_INIT_BASIC(10);
					if (width) {
						fmt_spec.has_width = 1;
						fmt_spec.width     = (unsigned int)width;
						if (repr_mode != ' ')
							fmt_spec.pad_zero = 1;
					}
					if (ascii_printer_SpecWriteUInt64(printer, attribval, &fmt_spec) < 0)
						goto err;
#else
					printf("%I64u", (uint64_t)attribval);
#endif
					if (repr_mode == 'n') {
						size_t suffix_offset = (attribval >= 3 ? (size_t)3 : (size_t)attribval) * 2;
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



/* Returns the integer representation of the given time, or
 * throw an Error.TypeError if the time object hasn't been
 * assigned a specific representation. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_intrepr(DeeTimeObject *__restrict self,
             dtime_t *__restrict presult) {
	if (self->t_repr != TIME_REPR_NONE) {
		*presult = time_getint(self, self->t_repr);
		return 0;
	}
	DeeError_Throwf(&DeeError_TypeError,
	                "No representation has been set for the time object");
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_int(DeeTimeObject *__restrict self) {
#ifdef HAVE_128BIT_TIME
	return DeeInt_NewS128(DeeTime_Get(self));
#else /* HAVE_128BIT_TIME */
	return DeeInt_NewS64(DeeTime_Get(self));
#endif /* !HAVE_128BIT_TIME */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_int64(DeeTimeObject *__restrict self,
           int64_t *__restrict presult) {
#ifdef HAVE_128BIT_TIME
	dtime_t value = DeeTime_Get(self);
	/* Check for overflow by probing the upper 64 bits. */
	if unlikely(!DSINT128_IS64(value)) {
		DeeError_Throwf(&DeeError_IntegerOverflow,
		                "%s integer overflow after 64 bits in %k",
		                value < 0 ? "negative" : "positive", self);
		goto err;
	}
	*presult = (int64_t)value;
	return INT_SIGNED;
err:
	return -1;
#else /* HAVE_128BIT_TIME */
	*presult = DeeTime_Get(self);
	return INT_SIGNED;
#endif /* !HAVE_128BIT_TIME */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_int32(DeeTimeObject *__restrict self,
           int32_t *__restrict presult) {
	dtime_t value = DeeTime_Get(self);
	if unlikely(value < INT32_MIN || value > INT32_MAX) {
		DeeError_Throwf(&DeeError_IntegerOverflow,
		                "%s integer overflow after 32 bits in %k",
		                value < 0 ? "negative" : "positive", self);
		goto err;
	}
	*presult = (int32_t)value;
	return INT_SIGNED;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
time_doformat_string(DeeTimeObject *__restrict self,
                     char const *__restrict format) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (time_format(&printer, format, self) < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_doformat(DeeTimeObject *self, size_t argc, DeeObject *const *argv) {
	char const *format;
	if (DeeArg_Unpack(argc, argv, "s:format", &format))
		return NULL;
	return time_doformat_string(self, format);
}

/* From http://stackoverflow.com/questions/5590429/calculating-daylight-savings-time-from-only-date */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
determine_isdst(DeeTimeObject *__restrict self) {
	int previousSunday;
	unsigned int month;
	unsigned int mday, wday;
	month = (unsigned int)time_getint(self, TIME_REPR_MON);
	//January, February, and December are out.
	if (month < 3 || month > 11) {
		return (false);
	}
	//April to October are in
	if (month > 3 && month < 11) {
		return (true);
	}
	mday           = (unsigned int)time_getint(self, TIME_REPR_MDAY);
	wday           = (unsigned int)time_getint(self, TIME_REPR_WDAY);
	previousSunday = mday - wday;
	//In march, we are DST if our previous Sunday was on or after the 8th.
	if (month == 3) {
		return (previousSunday >= 8);
	}
	//In November we must be before the first Sunday to be dst.
	//That means the previous Sunday must be before the 1st.
	return (previousSunday <= 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_isdst(DeeTimeObject *__restrict self) {
	return_bool(determine_isdst(self));
}

PRIVATE struct type_method tpconst time_methods[] = {
	{ "as", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&time_as,
	  DOC("(string repr)->time\n"
	      "@throw ValueError The given @repr is not a known string representation\n"
	      "Re-return @this time object using a given representation\n"
	      "Views of all available representation can also be generated "
	      "using properties of the same names, meaning that this function "
	      "could also be implemented using ?Eoperators:getattr") },
	{ "format", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&time_doformat,
	  DOC("(format:?Dstring)->?Dstring\n"
	      "Format @this time object using a given strftime-style @format string") },
	{ "__format__", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&time_doformat,
	  DOC("(format:?Dstring)->?Dstring\n"
	      "Internal alias for ?#format") },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_intval(DeeTimeObject *__restrict self) {
	dtime_t result;
	if (time_intrepr(self, &result))
		return NULL;
#ifdef HAVE_128BIT_TIME
	return DeeInt_NewS128(result);
#else /* HAVE_128BIT_TIME */
	return DeeInt_NewS64(result);
#endif /* !HAVE_128BIT_TIME */
}

#ifdef HAVE_128BIT_TIME
#define object_as_time      DeeObject_AsInt128
#define object_as_time_half DeeObject_AsInt64
#else /* HAVE_128BIT_TIME */
#define object_as_time      DeeObject_AsInt64
#define object_as_time_half DeeObject_AsInt32
#endif /* !HAVE_128BIT_TIME */

#define DEFINE_TIME_AS(name, repr)                                         \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                     \
	time_getas_##name(DeeObject *__restrict self) {                        \
		return (DREF DeeObject *)time_asrepr((DeeTimeObject *)self, repr); \
	}                                                                      \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL                              \
	time_setas_##name(DeeObject *self, DREF DeeObject *value) {            \
		dtime_t tval;                                                      \
		if (object_as_time(value, &tval))                                  \
			return -1;                                                     \
		return time_setint((DeeTimeObject *)self, repr, tval);             \
	}                                                                      \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                                 \
	time_delas_##name(DeeObject *__restrict self) {                        \
		return time_setint((DeeTimeObject *)self, repr, 0);                \
	}
DEFINE_TIME_AS(mic, TIME_REPR_MIC)
DEFINE_TIME_AS(mil, TIME_REPR_MIL)
DEFINE_TIME_AS(sec, TIME_REPR_SEC)
#undef min
DEFINE_TIME_AS(min, TIME_REPR_MIN)
DEFINE_TIME_AS(hor, TIME_REPR_HOR)
DEFINE_TIME_AS(wday, TIME_REPR_WDAY)
DEFINE_TIME_AS(mwek, TIME_REPR_MWEK)
DEFINE_TIME_AS(mon, TIME_REPR_MON)
DEFINE_TIME_AS(yer, TIME_REPR_YER)
DEFINE_TIME_AS(dec, TIME_REPR_DEC)
DEFINE_TIME_AS(cen, TIME_REPR_CEN)
DEFINE_TIME_AS(mll, TIME_REPR_MLL)
DEFINE_TIME_AS(mday, TIME_REPR_MDAY)
DEFINE_TIME_AS(yday, TIME_REPR_YDAY)
DEFINE_TIME_AS(ywek, TIME_REPR_YWEK)
DEFINE_TIME_AS(mics, TIME_REPR_MICS)
DEFINE_TIME_AS(mils, TIME_REPR_MILS)
DEFINE_TIME_AS(secs, TIME_REPR_SECS)
DEFINE_TIME_AS(mins, TIME_REPR_MINS)
DEFINE_TIME_AS(hors, TIME_REPR_HORS)
DEFINE_TIME_AS(days, TIME_REPR_DAYS)
DEFINE_TIME_AS(weks, TIME_REPR_WEKS)
DEFINE_TIME_AS(mons, TIME_REPR_MONS)
#undef DEFINE_TIME_AS

PRIVATE char const timestr_mic[]          = "mic";
PRIVATE char const timestr_mil[]          = "mil";
PRIVATE char const timestr_sec[]          = "sec";
PRIVATE char const timestr_min[]          = "min";
PRIVATE char const timestr_hor[]          = "hor";
PRIVATE char const timestr_day[]          = "day";
PRIVATE char const timestr_wday[]         = "wday";
PRIVATE char const timestr_mwek[]         = "mwek";
PRIVATE char const timestr_mon[]          = "mon";
PRIVATE char const timestr_yer[]          = "yer";
PRIVATE char const timestr_dec[]          = "dec";
PRIVATE char const timestr_cen[]          = "cen";
PRIVATE char const timestr_mll[]          = "mll";
PRIVATE char const timestr_mday[]         = "mday";
PRIVATE char const timestr_yday[]         = "yday";
PRIVATE char const timestr_ywek[]         = "ywek";
PRIVATE char const timestr_mics[]         = "mics";
PRIVATE char const timestr_mils[]         = "mils";
PRIVATE char const timestr_secs[]         = "secs";
PRIVATE char const timestr_mins[]         = "mins";
PRIVATE char const timestr_hors[]         = "hors";
PRIVATE char const timestr_days[]         = "days";
PRIVATE char const timestr_weks[]         = "weks";
PRIVATE char const timestr_mons[]         = "mons";
PRIVATE char const timestr_yers[]         = "yers";
PRIVATE char const timestr_decs[]         = "decs";
PRIVATE char const timestr_cens[]         = "cens";
PRIVATE char const timestr_mlls[]         = "mlls";
PRIVATE char const timestr_microsecond[]  = "microsecond";
PRIVATE char const timestr_millisecond[]  = "millisecond";
PRIVATE char const timestr_second[]       = "second";
PRIVATE char const timestr_minute[]       = "minute";
PRIVATE char const timestr_hour[]         = "hour";
PRIVATE char const timestr_weekday[]      = "weekday";
PRIVATE char const timestr_monthweek[]    = "monthweek";
PRIVATE char const timestr_month[]        = "month";
PRIVATE char const timestr_year[]         = "year";
PRIVATE char const timestr_decade[]       = "decade";
PRIVATE char const timestr_century[]      = "century";
PRIVATE char const timestr_millennium[]   = "millennium";
PRIVATE char const timestr_monthday[]     = "monthday";
PRIVATE char const timestr_yearday[]      = "yearday";
PRIVATE char const timestr_yearweek[]     = "yearweek";
PRIVATE char const timestr_microseconds[] = "microseconds";
PRIVATE char const timestr_milliseconds[] = "milliseconds";
PRIVATE char const timestr_seconds[]      = "seconds";
PRIVATE char const timestr_minutes[]      = "minutes";
PRIVATE char const timestr_hours[]        = "hours";
PRIVATE char const timestr_weeks[]        = "weeks";
PRIVATE char const timestr_months[]       = "months";
PRIVATE char const timestr_years[]        = "years";
PRIVATE char const timestr_decades[]      = "decades";
PRIVATE char const timestr_centuries[]    = "centuries";
PRIVATE char const timestr_millenia[]     = "millenia";

PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_timepart_get(DeeTimeObject *__restrict self) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_kind = self->t_kind;
	result->t_time = self->t_time;
	if (self->t_kind == TIME_MICROSECONDS) {
		result->t_time %= MICROSECONDS_PER_DAY;
	} else {
		result->t_months = 0;
	}
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_timepart_set(DeeTimeObject *self,
                  DeeObject *value) {
	dtime_t tval = DeeTime_Get(self);
	dtime_half_t addend;
	if (object_as_time_half(value, &addend))
		goto err;
	if unlikely((dutime_half_t)addend > MICROSECONDS_PER_DAY) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Time-value %k is greater than one day",
		                value);
		goto err;
	}
	self->t_type = TIME_MICROSECONDS;
	self->t_time = tval + addend;
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
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	result->t_kind = self->t_kind;
	result->t_time = self->t_time;
	if (self->t_kind == TIME_MICROSECONDS) {
		result->t_time = (result->t_time / MICROSECONDS_PER_DAY) * MICROSECONDS_PER_DAY;
	}
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_datepart_set(DeeTimeObject *self,
                  DeeObject *value) {
	dtime_t tval = DeeTime_Get(self);
	dtime_t addend;
	if (object_as_time(value, &addend))
		goto err;
	if unlikely((addend % MICROSECONDS_PER_DAY) != 0) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Date-value %k contains a time-part",
		                value);
		goto err;
	}
	self->t_type = TIME_MICROSECONDS;
	self->t_time = tval + addend;
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
	dtime_t result;
	result = (DeeTime_Get(self) / MICROSECONDS_PER_SECOND);
	result -= time_yer2day(1970) * (MICROSECONDS_PER_DAY / MICROSECONDS_PER_SECOND);
#ifdef HAVE_128BIT_TIME
	return DeeInt_NewS128(result);
#else /* HAVE_128BIT_TIME */
	return DeeInt_NewS64(result);
#endif /* !HAVE_128BIT_TIME */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_set_time_t(DeeTimeObject *self,
                DeeObject *value) {
	dtime_t tval;
	if (object_as_time(value, &tval))
		return -1;
	self->t_time = ((time_yer2day(1970) * MICROSECONDS_PER_DAY) +
	                (tval * MICROSECONDS_PER_SECOND));
	self->t_type = TIME_MICROSECONDS;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
time_del_time_t(DeeTimeObject *__restrict self) {
	return time_set_time_t(self, Dee_None);
}

DOC_DEF(docof_timeas, "->?GTime\n"
                      "@throw ValueError The given value cannot be negative, or is too large to fit the view\n"
                      "Get/Set this specific representation of time, potentially clamped to the limits of the next-greater view\n"
                      "Note that these representations are always zero-based, meaning "
                      "that January is $0, the 2nd day of the month is $1, etc.");
PRIVATE struct type_getset tpconst time_getsets[] = {
	{ "intval", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_intval, NULL, NULL,
	  DOC("->?Dint\n"
	      "Returns the integer value of the selected time representation\n"
	      "This differs from the regular int-operator which always return the time in microseconds:\n"
	      "${"
	      "import * from time;\n"
	      "local x = days(2);\n"
	      "print x;        // 2 days\n"
	      "print int(x);   // 2*24*60*60*1000*1000\n"
	      "print x.intval; // 2"
	      "}") },
	{ "isdst", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_isdst, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if DaylightSavingsTime is in active at @this time\n"
	      "Note that this implementation does not perform any special "
	      "handling no matter if daylight savings is active or not") },
	{ "timepart",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_timepart_get,
	  (int (DCALL *)(DeeObject *__restrict))&time_timepart_del,
	  (int (DCALL *)(DeeObject *, DeeObject *))&time_timepart_set,
	  DOC("->?GTime\n"
	      "Read/write the time portion of @this time object, that is everything below the "
	      "day-threshold, including ?#hour, ?#minute, ?#second, ?#millisecond and ?#microsecond\n"
	      "When setting, the passed objected is interpreted as an integer describing the "
	      "number of microsecond since the day began") },
	{ "datepart",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_datepart_get,
	  (int (DCALL *)(DeeObject *__restrict))&time_datepart_del,
	  (int (DCALL *)(DeeObject *, DeeObject *))&time_datepart_set,
	  DOC("->?GTime\n"
	      "@throw ValueError Attempted to assign a time value with a non-zero ?#timepart\n"
	      "Read/write the date portion of @this time object, that is everything "
	      "above the day-threshold, including ?#mday, ?#month and ?#year\n"
	      "When setting, the passed objected is interpreted as an integer "
	      "describing the number of microsecond since 1.1.0000") },
	{ timestr_mic, &time_getas_mic, &time_delas_mic, &time_setas_mic, DOC_GET(docof_timeas) },
	{ timestr_mil, &time_getas_mil, &time_delas_mil, &time_setas_mil, DOC_GET(docof_timeas) },
	{ timestr_sec, &time_getas_sec, &time_delas_sec, &time_setas_sec, DOC_GET(docof_timeas) },
	{ timestr_min, &time_getas_min, &time_delas_min, &time_setas_min, DOC_GET(docof_timeas) },
	{ timestr_hor, &time_getas_hor, &time_delas_hor, &time_setas_hor, DOC_GET(docof_timeas) },
	{ timestr_wday, &time_getas_wday, &time_delas_wday, &time_setas_wday, DOC_GET(docof_timeas) },
	{ timestr_mwek, &time_getas_mwek, &time_delas_mwek, &time_setas_mwek, DOC_GET(docof_timeas) },
	{ timestr_mon, &time_getas_mon, &time_delas_mon, &time_setas_mon, DOC_GET(docof_timeas) },
	{ timestr_yer, &time_getas_yer, &time_delas_yer, &time_setas_yer, DOC_GET(docof_timeas) },
	{ timestr_dec, &time_getas_dec, &time_delas_dec, &time_setas_dec, DOC_GET(docof_timeas) },
	{ timestr_cen, &time_getas_cen, &time_delas_cen, &time_setas_cen, DOC_GET(docof_timeas) },
	{ timestr_mll, &time_getas_mll, &time_delas_mll, &time_setas_mll, DOC_GET(docof_timeas) },
	{ timestr_mday, &time_getas_mday, &time_delas_mday, &time_setas_mday, DOC_GET(docof_timeas) },
	{ timestr_yday, &time_getas_yday, &time_delas_yday, &time_setas_yday, DOC_GET(docof_timeas) },
	{ timestr_ywek, &time_getas_ywek, &time_delas_ywek, &time_setas_ywek, DOC_GET(docof_timeas) },
	{ timestr_mics, &time_getas_mics, &time_delas_mics, &time_setas_mics, DOC_GET(docof_timeas) },
	{ timestr_mils, &time_getas_mils, &time_delas_mils, &time_setas_mils, DOC_GET(docof_timeas) },
	{ timestr_secs, &time_getas_secs, &time_delas_secs, &time_setas_secs, DOC_GET(docof_timeas) },
	{ timestr_mins, &time_getas_mins, &time_delas_mins, &time_setas_mins, DOC_GET(docof_timeas) },
	{ timestr_hors, &time_getas_hors, &time_delas_hors, &time_setas_hors, DOC_GET(docof_timeas) },
	{ timestr_days, &time_getas_days, &time_delas_days, &time_setas_days, DOC_GET(docof_timeas) },
	{ timestr_weks, &time_getas_weks, &time_delas_weks, &time_setas_weks, DOC_GET(docof_timeas) },
	{ timestr_mons, &time_getas_mons, &time_delas_mons, &time_setas_mons, DOC_GET(docof_timeas) },
	{ timestr_yers, &time_getas_yer, &time_delas_yer, &time_setas_yer, DOC_GET(docof_timeas) },
	{ timestr_decs, &time_getas_dec, &time_delas_dec, &time_setas_dec, DOC_GET(docof_timeas) },
	{ timestr_cens, &time_getas_cen, &time_delas_cen, &time_setas_cen, DOC_GET(docof_timeas) },
	{ timestr_mlls, &time_getas_mll, &time_delas_mll, &time_setas_mll, DOC_GET(docof_timeas) },
	{ timestr_microsecond, &time_getas_mic, &time_delas_mic, &time_setas_mic, DOC_GET(docof_timeas) },
	{ timestr_millisecond, &time_getas_mil, &time_delas_mil, &time_setas_mil, DOC_GET(docof_timeas) },
	{ timestr_second, &time_getas_sec, &time_delas_sec, &time_setas_sec, DOC_GET(docof_timeas) },
	{ timestr_minute, &time_getas_min, &time_delas_min, &time_setas_min, DOC_GET(docof_timeas) },
	{ timestr_hour, &time_getas_hor, &time_delas_hor, &time_setas_hor, DOC_GET(docof_timeas) },
	{ timestr_weekday, &time_getas_wday, &time_delas_wday, &time_setas_wday, DOC_GET(docof_timeas) },
	{ timestr_monthweek, &time_getas_mwek, &time_delas_mwek, &time_setas_mwek, DOC_GET(docof_timeas) },
	{ timestr_month, &time_getas_mon, &time_delas_mon, &time_setas_mon, DOC_GET(docof_timeas) },
	{ timestr_year, &time_getas_yer, &time_delas_yer, &time_setas_yer, DOC_GET(docof_timeas) },
	{ timestr_decade, &time_getas_dec, &time_delas_dec, &time_setas_dec, DOC_GET(docof_timeas) },
	{ timestr_century, &time_getas_cen, &time_delas_cen, &time_setas_cen, DOC_GET(docof_timeas) },
	{ timestr_millennium, &time_getas_mll, &time_delas_mll, &time_setas_mll, DOC_GET(docof_timeas) },
	{ timestr_monthday, &time_getas_mday, &time_delas_mday, &time_setas_mday, DOC_GET(docof_timeas) },
	{ timestr_yearday, &time_getas_yday, &time_delas_yday, &time_setas_yday, DOC_GET(docof_timeas) },
	{ timestr_yearweek, &time_getas_ywek, &time_delas_ywek, &time_setas_ywek, DOC_GET(docof_timeas) },
	{ timestr_microseconds, &time_getas_mics, &time_delas_mics, &time_setas_mics, DOC_GET(docof_timeas) },
	{ timestr_milliseconds, &time_getas_mils, &time_delas_mils, &time_setas_mils, DOC_GET(docof_timeas) },
	{ timestr_seconds, &time_getas_secs, &time_delas_secs, &time_setas_secs, DOC_GET(docof_timeas) },
	{ timestr_minutes, &time_getas_mins, &time_delas_mins, &time_setas_mins, DOC_GET(docof_timeas) },
	{ timestr_hours, &time_getas_hors, &time_delas_hors, &time_setas_hors, DOC_GET(docof_timeas) },
	//{ timestr_days, &time_getas_days, &time_delas_days, &time_setas_days, DOC_GET(docof_timeas) },
	{ timestr_weeks, &time_getas_weks, &time_delas_weks, &time_setas_weks, DOC_GET(docof_timeas) },
	{ timestr_months, &time_getas_mons, &time_delas_mons, &time_setas_mons, DOC_GET(docof_timeas) },
	{ timestr_years, &time_getas_yer, &time_delas_yer, &time_setas_yer, DOC_GET(docof_timeas) },
	{ timestr_decades, &time_getas_dec, &time_delas_dec, &time_setas_dec, DOC_GET(docof_timeas) },
	{ timestr_centuries, &time_getas_cen, &time_delas_cen, &time_setas_cen, DOC_GET(docof_timeas) },
	{ timestr_millenia, &time_getas_mll, &time_delas_mll, &time_setas_mll, DOC_GET(docof_timeas) },
	{ "mweek", &time_getas_mwek, &time_delas_mwek, &time_setas_mwek, DOC("->?GTime\nMiddle-way alias for ?#monthweek") },
	{ "yweek", &time_getas_ywek, &time_delas_ywek, &time_setas_ywek, DOC("->?GTime\nMiddle-way alias for ?#yearweek") },
	/* Deprecated names/functions. */
	{ "msecond", &time_getas_mil, &time_delas_mil, &time_setas_mil, DOC("->?GTime\nDeprecated alias for ?#mic / ?#millisecond") },
	{ "mseconds", &time_getas_mils, &time_delas_mils, &time_setas_mils, DOC("->?GTime\nDeprecated alias for ?#mics / ?#milliseconds") },
	{ "time",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_timepart_get,
	  (int (DCALL *)(DeeObject *__restrict))&time_timepart_del,
	  (int (DCALL *)(DeeObject *, DeeObject *))&time_timepart_set,
	  DOC("->?GTime\nDeprecated alias for ?#timepart") },
	{ "part",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_datepart_get,
	  (int (DCALL *)(DeeObject *__restrict))&time_datepart_del,
	  (int (DCALL *)(DeeObject *, DeeObject *))&time_datepart_set,
	  DOC("->?GTime\nDeprecated alias for ?#datepart") },
	{ "time_t",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_get_time_t,
	  (int (DCALL *)(DeeObject *__restrict))&time_del_time_t,
	  (int (DCALL *)(DeeObject *, DeeObject *))&time_set_time_t,
	  DOC("->?Dint\nDeprecated") },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeTimeObject *DCALL
time_neg(DeeTimeObject *__restrict self) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTime_Type);
	if (self->t_type == TIME_MONTHS) {
		result->t_months = -self->t_months;
	} else {
		result->t_time = -self->t_time;
	}
	result->t_kind = self->t_kind;
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_add(DeeTimeObject *self, DeeTimeObject *other) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_repr = self->t_repr;
	if (DeeTime_Check(other)) {
		if (self->t_type == other->t_type) {
			result->t_type = self->t_type;
			if (self->t_type == TIME_MICROSECONDS) {
				result->t_time = self->t_time + other->t_time;
			} else {
				result->t_months = self->t_months + other->t_months;
			}
		} else {
			result->t_type = TIME_MICROSECONDS;
			result->t_time = (DeeTime_Get(self) +
			                  DeeTime_Get(other));
		}
	} else {
		dtime_t other_time;
		if (object_as_time((DeeObject *)other, &other_time))
			goto err;
		result->t_type = TIME_MICROSECONDS;
		result->t_time = (DeeTime_Get(self) + other_time);
	}
	DeeObject_Init(result, &DeeTime_Type);
done:
	return result;
err:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_sub(DeeTimeObject *self, DeeTimeObject *other) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_repr = self->t_repr;
	if (DeeTime_Check(other)) {
		if (self->t_type == other->t_type) {
			result->t_type = self->t_type;
			if (self->t_type == TIME_MICROSECONDS) {
				result->t_time = self->t_time - other->t_time;
			} else {
				result->t_months = self->t_months - other->t_months;
			}
		} else {
			result->t_type = TIME_MICROSECONDS;
			result->t_time = (DeeTime_Get(self) -
			                  DeeTime_Get(other));
		}
	} else {
		dtime_t other_time;
		if (object_as_time((DeeObject *)other, &other_time))
			goto err;
		result->t_type = TIME_MICROSECONDS;
		result->t_time = (DeeTime_Get(self) - other_time);
	}
	DeeObject_Init(result, &DeeTime_Type);
done:
	return result;
err:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_mul(DeeTimeObject *self, DeeObject *other) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_kind = self->t_kind;
	if (self->t_type == TIME_MICROSECONDS) {
		if (object_as_time(other, &result->t_time))
			goto err;
		result->t_time *= self->t_time;
	} else {
		if (object_as_time_half(other, &result->t_months))
			goto err;
		result->t_months *= self->t_months;
	}
	DeeObject_Init(result, &DeeTime_Type);
done:
	return result;
err:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((1, 2)) void DCALL
err_divide_by_zero(DeeTimeObject *self, DeeObject *other) {
	DeeError_Throwf(&DeeError_DivideByZero,
	                "Divide by Zero: `%k / %k'",
	                self, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_div(DeeTimeObject *self, DeeObject *other) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_kind = self->t_kind;
	if (self->t_type == TIME_MICROSECONDS) {
		if (object_as_time(other, &result->t_time))
			goto err;
		if unlikely(!result->t_time)
			goto err_divzero;
		result->t_time = self->t_time / result->t_time;
	} else {
		if (object_as_time_half(other, &result->t_months))
			goto err;
		if unlikely(!result->t_months)
			goto err_divzero;
		result->t_months = self->t_months / result->t_months;
	}
	/* XXX: Should this return an int or float when a Time object was given,
	 *      and only return a time object when int or float was given:
	 *      minutes(1) / seconds(30) == 2
	 *      minutes(1) / 2           == seconds(30)
	 */
	DeeObject_Init(result, &DeeTime_Type);
done:
	return result;
err_divzero:
	err_divide_by_zero(self, other);
err:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTimeObject *DCALL
time_mod(DeeTimeObject *self, DeeObject *other) {
	DREF DeeTimeObject *result;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_kind = self->t_kind;
	if (self->t_type == TIME_MICROSECONDS) {
		if (object_as_time(other, &result->t_time))
			goto err;
		if unlikely(!result->t_time)
			goto err_divzero;
		result->t_time = self->t_time % result->t_time;
	} else {
		if (object_as_time_half(other, &result->t_months))
			goto err;
		if unlikely(!result->t_months)
			goto err_divzero;
		result->t_months = self->t_months % result->t_months;
	}
	/* XXX: Same as with DIV: return int under certain circumstances */
	DeeObject_Init(result, &DeeTime_Type);
done:
	return result;
err_divzero:
	err_divide_by_zero(self, other);
err:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE struct type_math time_math = {
	/* .tp_int32  = */ (int (DCALL *)(DeeObject *__restrict, int32_t *__restrict))&time_int32,
	/* .tp_int64  = */ (int (DCALL *)(DeeObject *__restrict, int64_t *__restrict))&time_int64,
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

#ifdef HAVE_128BIT_TIME
PRIVATE DEFINE_INT15(time_class_bits, 128);
#else /* HAVE_128BIT_TIME */
PRIVATE DEFINE_INT15(time_class_bits, 64);
#endif /* !HAVE_128BIT_TIME */

PRIVATE struct type_member tpconst time_class_members[] = {
	TYPE_MEMBER_CONST_DOC("bits", &time_class_bits,
	                      "The number of bits used by the implementation to represent a ?. object"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED DREF DeeObject *DCALL
f_libtime_now(size_t argc, DeeObject *const *argv) {
	DREF DeeTimeObject *result;
	if (DeeArg_Unpack(argc, argv, ":now"))
		goto err;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_kind = TIME_KIND(TIME_MICROSECONDS, TIME_REPR_NONE);
	/* Load the current time. */
	result->t_time = time_now();
	DeeObject_Init(result, &DeeTime_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_libtime_tick(size_t argc, DeeObject *const *argv) {
	DREF DeeTimeObject *result;
	if (DeeArg_Unpack(argc, argv, ":tick"))
		goto err;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	/* Initialize using micro-second precision.
	 * XXX: If the system can't provide that precision, we
	 *      should probably use a different representation. */
	result->t_kind = TIME_KIND(TIME_MICROSECONDS,
	                           TIME_REPR_MICS);
	/* Load the current tick. */
	result->t_time = time_tick();
	DeeObject_Init(result, &DeeTime_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


#ifdef HAVE_128BIT_TIME
#define DEFINE_INTERVAL_GENERATOR(name, new)                    \
	PRIVATE WUNUSED DREF DeeObject *DCALL                       \
	f_libtime_##name(size_t argc, DeeObject *const *argv) {     \
		DeeObject *value_ob;                                    \
		dtime_t value;                                          \
		if (DeeArg_Unpack(argc, argv, "o:" #name, &value_ob) || \
		    object_as_time(value_ob, &value))                   \
			return NULL;                                        \
		return new;                                             \
	}                                                           \
	PRIVATE DEFINE_CMETHOD(libtime_##name, &f_libtime_##name)
#define DEFINE_INTERVAL_GENERATOR_HALF(name, new)             \
	PRIVATE WUNUSED DREF DeeObject *DCALL                     \
	f_libtime_##name(size_t argc, DeeObject *const *argv) {   \
		dtime_half_t value;                                   \
		if (DeeArg_Unpack(argc, argv, "I64d:" #name, &value)) \
			return NULL;                                      \
		return new;                                           \
	}                                                         \
	PRIVATE DEFINE_CMETHOD(libtime_##name, &f_libtime_##name)
#else /* HAVE_128BIT_TIME */
#define DEFINE_INTERVAL_GENERATOR(name, new)                  \
	PRIVATE WUNUSED DREF DeeObject *DCALL                     \
	f_libtime_##name(size_t argc, DeeObject *const *argv) {   \
		dtime_t value;                                        \
		if (DeeArg_Unpack(argc, argv, "I64d:" #name, &value)) \
			return NULL;                                      \
		return new;                                           \
	}                                                         \
	PRIVATE DEFINE_CMETHOD(libtime_##name, &f_libtime_##name)
#define DEFINE_INTERVAL_GENERATOR_HALF(name, new)             \
	PRIVATE WUNUSED DREF DeeObject *DCALL                     \
	f_libtime_##name(size_t argc, DeeObject *const *argv) {   \
		dtime_half_t value;                                   \
		if (DeeArg_Unpack(argc, argv, "I32d:" #name, &value)) \
			return NULL;                                      \
		return new;                                           \
	}                                                         \
	PRIVATE DEFINE_CMETHOD(libtime_##name, &f_libtime_##name)
#endif /* !HAVE_128BIT_TIME */
DEFINE_INTERVAL_GENERATOR(microseconds, DeeTime_New(value, TIME_REPR_MICS));
DEFINE_INTERVAL_GENERATOR(milliseconds, DeeTime_New(value * MICROSECONDS_PER_MILLISECOND, TIME_REPR_MILS));
DEFINE_INTERVAL_GENERATOR(seconds, DeeTime_New(value * MICROSECONDS_PER_SECOND, TIME_REPR_SECS));
DEFINE_INTERVAL_GENERATOR(minutes, DeeTime_New(value * MICROSECONDS_PER_MINUTE, TIME_REPR_MINS));
DEFINE_INTERVAL_GENERATOR(hours, DeeTime_New(value * MICROSECONDS_PER_HOUR, TIME_REPR_HORS));
DEFINE_INTERVAL_GENERATOR(days, DeeTime_New(value * MICROSECONDS_PER_DAY, TIME_REPR_DAYS));
DEFINE_INTERVAL_GENERATOR(weeks, DeeTime_New(value * MICROSECONDS_PER_WEEK, TIME_REPR_WEKS));
DEFINE_INTERVAL_GENERATOR_HALF(months, DeeTime_NewMonths(value, TIME_REPR_MONS));
DEFINE_INTERVAL_GENERATOR_HALF(years, DeeTime_NewMonths(value * MONTHS_PER_YEAR, TIME_REPR_YER));
DEFINE_INTERVAL_GENERATOR_HALF(decades, DeeTime_NewMonths(value * MONTHS_PER_DECADE, TIME_REPR_DEC));
DEFINE_INTERVAL_GENERATOR_HALF(centuries, DeeTime_NewMonths(value * MONTHS_PER_CENTURY, TIME_REPR_CEN));
DEFINE_INTERVAL_GENERATOR_HALF(millenia, DeeTime_NewMonths(value * MONTHS_PER_MILLENIUM, TIME_REPR_MIL));
#undef DEFINE_INTERVAL_GENERATOR_HALF
#undef DEFINE_INTERVAL_GENERATOR


PRIVATE WUNUSED DREF DeeObject *DCALL
f_libtime_maketime(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeTimeObject *result;
	unsigned int hor = 0, min = 0, sec = 0;
	unsigned int mil = 0, mic = 0;
	PRIVATE struct keyword kwlist[] = {
		KS(timestr_hour),
		KS(timestr_minute),
		KS(timestr_second),
		KS(timestr_millisecond),
		KS(timestr_microsecond),
		KEND
	};
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|uuuuu:maketime", &hor, &min, &sec, &mil, &mic))
		goto err;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_kind = TIME_KIND(TIME_MICROSECONDS, TIME_REPR_NONE);
	result->t_time = 0;
	if (time_setint(result, TIME_REPR_HOR, hor) ||
	    time_setint(result, TIME_REPR_MIN, min) ||
	    time_setint(result, TIME_REPR_SEC, sec) ||
	    time_setint(result, TIME_REPR_MIL, mil) ||
	    time_setint(result, TIME_REPR_MIC, mic))
		goto err_r;
	DeeObject_Init(result, &DeeTime_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_libtime_makedate(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeTimeObject *result;
	dtime_half_t yer = 0;
	unsigned int mon = 0, day = 0;
	PRIVATE struct keyword kwlist[] = {
		KS(timestr_year),
		KS(timestr_month),
		KS(timestr_day),
		KEND
	};
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "|" DUTIME_HALF_UNPACK "uu:makedate", &yer, &mon, &day))
		goto err;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_kind = TIME_KIND(TIME_MICROSECONDS, TIME_REPR_NONE);
	result->t_time = 0;
	if (time_setint(result, TIME_REPR_YER, yer) ||
	    time_setint(result, TIME_REPR_MON, mon) ||
	    time_setint(result, TIME_REPR_MDAY, day))
		goto err_r;
	DeeObject_Init(result, &DeeTime_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_libtime_makeanon(size_t argc, DeeObject *const *argv) {
	dtime_t value;
	DREF DeeTimeObject *result;
#ifdef HAVE_128BIT_TIME
	DeeObject *temp;
	if (DeeArg_Unpack(argc, argv, "o:makeanon", &temp))
		goto err;
	if (object_as_time(temp, &value))
		goto err;
#else /* HAVE_128BIT_TIME */
	if (DeeArg_Unpack(argc, argv, "I64u:makeanon", &value))
		goto err;
#endif /* !HAVE_128BIT_TIME */
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto done;
	result->t_kind = TIME_KIND(TIME_MICROSECONDS, TIME_REPR_NONE);
	result->t_time = value;
	DeeObject_Init(result, &DeeTime_Type);
done:
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_now(DeeObject *UNUSED(self),
               size_t argc, DeeObject *const *argv) {
	return f_libtime_now(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_tick(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	return f_libtime_tick(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_milliseconds(DeeObject *UNUSED(self),
                        size_t argc, DeeObject *const *argv) {
	return f_libtime_milliseconds(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_seconds(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	return f_libtime_seconds(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_minutes(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	return f_libtime_minutes(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_hours(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	return f_libtime_hours(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_days(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	return f_libtime_days(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_weeks(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	return f_libtime_weeks(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_months(DeeObject *UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	return f_libtime_months(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_years(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	return f_libtime_years(argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_maketime(DeeObject *UNUSED(self),
                    size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return f_libtime_maketime(argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_makedate(DeeObject *UNUSED(self),
                    size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return f_libtime_makedate(argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_from_time_t(DeeObject *UNUSED(self),
                       size_t argc, DeeObject *const *argv) {
	time_t value;
	DREF DeeTimeObject *result;
	if (DeeArg_Unpack(argc, argv, sizeof(time_t) == 4 ? "I32u:from_time_t" : "I64u:from_time_t",
	                  &value))
		goto err;
	result = DeeObject_MALLOC(DeeTimeObject);
	if unlikely(!result)
		goto err;
	result->t_kind = TIME_KIND(TIME_MICROSECONDS, TIME_REPR_NONE);
	result->t_time = time_yer2day(1970) * MICROSECONDS_PER_DAY + (dtime_t)value * MICROSECONDS_PER_SECOND;
	DeeObject_Init(result, &DeeTime_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_class_freq(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":freq"))
		return NULL;
	return DeeInt_NewInt(MICROSECONDS_PER_SECOND);
}

PRIVATE struct type_method tpconst time_class_methods[] = {
	/* For backwards compatibility with the old deemon (which
	 * did everything as part of the `time' builtin type) */
	{ "now", &time_class_now,
	  DOC("->?.\n"
	      "Deprecated. Use ?Gnow instead") },
	{ "tick", &time_class_tick,
	  DOC("->?.\n"
	      "Deprecated. Use ?Gtick instead") },
	{ "freq", &time_class_freq,
	  DOC("->?Dint\n"
	      "Deprecated. Always returns $1000000") },
	{ "time", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&time_class_maketime,
	  DOC("(hour=!0,minute=!0,second=!0,millisecond=!0,microsecond=!0)->?.\n"
	      "Deprecated. Use ?Gmaketime instead"),
	  TYPE_METHOD_FKWDS },
	{ "date", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&time_class_makedate,
	  DOC("(year=!0,month=!0,day=!0)->?.\n"
	      "Deprecated. Use ?Gmakedate instead"),
	  TYPE_METHOD_FKWDS },
	{ "from_time_t", &time_class_from_time_t,
	  DOC("(time_t_value:?Dint)->?.\n"
	      "Deprecated") },
#define ADD_INTERVAL_CALLBACK(name, func) \
	{ name, &time_class_##func,           \
	  DOC("(value:?Dint)->?.\n"           \
		  "Deprecated. Use ?G" #func " instead") }
	ADD_INTERVAL_CALLBACK("mseconds", milliseconds),
	ADD_INTERVAL_CALLBACK(timestr_seconds, seconds),
	ADD_INTERVAL_CALLBACK(timestr_minutes, minutes),
	ADD_INTERVAL_CALLBACK(timestr_hours, hours),
	ADD_INTERVAL_CALLBACK(timestr_days, days),
	ADD_INTERVAL_CALLBACK(timestr_weeks, weeks),
	ADD_INTERVAL_CALLBACK(timestr_months, months),
	ADD_INTERVAL_CALLBACK(timestr_years, years),
#undef ADD_INTERVAL_CALLBACK
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
time_bool(DeeTimeObject *__restrict self) {
	return self->t_time != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_copy(DeeTimeObject *__restrict self,
          DeeTimeObject *__restrict other) {
	self->t_kind = other->t_kind;
	self->t_time = other->t_time;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
time_assign(DeeTimeObject *self, DeeTimeObject *other) {
	if (DeeTime_Check(other)) {
		self->t_kind = other->t_kind;
		self->t_time = other->t_time;
	} else {
		if (object_as_time((DeeObject *)other, &self->t_time))
			return -1;
		self->t_kind = TIME_KIND(TIME_MICROSECONDS, TIME_REPR_NONE);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
time_init_kw(DeeTimeObject *__restrict self,
             size_t argc, DeeObject *const *argv,
             DeeObject *kw) {
	dtime_half_t yer = 0;
	unsigned int mon = 0, day = 0, hor = 0;
	unsigned int min = 0, sec = 0, mil = 0, mic = 0;
	PRIVATE struct keyword kwlist[] = {
		KS(timestr_year),
		KS(timestr_month),
		KS(timestr_day),
		KS(timestr_hour),
		KS(timestr_minute),
		KS(timestr_second),
		KS(timestr_millisecond),
		KS(timestr_microsecond),
		KEND
	};
	self->t_kind = TIME_KIND(TIME_MICROSECONDS, TIME_REPR_NONE);
	self->t_time = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "|" DUTIME_HALF_UNPACK "uuuuuuu:time",
	                    &yer, &mon, &day, &hor,
	                    &min, &sec, &mil, &mic))
		goto err;
	if (yer) {
		if unlikely(time_setint(self, TIME_REPR_YER, yer))
			goto err;
	}
	if (mon) {
		if unlikely(time_setint(self, TIME_REPR_MON, mon))
			goto err;
	}
	if (day) {
		if unlikely(time_setint(self, TIME_REPR_MDAY, day))
			goto err;
	}
	if (hor) {
		if unlikely(time_setint(self, TIME_REPR_HOR, hor))
			goto err;
	}
	if (min) {
		if unlikely(time_setint(self, TIME_REPR_MIN, min))
			goto err;
	}
	if (sec) {
		if unlikely(time_setint(self, TIME_REPR_SEC, sec))
			goto err;
	}
	if (mil) {
		if unlikely(time_setint(self, TIME_REPR_MIL, mil))
			goto err;
	}
	if (mic) {
		if unlikely(time_setint(self, TIME_REPR_MIC, mic))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_str(DeeTimeObject *__restrict self) {
	DREF DeeObject *result;
	dtime_t tmval;
	if (self->t_repr == TIME_REPR_NONE)
		return time_doformat_string(self, "%A, the %[n:mday] of %B %Y, %H:%M:%S");
	tmval = time_getint(self, self->t_repr);
	if (self->t_repr & TIME_REPR_PLURAL) {
		char const *reprname;
		reprname = get_repr_name(tmval == 1
		                         ? (uint8_t)(self->t_repr & ~(TIME_REPR_PLURAL))
		                         : (uint8_t)(self->t_repr | TIME_REPR_PLURAL));
		result = DeeString_Newf("%I64d %s", tmval, reprname);
	} else {
		char const *reprname;
		reprname = get_repr_name(self->t_repr);
		result = DeeString_Newf("%s %I64d", reprname, tmval);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
time_repr(DeeTimeObject *__restrict self) {
	return DeeString_Newf("time(" DUTIME_HALF_PRINTF ", %u, %u, %u, %u, %u, %u, %u)%s%s",
	                      (dtime_half_t)time_getint(self, TIME_REPR_YER),
	                      (unsigned int)time_getint(self, TIME_REPR_MON),
	                      (unsigned int)time_getint(self, TIME_REPR_MDAY),
	                      (unsigned int)time_getint(self, TIME_REPR_HOR),
	                      (unsigned int)time_getint(self, TIME_REPR_MIN),
	                      (unsigned int)time_getint(self, TIME_REPR_SEC),
	                      (unsigned int)time_getint(self, TIME_REPR_MIL),
	                      (unsigned int)time_getint(self, TIME_REPR_MIC),
	                      self->t_repr != TIME_REPR_NONE ? "." : "",
	                      self->t_repr != TIME_REPR_NONE ? get_repr_name(self->t_repr) : "");
}


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
time_hash(DeeTimeObject *__restrict self) {
#if SIZEOF_DTIME_T == __SIZEOF_POINTER__
	return (dhash_t)(dutime_t)DeeTime_Get(self);
#else /* SIZEOF_DTIME_T == __SIZEOF_POINTER__ */
	dutime_t val   = (dutime_t)DeeTime_Get(self);
	dhash_t result = (dhash_t)val;
	result ^= (dhash_t)(val >> 1 * (__SIZEOF_POINTER__ * 8));
#if SIZEOF_DTIME_T > 2 * __SIZEOF_POINTER__
	result ^= (dhash_t)(val >> 2 * (__SIZEOF_POINTER__ * 8));
#if SIZEOF_DTIME_T > 3 * __SIZEOF_POINTER__
	result ^= (dhash_t)(val >> 3 * (__SIZEOF_POINTER__ * 8));
#if SIZEOF_DTIME_T > 4 * __SIZEOF_POINTER__
	result ^= (dhash_t)(val >> 4 * (__SIZEOF_POINTER__ * 8));
#if SIZEOF_DTIME_T > 5 * __SIZEOF_POINTER__
#error FIXME (Continue the chain...)
#endif /* SIZEOF_DTIME_T > 5 * __SIZEOF_POINTER__ */
#endif /* SIZEOF_DTIME_T > 4 * __SIZEOF_POINTER__ */
#endif /* SIZEOF_DTIME_T > 3 * __SIZEOF_POINTER__ */
#endif /* SIZEOF_DTIME_T > 2 * __SIZEOF_POINTER__ */
	return result;
#endif /* SIZEOF_DTIME_T != __SIZEOF_POINTER__ */
}

#define DEFINE_TIME_CMP(name, op)                         \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(DeeTimeObject *self, DeeObject *other) {         \
		dtime_t val;                                      \
		if (object_as_time(other, &val))                  \
			goto err;                                     \
		return_bool(DeeTime_Get(self) op val);            \
	err:                                                  \
		return NULL;                                      \
	}
DEFINE_TIME_CMP(time_eq, ==)
DEFINE_TIME_CMP(time_ne, !=)
DEFINE_TIME_CMP(time_lo, <)
DEFINE_TIME_CMP(time_le, <=)
DEFINE_TIME_CMP(time_gr, >)
DEFINE_TIME_CMP(time_ge, >=)
#undef DEFINE_TIME_CMP

PRIVATE struct type_cmp time_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict self))&time_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&time_ge
};


INTERN DeeTypeObject DeeTime_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Time",
	/* .tp_doc      = */ DOC("(year=!0,month=!0,day=!0,hour=!0,minute=!0,"
	                          "second=!0,millisecond=!0,microsecond=!0)\n"
	                          "Construct a new time object from the given arguments.\n"

	                          "\n"
	                          "str->\n"
	                          "Returns value of @this time object when it was constructed to "
	                          "represent an explicit view (such as through use of ?Gdays, "
	                          "or through a sub-view such as ?#days), or return the time "
	                          "represented in a human-readable fashion\n"

	                          "\n"
	                          "repr->\n"
	                          "Returns a string representation of the components of @this time object\n"

	                          "\n"
	                          "int->\n"
	                          "Returns the value of @this time object as an offset from 1.1.0000 in microseconds\n"
	                          "This operator allows time objects to be passed to system functions that take integer "
	                          "timeouts in microseconds, such as :Thread.sleep or ?Aaccept?Enet:socket"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ (void *)&time_copy,
				/* .tp_deep_ctor = */ (void *)&time_copy,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeTimeObject),
				/* .tp_any_ctor_kw = */ (void *)&time_init_kw,
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *__restric, DeeObject *__restrict))&time_assign,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&time_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&time_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &time_math,
	/* .tp_cmp           = */ &time_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ time_methods,
	/* .tp_getsets       = */ time_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ time_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ time_class_members
};


PRIVATE DEFINE_CMETHOD(libtime_now, &f_libtime_now);
PRIVATE DEFINE_CMETHOD(libtime_tick, &f_libtime_tick);
PRIVATE DEFINE_KWCMETHOD(libtime_maketime, &f_libtime_maketime);
PRIVATE DEFINE_KWCMETHOD(libtime_makedate, &f_libtime_makedate);
PRIVATE DEFINE_CMETHOD(libtime_makeanon, &f_libtime_makeanon);

PRIVATE struct dex_symbol symbols[] = {
	{ "Time", (DeeObject *)&DeeTime_Type },
	{ "now", (DeeObject *)&libtime_now, MODSYM_FNORMAL,
	  DOC("->?GTime\n"
	      "Returns the current time with as much precision as possible") },
	{ "tick", (DeeObject *)&libtime_tick, MODSYM_FNORMAL,
	  DOC("->?GTime\n"
	      "Returns the current tick suitable for high-precision timings.\n"
	      "The tick itself is offset from some undefined point in time, meaning that the only "
	      "meaningful use, is to subtract the return values of two calls to this function.\n"
	      "The value of the tick (in microseconds) can easily be extracted by casting/using "
	      "the return value as an integer") },
	{ "maketime", (DeeObject *)&libtime_maketime, MODSYM_FNORMAL,
	  DOC("(hour=!0,minute=!0,second=!0,millisecond=!0,microsecond=!0)->?GTime\n"
	      "Construct a new :time object using the given arguments for the "
	      "sub-day portion, while filling in the remainder as all zeroes:\n"
	      "${"
	      "import Time from time;"
	      "Time(0, 0, 0, hour, minute, second, millisecond, microsecond);"
	      "}") },
	{ "makedate", (DeeObject *)&libtime_makedate, MODSYM_FNORMAL,
	  DOC("(year=!0,month=!0,day=!0)->?GTime\n"
	      "Construct a new :time object using the given arguments for the "
	      "post-day portion, while filling in the remainder as all zeroes:\n"
	      "${"
	      "import Time from time;"
	      "Time(year, month, day, 0, 0, 0, 0, 0);"
	      "}") },
	{ "makeanon", (DeeObject *)&libtime_makeanon, MODSYM_FNORMAL,
	  DOC("(microseconds:?Dint)->?GTime\n"
	      "Construct a new anonymous (generic-representation) time object, "
	      "given the amount of @microseconds it ought to represent\n"
	      "This function is mainly used by other libraries for constructing time "
	      "objects, given their absolute point in time in @microseconds") },

	/* Export various functions for constructing time intervals.
	 * NOTE: These functions are highly useful for specifying timeouts:
	 * >> import seconds from time;
	 * >> import Thread from deemon;
	 * >> 
	 * >> begin "Begin waiting for 2 seconds";
	 * >> Thread.sleep(seconds(2));
	 * >> begin "Done waiting";
	 * >> 
	 */
#define ADD_INTERVAL_CALLBACK(name, func)                 \
	{ name, (DeeObject *)&libtime_##func, MODSYM_FNORMAL, \
	  DOC("(value:?Dint)->?GTime\n"                       \
		  "@return A time interval of @value " #func) }
	ADD_INTERVAL_CALLBACK(timestr_microseconds, microseconds),
	ADD_INTERVAL_CALLBACK(timestr_milliseconds, milliseconds),
	ADD_INTERVAL_CALLBACK(timestr_seconds, seconds),
	ADD_INTERVAL_CALLBACK(timestr_minutes, minutes),
	ADD_INTERVAL_CALLBACK(timestr_hours, hours),
	ADD_INTERVAL_CALLBACK(timestr_days, days),
	ADD_INTERVAL_CALLBACK(timestr_weeks, weeks),
	ADD_INTERVAL_CALLBACK(timestr_months, months),
	ADD_INTERVAL_CALLBACK(timestr_years, years),
	ADD_INTERVAL_CALLBACK(timestr_decades, decades),
	ADD_INTERVAL_CALLBACK(timestr_centuries, centuries),
	ADD_INTERVAL_CALLBACK(timestr_millenia, millenia),
	ADD_INTERVAL_CALLBACK(timestr_mics, microseconds),
	ADD_INTERVAL_CALLBACK(timestr_mils, milliseconds),
	ADD_INTERVAL_CALLBACK(timestr_secs, seconds),
	ADD_INTERVAL_CALLBACK(timestr_mins, minutes),
	ADD_INTERVAL_CALLBACK(timestr_hors, hours),
	ADD_INTERVAL_CALLBACK(timestr_weks, weeks),
	ADD_INTERVAL_CALLBACK(timestr_mons, months),
	ADD_INTERVAL_CALLBACK(timestr_yers, years),
	ADD_INTERVAL_CALLBACK(timestr_decs, decades),
	ADD_INTERVAL_CALLBACK(timestr_cens, centuries),
	ADD_INTERVAL_CALLBACK(timestr_mlls, millenia),
#undef ADD_INTERVAL_CALLBACK
	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END


#endif /* !GUARD_DEX_TIME_LIBTIME_C */
