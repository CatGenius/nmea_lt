//#include "configbits.h"		/* PIC MCU configuration bits, include before anything else */
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cmdline.h"
#include "serial.h"


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
	struct tm  *utc = gmtime(&utc_secs);

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


void disable_peripherals(void)
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
	ANSELC &= ~(1 << 1);

	/* Unlock Peripheral Pin Select module (PPS) */
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;

	/* UART1 */
	RX1DTPPS = 0x15;  /* Connect RX2 to RC5 input pin */
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
	mkdst(utc_secs, &local);

	local_secs = utc_secs +
	             LT_OFFSET_S +
	             ((local.tm_isdst > 0) ? DST_OFFSET_S : 0);

//	localtime();
//	gmtime();

	/* Initialize the serial port for stdio */
	serial_init(115200, 0);

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

	cmdline_init();

	/* Initialize interrupts */
	init_interrupt();

	/* Execute the run loop */
	for(;;) {
		cmdline_work();

		CLRWDT();
	}
}
