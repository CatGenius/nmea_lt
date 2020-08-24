#include "rtc.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define SECONDS_PER_MINUTE      60UL
#define MINUTES_PER_HOUR        60UL
#define HOURS_PER_DAY           24UL
#define DAYS_PER_WEEK           7
#define DAYS_PER_YEAR           365              /* Number of days in a non-leap year */
#define DAYS_PER_4_YEARS        (3 * 365 + 366)  /* There is one leap year every 4 years */
#define DAYS_PER_CENTURY        36524UL          /* There are 24 leap years in 100-year periods. ((100 - 24) * 365 + 24 * 366) */
#define DAYS_PER_ERA            146097UL         /* There are 97 leap years in 400-year periods. ((400 - 97) * 365 + 97 * 366) */
#define YEARS_PER_ERA           400

#define SECONDS_PER_HOUR        (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY         (SECONDS_PER_HOUR * HOURS_PER_DAY)

#define EPOCH_DAY               1
#define EPOCH_MONTH             0
#define EPOCH_WEEKDAY           6

#define BASE_YEAR               0                /* The year 0000 */
#define EPOCH_OFFSET_DAYS       (730425UL)


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


static rtcsecs_t year2days(unsigned int year)
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
int rtc_time2secs(struct rtctime_t *rtctime, rtcsecs_t *rtcsecs)
{
	if (rtctime->year > 105 ||
	    rtctime->mon  >  11 ||
	    rtctime->day  >  31 ||
	    rtctime->hour >  23 ||
	    rtctime->min  >  59 ||
	    rtctime->sec  >  60)
		return -1;

	*rtcsecs = date2days((unsigned int)rtctime->year + EPOCH_YEAR,
	                 rtctime->mon + 1, 
	                 rtctime->day);
	*rtcsecs -= date2days(EPOCH_YEAR,
	                  EPOCH_MONTH + 1,
	                  EPOCH_DAY);

	*rtcsecs = *rtcsecs * HOURS_PER_DAY      + rtctime->hour;
	*rtcsecs = *rtcsecs * MINUTES_PER_HOUR   + rtctime->min;
	*rtcsecs = *rtcsecs * SECONDS_PER_MINUTE + rtctime->sec;

	return 0;
}


void rtc_secs2time(rtcsecs_t rtcsecs, struct rtctime_t *rtctime)
{
	rtcsecs_t      days;
	rtcsecs_t      remsecs;
	unsigned char  era;
	rtcsecs_t      eraday;
	unsigned int   erayear;
	unsigned int   yearday;
	unsigned char  month;
	unsigned char  day;
	unsigned int   year;

	/* Split the given time up in whole days and the seconds remainder */
	days    = rtcsecs / SECONDS_PER_DAY + EPOCH_OFFSET_DAYS;  /* [730425, 780135] */
	remsecs = rtcsecs % SECONDS_PER_DAY;                      /* [0, 86399] */

	/* Calculate the time of day from the remainder */
	rtctime->hour = (unsigned char)(remsecs / SECONDS_PER_HOUR);
	remsecs %= SECONDS_PER_HOUR;
	rtctime->min  = (unsigned char)(remsecs / SECONDS_PER_MINUTE);
	rtctime->sec  = (unsigned char)(remsecs % SECONDS_PER_MINUTE);

	/* Calculate the date (see: http://howardhinnant.github.io/date_algorithms.html#civil_from_days) */
	era     = days / DAYS_PER_ERA;        /* [0, 5] */
	eraday  = days - era * DAYS_PER_ERA;  /* [0, 146096] */
	erayear = ( eraday -
	            eraday / (DAYS_PER_4_YEARS - 1) +
	            eraday / DAYS_PER_CENTURY -
	            eraday / (DAYS_PER_ERA - 1) ) / DAYS_PER_YEAR;  /* [0, 399] */
	yearday = eraday - (DAYS_PER_YEAR * erayear + erayear / 4 - erayear / 100);  /* [0, 365] */
	month   = (5 * yearday + 2) / 153;  /* [0, 11] */
	day     = yearday - (153 * month + 2) / 5 + 1;  /* [1, 31] */
	month  += month < 10 ? 2 : -10;
	year    = BASE_YEAR + erayear + era * YEARS_PER_ERA + (month <= 1);

	rtctime->year = (unsigned char)(year - EPOCH_YEAR);
	rtctime->mon  = (unsigned char)month;
	rtctime->day  = (unsigned char)day;
}


unsigned char rtc_weekday(rtcsecs_t rtcsecs)
{
	return (EPOCH_WEEKDAY + rtcsecs / HOURS_PER_DAY / MINUTES_PER_HOUR / SECONDS_PER_MINUTE) % DAYS_PER_WEEK;
}
