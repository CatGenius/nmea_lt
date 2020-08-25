#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtc.h"

#include "mkdst.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
//#define TEST_DST
#define UNIX_EPOCH_OFFSET       946684800UL
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
#ifdef TEST_DST
void test_dst_eu(void)
{
	static const struct reftime_t {
		struct rtctime_t  rtctime;            /* Time to test */
		rtcsecs_t         rtcsecs;            /* Expected number of seconds since Epoch for time to test */
		unsigned char     wday;               /* Expected weekday for time to test */
		unsigned char     dst;                /* Expected daylight saving time state for time to test */
		unsigned char     dst_one_sec_prior;  /* Expected daylight saving time state for 1 second prior to time to test */
	} reftime[] = {
		/* Test daylight saving time switch-over (2017 was chosen because it tests all cases in dst_eu()) */
		{ {  0,  0,  0, 24,  2,  17 }, 1490313600UL - UNIX_EPOCH_OFFSET, 5, 0, 0 },  /* 0:00:00 | Friday,    24 March    2017 */
		{ {  0,  0,  0, 25,  2,  17 }, 1490400000UL - UNIX_EPOCH_OFFSET, 6, 0, 0 },  /* 0:00:00 | Saturday,  25 March    2017 */
		{ {  0,  0,  0, 26,  2,  17 }, 1490486400UL - UNIX_EPOCH_OFFSET, 0, 0, 0 },  /* 0:00:00 | Sunday,    26 March    2017 */
		{ {  0,  0,  1, 26,  2,  17 }, 1490490000UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    26 March    2017 */
		{ {  0,  0,  2, 26,  2,  17 }, 1490493600UL - UNIX_EPOCH_OFFSET, 0, 1, 1 },  /* 2:00:00 | Sunday,    26 March    2017 */
		{ {  0,  0,  0, 27,  2,  17 }, 1490572800UL - UNIX_EPOCH_OFFSET, 1, 1, 1 },  /* 0:00:00 | Monday,    27 March    2017 */
		{ {  0,  0,  0, 28,  2,  17 }, 1490659200UL - UNIX_EPOCH_OFFSET, 2, 1, 1 },  /* 0:00:00 | Tuesday,   28 March    2017 */
		{ {  0,  0,  0, 29,  2,  17 }, 1490745600UL - UNIX_EPOCH_OFFSET, 3, 1, 1 },  /* 0:00:00 | Wednesday, 29 March    2017 */
		{ {  0,  0,  0, 30,  2,  17 }, 1490832000UL - UNIX_EPOCH_OFFSET, 4, 1, 1 },  /* 0:00:00 | Thursday,  30 March    2017 */
		{ {  0,  0,  0, 31,  2,  17 }, 1490918400UL - UNIX_EPOCH_OFFSET, 5, 1, 1 },  /* 0:00:00 | Friday,    31 March    2017 */

		/* Test standard time switch-over (2017 was chosen because it tests all cases in dst_eu()) */
		{ {  0,  0,  0, 24,  9,  17 }, 1508803200UL - UNIX_EPOCH_OFFSET, 2, 1, 1 },  /* 0:00:00 | Tuesday,   24 October  2017 */
		{ {  0,  0,  0, 25,  9,  17 }, 1508889600UL - UNIX_EPOCH_OFFSET, 3, 1, 1 },  /* 0:00:00 | Wednesday, 25 October  2017 */
		{ {  0,  0,  0, 26,  9,  17 }, 1508976000UL - UNIX_EPOCH_OFFSET, 4, 1, 1 },  /* 0:00:00 | Thursday,  26 October  2017 */
		{ {  0,  0,  0, 27,  9,  17 }, 1509062400UL - UNIX_EPOCH_OFFSET, 5, 1, 1 },  /* 0:00:00 | Friday,    27 October  2017 */
		{ {  0,  0,  0, 28,  9,  17 }, 1509148800UL - UNIX_EPOCH_OFFSET, 6, 1, 1 },  /* 0:00:00 | Saturday,  28 October  2017 */
		{ {  0,  0,  0, 29,  9,  17 }, 1509235200UL - UNIX_EPOCH_OFFSET, 0, 1, 1 },  /* 0:00:00 | Sunday,    29 October  2017 */
		{ {  0,  0,  1, 29,  9,  17 }, 1509238800UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    29 October  2017 */
		{ {  0,  0,  2, 29,  9,  17 }, 1509242400UL - UNIX_EPOCH_OFFSET, 0, 0, 0 },  /* 2:00:00 | Sunday,    29 October  2017 */
		{ {  0,  0,  0, 30,  9,  17 }, 1509321600UL - UNIX_EPOCH_OFFSET, 1, 0, 0 },  /* 0:00:00 | Monday,    30 October  2017 */
		{ {  0,  0,  0, 31,  9,  17 }, 1509408000UL - UNIX_EPOCH_OFFSET, 2, 0, 0 },  /* 0:00:00 | Tuesday,   31 October  2017 */

		/* Test at the exact switch-over times */
		{ {  0,  0,  1, 25,  2,  18 }, 1521939600UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    25 March    2018 */
		{ {  0,  0,  1, 28,  9,  18 }, 1540688400UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    28 October  2018 */
		{ {  0,  0,  1, 31,  2,  19 }, 1553994000UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    31 March    2019 */
		{ {  0,  0,  1, 27,  9,  19 }, 1572138000UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    27 October  2019 */
		{ {  0,  0,  1, 29,  2,  20 }, 1585443600UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    29 March    2020 */
		{ {  0,  0,  1, 25,  9,  20 }, 1603587600UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    25 October  2020 */
		{ {  0,  0,  1, 28,  2,  21 }, 1616893200UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    28 March    2021 */
		{ {  0,  0,  1, 31,  9,  21 }, 1635642000UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    31 October  2021 */
		{ {  0,  0,  1, 27,  2,  22 }, 1648342800UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    27 March    2022 */
		{ {  0,  0,  1, 30,  9,  22 }, 1667091600UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    30 October  2022 */
		{ {  0,  0,  1, 26,  2,  23 }, 1679792400UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    26 March    2023 */
		{ {  0,  0,  1, 29,  9,  23 }, 1698541200UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    29 October  2023 */
		{ {  0,  0,  1, 31,  2,  24 }, 1711846800UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    31 March    2024 */
		{ {  0,  0,  1, 27,  9,  24 }, 1729990800UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    27 October  2024 */
		{ {  0,  0,  1, 30,  2,  25 }, 1743296400UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    30 March    2025 */
		{ {  0,  0,  1, 26,  9,  25 }, 1761440400UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    26 October  2025 */
		{ {  0,  0,  1, 29,  2,  26 }, 1774746000UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    29 March    2026 */
		{ {  0,  0,  1, 25,  9,  26 }, 1792890000UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    25 October  2026 */
		{ {  0,  0,  1, 28,  2,  27 }, 1806195600UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    28 March    2027 */
		{ {  0,  0,  1, 31,  9,  27 }, 1824944400UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    31 October  2027 */
		{ {  0,  0,  1, 26,  2,  28 }, 1837645200UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    26 March    2028 */
		{ {  0,  0,  1, 29,  9,  28 }, 1856394000UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    29 October  2028 */
		{ {  0,  0,  1, 25,  2,  29 }, 1869094800UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    25 March    2029 */
		{ {  0,  0,  1, 28,  9,  29 }, 1887843600UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    28 October  2029 */
		{ {  0,  0,  1, 31,  2,  30 }, 1901149200UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    31 March    2030 */
		{ {  0,  0,  1, 27,  9,  30 }, 1919293200UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    27 October  2030 */

		/* Test past 2039 32-bit Epoch roll-over */
		{ {  0,  0,  1, 28,  2,  38 }, 2153350800UL - UNIX_EPOCH_OFFSET, 0, 1, 0 },  /* 1:00:00 | Sunday,    28 March    2038 */
		{ {  0,  0,  1, 31,  9,  38 }, 2172099600UL - UNIX_EPOCH_OFFSET, 0, 0, 1 },  /* 1:00:00 | Sunday,    31 October  2038 */

		/* Test latest date supported */
		{ { 59, 59, 23, 31, 11, 105 }, 4291747199UL - UNIX_EPOCH_OFFSET, 4, 0, 0 }  /* 23:59:59 | Thursday,  31 December 2038 */
	};
	unsigned int  ndx;

	for (ndx = 0; ndx < ARRAY_SIZE(reftime); ndx++) {
		int               result;
		rtcsecs_t         rtcsecs;
		struct rtctime_t  rtctime;
		unsigned char     weekday;
		unsigned char     dst;

		/* Test if rtc_time2secs() can convert this date into seconds */
		result = rtc_time2secs(&reftime[ndx].rtctime, &rtcsecs);
		if (result) {
			printf("rtc_time2secs() @%u (result = %d)\n", ndx, result);
			continue;
		}

		/* Test if rtc_time2secs() produced the expected number of seconds */
		if (rtcsecs != reftime[ndx].rtcsecs) {
			printf("rtc_time2secs() @%u (%lu, expected %lu)\n", ndx, rtcsecs, reftime[ndx].rtcsecs);
			continue;
		}

		/* Test if rtc_secs2time() produces the expected broken-down time */
		rtc_secs2time(rtcsecs, &rtctime);
		if (rtctime.sec  != reftime[ndx].rtctime.sec  ||
		    rtctime.min  != reftime[ndx].rtctime.min  ||
		    rtctime.hour != reftime[ndx].rtctime.hour ||
		    rtctime.day  != reftime[ndx].rtctime.day  ||
		    rtctime.mon  != reftime[ndx].rtctime.mon  ||
		    rtctime.year != reftime[ndx].rtctime.year) {
			printf("rtc_secs2time() @%u (%04u/%02u/%02u %02u:%02u:%02u, expected %04u/%02u/%02u %02u:%02u:%02u)\n", ndx,
			                2000 + rtctime.year, rtctime.mon, rtctime.day, rtctime.hour, rtctime.min, rtctime.sec,
			                2000 + reftime[ndx].rtctime.year, reftime[ndx].rtctime.mon, reftime[ndx].rtctime.day, reftime[ndx].rtctime.hour, reftime[ndx].rtctime.min, reftime[ndx].rtctime.sec);
			continue;
		}

		/* Test if rtc_weekday() produces the expected day of week */
		weekday = rtc_weekday(rtcsecs);
		if (weekday != reftime[ndx].wday) {
			printf("rtc_weekday() @%u (%u, expected %u)\n", ndx, weekday, reftime[ndx].wday);
			continue;
		}

		/* Test if dst_eu() produces the expected daylight saving time state */
		dst = dst_eu(&rtctime, weekday);
		if (dst != reftime[ndx].dst) {
			printf("dst_eu() failed for ndx %u (dst = %u, expected %u)\n", ndx, dst, reftime[ndx].dst);
			continue;
		}

		/* Recalculate for one second prior */
		rtcsecs--;
		rtc_secs2time(rtcsecs, &rtctime);
		weekday = rtc_weekday(rtcsecs);

		/* Test if dst_eu() produces the expected daylight saving time state for one second prior */
		dst = dst_eu(&rtctime, weekday);
		if (dst != reftime[ndx].dst_one_sec_prior) {
			printf("dst_eu() failed for 1 second prior to ndx %d (tm_isdst = %d, expected %d)\n", ndx, dst, reftime[ndx].dst_one_sec_prior);
			continue;
		}

		printf("@%u OK\n", ndx);
	}
}
#endif /* TEST_DST */


unsigned char dst_eu(struct rtctime_t *utc, unsigned char weekday)
{
	unsigned char  dst = 0;

	switch(utc->mon) {
	case 10:  /* November */
	case 11:  /* December */
	case 0:   /* January */
	case 1:   /* February */
		break;

	case 3:   /* April */
	case 4:   /* May */
	case 5:   /* June */
	case 6:   /* July */
	case 7:   /* August */
	case 8:   /* September */
		dst = 1;
		break;

	case 2:  /* March */
	case 9:  /* October */
		/* Check if it is not the last week of the month yet */
		if (utc->day <= 31 - 7) {
			dst = (utc->mon == 9) ? 1U : 0U;
			break;
		}

		/* Check if it is the last Sunday now */
		if (weekday == 0) {
			if (utc->hour > 0) {
				dst = (utc->mon == 9) ? 0U : 1U;
				break;
			}
			dst = (utc->mon == 9) ? 1U : 0U;
			break;
		}

		/* Check if it is past the last Sunday */
		if (utc->day + 7 - weekday > 31) {
			dst = (utc->mon == 9) ? 0U : 1U;
			break;
		}

		/* It is before the last Sunday */
		dst = (utc->mon == 9) ? 1U : 0U;
		break;
	}

	return dst;
}
