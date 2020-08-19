#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
//#define TEST_MKDST


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
#ifdef TEST_MKDST
void test_mkdst(void)
{
	static const struct reftime_t {
		struct tm  tm;                 /* Time to test */
		time_t     secs;               /* Expected number of seconds since Epoch for time to test */
		int        wday;               /* Expected weekday for time to test */
		int        dst;                /* Expected daylight saving time state for time to test */
		int        dst_one_sec_prior;  /* Expected daylight saving time state for 1 second prior to time to test */
	} reftime[] = {
		/* Test daylight saving time switch-over (2017 was chosen because it tests all cases in mkdst()) */
		{ { 0, 0, 0, 24, 2, 117, -1, -1, -1 }, 1490313600, 5, 0, 0 },  /* 0:00:00 am | Friday, 24 March 2017 */
		{ { 0, 0, 0, 25, 2, 117, -1, -1, -1 }, 1490400000, 6, 0, 0 },  /* 0:00:00 am | Saturday, 25 March 2017 */
		{ { 0, 0, 0, 26, 2, 117, -1, -1, -1 }, 1490486400, 0, 0, 0 },  /* 0:00:00 am | Sunday, 26 March 2017 */
		{ { 0, 0, 1, 26, 2, 117, -1, -1, -1 }, 1490490000, 0, 1, 0 },  /* 1:00:00 am | Sunday, 26 March 2017 */
		{ { 0, 0, 2, 26, 2, 117, -1, -1, -1 }, 1490493600, 0, 1, 1 },  /* 2:00:00 am | Sunday, 26 March 2017 */
		{ { 0, 0, 0, 27, 2, 117, -1, -1, -1 }, 1490572800, 1, 1, 1 },  /* 0:00:00 am | Monday, 27 March 2017 */
		{ { 0, 0, 0, 28, 2, 117, -1, -1, -1 }, 1490659200, 2, 1, 1 },  /* 0:00:00 am | Tuesday, 28 March 2017 */
		{ { 0, 0, 0, 29, 2, 117, -1, -1, -1 }, 1490745600, 3, 1, 1 },  /* 0:00:00 am | Wednesday, 29 March 2017 */
		{ { 0, 0, 0, 30, 2, 117, -1, -1, -1 }, 1490832000, 4, 1, 1 },  /* 0:00:00 am | Thursday, 30 March 2017 */
		{ { 0, 0, 0, 31, 2, 117, -1, -1, -1 }, 1490918400, 5, 1, 1 },  /* 0:00:00 am | Friday, 31 March 2017 */

		/* Test standard time switch-over (2017 was chosen because it tests all cases in mkdst()) */
		{ { 0, 0, 0, 24, 9, 117, -1, -1, -1 }, 1508803200, 2, 1, 1 },  /* 0:00:00 am | Tuesday, 24 October 2017 */
		{ { 0, 0, 0, 25, 9, 117, -1, -1, -1 }, 1508889600, 3, 1, 1 },  /* 0:00:00 am | Wednesday, 25 October 2017 */
		{ { 0, 0, 0, 26, 9, 117, -1, -1, -1 }, 1508976000, 4, 1, 1 },  /* 0:00:00 am | Thursday, 26 October 2017 */
		{ { 0, 0, 0, 27, 9, 117, -1, -1, -1 }, 1509062400, 5, 1, 1 },  /* 0:00:00 am | Friday, 27 October 2017 */
		{ { 0, 0, 0, 28, 9, 117, -1, -1, -1 }, 1509148800, 6, 1, 1 },  /* 0:00:00 am | Saturday, 28 October 2017 */
		{ { 0, 0, 0, 29, 9, 117, -1, -1, -1 }, 1509235200, 0, 1, 1 },  /* 0:00:00 am | Sunday, 29 October 2017 */
		{ { 0, 0, 1, 29, 9, 117, -1, -1, -1 }, 1509238800, 0, 0, 1 },  /* 1:00:00 am | Sunday, 29 October 2017 */
		{ { 0, 0, 2, 29, 9, 117, -1, -1, -1 }, 1509242400, 0, 0, 0 },  /* 2:00:00 am | Sunday, 29 October 2017 */
		{ { 0, 0, 0, 30, 9, 117, -1, -1, -1 }, 1509321600, 1, 0, 0 },  /* 0:00:00 am | Monday, 30 October 2017 */
		{ { 0, 0, 0, 31, 9, 117, -1, -1, -1 }, 1509408000, 2, 0, 0 },  /* 0:00:00 am | Tuesday, 31 October 2017 */

		/* Test at the exact switch-over times */
		{ { 0, 0, 1, 25, 2, 118, -1, -1, -1 }, 1521939600, 0, 1, 0 },  /* 1:00:00 am | Sunday, 25 March 2018 */
		{ { 0, 0, 1, 28, 9, 118, -1, -1, -1 }, 1540688400, 0, 0, 1 },  /* 1:00:00 am | Sunday, 28 October 2018 */
		{ { 0, 0, 1, 31, 2, 119, -1, -1, -1 }, 1553994000, 0, 1, 0 },  /* 1:00:00 am | Sunday, 31 March 2019 */
		{ { 0, 0, 1, 27, 9, 119, -1, -1, -1 }, 1572138000, 0, 0, 1 },  /* 1:00:00 am | Sunday, 27 October 2019 */
		{ { 0, 0, 1, 29, 2, 120, -1, -1, -1 }, 1585443600, 0, 1, 0 },  /* 1:00:00 am | Sunday, 29 March 2020 */
		{ { 0, 0, 1, 25, 9, 120, -1, -1, -1 }, 1603587600, 0, 0, 1 },  /* 1:00:00 am | Sunday, 25 October 2020 */
		{ { 0, 0, 1, 28, 2, 121, -1, -1, -1 }, 1616893200, 0, 1, 0 },  /* 1:00:00 am | Sunday, 28 March 2021 */
		{ { 0, 0, 1, 31, 9, 121, -1, -1, -1 }, 1635642000, 0, 0, 1 },  /* 1:00:00 am | Sunday, 31 October 2021 */
		{ { 0, 0, 1, 27, 2, 122, -1, -1, -1 }, 1648342800, 0, 1, 0 },  /* 1:00:00 am | Sunday, 27 March 2022 */
		{ { 0, 0, 1, 30, 9, 122, -1, -1, -1 }, 1667091600, 0, 0, 1 },  /* 1:00:00 am | Sunday, 30 October 2022 */
		{ { 0, 0, 1, 26, 2, 123, -1, -1, -1 }, 1679792400, 0, 1, 0 },  /* 1:00:00 am | Sunday, 26 March 2023 */
		{ { 0, 0, 1, 29, 9, 123, -1, -1, -1 }, 1698541200, 0, 0, 1 },  /* 1:00:00 am | Sunday, 29 October 2023 */
		{ { 0, 0, 1, 31, 2, 124, -1, -1, -1 }, 1711846800, 0, 1, 0 },  /* 1:00:00 am | Sunday, 31 March 2024 */
		{ { 0, 0, 1, 27, 9, 124, -1, -1, -1 }, 1729990800, 0, 0, 1 },  /* 1:00:00 am | Sunday, 27 October 2024 */
		{ { 0, 0, 1, 30, 2, 125, -1, -1, -1 }, 1743296400, 0, 1, 0 },  /* 1:00:00 am | Sunday, 30 March 2025 */
		{ { 0, 0, 1, 26, 9, 125, -1, -1, -1 }, 1761440400, 0, 0, 1 },  /* 1:00:00 am | Sunday, 26 October 2025 */
		{ { 0, 0, 1, 29, 2, 126, -1, -1, -1 }, 1774746000, 0, 1, 0 },  /* 1:00:00 am | Sunday, 29 March 2026 */
		{ { 0, 0, 1, 25, 9, 126, -1, -1, -1 }, 1792890000, 0, 0, 1 },  /* 1:00:00 am | Sunday, 25 October 2026 */
		{ { 0, 0, 1, 28, 2, 127, -1, -1, -1 }, 1806195600, 0, 1, 0 },  /* 1:00:00 am | Sunday, 28 March 2027 */
		{ { 0, 0, 1, 31, 9, 127, -1, -1, -1 }, 1824944400, 0, 0, 1 },  /* 1:00:00 am | Sunday, 31 October 2027 */
		{ { 0, 0, 1, 26, 2, 128, -1, -1, -1 }, 1837645200, 0, 1, 0 },  /* 1:00:00 am | Sunday, 26 March 2028 */
		{ { 0, 0, 1, 29, 9, 128, -1, -1, -1 }, 1856394000, 0, 0, 1 },  /* 1:00:00 am | Sunday, 29 October 2028 */
		{ { 0, 0, 1, 25, 2, 129, -1, -1, -1 }, 1869094800, 0, 1, 0 },  /* 1:00:00 am | Sunday, 25 March 2029 */
		{ { 0, 0, 1, 28, 9, 129, -1, -1, -1 }, 1887843600, 0, 0, 1 },  /* 1:00:00 am | Sunday, 28 October 2029 */
		{ { 0, 0, 1, 31, 2, 130, -1, -1, -1 }, 1901149200, 0, 1, 0 },  /* 1:00:00 am | Sunday, 31 March 2030 */
		{ { 0, 0, 1, 27, 9, 130, -1, -1, -1 }, 1919293200, 0, 0, 1 },  /* 1:00:00 am | Sunday, 27 October 2030 */

		/* Test past 2039 32-bit Epoch roll-over */
		{ { 0, 0, 1, 28, 2, 130, -1, -1, -1 }, 2153350800, 0, 1, 0 },  /* 1:00:00 am | Sunday, 28 March 2038 */
		{ { 0, 0, 1, 31, 9, 130, -1, -1, -1 }, 2172099600, 0, 0, 1 }   /* 1:00:00 am | Sunday, 31 October 2038 */
	};
	int  ndx;

	for (ndx = 0; ndx < ARRAY_SIZE(reftime); ndx++) {
		struct tm  dst;
		time_t     secs = mktime(&reftime[ndx].tm);

		/* Test if mktime() produced the expected number of seconds since Epoch */
		if (secs != reftime[ndx].secs) {
			printf("mktime() failed for ndx %d (secs = %ld, expected %ld)\n", ndx, secs, reftime[ndx].secs);
			continue;
		}

		mkdst(secs, &dst);
		if (dst.tm_wday != reftime[ndx].wday) {
			printf("gmtime() failed for ndx %d (tm_wday = %d, expected %d)\n", ndx, dst.tm_wday, reftime[ndx].wday);
			continue;
		}
		if (dst.tm_isdst != reftime[ndx].dst) {
			printf("mkdst() failed for ndx %d (tm_isdst = %d, expected %d)\n", ndx, dst.tm_isdst, reftime[ndx].dst);
			continue;
		}

		mkdst(secs - 1, &dst);
		if (dst.tm_isdst != reftime[ndx].dst_one_sec_prior) {
			printf("mkdst() failed for 1 second prior to ndx %d (tm_isdst = %d, expected %d)\n", ndx, dst.tm_isdst, reftime[ndx].dst_one_sec_prior);
			continue;
		}

		printf("ndx %d OK\n", ndx);
	}
}
#endif /* TEST_MKDST */


