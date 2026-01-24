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
#ifndef GUARD_DEX_TIME_LIBTIME_H
#define GUARD_DEX_TIME_LIBTIME_H 1

#include <deemon/api.h>

#include <deemon/object.h>

#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */
#include <hybrid/int128.h>    /* __HYBRID_INT128_INIT16N, __hybrid_int128_* */

#include <stdbool.h> /* bool */
#include <stdint.h>  /* UINTn_C, int64_t, uintN_t */

DECL_BEGIN

#undef ceildiv
#define ceildiv(x, y) (((x) + ((y) - 1)) / (y))

/* Time types */
#define TIME_TYPE_NANOSECONDS 0 /* Nanoseconds since 01-01-0000T00:00:00 (or # of delta nanoseconds) */
#define TIME_TYPE_MONTHS      1 /* Months since 01.0000 (or # of delta months) */
#define TIME_TYPE_INVALID     2 /* Invalid */

/* Time kinds */
#define TIME_KIND_TIMESTAMP 0 /* Time since 01-01-0000 */
#define TIME_KIND_DELTA     1 /* Delta time */
#define TIME_KIND_INVALID   2 /* Invalid */


/* Time representations. */
#define TIME_REPR_INVALID     0  /* Invalid ID */
#define TIME_REPR_NANOSECOND  1  /* Nanosecond of second */
#define TIME_REPR_MICROSECOND 2  /* Microsecond of second */
#define TIME_REPR_MILLISECOND 3  /* Millisecond of second */
#define TIME_REPR_SECOND      4  /* Second of minute */
#define TIME_REPR_MINUTE      5  /* Minute of hour */
#define TIME_REPR_HOUR        6  /* Hour of day */
#define TIME_REPR_WDAY        7  /* Day of week (0-based; 0 is Sunday) */
#define TIME_REPR_MWEEK       8  /* Week of month (week 1 starts on the first sunday of the month; if the month doesn't start on a sun-day, week 0 exists) */
#define TIME_REPR_MONTH       9  /* Month of year (1-based) */
#define TIME_REPR_YEAR        10 /* Year */
#define TIME_REPR_DECADE      11 /* Decade */
#define TIME_REPR_CENTURY     12 /* Century */
#define TIME_REPR_MILLENNIUM  13 /* Millennium */
#define TIME_REPR_MDAY        14 /* Day of month (1-based) */
#define TIME_REPR_YDAY        15 /* Day of year (1-based) */
#define TIME_REPR_YWEEK       16 /* Week of year (week 1 starts on the first sunday of the year; if the year doesn't start on a sun-day, week 0 exists) */

/* Non-modulated variations (the other representation
 * is truncated to singles of the next higher-level view) */
#define TIME_REPR_PLURAL       32
#define TIME_REPR_NANOSECONDS  (TIME_REPR_PLURAL | TIME_REPR_NANOSECOND)  /* Total nanoseconds */
#define TIME_REPR_MICROSECONDS (TIME_REPR_PLURAL | TIME_REPR_MICROSECOND) /* Total microseconds */
#define TIME_REPR_MILLISECONDS (TIME_REPR_PLURAL | TIME_REPR_MILLISECOND) /* Total milliseconds */
#define TIME_REPR_SECONDS      (TIME_REPR_PLURAL | TIME_REPR_SECOND)      /* Total seconds */
#define TIME_REPR_MINUTES      (TIME_REPR_PLURAL | TIME_REPR_MINUTE)      /* Total minutes */
#define TIME_REPR_HOURS        (TIME_REPR_PLURAL | TIME_REPR_HOUR)        /* Total hours */
#define TIME_REPR_DAYS         (TIME_REPR_PLURAL | TIME_REPR_WDAY)        /* Total days */
#define TIME_REPR_WEEKS        (TIME_REPR_PLURAL | TIME_REPR_MWEEK)       /* Total weeks */
#define TIME_REPR_MONTHS       (TIME_REPR_PLURAL | TIME_REPR_MONTH)       /* Total months */
#define TIME_REPR_YEARS        (TIME_REPR_PLURAL | TIME_REPR_YEAR)        /* Total years */
#define TIME_REPR_DECADES      (TIME_REPR_PLURAL | TIME_REPR_DECADE)      /* Total decade */
#define TIME_REPR_CENTURIES    (TIME_REPR_PLURAL | TIME_REPR_CENTURY)     /* Total centuries */
#define TIME_REPR_MILLENNIA    (TIME_REPR_PLURAL | TIME_REPR_MILLENNIUM)  /* Total millennia */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TIME_TYPEKIND(type, kind) ((type) | (kind) << 8)
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define TIME_TYPEKIND(type, kind) ((type) << 8 | (kind))
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */

