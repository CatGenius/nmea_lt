#include "rtc.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/


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

	secs = date2days((unsigned int)rtctime->year + 2000, 
	                 rtctime->mon + 1, 
	                 rtctime->day);
	secs -= date2days(2000, 1, 1);

	secs = secs * 24 + rtctime->hour;
	secs = secs * 60 + rtctime->min;
	secs = secs * 60 + rtctime->sec;

	return secs;
}
