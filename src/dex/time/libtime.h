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
#ifndef GUARD_DEX_TIME_LIBTIME_H
#define GUARD_DEX_TIME_LIBTIME_H 1

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/thread.h>

#include <stdint.h>

DECL_BEGIN

#define TIMECALL FCALL

#undef ceildiv
#define ceildiv(x, y) (((x) + ((y)-1)) / (y))

#ifdef CONFIG_NATIVE_INT128

#define HAVE_128BIT_TIME 1
#define SIZEOF_DTIME_T 16
typedef dint128_t  dtime_t;
typedef duint128_t dutime_t;
typedef int64_t    dtime_half_t;
typedef uint64_t   dutime_half_t;

#else /* CONFIG_NATIVE_INT128 */

/* >> 2000*365*24*60*60*1000*1000 (MSB: 56 --> The top 8 bits are still
 *    clear, so this timer won't overrun for at least 582000 more years,
 *    at which point I'm assuming that anything will be capable of handling
 *    a 128-bit integer (which is why support for those exists)) */
#define SIZEOF_DTIME_T 8
typedef int64_t  dtime_t;
typedef uint64_t dutime_t;
typedef int32_t  dtime_half_t;
typedef uint32_t dutime_half_t;

#endif /* !CONFIG_NATIVE_INT128 */

/* NOTE: Everything >= day can be represented using `dtime_half_t' */



/* Time encoding */
#define TIME_MICROSECONDS  0
#define TIME_MONTHS        1
/* Preferred representation.
 * >> print days(2);            // `2 days'
 * >> print days(2).hours;      // `48 hours'
 * >> print int(days(2));       // 2*24*60*60*1000*1000
 * >> print int(days(2).hours); // 2*24*60*60*1000*1000
 */
#define TIME_REPR_NONE     0 /* No specific representation.
                              * When this is set, an `Error.TypeError'
                              * is thrown when `intval' is invoked. */
#define TIME_REPR_MIC      1
#define TIME_REPR_MIL      2
#define TIME_REPR_SEC      3
#define TIME_REPR_MIN      4
#define TIME_REPR_HOR      5
#define TIME_REPR_WDAY     6
#define TIME_REPR_MWEK     7
#define TIME_REPR_MON      8
#define TIME_REPR_YER      9
#define TIME_REPR_DEC     10
#define TIME_REPR_CEN     11
#define TIME_REPR_MLL     12
#define TIME_REPR_MDAY    13
#define TIME_REPR_YDAY    14
#define TIME_REPR_YWEK    15
 /* Non-modulated variations (the other representation
  * is truncated to singles of the next higher-level view) */
#define TIME_REPR_PLURAL  16
#define TIME_REPR_MICS   (TIME_REPR_PLURAL | 1)
#define TIME_REPR_MILS   (TIME_REPR_PLURAL | 2)
#define TIME_REPR_SECS   (TIME_REPR_PLURAL | 3)
#define TIME_REPR_MINS   (TIME_REPR_PLURAL | 4)
#define TIME_REPR_HORS   (TIME_REPR_PLURAL | 5)
#define TIME_REPR_DAYS   (TIME_REPR_PLURAL | 6)
#define TIME_REPR_WEKS   (TIME_REPR_PLURAL | 7)
#define TIME_REPR_MONS   (TIME_REPR_PLURAL | 8)
#ifdef CONFIG_LITTLE_ENDIAN
#define TIME_KIND(type, repr) ((type) | (repr) << 8)
#else /* CONFIG_LITTLE_ENDIAN */
#define TIME_KIND(type, repr) ((type) << 8 | (repr))
#endif /* !CONFIG_LITTLE_ENDIAN */

typedef struct time_object DeeTimeObject;
struct time_object {
	OBJECT_HEAD
	union {
		/* Microseconds (1000*1000 / sec) since 0:0:0 1.1.0000 (Gregorian calender).
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
		dtime_t      t_time;   /* TIME_MICROSECONDS */
#ifdef HAVE_128BIT_TIME
		dtime_half_t t_time_half[2];
#endif
		/* Because months can have differing lengths dependent on
		 * which which they refer to, the time type needs to be able
		 * to represent months and years just as well as microseconds.
		 * For that reason, they are represented individually here, but are
		 * converted into microseconds adjusted for some other time-object.
		 * Arithmetic rules are as follows:
		 *   - MICROSECONDS + MICROSECONDS --> MICROSECONDS
		 *   - MICROSECONDS + MONTHS       --> MICROSECONDS
		 *   - MONTHS + MICROSECONDS       --> MICROSECONDS
		 *   - MONTHS + MONTHS             --> MONTHS
		 * As far as time representation goes, the representation
		 * of the left-hand-side is always inherited by the result.
		 */
		dtime_half_t t_months; /* TIME_MONTHS */
	};
	union {
		struct {
			uint8_t  t_type; /* Time encoding (One of `TIME_*') */
			uint8_t  t_repr; /* Time representation (One of `TIME_REPR_*') */
		};
		uint16_t     t_kind; /* Encoded using TIME_KIND(type,repr) */
	};
};

