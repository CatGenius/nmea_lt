#ifndef	RTC_H
#define	RTC_H


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define SECONDS_PER_MINUTE      60UL
#define MINUTES_PER_HOUR        60UL
#define HOURS_PER_DAY           24UL

#define SECONDS_PER_HOUR        (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY         (SECONDS_PER_HOUR * HOURS_PER_DAY)

#define EPOCH_YEAR              2000
#define UNIX_EPOCH_OFFSET       946684800UL


/******************************************************************************/
/* Types                                                                      */
/******************************************************************************/
#ifdef __x86_64__
typedef	unsigned int   rtcsecs_t;
#else
typedef	unsigned long  rtcsecs_t;
#endif

struct rtctime_t {
	unsigned char  sec;   /* 0...59 & 60 */
	unsigned char  min;   /* 0...59 */
	unsigned char  hour;  /* 0...23 */
	unsigned char  day;   /* 1...31 */
	unsigned char  mon;   /* 0...11 */
	unsigned char  year;  /* 0...105 (for 2000...2105) */
};


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
int           rtc_time2secs  (const struct rtctime_t  *rtctime,
                              rtcsecs_t               *rtcsecs);
void          rtc_secs2time  (rtcsecs_t               rtcsecs,
                              struct rtctime_t        *rtctime);
unsigned char rtc_weekday    (rtcsecs_t               rtcsecs);
unsigned char rtc_dst_eu     (struct rtctime_t        *utc,
                              unsigned char           weekday);
void          rtc_dst_eu_test(void);


#endif	/* RTC_H */
