#include "rtc.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define DAYS_PER_WEEK       7
#define HOURS_PER_DAY       24
#define MINUTE_PER_HOUR     60
#define SECONDS_PER_MINUTE  60

#define EPOCH_DAY           1
#define EPOCH_MONTH         0
#define EPOCH_YEAR          2000
#define EPOCH_WEEKDAY       6


/******************************************************************************/
/* Static functions                                                           */
/******************************************************************************/
static unsigned char leapyear(unsigned int year)
{
	return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}


static rtcsecs_t month2days(unsigned char mon)
{
	return ((rtcsecs_t)mon * 3057 - 3007) / 100;
}


static unsigned long year2days(unsigned int year)
{
	return year * 365 + year / 4 - year / 100 + year / 400;
}


static rtcsecs_t date2days(unsigned int year, unsigned char mon, unsigned char day)
{
	rtcsecs_t  days = (rtcsecs_t)day + month2days(mon);

	if (mon > 2)
		days -= leapyear(year) ? 1 : 2;
	year--;
	days += year2days(year);

	return days;
}


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
rtcsecs_t rtc_time2secs(struct rtctime_t *rtctime)
{
	rtcsecs_t  secs;

	if (rtctime->year > 105 ||
	    rtctime->mon  >  11 ||
	    rtctime->day  >  31 ||
	    rtctime->hour >  23 ||
	    rtctime->min  >  59 ||
	    rtctime->sec  >  60)
		return (rtcsecs_t)-1;

	secs = date2days((unsigned int)rtctime->year + EPOCH_YEAR,
	                 rtctime->mon + 1, 
	                 rtctime->day);
	secs -= date2days(EPOCH_YEAR,
	                  EPOCH_MONTH + 1,
	                  EPOCH_DAY);

	secs = secs * HOURS_PER_DAY      + rtctime->hour;
	secs = secs * MINUTE_PER_HOUR    + rtctime->min;
	secs = secs * SECONDS_PER_MINUTE + rtctime->sec;

	return secs;
}


unsigned char rtc_weekday(rtcsecs_t rtcsecs)
{
	return (EPOCH_WEEKDAY + rtcsecs / HOURS_PER_DAY / MINUTE_PER_HOUR / SECONDS_PER_MINUTE) % DAYS_PER_WEEK;
}