#ifdef HAVE_128BIT_TIME
#ifdef CONFIG_LITTLE_ENDIAN
#   define _Time_Set64(x,y) ((x).t_time_half[1] = 0,(x).t_time_half[0] = (y))
#   define _Time_Get64(x)    (x).t_time_half[0]
#else /* CONFIG_LITTLE_ENDIAN */
#   define _Time_Set64(x,y) ((x).t_time_half[0] = 0,(x).t_time_half[1] = (y))
#   define _Time_Get64(x)    (x).t_time_half[1]
#endif /* !CONFIG_LITTLE_ENDIAN */
#else /* HAVE_128BIT_TIME */
#   define _Time_Get64(x)    (x).t_time
#   define _Time_Set64(x,y) ((x).t_time = (y))
#endif /* !HAVE_128BIT_TIME */

#ifdef CONFIG_BUILDING_LIBTIME
INTDEF DeeTypeObject DeeTime_Type;
#define DeeTime_Check(ob)      DeeObject_InstanceOf(ob, &DeeTime_Type)
#define DeeTime_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeTime_Type)

EXPDEF DREF DeeObject *(DCALL DeeTime_New)(uint64_t microseconds);
INTDEF DREF DeeObject *DCALL DeeTime_New_(dtime_t microseconds, uint16_t kind);
INTDEF DREF DeeObject *DCALL DeeTime_NewMonths_(dtime_half_t num_months, uint16_t kind);
#define DeeTime_New(microseconds, repr)     DeeTime_New_(microseconds, TIME_KIND(TIME_MICROSECONDS, repr))
#define DeeTime_NewMonths(num_months, repr) DeeTime_NewMonths_(num_months, TIME_KIND(TIME_MONTHS, repr))
INTDEF dtime_t DCALL DeeTime_Get(DeeTimeObject const *__restrict self);
#ifdef HAVE_128BIT_TIME
INTDEF DREF DeeObject *DCALL DeeTime_New64_(uint64_t microseconds, uint16_t kind);
#define DeeTime_New64(microseconds, repr) DeeTime_New64_(microseconds, TIME_KIND(TIME_MICROSECONDS, repr))
INTDEF WUNUSED NONNULL((1)) uint64_t DCALL DeeTime_Get64(DeeTimeObject const *__restrict self);
#else /* HAVE_128BIT_TIME */
#define DeeTime_Get64(self)               DeeTime_Get(self)
#define DeeTime_New64(microseconds, repr) DeeTime_New(microseconds, repr)
#endif /* !HAVE_128BIT_TIME */
#define DeeTime_Set(self, value) \
	(void)((self)->t_time = (value), (self)->t_type = TIME_MICROSECONDS)
#define DeeTime_Set64(self, value) \
	(void)(_Time_Set64(*(self), value), (self)->t_type = TIME_MICROSECONDS)

INTDEF dtime_t DCALL time_now(void);
#endif /* CONFIG_BUILDING_LIBTIME */

#define time_tick() DeeThread_GetTimeMicroSeconds()

/* Convert between different time formats.
 *   - mic -- Microsecond (Really f-ing short)
 *   - mil -- Millisecond (1000 Microsecond)
 *   - sec -- Second (1000 Millisecond)
 *   - min -- Minute (60 Seconds)
 *   - hor -- Hour (60 Minutes)
 *   - day -- Day (24 Hours)
 *   - wek -- Week (7 Days)
 *   - mon -- Month (28-31 Days)
 *   - yer -- Year (12 Months / 365|366 Days)
 *   - dec -- Decade (10 Years)
 *   - cen -- Century (100 Years)
 *   - mll -- Millennium (1000 Years)
 */


#define MICROSECONDS_PER_MILLISECOND UINT64_C(1000)
#define MILLISECONDS_PER_SECOND      UINT64_C(1000)
#define SECONDS_PER_MINUTE           UINT64_C(60)
#define MINUTES_PER_HOUR             UINT64_C(60)
#define HOURS_PER_DAY                UINT64_C(24)
#define DAYS_PER_WEEK                UINT64_C(7)
#define MONTHS_PER_YEAR              UINT64_C(12)
#define YEARS_PER_DECADE             UINT64_C(10)
#define DECADES_PER_CENTURY          UINT64_C(10)
#define CENTURIES_PER_MILLENIUM      UINT64_C(10)
#define MAX_DAYS_PER_MONTH           31
#define MAX_DAYS_PER_YEAR            366

