//#include "configbits.h"		/* PIC MCU configuration bits, include before anything else */
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "uart1.h"
#include "uart2.h"
#include "mkdst.h"
#include "cmdline.h"
#include "nmea.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define EPOCH_YEAR              (70)
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
	{"?",     cmdline_help},
	{"help",  cmdline_help},
	{"echo",  cmdline_echo},
	{NULL,    NULL}
};

static void handle_gprmc(int argc, char *argv[]);
const struct nmea_t     nmea[] = {
	{"GPRMC", handle_gprmc},
	{NULL,    NULL}
};


/******************************************************************************/
/* Static functions                                                           */
/******************************************************************************/
static void update_rtc(time_t utc)
{
	printf("time = %ld\n", utc);
}


static int get_octets(char *str, int *octet[])
{
	int  ndx;

	/* Walk the given string, back to front, two digits at a time */
	for (ndx = 4; ndx >= 0; ndx -= 2) {
		char           *endptr;
		unsigned long  value = strtoul(&str[ndx], &endptr, 10);  /* Convert this part of the string to a numerical value */

		/* Test for trailing garbage */
		if (*endptr != '\0') {
			printf("Error converting %s to a number\n", &str[ndx]);
			return -1;
		}

		/* Copy the numerical value into the corresponding octet */
		*octet[ndx >> 1] = value;

		/* Overwrite the current part of the sting with a 0-terminator, so after we move back to the front, the string stops here */
		str[ndx] = '\0';
	}

	return 0;
}


static void handle_gprmc(int argc, char *argv[])
{
	struct tm  utc;
	struct tm  dst;
	struct tm  *local;
	time_t     utc_secs;
	time_t     local_secs;
	int        *octet[3];
	int        ndx;

	/* Get the 3 octets holding the time from the time argument */
	octet[0] = &utc.tm_hour;
	octet[1] = &utc.tm_min;
	octet[2] = &utc.tm_sec;
	if (get_octets(argv[1], octet))
		return;

	/* Get the 3 octets holding the date from the date argument */
	octet[0] = &utc.tm_mday;
	octet[1] = &utc.tm_mon;
	octet[2] = &utc.tm_year;
	if (get_octets(argv[9], octet))
		return;

	/* Make month 0-based */
	utc.tm_mon--;
	/* Make year 1900-based */
	if (utc.tm_year < EPOCH_YEAR)
		utc.tm_year += 100;

	/* Convert broken-down UTC time to seconds and complete with weekday */
	/* (Caveat: XC8 mktime) does NOT fill out tm_wday or tm_yday */
	if ((utc_secs = mktime(&utc)) < 0)
		return;

	/* Send the newly received UTC time to the Real Time Clock */
	update_rtc(utc_secs);

	/* Set the DST flag (tzset() is not supported) */
	mkdst(utc_secs, &dst);

	/* Add local time offset and daylight saving time to UTC to get local time */
	local_secs = utc_secs +
	             LT_OFFSET_S +
	             ((dst.tm_isdst > 0) ? DST_OFFSET_S : 0);

	/* Break down local time in seconds */
	local = gmtime(&local_secs);

	/* Make month 1-based */
	local->tm_mon++;
	/* Make year 1900/2000-based */
	local->tm_year %= 100;

	/* Print the broken-down time back into the corresponding arguments (snprintf is not supported, modulo 100 should offer enough protection) */
	sprintf(argv[1], "%02d%02d%02d", local->tm_hour % 100, local->tm_min % 100, local->tm_sec % 100);
	sprintf(argv[9], "%02d%02d%02d", local->tm_mday % 100, local->tm_mon % 100, local->tm_year);

	nmea_send(argc, argv);

	return;
}


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
		uart2_rx_isr();
	if (TX2IF)
		uart2_tx_isr();
}


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
void main(void)
{
	disable_peripherals();
	init_clocks();
	init_pins();

	/* Initialize the serial port for stdio */
	uart2_init(115200, 0);
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
		nmea_work();
		CLRWDT();
	}
}
