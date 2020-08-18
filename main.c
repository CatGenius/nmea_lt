//#include "configbits.h"		/* PIC MCU configuration bits, include before anything else */
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cmdline.h"
#include "uart1.h"
#include "uart2.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define MINUTES_PER_HOUR        (60)
#define SECONDS_PER_MINUTES     (60)
#define SECONDS_PER_HOUR        (MINUTES_PER_HOUR * SECONDS_PER_MINUTES)
#define DST_OFFSET_S            (1 * SECONDS_PER_HOUR)
#define LT_OFFSET_S             (1 * SECONDS_PER_HOUR)

#define TIME_ZONE               (1)
#define TIME_ZONE_M             (TIME_ZONE * MINUTES_PER_HOUR)

//#define TEST_MKDST
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))


/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
const struct command_t  commands[] = {
	{"?",    cmdline_help},
	{"help", cmdline_help},
	{"echo", cmdline_echo},
	{"",     NULL}
};


/******************************************************************************/
/* Static functions                                                           */
/******************************************************************************/
static void mkdst(time_t utc_secs, struct tm *local)
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


#ifdef TEST_MKDST
static void test_mkdst(void)
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

static void disable_peripherals(void)
{
/*
	PMD0bits.IOCMD   = 0;
	PMD0bits.CLKRMD  = 0;
	PMD0bits.NVMMD   = 0;
	PMD0bits.FVRMD   = 0;
	PMD0bits.SYSCMD  = 0;

	PMD1bits.TMR0MD  = 0;
	PMD1bits.TMR1MD  = 0;
	PMD1bits.TMR2MD  = 0;
	PMD1bits.DDS1MD  = 0;
*/
	PMD2bits.ZCDMD   = 0;
	PMD2bits.CMP1MD  = 0;
	PMD2bits.CMP2MD  = 0;
	PMD2bits.ADCMD   = 0;
	PMD2bits.DAC1MD  = 0;

	PMD3bits.CCP1MD  = 0;
	PMD3bits.CCP2MD  = 0;
	PMD3bits.PWM3MD  = 0;
	PMD3bits.PWM4MD  = 0;
	PMD3bits.PWM5MD  = 0;
	PMD3bits.PWM6MD  = 0;

	PMD4bits.CWG1MD  = 0;
	PMD4bits.MSSP1MD = 0;
//	PMD4bits.UART1MD = 1;
//	PMD4bits.UART2MD = 1;

	PMD5bits.CLC1MD  = 0;
	PMD5bits.CLC2MD  = 0;
	PMD5bits.CLC3MD  = 0;
	PMD5bits.CLC4MD  = 0;
}


static void init_clocks(void)
{
	/* Select HFINTOSC, divide by 1*/
	OSCCON1bits.NDIV = 0;
	OSCCON1bits.NOSC = 6;

	/* Clock switch may proceed when the oscillator selected by NOSC is ready may proceed. */
	/* Secondary oscillator operating in Low-power mode */
	OSCCON3 = 0x00;

	/* Disable EXTOEN, HFOEN. MFOEN, LFOEN, SOSCEN and ADOEN */
	OSCEN = 0x00;

	/* Set HFINTOSC to 32MHz */
	OSCFRQbits.HFFRQ = 0x06;

	/* Reset oscillator tuning */
	OSCTUNEbits.HFTUN = 0x00;
}


static void init_pins(void)
{
	/* Disable analog input on pins used digitally */
	ANSELC &= ~((1 << 1) |  /* Pin RC1 */
	            (1 << 5));  /* Pin RC5 */

	/* Unlock Peripheral Pin Select module (PPS) */
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;

	/* UART1 */
	RX1DTPPS = 0x15;  /* Connect RX1 to RC5 input pin */
	RC4PPS   = 0x0f;  /* Connect RC4 output to TX1 */

	/* UART2 */
	RX2DTPPS = 0x11;  /* Connect RX2 to RC1 input pin */
	RC0PPS   = 0x11;  /* Connect RC0 output pin to TX2 */

	/* Lock Peripheral Pin Select module (PPS) */
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 1;
}


static void init_interrupt(void)
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
	if (RC1IF)
		uart1_rx_isr();
	if (TX1IF)
		uart1_tx_isr();

	/* (E)USART 2 interrupts */
	if (RC2IF)
		serial_rx_isr();
	if (TX2IF)
		serial_tx_isr();
}


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
void main(void)
{
	struct tm  utc;
	struct tm  local;
	time_t     utc_secs;
	time_t     local_secs;

	disable_peripherals();
	init_clocks();
	init_pins();

	time_zone = TIME_ZONE;

	utc.tm_sec   = 0;   /* The number of seconds after the minute, normally in the
		               range 0 to 59, but can be up to 60 to allow for leap sec-
		               onds. */
	utc.tm_min   = 20;  /* The number of minutes after the hour, in the range 0 to 59. */
	utc.tm_hour  = 20;  /* The number of hours past midnight, in the range 0 to 23. */
	utc.tm_mday  = 11;  /* The day of the month, in the range 1 to 31. */
	utc.tm_mon   =  7;  /* The number of months since January, in the range 0 to 11. */
	utc.tm_year  = 120; /* The number of years since 1900. */
	utc.tm_wday  = -1;  /* The number of days since Sunday, in the range 0 to 6. */
	utc.tm_yday  = -1;  /* The number of days since January 1, in the range 0 to 365. */
	utc.tm_isdst = -1;  /* A flag that indicates whether daylight saving time is in
		               effect at the time described.  The value is positive if
		               daylight saving time is in effect, zero if it is not, and
			       negative if the information is not available. */

	/* Convert UTC time to seconds and complete with weekday */
	/* (Caveat: XC8 mktime) does NOT fill out tm_wday or tm_yday */
	utc_secs = mktime(&utc);

	/* Set the DST flag (tzset() is not supported) */
	mkdst(utc_secs, &local);

	local_secs = utc_secs +
	             LT_OFFSET_S +
	             ((local.tm_isdst > 0) ? DST_OFFSET_S : 0);
//	printf("local_secs = %ld\n", local_secs);

//	printf("diff = %ld\n", local_secs - utc_secs);

//	localtime();
//	gmtime();

	/* Initialize the serial port for stdio */
	serial_init(115200, 0);
	uart1_init(4800, 0);
		
	printf("\n*** NMEA local time converter ***\n");
	if (!nPOR)
		printf("Power-on reset\n");
	else if (!nBOR)
		printf("Brown-out reset\n");
	else if (!__timeout)
		printf("Watchdog reset\n");
	else if (!__powerdown)
		printf("Pin reset (sleep)\n");
	else
		printf("Pin reset\n");
	nPOR = 1;
	nBOR = 1;

#ifdef TEST_MKDST
	test_mkdst();
#endif /* TEST_MKDST */

	cmdline_init();

	/* Initialize interrupts */
	init_interrupt();

	/* Execute the run loop */
	for(;;) {
		cmdline_work();
		uart1_work();
		CLRWDT();
	}
}