union u_time_repr_kind {
	struct {
		uint8_t  t_type; /* Time encoding (One of `TIME_*') */
		uint8_t  t_kind; /* Time representation (One of `TIME_REPR_*') */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
	_dee_astruct
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
	;
	uint16_t     t_typekind; /* Encoded using TIME_TYPEKIND(type, kind) */
};


typedef struct time_object DeeTimeObject;
struct time_object {
	Dee_OBJECT_HEAD
	union {
		/* Nanoseconds (1_000_000_000 / sec) since 01-01-0000T00:00:00 (Gregorian calender).
		 * NOTE: 0 was chosen due to the fact that this way time-offsets/timeouts
		 *       and exact points in time can use the same type internally,
		 *       making it quite strait-forward to do time-based calculation such
		 *       as `print (now()+days(5)).wday; // The week day 5 days into the future'
		 *       Also note that this implementation assumes a flat time-scale which
		 *       doesn't take leap seconds or daylight-savings into account, meaning
		 *       that as far as this timer is concerned, it'll just jump back and
		 *       forth twice a year by one hour.
		 *       The implementation does however take leap years into account!
		 */
		Dee_int128_t t_nanos;   /* TIME_TYPE_NANOSECONDS */
		Dee_uint128_t t_unanos; /* TIME_TYPE_NANOSECONDS */

