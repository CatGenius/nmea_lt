//#include "configbits.h"		/* PIC MCU configuration bits, include before anything else */
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "serial.h"


#define MINUTES_PER_HOUR        (60)
#define SECONDS_PER_MINUTES     (60)
#define SECONDS_PER_HOUR        (MINUTES_PER_HOUR * SECONDS_PER_MINUTES)
#define DST_OFFSET_S            (1 * SECONDS_PER_HOUR)
#define LT_OFFSET_S             (1 * SECONDS_PER_HOUR)

#define TIME_ZONE               (1)
#define TIME_ZONE_M             (TIME_ZONE * MINUTES_PER_HOUR)


static void mkdst(struct tm *utc, struct tm *local)
{
	/* Return if flag is already valid */
	if (utc->tm_isdst >= 0)
		return;

	/* Copy UTC to Local time */
	*local = *utc;

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
		/* Check if we're not in the last week of the month */
		if (local->tm_mday <= 31 - 7) {
			local->tm_isdst = (local->tm_mon == 9) ? 1 : 0;
			return;
		}

		/* Check if it's the last Sunday now */
		if (local->tm_wday == 0) {

			return;
		}

		/* Check if we're past the last Sunday */
		if (7 - local->tm_wday > 31) {

			return;
		}

		break;
	}
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
static void interrupt_init(void)
{
	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}

static void interrupt isr(void)
{
	/* Timer 1 interrupt */
	if (TMR1IF) {
		/* Reset interrupt */
		TMR1IF = 0;
		/* Handle interrupt */
//		timer_isr();
	}

	/* (E)USART 1 interrupts */
//	if (RC1IF)
//		serial_rx_isr();
//	if (TX1IF)
//		serial_tx_isr();

	/* (E)USART 2 interrupts */
	if (RC2IF)
		serial_rx_isr();
	if (TX2IF)
		serial_tx_isr();
}

void main(void)
{
	struct tm  utc;
	struct tm  local;
	time_t     utc_secs;
	time_t     local_secs;

	time_zone = TIME_ZONE;

	utc.tm_sec   = 0;   /* The number of seconds after the minute, normally in the
		               range 0 to 59, but can be up to 60 to allow for leap sec-
		               onds. */
	utc.tm_min   = 20;  /* The number of minutes after the hour, in the range 0 to 59. */
	utc.tm_hour  = 20;  /* The number of hours past midnight, in the range 0 to 23. */
	utc.tm_mday  = 11;  /* The day of the month, in the range 1 to 31. */
	utc.tm_mon   = 8;   /* The number of months since January, in the range 0 to 11. */
	utc.tm_year  = 120; /* The number of years since 1900. */
	utc.tm_wday  = -1;  /* The number of days since Sunday, in the range 0 to 6. */
	utc.tm_yday  = -1;  /* The number of days since January 1, in the range 0 to 365. */
	utc.tm_isdst = -1;  /* A flag that indicates whether daylight saving time is in
		               effect at the time described.  The value is positive if
		               daylight saving time is in effect, zero if it is not, and
			       negative if the information is not available. */

	/* Convert UTC time to seconds and complete with weekday */
	if (!(utc_secs = mktime(&utc)))
		return;

	/* Set the DST flag (tzset() is not supported) */
	mkdst(&utc, &local);
	
	local_secs = utc_secs +
	             LT_OFFSET_S +
	             ((local.tm_isdst > 0) ? DST_OFFSET_S : 0);

//	localtime();
//	gmtime();

//	ODCONC &= ~0x01;	/* Disable open-drain on output RC0, setting it to push-pull */

    // NOSC HFINTOSC; NDIV 1;
    OSCCON1 = 0x60;
    // CSWHOLD may proceed; SOSCPWR Low power;
    OSCCON3 = 0x00;
    // MFOEN disabled; LFOEN disabled; ADOEN disabled; SOSCEN disabled; EXTOEN disabled; HFOEN disabled;
    OSCEN = 0x00;
    // HFFRQ 32_MHz;
    OSCFRQ = 0x06;
    // MFOR not ready;
    OSCSTAT = 0x00;
    // HFTUN 0;
    OSCTUNE = 0x00;
    
    
GIE = 0;
PPSLOCK = 0x55;
PPSLOCK = 0xAA;
PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS

RC0PPS = 0x11;  /* Connect RC0 output to TX2 */
//RB7PPS = 0x0F;  /* Connect RB7 output to TX1 */
PPSLOCK = 0x55;
PPSLOCK = 0xAA;
PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS

	/* Initialize the serial port for stdio */
	serial_init(115200, 0);

	/* Initialize interrupts */
	interrupt_init();

	/* Execute the run loop */
	for(;;) {
//		LATC = 0;
//		LATC = 1;
		printf("Test\n");
	}
}