#define time_isleapyear(year)   (!((year) % 400) || (((year) % 100) && !((year) % 4)))
#define time_numleapyears(year) ((((year) / 4) - (((year) / 100) - ((year) / 400))) + 1)

/* NOTE: Converting days <-> years and days <-> months assumes a base of `0'! */
#define time_day2yer(x)    ((400 * ((x) + 1)) / 146097)
#define time_yer2day(x)    (((146097 * (x)) / 400) /*-1*/)
#ifdef CONFIG_BUILDING_LIBTIME
INTDEF dtime_half_t (TIMECALL time_day2mon)(dtime_half_t x);
INTDEF dtime_half_t (TIMECALL time_mon2day)(dtime_half_t x);
#endif /* CONFIG_BUILDING_LIBTIME */
#define time_day2mon(x) time_day2mon((dtime_half_t)(x))
#define time_mon2day(x) time_mon2day((dtime_half_t)(x))



#define MICROSECONDS_PER_SECOND (MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND)
#define MICROSECONDS_PER_MINUTE (MICROSECONDS_PER_SECOND * SECONDS_PER_MINUTE)
#define MICROSECONDS_PER_HOUR   (MICROSECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define MICROSECONDS_PER_DAY    (MICROSECONDS_PER_HOUR * HOURS_PER_DAY)
#define MICROSECONDS_PER_WEEK   (MICROSECONDS_PER_DAY * DAYS_PER_WEEK)
#define MONTHS_PER_DECADE       (MONTHS_PER_YEAR * YEARS_PER_DECADE)
#define MONTHS_PER_CENTURY      (MONTHS_PER_DECADE * DECADES_PER_CENTURY)
#define MONTHS_PER_MILLENIUM    (MONTHS_PER_CENTURY * CENTURIES_PER_MILLENIUM)
#define YEARS_PER_CENTURY       (YEARS_PER_DECADE * DECADES_PER_CENTURY)
#define YEARS_PER_MILLENIUM     (YEARS_PER_CENTURY * CENTURIES_PER_MILLENIUM)

/* Given `x' as microseconds from 0, return various interpretations. */
#define time_get_microseconds(x)   (x)
#define time_get_milliseconds(x)  ((x) / MICROSECONDS_PER_MILLISECOND)
#define time_get_seconds(x)       ((x) / MICROSECONDS_PER_SECOND)
#define time_get_minutes(x)       ((x) / MICROSECONDS_PER_MINUTE)
#define time_get_hours(x)         ((x) / MICROSECONDS_PER_HOUR)
#define time_get_days(x)          ((x) / MICROSECONDS_PER_DAY)
#define time_get_weeks(x)         ((x) / MICROSECONDS_PER_WEEK)
#define time_get_months(x)          time_day2mon(time_get_days(x))
#define time_get_years(x)           time_day2yer(time_get_days(x))
#define time_get_decades(x)        (time_day2yer(time_get_days(x)) / YEARS_PER_DECADE)
#define time_get_centuries(x)      (time_day2yer(time_get_days(x)) / YEARS_PER_CENTURY)
#define time_get_millenia(x)       (time_day2yer(time_get_days(x)) / YEARS_PER_MILLENIUM)
#define time_set_microseconds(x)   (x)
#define time_set_milliseconds(x)  ((x) * MICROSECONDS_PER_MILLISECOND)
#define time_set_seconds(x)       ((x) * MICROSECONDS_PER_SECOND)
#define time_set_minutes(x)       ((x) * MICROSECONDS_PER_MINUTE)
#define time_set_hours(x)         ((x) * MICROSECONDS_PER_HOUR)
#define time_set_days(x)          ((x) * MICROSECONDS_PER_DAY)
#define time_set_weeks(x)         ((x) * MICROSECONDS_PER_WEEK)
#define time_set_months(x)          time_set_days(time_mon2day(x))
#define time_set_years(x)           time_set_days(time_yer2day(x))
#define time_set_decades(x)         time_set_days(time_yer2day((x) * YEARS_PER_DECADE)))
#define time_set_centuries(x)       time_set_days(time_yer2day((x) * YEARS_PER_CENTURY)))
#define time_set_millenia(x)        time_set_days(time_yer2day((x) * YEARS_PER_MILLENIUM)))


DECL_END

#endif /* !GUARD_DEX_TIME_LIBTIME_H */