		/* Because months can have differing lengths dependent on
		 * which which they refer to, the time type needs to be able
		 * to represent months and years just as well as nanoseconds.
		 * For that reason, they are represented individually here, but are
		 * converted into nanoseconds adjusted for some other time-object.
		 * Arithmetic rules are as follows:
		 *   - NANOSECONDS <op> NANOSECONDS --> NANOSECONDS
		 *   - NANOSECONDS <op> MONTHS      --> NANOSECONDS
		 *   - MONTHS <op> NANOSECONDS      --> NANOSECONDS
		 *   - MONTHS <op> MONTHS           --> MONTHS
		 * As far as time representation goes, the representation
		 * of the left-hand-side is always inherited by the result.
		 */
		Dee_int128_t t_months; /* TIME_TYPE_MONTHS */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define t_nanos  _dee_aunion.t_nanos
#define t_months _dee_aunion.t_months
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	union {
		struct {
			uint8_t  t_type; /* Time encoding (One of `TIME_*') */
			uint8_t  t_kind; /* Time kind (One of `TIME_KIND_*') */
		}
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
		_dee_astruct
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
		;
		uint16_t     t_typekind; /* Encoded using TIME_TYPEKIND(type, kind) */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion2
#define t_typekind   _dee_aunion2.t_typekind
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define t_type   _dee_aunion2._dee_astruct.t_type
#define t_kind   _dee_aunion2._dee_astruct.t_kind
#else /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#define t_type   _dee_aunion2.t_type
#define t_kind   _dee_aunion2.t_kind
#endif /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#elif !defined(__COMPILER_HAVE_TRANSPARENT_STRUCT)
#define t_type   _dee_astruct.t_type
#define t_kind   _dee_astruct.t_kind
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
	;
};

#define DeeTime_IsTimestamp(self) ((self)->t_kind == TIME_KIND_TIMESTAMP)
#define DeeTime_IsDelta(self)     ((self)->t_kind != TIME_KIND_TIMESTAMP)


/* Some time constants */
#define MICROSECONDS_PER_MILLISECOND UINT16_C(1000)         /* signed-11-bit */
#define MICROSECONDS_PER_SECOND      UINT32_C(1000000)      /* signed-21-bit */
#define MICROSECONDS_PER_MINUTE      UINT32_C(60000000)     /* signed-27-bit */
#define MICROSECONDS_PER_HOUR        UINT64_C(3600000000)   /* signed-33-bit */
#define MICROSECONDS_PER_DAY         UINT64_C(86400000000)  /* signed-38-bit */
#define MICROSECONDS_PER_WEEK        UINT64_C(604800000000) /* signed-41-bit */
#define MILLISECONDS_PER_SECOND      UINT16_C(1000)         /* signed-11-bit */
#define SECONDS_PER_MINUTE           UINT8_C(60)            /* signed-7-bit */
#define MINUTES_PER_HOUR             UINT8_C(60)            /* signed-7-bit */
#define HOURS_PER_DAY                UINT8_C(24)            /* signed-6-bit */
#define DAYS_PER_WEEK                UINT8_C(7)             /* signed-4-bit */
#define MONTHS_PER_YEAR              UINT8_C(12)            /* signed-5-bit */
#define MONTHS_PER_DECADE            UINT8_C(120)           /* signed-8-bit */
#define MONTHS_PER_CENTURY           UINT16_C(1200)         /* signed-12-bit */
#define MONTHS_PER_MILLENNIUM        UINT16_C(12000)        /* signed-15-bit */
#define YEARS_PER_DECADE             UINT8_C(10)            /* signed-5-bit */
#define DECADES_PER_CENTURY          UINT8_C(10)            /* signed-5-bit */
#define CENTURIES_PER_MILLENNIUM     UINT8_C(10)            /* signed-5-bit */
#define SECONDS_PER_YEAR             UINT32_C(31556952)     /* signed-26-bit */
#define MAX_DAYS_PER_MONTH           31
#define MAX_DAYS_PER_YEAR            366

#define SECONDS_PER_HOUR   UINT16_C(3600)        /* signed-13-bit */
#define SECONDS_PER_DAY    UINT32_C(86400)       /* signed-18-bit */
#define SECONDS_01_01_1970 UINT64_C(0xe79747c00) /* Seconds from 01-01-0000 to 01-01-1970 */


/*
 * Since we use nanoseconds from 01-01-0000T00:00:00, and 128-bit integers, that gives us the following limits:
 * - nanoseconds  (128-bit): [-170141183460469231731687303715884105728, 170141183460469231731687303715884105727]
 * - microseconds (119-bit): [-170141183460469231731687303715884106, 170141183460469231731687303715884105]
 * - milliseconds (109-bit): [-170141183460469231731687303715885, 170141183460469231731687303715884]
 * - seconds       (99-bit): [-170141183460469231731687303716, 170141183460469231731687303715]
 * - minutes       (93-bit): [-2835686391007820528861455062, 2835686391007820528861455061]
 * - hours         (87-bit): [-47261439850130342147690918, 47261439850130342147690917]
 * - days          (82-bit): [-1969226660422097589487122, 1969226660422097589487121]
 * - weeks         (79-bit): [-281318094346013941355304, 281318094346013941355303]
 * - years         (74-bit): [-5391559471918239497012, 5391559471918239497011]
 * - decades       (70-bit): [-539155947191823949702, 539155947191823949701]
 * - centuries     (67-bit): [-53915594719182394971, 53915594719182394970]
 * - millennia     (64-bit): [-5391559471918239498, 5391559471918239497]
 */
#define NANOSECONDS_PER_MICROSECOND UINT16_C(1000)                /* signed-11-bit */
#define NANOSECONDS_PER_MILLISECOND UINT32_C(1000000)             /* signed-21-bit */
#define NANOSECONDS_PER_SECOND      UINT32_C(1000000000)          /* signed-31-bit */
#define NANOSECONDS_PER_MINUTE      UINT64_C(60000000000)         /* signed-37-bit */
#define NANOSECONDS_PER_HOUR        UINT64_C(3600000000000)       /* signed-43-bit */
#define NANOSECONDS_PER_DAY         UINT64_C(86400000000000)      /* signed-48-bit */
#define NANOSECONDS_PER_WEEK        UINT64_C(604800000000000)     /* signed-51-bit */
#define NANOSECONDS_PER_MONTH_AVG   UINT64_C(2629746000000000)    /* signed-53-bit */
#define NANOSECONDS_PER_YEAR_AVG    UINT64_C(31556952000000000)   /* signed-56-bit */
#define NANOSECONDS_PER_DECADE_AVG  UINT64_C(315569520000000000)  /* signed-60-bit */
#define NANOSECONDS_PER_CENTURY_AVG UINT64_C(3155695200000000000) /* signed-63-bit */
#define MONTHS_PER_DECADE           UINT8_C(120)                  /* signed-8-bit */
#define MONTHS_PER_CENTURY          UINT16_C(1200)                /* signed-12-bit */
#define MONTHS_PER_MILLENNIUM       UINT16_C(12000)               /* signed-15-bit */
#define YEARS_PER_CENTURY           UINT8_C(100)                  /* signed-8-bit */
#define YEARS_PER_MILLENNIUM        UINT16_C(1000)                /* signed-11-bit */
#define DAYS_PER_400_YEARS          UINT32_C(146097)              /* signed-19-bit */
PRIVATE Dee_int128_t const NANOSECONDS_PER_MILLENNIUM_AVG =       /* signed-66-bit */
__HYBRID_INT128_INIT16N(0x0000, 0x0000, 0x0000, 0x0001, 0xb5f0, 0xd0a5, 0xea0d, 0x8000);


#define time_inplace_clearmod32(p_value, clearmod_mask)                       \
	do {                                                                      \
		uint32_t _ticm32_sub;                                                 \
		__hybrid_int128_floormod32_r(*(p_value), clearmod_mask, _ticm32_sub); \
		__hybrid_int128_sub32(*(p_value), _ticm32_sub);                       \
	}	__WHILE0
#define time_inplace_clearmod64(p_value, clearmod_mask)                       \
	do {                                                                      \
		uint64_t _ticm64_sub;                                                 \
		__hybrid_int128_floormod64_r(*(p_value), clearmod_mask, _ticm64_sub); \
		__hybrid_int128_sub64(*(p_value), _ticm64_sub);                       \
	}	__WHILE0
#define time_inplace_clearmod64_keepmod64(p_value, clearmod_mask, keepmod_mask) \
	do {                                                                        \
		uint64_t _ticm64_sub, _ticm64_add;                                      \
		__hybrid_int128_floormod64_r(*(p_value), clearmod_mask, _ticm64_sub);   \
		__hybrid_int128_floormod64_r(*(p_value), keepmod_mask, _ticm64_add);    \
		__hybrid_int128_sub64(*(p_value), _ticm64_sub);                         \
		__hybrid_int128_add64(*(p_value), _ticm64_add);                         \
	}	__WHILE0


/* Proper (exact) conversion between days-since-01-01-0000 and the relevant year (or a year, and that year's 01-01-XXXX) */
#define time_inplace_decade2day(p_value)     (__hybrid_int128_mul8(*(p_value), UINT8_C(10)), time_inplace_year2day(p_value))
#define time_inplace_century2day(p_value)    (__hybrid_int128_mul8(*(p_value), UINT8_C(100)), time_inplace_year2day(p_value))
#define time_inplace_millennium2day(p_value) (__hybrid_int128_mul16(*(p_value), UINT8_C(1000)), time_inplace_year2day(p_value))
#define time_inplace_day2decade(p_value)     (time_inplace_day2year(p_value), __hybrid_int128_floordiv8(*(p_value), UINT8_C(10)))
#define time_inplace_day2century(p_value)    (time_inplace_day2year(p_value), __hybrid_int128_floordiv8(*(p_value), UINT8_C(100)))
#define time_inplace_day2millennium(p_value) (time_inplace_day2year(p_value), __hybrid_int128_floordiv16(*(p_value), UINT16_C(1000)))

/* Convert a specific year/... to that year/...'s starting nanosecond (rather than an amount of years/...) */
#define time_inplace_year2nanosecond(p_value)       (time_inplace_year2day(p_value), time_inplace_days2nanoseconds(p_value))
#define time_inplace_nanosecond2year(p_value)       (time_inplace_nanoseconds2days(p_value), time_inplace_day2year(p_value))
#define time_inplace_decade2nanosecond(p_value)     (time_inplace_decade2day(p_value), time_inplace_days2nanoseconds(p_value))
#define time_inplace_nanosecond2decade(p_value)     (time_inplace_nanoseconds2days(p_value), time_inplace_day2decade(p_value))
#define time_inplace_century2nanosecond(p_value)    (time_inplace_century2day(p_value), time_inplace_days2nanoseconds(p_value))
#define time_inplace_nanosecond2century(p_value)    (time_inplace_nanoseconds2days(p_value), time_inplace_day2century(p_value))
#define time_inplace_millennium2nanosecond(p_value) (time_inplace_millennium2day(p_value), time_inplace_days2nanoseconds(p_value))
#define time_inplace_nanosecond2millennium(p_value) (time_inplace_nanoseconds2days(p_value), time_inplace_day2millennium(p_value))

/* Convert between time quantities (in the case of variable-length quantities, use that quantity's average length) */
#define time_inplace_microseconds2nanoseconds(p_value) __hybrid_int128_mul16(*(p_value), NANOSECONDS_PER_MICROSECOND)
#define time_inplace_nanoseconds2microseconds(p_value) __hybrid_int128_floordiv16(*(p_value), NANOSECONDS_PER_MICROSECOND)
#define time_inplace_milliseconds2nanoseconds(p_value) __hybrid_int128_mul32(*(p_value), NANOSECONDS_PER_MILLISECOND)
#define time_inplace_nanoseconds2milliseconds(p_value) __hybrid_int128_floordiv32(*(p_value), NANOSECONDS_PER_MILLISECOND)
#define time_inplace_seconds2nanoseconds(p_value)      __hybrid_int128_mul32(*(p_value), NANOSECONDS_PER_SECOND)
#define time_inplace_nanoseconds2seconds(p_value)      __hybrid_int128_floordiv32(*(p_value), NANOSECONDS_PER_SECOND)
#define time_inplace_minutes2nanoseconds(p_value)      __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_MINUTE)
#define time_inplace_nanoseconds2minutes(p_value)      __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_MINUTE)
#define time_inplace_hours2nanoseconds(p_value)        __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_HOUR)
#define time_inplace_nanoseconds2hours(p_value)        __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_HOUR)
#define time_inplace_days2nanoseconds(p_value)         __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_DAY)
#define time_inplace_nanoseconds2days(p_value)         __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_DAY)
#define time_inplace_weeks2nanoseconds(p_value)        __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_WEEK)
#define time_inplace_nanoseconds2weeks(p_value)        __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_WEEK)
#define time_inplace_months2nanoseconds(p_value)       __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_MONTH_AVG)
#define time_inplace_nanoseconds2months(p_value)       __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_MONTH_AVG)
#define time_inplace_years2nanoseconds(p_value)        __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_YEAR_AVG)
#define time_inplace_nanoseconds2years(p_value)        __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_YEAR_AVG)
#define time_inplace_decades2nanoseconds(p_value)      __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_DECADE_AVG)
#define time_inplace_nanoseconds2decades(p_value)      __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_DECADE_AVG)
#define time_inplace_centuries2nanoseconds(p_value)    __hybrid_int128_mul64(*(p_value), NANOSECONDS_PER_CENTURY_AVG)
#define time_inplace_nanoseconds2centuries(p_value)    __hybrid_int128_floordiv64(*(p_value), NANOSECONDS_PER_CENTURY_AVG)
#define time_inplace_millennia2nanoseconds(p_value)    __hybrid_int128_mul128(*(p_value), NANOSECONDS_PER_MILLENNIUM_AVG)
#define time_inplace_nanoseconds2millennia(p_value)    __hybrid_int128_floordiv128(*(p_value), NANOSECONDS_PER_MILLENNIUM_AVG)

/* Encode/decode a time-value to/from various different representations. */
#define time_get_nanoseconds(p_self, p_result)  (void)(*(p_result) = *(p_self))
#define time_get_microseconds(p_self, p_result) (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2microseconds(p_result))
#define time_get_milliseconds(p_self, p_result) (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2milliseconds(p_result))
#define time_get_seconds(p_self, p_result)      (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2seconds(p_result))
#define time_get_minutes(p_self, p_result)      (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2minutes(p_result))
#define time_get_hours(p_self, p_result)        (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2hours(p_result))
#define time_get_days(p_self, p_result)         (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2days(p_result))
#define time_get_weeks(p_self, p_result)        (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2weeks(p_result))
#define time_get_month(p_self, p_result)        (void)(*(p_result) = *(p_self), time_inplace_nanosecond2month(p_result)) /* # of whole months since 01-01-0000 */
#define time_get_months(p_self, p_result)       (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2months(p_result))
#define time_get_year(p_self, p_result)         (void)(*(p_result) = *(p_self), time_inplace_nanosecond2year(p_result)) /* # of whole years since 01-01-0000 */
#define time_get_years(p_self, p_result)        (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2years(p_result))
#define time_get_decade(p_self, p_result)       (void)(*(p_result) = *(p_self), time_inplace_nanosecond2decade(p_result)) /* # of whole decades since 01-01-0000 */
#define time_get_decades(p_self, p_result)      (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2decades(p_result))
#define time_get_century(p_self, p_result)      (void)(*(p_result) = *(p_self), time_inplace_nanosecond2century(p_result)) /* # of whole centuries since 01-01-0000 */
#define time_get_centuries(p_self, p_result)    (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2centuries(p_result))
#define time_get_millennium(p_self, p_result)   (void)(*(p_result) = *(p_self), time_inplace_nanosecond2millennium(p_result)) /* # of whole millennia since 01-01-0000 */
#define time_get_millennia(p_self, p_result)    (void)(*(p_result) = *(p_self), time_inplace_nanoseconds2millennia(p_result))

#define time_set_nanoseconds(p_self, p_value)  (void)(*(p_self) = *(p_value))
#define time_set_microseconds(p_self, p_value) (void)(*(p_self) = *(p_value), time_inplace_microseconds2nanoseconds(p_self))
#define time_set_milliseconds(p_self, p_value) (void)(*(p_self) = *(p_value), time_inplace_milliseconds2nanoseconds(p_self))
#define time_set_seconds(p_self, p_value)      (void)(*(p_self) = *(p_value), time_inplace_seconds2nanoseconds(p_self))
#define time_set_minutes(p_self, p_value)      (void)(*(p_self) = *(p_value), time_inplace_minutes2nanoseconds(p_self))
#define time_set_hours(p_self, p_value)        (void)(*(p_self) = *(p_value), time_inplace_hours2nanoseconds(p_self))
#define time_set_days(p_self, p_value)         (void)(*(p_self) = *(p_value), time_inplace_days2nanoseconds(p_self))
#define time_set_weeks(p_self, p_value)        (void)(*(p_self) = *(p_value), time_inplace_weeks2nanoseconds(p_self))
#define time_set_months(p_self, p_value)       (void)(*(p_self) = *(p_value), time_inplace_months2nanoseconds(p_self))
#define time_set_years(p_self, p_value)        (void)(*(p_self) = *(p_value), time_inplace_years2nanoseconds(p_self))
#define time_set_decades(p_self, p_value)      (void)(*(p_self) = *(p_value), time_inplace_decades2nanoseconds(p_self))
#define time_set_centuries(p_self, p_value)    (void)(*(p_self) = *(p_value), time_inplace_centuries2nanoseconds(p_self))
#define time_set_millennia(p_self, p_value)    (void)(*(p_self) = *(p_value), time_inplace_millennia2nanoseconds(p_self))

#ifdef CONFIG_BUILDING_LIBTIME
INTDEF DeeTypeObject DeeTime_Type;
#define DeeTime_Check(ob)      DeeObject_InstanceOf(ob, &DeeTime_Type)
#define DeeTime_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeTime_Type)

/* Return the integer value for the specified representation of `self' */
INTDEF NONNULL((1, 2)) void DFCALL
_DeeTime_GetRepr(Dee_int128_t *__restrict p_result,
                 DeeTimeObject const *__restrict self,
                 uint8_t repr);
INTDEF WUNUSED NONNULL((1)) Dee_int128_t DFCALL
DeeTime_GetRepr(DeeTimeObject const *__restrict self,
                uint8_t repr);
INTDEF WUNUSED NONNULL((1)) uint8_t DFCALL
DeeTime_GetRepr8(DeeTimeObject const *__restrict self, uint8_t repr);
INTDEF WUNUSED NONNULL((1)) uint32_t DFCALL
DeeTime_GetRepr32(DeeTimeObject const *__restrict self, uint8_t repr);

/* Set the integer value for the specified representation of `self' */
INTDEF NONNULL((1, 2)) void DFCALL
DeeTime_SetRepr(DeeTimeObject *__restrict self,
                Dee_int128_t const *__restrict p_value,
                uint8_t repr);


/* Return the nano-seconds value of `self'. When `self' references months,
 * calculate the number of nanoseconds for those months since `01.0000'. */
#define DeeTime_AsNano(self, p_result)               \
	(*(p_result) = (self)->t_nanos,                  \
	 likely((self)->t_type == TIME_TYPE_NANOSECONDS) \
	 ? (void)0                                       \
	 : time_inplace_months2nanoseconds(p_result))
#define DeeTime_MakeNano(self)                                   \
	(likely((self)->t_type == TIME_TYPE_NANOSECONDS)             \
	 ? (void)0                                                   \
	 : (void)(time_inplace_months2nanoseconds(&(self)->t_nanos), \
	          (self)->t_type = TIME_TYPE_NANOSECONDS))


/* Return the current time in UTC */
INTDEF NONNULL((1)) void DFCALL time_now_utc(Dee_int128_t *__restrict p_result);
INTDEF NONNULL((1)) void DFCALL time_now_local(Dee_int128_t *__restrict p_result);

/* Convert between days-since-01-01-0000 and that the relevant year.
 * When converting from year-to-day, return that year's 01-01-XXXX. */
INTDEF NONNULL((1)) void DFCALL time_inplace_day2year(Dee_int128_t *__restrict p_value);
INTDEF NONNULL((1)) void DFCALL time_inplace_year2day(Dee_int128_t *__restrict p_value);

/* Convert between a specific month and that month's starting nano-second */
INTDEF NONNULL((1)) void DFCALL time_inplace_nanosecond2month(Dee_int128_t *__restrict p_value);
INTDEF NONNULL((1)) void DFCALL time_inplace_month2nanosecond(Dee_int128_t *__restrict p_value);

/* Check if the year referenced by the year-counter `*p_year' is a leap-year */
INTDEF WUNUSED NONNULL((1)) bool DFCALL
time_years_isleapyear(Dee_int128_t const *__restrict p_year);

struct month {
	uint64_t m_start; /* Nano-seconds into the year for when this month starts */
	uint16_t m_len;   /* == end-start */
	uint16_t m_name;  /* Offset into month_names to the month's name.
	                   * NOTE: The full name can be found at `m_name+4' */
};
#define month_getstart(self)     ((self)[0].m_start)
#define month_getend(self)       ((self)[1].m_start)
#define month_getlen(self)       ((self)->m_len)
#define month_getname_abbr(self) (month_names + (self)->m_name)
#define month_getname_full(self) (month_names + (self)->m_name + 4)

INTDEF char const abbr_wday_names[7][4];
INTDEF char const full_wday_names[7][10];
INTDEF char const am_pm[2][3];
INTDEF char const am_pm_lower[2][3];
INTDEF char const month_names[];
INTDEF struct month const month_info[2][MONTHS_PER_YEAR + 1];
#define month_info_for_year(p_year) month_info[time_years_isleapyear(p_year)]

#define get_wday_full(i)  full_wday_names[i]
#define get_wday_abbr(i)  abbr_wday_names[i]
#define get_month_full(i) month_getname_full(&month_info[0][i])
#define get_month_abbr(i) month_getname_abbr(&month_info[0][i])


/* C-API functions exported by the `time' dex. */
#ifdef CONFIG_BUILDING_LIBTIME
#define LIBTIME_FUNDEF __EXPDEF
#else /* CONFIG_BUILDING_LIBTIME */
#define LIBTIME_FUNDEF __IMPDEF
#endif /* !CONFIG_BUILDING_LIBTIME */

LIBTIME_FUNDEF WUNUSED DREF DeeObject *DCALL
DeeTime_NewUnix(int64_t seconds_since_01_01_1970,
                uint32_t extra_nanoseconds);
#ifdef CONFIG_HOST_WINDOWS
LIBTIME_FUNDEF WUNUSED DREF DeeObject *DCALL
DeeTime_NewFILETIME(void const *p_filetime);
#endif /* CONFIG_HOST_WINDOWS */

#endif /* CONFIG_BUILDING_LIBTIME */


DECL_END

#endif /* !GUARD_DEX_TIME_LIBTIME_H */