void mkdst(time_t utc_secs, struct tm *local)
{
	struct tm  *utc = gmtime(&utc_secs);  /* Note: XC8 completes struct tm with tm_wday in gmtime(), not in mktime() as is standard */

	/* Copy UTC to Local time */
	*local = *utc;

	/* Invalidate daylight saving time flag, mistakenly set 0 instead of -1 by gmtime() */
	local->tm_isdst = -1;

	switch(local->tm_mon) {
	case 10:  /* November */
	case 11:  /* December */
	case 0:   /* January */
	case 1:   /* February */
		local->tm_isdst = 0;
		return;

	case 3:   /* April */
	case 4:   /* May */
	case 5:   /* June */
	case 6:   /* July */
	case 7:   /* August */
	case 8:   /* September */
		local->tm_isdst = 1;
		return;

	case 2:  /* March */
	case 9:  /* October */
		/* Check if it is not the last week of the month yet */
		if (local->tm_mday <= 31 - 7) {
			local->tm_isdst = (local->tm_mon == 9) ? 1 : 0;
			return;
		}

		/* Check if it is the last Sunday now */
		if (local->tm_wday == 0) {
			if (local->tm_hour > 0)
				local->tm_isdst = (local->tm_mon == 9) ? 0 : 1;
			else
				local->tm_isdst = (local->tm_mon == 9) ? 1 : 0;
			return;
		}

		/* Check if it is past the last Sunday */
		if (local->tm_mday + 7 - local->tm_wday > 31) {
			local->tm_isdst = (local->tm_mon == 9) ? 0 : 1;
			return;
		}

		/* It is before the last Sunday */
		local->tm_isdst = (local->tm_mon == 9) ? 1 : 0;
		return;
	}
}
