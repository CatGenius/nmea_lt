#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rtc.h"


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
int main(int argc, char* argv[])
{
	struct tm  start_tm = {
		.tm_sec   =   0,  /* 0...59 & 60 */
		.tm_min   =   0,  /* 0...59 */
		.tm_hour  =   0,  /* 0...23 */
		.tm_mday  =   1,  /* 1...31 */
		.tm_mon   =   0,  /* 0...11 */
		.tm_year  = 100,  /*  */
		.tm_wday  =  -1,  /* Invalid */
		.tm_yday  =  -1,  /* Invalid */
		.tm_isdst =  -1   /* Invalid */
	};
	struct tm  end_tm = {
		.tm_sec   =  59,  /* 0...59 & 60 */
		.tm_min   =  59,  /* 0...59 */
		.tm_hour  =  23,  /* 0...23 */
		.tm_mday  =  31,  /* 1...31 */
		.tm_mon   =  11,  /* 0...11 */
		.tm_year  = 205,  /*  */
		.tm_wday  =  -1,  /* Invalid */
		.tm_yday  =  -1,  /* Invalid */
		.tm_isdst =  -1   /* Invalid */
	};
	time_t  start_tm_secs;
	time_t  end_tm_secs;
	time_t  test_secs;

	if (sizeof(time_t) < 8) {
		fprintf(stderr, "Error: glibc on this machine is not y2038 proof\n");
		exit(EXIT_FAILURE);
	}

	start_tm_secs = timegm(&start_tm);
	end_tm_secs   = timegm(&end_tm);

	for (test_secs = start_tm_secs; test_secs < end_tm_secs; test_secs++) {
		struct tm         test_tm;
		struct rtctime_t  rtctime;
		rtcsecs_t         rtcsecs;
		unsigned char     wday;

		/* Convert time-under-test into broken-down time */
		if (gmtime_r(&test_secs, &test_tm) != &test_tm) {
			fprintf(stderr, "Error: time_t %ld could not be converted into broken-down time\n", test_secs);
			exit(EXIT_FAILURE);
		}

		/* Copy broken-down time into RTC struct */
		rtctime.sec  = test_tm.tm_sec;
		rtctime.min  = test_tm.tm_min;
		rtctime.hour = test_tm.tm_hour;
		rtctime.day  = test_tm.tm_mday;
		rtctime.mon  = test_tm.tm_mon;
		rtctime.year = test_tm.tm_year - 100;

		/* Convert broken-down time in RTC struct into seconds (since 1/1/2000) */
		if (rtc_time2secs(&rtctime, &rtcsecs) == -1) {
			fprintf(stderr, "Error: rtc_time2secs() failed for time %s", asctime(&test_tm));
			exit(EXIT_FAILURE);
		}

		/* Test the result */
		if (rtcsecs != test_secs - start_tm_secs) {
			fprintf(stderr, "Error: rtc_time2secs() produced %d, expected %ld for time %s", rtcsecs, test_secs - start_tm_secs, asctime(&test_tm));
			exit(EXIT_FAILURE);
		}

		/* Test the weekday function */
		if ((wday = rtc_weekday(rtcsecs)) != test_tm.tm_wday) {
			fprintf(stderr, "Error: rtc_weekday() produced %d, expected %d for time %s", wday, test_tm.tm_wday, asctime(&test_tm));
			exit(EXIT_FAILURE);
		}

		rtc_secs2time(rtcsecs, &rtctime);
		if (rtctime.sec  != test_tm.tm_sec  ||
		    rtctime.min  != test_tm.tm_min  ||
		    rtctime.hour != test_tm.tm_hour ||
		    rtctime.day  != test_tm.tm_mday ||
		    rtctime.mon  != test_tm.tm_mon  ||
		    rtctime.year != test_tm.tm_year - 100) {
			fprintf(stderr, "Error: rtc_secs2time() produced %02u/%02u/%02u %02u:%02u:%02u, expected %02u/%02u/%02u %02u:%02u:%02u for time %s",
			                rtctime.year, rtctime.mon, rtctime.day, rtctime.hour, rtctime.min, rtctime.sec,
			                test_tm.tm_year - 100, test_tm.tm_mon, test_tm.tm_mday, test_tm.tm_hour, test_tm.tm_min, test_tm.tm_sec,
			                asctime(&test_tm));
			exit(EXIT_FAILURE);
		}
	}

	fprintf(stderr, "Test completed successfully\n");

	return EXIT_SUCCESS;
}
